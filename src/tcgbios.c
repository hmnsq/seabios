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

sa_nracanaa {
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

static puur_naa_nka tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    ꦪꦼꦤ꧀ (!log_area_start_address || !log_area_minimum_length)
        ꦮꦭꦶ -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = NULL;
    tpm_state.entry_count = 0;
    ꦮꦭꦶ 0;
}

static puur_naa_nka
tpm_tcpa_probe(void)
{
    sa_nracanaa tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    ꦪꦼꦤ꧀ (!tcpa)
        ꦮꦭꦶ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    ꦮꦭꦶ tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

static puur_naa_nka
tpm_tpm2_probe(void)
{
    sa_nracanaa tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    ꦪꦼꦤ꧀ (!tpm2)
        ꦮꦭꦶ -1;

    ꦪꦼꦤ꧀ (tpm2->length < 76)
        ꦮꦭꦶ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    ꦮꦭꦶ tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
static puur_naa_nka
tpm_log_event(sa_nracanaa tpm_log_header *entry, puur_naa_nka digest_len
              , const void *event, puur_naa_nka event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    ꦪꦼꦤ꧀ (tpm_state.log_area_next_entry == NULL)
        ꦮꦭꦶ -1;

    u32 size = (parimaa_na(*entry) + digest_len
                + parimaa_na(sa_nracanaa tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    ꦪꦼꦤ꧀ (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        ꦮꦭꦶ -1;
    }

    void *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, parimaa_na(*entry) + digest_len);
    sa_nracanaa tpm_log_trailer *t = dest + parimaa_na(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_count++;

    ꦮꦭꦶ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

static TPMVersion TPM_version;
static u32 tpm20_pcr_selection_size;
static sa_nracanaa tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
sa_nracanaa tpm_log_entry {
    sa_nracanaa tpm_log_header hdr;
    u8 pad[parimaa_na(sa_nracanaa tpm2_digest_values)
           + 8 * parimaa_na(sa_nracanaa tpm2_digest_value)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

static const sa_nracanaa hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    const var_na *name;
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

static puur_naa_nka
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    ꦏꦁꦒꦺꦴ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ꦪꦼꦤ꧀ (hash_parameters[i].hashalg == hashAlg)
            ꦮꦭꦶ hash_parameters[i].hash_buffersize;
    }
    ꦮꦭꦶ -1;
}

static u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    ꦏꦁꦒꦺꦴ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ꦪꦼꦤ꧀ (hash_parameters[i].hashalg == hashAlg)
            ꦮꦭꦶ hash_parameters[i].hashalg_flag;
    }
    ꦮꦭꦶ 0;
}

static u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    ꦏꦁꦒꦺꦴ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ꦪꦼꦤ꧀ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ꦮꦭꦶ hash_parameters[i].hashalg;
    }
    ꦮꦭꦶ 0;
}

static const var_na *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    ꦏꦁꦒꦺꦴ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ꦪꦼꦤ꧀ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ꦮꦭꦶ hash_parameters[i].name;
    }
    ꦮꦭꦶ NULL;
}

