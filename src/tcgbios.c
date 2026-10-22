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

ساخت {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_لمبائی;

    /* start address of TCPA log buffer */
    u8 *          log_area_آغاز_پتہ;

    /* number of log entries written */
    u32           اندراج_گنتی;

    /* address to write next log entry to */
    u8 *          log_area_اگلا_اندراج;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_آخری_اندراج;
} tpm_state VARLOW;

ساکن صحیح tpm_سیٹ_log_area(u8 *log_area_آغاز_پتہ,
                            u32 log_area_minimum_لمبائی)
{
    اگر (!log_area_آغاز_پتہ || !log_area_minimum_لمبائی)
        واپس -1;

    memset(log_area_آغاز_پتہ, 0, log_area_minimum_لمبائی);
    tpm_state.log_area_آغاز_پتہ = log_area_آغاز_پتہ;
    tpm_state.log_area_minimum_لمبائی = log_area_minimum_لمبائی;
    tpm_state.log_area_اگلا_اندراج = log_area_آغاز_پتہ;
    tpm_state.log_area_آخری_اندراج = تہی;
    tpm_state.اندراج_گنتی = 0;
    واپس 0;
}

ساکن صحیح
tpm_tcpa_probe(خالی)
{
    ساخت tcpa_descriptor_rev2 *tcpa = ڈھونڈو_acpi_جدول(TCPA_SIGNATURE);
    اگر (!tcpa)
        واپس -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(طویل)tcpa->log_area_آغاز_پتہ,
            tcpa->log_area_minimum_لمبائی);

    واپس tpm_سیٹ_log_area((u8*)(طویل)tcpa->log_area_آغاز_پتہ,
                            tcpa->log_area_minimum_لمبائی);
}

ساکن صحیح
tpm_tpm2_probe(خالی)
{
    ساخت tpm2_descriptor_rev2 *tpm2 = ڈھونڈو_acpi_جدول(TPM2_SIGNATURE);
    اگر (!tpm2)
        واپس -1;

    اگر (tpm2->لمبائی < 76)
        واپس -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(طویل)tpm2->log_area_آغاز_پتہ,
            tpm2->log_area_minimum_لمبائی);

    واپس tpm_سیٹ_log_area((u8*)(طویل)tpm2->log_area_آغاز_پتہ,
                            tpm2->log_area_minimum_لمبائی);
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
ساکن صحیح
tpm_log_event(ساخت tpm_log_سرخی *اندراج_2, صحیح digest_len
              , ثابت خالی *event, صحیح event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_آغاز_پتہ, tpm_state.log_area_اگلا_اندراج);

    اگر (tpm_state.log_area_اگلا_اندراج == تہی)
        واپس -1;

    u32 سائز = (حجم(*اندراج_2) + digest_len
                + حجم(ساخت tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_اگلا_اندراج + سائز
                   - tpm_state.log_area_آغاز_پتہ);
    اگر (logsize > tpm_state.log_area_minimum_لمبائی) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", سائز);
        واپس -1;
    }

    خالی *dest = tpm_state.log_area_اگلا_اندراج;
    حافظہ_نقل(dest, اندراج_2, حجم(*اندراج_2) + digest_len);
    ساخت tpm_log_trailer *t = dest + حجم(*اندراج_2) + digest_len;
    t->eventdatasize = event_len;
    حافظہ_نقل(t->event, event, event_len);

    tpm_state.log_area_آخری_اندراج = tpm_state.log_area_اگلا_اندراج;
    tpm_state.log_area_اگلا_اندراج += سائز;
    tpm_state.اندراج_گنتی++;

    واپس 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ساکن TPMVersion TPM_نسخہ;
