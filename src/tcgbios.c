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

#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "byteorder.h" // cpu_to_*
#অন্তর্ভাব "config.h" // CONFIG_TCGBIOS
#অন্তর্ভাব "farptr.h" // MAKE_FLATPTR
#অন্তর্ভাব "fw/paravirt.h" // runningOnXen
#অন্তর্ভাব "hw/tpm_drivers.h" // tpm_drivers[]
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "sha.h" // sha1, sha256, ...
#অন্তর্ভাব "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#অন্তর্ভাব "std/smbios.h" // struct smbios_21_entry_point
#অন্তর্ভাব "std/tcg.h" // TCG_PC_LOGOVERFLOW
#অন্তর্ভাব "string.h" // checksum
#অন্তর্ভাব "tcgbios.h"// tpm_*, prototypes
#অন্তর্ভাব "util.h" // printf, get_keystroke
#অন্তর্ভাব "stacks.h" // wait_threads, reset
#অন্তর্ভাব "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

কাঠামো {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_দৈর্ঘ্য;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_গুনতি;

    /* address to write next log entry to */
    u8 *          log_area_পরবর্তী_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

স্থির পূর্ণসংখ্যা tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_দৈর্ঘ্য)
{
    যদি (!log_area_start_address || !log_area_minimum_দৈর্ঘ্য)
        ফেরত -1;

    memset(log_area_start_address, 0, log_area_minimum_দৈর্ঘ্য);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_দৈর্ঘ্য = log_area_minimum_দৈর্ঘ্য;
    tpm_state.log_area_পরবর্তী_entry = log_area_start_address;
    tpm_state.log_area_last_entry = ফাঁকা;
    tpm_state.entry_গুনতি = 0;
    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
tpm_tcpa_probe(শূন্য)
{
    কাঠামো tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    যদি (!tcpa)
        ফেরত -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_দৈর্ঘ্য);

    ফেরত tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_দৈর্ঘ্য);
}

স্থির পূর্ণসংখ্যা
tpm_tpm2_probe(শূন্য)
{
    কাঠামো tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    যদি (!tpm2)
        ফেরত -1;

    যদি (tpm2->দৈর্ঘ্য < 76)
        ফেরত -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_দৈর্ঘ্য);

    ফেরত tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
                            tpm2->log_area_minimum_দৈর্ঘ্য);
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
স্থির পূর্ণসংখ্যা
tpm_log_event(কাঠামো tpm_log_header *entry, পূর্ণসংখ্যা digest_len
              , ধ্রুবক শূন্য *event, পূর্ণসংখ্যা event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_পরবর্তী_entry);

    যদি (tpm_state.log_area_পরবর্তী_entry == ফাঁকা)
        ফেরত -1;

    u32 আকার_2 = (আকার(*entry) + digest_len
                + আকার(কাঠামো tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_পরবর্তী_entry + আকার_2
                   - tpm_state.log_area_start_address);
    যদি (logsize > tpm_state.log_area_minimum_দৈর্ঘ্য) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", আকার_2);
        ফেরত -1;
    }

    শূন্য *dest = tpm_state.log_area_পরবর্তী_entry;
    memcpy(dest, entry, আকার(*entry) + digest_len);
    কাঠামো tpm_log_trailer *t = dest + আকার(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_পরবর্তী_entry;
    tpm_state.log_area_পরবর্তী_entry += আকার_2;
    tpm_state.entry_গুনতি++;

    ফেরত 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

স্থির TPMVersion TPM_version;
স্থির u32 tpm20_pcr_selection_আকার;
স্থির কাঠামো tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
কাঠামো tpm_log_entry {
    কাঠামো tpm_log_header hdr;
    u8 pad[আকার(কাঠামো tpm2_digest_মানসমূহ)
           + 8 * আকার(কাঠামো tpm2_digest_মান)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

স্থির ধ্রুবক কাঠামো hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ধ্রুবক অক্ষর *name;
    শূন্য (*hashfunc)(ধ্রুবক u8 *তথ্য, u32 দৈর্ঘ্য, u8 *hash);
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

স্থির পূর্ণসংখ্যা
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    জন্য (i = 0; i < ARRAY_আকার(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg == hashAlg)
            ফেরত hash_parameters[i].hash_buffersize;
    }
    ফেরত -1;
}

স্থির u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    জন্য (i = 0; i < ARRAY_আকার(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg == hashAlg)
            ফেরত hash_parameters[i].hashalg_flag;
    }
    ফেরত 0;
}

স্থির u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    জন্য (i = 0; i < ARRAY_আকার(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg_flag == hashalg_flag)
            ফেরত hash_parameters[i].hashalg;
    }
    ফেরত 0;
}

স্থির ধ্রুবক অক্ষর *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    জন্য (i = 0; i < ARRAY_আকার(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg_flag == hashalg_flag)
            ফেরত hash_parameters[i].name;
    }
    ফেরত ফাঁকা;
}

