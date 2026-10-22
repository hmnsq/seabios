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

#ضمن "bregs.h" // struct bregs
#ضمن "byteorder.h" // cpu_to_*
#ضمن "config.h" // CONFIG_TCGBIOS
#ضمن "farptr.h" // MAKE_FLATPTR
#ضمن "fw/paravirt.h" // runningOnXen
#ضمن "hw/tpm_drivers.h" // tpm_drivers[]
#ضمن "output.h" // dprintf
#ضمن "sha.h" // sha1, sha256, ...
#ضمن "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#ضمن "std/smbios.h" // struct smbios_21_entry_point
#ضمن "std/tcg.h" // TCG_PC_LOGOVERFLOW
#ضمن "string.h" // checksum
#ضمن "tcgbios.h"// tpm_*, prototypes
#ضمن "util.h" // printf, get_keystroke
#ضمن "stacks.h" // wait_threads, reset
#ضمن "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

بنية {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_درازی;

    /* start address of TCPA log buffer */
    u8 *          log_area_شروع_آدرس;

    /* number of log entries written */
    u32           مدخل_شمار;

    /* address to write next log entry to */
    u8 *          log_area_بعدی_مدخل;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_آخرین_مدخل;
} tpm_state VARLOW;

ساكن صحيح tpm_مجموعه_log_area(u8 *log_area_شروع_آدرس,
                            u32 log_area_minimum_درازی)
{
    إذا (!log_area_شروع_آدرس || !log_area_minimum_درازی)
        أرجع -1;

    memset(log_area_شروع_آدرس, 0, log_area_minimum_درازی);
    tpm_state.log_area_شروع_آدرس = log_area_شروع_آدرس;
    tpm_state.log_area_minimum_درازی = log_area_minimum_درازی;
    tpm_state.log_area_بعدی_مدخل = log_area_شروع_آدرس;
    tpm_state.log_area_آخرین_مدخل = عدم;
    tpm_state.مدخل_شمار = 0;
    أرجع 0;
}

ساكن صحيح
tpm_tcpa_probe(فراغ)
{
    بنية tcpa_descriptor_rev2 *tcpa = پیدا_کن_acpi_جدول(TCPA_SIGNATURE);
    إذا (!tcpa)
        أرجع -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(دراز)tcpa->log_area_شروع_آدرس,
            tcpa->log_area_minimum_درازی);

    أرجع tpm_مجموعه_log_area((u8*)(دراز)tcpa->log_area_شروع_آدرس,
                            tcpa->log_area_minimum_درازی);
}

ساكن صحيح
tpm_tpm2_probe(فراغ)
{
    بنية tpm2_descriptor_rev2 *tpm2 = پیدا_کن_acpi_جدول(TPM2_SIGNATURE);
    إذا (!tpm2)
        أرجع -1;

    إذا (tpm2->درازی < 76)
        أرجع -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(دراز)tpm2->log_area_شروع_آدرس,
            tpm2->log_area_minimum_درازی);

    أرجع tpm_مجموعه_log_area((u8*)(دراز)tpm2->log_area_شروع_آدرس,
                            tpm2->log_area_minimum_درازی);
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
ساكن صحيح
tpm_log_event(بنية tpm_log_سرآیند *مدخل, صحيح digest_len
              , ثابت فراغ *event, صحيح event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_شروع_آدرس, tpm_state.log_area_بعدی_مدخل);

    إذا (tpm_state.log_area_بعدی_مدخل == عدم)
        أرجع -1;

    u32 اندازه = (حجم(*مدخل) + digest_len
                + حجم(بنية tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_بعدی_مدخل + اندازه
                   - tpm_state.log_area_شروع_آدرس);
    إذا (logsize > tpm_state.log_area_minimum_درازی) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", اندازه);
        أرجع -1;
    }

    فراغ *dest = tpm_state.log_area_بعدی_مدخل;
    کپی_حافظه(dest, مدخل, حجم(*مدخل) + digest_len);
    بنية tpm_log_trailer *t = dest + حجم(*مدخل) + digest_len;
    t->eventdatasize = event_len;
    کپی_حافظه(t->event, event, event_len);

    tpm_state.log_area_آخرین_مدخل = tpm_state.log_area_بعدی_مدخل;
    tpm_state.log_area_بعدی_مدخل += اندازه;
    tpm_state.مدخل_شمار++;

    أرجع 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ساكن TPMVersion TPM_نسخه;
