//  Implementation of the TCG BIOS extension according to the specification
//  described in specs found at
//  http://www.trustedcomputinggroup.org/resources/pc_client_work_group_specific_implementation_specification_for_conventional_bios
//
//  Copyright (C) 2006-2011, 2014, 2015 IBM Corporation
//
//  Authors:
//      Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "bregs.h" // struct bregs
#include "byteorder.h" // cpu_to_*
#include "config.h" // CONFIG_TCGBIOS
#include "farptr.h" // MAKE_FLATPTR
#include "fw/paravirt.h" // runningOnXen
#include "hw/tpm_drivers.h" // tpm_drivers[]
#include "output.h" // dprintf
#include "sha.h" // sha1, sha256, ...
#include "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#include "std/smbios.h" // struct smbios_21_entry_point
#include "std/tcg.h" // TCG_PC_LOGOVERFLOW
#include "string.h" // checksum
#include "tcgbios.h"// tpm_*, prototypes
#include "util.h" // printf, get_keystroke
#include "stacks.h" // wait_threads, reset
#include "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

struct {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_count;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

static 정수 tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    만약 (!log_area_start_address || !log_area_minimum_length)
        반환 -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = NULL;
    tpm_state.entry_count = 0;
    반환 0;
}

static 정수
tpm_tcpa_probe(void)
{
    struct tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    만약 (!tcpa)
        반환 -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    반환 tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

static 정수
tpm_tpm2_probe(void)
{
    struct tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    만약 (!tpm2)
        반환 -1;

    만약 (tpm2->length < 76)
        반환 -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    반환 tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
                            tpm2->log_area_minimum_length);
}

/*
 * Extend the ACPI log with the given entry by copying the
 * entry data into the log.
 * Input
 *  entry : The header data to use (including the variable length digest)
 *  digest_len : Length of the digest in 'entry'
 *  event : Pointer to the event body to be copied into the log
 *  event_len : Length of 'event'
 *
 * Output:
 *  Returns an error code in case of faiure, 0 in case of success
 */
static 정수
tpm_log_event(struct tpm_log_header *entry, 정수 digest_len
              , const void *event, 정수 event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    만약 (tpm_state.log_area_next_entry == NULL)
        반환 -1;

    u32 size = (sizeof(*entry) + digest_len
                + sizeof(struct tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    만약 (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        반환 -1;
    }

    void *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, sizeof(*entry) + digest_len);
    struct tpm_log_trailer *t = dest + sizeof(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_count++;

    반환 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

static TPMVersion TPM_version;
static u32 tpm20_pcr_selection_size;
static struct tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
struct tpm_log_entry {
    struct tpm_log_header hdr;
    u8 pad[sizeof(struct tpm2_digest_values)
           + 8 * sizeof(struct tpm2_digest_value)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

static const struct hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    const char *name;
    void (*hashfunc)(const u8 *data, u32 length, u8 *hash);
} hash_parameters[] = {
    {
        .hashalg = TPM2_ALG_SHA1,
        .hashalg_flag = TPM2_ALG_SHA1_FLAG,
        .hash_buffersize = SHA1_BUFSIZE,
        .name = "SHA1",
        .hashfunc = sha1,
    }, {
        .hashalg = TPM2_ALG_SHA256,
        .hashalg_flag = TPM2_ALG_SHA256_FLAG,
        .hash_buffersize = SHA256_BUFSIZE,
        .name = "SHA256",
        .hashfunc = sha256,
    }, {
        .hashalg = TPM2_ALG_SHA384,
        .hashalg_flag = TPM2_ALG_SHA384_FLAG,
        .hash_buffersize = SHA384_BUFSIZE,
        .name = "SHA384",
        .hashfunc = sha384,
    }, {
        .hashalg = TPM2_ALG_SHA512,
        .hashalg_flag = TPM2_ALG_SHA512_FLAG,
        .hash_buffersize = SHA512_BUFSIZE,
        .name = "SHA512",
        .hashfunc = sha512,
    }, {
        .hashalg = TPM2_ALG_SM3_256,
        .hashalg_flag = TPM2_ALG_SM3_256_FLAG,
        .hash_buffersize = SM3_256_BUFSIZE,
        .name = "SM3-256",
    }, {
        .hashalg = TPM2_ALG_SHA3_256,
        .hashalg_flag = TPM2_ALG_SHA3_256_FLAG,
        .hash_buffersize = SHA3_256_BUFSIZE,
        .name = "SHA3-256",
    }, {
        .hashalg = TPM2_ALG_SHA3_384,
        .hashalg_flag = TPM2_ALG_SHA3_384_FLAG,
        .hash_buffersize = SHA3_384_BUFSIZE,
        .name = "SHA3-384",
    }, {
        .hashalg = TPM2_ALG_SHA3_512,
        .hashalg_flag = TPM2_ALG_SHA3_512_FLAG,
        .hash_buffersize = SHA3_512_BUFSIZE,
        .name = "SHA3-512",
    }
};

static 정수
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    위해 (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        만약 (hash_parameters[i].hashalg == hashAlg)
            반환 hash_parameters[i].hash_buffersize;
    }
    반환 -1;
}

static u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    위해 (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        만약 (hash_parameters[i].hashalg == hashAlg)
            반환 hash_parameters[i].hashalg_flag;
    }
    반환 0;
}

static u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    위해 (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        만약 (hash_parameters[i].hashalg_flag == hashalg_flag)
            반환 hash_parameters[i].hashalg;
    }
    반환 0;
}

static const char *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    위해 (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        만약 (hash_parameters[i].hashalg_flag == hashalg_flag)
            반환 hash_parameters[i].name;
    }
    반환 NULL;
}