ساکن u32 tpm20_pcr_selection_سائز;
ساکن ساخت tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
ساخت tpm_log_اندراج {
    ساخت tpm_log_سرخی hdr;
    u8 pad[حجم(ساخت tpm2_digest_اقدار)
           + 8 * حجم(ساخت tpm2_digest_قدر)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ساکن ثابت ساخت hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ثابت حرف *name;
    خالی (*hashfunc)(ثابت u8 *ڈیٹا, u32 لمبائی, u8 *hash);
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

ساکن صحیح
tpm20_get_hash_buffersize(u16 hashAlg)
{
    غیرعلامتی i;

    برائے (i = 0; i < ARRAY_سائز(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg == hashAlg)
            واپس hash_parameters[i].hash_buffersize;
    }
    واپس -1;
}

ساکن u8
tpm20_hashalg_مقصد_flag(u16 hashAlg)
{
    غیرعلامتی i;

    برائے (i = 0; i < ARRAY_سائز(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg == hashAlg)
            واپس hash_parameters[i].hashalg_flag;
    }
    واپس 0;
}

ساکن u16
tpm20_hashalg_flag_مقصد_hashalg(u8 hashalg_flag)
{
    غیرعلامتی i;

    برائے (i = 0; i < ARRAY_سائز(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg_flag == hashalg_flag)
            واپس hash_parameters[i].hashalg;
    }
    واپس 0;
}

ساکن ثابت حرف *
tpm20_hashalg_flag_مقصد_name(u8 hashalg_flag)
{
    غیرعلامتی i;

    برائے (i = 0; i < ARRAY_سائز(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg_flag == hashalg_flag)
            واپس hash_parameters[i].name;
    }
    واپس تہی;
}

ساکن خالی tpm2_hash_ڈیٹا(u16 hashAlg, ثابت u8 *ڈیٹا, u32 ڈیٹا_len, u8 *hash)
{
    غیرعلامتی i;

    برائے (i = 0; i < ARRAY_سائز(hash_parameters); i++) {
        اگر (hash_parameters[i].hashalg == hashAlg) {
            اگر (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(ڈیٹا, ڈیٹا_len, hash);
            } ورنہ {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ساکن صحیح
tpm20_write_EfiSpecIdEventStruct(خالی)
{
    اگر (!tpm20_pcr_selection)
        واپس -1;

    ساخت {
        ساخت TCG_EfiSpecIdEventStruct hdr;
        u8 pad[حجم(ساخت tpm_log_اندراج) + حجم(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.spec_نسخہ_Minor = 0,
        .hdr.spec_نسخہ_Major = 2,
        .hdr.specErrata = 2,
        .hdr.uintn_سائز = 2,
    };

    ساخت tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    خالی *nsel, *انتہا = (خالی*)tpm20_pcr_selection + tpm20_pcr_selection_سائز;

    u32 گنتی, عدد_Algs = 0;
    برائے (گنتی = 0; گنتی < be32_مقصد_cpu(tpm20_pcr_selection->گنتی); گنتی++) {
        u8 سائز_کا_Select = sel->سائز_کا_Select;

        nsel = (خالی*)sel + حجم(*sel) + سائز_کا_Select;
        اگر (nsel > انتہا)
            توڑو;

        اگر (!سائز_کا_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            جاری;
        }

        صحیح hsize = tpm20_get_hash_buffersize(be16_مقصد_cpu(sel->hashAlg));
        اگر (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_مقصد_cpu(sel->hashAlg));
            واپس -1;
        }

        صحیح event_سائز = offsetof(ساخت TCG_EfiSpecIdEventStruct
                                  , digestSizes[گنتی+1]);
        اگر (event_سائز > حجم(event) - حجم(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            واپس -1;
        }

        event.hdr.digestSizes[عدد_Algs].algorithmId = be16_مقصد_cpu(sel->hashAlg);
        event.hdr.digestSizes[عدد_Algs].digest_سائز = hsize;
        عدد_Algs++;

        sel = nsel;
    }

    اگر (sel != انتہا) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        واپس -1;
    }

    event.hdr.نمبر_کا_Algorithms = عدد_Algs;
    صحیح event_سائز = offsetof(ساخت TCG_EfiSpecIdEventStruct
                              , digestSizes[عدد_Algs]);
    u8 *vendor_Info_سائز = (خالی*)&event + event_سائز;
    *vendor_Info_سائز = 0;
    event_سائز += حجم(*vendor_Info_سائز);

    ساخت tpm_log_اندراج le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    واپس tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_سائز);
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
ساکن صحیح
tpm20_build_digest(ساخت tpm_log_اندراج *le,
                   ثابت u8 *hashdata, u32 hashdata_len, صحیح bigEndian)
{
    اگر (!tpm20_pcr_selection)
        واپس -1;

    ساخت tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    خالی *nsel, *انتہا = (خالی*)tpm20_pcr_selection + tpm20_pcr_selection_سائز;
    خالی *dest = le->hdr.digest + حجم(ساخت tpm2_digest_اقدار);

    u32 گنتی, عدد_Algs = 0;
    برائے (گنتی = 0; گنتی < be32_مقصد_cpu(tpm20_pcr_selection->گنتی); گنتی++) {
        u8 سائز_کا_Select = sel->سائز_کا_Select;

        nsel = (خالی*)sel + حجم(*sel) + سائز_کا_Select;
        اگر (nsel > انتہا)
            توڑو;

        /* PCR 0-7 unused? -- skip */
        اگر (!سائز_کا_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            جاری;
        }

        صحیح hsize = tpm20_get_hash_buffersize(be16_مقصد_cpu(sel->hashAlg));
        اگر (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_مقصد_cpu(sel->hashAlg));
            واپس -1;
        }

        /* buffer size sanity check before writing */
        ساخت tpm2_digest_قدر *v = dest;
        اگر (dest + حجم(*v) + hsize > (خالی*)le + حجم(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            واپس -1;
        }

        اگر (bigEndian)
            v->hashAlg = sel->hashAlg;
        ورنہ
            v->hashAlg = be16_مقصد_cpu(sel->hashAlg);

        tpm2_hash_ڈیٹا(be16_مقصد_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += حجم(*v) + hsize;
        sel = nsel;

        عدد_Algs++;
    }

    اگر (sel != انتہا) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        واپس -1;
    }

    ساخت tpm2_digest_اقدار *v = (خالی*)le->hdr.digest;
    اگر (bigEndian)
        v->گنتی = cpu_مقصد_be32(عدد_Algs);
    ورنہ
        v->گنتی = عدد_Algs;

    واپس dest - (خالی*)le->hdr.digest;
}

ساکن صحیح
tpm12_build_digest(ساخت tpm_log_اندراج *le,
                   ثابت u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    واپس SHA1_BUFSIZE;
}

ساکن صحیح
tpm12_build_digest_براہ_راست(ساخت tpm_log_اندراج *le, ثابت u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    حافظہ_نقل(le->hdr.digest, sha1, SHA1_BUFSIZE);
    واپس SHA1_BUFSIZE;
}

ساکن صحیح
tpm_build_digest(ساخت tpm_log_اندراج *le, ثابت u8 *hashdata, u32 hashdata_len
                 , صحیح bigEndian)
{
    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        واپس tpm12_build_digest(le, hashdata, hashdata_len);
    حالت TPM_نسخہ_2:
        واپس tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    واپس -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ساکن صحیح
tpm_simple_cmd(u8 locty, u32 ordinal
               , صحیح param_سائز, u16 param, شمار tpmDurationType مقصد_t)
{
    ساخت {
        ساخت tpm_req_سرخی trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_مقصد_be32(حجم(req.trqh) + param_سائز),
        .trqh.ordinal = cpu_مقصد_be32(ordinal),
        .param = param_سائز == 2 ? cpu_مقصد_be16(param) : param,
    };
    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        req.trqh.لیبل = cpu_مقصد_be16(TPM_لیبل_RQU_CMD);
        توڑو;
    حالت TPM_نسخہ_2:
        req.trqh.لیبل = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS);
        توڑو;
    }

    u8 obuffer[64];
    ساخت tpm_rsp_سرخی *trsh = (خالی*)obuffer;
    u32 obuffer_len = حجم(obuffer);
    memset(obuffer, 0x0, حجم(obuffer));

    صحیح ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, مقصد_t);
    ret = ret ? -1 : be32_مقصد_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    واپس ret;
}

ساکن صحیح
tpm20_getcapability(u32 capability, u32 property, u32 گنتی,
                    ساخت tpm_rsp_سرخی *rsp, u32 rsize)
{
    ساخت tpm2_req_getcapability trg = {
        .hdr.لیبل = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(trg)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_GetCapability),
        .capability = cpu_مقصد_be32(capability),
        .property = cpu_مقصد_be32(property),
        .propertycount = cpu_مقصد_be32(گنتی),
    };

    u32 resp_سائز = rsize;
    صحیح ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_سائز,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_مقصد_cpu(rsp->totlen)) ? -1 : be32_مقصد_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    واپس ret;
}

ساکن صحیح
tpm20_get_pcrbanks(خالی)
{
    u8 بفر[128];
    ساخت tpm2_res_getcapability *trg =
      (ساخت tpm2_res_getcapability *)&بفر;

    صحیح ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  حجم(بفر));
    اگر (ret)
        واپس ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_مقصد_cpu(trg->hdr.totlen);
    اگر (resplen <= offsetof(ساخت tpm2_res_getcapability, ڈیٹا))
        واپس -1;

    u32 سائز = resplen - offsetof(ساخت tpm2_res_getcapability, ڈیٹا);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    اگر (سائز < offsetof(ساخت tpml_pcr_selection, selections) +
               offsetof(ساخت tpms_pcr_selection, pcrSelect))
        واپس -1;

    tpm20_pcr_selection = malloc_بلند(سائز);
    اگر (tpm20_pcr_selection) {
        حافظہ_نقل(tpm20_pcr_selection, &trg->ڈیٹا, سائز);
        tpm20_pcr_selection_سائز = سائز;
    } ورنہ {
        warn_noalloc();
        ret = -1;
    }

    واپس ret;
}

ساکن صحیح
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *فعال_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *فعال_pcrbanks = 0;

    اگر (!tpm20_pcr_selection)
        واپس -1;

    ساخت tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    خالی *انتہا = (خالی*)tpm20_pcr_selection + tpm20_pcr_selection_سائز;

    جب (1) {
        u8 سائز_کا_Select = sel->سائز_کا_Select;
        خالی *nsel = (خالی*)sel + حجم(*sel) + سائز_کا_Select;
        اگر (nsel > انتہا)
            واپس 0;

        u16 hashalg = be16_مقصد_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_مقصد_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        غیرعلامتی i;
        برائے (i = 0; i < سائز_کا_Select; i++) {
            اگر (sel->pcrSelect[i]) {
                *فعال_pcrbanks |= hashalg_flag;
                توڑو;
            }
        }

        sel = nsel;
    }
}

ساکن صحیح
tpm20_سیٹ_pcrbanks(u32 فعال_banks)
{
    ساخت tpm2_req_pcr_allocate trpa = {
        .hdr.لیبل     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_مقصد_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_مقصد_be32(حجم(trpa.authblock)),
        .authblock = {
            .handle = cpu_مقصد_be32(TPM2_RS_PW),
            .noncesize = cpu_مقصد_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_مقصد_be16(0),
        },
    };
    ساخت tpms_pcr_selection3 {
        u16 hashAlg;
        u8 سائز_کا_Select;
        u8 pcrSelect[3];
    } tps[ARRAY_سائز(trpa.tpms_pcr_selections)];
    صحیح i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    جب (hashalg_flag) {
        اگر ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_مقصد_hashalg(hashalg_flag);

            اگر (hashalg) {
                u8 نقاب = 0;
                tps[i].hashAlg = cpu_مقصد_be16(hashalg);
                tps[i].سائز_کا_Select = 3;

                اگر (فعال_banks & hashalg_flag)
                    نقاب = 0xff;

                tps[i].pcrSelect[0] = نقاب;
                tps[i].pcrSelect[1] = نقاب;
                tps[i].pcrSelect[2] = نقاب;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.گنتی = cpu_مقصد_be32(i);
    حافظہ_نقل(trpa.tpms_pcr_selections, tps, i * حجم(tps[0]));
    trpa.hdr.totlen = cpu_مقصد_be32(offsetof(ساخت tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * حجم(tps[0]));

    ساخت tpm_rsp_سرخی rsp;
    u32 resp_لمبائی = حجم(rsp);

    صحیح ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_مقصد_cpu(rsp.errcode);

    واپس ret;
}

ساکن صحیح tpm20_activate_pcrbanks(u32 فعال_banks)
{
    صحیح ret = tpm20_سیٹ_pcrbanks(فعال_banks);
    اگر (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    اگر (!ret)
        reset();
    واپس ret;
}

ساکن صحیح
tpm12_get_capability(u32 cap, u32 subcap, ساخت tpm_rsp_سرخی *rsp, u32 rsize)
{
    ساخت tpm_req_getcap trgc = {
        .hdr.لیبل = cpu_مقصد_be16(TPM_لیبل_RQU_CMD),
        .hdr.totlen = cpu_مقصد_be32(حجم(trgc)),
        .hdr.ordinal = cpu_مقصد_be32(TPM_ORD_GetCapability),
        .capArea = cpu_مقصد_be32(cap),
        .sub_Cap_سائز = cpu_مقصد_be32(حجم(trgc.subCap)),
        .subCap = cpu_مقصد_be32(subcap)
    };
    u32 resp_سائز = rsize;
    صحیح ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_سائز,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_سائز != rsize) ? -1 : be32_مقصد_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    واپس ret;
}

ساکن صحیح
tpm12_read_permanent_flags(حرف *buf, صحیح buf_len)
{
    memset(buf, 0, buf_len);

    ساخت tpm_res_getcap_perm_flags pf;
    صحیح ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, حجم(pf));
    اگر (ret)
        واپس -1;

    حافظہ_نقل(buf, &pf.perm_flags, buf_len);

    واپس 0;
}

ساکن صحیح
tpm12_determine_timeouts(خالی)
{
    ساخت tpm_res_getcap_timeouts timeouts;
    صحیح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, حجم(timeouts));
    اگر (ret)
        واپس ret;

    ساخت tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, حجم(durations));
    اگر (ret)
        واپس ret;

    صحیح i;
    برائے (i = 0; i < 3; i++)
        durations.durations[i] = be32_مقصد_cpu(durations.durations[i]);

    برائے (i = 0; i < 4; i++)
        timeouts.timeouts[i] = be32_مقصد_cpu(timeouts.timeouts[i]);

    dprintf(DEBUG_tcg, "TCGBIOS: timeouts: %u %u %u %u\n",
            timeouts.timeouts[0],
            timeouts.timeouts[1],
            timeouts.timeouts[2],
            timeouts.timeouts[3]);

    dprintf(DEBUG_tcg, "TCGBIOS: durations: %u %u %u\n",
            durations.durations[0],
            durations.durations[1],
            durations.durations[2]);

    tpmhw_سیٹ_timeouts(timeouts.timeouts, durations.durations);

    واپس 0;
}

ساکن خالی
tpm20_سیٹ_timeouts(خالی)
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

    tpmhw_سیٹ_timeouts(timeouts, durations);
}