ساكن u32 tpm20_pcr_selection_اندازه;
ساكن بنية tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
بنية tpm_log_مدخل {
    بنية tpm_log_سرآیند hdr;
    u8 pad[حجم(بنية tpm2_digest_مقادیر)
           + 8 * حجم(بنية tpm2_digest_مقدار)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ساكن ثابت بنية hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ثابت حرف *name;
    فراغ (*hashfunc)(ثابت u8 *داده, u32 درازی, u8 *hash);
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

ساكن صحيح
tpm20_get_hash_buffersize(u16 hashAlg)
{
    بیعلامت i;

    لكل (i = 0; i < ARRAY_اندازه(hash_parameters); i++) {
        إذا (hash_parameters[i].hashalg == hashAlg)
            أرجع hash_parameters[i].hash_buffersize;
    }
    أرجع -1;
}

ساكن u8
tpm20_hashalg_مقصد_flag(u16 hashAlg)
{
    بیعلامت i;

    لكل (i = 0; i < ARRAY_اندازه(hash_parameters); i++) {
        إذا (hash_parameters[i].hashalg == hashAlg)
            أرجع hash_parameters[i].hashalg_flag;
    }
    أرجع 0;
}

ساكن u16
tpm20_hashalg_flag_مقصد_hashalg(u8 hashalg_flag)
{
    بیعلامت i;

    لكل (i = 0; i < ARRAY_اندازه(hash_parameters); i++) {
        إذا (hash_parameters[i].hashalg_flag == hashalg_flag)
            أرجع hash_parameters[i].hashalg;
    }
    أرجع 0;
}

ساكن ثابت حرف *
tpm20_hashalg_flag_مقصد_name(u8 hashalg_flag)
{
    بیعلامت i;

    لكل (i = 0; i < ARRAY_اندازه(hash_parameters); i++) {
        إذا (hash_parameters[i].hashalg_flag == hashalg_flag)
            أرجع hash_parameters[i].name;
    }
    أرجع عدم;
}

ساكن فراغ tpm2_hash_داده(u16 hashAlg, ثابت u8 *داده, u32 داده_len, u8 *hash)
{
    بیعلامت i;

    لكل (i = 0; i < ARRAY_اندازه(hash_parameters); i++) {
        إذا (hash_parameters[i].hashalg == hashAlg) {
            إذا (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(داده, داده_len, hash);
            } وإلا {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ساكن صحيح
tpm20_write_EfiSpecIdEventStruct(فراغ)
{
    إذا (!tpm20_pcr_selection)
        أرجع -1;

    بنية {
        بنية TCG_EfiSpecIdEventStruct hdr;
        u8 pad[حجم(بنية tpm_log_مدخل) + حجم(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.spec_نسخه_Minor = 0,
        .hdr.spec_نسخه_Major = 2,
        .hdr.specErrata = 2,
        .hdr.uintn_اندازه = 2,
    };

    بنية tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    فراغ *nsel, *انتها = (فراغ*)tpm20_pcr_selection + tpm20_pcr_selection_اندازه;

    u32 شمار, رقم_Algs = 0;
    لكل (شمار = 0; شمار < be32_مقصد_cpu(tpm20_pcr_selection->شمار); شمار++) {
        u8 اندازه_از_Select = sel->اندازه_از_Select;

        nsel = (فراغ*)sel + حجم(*sel) + اندازه_از_Select;
        إذا (nsel > انتها)
            اكسر;

        إذا (!اندازه_از_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            استمر;
        }

        صحيح hsize = tpm20_get_hash_buffersize(be16_مقصد_cpu(sel->hashAlg));
        إذا (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_مقصد_cpu(sel->hashAlg));
            أرجع -1;
        }

        صحيح event_اندازه = offsetof(بنية TCG_EfiSpecIdEventStruct
                                  , digestSizes[شمار+1]);
        إذا (event_اندازه > حجم(event) - حجم(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            أرجع -1;
        }

        event.hdr.digestSizes[رقم_Algs].algorithmId = be16_مقصد_cpu(sel->hashAlg);
        event.hdr.digestSizes[رقم_Algs].digest_اندازه = hsize;
        رقم_Algs++;

        sel = nsel;
    }

    إذا (sel != انتها) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        أرجع -1;
    }

    event.hdr.عدد_از_Algorithms = رقم_Algs;
    صحيح event_اندازه = offsetof(بنية TCG_EfiSpecIdEventStruct
                              , digestSizes[رقم_Algs]);
    u8 *vendor_Info_اندازه = (فراغ*)&event + event_اندازه;
    *vendor_Info_اندازه = 0;
    event_اندازه += حجم(*vendor_Info_اندازه);

    بنية tpm_log_مدخل le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    أرجع tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_اندازه);
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
ساكن صحيح
tpm20_build_digest(بنية tpm_log_مدخل *le,
                   ثابت u8 *hashdata, u32 hashdata_len, صحيح bigEndian)
{
    إذا (!tpm20_pcr_selection)
        أرجع -1;

    بنية tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    فراغ *nsel, *انتها = (فراغ*)tpm20_pcr_selection + tpm20_pcr_selection_اندازه;
    فراغ *dest = le->hdr.digest + حجم(بنية tpm2_digest_مقادیر);

    u32 شمار, رقم_Algs = 0;
    لكل (شمار = 0; شمار < be32_مقصد_cpu(tpm20_pcr_selection->شمار); شمار++) {
        u8 اندازه_از_Select = sel->اندازه_از_Select;

        nsel = (فراغ*)sel + حجم(*sel) + اندازه_از_Select;
        إذا (nsel > انتها)
            اكسر;

        /* PCR 0-7 unused? -- skip */
        إذا (!اندازه_از_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            استمر;
        }

        صحيح hsize = tpm20_get_hash_buffersize(be16_مقصد_cpu(sel->hashAlg));
        إذا (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_مقصد_cpu(sel->hashAlg));
            أرجع -1;
        }

        /* buffer size sanity check before writing */
        بنية tpm2_digest_مقدار *v = dest;
        إذا (dest + حجم(*v) + hsize > (فراغ*)le + حجم(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            أرجع -1;
        }

        إذا (bigEndian)
            v->hashAlg = sel->hashAlg;
        وإلا
            v->hashAlg = be16_مقصد_cpu(sel->hashAlg);

        tpm2_hash_داده(be16_مقصد_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += حجم(*v) + hsize;
        sel = nsel;

        رقم_Algs++;
    }

    إذا (sel != انتها) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        أرجع -1;
    }

    بنية tpm2_digest_مقادیر *v = (فراغ*)le->hdr.digest;
    إذا (bigEndian)
        v->شمار = cpu_مقصد_be32(رقم_Algs);
    وإلا
        v->شمار = رقم_Algs;

    أرجع dest - (فراغ*)le->hdr.digest;
}

ساكن صحيح
tpm12_build_digest(بنية tpm_log_مدخل *le,
                   ثابت u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    أرجع SHA1_BUFSIZE;
}

ساكن صحيح
tpm12_build_digest_مستقیم(بنية tpm_log_مدخل *le, ثابت u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    کپی_حافظه(le->hdr.digest, sha1, SHA1_BUFSIZE);
    أرجع SHA1_BUFSIZE;
}

ساكن صحيح
tpm_build_digest(بنية tpm_log_مدخل *le, ثابت u8 *hashdata, u32 hashdata_len
                 , صحيح bigEndian)
{
    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        أرجع tpm12_build_digest(le, hashdata, hashdata_len);
    حالة TPM_نسخه_2:
        أرجع tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    أرجع -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ساكن صحيح
tpm_simple_cmd(u8 locty, u32 ordinal
               , صحيح param_اندازه, u16 param, تعداد tpmDurationType مقصد_t)
{
    بنية {
        بنية tpm_req_سرآیند trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_مقصد_be32(حجم(req.trqh) + param_اندازه),
        .trqh.ordinal = cpu_مقصد_be32(ordinal),
        .param = param_اندازه == 2 ? cpu_مقصد_be16(param) : param,
    };
    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        req.trqh.برچسب = cpu_مقصد_be16(TPM_برچسب_RQU_CMD);
        اكسر;
    حالة TPM_نسخه_2:
        req.trqh.برچسب = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS);
        اكسر;
    }

    u8 obuffer[64];
    بنية tpm_rsp_سرآیند *trsh = (فراغ*)obuffer;
    u32 obuffer_len = حجم(obuffer);
    memset(obuffer, 0x0, حجم(obuffer));

    صحيح ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, مقصد_t);
    ret = ret ? -1 : be32_مقصد_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    أرجع ret;
}

ساكن صحيح
tpm20_getcapability(u32 capability, u32 property, u32 شمار,
                    بنية tpm_rsp_سرآیند *rsp, u32 rsize)
{
    بنية tpm2_req_getcapability trg = {
        .hdr.برچسب = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(trg)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_GetCapability),
        .capability = cpu_مقصد_be32(capability),
        .property = cpu_مقصد_be32(property),
        .propertycount = cpu_مقصد_be32(شمار),
    };

    u32 resp_اندازه = rsize;
    صحيح ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_اندازه,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_مقصد_cpu(rsp->totlen)) ? -1 : be32_مقصد_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    أرجع ret;
}

ساكن صحيح
tpm20_get_pcrbanks(فراغ)
{
    u8 میانگیر[128];
    بنية tpm2_res_getcapability *trg =
      (بنية tpm2_res_getcapability *)&میانگیر;

    صحيح ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  حجم(میانگیر));
    إذا (ret)
        أرجع ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_مقصد_cpu(trg->hdr.totlen);
    إذا (resplen <= offsetof(بنية tpm2_res_getcapability, داده))
        أرجع -1;

    u32 اندازه = resplen - offsetof(بنية tpm2_res_getcapability, داده);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    إذا (اندازه < offsetof(بنية tpml_pcr_selection, selections) +
               offsetof(بنية tpms_pcr_selection, pcrSelect))
        أرجع -1;

    tpm20_pcr_selection = malloc_بالا(اندازه);
    إذا (tpm20_pcr_selection) {
        کپی_حافظه(tpm20_pcr_selection, &trg->داده, اندازه);
        tpm20_pcr_selection_اندازه = اندازه;
    } وإلا {
        warn_noalloc();
        ret = -1;
    }

    أرجع ret;
}

ساكن صحيح
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *فعال_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *فعال_pcrbanks = 0;

    إذا (!tpm20_pcr_selection)
        أرجع -1;

    بنية tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    فراغ *انتها = (فراغ*)tpm20_pcr_selection + tpm20_pcr_selection_اندازه;

    طالما (1) {
        u8 اندازه_از_Select = sel->اندازه_از_Select;
        فراغ *nsel = (فراغ*)sel + حجم(*sel) + اندازه_از_Select;
        إذا (nsel > انتها)
            أرجع 0;

        u16 hashalg = be16_مقصد_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_مقصد_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        بیعلامت i;
        لكل (i = 0; i < اندازه_از_Select; i++) {
            إذا (sel->pcrSelect[i]) {
                *فعال_pcrbanks |= hashalg_flag;
                اكسر;
            }
        }

        sel = nsel;
    }
}

ساكن صحيح
tpm20_مجموعه_pcrbanks(u32 فعال_banks)
{
    بنية tpm2_req_pcr_allocate trpa = {
        .hdr.برچسب     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
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
    بنية tpms_pcr_selection3 {
        u16 hashAlg;
        u8 اندازه_از_Select;
        u8 pcrSelect[3];
    } tps[ARRAY_اندازه(trpa.tpms_pcr_selections)];
    صحيح i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    طالما (hashalg_flag) {
        إذا ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_مقصد_hashalg(hashalg_flag);

            إذا (hashalg) {
                u8 ماسک = 0;
                tps[i].hashAlg = cpu_مقصد_be16(hashalg);
                tps[i].اندازه_از_Select = 3;

                إذا (فعال_banks & hashalg_flag)
                    ماسک = 0xff;

                tps[i].pcrSelect[0] = ماسک;
                tps[i].pcrSelect[1] = ماسک;
                tps[i].pcrSelect[2] = ماسک;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.شمار = cpu_مقصد_be32(i);
    کپی_حافظه(trpa.tpms_pcr_selections, tps, i * حجم(tps[0]));
    trpa.hdr.totlen = cpu_مقصد_be32(offsetof(بنية tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * حجم(tps[0]));

    بنية tpm_rsp_سرآیند rsp;
    u32 resp_درازی = حجم(rsp);

    صحيح ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_مقصد_cpu(rsp.errcode);

    أرجع ret;
}

ساكن صحيح tpm20_activate_pcrbanks(u32 فعال_banks)
{
    صحيح ret = tpm20_مجموعه_pcrbanks(فعال_banks);
    إذا (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    إذا (!ret)
        reset();
    أرجع ret;
}

ساكن صحيح
tpm12_get_capability(u32 cap, u32 subcap, بنية tpm_rsp_سرآیند *rsp, u32 rsize)
{
    بنية tpm_req_getcap trgc = {
        .hdr.برچسب = cpu_مقصد_be16(TPM_برچسب_RQU_CMD),
        .hdr.totlen = cpu_مقصد_be32(حجم(trgc)),
        .hdr.ordinal = cpu_مقصد_be32(TPM_ORD_GetCapability),
        .capArea = cpu_مقصد_be32(cap),
        .sub_Cap_اندازه = cpu_مقصد_be32(حجم(trgc.subCap)),
        .subCap = cpu_مقصد_be32(subcap)
    };
    u32 resp_اندازه = rsize;
    صحيح ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_اندازه,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_اندازه != rsize) ? -1 : be32_مقصد_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    أرجع ret;
}

ساكن صحيح
tpm12_read_permanent_flags(حرف *buf, صحيح buf_len)
{
    memset(buf, 0, buf_len);

    بنية tpm_res_getcap_perm_flags pf;
    صحيح ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, حجم(pf));
    إذا (ret)
        أرجع -1;

    کپی_حافظه(buf, &pf.perm_flags, buf_len);

    أرجع 0;
}

ساكن صحيح
tpm12_determine_timeouts(فراغ)
{
    بنية tpm_res_getcap_timeouts timeouts;
    صحيح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, حجم(timeouts));
    إذا (ret)
        أرجع ret;

    بنية tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, حجم(durations));
    إذا (ret)
        أرجع ret;

    صحيح i;
    لكل (i = 0; i < 3; i++)
        durations.durations[i] = be32_مقصد_cpu(durations.durations[i]);

    لكل (i = 0; i < 4; i++)
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

    tpmhw_مجموعه_timeouts(timeouts.timeouts, durations.durations);

    أرجع 0;
}

ساكن فراغ
tpm20_مجموعه_timeouts(فراغ)
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

    tpmhw_مجموعه_timeouts(timeouts, durations);
}

ساكن صحيح
tpm12_extend(بنية tpm_log_مدخل *le, صحيح digest_len)
{
    بنية tpm_req_extend tre = {
        .hdr.برچسب     = cpu_مقصد_be16(TPM_برچسب_RQU_CMD),
        .hdr.totlen  = cpu_مقصد_be32(حجم(tre)),
        .hdr.ordinal = cpu_مقصد_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_مقصد_be32(le->hdr.pcrindex),
    };
    کپی_حافظه(tre.digest, le->hdr.digest, حجم(tre.digest));

    بنية tpm_rsp_extend rsp;
    u32 resp_درازی = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_SHORT);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.hdr.errcode)
        أرجع -1;

    أرجع 0;
}

