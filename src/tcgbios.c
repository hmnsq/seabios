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

#شامل "bregs.h" // struct bregs
#شامل "byteorder.h" // cpu_to_*
#شامل "config.h" // CONFIG_TCGBIOS
#شامل "farptr.h" // MAKE_FLATPTR
#شامل "fw/paravirt.h" // runningOnXen
#شامل "hw/tpm_drivers.h" // tpm_drivers[]
#شامل "output.h" // dprintf
#شامل "sha.h" // sha1, sha256, ...
#شامل "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#شامل "std/smbios.h" // struct smbios_21_entry_point
#شامل "std/tcg.h" // TCG_PC_LOGOVERFLOW
#شامل "string.h" // checksum
#شامل "tcgbios.h"// tpm_*, prototypes
#شامل "util.h" // printf, get_keystroke
#شامل "stacks.h" // wait_threads, reset
#شامل "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

ساختار {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_شمار;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

ایستا صحیح tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    اگر (!log_area_start_address || !log_area_minimum_length)
        بازگشت -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = پوچ;
    tpm_state.entry_شمار = 0;
    بازگشت 0;
}

ایستا صحیح
tpm_tcpa_probe(تهی)
{
    ساختار tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    اگر (!tcpa)
        بازگشت -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    بازگشت tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

ایستا صحیح
tpm_tpm2_probe(تهی)
{
    ساختار tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    اگر (!tpm2)
        بازگشت -1;

    اگر (tpm2->length < 76)
        بازگشت -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    بازگشت tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
ایستا صحیح
tpm_log_event(ساختار tpm_log_header *entry, صحیح digest_len
              , ثابت تهی *event, صحیح event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    اگر (tpm_state.log_area_next_entry == پوچ)
        بازگشت -1;

    u32 size = (اندازه(*entry) + digest_len
                + اندازه(ساختار tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    اگر (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        بازگشت -1;
    }

    تهی *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, اندازه(*entry) + digest_len);
    ساختار tpm_log_trailer *t = dest + اندازه(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_شمار++;

    بازگشت 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ایستا TPMVersion TPM_version;
ایستا u32 tpm20_pcr_selection_size;
ایستا ساختار tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
ساختار tpm_log_entry {
    ساختار tpm_log_header hdr;
    u8 pad[اندازه(ساختار tpm2_digest_مقادیر)
           + 8 * اندازه(ساختار tpm2_digest_مقدار)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ایستا ثابت ساختار hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ثابت نویسه *name;
    تهی (*hashfunc)(ثابت u8 *data, u32 length, u8 *hash);
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

ایستا صحیح
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    برای (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg == hashAlg)
            بازگشت hash_parameters[i].hash_buffersize;
    }
    بازگشت -1;
}

ایستا u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    برای (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg == hashAlg)
            بازگشت hash_parameters[i].hashalg_flag;
    }
    بازگشت 0;
}

ایستا u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    برای (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg_flag == hashalg_flag)
            بازگشت hash_parameters[i].hashalg;
    }
    بازگشت 0;
}

ایستا ثابت نویسه *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    برای (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg_flag == hashalg_flag)
            بازگشت hash_parameters[i].name;
    }
    بازگشت پوچ;
}