ساکن صحیح
tpm12_extend(ساخت tpm_log_اندراج *le, صحیح digest_len)
{
    ساخت tpm_req_extend tre = {
        .hdr.لیبل     = cpu_مقصد_be16(TPM_لیبل_RQU_CMD),
        .hdr.totlen  = cpu_مقصد_be32(حجم(tre)),
        .hdr.ordinal = cpu_مقصد_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_مقصد_be32(le->hdr.pcrindex),
    };
    حافظہ_نقل(tre.digest, le->hdr.digest, حجم(tre.digest));

    ساخت tpm_rsp_extend rsp;
    u32 resp_لمبائی = حجم(rsp);
    صحیح ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.hdr.errcode)
        واپس -1;

    واپس 0;
}

ساکن صحیح tpm20_extend(ساخت tpm_log_اندراج *le, صحیح digest_len)
{
    ساخت tpm2_req_extend tmp_tre = {
        .hdr.لیبل     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_مقصد_be32(0),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_مقصد_be32(le->hdr.pcrindex),
        .authblocksize = cpu_مقصد_be32(حجم(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_مقصد_be32(TPM2_RS_PW),
            .noncesize = cpu_مقصد_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_مقصد_be16(0),
        },
    };
    u8 بفر[حجم(tmp_tre) + حجم(le->pad)];
    ساخت tpm2_req_extend *tre = (ساخت tpm2_req_extend *)بفر;

    حافظہ_نقل(tre, &tmp_tre, حجم(tmp_tre));
    حافظہ_نقل(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_مقصد_be32(حجم(tmp_tre) + digest_len);

    ساخت tpm_rsp_سرخی rsp;
    u32 resp_لمبائی = حجم(rsp);
    صحیح ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.errcode)
        واپس -1;

    واپس 0;
}

ساکن صحیح
tpm_extend(ساخت tpm_log_اندراج *le, صحیح digest_len)
{
    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        واپس tpm12_extend(le, digest_len);
    حالت TPM_نسخہ_2:
        واپس tpm20_extend(le, digest_len);
    }
    واپس -1;
}