ساكن صحيح tpm20_extend(بنية tpm_log_مدخل *le, صحيح digest_len)
{
    بنية tpm2_req_extend tmp_tre = {
        .hdr.برچسب     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
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
    u8 میانگیر[حجم(tmp_tre) + حجم(le->pad)];
    بنية tpm2_req_extend *tre = (بنية tpm2_req_extend *)میانگیر;

    کپی_حافظه(tre, &tmp_tre, حجم(tmp_tre));
    کپی_حافظه(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_مقصد_be32(حجم(tmp_tre) + digest_len);

    بنية tpm_rsp_سرآیند rsp;
    u32 resp_درازی = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_SHORT);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.errcode)
        أرجع -1;

    أرجع 0;
}

ساكن صحيح
tpm_extend(بنية tpm_log_مدخل *le, صحيح digest_len)
{
    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        أرجع tpm12_extend(le, digest_len);
    حالة TPM_نسخه_2:
        أرجع tpm20_extend(le, digest_len);
    }
    أرجع -1;
}

ساكن صحيح
tpm20_stirrandom(فراغ)
{
    بنية tpm2_req_stirrandom stir = {
        .hdr.برچسب = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(stir)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_StirRandom),
        .اندازه = cpu_مقصد_be16(حجم(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    بنية tpm_rsp_سرآیند rsp;
    u32 resp_درازی = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_SHORT);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    أرجع ret;
}

ساكن صحيح
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    بنية tpm2_res_getrandom rsp;

    إذا (buf_len > حجم(rsp.rnd.میانگیر))
        أرجع -1;

    بنية tpm2_req_getrandom trgr = {
        .hdr.برچسب = cpu_مقصد_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_مقصد_be32(حجم(trgr)),
        .hdr.ordinal = cpu_مقصد_be32(TPM2_CC_GetRandom),
        .بایتها_Requested = cpu_مقصد_be16(buf_len),
    };
    u32 resp_درازی = حجم(rsp);

    صحيح ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_MEDIUM);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.hdr.errcode)
        ret = -1;
    وإلا
        کپی_حافظه(buf, rsp.rnd.میانگیر, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    أرجع ret;
}