static void tpm2_hash_data(u16 hashAlg, const u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    ꦏꦁꦒꦺꦴ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ꦪꦼꦤ꧀ (hash_parameters[i].hashalg == hashAlg) {
            ꦪꦼꦤ꧀ (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } ꦭꦶꦪ {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
static puur_naa_nka
tpm20_write_EfiSpecIdEventStruct(void)
{
    ꦪꦼꦤ꧀ (!tpm20_pcr_selection)
        ꦮꦭꦶ -1;

    sa_nracanaa {
        sa_nracanaa TCG_EfiSpecIdEventStruct hdr;
        u8 pad[parimaa_na(sa_nracanaa tpm_log_entry) + parimaa_na(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    sa_nracanaa tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    void *nsel, *end = (void*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 count, numAlgs = 0;
    ꦏꦁꦒꦺꦴ (count = 0; count < be32_to_cpu(tpm20_pcr_selection->count); count++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (void*)sel + parimaa_na(*sel) + sizeOfSelect;
        ꦪꦼꦤ꧀ (nsel > end)
            bha_nga;

        ꦪꦼꦤ꧀ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            continue;
        }

        puur_naa_nka hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ꦪꦼꦤ꧀ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ꦮꦭꦶ -1;
        }

        puur_naa_nka event_size = offsetof(sa_nracanaa TCG_EfiSpecIdEventStruct
                                  , digestSizes[count+1]);
        ꦪꦼꦤ꧀ (event_size > parimaa_na(event) - parimaa_na(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            ꦮꦭꦶ -1;
        }

        event.hdr.digestSizes[numAlgs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[numAlgs].digestSize = hsize;
        numAlgs++;

        sel = nsel;
    }

    ꦪꦼꦤ꧀ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ꦮꦭꦶ -1;
    }

    event.hdr.numberOfAlgorithms = numAlgs;
    puur_naa_nka event_size = offsetof(sa_nracanaa TCG_EfiSpecIdEventStruct
                              , digestSizes[numAlgs]);
    u8 *vendorInfoSize = (void*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += parimaa_na(*vendorInfoSize);

    sa_nracanaa tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    ꦮꦭꦶ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
static puur_naa_nka
tpm20_build_digest(sa_nracanaa tpm_log_entry *le,
                   const u8 *hashdata, u32 hashdata_len, puur_naa_nka bigEndian)
{
    ꦪꦼꦤ꧀ (!tpm20_pcr_selection)
        ꦮꦭꦶ -1;

    sa_nracanaa tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    void *nsel, *end = (void*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    void *dest = le->hdr.digest + parimaa_na(sa_nracanaa tpm2_digest_values);

    u32 count, numAlgs = 0;
    ꦏꦁꦒꦺꦴ (count = 0; count < be32_to_cpu(tpm20_pcr_selection->count); count++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (void*)sel + parimaa_na(*sel) + sizeOfSelect;
        ꦪꦼꦤ꧀ (nsel > end)
            bha_nga;

        /* PCR 0-7 unused? -- skip */
        ꦪꦼꦤ꧀ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            continue;
        }

        puur_naa_nka hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ꦪꦼꦤ꧀ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ꦮꦭꦶ -1;
        }

        /* buffer size sanity check before writing */
        sa_nracanaa tpm2_digest_value *v = dest;
        ꦪꦼꦤ꧀ (dest + parimaa_na(*v) + hsize > (void*)le + parimaa_na(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            ꦮꦭꦶ -1;
        }

        ꦪꦼꦤ꧀ (bigEndian)
            v->hashAlg = sel->hashAlg;
        ꦭꦶꦪ
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += parimaa_na(*v) + hsize;
        sel = nsel;

        numAlgs++;
    }

    ꦪꦼꦤ꧀ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ꦮꦭꦶ -1;
    }

    sa_nracanaa tpm2_digest_values *v = (void*)le->hdr.digest;
    ꦪꦼꦤ꧀ (bigEndian)
        v->count = cpu_to_be32(numAlgs);
    ꦭꦶꦪ
        v->count = numAlgs;

    ꦮꦭꦶ dest - (void*)le->hdr.digest;
}

static puur_naa_nka
tpm12_build_digest(sa_nracanaa tpm_log_entry *le,
                   const u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    ꦮꦭꦶ SHA1_BUFSIZE;
}

static puur_naa_nka
tpm12_build_digest_direct(sa_nracanaa tpm_log_entry *le, const u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    ꦮꦭꦶ SHA1_BUFSIZE;
}

static puur_naa_nka
tpm_build_digest(sa_nracanaa tpm_log_entry *le, const u8 *hashdata, u32 hashdata_len
                 , puur_naa_nka bigEndian)
{
    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        ꦮꦭꦶ tpm12_build_digest(le, hashdata, hashdata_len);
    pak_sa TPM_VERSION_2:
        ꦮꦭꦶ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    ꦮꦭꦶ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
static puur_naa_nka
tpm_simple_cmd(u8 locty, u32 ordinal
               , puur_naa_nka param_size, u16 param, ga_nanaa tpmDurationType to_t)
{
    sa_nracanaa {
        sa_nracanaa tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(parimaa_na(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        bha_nga;
    pak_sa TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        bha_nga;
    }

    u8 obuffer[64];
    sa_nracanaa tpm_rsp_header *trsh = (void*)obuffer;
    u32 obuffer_len = parimaa_na(obuffer);
    memset(obuffer, 0x0, parimaa_na(obuffer));

    puur_naa_nka ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_getcapability(u32 capability, u32 property, u32 count,
                    sa_nracanaa tpm_rsp_header *rsp, u32 rsize)
{
    sa_nracanaa tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(parimaa_na(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(count),
    };

    u32 resp_size = rsize;
    puur_naa_nka ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_get_pcrbanks(void)
{
    u8 buffer[128];
    sa_nracanaa tpm2_res_getcapability *trg =
      (sa_nracanaa tpm2_res_getcapability *)&buffer;

    puur_naa_nka ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  parimaa_na(buffer));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    ꦪꦼꦤ꧀ (resplen <= offsetof(sa_nracanaa tpm2_res_getcapability, data))
        ꦮꦭꦶ -1;

    u32 size = resplen - offsetof(sa_nracanaa tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    ꦪꦼꦤ꧀ (size < offsetof(sa_nracanaa tpml_pcr_selection, selections) +
               offsetof(sa_nracanaa tpms_pcr_selection, pcrSelect))
        ꦮꦭꦶ -1;

    tpm20_pcr_selection = malloc_high(size);
    ꦪꦼꦤ꧀ (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } ꦭꦶꦪ {
        warn_noalloc();
        ret = -1;
    }

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    ꦪꦼꦤ꧀ (!tpm20_pcr_selection)
        ꦮꦭꦶ -1;

    sa_nracanaa tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    void *end = (void*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    ꦱꦼꦢꦼꦭ (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        void *nsel = (void*)sel + parimaa_na(*sel) + sizeOfSelect;
        ꦪꦼꦤ꧀ (nsel > end)
            ꦮꦭꦶ 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        ꦏꦁꦒꦺꦴ (i = 0; i < sizeOfSelect; i++) {
            ꦪꦼꦤ꧀ (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                bha_nga;
            }
        }

        sel = nsel;
    }
}

static puur_naa_nka
tpm20_set_pcrbanks(u32 active_banks)
{
    sa_nracanaa tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(parimaa_na(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    sa_nracanaa tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    puur_naa_nka i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    ꦱꦼꦢꦼꦭ (hashalg_flag) {
        ꦪꦼꦤ꧀ ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            ꦪꦼꦤ꧀ (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                ꦪꦼꦤ꧀ (active_banks & hashalg_flag)
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
    memcpy(trpa.tpms_pcr_selections, tps, i * parimaa_na(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(sa_nracanaa tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * parimaa_na(tps[0]));

    sa_nracanaa tpm_rsp_header rsp;
    u32 resp_length = parimaa_na(rsp);

    puur_naa_nka ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    ꦮꦭꦶ ret;
}

static puur_naa_nka tpm20_activate_pcrbanks(u32 active_banks)
{
    puur_naa_nka ret = tpm20_set_pcrbanks(active_banks);
    ꦪꦼꦤ꧀ (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (!ret)
        reset();
    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm12_get_capability(u32 cap, u32 subcap, sa_nracanaa tpm_rsp_header *rsp, u32 rsize)
{
    sa_nracanaa tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(parimaa_na(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(parimaa_na(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    puur_naa_nka ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm12_read_permanent_flags(var_na *buf, puur_naa_nka buf_len)
{
    memset(buf, 0, buf_len);

    sa_nracanaa tpm_res_getcap_perm_flags pf;
    puur_naa_nka ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, parimaa_na(pf));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    ꦮꦭꦶ 0;
}

static puur_naa_nka
tpm12_determine_timeouts(void)
{
    sa_nracanaa tpm_res_getcap_timeouts timeouts;
    puur_naa_nka ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, parimaa_na(timeouts));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ ret;

    sa_nracanaa tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, parimaa_na(durations));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ ret;

    puur_naa_nka i;
    ꦏꦁꦒꦺꦴ (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    ꦏꦁꦒꦺꦴ (i = 0; i < 4; i++)
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

    ꦮꦭꦶ 0;
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

static puur_naa_nka
tpm12_extend(sa_nracanaa tpm_log_entry *le, puur_naa_nka digest_len)
{
    sa_nracanaa tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(parimaa_na(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, parimaa_na(tre.digest));

    sa_nracanaa tpm_rsp_extend rsp;
    u32 resp_length = parimaa_na(rsp);
    puur_naa_nka ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.hdr.errcode)
        ꦮꦭꦶ -1;

    ꦮꦭꦶ 0;
}

static puur_naa_nka tpm20_extend(sa_nracanaa tpm_log_entry *le, puur_naa_nka digest_len)
{
    sa_nracanaa tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(parimaa_na(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[parimaa_na(tmp_tre) + parimaa_na(le->pad)];
    sa_nracanaa tpm2_req_extend *tre = (sa_nracanaa tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, parimaa_na(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(parimaa_na(tmp_tre) + digest_len);

    sa_nracanaa tpm_rsp_header rsp;
    u32 resp_length = parimaa_na(rsp);
    puur_naa_nka ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.errcode)
        ꦮꦭꦶ -1;

    ꦮꦭꦶ 0;
}

static puur_naa_nka
tpm_extend(sa_nracanaa tpm_log_entry *le, puur_naa_nka digest_len)
{
    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        ꦮꦭꦶ tpm12_extend(le, digest_len);
    pak_sa TPM_VERSION_2:
        ꦮꦭꦶ tpm20_extend(le, digest_len);
    }
    ꦮꦭꦶ -1;
}

static puur_naa_nka
tpm20_stirrandom(void)
{
    sa_nracanaa tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(parimaa_na(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(parimaa_na(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    sa_nracanaa tpm_rsp_header rsp;
    u32 resp_length = parimaa_na(rsp);
    puur_naa_nka ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    sa_nracanaa tpm2_res_getrandom rsp;

    ꦪꦼꦤ꧀ (buf_len > parimaa_na(rsp.rnd.buffer))
        ꦮꦭꦶ -1;

    sa_nracanaa tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(parimaa_na(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = parimaa_na(rsp);

    puur_naa_nka ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.hdr.errcode)
        ret = -1;
    ꦭꦶꦪ
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    sa_nracanaa tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(parimaa_na(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(parimaa_na(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    sa_nracanaa tpm_rsp_header rsp;
    u32 resp_length = parimaa_na(rsp);
    puur_naa_nka ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_hierarchychangeauth(u8 auth[20])
{
    sa_nracanaa tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(parimaa_na(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(parimaa_na(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(parimaa_na(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, parimaa_na(trhca.newAuth.buffer));

    sa_nracanaa tpm_rsp_header rsp;
    u32 resp_length = parimaa_na(rsp);
    puur_naa_nka ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    ꦮꦭꦶ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

static puur_naa_nka TPM_has_physical_presence;
u8 TPM_working VARLOW;

static puur_naa_nka
tpm_is_working(void)
{
    ꦮꦭꦶ CONFIG_TCGBIOS && TPM_working;
}

static void
tpm_set_failure(void)
{
    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        bha_nga;
    pak_sa TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        bha_nga;
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
                           const var_na *event, u32 event_length,
                           const u8 *hashdata, u32 hashdata_length)
{
    ꦪꦼꦤ꧀ (!tpm_is_working())
        ꦮꦭꦶ;

    sa_nracanaa tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    puur_naa_nka digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    ꦪꦼꦤ꧀ (digest_len < 0)
        ꦮꦭꦶ;
    puur_naa_nka ret = tpm_extend(&le, digest_len);
    ꦪꦼꦤ꧀ (ret) {
        tpm_set_failure();
        ꦮꦭꦶ;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
static void
tpm_add_action(u32 pcrIndex, const var_na *string)
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
    ꦏꦁꦒꦺꦴ (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (const var_na *)evt_separator,
                                   parimaa_na(evt_separator),
                                   evt_separator,
                                   parimaa_na(evt_separator));
}

static void
tpm_smbios_measure(void)
{
    sa_nracanaa pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    void *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    ꦪꦼꦤ꧀ (!smbios_tables)
        ꦮꦭꦶ;

    sha1((const u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (const var_na *)&pcctes, parimaa_na(pcctes),
                               (u8 *)&pcctes, parimaa_na(pcctes));
}

static puur_naa_nka
tpm12_assert_physical_presence(void)
{
    puur_naa_nka ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (!ret)
        ꦮꦭꦶ 0;

    sa_nracanaa tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((var_na *)&pf, parimaa_na(pf));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;

    /* check if hardware physical presence is supported */
    ꦪꦼꦤ꧀ (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        ꦮꦭꦶ 0;
    }

    ꦪꦼꦤ꧀ (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        ꦮꦭꦶ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    ꦮꦭꦶ -1;
}

static puur_naa_nka
tpm12_startup(void)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    puur_naa_nka ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    ꦪꦼꦤ꧀ (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret && ret != TPM_BAD_LOCALITY)
        goto err_exit;

    ꦮꦭꦶ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ꦮꦭꦶ -1;
}

static puur_naa_nka
tpm20_startup(void)
{
    tpm20_set_timeouts();

    puur_naa_nka ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    ꦪꦼꦤ꧀ (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ret = tpm20_get_pcrbanks();
    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ꦮꦭꦶ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ꦮꦭꦶ -1;
}

static puur_naa_nka
tpm_startup(void)
{
    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        ꦮꦭꦶ tpm12_startup();
    pak_sa TPM_VERSION_2:
        ꦮꦭꦶ tpm20_startup();
    }
    ꦮꦭꦶ -1;
}

void
tpm_setup(void)
{
    ꦪꦼꦤ꧀ (!CONFIG_TCGBIOS)
        ꦮꦭꦶ;

    puur_naa_nka ret = tpm_tpm2_probe();
    ꦪꦼꦤ꧀ (ret) {
        ret = tpm_tcpa_probe();
        ꦪꦼꦤ꧀ (ret)
            ꦮꦭꦶ;
    }

    TPM_version = tpmhw_probe();
    ꦪꦼꦤ꧀ (TPM_version == TPM_VERSION_NONE)
        ꦮꦭꦶ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    ꦪꦼꦤ꧀ (runningOnXen())
        ꦮꦭꦶ;

    ret = tpm_startup();
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

static void
tpm20_prepboot(void)
{
    puur_naa_nka ret = tpm20_stirrandom();
    ꦪꦼꦤ꧀ (ret)
         goto err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], parimaa_na(auth));
    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ꦮꦭꦶ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

void
tpm_prepboot(void)
{
    ꦪꦼꦤ꧀ (!CONFIG_TCGBIOS)
        ꦮꦭꦶ;

    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        ꦪꦼꦤ꧀ (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        bha_nga;
    pak_sa TPM_VERSION_2:
        tpm20_prepboot();
        bha_nga;
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
    ꦪꦼꦤ꧀ (!tpm_is_working())
        ꦮꦭꦶ;

    sa_nracanaa pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = parimaa_na(u16) + parimaa_na(u16) + SHA1_BUFSIZE,
    };
    sha1((const u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (const var_na *)&pcctes, parimaa_na(pcctes),
                               (u8 *)&pcctes, parimaa_na(pcctes));
}

void
tpm_add_bcv(u32 bootdrv, const u8 *addr, u32 length)
{
    ꦪꦼꦤ꧀ (!tpm_is_working())
        ꦮꦭꦶ;

    ꦪꦼꦤ꧀ (length < 0x200)
        ꦮꦭꦶ;

    const var_na *string = "Booting BCV device 00h (Floppy)";
    ꦪꦼꦤ꧀ (bootdrv == 0x80)
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
    ꦪꦼꦤ꧀ (!tpm_is_working())
        ꦮꦭꦶ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    const var_na *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

void
tpm_add_cdrom_catalog(const u8 *addr, u32 length)
{
    ꦪꦼꦤ꧀ (!tpm_is_working())
        ꦮꦭꦶ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    const var_na *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

void
tpm_s3_resume(void)
{
    ꦪꦼꦤ꧀ (!tpm_is_working())
        ꦮꦭꦶ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    puur_naa_nka ret = -1;

    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        bha_nga;
    pak_sa TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        ꦪꦼꦤ꧀ (ret)
            goto err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        bha_nga;
    }

    ꦪꦼꦤ꧀ (ret)
        goto err_exit;

    ꦮꦭꦶ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

static anta_hstha void *input_buf32(sa_nracanaa bregs *regs)
{
    ꦮꦭꦶ MAKE_FLATPTR(regs->es, regs->di);
}

static anta_hstha void *output_buf32(sa_nracanaa bregs *regs)
{
    ꦮꦭꦶ MAKE_FLATPTR(regs->ds, regs->si);
}

static u32
hash_log_extend(sa_nracanaa pcpes *pcpes, const void *hashdata, u32 hashdata_length
                , void *event, puur_naa_nka extend)
{
    ꦪꦼꦤ꧀ (pcpes->pcrindex >= 24)
        ꦮꦭꦶ TCG_INVALID_INPUT_PARA;
    ꦪꦼꦤ꧀ (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    sa_nracanaa tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    puur_naa_nka digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    ꦪꦼꦤ꧀ (digest_len < 0)
        ꦮꦭꦶ TCG_GENERAL_ERROR;
    ꦪꦼꦤ꧀ (extend) {
        puur_naa_nka ret = tpm_extend(&le, digest_len);
        ꦪꦼꦤ꧀ (ret)
            ꦮꦭꦶ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    puur_naa_nka ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ TCG_PC_LOGOVERFLOW;
    ꦮꦭꦶ 0;
}

static u32
hash_log_extend_event_int(const sa_nracanaa hleei_short *hleei_s,
                          sa_nracanaa hleeo *hleeo)
{
    u32 rc = 0;
    sa_nracanaa hleei_long *hleei_l = (sa_nracanaa hleei_long *)hleei_s;
    const void *logdataptr;
    u32 logdatalen;
    sa_nracanaa pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    switch (hleei_s->ipblength) {
    pak_sa parimaa_na(sa_nracanaa hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    bha_nga;

    pak_sa parimaa_na(sa_nracanaa hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    bha_nga;

    default:
        /* bad input block */
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }

    pcpes = (sa_nracanaa pcpes *)logdataptr;

    ꦪꦼꦤ꧀ (pcpes->pcrindex != pcrindex
        || logdatalen != parimaa_na(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    ꦪꦼꦤ꧀ (rc)
        goto err_exit;

    hleeo->opblength = parimaa_na(sa_nracanaa hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_count;
    memcpy(hleeo->digest, pcpes->digest, parimaa_na(hleeo->digest));

err_exit:
    ꦪꦼꦤ꧀ (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    ꦮꦭꦶ rc;
}

static u32
pass_through_to_tpm_int(sa_nracanaa pttti *pttti, sa_nracanaa pttto *pttto)
{
    u32 rc = 0;
    sa_nracanaa tpm_req_header *trh = (void*)pttti->tpmopin;

    ꦪꦼꦤ꧀ (pttti->ipblength < parimaa_na(sa_nracanaa pttti) + parimaa_na(*trh)
        || pttti->ipblength != parimaa_na(sa_nracanaa pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < parimaa_na(sa_nracanaa pttto)) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        ꦪꦼꦤ꧀ (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_INPUT_PARA;
            goto err_exit;
        }
        bha_nga;
    pak_sa TPM_VERSION_2:
        ꦪꦼꦤ꧀ (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_INPUT_PARA;
            goto err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(sa_nracanaa pttto, tpmopout);
    puur_naa_nka ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    ꦪꦼꦤ꧀ (ret) {
        rc = TCG_FATAL_COM_ERROR;
        goto err_exit;
    }

    pttto->opblength = offsetof(sa_nracanaa pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    ꦪꦼꦤ꧀ (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    ꦮꦭꦶ rc;
}

static u32
shutdown_preboot_interface(void)
{
    TPM_interface_shutdown = 1;
    ꦮꦭꦶ 0;
}

static u32
hash_log_event_int(const sa_nracanaa hlei *hlei, sa_nracanaa hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    sa_nracanaa pcpes *pcpes;

    size = hlei->ipblength;
    ꦪꦼꦤ꧀ (size != parimaa_na(*hlei)) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }

    pcpes = (sa_nracanaa pcpes *)hlei->logdataptr;

    ꦪꦼꦤ꧀ (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != parimaa_na(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_INPUT_PARA;
        goto err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    ꦪꦼꦤ꧀ (rc)
        goto err_exit;

    /* updating the log was fine */
    hleo->opblength = parimaa_na(sa_nracanaa hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_count;

err_exit:
    ꦪꦼꦤ꧀ (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    ꦮꦭꦶ rc;
}

static u32
hash_all_int(const sa_nracanaa hai *hai, u8 *hash)
{
    ꦪꦼꦤ꧀ (hai->ipblength != parimaa_na(sa_nracanaa hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        ꦮꦭꦶ TCG_INVALID_INPUT_PARA;

    sha1((const u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    ꦮꦭꦶ 0;
}

static u32
tss_int(sa_nracanaa ti *ti, sa_nracanaa to *to)
{
    to->opblength = parimaa_na(sa_nracanaa to);
    to->reserved  = 0;

    ꦮꦭꦶ TCG_PC_UNSUPPORTED;
}

static u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    sa_nracanaa pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = parimaa_na(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    ꦪꦼꦤ꧀ (rc)
        ꦮꦭꦶ rc;

    *edx_ptr = tpm_state.entry_count;
    ꦮꦭꦶ 0;
}

void VISIBLE32FLAT
tpm_interrupt_handler32(sa_nracanaa bregs *regs)
{
    ꦪꦼꦤ꧀ (!CONFIG_TCGBIOS)
        ꦮꦭꦶ;

    set_cf(regs, 0);

    ꦪꦼꦤ꧀ (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        ꦮꦭꦶ;
    }

    switch ((ga_nanaa irq_ids)regs->al) {
    pak_sa TCG_StatusCheck:
        ꦪꦼꦤ꧀ (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } ꦭꦶꦪ {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        bha_nga;

    pak_sa TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (sa_nracanaa hleei_short *)input_buf32(regs),
                  (sa_nracanaa hleeo *)output_buf32(regs));
        bha_nga;

    pak_sa TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((sa_nracanaa pttti *)input_buf32(regs),
                                    (sa_nracanaa pttto *)output_buf32(regs));
        bha_nga;

    pak_sa TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        bha_nga;

    pak_sa TCG_HashLogEvent:
        regs->eax = hash_log_event_int((sa_nracanaa hlei*)input_buf32(regs),
                                       (sa_nracanaa hleo*)output_buf32(regs));
        bha_nga;

    pak_sa TCG_HashAll:
        regs->eax =
            hash_all_int((sa_nracanaa hai*)input_buf32(regs),
                          (u8 *)output_buf32(regs));
        bha_nga;

    pak_sa TCG_TSS:
        regs->eax = tss_int((sa_nracanaa ti*)input_buf32(regs),
                            (sa_nracanaa to*)output_buf32(regs));
        bha_nga;

    pak_sa TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)input_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        bha_nga;

    default:
        set_cf(regs, 1);
    }

    ꦮꦭꦶ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

typedef u8 tpm_ppi_code;

static puur_naa_nka
tpm12_read_has_owner(puur_naa_nka *has_owner)
{
    sa_nracanaa tpm_res_getcap_ownerauth oauth;
    puur_naa_nka ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, parimaa_na(oauth));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;

    *has_owner = oauth.flag;

    ꦮꦭꦶ 0;
}

static puur_naa_nka
tpm12_enable_tpm(puur_naa_nka enable, puur_naa_nka verbose)
{
    sa_nracanaa tpm_permanent_flags pf;
    puur_naa_nka ret = tpm12_read_permanent_flags((var_na *)&pf, parimaa_na(pf));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;

    ꦪꦼꦤ꧀ (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        ꦮꦭꦶ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret) {
        ꦪꦼꦤ꧀ (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        ꦭꦶꦪ
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm12_activate_tpm(puur_naa_nka activate, puur_naa_nka allow_reset, puur_naa_nka verbose)
{
    sa_nracanaa tpm_permanent_flags pf;
    puur_naa_nka ret = tpm12_read_permanent_flags((var_na *)&pf, parimaa_na(pf));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;

    ꦪꦼꦤ꧀ (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        ꦮꦭꦶ 0;

    ꦪꦼꦤ꧀ (pf.flags[PERM_FLAG_IDX_DISABLE])
        ꦮꦭꦶ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ ret;

    ꦪꦼꦤ꧀ (activate && allow_reset) {
        ꦪꦼꦤ꧀ (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    ꦮꦭꦶ 0;
}

static puur_naa_nka
tpm12_enable_activate(puur_naa_nka allow_reset, puur_naa_nka verbose)
{
    puur_naa_nka ret = tpm12_enable_tpm(1, verbose);
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ ret;

    ꦮꦭꦶ tpm12_activate_tpm(1, allow_reset, verbose);
}

static puur_naa_nka
tpm12_force_clear(puur_naa_nka enable_activate_before, puur_naa_nka enable_activate_after,
                  puur_naa_nka verbose)
{
    puur_naa_nka has_owner;
    puur_naa_nka ret = tpm12_read_has_owner(&has_owner);
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;
    ꦪꦼꦤ꧀ (!has_owner) {
        ꦪꦼꦤ꧀ (verbose)
            printf("TPM does not have an owner.\n");
        ꦮꦭꦶ 0;
    }

    ꦪꦼꦤ꧀ (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        ꦪꦼꦤ꧀ (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            ꦮꦭꦶ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ ret;

    ꦪꦼꦤ꧀ (!enable_activate_after) {
        ꦪꦼꦤ꧀ (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        ꦮꦭꦶ 0;
    }

    ꦮꦭꦶ tpm12_enable_activate(1, verbose);
}

static puur_naa_nka
tpm12_set_owner_install(puur_naa_nka allow, puur_naa_nka verbose)
{
    puur_naa_nka has_owner;
    puur_naa_nka ret = tpm12_read_has_owner(&has_owner);
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;
    ꦪꦼꦤ꧀ (has_owner) {
        ꦪꦼꦤ꧀ (verbose)
            printf("Must first remove owner.\n");
        ꦮꦭꦶ 0;
    }

    sa_nracanaa tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((var_na *)&pf, parimaa_na(pf));
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ -1;

    ꦪꦼꦤ꧀ (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        ꦪꦼꦤ꧀ (verbose)
            printf("TPM must first be enable.\n");
        ꦮꦭꦶ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret)
        ꦮꦭꦶ ret;

    ꦪꦼꦤ꧀ (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    ꦮꦭꦶ 0;
}

static puur_naa_nka
tpm12_process_cfg(tpm_ppi_code msgCode, puur_naa_nka verbose)
{
    puur_naa_nka ret = 0;

    switch (msgCode) {
        pak_sa TPM_PPI_OP_NOOP: /* no-op */
            bha_nga;

        pak_sa TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            bha_nga;

        pak_sa TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            bha_nga;

        pak_sa TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            bha_nga;

        pak_sa TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            bha_nga;

        pak_sa TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            bha_nga;

        pak_sa TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            bha_nga;

        pak_sa TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            bha_nga;

        default:
            bha_nga;
    }

    ꦪꦼꦤ꧀ (ret)
        printf("Op %d: An error occurred: 0x%x\n", msgCode, ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_clearcontrol(u8 disable, puur_naa_nka verbose)
{
    sa_nracanaa tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(parimaa_na(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(parimaa_na(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    sa_nracanaa tpm_rsp_header rsp;
    u32 resp_length = parimaa_na(rsp);
    puur_naa_nka ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_clear(void)
{
    sa_nracanaa tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(parimaa_na(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(parimaa_na(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    sa_nracanaa tpm_rsp_header rsp;
    u32 resp_length = parimaa_na(rsp);
    puur_naa_nka ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ꦪꦼꦤ꧀ (ret || resp_length != parimaa_na(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm20_process_cfg(tpm_ppi_code msgCode, puur_naa_nka verbose)
{
    puur_naa_nka ret = 0;

    switch (msgCode) {
        pak_sa TPM_PPI_OP_NOOP: /* no-op */
            bha_nga;

        pak_sa TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            ꦪꦼꦤ꧀ (!ret)
                 ret = tpm20_clear();
            bha_nga;
    }

    ꦪꦼꦤ꧀ (ret)
        printf("Op %d: An error occurred: 0x%x\n", msgCode, ret);

    ꦮꦭꦶ ret;
}

static puur_naa_nka
tpm12_get_tpm_state(void)
{
    puur_naa_nka state = 0;
    sa_nracanaa tpm_permanent_flags pf;
    puur_naa_nka has_owner;

    ꦪꦼꦤ꧀ (tpm12_read_permanent_flags((var_na *)&pf, parimaa_na(pf)) ||
        tpm12_read_has_owner(&has_owner))
        ꦮꦭꦶ ~0;

    ꦪꦼꦤ꧀ (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    ꦪꦼꦤ꧀ (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    ꦪꦼꦤ꧀ (has_owner) {
        state |= TPM_STATE_OWNED;
    } ꦭꦶꦪ {
        ꦪꦼꦤ꧀ (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    ꦮꦭꦶ state;
}

static void
tpm12_show_tpm_menu(puur_naa_nka state, puur_naa_nka next_scancodes[7])
{
    puur_naa_nka i = 0;

    printf("\nThe current state of the TPM is:\n");

    ꦪꦼꦤ꧀ (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    ꦭꦶꦪ
        printf("  Disabled");

    ꦪꦼꦤ꧀ (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    ꦭꦶꦪ
        printf(" and deactivated\n");

    ꦪꦼꦤ꧀ (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    ꦭꦶꦪ {
        printf("  Ownership has not been taken\n");
        ꦪꦼꦤ꧀ (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        ꦭꦶꦪ
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    ꦪꦼꦤ꧀ ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    ꦪꦼꦤ꧀ (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        ꦪꦼꦤ꧀ (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            ꦪꦼꦤ꧀ (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } ꦭꦶꦪ {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } ꦭꦶꦪ {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } ꦭꦶꦪ {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    ꦪꦼꦤ꧀ (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

static void
tpm12_menu(void)
{
    puur_naa_nka scancode, next_scancodes[7];
    tpm_ppi_code msgCode;
    puur_naa_nka state = 0, i;
    puur_naa_nka waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    ꦏꦁꦒꦺꦴ (;;) {
        ꦪꦼꦤ꧀ ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } ꦭꦶꦪ {
            printf("TPM is not working correctly.\n");
            ꦮꦭꦶ;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        msgCode = TPM_PPI_OP_NOOP;

        waitkey = 1;

        ꦱꦼꦢꦼꦭ (waitkey) {
            ꦱꦼꦢꦼꦭ ((scancode = get_keystroke(1000)) == ~0)
                ;

            switch (scancode) {
            pak_sa 1:
                // ESC
                reset();
                bha_nga;
            pak_sa 18: /* e. enable */
                msgCode = TPM_PPI_OP_ENABLE;
                bha_nga;
            pak_sa 32: /* d. disable */
                msgCode = TPM_PPI_OP_DISABLE;
                bha_nga;
            pak_sa 30: /* a. activate */
                msgCode = TPM_PPI_OP_ACTIVATE;
                bha_nga;
            pak_sa 47: /* v. deactivate */
                msgCode = TPM_PPI_OP_DEACTIVATE;
                bha_nga;
            pak_sa 46: /* c. clear owner */
                msgCode = TPM_PPI_OP_CLEAR;
                bha_nga;
            pak_sa 25: /* p. prevent ownerinstall */
                msgCode = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                bha_nga;
            pak_sa 31: /* s. allow ownerinstall */
                msgCode = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                bha_nga;
            default:
                continue;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            ꦏꦁꦒꦺꦴ (i = 0; i < parimaa_na(next_scancodes); i++) {
                ꦪꦼꦤ꧀ (next_scancodes[i] == 0)
                    bha_nga;

                ꦪꦼꦤ꧀ (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(msgCode, 1);
                    waitkey = 0;
                    bha_nga;
                }
            }
        }
    }
}

static puur_naa_nka
tpm20_menu_change_active_pcrbanks(void)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    ꦱꦼꦢꦼꦭ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        ꦱꦼꦢꦼꦭ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            const var_na *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            ꦪꦼꦤ꧀ (hashname) {
                printf("  %d: %s", i, hashname);
                ꦪꦼꦤ꧀ (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        ꦪꦼꦤ꧀ (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        puur_naa_nka show = 0;
        ꦱꦼꦢꦼꦭ (!show) {
            puur_naa_nka scancode = get_keystroke(1000);

            switch (scancode) {
            pak_sa ~0:
                continue;
            pak_sa 1: /* ESC */
                printf("\n");
                ꦮꦭꦶ -1;
            pak_sa 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                ꦪꦼꦤ꧀ (flagnum > i)
                    continue;
                ꦪꦼꦤ꧀ (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                bha_nga;
            pak_sa 30: /* a */
                ꦪꦼꦤ꧀ (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

static void
tpm20_menu(void)
{
    puur_naa_nka scan_code;
    tpm_ppi_code msgCode;

    ꦏꦁꦒꦺꦴ (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        msgCode = TPM_PPI_OP_NOOP;

        ꦱꦼꦢꦼꦭ ((scan_code = get_keystroke(1000)) == ~0)
            ;

        switch (scan_code) {
        pak_sa 1:
            // ESC
            reset();
            bha_nga;
        pak_sa 2:
            msgCode = TPM_PPI_OP_CLEAR;
            bha_nga;
        pak_sa 3:
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
    ꦪꦼꦤ꧀ (!CONFIG_TCGBIOS)
        ꦮꦭꦶ;

    ꦱꦼꦢꦼꦭ (get_keystroke(0) >= 0)
        ;
    wait_threads();

    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        tpm12_menu();
        bha_nga;
    pak_sa TPM_VERSION_2:
        tpm20_menu();
        bha_nga;
    }
}

puur_naa_nka
tpm_can_show_menu(void)
{
    switch (TPM_version) {
    pak_sa TPM_VERSION_1_2:
        ꦮꦭꦶ tpm_is_working() && TPM_has_physical_presence;
    pak_sa TPM_VERSION_2:
        ꦮꦭꦶ tpm_is_working();
    }
    ꦮꦭꦶ 0;
}