ساکن صحیح
tpm20_stirrandom(خالی)
{
    ساخت tpm2_req_stirrandom stir = {
        .hdr.لیبل = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(stir)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_StirRandom),
        .سائز = cpu_مقصد_be16(حجم(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    ساخت tpm_rsp_سرخی rsp;
    u32 resp_لمبائی = حجم(rsp);
    صحیح ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    واپس ret;
}

ساکن صحیح
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    ساخت tpm2_res_getrandom rsp;

    اگر (buf_len > حجم(rsp.rnd.بفر))
        واپس -1;

    ساخت tpm2_req_getrandom trgr = {
        .hdr.لیبل = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(trgr)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_GetRandom),
        .بائٹیں_Requested = cpu_مقصد_be16(buf_len),
    };
    u32 resp_لمبائی = حجم(rsp);

    صحیح ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.hdr.errcode)
        ret = -1;
    ورنہ
        حافظہ_نقل(buf, rsp.rnd.بفر, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    واپس ret;
}

ساکن صحیح
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    ساخت tpm2_req_hierarchycontrol trh = {
        .hdr.لیبل = cpu_مقصد_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(trh)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_مقصد_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_مقصد_be32(حجم(trh.authblock)),
        .authblock = {
            .handle = cpu_مقصد_be32(TPM2_RS_PW),
            .noncesize = cpu_مقصد_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_مقصد_be16(0),
        },
        .enable = cpu_مقصد_be32(hierarchy),
        .state = state,
    };
    ساخت tpm_rsp_سرخی rsp;
    u32 resp_لمبائی = حجم(rsp);
    صحیح ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    واپس ret;
}

ساکن صحیح
tpm20_hierarchychangeauth(u8 auth[20])
{
    ساخت tpm2_req_hierarchychangeauth trhca = {
        .hdr.لیبل = cpu_مقصد_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(trhca)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_مقصد_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_مقصد_be32(حجم(trhca.authblock)),
        .authblock = {
            .handle = cpu_مقصد_be32(TPM2_RS_PW),
            .noncesize = cpu_مقصد_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_مقصد_be16(0),
        },
        .newAuth = {
            .سائز = cpu_مقصد_be16(حجم(trhca.newAuth.بفر)),
        },
    };
    حافظہ_نقل(trhca.newAuth.بفر, auth, حجم(trhca.newAuth.بفر));

    ساخت tpm_rsp_سرخی rsp;
    u32 resp_لمبائی = حجم(rsp);
    صحیح ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    واپس ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ساکن صحیح TPM_has_physical_presence;
u8 TPM_working VARLOW;

ساکن صحیح
tpm_is_working(خالی)
{
    واپس CONFIG_TCGBIOS && TPM_working;
}

ساکن خالی
tpm_سیٹ_failure(خالی)
{
    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_سیٹ_عارضی_Deactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        توڑو;
    حالت TPM_نسخہ_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        توڑو;
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
ساکن خالی
tpm_جوڑو_measurement_مقصد_log(u32 pcrindex, u32 event_type,
                           ثابت حرف *event, u32 event_لمبائی,
                           ثابت u8 *hashdata, u32 hashdata_لمبائی)
{
    اگر (!tpm_is_working())
        واپس;

    ساخت tpm_log_اندراج le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    صحیح digest_len = tpm_build_digest(&le, hashdata, hashdata_لمبائی, 1);
    اگر (digest_len < 0)
        واپس;
    صحیح ret = tpm_extend(&le, digest_len);
    اگر (ret) {
        tpm_سیٹ_failure();
        واپس;
    }
    tpm_build_digest(&le, hashdata, hashdata_لمبائی, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_لمبائی);
}

// Add an EV_ACTION measurement to the list of measurements
ساکن خالی
tpm_جوڑو_action(u32 pcr_اشاریہ, ثابت حرف *string)
{
    u32 len = لڑی_لمبائی(string);
    tpm_جوڑو_measurement_مقصد_log(pcr_اشاریہ, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ساکن خالی
tpm_جوڑو_event_separators(خالی)
{
    ساکن ثابت u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcr_اشاریہ;
    برائے (pcr_اشاریہ = 0; pcr_اشاریہ <= 7; pcr_اشاریہ++)
        tpm_جوڑو_measurement_مقصد_log(pcr_اشاریہ, EV_SEPARATOR,
                                   (ثابت حرف *)evt_separator,
                                   حجم(evt_separator),
                                   evt_separator,
                                   حجم(evt_separator));
}

ساکن خالی
tpm_smbios_measure(خالی)
{
    ساخت pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    خالی *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    اگر (!smbios_tables)
        واپس;

    sha1((ثابت u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_جوڑو_measurement_مقصد_log(1,
                               EV_EVENT_لیبل,
                               (ثابت حرف *)&pcctes, حجم(pcctes),
                               (u8 *)&pcctes, حجم(pcctes));
}

ساکن صحیح
tpm12_assert_physical_presence(خالی)
{
    صحیح ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    اگر (!ret)
        واپس 0;

    ساخت tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    اگر (ret)
        واپس -1;

    /* check if hardware physical presence is supported */
    اگر (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        واپس 0;
    }

    اگر (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_تالا]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        واپس tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    واپس -1;
}

ساکن صحیح
tpm12_startup(خالی)
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
        جاؤ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    اگر (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    اگر (ret)
        جاؤ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_Self_Test_بھرا,
                         0, 0, TPM_DURATION_TYPE_LONG);
    اگر (ret)
        جاؤ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_Reset_Establishment_ثنائی_ہندسہ,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    اگر (ret && ret != TPM_BAD_LOCALITY)
        جاؤ err_exit;

    واپس 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_سیٹ_failure();
    واپس -1;
}

ساکن صحیح
tpm20_startup(خالی)
{
    tpm20_سیٹ_timeouts();

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
        جاؤ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    اگر (ret)
        جاؤ err_exit;

    ret = tpm20_get_pcrbanks();
    اگر (ret)
        جاؤ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    اگر (ret)
        جاؤ err_exit;

    واپس 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_سیٹ_failure();
    واپس -1;
}

ساکن صحیح
tpm_startup(خالی)
{
    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        واپس tpm12_startup();
    حالت TPM_نسخہ_2:
        واپس tpm20_startup();
    }
    واپس -1;
}

خالی
tpm_تیاری(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس;

    صحیح ret = tpm_tpm2_probe();
    اگر (ret) {
        ret = tpm_tcpa_probe();
        اگر (ret)
            واپس;
    }

    TPM_نسخہ = tpmhw_probe();
    اگر (TPM_نسخہ == TPM_نسخہ_NONE)
        واپس;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_نسخہ == TPM_نسخہ_1_2) ? "1.2" : "2");

    TPM_working = 1;

    اگر (runningOnXen())
        واپس;

    ret = tpm_startup();
    اگر (ret)
        واپس;

    tpm_smbios_measure();
    tpm_جوڑو_action(2, "Start Option ROM Scan");
}

ساکن خالی
tpm20_prepboot(خالی)
{
    صحیح ret = tpm20_stirrandom();
    اگر (ret)
         جاؤ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], حجم(auth));
    اگر (ret)
        جاؤ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    اگر (ret)
        جاؤ err_exit;

    واپس;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_سیٹ_failure();
}