ساكن صحيح
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    بنية tpm2_req_hierarchycontrol trh = {
        .hdr.برچسب = cpu_مقصد_be16(TPM2_ST_SESSIONS),
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
    بنية tpm_rsp_سرآیند rsp;
    u32 resp_درازی = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_MEDIUM);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    أرجع ret;
}

ساكن صحيح
tpm20_hierarchychangeauth(u8 auth[20])
{
    بنية tpm2_req_hierarchychangeauth trhca = {
        .hdr.برچسب = cpu_مقصد_be16(TPM2_ST_SESSIONS),
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
            .اندازه = cpu_مقصد_be16(حجم(trhca.newAuth.میانگیر)),
        },
    };
    کپی_حافظه(trhca.newAuth.میانگیر, auth, حجم(trhca.newAuth.میانگیر));

    بنية tpm_rsp_سرآیند rsp;
    u32 resp_درازی = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_MEDIUM);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    أرجع ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ساكن صحيح TPM_has_physical_presence;
u8 TPM_working VARLOW;

ساكن صحيح
tpm_is_working(فراغ)
{
    أرجع CONFIG_TCGBIOS && TPM_working;
}

ساكن فراغ
tpm_مجموعه_failure(فراغ)
{
    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_مجموعه_موقت_Deactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        اكسر;
    حالة TPM_نسخه_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        اكسر;
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
ساكن فراغ
tpm_جمع_کن_measurement_مقصد_log(u32 pcrindex, u32 event_type,
                           ثابت حرف *event, u32 event_درازی,
                           ثابت u8 *hashdata, u32 hashdata_درازی)
{
    إذا (!tpm_is_working())
        أرجع;

    بنية tpm_log_مدخل le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    صحيح digest_len = tpm_build_digest(&le, hashdata, hashdata_درازی, 1);
    إذا (digest_len < 0)
        أرجع;
    صحيح ret = tpm_extend(&le, digest_len);
    إذا (ret) {
        tpm_مجموعه_failure();
        أرجع;
    }
    tpm_build_digest(&le, hashdata, hashdata_درازی, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_درازی);
}

// Add an EV_ACTION measurement to the list of measurements
ساكن فراغ
tpm_جمع_کن_action(u32 pcr_شاخص, ثابت حرف *string)
{
    u32 len = طول_رشته(string);
    tpm_جمع_کن_measurement_مقصد_log(pcr_شاخص, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ساكن فراغ
tpm_جمع_کن_event_separators(فراغ)
{
    ساكن ثابت u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcr_شاخص;
    لكل (pcr_شاخص = 0; pcr_شاخص <= 7; pcr_شاخص++)
        tpm_جمع_کن_measurement_مقصد_log(pcr_شاخص, EV_SEPARATOR,
                                   (ثابت حرف *)evt_separator,
                                   حجم(evt_separator),
                                   evt_separator,
                                   حجم(evt_separator));
}

ساكن فراغ
tpm_smbios_measure(فراغ)
{
    بنية pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    فراغ *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    إذا (!smbios_tables)
        أرجع;

    sha1((ثابت u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_جمع_کن_measurement_مقصد_log(1,
                               EV_EVENT_برچسب,
                               (ثابت حرف *)&pcctes, حجم(pcctes),
                               (u8 *)&pcctes, حجم(pcctes));
}

ساكن صحيح
tpm12_assert_physical_presence(فراغ)
{
    صحيح ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    إذا (!ret)
        أرجع 0;

    بنية tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    إذا (ret)
        أرجع -1;

    /* check if hardware physical presence is supported */
    إذا (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        أرجع 0;
    }

    إذا (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_قفل]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        أرجع tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    أرجع -1;
}

ساكن صحيح
tpm12_startup(فراغ)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    صحيح ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    إذا (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    إذا (ret)
        اذهب err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    إذا (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    إذا (ret)
        اذهب err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_Self_Test_پر,
                         0, 0, TPM_DURATION_TYPE_LONG);
    إذا (ret)
        اذهب err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_Reset_Establishment_بیت,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    إذا (ret && ret != TPM_BAD_LOCALITY)
        اذهب err_exit;

    أرجع 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_مجموعه_failure();
    أرجع -1;
}

ساكن صحيح
tpm20_startup(فراغ)
{
    tpm20_مجموعه_timeouts();

    صحيح ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    إذا (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    إذا (ret)
        اذهب err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    إذا (ret)
        اذهب err_exit;

    ret = tpm20_get_pcrbanks();
    إذا (ret)
        اذهب err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    إذا (ret)
        اذهب err_exit;

    أرجع 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_مجموعه_failure();
    أرجع -1;
}

ساكن صحيح
tpm_startup(فراغ)
{
    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        أرجع tpm12_startup();
    حالة TPM_نسخه_2:
        أرجع tpm20_startup();
    }
    أرجع -1;
}

فراغ
tpm_راهاندازی(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع;

    صحيح ret = tpm_tpm2_probe();
    إذا (ret) {
        ret = tpm_tcpa_probe();
        إذا (ret)
            أرجع;
    }

    TPM_نسخه = tpmhw_probe();
    إذا (TPM_نسخه == TPM_نسخه_NONE)
        أرجع;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_نسخه == TPM_نسخه_1_2) ? "1.2" : "2");

    TPM_working = 1;

    إذا (runningOnXen())
        أرجع;

    ret = tpm_startup();
    إذا (ret)
        أرجع;

    tpm_smbios_measure();
    tpm_جمع_کن_action(2, "Start Option ROM Scan");
}

ساكن فراغ
tpm20_prepboot(فراغ)
{
    صحيح ret = tpm20_stirrandom();
    إذا (ret)
         اذهب err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], حجم(auth));
    إذا (ret)
        اذهب err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    إذا (ret)
        اذهب err_exit;

    أرجع;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_مجموعه_failure();
}

فراغ
tpm_prepboot(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع;

    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        إذا (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_قفل, TPM_DURATION_TYPE_SHORT);
        اكسر;
    حالة TPM_نسخه_2:
        tpm20_prepboot();
        اكسر;
    }

    tpm_جمع_کن_action(4, "Calling INT 19h");
    tpm_جمع_کن_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
فراغ
tpm_option_rom(ثابت فراغ *addr, u32 len)
{
    إذا (!tpm_is_working())
        أرجع;

    بنية pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = حجم(u16) + حجم(u16) + SHA1_BUFSIZE,
    };
    sha1((ثابت u8 *)addr, len, pcctes.digest);
    tpm_جمع_کن_measurement_مقصد_log(2,
                               EV_EVENT_برچسب,
                               (ثابت حرف *)&pcctes, حجم(pcctes),
                               (u8 *)&pcctes, حجم(pcctes));
}