ایستا تهی tpm2_hash_data(u16 hashAlg, ثابت u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    برای (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg == hashAlg) {
            اگر (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } وگرنه {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ایستا صحیح
tpm20_write_EfiSpecIdEventStruct(تهی)
{
    اگر (!tpm20_pcr_selection)
        بازگشت -1;

    ساختار {
        ساختار TCG_EfiSpecIdEventStruct hdr;
        u8 pad[اندازه(ساختار tpm_log_entry) + اندازه(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    ساختار tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    تهی *nsel, *end = (تهی*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 شمار, رقم_Algs = 0;
    برای (شمار = 0; شمار < be32_to_cpu(tpm20_pcr_selection->شمار); شمار++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (تهی*)sel + اندازه(*sel) + sizeOfSelect;
        اگر (nsel > end)
            بشکن;

        اگر (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ادامه;
        }

        صحیح hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        اگر (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            بازگشت -1;
        }

        صحیح event_size = offsetof(ساختار TCG_EfiSpecIdEventStruct
                                  , digestSizes[شمار+1]);
        اگر (event_size > اندازه(event) - اندازه(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            بازگشت -1;
        }

        event.hdr.digestSizes[رقم_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[رقم_Algs].digestSize = hsize;
        رقم_Algs++;

        sel = nsel;
    }

    اگر (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        بازگشت -1;
    }

    event.hdr.عدد_Of_Algorithms = رقم_Algs;
    صحیح event_size = offsetof(ساختار TCG_EfiSpecIdEventStruct
                              , digestSizes[رقم_Algs]);
    u8 *vendorInfoSize = (تهی*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += اندازه(*vendorInfoSize);

    ساختار tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    بازگشت tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
ایستا صحیح
tpm20_build_digest(ساختار tpm_log_entry *le,
                   ثابت u8 *hashdata, u32 hashdata_len, صحیح bigEndian)
{
    اگر (!tpm20_pcr_selection)
        بازگشت -1;

    ساختار tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    تهی *nsel, *end = (تهی*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    تهی *dest = le->hdr.digest + اندازه(ساختار tpm2_digest_مقادیر);

    u32 شمار, رقم_Algs = 0;
    برای (شمار = 0; شمار < be32_to_cpu(tpm20_pcr_selection->شمار); شمار++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (تهی*)sel + اندازه(*sel) + sizeOfSelect;
        اگر (nsel > end)
            بشکن;

        /* PCR 0-7 unused? -- skip */
        اگر (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ادامه;
        }

        صحیح hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        اگر (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            بازگشت -1;
        }

        /* buffer size sanity check before writing */
        ساختار tpm2_digest_مقدار *v = dest;
        اگر (dest + اندازه(*v) + hsize > (تهی*)le + اندازه(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            بازگشت -1;
        }

        اگر (bigEndian)
            v->hashAlg = sel->hashAlg;
        وگرنه
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += اندازه(*v) + hsize;
        sel = nsel;

        رقم_Algs++;
    }

    اگر (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        بازگشت -1;
    }

    ساختار tpm2_digest_مقادیر *v = (تهی*)le->hdr.digest;
    اگر (bigEndian)
        v->شمار = cpu_to_be32(رقم_Algs);
    وگرنه
        v->شمار = رقم_Algs;

    بازگشت dest - (تهی*)le->hdr.digest;
}

ایستا صحیح
tpm12_build_digest(ساختار tpm_log_entry *le,
                   ثابت u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    بازگشت SHA1_BUFSIZE;
}

ایستا صحیح
tpm12_build_digest_direct(ساختار tpm_log_entry *le, ثابت u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    بازگشت SHA1_BUFSIZE;
}

ایستا صحیح
tpm_build_digest(ساختار tpm_log_entry *le, ثابت u8 *hashdata, u32 hashdata_len
                 , صحیح bigEndian)
{
    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        بازگشت tpm12_build_digest(le, hashdata, hashdata_len);
    حالت TPM_VERSION_2:
        بازگشت tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    بازگشت -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ایستا صحیح
tpm_simple_cmd(u8 locty, u32 ordinal
               , صحیح param_size, u16 param, شمارش tpmDurationType to_t)
{
    ساختار {
        ساختار tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(اندازه(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        بشکن;
    حالت TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        بشکن;
    }

    u8 obuffer[64];
    ساختار tpm_rsp_header *trsh = (تهی*)obuffer;
    u32 obuffer_len = اندازه(obuffer);
    memset(obuffer, 0x0, اندازه(obuffer));

    صحیح ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    بازگشت ret;
}

ایستا صحیح
tpm20_getcapability(u32 capability, u32 property, u32 شمار,
                    ساختار tpm_rsp_header *rsp, u32 rsize)
{
    ساختار tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(اندازه(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(شمار),
    };

    u32 resp_size = rsize;
    صحیح ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    بازگشت ret;
}

ایستا صحیح
tpm20_get_pcrbanks(تهی)
{
    u8 buffer[128];
    ساختار tpm2_res_getcapability *trg =
      (ساختار tpm2_res_getcapability *)&buffer;

    صحیح ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  اندازه(buffer));
    اگر (ret)
        بازگشت ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    اگر (resplen <= offsetof(ساختار tpm2_res_getcapability, data))
        بازگشت -1;

    u32 size = resplen - offsetof(ساختار tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    اگر (size < offsetof(ساختار tpml_pcr_selection, selections) +
               offsetof(ساختار tpms_pcr_selection, pcrSelect))
        بازگشت -1;

    tpm20_pcr_selection = malloc_بالا(size);
    اگر (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } وگرنه {
        warn_noalloc();
        ret = -1;
    }

    بازگشت ret;
}

ایستا صحیح
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    اگر (!tpm20_pcr_selection)
        بازگشت -1;

    ساختار tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    تهی *end = (تهی*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    وقتی (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        تهی *nsel = (تهی*)sel + اندازه(*sel) + sizeOfSelect;
        اگر (nsel > end)
            بازگشت 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        برای (i = 0; i < sizeOfSelect; i++) {
            اگر (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                بشکن;
            }
        }

        sel = nsel;
    }
}

ایستا صحیح
tpm20_set_pcrbanks(u32 active_banks)
{
    ساختار tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(اندازه(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ساختار tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    صحیح i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    وقتی (hashalg_flag) {
        اگر ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            اگر (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                اگر (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.شمار = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * اندازه(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(ساختار tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * اندازه(tps[0]));

    ساختار tpm_rsp_header rsp;
    u32 resp_length = اندازه(rsp);

    صحیح ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    بازگشت ret;
}

ایستا صحیح tpm20_activate_pcrbanks(u32 active_banks)
{
    صحیح ret = tpm20_set_pcrbanks(active_banks);
    اگر (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    اگر (!ret)
        reset();
    بازگشت ret;
}

ایستا صحیح
tpm12_get_capability(u32 cap, u32 subcap, ساختار tpm_rsp_header *rsp, u32 rsize)
{
    ساختار tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(اندازه(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(اندازه(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    صحیح ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    بازگشت ret;
}

ایستا صحیح
tpm12_read_permanent_flags(نویسه *buf, صحیح buf_len)
{
    memset(buf, 0, buf_len);

    ساختار tpm_res_getcap_perm_flags pf;
    صحیح ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, اندازه(pf));
    اگر (ret)
        بازگشت -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    بازگشت 0;
}

ایستا صحیح
tpm12_determine_timeouts(تهی)
{
    ساختار tpm_res_getcap_timeouts timeouts;
    صحیح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, اندازه(timeouts));
    اگر (ret)
        بازگشت ret;

    ساختار tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, اندازه(durations));
    اگر (ret)
        بازگشت ret;

    صحیح i;
    برای (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    برای (i = 0; i < 4; i++)
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

    بازگشت 0;
}

ایستا تهی
tpm20_set_timeouts(تهی)
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

ایستا صحیح
tpm12_extend(ساختار tpm_log_entry *le, صحیح digest_len)
{
    ساختار tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(اندازه(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, اندازه(tre.digest));

    ساختار tpm_rsp_extend rsp;
    u32 resp_length = اندازه(rsp);
    صحیح ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_length != اندازه(rsp) || rsp.hdr.errcode)
        بازگشت -1;

    بازگشت 0;
}

ایستا صحیح tpm20_extend(ساختار tpm_log_entry *le, صحیح digest_len)
{
    ساختار tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(اندازه(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[اندازه(tmp_tre) + اندازه(le->pad)];
    ساختار tpm2_req_extend *tre = (ساختار tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, اندازه(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(اندازه(tmp_tre) + digest_len);

    ساختار tpm_rsp_header rsp;
    u32 resp_length = اندازه(rsp);
    صحیح ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_length != اندازه(rsp) || rsp.errcode)
        بازگشت -1;

    بازگشت 0;
}

ایستا صحیح
tpm_extend(ساختار tpm_log_entry *le, صحیح digest_len)
{
    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        بازگشت tpm12_extend(le, digest_len);
    حالت TPM_VERSION_2:
        بازگشت tpm20_extend(le, digest_len);
    }
    بازگشت -1;
}

ایستا صحیح
tpm20_stirrandom(تهی)
{
    ساختار tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(اندازه(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(اندازه(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    ساختار tpm_rsp_header rsp;
    u32 resp_length = اندازه(rsp);
    صحیح ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_length != اندازه(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    بازگشت ret;
}

ایستا صحیح
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    ساختار tpm2_res_getrandom rsp;

    اگر (buf_len > اندازه(rsp.rnd.buffer))
        بازگشت -1;

    ساختار tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(اندازه(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = اندازه(rsp);

    صحیح ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_length != اندازه(rsp) || rsp.hdr.errcode)
        ret = -1;
    وگرنه
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    بازگشت ret;
}

ایستا صحیح
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    ساختار tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(اندازه(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(اندازه(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    ساختار tpm_rsp_header rsp;
    u32 resp_length = اندازه(rsp);
    صحیح ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_length != اندازه(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    بازگشت ret;
}

ایستا صحیح
tpm20_hierarchychangeauth(u8 auth[20])
{
    ساختار tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(اندازه(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(اندازه(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(اندازه(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, اندازه(trhca.newAuth.buffer));

    ساختار tpm_rsp_header rsp;
    u32 resp_length = اندازه(rsp);
    صحیح ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_length != اندازه(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    بازگشت ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ایستا صحیح TPM_has_physical_presence;
u8 TPM_working VARLOW;

ایستا صحیح
tpm_is_working(تهی)
{
    بازگشت CONFIG_TCGBIOS && TPM_working;
}

ایستا تهی
tpm_set_failure(تهی)
{
    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        بشکن;
    حالت TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        بشکن;
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
ایستا تهی
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           ثابت نویسه *event, u32 event_length,
                           ثابت u8 *hashdata, u32 hashdata_length)
{
    اگر (!tpm_is_working())
        بازگشت;

    ساختار tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    صحیح digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    اگر (digest_len < 0)
        بازگشت;
    صحیح ret = tpm_extend(&le, digest_len);
    اگر (ret) {
        tpm_set_failure();
        بازگشت;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
ایستا تهی
tpm_add_action(u32 pcrIndex, ثابت نویسه *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ایستا تهی
tpm_add_event_separators(تهی)
{
    ایستا ثابت u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    برای (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (ثابت نویسه *)evt_separator,
                                   اندازه(evt_separator),
                                   evt_separator,
                                   اندازه(evt_separator));
}

ایستا تهی
tpm_smbios_measure(تهی)
{
    ساختار pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    تهی *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    اگر (!smbios_tables)
        بازگشت;

    sha1((ثابت u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (ثابت نویسه *)&pcctes, اندازه(pcctes),
                               (u8 *)&pcctes, اندازه(pcctes));
}

ایستا صحیح
tpm12_assert_physical_presence(تهی)
{
    صحیح ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    اگر (!ret)
        بازگشت 0;

    ساختار tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((نویسه *)&pf, اندازه(pf));
    اگر (ret)
        بازگشت -1;

    /* check if hardware physical presence is supported */
    اگر (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        بازگشت 0;
    }

    اگر (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        بازگشت tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    بازگشت -1;
}

ایستا صحیح
tpm12_startup(تهی)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    صحیح ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    اگر (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    اگر (ret)
        برو err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    اگر (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    اگر (ret)
        برو err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    اگر (ret)
        برو err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    اگر (ret && ret != TPM_BAD_LOCALITY)
        برو err_exit;

    بازگشت 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    بازگشت -1;
}

ایستا صحیح
tpm20_startup(تهی)
{
    tpm20_set_timeouts();

    صحیح ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    اگر (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    اگر (ret)
        برو err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    اگر (ret)
        برو err_exit;

    ret = tpm20_get_pcrbanks();
    اگر (ret)
        برو err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    اگر (ret)
        برو err_exit;

    بازگشت 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    بازگشت -1;
}

ایستا صحیح
tpm_startup(تهی)
{
    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        بازگشت tpm12_startup();
    حالت TPM_VERSION_2:
        بازگشت tpm20_startup();
    }
    بازگشت -1;
}

تهی
tpm_setup(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت;

    صحیح ret = tpm_tpm2_probe();
    اگر (ret) {
        ret = tpm_tcpa_probe();
        اگر (ret)
            بازگشت;
    }

    TPM_version = tpmhw_probe();
    اگر (TPM_version == TPM_VERSION_NONE)
        بازگشت;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    اگر (runningOnXen())
        بازگشت;

    ret = tpm_startup();
    اگر (ret)
        بازگشت;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

ایستا تهی
tpm20_prepboot(تهی)
{
    صحیح ret = tpm20_stirrandom();
    اگر (ret)
         برو err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], اندازه(auth));
    اگر (ret)
        برو err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    اگر (ret)
        برو err_exit;

    بازگشت;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

تهی
tpm_prepboot(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت;

    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        اگر (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        بشکن;
    حالت TPM_VERSION_2:
        tpm20_prepboot();
        بشکن;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
تهی
tpm_option_rom(ثابت تهی *addr, u32 len)
{
    اگر (!tpm_is_working())
        بازگشت;

    ساختار pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = اندازه(u16) + اندازه(u16) + SHA1_BUFSIZE,
    };
    sha1((ثابت u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (ثابت نویسه *)&pcctes, اندازه(pcctes),
                               (u8 *)&pcctes, اندازه(pcctes));
}

تهی
tpm_add_bcv(u32 bootdrv, ثابت u8 *addr, u32 length)
{
    اگر (!tpm_is_working())
        بازگشت;

    اگر (length < 0x200)
        بازگشت;

    ثابت نویسه *string = "Booting BCV device 00h (Floppy)";
    اگر (bootdrv == 0x80)
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

تهی
tpm_add_cdrom(u32 bootdrv, ثابت u8 *addr, u32 length)
{
    اگر (!tpm_is_working())
        بازگشت;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت نویسه *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

تهی
tpm_add_cdrom_catalog(ثابت u8 *addr, u32 length)
{
    اگر (!tpm_is_working())
        بازگشت;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت نویسه *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

تهی
tpm_s3_resume(تهی)
{
    اگر (!tpm_is_working())
        بازگشت;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    صحیح ret = -1;

    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        بشکن;
    حالت TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        اگر (ret)
            برو err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        بشکن;
    }

    اگر (ret)
        برو err_exit;

    بازگشت;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ایستا درونخطی تهی *درون_داد_buf32(ساختار bregs *regs)
{
    بازگشت MAKE_FLATPTR(regs->es, regs->di);
}

ایستا درونخطی تهی *output_buf32(ساختار bregs *regs)
{
    بازگشت MAKE_FLATPTR(regs->ds, regs->si);
}

ایستا u32
hash_log_extend(ساختار pcpes *pcpes, ثابت تهی *hashdata, u32 hashdata_length
                , تهی *event, صحیح extend)
{
    اگر (pcpes->pcrindex >= 24)
        بازگشت TCG_INVALID_درون_داد_PARA;
    اگر (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    ساختار tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    صحیح digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    اگر (digest_len < 0)
        بازگشت TCG_GENERAL_ERROR;
    اگر (extend) {
        صحیح ret = tpm_extend(&le, digest_len);
        اگر (ret)
            بازگشت TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    صحیح ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    اگر (ret)
        بازگشت TCG_PC_LOGOVERFLOW;
    بازگشت 0;
}

ایستا u32
hash_log_extend_event_int(ثابت ساختار hleei_short *hleei_s,
                          ساختار hleeo *hleeo)
{
    u32 rc = 0;
    ساختار hleei_long *hleei_l = (ساختار hleei_long *)hleei_s;
    ثابت تهی *logdataptr;
    u32 logdatalen;
    ساختار pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    انتخاب (hleei_s->ipblength) {
    حالت اندازه(ساختار hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    بشکن;

    حالت اندازه(ساختار hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    بشکن;

    پیشفرض:
        /* bad input block */
        rc = TCG_INVALID_درون_داد_PARA;
        برو err_exit;
    }

    pcpes = (ساختار pcpes *)logdataptr;

    اگر (pcpes->pcrindex != pcrindex
        || logdatalen != اندازه(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_درون_داد_PARA;
        برو err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    اگر (rc)
        برو err_exit;

    hleeo->opblength = اندازه(ساختار hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_شمار;
    memcpy(hleeo->digest, pcpes->digest, اندازه(hleeo->digest));

err_exit:
    اگر (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    بازگشت rc;
}

ایستا u32
pass_through_to_tpm_int(ساختار pttti *pttti, ساختار pttto *pttto)
{
    u32 rc = 0;
    ساختار tpm_req_header *trh = (تهی*)pttti->tpmopin;

    اگر (pttti->ipblength < اندازه(ساختار pttti) + اندازه(*trh)
        || pttti->ipblength != اندازه(ساختار pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < اندازه(ساختار pttto)) {
        rc = TCG_INVALID_درون_داد_PARA;
        برو err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        اگر (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_درون_داد_PARA;
            برو err_exit;
        }
        بشکن;
    حالت TPM_VERSION_2:
        اگر (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_درون_داد_PARA;
            برو err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(ساختار pttto, tpmopout);
    صحیح ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    اگر (ret) {
        rc = TCG_FATAL_COM_ERROR;
        برو err_exit;
    }

    pttto->opblength = offsetof(ساختار pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    اگر (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    بازگشت rc;
}

ایستا u32
shutdown_preboot_interface(تهی)
{
    TPM_interface_shutdown = 1;
    بازگشت 0;
}

ایستا u32
hash_log_event_int(ثابت ساختار hlei *hlei, ساختار hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    ساختار pcpes *pcpes;

    size = hlei->ipblength;
    اگر (size != اندازه(*hlei)) {
        rc = TCG_INVALID_درون_داد_PARA;
        برو err_exit;
    }

    pcpes = (ساختار pcpes *)hlei->logdataptr;

    اگر (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != اندازه(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_درون_داد_PARA;
        برو err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    اگر (rc)
        برو err_exit;

    /* updating the log was fine */
    hleo->opblength = اندازه(ساختار hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_شمار;

err_exit:
    اگر (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    بازگشت rc;
}

ایستا u32
hash_all_int(ثابت ساختار hai *hai, u8 *hash)
{
    اگر (hai->ipblength != اندازه(ساختار hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        بازگشت TCG_INVALID_درون_داد_PARA;

    sha1((ثابت u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    بازگشت 0;
}

ایستا u32
tss_int(ساختار ti *ti, ساختار to *to)
{
    to->opblength = اندازه(ساختار to);
    to->reserved  = 0;

    بازگشت TCG_PC_UNSUPPORTED;
}

ایستا u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    ساختار pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = اندازه(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    اگر (rc)
        بازگشت rc;

    *edx_ptr = tpm_state.entry_شمار;
    بازگشت 0;
}

تهی VISIBLE32FLAT
tpm_interrupt_handler32(ساختار bregs *regs)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت;

    set_cf(regs, 0);

    اگر (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        بازگشت;
    }

    انتخاب ((شمارش irq_ids)regs->al) {
    حالت TCG_StatusCheck:
        اگر (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } وگرنه {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        بشکن;

    حالت TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (ساختار hleei_short *)درون_داد_buf32(regs),
                  (ساختار hleeo *)output_buf32(regs));
        بشکن;

    حالت TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((ساختار pttti *)درون_داد_buf32(regs),
                                    (ساختار pttto *)output_buf32(regs));
        بشکن;

    حالت TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        بشکن;

    حالت TCG_HashLogEvent:
        regs->eax = hash_log_event_int((ساختار hlei*)درون_داد_buf32(regs),
                                       (ساختار hleo*)output_buf32(regs));
        بشکن;

    حالت TCG_HashAll:
        regs->eax =
            hash_all_int((ساختار hai*)درون_داد_buf32(regs),
                          (u8 *)output_buf32(regs));
        بشکن;

    حالت TCG_TSS:
        regs->eax = tss_int((ساختار ti*)درون_داد_buf32(regs),
                            (ساختار to*)output_buf32(regs));
        بشکن;

    حالت TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)درون_داد_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        بشکن;

    پیشفرض:
        set_cf(regs, 1);
    }

    بازگشت;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

تعریفنوع u8 tpm_ppi_code;

ایستا صحیح
tpm12_read_has_owner(صحیح *has_owner)
{
    ساختار tpm_res_getcap_ownerauth oauth;
    صحیح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, اندازه(oauth));
    اگر (ret)
        بازگشت -1;

    *has_owner = oauth.flag;

    بازگشت 0;
}

ایستا صحیح
tpm12_enable_tpm(صحیح enable, صحیح verbose)
{
    ساختار tpm_permanent_flags pf;
    صحیح ret = tpm12_read_permanent_flags((نویسه *)&pf, اندازه(pf));
    اگر (ret)
        بازگشت -1;

    اگر (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        بازگشت 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    اگر (ret) {
        اگر (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        وگرنه
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    بازگشت ret;
}

ایستا صحیح
tpm12_activate_tpm(صحیح activate, صحیح allow_reset, صحیح verbose)
{
    ساختار tpm_permanent_flags pf;
    صحیح ret = tpm12_read_permanent_flags((نویسه *)&pf, اندازه(pf));
    اگر (ret)
        بازگشت -1;

    اگر (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        بازگشت 0;

    اگر (pf.flags[PERM_FLAG_IDX_DISABLE])
        بازگشت 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    اگر (ret)
        بازگشت ret;

    اگر (activate && allow_reset) {
        اگر (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    بازگشت 0;
}

ایستا صحیح
tpm12_enable_activate(صحیح allow_reset, صحیح verbose)
{
    صحیح ret = tpm12_enable_tpm(1, verbose);
    اگر (ret)
        بازگشت ret;

    بازگشت tpm12_activate_tpm(1, allow_reset, verbose);
}

ایستا صحیح
tpm12_force_clear(صحیح enable_activate_before, صحیح enable_activate_after,
                  صحیح verbose)
{
    صحیح has_owner;
    صحیح ret = tpm12_read_has_owner(&has_owner);
    اگر (ret)
        بازگشت -1;
    اگر (!has_owner) {
        اگر (verbose)
            printf("TPM does not have an owner.\n");
        بازگشت 0;
    }

    اگر (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        اگر (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            بازگشت ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    اگر (ret)
        بازگشت ret;

    اگر (!enable_activate_after) {
        اگر (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        بازگشت 0;
    }

    بازگشت tpm12_enable_activate(1, verbose);
}

ایستا صحیح
tpm12_set_owner_install(صحیح allow, صحیح verbose)
{
    صحیح has_owner;
    صحیح ret = tpm12_read_has_owner(&has_owner);
    اگر (ret)
        بازگشت -1;
    اگر (has_owner) {
        اگر (verbose)
            printf("Must first remove owner.\n");
        بازگشت 0;
    }

    ساختار tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((نویسه *)&pf, اندازه(pf));
    اگر (ret)
        بازگشت -1;

    اگر (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        اگر (verbose)
            printf("TPM must first be enable.\n");
        بازگشت 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    اگر (ret)
        بازگشت ret;

    اگر (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    بازگشت 0;
}

ایستا صحیح
tpm12_process_cfg(tpm_ppi_code پیام_Code, صحیح verbose)
{
    صحیح ret = 0;

    انتخاب (پیام_Code) {
        حالت TPM_PPI_OP_NOOP: /* no-op */
            بشکن;

        حالت TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            بشکن;

        حالت TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            بشکن;

        حالت TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            بشکن;

        حالت TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            بشکن;

        حالت TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            بشکن;

        حالت TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            بشکن;

        حالت TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            بشکن;

        پیشفرض:
            بشکن;
    }

    اگر (ret)
        printf("Op %d: An error occurred: 0x%x\n", پیام_Code, ret);

    بازگشت ret;
}

ایستا صحیح
tpm20_clearcontrol(u8 disable, صحیح verbose)
{
    ساختار tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(اندازه(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(اندازه(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    ساختار tpm_rsp_header rsp;
    u32 resp_length = اندازه(rsp);
    صحیح ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_length != اندازه(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    بازگشت ret;
}

ایستا صحیح
tpm20_clear(تهی)
{
    ساختار tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(اندازه(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(اندازه(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ساختار tpm_rsp_header rsp;
    u32 resp_length = اندازه(rsp);
    صحیح ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_length != اندازه(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    بازگشت ret;
}

ایستا صحیح
tpm20_process_cfg(tpm_ppi_code پیام_Code, صحیح verbose)
{
    صحیح ret = 0;

    انتخاب (پیام_Code) {
        حالت TPM_PPI_OP_NOOP: /* no-op */
            بشکن;

        حالت TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            اگر (!ret)
                 ret = tpm20_clear();
            بشکن;
    }

    اگر (ret)
        printf("Op %d: An error occurred: 0x%x\n", پیام_Code, ret);

    بازگشت ret;
}

ایستا صحیح
tpm12_get_tpm_state(تهی)
{
    صحیح state = 0;
    ساختار tpm_permanent_flags pf;
    صحیح has_owner;

    اگر (tpm12_read_permanent_flags((نویسه *)&pf, اندازه(pf)) ||
        tpm12_read_has_owner(&has_owner))
        بازگشت ~0;

    اگر (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    اگر (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    اگر (has_owner) {
        state |= TPM_STATE_OWNED;
    } وگرنه {
        اگر (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    بازگشت state;
}

ایستا تهی
tpm12_show_tpm_menu(صحیح state, صحیح next_scancodes[7])
{
    صحیح i = 0;

    printf("\nThe current state of the TPM is:\n");

    اگر (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    وگرنه
        printf("  Disabled");

    اگر (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    وگرنه
        printf(" and deactivated\n");

    اگر (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    وگرنه {
        printf("  Ownership has not been taken\n");
        اگر (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        وگرنه
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    اگر ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    اگر (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        اگر (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            اگر (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } وگرنه {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } وگرنه {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } وگرنه {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    اگر (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

ایستا تهی
tpm12_menu(تهی)
{
    صحیح scancode, next_scancodes[7];
    tpm_ppi_code پیام_Code;
    صحیح state = 0, i;
    صحیح waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    برای (;;) {
        اگر ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } وگرنه {
            printf("TPM is not working correctly.\n");
            بازگشت;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        پیام_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        وقتی (waitkey) {
            وقتی ((scancode = get_keystroke(1000)) == ~0)
                ;

            انتخاب (scancode) {
            حالت 1:
                // ESC
                reset();
                بشکن;
            حالت 18: /* e. enable */
                پیام_Code = TPM_PPI_OP_ENABLE;
                بشکن;
            حالت 32: /* d. disable */
                پیام_Code = TPM_PPI_OP_DISABLE;
                بشکن;
            حالت 30: /* a. activate */
                پیام_Code = TPM_PPI_OP_ACTIVATE;
                بشکن;
            حالت 47: /* v. deactivate */
                پیام_Code = TPM_PPI_OP_DEACTIVATE;
                بشکن;
            حالت 46: /* c. clear owner */
                پیام_Code = TPM_PPI_OP_CLEAR;
                بشکن;
            حالت 25: /* p. prevent ownerinstall */
                پیام_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                بشکن;
            حالت 31: /* s. allow ownerinstall */
                پیام_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                بشکن;
            پیشفرض:
                ادامه;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            برای (i = 0; i < اندازه(next_scancodes); i++) {
                اگر (next_scancodes[i] == 0)
                    بشکن;

                اگر (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(پیام_Code, 1);
                    waitkey = 0;
                    بشکن;
                }
            }
        }
    }
}

ایستا صحیح
tpm20_menu_change_active_pcrbanks(تهی)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    وقتی (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        وقتی (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ثابت نویسه *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            اگر (hashname) {
                printf("  %d: %s", i, hashname);
                اگر (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        اگر (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        صحیح show = 0;
        وقتی (!show) {
            صحیح scancode = get_keystroke(1000);

            انتخاب (scancode) {
            حالت ~0:
                ادامه;
            حالت 1: /* ESC */
                printf("\n");
                بازگشت -1;
            حالت 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                اگر (flagnum > i)
                    ادامه;
                اگر (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                بشکن;
            حالت 30: /* a */
                اگر (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ایستا تهی
tpm20_menu(تهی)
{
    صحیح scan_code;
    tpm_ppi_code پیام_Code;

    برای (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        پیام_Code = TPM_PPI_OP_NOOP;

        وقتی ((scan_code = get_keystroke(1000)) == ~0)
            ;

        انتخاب (scan_code) {
        حالت 1:
            // ESC
            reset();
            بشکن;
        حالت 2:
            پیام_Code = TPM_PPI_OP_CLEAR;
            بشکن;
        حالت 3:
            tpm20_menu_change_active_pcrbanks();
            ادامه;
        پیشفرض:
            ادامه;
        }

        tpm20_process_cfg(پیام_Code, 0);
    }
}

تهی
tpm_menu(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت;

    وقتی (get_keystroke(0) >= 0)
        ;
    wait_threads();

    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        tpm12_menu();
        بشکن;
    حالت TPM_VERSION_2:
        tpm20_menu();
        بشکن;
    }
}

صحیح
tpm_can_show_menu(تهی)
{
    انتخاب (TPM_version) {
    حالت TPM_VERSION_1_2:
        بازگشت tpm_is_working() && TPM_has_physical_presence;
    حالت TPM_VERSION_2:
        بازگشت tpm_is_working();
    }
    بازگشت 0;
}