خالی
tpm_prepboot(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس;

    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        اگر (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_تالا, TPM_DURATION_TYPE_SHORT);
        توڑو;
    حالت TPM_نسخہ_2:
        tpm20_prepboot();
        توڑو;
    }

    tpm_جوڑو_action(4, "Calling INT 19h");
    tpm_جوڑو_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
خالی
tpm_option_rom(ثابت خالی *addr, u32 len)
{
    اگر (!tpm_is_working())
        واپس;

    ساخت pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = حجم(u16) + حجم(u16) + SHA1_BUFSIZE,
    };
    sha1((ثابت u8 *)addr, len, pcctes.digest);
    tpm_جوڑو_measurement_مقصد_log(2,
                               EV_EVENT_لیبل,
                               (ثابت حرف *)&pcctes, حجم(pcctes),
                               (u8 *)&pcctes, حجم(pcctes));
}

خالی
tpm_جوڑو_bcv(u32 bootdrv, ثابت u8 *addr, u32 لمبائی)
{
    اگر (!tpm_is_working())
        واپس;

    اگر (لمبائی < 0x200)
        واپس;

    ثابت حرف *string = "Booting BCV device 00h (Floppy)";
    اگر (bootdrv == 0x80)
        string = "Booting BCV device 80h (HDD)";
    tpm_جوڑو_action(4, string);

    /* specs: see section 'Hard Disk Device or Hard Disk-Like Devices' */
    /* equivalent to: dd if=/dev/hda ibs=1 count=440 | sha1sum */
    string = "MBR";
    tpm_جوڑو_measurement_مقصد_log(4, EV_IPL,
                               string, لڑی_لمبائی(string),
                               addr, 0x1b8);

    /* equivalent to: dd if=/dev/hda ibs=1 count=72 skip=440 | sha1sum */
    string = "MBR PARTITION_TABLE";
    tpm_جوڑو_measurement_مقصد_log(5, EV_IPL_تقسیم_ڈیٹا,
                               string, لڑی_لمبائی(string),
                               addr + 0x1b8, 0x48);
}

خالی
tpm_جوڑو_cdrom(u32 bootdrv, ثابت u8 *addr, u32 لمبائی)
{
    اگر (!tpm_is_working())
        واپس;

    tpm_جوڑو_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت حرف *string = "EL TORITO IPL";
    tpm_جوڑو_measurement_مقصد_log(4, EV_IPL,
                               string, لڑی_لمبائی(string),
                               addr, لمبائی);
}

خالی
tpm_جوڑو_cdrom_catalog(ثابت u8 *addr, u32 لمبائی)
{
    اگر (!tpm_is_working())
        واپس;

    tpm_جوڑو_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت حرف *string = "BOOT CATALOG";
    tpm_جوڑو_measurement_مقصد_log(5, EV_IPL_تقسیم_ڈیٹا,
                               string, لڑی_لمبائی(string),
                               addr, لمبائی);
}

خالی
tpm_s3_resume(خالی)
{
    اگر (!tpm_is_working())
        واپس;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    صحیح ret = -1;

    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        توڑو;
    حالت TPM_نسخہ_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        اگر (ret)
            جاؤ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        توڑو;
    }

    اگر (ret)
        جاؤ err_exit;

    واپس;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_سیٹ_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ساکن اندرونی خالی *داخلہ_buf32(ساخت bregs *regs)
{
    واپس MAKE_FLATPTR(regs->es, regs->di);
}

ساکن اندرونی خالی *output_buf32(ساخت bregs *regs)
{
    واپس MAKE_FLATPTR(regs->ds, regs->si);
}