فراغ
tpm_جمع_کن_bcv(u32 bootdrv, ثابت u8 *addr, u32 درازی)
{
    إذا (!tpm_is_working())
        أرجع;

    إذا (درازی < 0x200)
        أرجع;

    ثابت حرف *string = "Booting BCV device 00h (Floppy)";
    إذا (bootdrv == 0x80)
        string = "Booting BCV device 80h (HDD)";
    tpm_جمع_کن_action(4, string);

    /* specs: see section 'Hard Disk Device or Hard Disk-Like Devices' */
    /* equivalent to: dd if=/dev/hda ibs=1 count=440 | sha1sum */
    string = "MBR";
    tpm_جمع_کن_measurement_مقصد_log(4, EV_IPL,
                               string, طول_رشته(string),
                               addr, 0x1b8);

    /* equivalent to: dd if=/dev/hda ibs=1 count=72 skip=440 | sha1sum */
    string = "MBR PARTITION_TABLE";
    tpm_جمع_کن_measurement_مقصد_log(5, EV_IPL_افراز_داده,
                               string, طول_رشته(string),
                               addr + 0x1b8, 0x48);
}

فراغ
tpm_جمع_کن_cdrom(u32 bootdrv, ثابت u8 *addr, u32 درازی)
{
    إذا (!tpm_is_working())
        أرجع;

    tpm_جمع_کن_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت حرف *string = "EL TORITO IPL";
    tpm_جمع_کن_measurement_مقصد_log(4, EV_IPL,
                               string, طول_رشته(string),
                               addr, درازی);
}

فراغ
tpm_جمع_کن_cdrom_catalog(ثابت u8 *addr, u32 درازی)
{
    إذا (!tpm_is_working())
        أرجع;

    tpm_جمع_کن_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت حرف *string = "BOOT CATALOG";
    tpm_جمع_کن_measurement_مقصد_log(5, EV_IPL_افراز_داده,
                               string, طول_رشته(string),
                               addr, درازی);
}

فراغ
tpm_s3_resume(فراغ)
{
    إذا (!tpm_is_working())
        أرجع;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    صحيح ret = -1;

    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        اكسر;
    حالة TPM_نسخه_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        إذا (ret)
            اذهب err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        اكسر;
    }

    إذا (ret)
        اذهب err_exit;

    أرجع;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_مجموعه_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ساكن مضمن فراغ *درون_داد_buf32(بنية bregs *regs)
{
    أرجع MAKE_FLATPTR(regs->es, regs->di);
}

ساكن مضمن فراغ *output_buf32(بنية bregs *regs)
{
    أرجع MAKE_FLATPTR(regs->ds, regs->si);
}