static void tpm2_hash_data(u16 hashAlg, const u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    위해 (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        만약 (hash_parameters[i].hashalg == hashAlg) {
            만약 (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } 아니면 {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
static 정수
tpm20_write_EfiSpecIdEventStruct(void)
{
    만약 (!tpm20_pcr_selection)
        반환 -1;

    struct {
        struct TCG_EfiSpecIdEventStruct hdr;
        u8 pad[sizeof(struct tpm_log_entry) + sizeof(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    struct tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    void *nsel, *end = (void*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 count, numAlgs = 0;
    위해 (count = 0; count < be32_to_cpu(tpm20_pcr_selection->count); count++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (void*)sel + sizeof(*sel) + sizeOfSelect;
        만약 (nsel > end)
            break;

        만약 (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            continue;
        }

        정수 hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        만약 (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            반환 -1;
        }

        정수 event_size = offsetof(struct TCG_EfiSpecIdEventStruct
                                  , digestSizes[count+1]);
        만약 (event_size > sizeof(event) - sizeof(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            반환 -1;
        }

        event.hdr.digestSizes[numAlgs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[numAlgs].digestSize = hsize;
        numAlgs++;

        sel = nsel;
    }

    만약 (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        반환 -1;
    }

    event.hdr.numberOfAlgorithms = numAlgs;
    정수 event_size = offsetof(struct TCG_EfiSpecIdEventStruct
                              , digestSizes[numAlgs]);
    u8 *vendorInfoSize = (void*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += sizeof(*vendorInfoSize);

    struct tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    반환 tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
}

/*
 * Build the TPM2 tpm2_digest_values data structure from the given hash.
 * Follow the PCR bank configuration of the TPM and write the same hash
 * in either truncated or zero-padded form in the areas of all the other
 * hashes. For example, write the sha1 hash in the area of the sha256
 * hash and fill the remaining bytes with zeros. Or truncate the sha256
 * hash when writing it in the area of the sha1 hash.
 *
 * le: the log entry to build the digest in
 * hashdata: the data to hash
 * hashdata_len: the length of the hashdata
 * bigEndian: whether to build in big endian format for the TPM or
 *            little endian for the log
 *
 * Returns the digest size; -1 on fatal error
 */
static 정수
tpm20_build_digest(struct tpm_log_entry *le,
                   const u8 *hashdata, u32 hashdata_len, 정수 bigEndian)
{
    만약 (!tpm20_pcr_selection)
        반환 -1;

    struct tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    void *nsel, *end = (void*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    void *dest = le->hdr.digest + sizeof(struct tpm2_digest_values);

    u32 count, numAlgs = 0;
    위해 (count = 0; count < be32_to_cpu(tpm20_pcr_selection->count); count++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (void*)sel + sizeof(*sel) + sizeOfSelect;
        만약 (nsel > end)
            break;

        /* PCR 0-7 unused? -- skip */
        만약 (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            continue;
        }

        정수 hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        만약 (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            반환 -1;
        }

        /* buffer size sanity check before writing */
        struct tpm2_digest_value *v = dest;
        만약 (dest + sizeof(*v) + hsize > (void*)le + sizeof(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            반환 -1;
        }

        만약 (bigEndian)
            v->hashAlg = sel->hashAlg;
        아니면
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += sizeof(*v) + hsize;
        sel = nsel;

        numAlgs++;
    }

    만약 (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        반환 -1;
    }

    struct tpm2_digest_values *v = (void*)le->hdr.digest;
    만약 (bigEndian)
        v->count = cpu_to_be32(numAlgs);
    아니면
        v->count = numAlgs;

    반환 dest - (void*)le->hdr.digest;
}

static 정수
tpm12_build_digest(struct tpm_log_entry *le,
                   const u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    반환 SHA1_BUFSIZE;
}

static 정수
tpm12_build_digest_direct(struct tpm_log_entry *le, const u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    반환 SHA1_BUFSIZE;
}

static 정수
tpm_build_digest(struct tpm_log_entry *le, const u8 *hashdata, u32 hashdata_len
                 , 정수 bigEndian)
{
    switch (TPM_version) {
    case TPM_VERSION_1_2:
        반환 tpm12_build_digest(le, hashdata, hashdata_len);
    case TPM_VERSION_2:
        반환 tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    반환 -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
static 정수
tpm_simple_cmd(u8 locty, u32 ordinal
               , 정수 param_size, u16 param, enum tpmDurationType to_t)
{
    struct {
        struct tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(sizeof(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    switch (TPM_version) {
    case TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        break;
    case TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        break;
    }

    u8 obuffer[64];
    struct tpm_rsp_header *trsh = (void*)obuffer;
    u32 obuffer_len = sizeof(obuffer);
    memset(obuffer, 0x0, sizeof(obuffer));

    정수 ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    반환 ret;
}

static 정수
tpm20_getcapability(u32 capability, u32 property, u32 count,
                    struct tpm_rsp_header *rsp, u32 rsize)
{
    struct tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(sizeof(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(count),
    };

    u32 resp_size = rsize;
    정수 ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    반환 ret;
}

static 정수
tpm20_get_pcrbanks(void)
{
    u8 buffer[128];
    struct tpm2_res_getcapability *trg =
      (struct tpm2_res_getcapability *)&buffer;

    정수 ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  sizeof(buffer));
    만약 (ret)
        반환 ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    만약 (resplen <= offsetof(struct tpm2_res_getcapability, data))
        반환 -1;

    u32 size = resplen - offsetof(struct tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    만약 (size < offsetof(struct tpml_pcr_selection, selections) +
               offsetof(struct tpms_pcr_selection, pcrSelect))
        반환 -1;

    tpm20_pcr_selection = malloc_high(size);
    만약 (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } 아니면 {
        warn_noalloc();
        ret = -1;
    }

    반환 ret;
}

static 정수
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    만약 (!tpm20_pcr_selection)
        반환 -1;

    struct tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    void *end = (void*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    동안 (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        void *nsel = (void*)sel + sizeof(*sel) + sizeOfSelect;
        만약 (nsel > end)
            반환 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        위해 (i = 0; i < sizeOfSelect; i++) {
            만약 (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                break;
            }
        }

        sel = nsel;
    }
}

static 정수
tpm20_set_pcrbanks(u32 active_banks)
{
    struct tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(sizeof(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    struct tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    정수 i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    동안 (hashalg_flag) {
        만약 ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            만약 (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                만약 (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.count = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * sizeof(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(struct tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * sizeof(tps[0]));

    struct tpm_rsp_header rsp;
    u32 resp_length = sizeof(rsp);

    정수 ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    반환 ret;
}

static 정수 tpm20_activate_pcrbanks(u32 active_banks)
{
    정수 ret = tpm20_set_pcrbanks(active_banks);
    만약 (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    만약 (!ret)
        reset();
    반환 ret;
}

static 정수
tpm12_get_capability(u32 cap, u32 subcap, struct tpm_rsp_header *rsp, u32 rsize)
{
    struct tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(sizeof(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(sizeof(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    정수 ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    반환 ret;
}

static 정수
tpm12_read_permanent_flags(char *buf, 정수 buf_len)
{
    memset(buf, 0, buf_len);

    struct tpm_res_getcap_perm_flags pf;
    정수 ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, sizeof(pf));
    만약 (ret)
        반환 -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    반환 0;
}

static 정수
tpm12_determine_timeouts(void)
{
    struct tpm_res_getcap_timeouts timeouts;
    정수 ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, sizeof(timeouts));
    만약 (ret)
        반환 ret;

    struct tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, sizeof(durations));
    만약 (ret)
        반환 ret;

    정수 i;
    위해 (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    위해 (i = 0; i < 4; i++)
        timeouts.timeouts[i] = be32_to_cpu(timeouts.timeouts[i]);

    dprintf(DEBUG_tcg, "TCGBIOS: timeouts: %u %u %u %u\n",
            timeouts.timeouts[0],
            timeouts.timeouts[1],
            timeouts.timeouts[2],
            timeouts.timeouts[3]);

    dprintf(DEBUG_tcg, "TCGBIOS: durations: %u %u %u\n",
            durations.durations[0],
            durations.durations[1],
            durations.durations[2]);

    tpmhw_set_timeouts(timeouts.timeouts, durations.durations);

    반환 0;
}

static void
tpm20_set_timeouts(void)
{
    u32 durations[3] = {
        TPM2_DEFAULT_DURATION_SHORT,
        TPM2_DEFAULT_DURATION_MEDIUM,
        TPM2_DEFAULT_DURATION_LONG,
    };
    u32 timeouts[4] = {
        TIS2_DEFAULT_TIMEOUT_A,
        TIS2_DEFAULT_TIMEOUT_B,
        TIS2_DEFAULT_TIMEOUT_C,
        TIS2_DEFAULT_TIMEOUT_D,
    };

    tpmhw_set_timeouts(timeouts, durations);
}

static 정수
tpm12_extend(struct tpm_log_entry *le, 정수 digest_len)
{
    struct tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(sizeof(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, sizeof(tre.digest));

    struct tpm_rsp_extend rsp;
    u32 resp_length = sizeof(rsp);
    정수 ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    만약 (ret || resp_length != sizeof(rsp) || rsp.hdr.errcode)
        반환 -1;

    반환 0;
}

static 정수 tpm20_extend(struct tpm_log_entry *le, 정수 digest_len)
{
    struct tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(sizeof(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[sizeof(tmp_tre) + sizeof(le->pad)];
    struct tpm2_req_extend *tre = (struct tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, sizeof(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(sizeof(tmp_tre) + digest_len);

    struct tpm_rsp_header rsp;
    u32 resp_length = sizeof(rsp);
    정수 ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    만약 (ret || resp_length != sizeof(rsp) || rsp.errcode)
        반환 -1;

    반환 0;
}

static 정수
tpm_extend(struct tpm_log_entry *le, 정수 digest_len)
{
    switch (TPM_version) {
    case TPM_VERSION_1_2:
        반환 tpm12_extend(le, digest_len);
    case TPM_VERSION_2:
        반환 tpm20_extend(le, digest_len);
    }
    반환 -1;
}

static 정수
tpm20_stirrandom(void)
{
    struct tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(sizeof(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(sizeof(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    struct tpm_rsp_header rsp;
    u32 resp_length = sizeof(rsp);
    정수 ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    만약 (ret || resp_length != sizeof(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    반환 ret;
}

static 정수
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    struct tpm2_res_getrandom rsp;

    만약 (buf_len > sizeof(rsp.rnd.buffer))
        반환 -1;

    struct tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(sizeof(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = sizeof(rsp);

    정수 ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    만약 (ret || resp_length != sizeof(rsp) || rsp.hdr.errcode)
        ret = -1;
    아니면
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    반환 ret;
}

static 정수
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    struct tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(sizeof(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(sizeof(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    struct tpm_rsp_header rsp;
    u32 resp_length = sizeof(rsp);
    정수 ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    만약 (ret || resp_length != sizeof(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    반환 ret;
}

static 정수
tpm20_hierarchychangeauth(u8 auth[20])
{
    struct tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(sizeof(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(sizeof(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(sizeof(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, sizeof(trhca.newAuth.buffer));

    struct tpm_rsp_header rsp;
    u32 resp_length = sizeof(rsp);
    정수 ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    만약 (ret || resp_length != sizeof(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    반환 ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

static 정수 TPM_has_physical_presence;
u8 TPM_working VARLOW;

static 정수
tpm_is_working(void)
{
    반환 CONFIG_TCGBIOS && TPM_working;
}

static void
tpm_set_failure(void)
{
    switch (TPM_version) {
    case TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        break;
    case TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        break;
    }

    TPM_working = 0;
}

/*
 * Add a measurement to the log; the data at data_seg:data/length are
 * appended to the TCG_PCClientPCREventStruct
 *
 * Input parameters:
 *  pcrindex   : which PCR to extend
 *  event_type : type of event; specs section on 'Event Types'
 *  event       : pointer to info (e.g., string) to be added to log as-is
 *  event_length: length of the event
 *  hashdata    : pointer to the data to be hashed
 *  hashdata_length: length of the data to be hashed
 */
static void
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           const char *event, u32 event_length,
                           const u8 *hashdata, u32 hashdata_length)
{
    만약 (!tpm_is_working())
        반환;

    struct tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    정수 digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    만약 (digest_len < 0)
        반환;
    정수 ret = tpm_extend(&le, digest_len);
    만약 (ret) {
        tpm_set_failure();
        반환;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
static void
tpm_add_action(u32 pcrIndex, const char *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
static void
tpm_add_event_separators(void)
{
    static const u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    위해 (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (const char *)evt_separator,
                                   sizeof(evt_separator),
                                   evt_separator,
                                   sizeof(evt_separator));
}

static void
tpm_smbios_measure(void)
{
    struct pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    void *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    만약 (!smbios_tables)
        반환;

    sha1((const u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (const char *)&pcctes, sizeof(pcctes),
                               (u8 *)&pcctes, sizeof(pcctes));
}

static 정수
tpm12_assert_physical_presence(void)
{
    정수 ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    만약 (!ret)
        반환 0;

    struct tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((char *)&pf, sizeof(pf));
    만약 (ret)
        반환 -1;

    /* check if hardware physical presence is supported */
    만약 (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        반환 0;
    }

    만약 (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        반환 tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    반환 -1;
}

static 정수
tpm12_startup(void)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    정수 ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    만약 (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    만약 (ret)
        goto err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    만약 (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    만약 (ret)
        goto err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    만약 (ret)
        goto err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    만약 (ret && ret != TPM_BAD_LOCALITY)
        goto err_exit;

    반환 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    반환 -1;
}

static 정수
tpm20_startup(void)
{
    tpm20_set_timeouts();

    정수 ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    만약 (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    만약 (ret)
        goto err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    만약 (ret)
        goto err_exit;

    ret = tpm20_get_pcrbanks();
    만약 (ret)
        goto err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    만약 (ret)
        goto err_exit;

    반환 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    반환 -1;
}

static 정수
tpm_startup(void)
{
    switch (TPM_version) {
    case TPM_VERSION_1_2:
        반환 tpm12_startup();
    case TPM_VERSION_2:
        반환 tpm20_startup();
    }
    반환 -1;
}

void
tpm_setup(void)
{
    만약 (!CONFIG_TCGBIOS)
        반환;

    정수 ret = tpm_tpm2_probe();
    만약 (ret) {
        ret = tpm_tcpa_probe();
        만약 (ret)
            반환;
    }

    TPM_version = tpmhw_probe();
    만약 (TPM_version == TPM_VERSION_NONE)
        반환;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    만약 (runningOnXen())
        반환;

    ret = tpm_startup();
    만약 (ret)
        반환;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

static void
tpm20_prepboot(void)
{
    정수 ret = tpm20_stirrandom();
    만약 (ret)
         goto err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], sizeof(auth));
    만약 (ret)
        goto err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    만약 (ret)
        goto err_exit;

    반환;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

void
tpm_prepboot(void)
{
    만약 (!CONFIG_TCGBIOS)
        반환;

    switch (TPM_version) {
    case TPM_VERSION_1_2:
        만약 (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        break;
    case TPM_VERSION_2:
        tpm20_prepboot();
        break;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
void
tpm_option_rom(const void *addr, u32 len)
{
    만약 (!tpm_is_working())
        반환;

    struct pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = sizeof(u16) + sizeof(u16) + SHA1_BUFSIZE,
    };
    sha1((const u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (const char *)&pcctes, sizeof(pcctes),
                               (u8 *)&pcctes, sizeof(pcctes));
}

void
tpm_add_bcv(u32 bootdrv, const u8 *addr, u32 length)
{
    만약 (!tpm_is_working())
        반환;

    만약 (length < 0x200)
        반환;

    const char *string = "Booting BCV device 00h (Floppy)";
    만약 (bootdrv == 0x80)
        string = "Booting BCV device 80h (HDD)";
    tpm_add_action(4, string);

    /* specs: see section 'Hard Disk Device or Hard Disk-Like Devices' */
    /* equivalent to: dd if=/dev/hda ibs=1 count=440 | sha1sum */
    string = "MBR";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, 0x1b8);

    /* equivalent to: dd if=/dev/hda ibs=1 count=72 skip=440 | sha1sum */
    string = "MBR PARTITION_TABLE";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr + 0x1b8, 0x48);
}

void
tpm_add_cdrom(u32 bootdrv, const u8 *addr, u32 length)
{
    만약 (!tpm_is_working())
        반환;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    const char *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

void
tpm_add_cdrom_catalog(const u8 *addr, u32 length)
{
    만약 (!tpm_is_working())
        반환;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    const char *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

void
tpm_s3_resume(void)
{
    만약 (!tpm_is_working())
        반환;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    정수 ret = -1;

    switch (TPM_version) {
    case TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        break;
    case TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        만약 (ret)
            goto err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        break;
    }

    만약 (ret)
        goto err_exit;

    반환;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

static inline void *input_buf32(struct bregs *regs)
{
    반환 MAKE_FLATPTR(regs->es, regs->di);
}

static inline void *output_buf32(struct bregs *regs)
{
    반환 MAKE_FLATPTR(regs->ds, regs->si);
}

static u32
hash_log_extend(struct pcpes *pcpes, const void *hashdata, u32 hashdata_length
                , void *event, 정수 extend)
{
    만약 (pcpes->pcrindex >= 24)
        반환 TCG_INVALID_INPUT_PARA;
    만약 (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    struct tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    정수 digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    만약 (digest_len < 0)
        반환 TCG_GENERAL_ERROR;
    만약 (extend) {
        정수 ret = tpm_extend(&le, digest_len);
        만약 (ret)
            반환 TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    정수 ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    만약 (ret)
        반환 TCG_PC_LOGOVERFLOW;
    반환 0;
}

static u32
hash_log_extend_event_int(const struct hleei_short *hleei_s,
                          struct hleeo *hleeo)
{
    u32 rc = 0;
    struct hleei_long *hleei_l = (struct hleei_long *)hleei_s;
    const void *logdataptr;
    u32 logdatalen;
    struct pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    switch (hleei_s->ipblength) {
    case sizeof(struct hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    break;

    case sizeof(struct hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    break;

    default:
        /* bad input block */
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }

    pcpes = (struct pcpes *)logdataptr;

    만약 (pcpes->pcrindex != pcrindex
        || logdatalen != sizeof(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    만약 (rc)
        goto err_exit;

    hleeo->opblength = sizeof(struct hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_count;
    memcpy(hleeo->digest, pcpes->digest, sizeof(hleeo->digest));

err_exit:
    만약 (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    반환 rc;
}

static u32
pass_through_to_tpm_int(struct pttti *pttti, struct pttto *pttto)
{
    u32 rc = 0;
    struct tpm_req_header *trh = (void*)pttti->tpmopin;

    만약 (pttti->ipblength < sizeof(struct pttti) + sizeof(*trh)
        || pttti->ipblength != sizeof(struct pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < sizeof(struct pttto)) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    switch (TPM_version) {
    case TPM_VERSION_1_2:
        만약 (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_INPUT_PARA;
            goto err_exit;
        }
        break;
    case TPM_VERSION_2:
        만약 (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_INPUT_PARA;
            goto err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(struct pttto, tpmopout);
    정수 ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    만약 (ret) {
        rc = TCG_FATAL_COM_ERROR;
        goto err_exit;
    }

    pttto->opblength = offsetof(struct pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    만약 (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    반환 rc;
}

static u32
shutdown_preboot_interface(void)
{
    TPM_interface_shutdown = 1;
    반환 0;
}

static u32
hash_log_event_int(const struct hlei *hlei, struct hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    struct pcpes *pcpes;

    size = hlei->ipblength;
    만약 (size != sizeof(*hlei)) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }

    pcpes = (struct pcpes *)hlei->logdataptr;

    만약 (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != sizeof(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    만약 (rc)
        goto err_exit;

    /* updating the log was fine */
    hleo->opblength = sizeof(struct hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_count;

err_exit:
    만약 (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    반환 rc;
}

static u32
hash_all_int(const struct hai *hai, u8 *hash)
{
    만약 (hai->ipblength != sizeof(struct hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        반환 TCG_INVALID_INPUT_PARA;

    sha1((const u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    반환 0;
}

static u32
tss_int(struct ti *ti, struct to *to)
{
    to->opblength = sizeof(struct to);
    to->reserved  = 0;

    반환 TCG_PC_UNSUPPORTED;
}

static u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    struct pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = sizeof(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    만약 (rc)
        반환 rc;

    *edx_ptr = tpm_state.entry_count;
    반환 0;
}

void VISIBLE32FLAT
tpm_interrupt_handler32(struct bregs *regs)
{
    만약 (!CONFIG_TCGBIOS)
        반환;

    set_cf(regs, 0);

    만약 (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        반환;
    }

    switch ((enum irq_ids)regs->al) {
    case TCG_StatusCheck:
        만약 (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } 아니면 {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        break;

    case TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (struct hleei_short *)input_buf32(regs),
                  (struct hleeo *)output_buf32(regs));
        break;

    case TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((struct pttti *)input_buf32(regs),
                                    (struct pttto *)output_buf32(regs));
        break;

    case TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        break;

    case TCG_HashLogEvent:
        regs->eax = hash_log_event_int((struct hlei*)input_buf32(regs),
                                       (struct hleo*)output_buf32(regs));
        break;

    case TCG_HashAll:
        regs->eax =
            hash_all_int((struct hai*)input_buf32(regs),
                          (u8 *)output_buf32(regs));
        break;

    case TCG_TSS:
        regs->eax = tss_int((struct ti*)input_buf32(regs),
                            (struct to*)output_buf32(regs));
        break;

    case TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)input_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        break;

    default:
        set_cf(regs, 1);
    }

    반환;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

typedef u8 tpm_ppi_code;

static 정수
tpm12_read_has_owner(정수 *has_owner)
{
    struct tpm_res_getcap_ownerauth oauth;
    정수 ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, sizeof(oauth));
    만약 (ret)
        반환 -1;

    *has_owner = oauth.flag;

    반환 0;
}

static 정수
tpm12_enable_tpm(정수 enable, 정수 verbose)
{
    struct tpm_permanent_flags pf;
    정수 ret = tpm12_read_permanent_flags((char *)&pf, sizeof(pf));
    만약 (ret)
        반환 -1;

    만약 (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        반환 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    만약 (ret) {
        만약 (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        아니면
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    반환 ret;
}

static 정수
tpm12_activate_tpm(정수 activate, 정수 allow_reset, 정수 verbose)
{
    struct tpm_permanent_flags pf;
    정수 ret = tpm12_read_permanent_flags((char *)&pf, sizeof(pf));
    만약 (ret)
        반환 -1;

    만약 (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        반환 0;

    만약 (pf.flags[PERM_FLAG_IDX_DISABLE])
        반환 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    만약 (ret)
        반환 ret;

    만약 (activate && allow_reset) {
        만약 (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    반환 0;
}

static 정수
tpm12_enable_activate(정수 allow_reset, 정수 verbose)
{
    정수 ret = tpm12_enable_tpm(1, verbose);
    만약 (ret)
        반환 ret;

    반환 tpm12_activate_tpm(1, allow_reset, verbose);
}

static 정수
tpm12_force_clear(정수 enable_activate_before, 정수 enable_activate_after,
                  정수 verbose)
{
    정수 has_owner;
    정수 ret = tpm12_read_has_owner(&has_owner);
    만약 (ret)
        반환 -1;
    만약 (!has_owner) {
        만약 (verbose)
            printf("TPM does not have an owner.\n");
        반환 0;
    }

    만약 (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        만약 (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            반환 ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    만약 (ret)
        반환 ret;

    만약 (!enable_activate_after) {
        만약 (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        반환 0;
    }

    반환 tpm12_enable_activate(1, verbose);
}

static 정수
tpm12_set_owner_install(정수 allow, 정수 verbose)
{
    정수 has_owner;
    정수 ret = tpm12_read_has_owner(&has_owner);
    만약 (ret)
        반환 -1;
    만약 (has_owner) {
        만약 (verbose)
            printf("Must first remove owner.\n");
        반환 0;
    }

    struct tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((char *)&pf, sizeof(pf));
    만약 (ret)
        반환 -1;

    만약 (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        만약 (verbose)
            printf("TPM must first be enable.\n");
        반환 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    만약 (ret)
        반환 ret;

    만약 (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    반환 0;
}

static 정수
tpm12_process_cfg(tpm_ppi_code msgCode, 정수 verbose)
{
    정수 ret = 0;

    switch (msgCode) {
        case TPM_PPI_OP_NOOP: /* no-op */
            break;

        case TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            break;

        case TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            break;

        case TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            break;

        case TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            break;

        case TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            break;

        case TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            break;

        case TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            break;

        default:
            break;
    }

    만약 (ret)
        printf("Op %d: An error occurred: 0x%x\n", msgCode, ret);

    반환 ret;
}

static 정수
tpm20_clearcontrol(u8 disable, 정수 verbose)
{
    struct tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(sizeof(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(sizeof(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    struct tpm_rsp_header rsp;
    u32 resp_length = sizeof(rsp);
    정수 ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    만약 (ret || resp_length != sizeof(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    반환 ret;
}

static 정수
tpm20_clear(void)
{
    struct tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(sizeof(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(sizeof(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    struct tpm_rsp_header rsp;
    u32 resp_length = sizeof(rsp);
    정수 ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    만약 (ret || resp_length != sizeof(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    반환 ret;
}

static 정수
tpm20_process_cfg(tpm_ppi_code msgCode, 정수 verbose)
{
    정수 ret = 0;

    switch (msgCode) {
        case TPM_PPI_OP_NOOP: /* no-op */
            break;

        case TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            만약 (!ret)
                 ret = tpm20_clear();
            break;
    }

    만약 (ret)
        printf("Op %d: An error occurred: 0x%x\n", msgCode, ret);

    반환 ret;
}

static 정수
tpm12_get_tpm_state(void)
{
    정수 state = 0;
    struct tpm_permanent_flags pf;
    정수 has_owner;

    만약 (tpm12_read_permanent_flags((char *)&pf, sizeof(pf)) ||
        tpm12_read_has_owner(&has_owner))
        반환 ~0;

    만약 (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    만약 (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    만약 (has_owner) {
        state |= TPM_STATE_OWNED;
    } 아니면 {
        만약 (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    반환 state;
}

static void
tpm12_show_tpm_menu(정수 state, 정수 next_scancodes[7])
{
    정수 i = 0;

    printf("\nThe current state of the TPM is:\n");

    만약 (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    아니면
        printf("  Disabled");

    만약 (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    아니면
        printf(" and deactivated\n");

    만약 (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    아니면 {
        printf("  Ownership has not been taken\n");
        만약 (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        아니면
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    만약 ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    만약 (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        만약 (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            만약 (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } 아니면 {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } 아니면 {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } 아니면 {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    만약 (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

static void
tpm12_menu(void)
{
    정수 scancode, next_scancodes[7];
    tpm_ppi_code msgCode;
    정수 state = 0, i;
    정수 waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    위해 (;;) {
        만약 ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } 아니면 {
            printf("TPM is not working correctly.\n");
            반환;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        msgCode = TPM_PPI_OP_NOOP;

        waitkey = 1;

        동안 (waitkey) {
            동안 ((scancode = get_keystroke(1000)) == ~0)
                ;

            switch (scancode) {
            case 1:
                // ESC
                reset();
                break;
            case 18: /* e. enable */
                msgCode = TPM_PPI_OP_ENABLE;
                break;
            case 32: /* d. disable */
                msgCode = TPM_PPI_OP_DISABLE;
                break;
            case 30: /* a. activate */
                msgCode = TPM_PPI_OP_ACTIVATE;
                break;
            case 47: /* v. deactivate */
                msgCode = TPM_PPI_OP_DEACTIVATE;
                break;
            case 46: /* c. clear owner */
                msgCode = TPM_PPI_OP_CLEAR;
                break;
            case 25: /* p. prevent ownerinstall */
                msgCode = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                break;
            case 31: /* s. allow ownerinstall */
                msgCode = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                break;
            default:
                continue;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            위해 (i = 0; i < sizeof(next_scancodes); i++) {
                만약 (next_scancodes[i] == 0)
                    break;

                만약 (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(msgCode, 1);
                    waitkey = 0;
                    break;
                }
            }
        }
    }
}

static 정수
tpm20_menu_change_active_pcrbanks(void)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    동안 (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        동안 (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            const char *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            만약 (hashname) {
                printf("  %d: %s", i, hashname);
                만약 (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        만약 (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        정수 show = 0;
        동안 (!show) {
            정수 scancode = get_keystroke(1000);

            switch (scancode) {
            case ~0:
                continue;
            case 1: /* ESC */
                printf("\n");
                반환 -1;
            case 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                만약 (flagnum > i)
                    continue;
                만약 (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                break;
            case 30: /* a */
                만약 (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

static void
tpm20_menu(void)
{
    정수 scan_code;
    tpm_ppi_code msgCode;

    위해 (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        msgCode = TPM_PPI_OP_NOOP;

        동안 ((scan_code = get_keystroke(1000)) == ~0)
            ;

        switch (scan_code) {
        case 1:
            // ESC
            reset();
            break;
        case 2:
            msgCode = TPM_PPI_OP_CLEAR;
            break;
        case 3:
            tpm20_menu_change_active_pcrbanks();
            continue;
        default:
            continue;
        }

        tpm20_process_cfg(msgCode, 0);
    }
}

void
tpm_menu(void)
{
    만약 (!CONFIG_TCGBIOS)
        반환;

    동안 (get_keystroke(0) >= 0)
        ;
    wait_threads();

    switch (TPM_version) {
    case TPM_VERSION_1_2:
        tpm12_menu();
        break;
    case TPM_VERSION_2:
        tpm20_menu();
        break;
    }
}

정수
tpm_can_show_menu(void)
{
    switch (TPM_version) {
    case TPM_VERSION_1_2:
        반환 tpm_is_working() && TPM_has_physical_presence;
    case TPM_VERSION_2:
        반환 tpm_is_working();
    }
    반환 0;
}