ساکن u32
hash_log_extend(ساخت pcpes *pcpes, ثابت خالی *hashdata, u32 hashdata_لمبائی
                , خالی *event, صحیح extend)
{
    اگر (pcpes->pcrindex >= 24)
        واپس TCG_INVALID_داخلہ_PARA;
    اگر (hashdata)
        sha1(hashdata, hashdata_لمبائی, pcpes->digest);

    ساخت tpm_log_اندراج le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    صحیح digest_len = tpm12_build_digest_براہ_راست(&le, pcpes->digest);
    اگر (digest_len < 0)
        واپس TCG_GENERAL_ERROR;
    اگر (extend) {
        صحیح ret = tpm_extend(&le, digest_len);
        اگر (ret)
            واپس TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_براہ_راست(&le, pcpes->digest);
    صحیح ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    اگر (ret)
        واپس TCG_PC_LOGOVERFLOW;
    واپس 0;
}

ساکن u32
hash_log_extend_event_int(ثابت ساخت hleei_short *hleei_s,
                          ساخت hleeo *hleeo)
{
    u32 rc = 0;
    ساخت hleei_long *hleei_l = (ساخت hleei_long *)hleei_s;
    ثابت خالی *logdataptr;
    u32 logdatalen;
    ساخت pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    انتخاب (hleei_s->ipblength) {
    حالت حجم(ساخت hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    توڑو;

    حالت حجم(ساخت hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    توڑو;

    طےشدہ:
        /* bad input block */
        rc = TCG_INVALID_داخلہ_PARA;
        جاؤ err_exit;
    }

    pcpes = (ساخت pcpes *)logdataptr;

    اگر (pcpes->pcrindex != pcrindex
        || logdatalen != حجم(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_داخلہ_PARA;
        جاؤ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    اگر (rc)
        جاؤ err_exit;

    hleeo->opblength = حجم(ساخت hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.اندراج_گنتی;
    حافظہ_نقل(hleeo->digest, pcpes->digest, حجم(hleeo->digest));

err_exit:
    اگر (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    واپس rc;
}

ساکن u32
pass_through_مقصد_tpm_int(ساخت pttti *pttti, ساخت pttto *pttto)
{
    u32 rc = 0;
    ساخت tpm_req_سرخی *trh = (خالی*)pttti->tpmopin;

    اگر (pttti->ipblength < حجم(ساخت pttti) + حجم(*trh)
        || pttti->ipblength != حجم(ساخت pttti) + be32_مقصد_cpu(trh->totlen)
        || pttti->opblength < حجم(ساخت pttto)) {
        rc = TCG_INVALID_داخلہ_PARA;
        جاؤ err_exit;
    }

    u16 لیبل = be16_مقصد_cpu(trh->لیبل);

    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        اگر (لیبل != TPM_لیبل_RQU_CMD && لیبل != TPM_لیبل_RQU_AUTH1_CMD
            && لیبل != TPM_لیبل_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_داخلہ_PARA;
            جاؤ err_exit;
        }
        توڑو;
    حالت TPM_نسخہ_2:
        اگر (لیبل != TPM2_ST_NO_SESSIONS && لیبل != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_داخلہ_PARA;
            جاؤ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(ساخت pttto, tpmopout);
    صحیح ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    اگر (ret) {
        rc = TCG_FATAL_COM_ERROR;
        جاؤ err_exit;
    }

    pttto->opblength = offsetof(ساخت pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    اگر (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    واپس rc;
}

ساکن u32
shutdown_preboot_interface(خالی)
{
    TPM_interface_shutdown = 1;
    واپس 0;
}

ساکن u32
hash_log_event_int(ثابت ساخت hlei *hlei, ساخت hleo *hleo)
{
    u32 rc = 0;
    u16 سائز;
    ساخت pcpes *pcpes;

    سائز = hlei->ipblength;
    اگر (سائز != حجم(*hlei)) {
        rc = TCG_INVALID_داخلہ_PARA;
        جاؤ err_exit;
    }

    pcpes = (ساخت pcpes *)hlei->logdataptr;

    اگر (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != حجم(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_داخلہ_PARA;
        جاؤ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    اگر (rc)
        جاؤ err_exit;

    /* updating the log was fine */
    hleo->opblength = حجم(ساخت hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.اندراج_گنتی;

err_exit:
    اگر (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    واپس rc;
}

ساکن u32
hash_all_int(ثابت ساخت hai *hai, u8 *hash)
{
    اگر (hai->ipblength != حجم(ساخت hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        واپس TCG_INVALID_داخلہ_PARA;

    sha1((ثابت u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    واپس 0;
}

ساکن u32
tss_int(ساخت ti *ti, ساخت مقصد *مقصد)
{
    مقصد->opblength = حجم(ساخت مقصد);
    مقصد->reserved  = 0;

    واپس TCG_PC_UNSUPPORTED;
}

ساکن u32
compact_hash_log_extend_event_int(u8 *بفر,
                                  u32 info,
                                  u32 لمبائی,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    ساخت pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = حجم(info),
    };
    u32 rc = hash_log_extend(&pcpes, بفر, لمبائی, &info, 1);
    اگر (rc)
        واپس rc;

    *edx_ptr = tpm_state.اندراج_گنتی;
    واپس 0;
}

خالی VISIBLE32FLAT
tpm_مداخلت_handler32(ساخت bregs *regs)
{
    اگر (!CONFIG_TCGBIOS)
        واپس;

    سیٹ_cf(regs, 0);

    اگر (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        واپس;
    }

    انتخاب ((شمار irq_ids)regs->al) {
    حالت TCG_حالت_Check:
        اگر (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } ورنہ {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_نسخہ_MAJOR;
            regs->cl = TCG_نسخہ_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_آغاز_پتہ;
            regs->edi = (u32)tpm_state.log_area_آخری_اندراج;
        }
        توڑو;

    حالت TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (ساخت hleei_short *)داخلہ_buf32(regs),
                  (ساخت hleeo *)output_buf32(regs));
        توڑو;

    حالت TCG_Pass_Through_مقصد_TPM:
        regs->eax =
            pass_through_مقصد_tpm_int((ساخت pttti *)داخلہ_buf32(regs),
                                    (ساخت pttto *)output_buf32(regs));
        توڑو;

    حالت TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        توڑو;

    حالت TCG_HashLogEvent:
        regs->eax = hash_log_event_int((ساخت hlei*)داخلہ_buf32(regs),
                                       (ساخت hleo*)output_buf32(regs));
        توڑو;

    حالت TCG_HashAll:
        regs->eax =
            hash_all_int((ساخت hai*)داخلہ_buf32(regs),
                          (u8 *)output_buf32(regs));
        توڑو;

    حالت TCG_TSS:
        regs->eax = tss_int((ساخت ti*)داخلہ_buf32(regs),
                            (ساخت مقصد*)output_buf32(regs));
        توڑو;

    حالت TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)داخلہ_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        توڑو;

    طےشدہ:
        سیٹ_cf(regs, 1);
    }

    واپس;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

قسمتعریف u8 tpm_ppi_رمز;

ساکن صحیح
tpm12_read_has_owner(صحیح *has_owner)
{
    ساخت tpm_res_getcap_ownerauth oauth;
    صحیح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, حجم(oauth));
    اگر (ret)
        واپس -1;

    *has_owner = oauth.flag;

    واپس 0;
}

ساکن صحیح
tpm12_enable_tpm(صحیح enable, صحیح verbose)
{
    ساخت tpm_permanent_flags pf;
    صحیح ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    اگر (ret)
        واپس -1;

    اگر (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        واپس 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    اگر (ret) {
        اگر (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        ورنہ
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    واپس ret;
}

ساکن صحیح
tpm12_activate_tpm(صحیح activate, صحیح allow_reset, صحیح verbose)
{
    ساخت tpm_permanent_flags pf;
    صحیح ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    اگر (ret)
        واپس -1;

    اگر (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        واپس 0;

    اگر (pf.flags[PERM_FLAG_IDX_DISABLE])
        واپس 0;

    ret = tpm_simple_cmd(0, TPM_ORD_Physical_سیٹ_Deactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    اگر (ret)
        واپس ret;

    اگر (activate && allow_reset) {
        اگر (verbose) {
            لکھو("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    واپس 0;
}

ساکن صحیح
tpm12_enable_activate(صحیح allow_reset, صحیح verbose)
{
    صحیح ret = tpm12_enable_tpm(1, verbose);
    اگر (ret)
        واپس ret;

    واپس tpm12_activate_tpm(1, allow_reset, verbose);
}

ساکن صحیح
tpm12_force_clear(صحیح enable_activate_before, صحیح enable_activate_after,
                  صحیح verbose)
{
    صحیح has_owner;
    صحیح ret = tpm12_read_has_owner(&has_owner);
    اگر (ret)
        واپس -1;
    اگر (!has_owner) {
        اگر (verbose)
            لکھو("TPM does not have an owner.\n");
        واپس 0;
    }

    اگر (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        اگر (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            واپس ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    اگر (ret)
        واپس ret;

    اگر (!enable_activate_after) {
        اگر (verbose)
            لکھو("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        واپس 0;
    }

    واپس tpm12_enable_activate(1, verbose);
}

ساکن صحیح
tpm12_سیٹ_owner_install(صحیح allow, صحیح verbose)
{
    صحیح has_owner;
    صحیح ret = tpm12_read_has_owner(&has_owner);
    اگر (ret)
        واپس -1;
    اگر (has_owner) {
        اگر (verbose)
            لکھو("Must first remove owner.\n");
        واپس 0;
    }

    ساخت tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    اگر (ret)
        واپس -1;

    اگر (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        اگر (verbose)
            لکھو("TPM must first be enable.\n");
        واپس 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_سیٹ_Owner_Install,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    اگر (ret)
        واپس ret;

    اگر (verbose)
        لکھو("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    واپس 0;
}

ساکن صحیح
tpm12_process_cfg(tpm_ppi_رمز پیغام_رمز, صحیح verbose)
{
    صحیح ret = 0;

    انتخاب (پیغام_رمز) {
        حالت TPM_PPI_OP_NOOP: /* no-op */
            توڑو;

        حالت TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            توڑو;

        حالت TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            توڑو;

        حالت TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            توڑو;

        حالت TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            توڑو;

        حالت TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            توڑو;

        حالت TPM_PPI_OP_سیٹ_OWNERINSTALL_TRUE:
            ret = tpm12_سیٹ_owner_install(1, verbose);
            توڑو;

        حالت TPM_PPI_OP_سیٹ_OWNERINSTALL_FALSE:
            ret = tpm12_سیٹ_owner_install(0, verbose);
            توڑو;

        طےشدہ:
            توڑو;
    }

    اگر (ret)
        لکھو("Op %d: An error occurred: 0x%x\n", پیغام_رمز, ret);

    واپس ret;
}

ساکن صحیح
tpm20_clearcontrol(u8 disable, صحیح verbose)
{
    ساخت tpm2_req_clearcontrol trc = {
        .hdr.لیبل     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_مقصد_be32(حجم(trc)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_مقصد_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_مقصد_be32(حجم(trc.authblock)),
        .authblock = {
            .handle = cpu_مقصد_be32(TPM2_RS_PW),
            .noncesize = cpu_مقصد_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_مقصد_be16(0),
        },
        .disable = disable,
    };
    ساخت tpm_rsp_سرخی rsp;
    u32 resp_لمبائی = حجم(rsp);
    صحیح ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_SHORT);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    واپس ret;
}

ساکن صحیح
tpm20_clear(خالی)
{
    ساخت tpm2_req_clear trq = {
        .hdr.لیبل     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_مقصد_be32(حجم(trq)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_Clear),
        .authhandle = cpu_مقصد_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_مقصد_be32(حجم(trq.authblock)),
        .authblock = {
            .handle = cpu_مقصد_be32(TPM2_RS_PW),
            .noncesize = cpu_مقصد_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_مقصد_be16(0),
        },
    };
    ساخت tpm_rsp_سرخی rsp;
    u32 resp_لمبائی = حجم(rsp);
    صحیح ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_لمبائی,
                             TPM_DURATION_TYPE_MEDIUM);
    اگر (ret || resp_لمبائی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    واپس ret;
}

ساکن صحیح
tpm20_process_cfg(tpm_ppi_رمز پیغام_رمز, صحیح verbose)
{
    صحیح ret = 0;

    انتخاب (پیغام_رمز) {
        حالت TPM_PPI_OP_NOOP: /* no-op */
            توڑو;

        حالت TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            اگر (!ret)
                 ret = tpm20_clear();
            توڑو;
    }

    اگر (ret)
        لکھو("Op %d: An error occurred: 0x%x\n", پیغام_رمز, ret);

    واپس ret;
}

ساکن صحیح
tpm12_get_tpm_state(خالی)
{
    صحیح state = 0;
    ساخت tpm_permanent_flags pf;
    صحیح has_owner;

    اگر (tpm12_read_permanent_flags((حرف *)&pf, حجم(pf)) ||
        tpm12_read_has_owner(&has_owner))
        واپس ~0;

    اگر (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    اگر (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_فعال;

    اگر (has_owner) {
        state |= TPM_STATE_OWNED;
    } ورنہ {
        اگر (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    واپس state;
}

ساکن خالی
tpm12_show_tpm_menu(صحیح state, صحیح اگلا_scancodes[7])
{
    صحیح i = 0;

    لکھو("\nThe current state of the TPM is:\n");

    اگر (state & TPM_STATE_ENABLED)
        لکھو("  Enabled");
    ورنہ
        لکھو("  Disabled");

    اگر (state & TPM_STATE_فعال)
        لکھو(" and active\n");
    ورنہ
        لکھو(" and deactivated\n");

    اگر (state & TPM_STATE_OWNED)
        لکھو("  Ownership has been taken\n");
    ورنہ {
        لکھو("  Ownership has not been taken\n");
        اگر (state & TPM_STATE_OWNERINSTALL)
            لکھو("  A user can take ownership of the TPM\n");
        ورنہ
            لکھو("  Taking ownership of the TPM has been disabled\n");
    }

    اگر ((state & (TPM_STATE_ENABLED | TPM_STATE_فعال)) !=
        (TPM_STATE_ENABLED | TPM_STATE_فعال)) {
        لکھو("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    لکھو("\nAvailable options are:\n");
    اگر (state & TPM_STATE_ENABLED) {
        لکھو(" d. Disable the TPM\n");
        اگلا_scancodes[i++] = 32;

        اگر (state & TPM_STATE_فعال) {
            لکھو(" v. Deactivate the TPM\n");
            اگلا_scancodes[i++] = 47;

            اگر (state & TPM_STATE_OWNERINSTALL) {
                لکھو(" p. Prevent installation of an owner\n");
                اگلا_scancodes[i++] = 25;
            } ورنہ {
                لکھو(" s. Allow installation of an owner\n");
                اگلا_scancodes[i++] = 31;
            }
        } ورنہ {
            لکھو(" a. Activate the TPM\n");
            اگلا_scancodes[i++] = 30;
        }

    } ورنہ {
        لکھو(" e. Enable the TPM\n");
        اگلا_scancodes[i++] = 18;
    }

    اگر (state & TPM_STATE_OWNED) {
        لکھو(" c. Clear ownership\n");
        اگلا_scancodes[i++] = 46;
    }

    اگلا_scancodes[i++] = 0;
}

ساکن خالی
tpm12_menu(خالی)
{
    صحیح scancode, اگلا_scancodes[7];
    tpm_ppi_رمز پیغام_رمز;
    صحیح state = 0, i;
    صحیح waitkey;

    لکھو("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    برائے (;;) {
        اگر ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, اگلا_scancodes);
        } ورنہ {
            لکھو("TPM is not working correctly.\n");
            واپس;
        }

        لکھو("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        پیغام_رمز = TPM_PPI_OP_NOOP;

        waitkey = 1;

        جب (waitkey) {
            جب ((scancode = get_keystroke(1000)) == ~0)
                ;

            انتخاب (scancode) {
            حالت 1:
                // ESC
                reset();
                توڑو;
            حالت 18: /* e. enable */
                پیغام_رمز = TPM_PPI_OP_ENABLE;
                توڑو;
            حالت 32: /* d. disable */
                پیغام_رمز = TPM_PPI_OP_DISABLE;
                توڑو;
            حالت 30: /* a. activate */
                پیغام_رمز = TPM_PPI_OP_ACTIVATE;
                توڑو;
            حالت 47: /* v. deactivate */
                پیغام_رمز = TPM_PPI_OP_DEACTIVATE;
                توڑو;
            حالت 46: /* c. clear owner */
                پیغام_رمز = TPM_PPI_OP_CLEAR;
                توڑو;
            حالت 25: /* p. prevent ownerinstall */
                پیغام_رمز = TPM_PPI_OP_سیٹ_OWNERINSTALL_FALSE;
                توڑو;
            حالت 31: /* s. allow ownerinstall */
                پیغام_رمز = TPM_PPI_OP_سیٹ_OWNERINSTALL_TRUE;
                توڑو;
            طےشدہ:
                جاری;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            برائے (i = 0; i < حجم(اگلا_scancodes); i++) {
                اگر (اگلا_scancodes[i] == 0)
                    توڑو;

                اگر (اگلا_scancodes[i] == scancode) {
                    tpm12_process_cfg(پیغام_رمز, 1);
                    waitkey = 0;
                    توڑو;
                }
            }
        }
    }
}

ساکن صحیح
tpm20_menu_change_فعال_pcrbanks(خالی)
{
    u8 فعال_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &فعال_banks);

    u8 activate_banks = فعال_banks;

    جب (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        لکھو("\nToggle active PCR banks by pressing number key\n\n");

        جب (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ثابت حرف *hashname = tpm20_hashalg_flag_مقصد_name(flag);

            i++;
            اگر (hashname) {
                لکھو("  %d: %s", i, hashname);
                اگر (activate_banks & hashalg_flag)
                    لکھو(" (enabled)");
                لکھو("\n");
            }

            hashalg_flag <<= 1;
        }
        لکھو("\n"
               "ESC: return to previous menu without changes\n");
        اگر (activate_banks)
            لکھو("A  : activate selection\n");

        u8 flagnum;
        صحیح show = 0;
        جب (!show) {
            صحیح scancode = get_keystroke(1000);

            انتخاب (scancode) {
            حالت ~0:
                جاری;
            حالت 1: /* ESC */
                لکھو("\n");
                واپس -1;
            حالت 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                اگر (flagnum > i)
                    جاری;
                اگر (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                توڑو;
            حالت 30: /* a */
                اگر (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ساکن خالی
tpm20_menu(خالی)
{
    صحیح جائزہ_رمز;
    tpm_ppi_رمز پیغام_رمز;

    برائے (;;) {
        لکھو("1. Clear TPM\n");
        لکھو("2. Change active PCR banks\n");

        لکھو("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        پیغام_رمز = TPM_PPI_OP_NOOP;

        جب ((جائزہ_رمز = get_keystroke(1000)) == ~0)
            ;

        انتخاب (جائزہ_رمز) {
        حالت 1:
            // ESC
            reset();
            توڑو;
        حالت 2:
            پیغام_رمز = TPM_PPI_OP_CLEAR;
            توڑو;
        حالت 3:
            tpm20_menu_change_فعال_pcrbanks();
            جاری;
        طےشدہ:
            جاری;
        }

        tpm20_process_cfg(پیغام_رمز, 0);
    }
}

خالی
tpm_menu(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس;

    جب (get_keystroke(0) >= 0)
        ;
    wait_دھاگے();

    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        tpm12_menu();
        توڑو;
    حالت TPM_نسخہ_2:
        tpm20_menu();
        توڑو;
    }
}

صحیح
tpm_can_show_menu(خالی)
{
    انتخاب (TPM_نسخہ) {
    حالت TPM_نسخہ_1_2:
        واپس tpm_is_working() && TPM_has_physical_presence;
    حالت TPM_نسخہ_2:
        واپس tpm_is_working();
    }
    واپس 0;
}