ساكن u32
hash_log_extend(بنية pcpes *pcpes, ثابت فراغ *hashdata, u32 hashdata_درازی
                , فراغ *event, صحيح extend)
{
    إذا (pcpes->pcrindex >= 24)
        أرجع TCG_INVALID_درون_داد_PARA;
    إذا (hashdata)
        sha1(hashdata, hashdata_درازی, pcpes->digest);

    بنية tpm_log_مدخل le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    صحيح digest_len = tpm12_build_digest_مستقیم(&le, pcpes->digest);
    إذا (digest_len < 0)
        أرجع TCG_GENERAL_ERROR;
    إذا (extend) {
        صحيح ret = tpm_extend(&le, digest_len);
        إذا (ret)
            أرجع TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_مستقیم(&le, pcpes->digest);
    صحيح ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    إذا (ret)
        أرجع TCG_PC_LOGOVERFLOW;
    أرجع 0;
}

ساكن u32
hash_log_extend_event_int(ثابت بنية hleei_short *hleei_s,
                          بنية hleeo *hleeo)
{
    u32 rc = 0;
    بنية hleei_long *hleei_l = (بنية hleei_long *)hleei_s;
    ثابت فراغ *logdataptr;
    u32 logdatalen;
    بنية pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    بدل (hleei_s->ipblength) {
    حالة حجم(بنية hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    اكسر;

    حالة حجم(بنية hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    اكسر;

    افتراضي:
        /* bad input block */
        rc = TCG_INVALID_درون_داد_PARA;
        اذهب err_exit;
    }

    pcpes = (بنية pcpes *)logdataptr;

    إذا (pcpes->pcrindex != pcrindex
        || logdatalen != حجم(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_درون_داد_PARA;
        اذهب err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    إذا (rc)
        اذهب err_exit;

    hleeo->opblength = حجم(بنية hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.مدخل_شمار;
    کپی_حافظه(hleeo->digest, pcpes->digest, حجم(hleeo->digest));

err_exit:
    إذا (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    أرجع rc;
}

ساكن u32
pass_through_مقصد_tpm_int(بنية pttti *pttti, بنية pttto *pttto)
{
    u32 rc = 0;
    بنية tpm_req_سرآیند *trh = (فراغ*)pttti->tpmopin;

    إذا (pttti->ipblength < حجم(بنية pttti) + حجم(*trh)
        || pttti->ipblength != حجم(بنية pttti) + be32_مقصد_cpu(trh->totlen)
        || pttti->opblength < حجم(بنية pttto)) {
        rc = TCG_INVALID_درون_داد_PARA;
        اذهب err_exit;
    }

    u16 برچسب = be16_مقصد_cpu(trh->برچسب);

    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        إذا (برچسب != TPM_برچسب_RQU_CMD && برچسب != TPM_برچسب_RQU_AUTH1_CMD
            && برچسب != TPM_برچسب_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_درون_داد_PARA;
            اذهب err_exit;
        }
        اكسر;
    حالة TPM_نسخه_2:
        إذا (برچسب != TPM2_ST_NO_SESSIONS && برچسب != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_درون_داد_PARA;
            اذهب err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(بنية pttto, tpmopout);
    صحيح ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    إذا (ret) {
        rc = TCG_FATAL_COM_ERROR;
        اذهب err_exit;
    }

    pttto->opblength = offsetof(بنية pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    إذا (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    أرجع rc;
}

ساكن u32
shutdown_preboot_interface(فراغ)
{
    TPM_interface_shutdown = 1;
    أرجع 0;
}

ساكن u32
hash_log_event_int(ثابت بنية hlei *hlei, بنية hleo *hleo)
{
    u32 rc = 0;
    u16 اندازه;
    بنية pcpes *pcpes;

    اندازه = hlei->ipblength;
    إذا (اندازه != حجم(*hlei)) {
        rc = TCG_INVALID_درون_داد_PARA;
        اذهب err_exit;
    }

    pcpes = (بنية pcpes *)hlei->logdataptr;

    إذا (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != حجم(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_درون_داد_PARA;
        اذهب err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    إذا (rc)
        اذهب err_exit;

    /* updating the log was fine */
    hleo->opblength = حجم(بنية hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.مدخل_شمار;

err_exit:
    إذا (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    أرجع rc;
}

ساكن u32
hash_all_int(ثابت بنية hai *hai, u8 *hash)
{
    إذا (hai->ipblength != حجم(بنية hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        أرجع TCG_INVALID_درون_داد_PARA;

    sha1((ثابت u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    أرجع 0;
}

ساكن u32
tss_int(بنية ti *ti, بنية مقصد *مقصد)
{
    مقصد->opblength = حجم(بنية مقصد);
    مقصد->reserved  = 0;

    أرجع TCG_PC_UNSUPPORTED;
}

ساكن u32
compact_hash_log_extend_event_int(u8 *میانگیر,
                                  u32 info,
                                  u32 درازی,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    بنية pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = حجم(info),
    };
    u32 rc = hash_log_extend(&pcpes, میانگیر, درازی, &info, 1);
    إذا (rc)
        أرجع rc;

    *edx_ptr = tpm_state.مدخل_شمار;
    أرجع 0;
}

فراغ VISIBLE32FLAT
tpm_وقفه_handler32(بنية bregs *regs)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع;

    مجموعه_cf(regs, 0);

    إذا (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        أرجع;
    }

    بدل ((تعداد irq_ids)regs->al) {
    حالة TCG_وضعیت_Check:
        إذا (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } وإلا {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_نسخه_MAJOR;
            regs->cl = TCG_نسخه_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_شروع_آدرس;
            regs->edi = (u32)tpm_state.log_area_آخرین_مدخل;
        }
        اكسر;

    حالة TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (بنية hleei_short *)درون_داد_buf32(regs),
                  (بنية hleeo *)output_buf32(regs));
        اكسر;

    حالة TCG_Pass_Through_مقصد_TPM:
        regs->eax =
            pass_through_مقصد_tpm_int((بنية pttti *)درون_داد_buf32(regs),
                                    (بنية pttto *)output_buf32(regs));
        اكسر;

    حالة TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        اكسر;

    حالة TCG_HashLogEvent:
        regs->eax = hash_log_event_int((بنية hlei*)درون_داد_buf32(regs),
                                       (بنية hleo*)output_buf32(regs));
        اكسر;

    حالة TCG_HashAll:
        regs->eax =
            hash_all_int((بنية hai*)درون_داد_buf32(regs),
                          (u8 *)output_buf32(regs));
        اكسر;

    حالة TCG_TSS:
        regs->eax = tss_int((بنية ti*)درون_داد_buf32(regs),
                            (بنية مقصد*)output_buf32(regs));
        اكسر;

    حالة TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)درون_داد_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        اكسر;

    افتراضي:
        مجموعه_cf(regs, 1);
    }

    أرجع;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

تعريفنوع u8 tpm_ppi_کد;

ساكن صحيح
tpm12_read_has_owner(صحيح *has_owner)
{
    بنية tpm_res_getcap_ownerauth oauth;
    صحيح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, حجم(oauth));
    إذا (ret)
        أرجع -1;

    *has_owner = oauth.flag;

    أرجع 0;
}

ساكن صحيح
tpm12_enable_tpm(صحيح enable, صحيح verbose)
{
    بنية tpm_permanent_flags pf;
    صحيح ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    إذا (ret)
        أرجع -1;

    إذا (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        أرجع 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    إذا (ret) {
        إذا (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        وإلا
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    أرجع ret;
}

ساكن صحيح
tpm12_activate_tpm(صحيح activate, صحيح allow_reset, صحيح verbose)
{
    بنية tpm_permanent_flags pf;
    صحيح ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    إذا (ret)
        أرجع -1;

    إذا (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        أرجع 0;

    إذا (pf.flags[PERM_FLAG_IDX_DISABLE])
        أرجع 0;

    ret = tpm_simple_cmd(0, TPM_ORD_Physical_مجموعه_Deactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    إذا (ret)
        أرجع ret;

    إذا (activate && allow_reset) {
        إذا (verbose) {
            چاپ_کن("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    أرجع 0;
}

ساكن صحيح
tpm12_enable_activate(صحيح allow_reset, صحيح verbose)
{
    صحيح ret = tpm12_enable_tpm(1, verbose);
    إذا (ret)
        أرجع ret;

    أرجع tpm12_activate_tpm(1, allow_reset, verbose);
}

ساكن صحيح
tpm12_force_clear(صحيح enable_activate_before, صحيح enable_activate_after,
                  صحيح verbose)
{
    صحيح has_owner;
    صحيح ret = tpm12_read_has_owner(&has_owner);
    إذا (ret)
        أرجع -1;
    إذا (!has_owner) {
        إذا (verbose)
            چاپ_کن("TPM does not have an owner.\n");
        أرجع 0;
    }

    إذا (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        إذا (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            أرجع ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    إذا (ret)
        أرجع ret;

    إذا (!enable_activate_after) {
        إذا (verbose)
            چاپ_کن("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        أرجع 0;
    }

    أرجع tpm12_enable_activate(1, verbose);
}

ساكن صحيح
tpm12_مجموعه_owner_install(صحيح allow, صحيح verbose)
{
    صحيح has_owner;
    صحيح ret = tpm12_read_has_owner(&has_owner);
    إذا (ret)
        أرجع -1;
    إذا (has_owner) {
        إذا (verbose)
            چاپ_کن("Must first remove owner.\n");
        أرجع 0;
    }

    بنية tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    إذا (ret)
        أرجع -1;

    إذا (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        إذا (verbose)
            چاپ_کن("TPM must first be enable.\n");
        أرجع 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_مجموعه_Owner_Install,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    إذا (ret)
        أرجع ret;

    إذا (verbose)
        چاپ_کن("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    أرجع 0;
}

ساكن صحيح
tpm12_process_cfg(tpm_ppi_کد پیام_کد, صحيح verbose)
{
    صحيح ret = 0;

    بدل (پیام_کد) {
        حالة TPM_PPI_OP_NOOP: /* no-op */
            اكسر;

        حالة TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            اكسر;

        حالة TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            اكسر;

        حالة TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            اكسر;

        حالة TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            اكسر;

        حالة TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            اكسر;

        حالة TPM_PPI_OP_مجموعه_OWNERINSTALL_TRUE:
            ret = tpm12_مجموعه_owner_install(1, verbose);
            اكسر;

        حالة TPM_PPI_OP_مجموعه_OWNERINSTALL_FALSE:
            ret = tpm12_مجموعه_owner_install(0, verbose);
            اكسر;

        افتراضي:
            اكسر;
    }

    إذا (ret)
        چاپ_کن("Op %d: An error occurred: 0x%x\n", پیام_کد, ret);

    أرجع ret;
}

ساكن صحيح
tpm20_clearcontrol(u8 disable, صحيح verbose)
{
    بنية tpm2_req_clearcontrol trc = {
        .hdr.برچسب     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
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
    بنية tpm_rsp_سرآیند rsp;
    u32 resp_درازی = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_SHORT);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    أرجع ret;
}

ساكن صحيح
tpm20_clear(فراغ)
{
    بنية tpm2_req_clear trq = {
        .hdr.برچسب     = cpu_مقصد_be16(TPM2_ST_SESSIONS),
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
    بنية tpm_rsp_سرآیند rsp;
    u32 resp_درازی = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_درازی,
                             TPM_DURATION_TYPE_MEDIUM);
    إذا (ret || resp_درازی != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    أرجع ret;
}

ساكن صحيح
tpm20_process_cfg(tpm_ppi_کد پیام_کد, صحيح verbose)
{
    صحيح ret = 0;

    بدل (پیام_کد) {
        حالة TPM_PPI_OP_NOOP: /* no-op */
            اكسر;

        حالة TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            إذا (!ret)
                 ret = tpm20_clear();
            اكسر;
    }

    إذا (ret)
        چاپ_کن("Op %d: An error occurred: 0x%x\n", پیام_کد, ret);

    أرجع ret;
}

ساكن صحيح
tpm12_get_tpm_state(فراغ)
{
    صحيح state = 0;
    بنية tpm_permanent_flags pf;
    صحيح has_owner;

    إذا (tpm12_read_permanent_flags((حرف *)&pf, حجم(pf)) ||
        tpm12_read_has_owner(&has_owner))
        أرجع ~0;

    إذا (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    إذا (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_فعال;

    إذا (has_owner) {
        state |= TPM_STATE_OWNED;
    } وإلا {
        إذا (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    أرجع state;
}

ساكن فراغ
tpm12_show_tpm_menu(صحيح state, صحيح بعدی_scancodes[7])
{
    صحيح i = 0;

    چاپ_کن("\nThe current state of the TPM is:\n");

    إذا (state & TPM_STATE_ENABLED)
        چاپ_کن("  Enabled");
    وإلا
        چاپ_کن("  Disabled");

    إذا (state & TPM_STATE_فعال)
        چاپ_کن(" and active\n");
    وإلا
        چاپ_کن(" and deactivated\n");

    إذا (state & TPM_STATE_OWNED)
        چاپ_کن("  Ownership has been taken\n");
    وإلا {
        چاپ_کن("  Ownership has not been taken\n");
        إذا (state & TPM_STATE_OWNERINSTALL)
            چاپ_کن("  A user can take ownership of the TPM\n");
        وإلا
            چاپ_کن("  Taking ownership of the TPM has been disabled\n");
    }

    إذا ((state & (TPM_STATE_ENABLED | TPM_STATE_فعال)) !=
        (TPM_STATE_ENABLED | TPM_STATE_فعال)) {
        چاپ_کن("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    چاپ_کن("\nAvailable options are:\n");
    إذا (state & TPM_STATE_ENABLED) {
        چاپ_کن(" d. Disable the TPM\n");
        بعدی_scancodes[i++] = 32;

        إذا (state & TPM_STATE_فعال) {
            چاپ_کن(" v. Deactivate the TPM\n");
            بعدی_scancodes[i++] = 47;

            إذا (state & TPM_STATE_OWNERINSTALL) {
                چاپ_کن(" p. Prevent installation of an owner\n");
                بعدی_scancodes[i++] = 25;
            } وإلا {
                چاپ_کن(" s. Allow installation of an owner\n");
                بعدی_scancodes[i++] = 31;
            }
        } وإلا {
            چاپ_کن(" a. Activate the TPM\n");
            بعدی_scancodes[i++] = 30;
        }

    } وإلا {
        چاپ_کن(" e. Enable the TPM\n");
        بعدی_scancodes[i++] = 18;
    }

    إذا (state & TPM_STATE_OWNED) {
        چاپ_کن(" c. Clear ownership\n");
        بعدی_scancodes[i++] = 46;
    }

    بعدی_scancodes[i++] = 0;
}

ساكن فراغ
tpm12_menu(فراغ)
{
    صحيح scancode, بعدی_scancodes[7];
    tpm_ppi_کد پیام_کد;
    صحيح state = 0, i;
    صحيح waitkey;

    چاپ_کن("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    لكل (;;) {
        إذا ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, بعدی_scancodes);
        } وإلا {
            چاپ_کن("TPM is not working correctly.\n");
            أرجع;
        }

        چاپ_کن("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        پیام_کد = TPM_PPI_OP_NOOP;

        waitkey = 1;

        طالما (waitkey) {
            طالما ((scancode = get_keystroke(1000)) == ~0)
                ;

            بدل (scancode) {
            حالة 1:
                // ESC
                reset();
                اكسر;
            حالة 18: /* e. enable */
                پیام_کد = TPM_PPI_OP_ENABLE;
                اكسر;
            حالة 32: /* d. disable */
                پیام_کد = TPM_PPI_OP_DISABLE;
                اكسر;
            حالة 30: /* a. activate */
                پیام_کد = TPM_PPI_OP_ACTIVATE;
                اكسر;
            حالة 47: /* v. deactivate */
                پیام_کد = TPM_PPI_OP_DEACTIVATE;
                اكسر;
            حالة 46: /* c. clear owner */
                پیام_کد = TPM_PPI_OP_CLEAR;
                اكسر;
            حالة 25: /* p. prevent ownerinstall */
                پیام_کد = TPM_PPI_OP_مجموعه_OWNERINSTALL_FALSE;
                اكسر;
            حالة 31: /* s. allow ownerinstall */
                پیام_کد = TPM_PPI_OP_مجموعه_OWNERINSTALL_TRUE;
                اكسر;
            افتراضي:
                استمر;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            لكل (i = 0; i < حجم(بعدی_scancodes); i++) {
                إذا (بعدی_scancodes[i] == 0)
                    اكسر;

                إذا (بعدی_scancodes[i] == scancode) {
                    tpm12_process_cfg(پیام_کد, 1);
                    waitkey = 0;
                    اكسر;
                }
            }
        }
    }
}

ساكن صحيح
tpm20_menu_change_فعال_pcrbanks(فراغ)
{
    u8 فعال_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &فعال_banks);

    u8 activate_banks = فعال_banks;

    طالما (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        چاپ_کن("\nToggle active PCR banks by pressing number key\n\n");

        طالما (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ثابت حرف *hashname = tpm20_hashalg_flag_مقصد_name(flag);

            i++;
            إذا (hashname) {
                چاپ_کن("  %d: %s", i, hashname);
                إذا (activate_banks & hashalg_flag)
                    چاپ_کن(" (enabled)");
                چاپ_کن("\n");
            }

            hashalg_flag <<= 1;
        }
        چاپ_کن("\n"
               "ESC: return to previous menu without changes\n");
        إذا (activate_banks)
            چاپ_کن("A  : activate selection\n");

        u8 flagnum;
        صحيح show = 0;
        طالما (!show) {
            صحيح scancode = get_keystroke(1000);

            بدل (scancode) {
            حالة ~0:
                استمر;
            حالة 1: /* ESC */
                چاپ_کن("\n");
                أرجع -1;
            حالة 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                إذا (flagnum > i)
                    استمر;
                إذا (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                اكسر;
            حالة 30: /* a */
                إذا (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ساكن فراغ
tpm20_menu(فراغ)
{
    صحيح پیمایش_کد;
    tpm_ppi_کد پیام_کد;

    لكل (;;) {
        چاپ_کن("1. Clear TPM\n");
        چاپ_کن("2. Change active PCR banks\n");

        چاپ_کن("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        پیام_کد = TPM_PPI_OP_NOOP;

        طالما ((پیمایش_کد = get_keystroke(1000)) == ~0)
            ;

        بدل (پیمایش_کد) {
        حالة 1:
            // ESC
            reset();
            اكسر;
        حالة 2:
            پیام_کد = TPM_PPI_OP_CLEAR;
            اكسر;
        حالة 3:
            tpm20_menu_change_فعال_pcrbanks();
            استمر;
        افتراضي:
            استمر;
        }

        tpm20_process_cfg(پیام_کد, 0);
    }
}

فراغ
tpm_menu(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع;

    طالما (get_keystroke(0) >= 0)
        ;
    wait_نخها();

    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        tpm12_menu();
        اكسر;
    حالة TPM_نسخه_2:
        tpm20_menu();
        اكسر;
    }
}

صحيح
tpm_can_show_menu(فراغ)
{
    بدل (TPM_نسخه) {
    حالة TPM_نسخه_1_2:
        أرجع tpm_is_working() && TPM_has_physical_presence;
    حالة TPM_نسخه_2:
        أرجع tpm_is_working();
    }
    أرجع 0;
}