স্থির শূন্য tpm2_hash_তথ্য(u16 hashAlg, ধ্রুবক u8 *তথ্য, u32 তথ্য_len, u8 *hash)
{
    unsigned i;

    জন্য (i = 0; i < ARRAY_আকার(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg == hashAlg) {
            যদি (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(তথ্য, তথ্য_len, hash);
            } অন্যথা {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
স্থির পূর্ণসংখ্যা
tpm20_write_EfiSpecIdEventStruct(শূন্য)
{
    যদি (!tpm20_pcr_selection)
        ফেরত -1;

    কাঠামো {
        কাঠামো TCG_EfiSpecIdEventStruct hdr;
        u8 pad[আকার(কাঠামো tpm_log_entry) + আকার(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintn_আকার = 2,
    };

    কাঠামো tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    শূন্য *nsel, *end = (শূন্য*)tpm20_pcr_selection + tpm20_pcr_selection_আকার;

    u32 গুনতি, অঙ্ক_Algs = 0;
    জন্য (গুনতি = 0; গুনতি < be32_to_cpu(tpm20_pcr_selection->গুনতি); গুনতি++) {
        u8 আকার_Of_Select = sel->আকার_Of_Select;

        nsel = (শূন্য*)sel + আকার(*sel) + আকার_Of_Select;
        যদি (nsel > end)
            বিরতি;

        যদি (!আকার_Of_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            চালু;
        }

        পূর্ণসংখ্যা hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        যদি (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ফেরত -1;
        }

        পূর্ণসংখ্যা event_আকার = offsetof(কাঠামো TCG_EfiSpecIdEventStruct
                                  , digestSizes[গুনতি+1]);
        যদি (event_আকার > আকার(event) - আকার(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            ফেরত -1;
        }

        event.hdr.digestSizes[অঙ্ক_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[অঙ্ক_Algs].digest_আকার = hsize;
        অঙ্ক_Algs++;

        sel = nsel;
    }

    যদি (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ফেরত -1;
    }

    event.hdr.সংখ্যা_Of_Algorithms = অঙ্ক_Algs;
    পূর্ণসংখ্যা event_আকার = offsetof(কাঠামো TCG_EfiSpecIdEventStruct
                              , digestSizes[অঙ্ক_Algs]);
    u8 *vendor_Info_আকার = (শূন্য*)&event + event_আকার;
    *vendor_Info_আকার = 0;
    event_আকার += আকার(*vendor_Info_আকার);

    কাঠামো tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    ফেরত tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_আকার);
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
স্থির পূর্ণসংখ্যা
tpm20_build_digest(কাঠামো tpm_log_entry *le,
                   ধ্রুবক u8 *hashdata, u32 hashdata_len, পূর্ণসংখ্যা bigEndian)
{
    যদি (!tpm20_pcr_selection)
        ফেরত -1;

    কাঠামো tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    শূন্য *nsel, *end = (শূন্য*)tpm20_pcr_selection + tpm20_pcr_selection_আকার;
    শূন্য *dest = le->hdr.digest + আকার(কাঠামো tpm2_digest_মানসমূহ);

    u32 গুনতি, অঙ্ক_Algs = 0;
    জন্য (গুনতি = 0; গুনতি < be32_to_cpu(tpm20_pcr_selection->গুনতি); গুনতি++) {
        u8 আকার_Of_Select = sel->আকার_Of_Select;

        nsel = (শূন্য*)sel + আকার(*sel) + আকার_Of_Select;
        যদি (nsel > end)
            বিরতি;

        /* PCR 0-7 unused? -- skip */
        যদি (!আকার_Of_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            চালু;
        }

        পূর্ণসংখ্যা hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        যদি (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ফেরত -1;
        }

        /* buffer size sanity check before writing */
        কাঠামো tpm2_digest_মান *v = dest;
        যদি (dest + আকার(*v) + hsize > (শূন্য*)le + আকার(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            ফেরত -1;
        }

        যদি (bigEndian)
            v->hashAlg = sel->hashAlg;
        অন্যথা
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_তথ্য(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += আকার(*v) + hsize;
        sel = nsel;

        অঙ্ক_Algs++;
    }

    যদি (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ফেরত -1;
    }

    কাঠামো tpm2_digest_মানসমূহ *v = (শূন্য*)le->hdr.digest;
    যদি (bigEndian)
        v->গুনতি = cpu_to_be32(অঙ্ক_Algs);
    অন্যথা
        v->গুনতি = অঙ্ক_Algs;

    ফেরত dest - (শূন্য*)le->hdr.digest;
}

স্থির পূর্ণসংখ্যা
tpm12_build_digest(কাঠামো tpm_log_entry *le,
                   ধ্রুবক u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    ফেরত SHA1_BUFSIZE;
}

স্থির পূর্ণসংখ্যা
tpm12_build_digest_direct(কাঠামো tpm_log_entry *le, ধ্রুবক u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    ফেরত SHA1_BUFSIZE;
}

স্থির পূর্ণসংখ্যা
tpm_build_digest(কাঠামো tpm_log_entry *le, ধ্রুবক u8 *hashdata, u32 hashdata_len
                 , পূর্ণসংখ্যা bigEndian)
{
    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        ফেরত tpm12_build_digest(le, hashdata, hashdata_len);
    ক্ষেত্র TPM_VERSION_2:
        ফেরত tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    ফেরত -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
স্থির পূর্ণসংখ্যা
tpm_simple_cmd(u8 locty, u32 ordinal
               , পূর্ণসংখ্যা param_আকার, u16 param, গণনা tpmDurationType to_t)
{
    কাঠামো {
        কাঠামো tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(আকার(req.trqh) + param_আকার),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_আকার == 2 ? cpu_to_be16(param) : param,
    };
    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        বিরতি;
    ক্ষেত্র TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        বিরতি;
    }

    u8 obuffer[64];
    কাঠামো tpm_rsp_header *trsh = (শূন্য*)obuffer;
    u32 obuffer_len = আকার(obuffer);
    memset(obuffer, 0x0, আকার(obuffer));

    পূর্ণসংখ্যা ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_getcapability(u32 capability, u32 property, u32 গুনতি,
                    কাঠামো tpm_rsp_header *rsp, u32 rsize)
{
    কাঠামো tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(আকার(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(গুনতি),
    };

    u32 resp_আকার = rsize;
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_আকার,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_get_pcrbanks(শূন্য)
{
    u8 বাফার[128];
    কাঠামো tpm2_res_getcapability *trg =
      (কাঠামো tpm2_res_getcapability *)&বাফার;

    পূর্ণসংখ্যা ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  আকার(বাফার));
    যদি (ret)
        ফেরত ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    যদি (resplen <= offsetof(কাঠামো tpm2_res_getcapability, তথ্য))
        ফেরত -1;

    u32 আকার_2 = resplen - offsetof(কাঠামো tpm2_res_getcapability, তথ্য);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    যদি (আকার_2 < offsetof(কাঠামো tpml_pcr_selection, selections) +
               offsetof(কাঠামো tpms_pcr_selection, pcrSelect))
        ফেরত -1;

    tpm20_pcr_selection = malloc_উপরের(আকার_2);
    যদি (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->তথ্য, আকার_2);
        tpm20_pcr_selection_আকার = আকার_2;
    } অন্যথা {
        warn_noalloc();
        ret = -1;
    }

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    যদি (!tpm20_pcr_selection)
        ফেরত -1;

    কাঠামো tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    শূন্য *end = (শূন্য*)tpm20_pcr_selection + tpm20_pcr_selection_আকার;

    যখন (1) {
        u8 আকার_Of_Select = sel->আকার_Of_Select;
        শূন্য *nsel = (শূন্য*)sel + আকার(*sel) + আকার_Of_Select;
        যদি (nsel > end)
            ফেরত 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        জন্য (i = 0; i < আকার_Of_Select; i++) {
            যদি (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                বিরতি;
            }
        }

        sel = nsel;
    }
}

স্থির পূর্ণসংখ্যা
tpm20_set_pcrbanks(u32 active_banks)
{
    কাঠামো tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(আকার(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    কাঠামো tpms_pcr_selection3 {
        u16 hashAlg;
        u8 আকার_Of_Select;
        u8 pcrSelect[3];
    } tps[ARRAY_আকার(trpa.tpms_pcr_selections)];
    পূর্ণসংখ্যা i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    যখন (hashalg_flag) {
        যদি ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            যদি (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].আকার_Of_Select = 3;

                যদি (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.গুনতি = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * আকার(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(কাঠামো tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * আকার(tps[0]));

    কাঠামো tpm_rsp_header rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);

    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা tpm20_activate_pcrbanks(u32 active_banks)
{
    পূর্ণসংখ্যা ret = tpm20_set_pcrbanks(active_banks);
    যদি (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    যদি (!ret)
        reset();
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm12_get_capability(u32 cap, u32 subcap, কাঠামো tpm_rsp_header *rsp, u32 rsize)
{
    কাঠামো tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(আকার(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .sub_Cap_আকার = cpu_to_be32(আকার(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_আকার = rsize;
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_আকার,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_আকার != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm12_read_permanent_flags(অক্ষর *buf, পূর্ণসংখ্যা buf_len)
{
    memset(buf, 0, buf_len);

    কাঠামো tpm_res_getcap_perm_flags pf;
    পূর্ণসংখ্যা ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, আকার(pf));
    যদি (ret)
        ফেরত -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
tpm12_determine_timeouts(শূন্য)
{
    কাঠামো tpm_res_getcap_timeouts timeouts;
    পূর্ণসংখ্যা ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, আকার(timeouts));
    যদি (ret)
        ফেরত ret;

    কাঠামো tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, আকার(durations));
    যদি (ret)
        ফেরত ret;

    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    জন্য (i = 0; i < 4; i++)
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

    ফেরত 0;
}

স্থির শূন্য
tpm20_set_timeouts(শূন্য)
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

স্থির পূর্ণসংখ্যা
tpm12_extend(কাঠামো tpm_log_entry *le, পূর্ণসংখ্যা digest_len)
{
    কাঠামো tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(আকার(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, আকার(tre.digest));

    কাঠামো tpm_rsp_extend rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.hdr.errcode)
        ফেরত -1;

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা tpm20_extend(কাঠামো tpm_log_entry *le, পূর্ণসংখ্যা digest_len)
{
    কাঠামো tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(আকার(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 বাফার[আকার(tmp_tre) + আকার(le->pad)];
    কাঠামো tpm2_req_extend *tre = (কাঠামো tpm2_req_extend *)বাফার;

    memcpy(tre, &tmp_tre, আকার(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(আকার(tmp_tre) + digest_len);

    কাঠামো tpm_rsp_header rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.errcode)
        ফেরত -1;

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
tpm_extend(কাঠামো tpm_log_entry *le, পূর্ণসংখ্যা digest_len)
{
    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        ফেরত tpm12_extend(le, digest_len);
    ক্ষেত্র TPM_VERSION_2:
        ফেরত tpm20_extend(le, digest_len);
    }
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
tpm20_stirrandom(শূন্য)
{
    কাঠামো tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(আকার(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .আকার_2 = cpu_to_be16(আকার(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    কাঠামো tpm_rsp_header rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    কাঠামো tpm2_res_getrandom rsp;

    যদি (buf_len > আকার(rsp.rnd.বাফার))
        ফেরত -1;

    কাঠামো tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(আকার(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_দৈর্ঘ্য = আকার(rsp);

    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.hdr.errcode)
        ret = -1;
    অন্যথা
        memcpy(buf, rsp.rnd.বাফার, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    কাঠামো tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(আকার(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(আকার(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    কাঠামো tpm_rsp_header rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_hierarchychangeauth(u8 auth[20])
{
    কাঠামো tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(আকার(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(আকার(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .আকার_2 = cpu_to_be16(আকার(trhca.newAuth.বাফার)),
        },
    };
    memcpy(trhca.newAuth.বাফার, auth, আকার(trhca.newAuth.বাফার));

    কাঠামো tpm_rsp_header rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    ফেরত ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

স্থির পূর্ণসংখ্যা TPM_has_physical_presence;
u8 TPM_working VARLOW;

স্থির পূর্ণসংখ্যা
tpm_is_working(শূন্য)
{
    ফেরত CONFIG_TCGBIOS && TPM_working;
}

স্থির শূন্য
tpm_set_failure(শূন্য)
{
    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_Set_অস্থায়ী_Deactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        বিরতি;
    ক্ষেত্র TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        বিরতি;
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
স্থির শূন্য
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           ধ্রুবক অক্ষর *event, u32 event_দৈর্ঘ্য,
                           ধ্রুবক u8 *hashdata, u32 hashdata_দৈর্ঘ্য)
{
    যদি (!tpm_is_working())
        ফেরত;

    কাঠামো tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    পূর্ণসংখ্যা digest_len = tpm_build_digest(&le, hashdata, hashdata_দৈর্ঘ্য, 1);
    যদি (digest_len < 0)
        ফেরত;
    পূর্ণসংখ্যা ret = tpm_extend(&le, digest_len);
    যদি (ret) {
        tpm_set_failure();
        ফেরত;
    }
    tpm_build_digest(&le, hashdata, hashdata_দৈর্ঘ্য, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_দৈর্ঘ্য);
}

// Add an EV_ACTION measurement to the list of measurements
স্থির শূন্য
tpm_add_action(u32 pcr_সূচক, ধ্রুবক অক্ষর *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcr_সূচক, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
স্থির শূন্য
tpm_add_event_separators(শূন্য)
{
    স্থির ধ্রুবক u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcr_সূচক;
    জন্য (pcr_সূচক = 0; pcr_সূচক <= 7; pcr_সূচক++)
        tpm_add_measurement_to_log(pcr_সূচক, EV_SEPARATOR,
                                   (ধ্রুবক অক্ষর *)evt_separator,
                                   আকার(evt_separator),
                                   evt_separator,
                                   আকার(evt_separator));
}

স্থির শূন্য
tpm_smbios_measure(শূন্য)
{
    কাঠামো pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    শূন্য *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    যদি (!smbios_tables)
        ফেরত;

    sha1((ধ্রুবক u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (ধ্রুবক অক্ষর *)&pcctes, আকার(pcctes),
                               (u8 *)&pcctes, আকার(pcctes));
}

স্থির পূর্ণসংখ্যা
tpm12_assert_physical_presence(শূন্য)
{
    পূর্ণসংখ্যা ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    যদি (!ret)
        ফেরত 0;

    কাঠামো tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((অক্ষর *)&pf, আকার(pf));
    যদি (ret)
        ফেরত -1;

    /* check if hardware physical presence is supported */
    যদি (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        ফেরত 0;
    }

    যদি (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        ফেরত tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
tpm12_startup(শূন্য)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    পূর্ণসংখ্যা ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    যদি (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    যদি (ret)
        যাও err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    যদি (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    যদি (ret)
        যাও err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    যদি (ret)
        যাও err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    যদি (ret && ret != TPM_BAD_LOCALITY)
        যাও err_exit;

    ফেরত 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
tpm20_startup(শূন্য)
{
    tpm20_set_timeouts();

    পূর্ণসংখ্যা ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    যদি (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    যদি (ret)
        যাও err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    যদি (ret)
        যাও err_exit;

    ret = tpm20_get_pcrbanks();
    যদি (ret)
        যাও err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    যদি (ret)
        যাও err_exit;

    ফেরত 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
tpm_startup(শূন্য)
{
    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        ফেরত tpm12_startup();
    ক্ষেত্র TPM_VERSION_2:
        ফেরত tpm20_startup();
    }
    ফেরত -1;
}

শূন্য
tpm_setup(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত;

    পূর্ণসংখ্যা ret = tpm_tpm2_probe();
    যদি (ret) {
        ret = tpm_tcpa_probe();
        যদি (ret)
            ফেরত;
    }

    TPM_version = tpmhw_probe();
    যদি (TPM_version == TPM_VERSION_NONE)
        ফেরত;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    যদি (runningOnXen())
        ফেরত;

    ret = tpm_startup();
    যদি (ret)
        ফেরত;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

স্থির শূন্য
tpm20_prepboot(শূন্য)
{
    পূর্ণসংখ্যা ret = tpm20_stirrandom();
    যদি (ret)
         যাও err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], আকার(auth));
    যদি (ret)
        যাও err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    যদি (ret)
        যাও err_exit;

    ফেরত;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

শূন্য
tpm_prepboot(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত;

    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        যদি (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        বিরতি;
    ক্ষেত্র TPM_VERSION_2:
        tpm20_prepboot();
        বিরতি;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
শূন্য
tpm_option_rom(ধ্রুবক শূন্য *addr, u32 len)
{
    যদি (!tpm_is_working())
        ফেরত;

    কাঠামো pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = আকার(u16) + আকার(u16) + SHA1_BUFSIZE,
    };
    sha1((ধ্রুবক u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (ধ্রুবক অক্ষর *)&pcctes, আকার(pcctes),
                               (u8 *)&pcctes, আকার(pcctes));
}

শূন্য
tpm_add_bcv(u32 bootdrv, ধ্রুবক u8 *addr, u32 দৈর্ঘ্য)
{
    যদি (!tpm_is_working())
        ফেরত;

    যদি (দৈর্ঘ্য < 0x200)
        ফেরত;

    ধ্রুবক অক্ষর *string = "Booting BCV device 00h (Floppy)";
    যদি (bootdrv == 0x80)
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
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_তথ্য,
                               string, strlen(string),
                               addr + 0x1b8, 0x48);
}

শূন্য
tpm_add_cdrom(u32 bootdrv, ধ্রুবক u8 *addr, u32 দৈর্ঘ্য)
{
    যদি (!tpm_is_working())
        ফেরত;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ধ্রুবক অক্ষর *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, দৈর্ঘ্য);
}

শূন্য
tpm_add_cdrom_catalog(ধ্রুবক u8 *addr, u32 দৈর্ঘ্য)
{
    যদি (!tpm_is_working())
        ফেরত;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ধ্রুবক অক্ষর *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_তথ্য,
                               string, strlen(string),
                               addr, দৈর্ঘ্য);
}

শূন্য
tpm_s3_resume(শূন্য)
{
    যদি (!tpm_is_working())
        ফেরত;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    পূর্ণসংখ্যা ret = -1;

    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        বিরতি;
    ক্ষেত্র TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        যদি (ret)
            যাও err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        বিরতি;
    }

    যদি (ret)
        যাও err_exit;

    ফেরত;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

স্থির অন্তঃরেখ শূন্য *প্রবেশ_buf32(কাঠামো bregs *regs)
{
    ফেরত MAKE_FLATPTR(regs->es, regs->di);
}

স্থির অন্তঃরেখ শূন্য *output_buf32(কাঠামো bregs *regs)
{
    ফেরত MAKE_FLATPTR(regs->ds, regs->si);
}

স্থির u32
hash_log_extend(কাঠামো pcpes *pcpes, ধ্রুবক শূন্য *hashdata, u32 hashdata_দৈর্ঘ্য
                , শূন্য *event, পূর্ণসংখ্যা extend)
{
    যদি (pcpes->pcrindex >= 24)
        ফেরত TCG_INVALID_প্রবেশ_PARA;
    যদি (hashdata)
        sha1(hashdata, hashdata_দৈর্ঘ্য, pcpes->digest);

    কাঠামো tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    পূর্ণসংখ্যা digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    যদি (digest_len < 0)
        ফেরত TCG_GENERAL_ERROR;
    যদি (extend) {
        পূর্ণসংখ্যা ret = tpm_extend(&le, digest_len);
        যদি (ret)
            ফেরত TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    পূর্ণসংখ্যা ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    যদি (ret)
        ফেরত TCG_PC_LOGOVERFLOW;
    ফেরত 0;
}

স্থির u32
hash_log_extend_event_int(ধ্রুবক কাঠামো hleei_short *hleei_s,
                          কাঠামো hleeo *hleeo)
{
    u32 rc = 0;
    কাঠামো hleei_long *hleei_l = (কাঠামো hleei_long *)hleei_s;
    ধ্রুবক শূন্য *logdataptr;
    u32 logdatalen;
    কাঠামো pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    নির্বাচন (hleei_s->ipblength) {
    ক্ষেত্র আকার(কাঠামো hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    বিরতি;

    ক্ষেত্র আকার(কাঠামো hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    বিরতি;

    পূর্বনির্ধারিত:
        /* bad input block */
        rc = TCG_INVALID_প্রবেশ_PARA;
        যাও err_exit;
    }

    pcpes = (কাঠামো pcpes *)logdataptr;

    যদি (pcpes->pcrindex != pcrindex
        || logdatalen != আকার(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        যাও err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    যদি (rc)
        যাও err_exit;

    hleeo->opblength = আকার(কাঠামো hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_গুনতি;
    memcpy(hleeo->digest, pcpes->digest, আকার(hleeo->digest));

err_exit:
    যদি (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    ফেরত rc;
}

স্থির u32
pass_through_to_tpm_int(কাঠামো pttti *pttti, কাঠামো pttto *pttto)
{
    u32 rc = 0;
    কাঠামো tpm_req_header *trh = (শূন্য*)pttti->tpmopin;

    যদি (pttti->ipblength < আকার(কাঠামো pttti) + আকার(*trh)
        || pttti->ipblength != আকার(কাঠামো pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < আকার(কাঠামো pttto)) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        যাও err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        যদি (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_প্রবেশ_PARA;
            যাও err_exit;
        }
        বিরতি;
    ক্ষেত্র TPM_VERSION_2:
        যদি (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_প্রবেশ_PARA;
            যাও err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(কাঠামো pttto, tpmopout);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    যদি (ret) {
        rc = TCG_FATAL_COM_ERROR;
        যাও err_exit;
    }

    pttto->opblength = offsetof(কাঠামো pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    যদি (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    ফেরত rc;
}

স্থির u32
shutdown_preboot_interface(শূন্য)
{
    TPM_interface_shutdown = 1;
    ফেরত 0;
}

স্থির u32
hash_log_event_int(ধ্রুবক কাঠামো hlei *hlei, কাঠামো hleo *hleo)
{
    u32 rc = 0;
    u16 আকার_2;
    কাঠামো pcpes *pcpes;

    আকার_2 = hlei->ipblength;
    যদি (আকার_2 != আকার(*hlei)) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        যাও err_exit;
    }

    pcpes = (কাঠামো pcpes *)hlei->logdataptr;

    যদি (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != আকার(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        যাও err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    যদি (rc)
        যাও err_exit;

    /* updating the log was fine */
    hleo->opblength = আকার(কাঠামো hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_গুনতি;

err_exit:
    যদি (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    ফেরত rc;
}

স্থির u32
hash_all_int(ধ্রুবক কাঠামো hai *hai, u8 *hash)
{
    যদি (hai->ipblength != আকার(কাঠামো hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        ফেরত TCG_INVALID_প্রবেশ_PARA;

    sha1((ধ্রুবক u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    ফেরত 0;
}

স্থির u32
tss_int(কাঠামো ti *ti, কাঠামো to *to)
{
    to->opblength = আকার(কাঠামো to);
    to->reserved  = 0;

    ফেরত TCG_PC_UNSUPPORTED;
}

স্থির u32
compact_hash_log_extend_event_int(u8 *বাফার,
                                  u32 info,
                                  u32 দৈর্ঘ্য,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    কাঠামো pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = আকার(info),
    };
    u32 rc = hash_log_extend(&pcpes, বাফার, দৈর্ঘ্য, &info, 1);
    যদি (rc)
        ফেরত rc;

    *edx_ptr = tpm_state.entry_গুনতি;
    ফেরত 0;
}

শূন্য VISIBLE32FLAT
tpm_interrupt_handler32(কাঠামো bregs *regs)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত;

    set_cf(regs, 0);

    যদি (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        ফেরত;
    }

    নির্বাচন ((গণনা irq_ids)regs->al) {
    ক্ষেত্র TCG_StatusCheck:
        যদি (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } অন্যথা {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        বিরতি;

    ক্ষেত্র TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (কাঠামো hleei_short *)প্রবেশ_buf32(regs),
                  (কাঠামো hleeo *)output_buf32(regs));
        বিরতি;

    ক্ষেত্র TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((কাঠামো pttti *)প্রবেশ_buf32(regs),
                                    (কাঠামো pttto *)output_buf32(regs));
        বিরতি;

    ক্ষেত্র TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        বিরতি;

    ক্ষেত্র TCG_HashLogEvent:
        regs->eax = hash_log_event_int((কাঠামো hlei*)প্রবেশ_buf32(regs),
                                       (কাঠামো hleo*)output_buf32(regs));
        বিরতি;

    ক্ষেত্র TCG_HashAll:
        regs->eax =
            hash_all_int((কাঠামো hai*)প্রবেশ_buf32(regs),
                          (u8 *)output_buf32(regs));
        বিরতি;

    ক্ষেত্র TCG_TSS:
        regs->eax = tss_int((কাঠামো ti*)প্রবেশ_buf32(regs),
                            (কাঠামো to*)output_buf32(regs));
        বিরতি;

    ক্ষেত্র TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)প্রবেশ_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        বিরতি;

    পূর্বনির্ধারিত:
        set_cf(regs, 1);
    }

    ফেরত;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

প্রকারলেখ u8 tpm_ppi_code;

স্থির পূর্ণসংখ্যা
tpm12_read_has_owner(পূর্ণসংখ্যা *has_owner)
{
    কাঠামো tpm_res_getcap_ownerauth oauth;
    পূর্ণসংখ্যা ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, আকার(oauth));
    যদি (ret)
        ফেরত -1;

    *has_owner = oauth.flag;

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
tpm12_enable_tpm(পূর্ণসংখ্যা enable, পূর্ণসংখ্যা verbose)
{
    কাঠামো tpm_permanent_flags pf;
    পূর্ণসংখ্যা ret = tpm12_read_permanent_flags((অক্ষর *)&pf, আকার(pf));
    যদি (ret)
        ফেরত -1;

    যদি (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        ফেরত 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    যদি (ret) {
        যদি (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        অন্যথা
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm12_activate_tpm(পূর্ণসংখ্যা activate, পূর্ণসংখ্যা allow_reset, পূর্ণসংখ্যা verbose)
{
    কাঠামো tpm_permanent_flags pf;
    পূর্ণসংখ্যা ret = tpm12_read_permanent_flags((অক্ষর *)&pf, আকার(pf));
    যদি (ret)
        ফেরত -1;

    যদি (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        ফেরত 0;

    যদি (pf.flags[PERM_FLAG_IDX_DISABLE])
        ফেরত 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    যদি (ret)
        ফেরত ret;

    যদি (activate && allow_reset) {
        যদি (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
tpm12_enable_activate(পূর্ণসংখ্যা allow_reset, পূর্ণসংখ্যা verbose)
{
    পূর্ণসংখ্যা ret = tpm12_enable_tpm(1, verbose);
    যদি (ret)
        ফেরত ret;

    ফেরত tpm12_activate_tpm(1, allow_reset, verbose);
}

স্থির পূর্ণসংখ্যা
tpm12_force_clear(পূর্ণসংখ্যা enable_activate_before, পূর্ণসংখ্যা enable_activate_after,
                  পূর্ণসংখ্যা verbose)
{
    পূর্ণসংখ্যা has_owner;
    পূর্ণসংখ্যা ret = tpm12_read_has_owner(&has_owner);
    যদি (ret)
        ফেরত -1;
    যদি (!has_owner) {
        যদি (verbose)
            printf("TPM does not have an owner.\n");
        ফেরত 0;
    }

    যদি (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        যদি (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            ফেরত ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    যদি (ret)
        ফেরত ret;

    যদি (!enable_activate_after) {
        যদি (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        ফেরত 0;
    }

    ফেরত tpm12_enable_activate(1, verbose);
}

স্থির পূর্ণসংখ্যা
tpm12_set_owner_install(পূর্ণসংখ্যা allow, পূর্ণসংখ্যা verbose)
{
    পূর্ণসংখ্যা has_owner;
    পূর্ণসংখ্যা ret = tpm12_read_has_owner(&has_owner);
    যদি (ret)
        ফেরত -1;
    যদি (has_owner) {
        যদি (verbose)
            printf("Must first remove owner.\n");
        ফেরত 0;
    }

    কাঠামো tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((অক্ষর *)&pf, আকার(pf));
    যদি (ret)
        ফেরত -1;

    যদি (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        যদি (verbose)
            printf("TPM must first be enable.\n");
        ফেরত 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    যদি (ret)
        ফেরত ret;

    যদি (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
tpm12_process_cfg(tpm_ppi_code বার্তা_Code, পূর্ণসংখ্যা verbose)
{
    পূর্ণসংখ্যা ret = 0;

    নির্বাচন (বার্তা_Code) {
        ক্ষেত্র TPM_PPI_OP_NOOP: /* no-op */
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            বিরতি;

        পূর্বনির্ধারিত:
            বিরতি;
    }

    যদি (ret)
        printf("Op %d: An error occurred: 0x%x\n", বার্তা_Code, ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_clearcontrol(u8 disable, পূর্ণসংখ্যা verbose)
{
    কাঠামো tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(আকার(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(আকার(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    কাঠামো tpm_rsp_header rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_clear(শূন্য)
{
    কাঠামো tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(আকার(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(আকার(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    কাঠামো tpm_rsp_header rsp;
    u32 resp_দৈর্ঘ্য = আকার(rsp);
    পূর্ণসংখ্যা ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_দৈর্ঘ্য,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_দৈর্ঘ্য != আকার(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm20_process_cfg(tpm_ppi_code বার্তা_Code, পূর্ণসংখ্যা verbose)
{
    পূর্ণসংখ্যা ret = 0;

    নির্বাচন (বার্তা_Code) {
        ক্ষেত্র TPM_PPI_OP_NOOP: /* no-op */
            বিরতি;

        ক্ষেত্র TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            যদি (!ret)
                 ret = tpm20_clear();
            বিরতি;
    }

    যদি (ret)
        printf("Op %d: An error occurred: 0x%x\n", বার্তা_Code, ret);

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
tpm12_get_tpm_state(শূন্য)
{
    পূর্ণসংখ্যা state = 0;
    কাঠামো tpm_permanent_flags pf;
    পূর্ণসংখ্যা has_owner;

    যদি (tpm12_read_permanent_flags((অক্ষর *)&pf, আকার(pf)) ||
        tpm12_read_has_owner(&has_owner))
        ফেরত ~0;

    যদি (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    যদি (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    যদি (has_owner) {
        state |= TPM_STATE_OWNED;
    } অন্যথা {
        যদি (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    ফেরত state;
}

স্থির শূন্য
tpm12_show_tpm_menu(পূর্ণসংখ্যা state, পূর্ণসংখ্যা পরবর্তী_scancodes[7])
{
    পূর্ণসংখ্যা i = 0;

    printf("\nThe current state of the TPM is:\n");

    যদি (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    অন্যথা
        printf("  Disabled");

    যদি (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    অন্যথা
        printf(" and deactivated\n");

    যদি (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    অন্যথা {
        printf("  Ownership has not been taken\n");
        যদি (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        অন্যথা
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    যদি ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    যদি (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        পরবর্তী_scancodes[i++] = 32;

        যদি (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            পরবর্তী_scancodes[i++] = 47;

            যদি (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                পরবর্তী_scancodes[i++] = 25;
            } অন্যথা {
                printf(" s. Allow installation of an owner\n");
                পরবর্তী_scancodes[i++] = 31;
            }
        } অন্যথা {
            printf(" a. Activate the TPM\n");
            পরবর্তী_scancodes[i++] = 30;
        }

    } অন্যথা {
        printf(" e. Enable the TPM\n");
        পরবর্তী_scancodes[i++] = 18;
    }

    যদি (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        পরবর্তী_scancodes[i++] = 46;
    }

    পরবর্তী_scancodes[i++] = 0;
}

স্থির শূন্য
tpm12_menu(শূন্য)
{
    পূর্ণসংখ্যা scancode, পরবর্তী_scancodes[7];
    tpm_ppi_code বার্তা_Code;
    পূর্ণসংখ্যা state = 0, i;
    পূর্ণসংখ্যা waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    জন্য (;;) {
        যদি ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, পরবর্তী_scancodes);
        } অন্যথা {
            printf("TPM is not working correctly.\n");
            ফেরত;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        বার্তা_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        যখন (waitkey) {
            যখন ((scancode = get_keystroke(1000)) == ~0)
                ;

            নির্বাচন (scancode) {
            ক্ষেত্র 1:
                // ESC
                reset();
                বিরতি;
            ক্ষেত্র 18: /* e. enable */
                বার্তা_Code = TPM_PPI_OP_ENABLE;
                বিরতি;
            ক্ষেত্র 32: /* d. disable */
                বার্তা_Code = TPM_PPI_OP_DISABLE;
                বিরতি;
            ক্ষেত্র 30: /* a. activate */
                বার্তা_Code = TPM_PPI_OP_ACTIVATE;
                বিরতি;
            ক্ষেত্র 47: /* v. deactivate */
                বার্তা_Code = TPM_PPI_OP_DEACTIVATE;
                বিরতি;
            ক্ষেত্র 46: /* c. clear owner */
                বার্তা_Code = TPM_PPI_OP_CLEAR;
                বিরতি;
            ক্ষেত্র 25: /* p. prevent ownerinstall */
                বার্তা_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                বিরতি;
            ক্ষেত্র 31: /* s. allow ownerinstall */
                বার্তা_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                বিরতি;
            পূর্বনির্ধারিত:
                চালু;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            জন্য (i = 0; i < আকার(পরবর্তী_scancodes); i++) {
                যদি (পরবর্তী_scancodes[i] == 0)
                    বিরতি;

                যদি (পরবর্তী_scancodes[i] == scancode) {
                    tpm12_process_cfg(বার্তা_Code, 1);
                    waitkey = 0;
                    বিরতি;
                }
            }
        }
    }
}

স্থির পূর্ণসংখ্যা
tpm20_menu_change_active_pcrbanks(শূন্য)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    যখন (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        যখন (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ধ্রুবক অক্ষর *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            যদি (hashname) {
                printf("  %d: %s", i, hashname);
                যদি (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        যদি (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        পূর্ণসংখ্যা show = 0;
        যখন (!show) {
            পূর্ণসংখ্যা scancode = get_keystroke(1000);

            নির্বাচন (scancode) {
            ক্ষেত্র ~0:
                চালু;
            ক্ষেত্র 1: /* ESC */
                printf("\n");
                ফেরত -1;
            ক্ষেত্র 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                যদি (flagnum > i)
                    চালু;
                যদি (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                বিরতি;
            ক্ষেত্র 30: /* a */
                যদি (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

স্থির শূন্য
tpm20_menu(শূন্য)
{
    পূর্ণসংখ্যা scan_code;
    tpm_ppi_code বার্তা_Code;

    জন্য (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        বার্তা_Code = TPM_PPI_OP_NOOP;

        যখন ((scan_code = get_keystroke(1000)) == ~0)
            ;

        নির্বাচন (scan_code) {
        ক্ষেত্র 1:
            // ESC
            reset();
            বিরতি;
        ক্ষেত্র 2:
            বার্তা_Code = TPM_PPI_OP_CLEAR;
            বিরতি;
        ক্ষেত্র 3:
            tpm20_menu_change_active_pcrbanks();
            চালু;
        পূর্বনির্ধারিত:
            চালু;
        }

        tpm20_process_cfg(বার্তা_Code, 0);
    }
}

শূন্য
tpm_menu(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত;

    যখন (get_keystroke(0) >= 0)
        ;
    wait_threads();

    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        tpm12_menu();
        বিরতি;
    ক্ষেত্র TPM_VERSION_2:
        tpm20_menu();
        বিরতি;
    }
}

পূর্ণসংখ্যা
tpm_can_show_menu(শূন্য)
{
    নির্বাচন (TPM_version) {
    ক্ষেত্র TPM_VERSION_1_2:
        ফেরত tpm_is_working() && TPM_has_physical_presence;
    ক্ষেত্র TPM_VERSION_2:
        ফেরত tpm_is_working();
    }
    ফেরত 0;
}
