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

সংরচনা {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_গুনতি;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

স্থির পূর্ণাঙ্ক tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    যদি (!log_area_start_address || !log_area_minimum_length)
        প্রত্যাগম -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = রিক্ত;
    tpm_state.entry_গুনতি = 0;
    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
tpm_tcpa_probe(শূন্য)
{
    সংরচনা tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    যদি (!tcpa)
        প্রত্যাগম -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    প্রত্যাগম tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

স্থির পূর্ণাঙ্ক
tpm_tpm2_probe(শূন্য)
{
    সংরচনা tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    যদি (!tpm2)
        প্রত্যাগম -1;

    যদি (tpm2->length < 76)
        প্রত্যাগম -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    প্রত্যাগম tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
স্থির পূর্ণাঙ্ক
tpm_log_event(সংরচনা tpm_log_header *entry, পূর্ণাঙ্ক digest_len
              , ধ্রুব শূন্য *event, পূর্ণাঙ্ক event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    যদি (tpm_state.log_area_next_entry == রিক্ত)
        প্রত্যাগম -1;

    u32 size = (পরিমাণ(*entry) + digest_len
                + পরিমাণ(সংরচনা tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    যদি (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        প্রত্যাগম -1;
    }

    শূন্য *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, পরিমাণ(*entry) + digest_len);
    সংরচনা tpm_log_trailer *t = dest + পরিমাণ(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_গুনতি++;

    প্রত্যাগম 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

স্থির TPMVersion TPM_version;
স্থির u32 tpm20_pcr_selection_size;
স্থির সংরচনা tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
সংরচনা tpm_log_entry {
    সংরচনা tpm_log_header hdr;
    u8 pad[পরিমাণ(সংরচনা tpm2_digest_মানসমূহ)
           + 8 * পরিমাণ(সংরচনা tpm2_digest_মান)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

স্থির ধ্রুব সংরচনা hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ধ্রুব বর্ণ *name;
    শূন্য (*hashfunc)(ধ্রুব u8 *data, u32 length, u8 *hash);
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

স্থির পূর্ণাঙ্ক
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    প্রতি (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg == hashAlg)
            প্রত্যাগম hash_parameters[i].hash_buffersize;
    }
    প্রত্যাগম -1;
}

স্থির u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    প্রতি (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg == hashAlg)
            প্রত্যাগম hash_parameters[i].hashalg_flag;
    }
    প্রত্যাগম 0;
}

স্থির u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    প্রতি (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg_flag == hashalg_flag)
            প্রত্যাগম hash_parameters[i].hashalg;
    }
    প্রত্যাগম 0;
}

স্থির ধ্রুব বর্ণ *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    প্রতি (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg_flag == hashalg_flag)
            প্রত্যাগম hash_parameters[i].name;
    }
    প্রত্যাগম রিক্ত;
}

স্থির শূন্য tpm2_hash_data(u16 hashAlg, ধ্রুব u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    প্রতি (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        যদি (hash_parameters[i].hashalg == hashAlg) {
            যদি (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } অন্যথা {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
স্থির পূর্ণাঙ্ক
tpm20_write_EfiSpecIdEventStruct(শূন্য)
{
    যদি (!tpm20_pcr_selection)
        প্রত্যাগম -1;

    সংরচনা {
        সংরচনা TCG_EfiSpecIdEventStruct hdr;
        u8 pad[পরিমাণ(সংরচনা tpm_log_entry) + পরিমাণ(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    সংরচনা tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    শূন্য *nsel, *end = (শূন্য*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 গুনতি, অঙ্ক_Algs = 0;
    প্রতি (গুনতি = 0; গুনতি < be32_to_cpu(tpm20_pcr_selection->গুনতি); গুনতি++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (শূন্য*)sel + পরিমাণ(*sel) + sizeOfSelect;
        যদি (nsel > end)
            ভঙ্গ;

        যদি (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            অনুবর্ত;
        }

        পূর্ণাঙ্ক hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        যদি (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            প্রত্যাগম -1;
        }

        পূর্ণাঙ্ক event_size = offsetof(সংরচনা TCG_EfiSpecIdEventStruct
                                  , digestSizes[গুনতি+1]);
        যদি (event_size > পরিমাণ(event) - পরিমাণ(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            প্রত্যাগম -1;
        }

        event.hdr.digestSizes[অঙ্ক_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[অঙ্ক_Algs].digestSize = hsize;
        অঙ্ক_Algs++;

        sel = nsel;
    }

    যদি (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        প্রত্যাগম -1;
    }

    event.hdr.সংখ্যা_Of_Algorithms = অঙ্ক_Algs;
    পূর্ণাঙ্ক event_size = offsetof(সংরচনা TCG_EfiSpecIdEventStruct
                              , digestSizes[অঙ্ক_Algs]);
    u8 *vendorInfoSize = (শূন্য*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += পরিমাণ(*vendorInfoSize);

    সংরচনা tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    প্রত্যাগম tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
স্থির পূর্ণাঙ্ক
tpm20_build_digest(সংরচনা tpm_log_entry *le,
                   ধ্রুব u8 *hashdata, u32 hashdata_len, পূর্ণাঙ্ক bigEndian)
{
    যদি (!tpm20_pcr_selection)
        প্রত্যাগম -1;

    সংরচনা tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    শূন্য *nsel, *end = (শূন্য*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    শূন্য *dest = le->hdr.digest + পরিমাণ(সংরচনা tpm2_digest_মানসমূহ);

    u32 গুনতি, অঙ্ক_Algs = 0;
    প্রতি (গুনতি = 0; গুনতি < be32_to_cpu(tpm20_pcr_selection->গুনতি); গুনতি++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (শূন্য*)sel + পরিমাণ(*sel) + sizeOfSelect;
        যদি (nsel > end)
            ভঙ্গ;

        /* PCR 0-7 unused? -- skip */
        যদি (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            অনুবর্ত;
        }

        পূর্ণাঙ্ক hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        যদি (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            প্রত্যাগম -1;
        }

        /* buffer size sanity check before writing */
        সংরচনা tpm2_digest_মান *v = dest;
        যদি (dest + পরিমাণ(*v) + hsize > (শূন্য*)le + পরিমাণ(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            প্রত্যাগম -1;
        }

        যদি (bigEndian)
            v->hashAlg = sel->hashAlg;
        অন্যথা
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += পরিমাণ(*v) + hsize;
        sel = nsel;

        অঙ্ক_Algs++;
    }

    যদি (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        প্রত্যাগম -1;
    }

    সংরচনা tpm2_digest_মানসমূহ *v = (শূন্য*)le->hdr.digest;
    যদি (bigEndian)
        v->গুনতি = cpu_to_be32(অঙ্ক_Algs);
    অন্যথা
        v->গুনতি = অঙ্ক_Algs;

    প্রত্যাগম dest - (শূন্য*)le->hdr.digest;
}

স্থির পূর্ণাঙ্ক
tpm12_build_digest(সংরচনা tpm_log_entry *le,
                   ধ্রুব u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    প্রত্যাগম SHA1_BUFSIZE;
}

স্থির পূর্ণাঙ্ক
tpm12_build_digest_direct(সংরচনা tpm_log_entry *le, ধ্রুব u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    প্রত্যাগম SHA1_BUFSIZE;
}

স্থির পূর্ণাঙ্ক
tpm_build_digest(সংরচনা tpm_log_entry *le, ধ্রুব u8 *hashdata, u32 hashdata_len
                 , পূর্ণাঙ্ক bigEndian)
{
    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        প্রত্যাগম tpm12_build_digest(le, hashdata, hashdata_len);
    পক্ষ TPM_VERSION_2:
        প্রত্যাগম tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    প্রত্যাগম -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
স্থির পূর্ণাঙ্ক
tpm_simple_cmd(u8 locty, u32 ordinal
               , পূর্ণাঙ্ক param_size, u16 param, গণনা tpmDurationType to_t)
{
    সংরচনা {
        সংরচনা tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(পরিমাণ(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        ভঙ্গ;
    পক্ষ TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        ভঙ্গ;
    }

    u8 obuffer[64];
    সংরচনা tpm_rsp_header *trsh = (শূন্য*)obuffer;
    u32 obuffer_len = পরিমাণ(obuffer);
    memset(obuffer, 0x0, পরিমাণ(obuffer));

    পূর্ণাঙ্ক ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_getcapability(u32 capability, u32 property, u32 গুনতি,
                    সংরচনা tpm_rsp_header *rsp, u32 rsize)
{
    সংরচনা tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(পরিমাণ(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(গুনতি),
    };

    u32 resp_size = rsize;
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_get_pcrbanks(শূন্য)
{
    u8 buffer[128];
    সংরচনা tpm2_res_getcapability *trg =
      (সংরচনা tpm2_res_getcapability *)&buffer;

    পূর্ণাঙ্ক ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  পরিমাণ(buffer));
    যদি (ret)
        প্রত্যাগম ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    যদি (resplen <= offsetof(সংরচনা tpm2_res_getcapability, data))
        প্রত্যাগম -1;

    u32 size = resplen - offsetof(সংরচনা tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    যদি (size < offsetof(সংরচনা tpml_pcr_selection, selections) +
               offsetof(সংরচনা tpms_pcr_selection, pcrSelect))
        প্রত্যাগম -1;

    tpm20_pcr_selection = malloc_উপরের(size);
    যদি (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } অন্যথা {
        warn_noalloc();
        ret = -1;
    }

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    যদি (!tpm20_pcr_selection)
        প্রত্যাগম -1;

    সংরচনা tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    শূন্য *end = (শূন্য*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    যাবত্ (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        শূন্য *nsel = (শূন্য*)sel + পরিমাণ(*sel) + sizeOfSelect;
        যদি (nsel > end)
            প্রত্যাগম 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        প্রতি (i = 0; i < sizeOfSelect; i++) {
            যদি (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                ভঙ্গ;
            }
        }

        sel = nsel;
    }
}

স্থির পূর্ণাঙ্ক
tpm20_set_pcrbanks(u32 active_banks)
{
    সংরচনা tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(পরিমাণ(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    সংরচনা tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    পূর্ণাঙ্ক i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    যাবত্ (hashalg_flag) {
        যদি ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            যদি (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

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
    memcpy(trpa.tpms_pcr_selections, tps, i * পরিমাণ(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(সংরচনা tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * পরিমাণ(tps[0]));

    সংরচনা tpm_rsp_header rsp;
    u32 resp_length = পরিমাণ(rsp);

    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক tpm20_activate_pcrbanks(u32 active_banks)
{
    পূর্ণাঙ্ক ret = tpm20_set_pcrbanks(active_banks);
    যদি (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    যদি (!ret)
        reset();
    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm12_get_capability(u32 cap, u32 subcap, সংরচনা tpm_rsp_header *rsp, u32 rsize)
{
    সংরচনা tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(পরিমাণ(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(পরিমাণ(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm12_read_permanent_flags(বর্ণ *buf, পূর্ণাঙ্ক buf_len)
{
    memset(buf, 0, buf_len);

    সংরচনা tpm_res_getcap_perm_flags pf;
    পূর্ণাঙ্ক ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, পরিমাণ(pf));
    যদি (ret)
        প্রত্যাগম -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
tpm12_determine_timeouts(শূন্য)
{
    সংরচনা tpm_res_getcap_timeouts timeouts;
    পূর্ণাঙ্ক ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, পরিমাণ(timeouts));
    যদি (ret)
        প্রত্যাগম ret;

    সংরচনা tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, পরিমাণ(durations));
    যদি (ret)
        প্রত্যাগম ret;

    পূর্ণাঙ্ক i;
    প্রতি (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    প্রতি (i = 0; i < 4; i++)
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

    প্রত্যাগম 0;
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

স্থির পূর্ণাঙ্ক
tpm12_extend(সংরচনা tpm_log_entry *le, পূর্ণাঙ্ক digest_len)
{
    সংরচনা tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(পরিমাণ(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, পরিমাণ(tre.digest));

    সংরচনা tpm_rsp_extend rsp;
    u32 resp_length = পরিমাণ(rsp);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.hdr.errcode)
        প্রত্যাগম -1;

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক tpm20_extend(সংরচনা tpm_log_entry *le, পূর্ণাঙ্ক digest_len)
{
    সংরচনা tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(পরিমাণ(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[পরিমাণ(tmp_tre) + পরিমাণ(le->pad)];
    সংরচনা tpm2_req_extend *tre = (সংরচনা tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, পরিমাণ(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(পরিমাণ(tmp_tre) + digest_len);

    সংরচনা tpm_rsp_header rsp;
    u32 resp_length = পরিমাণ(rsp);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.errcode)
        প্রত্যাগম -1;

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
tpm_extend(সংরচনা tpm_log_entry *le, পূর্ণাঙ্ক digest_len)
{
    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        প্রত্যাগম tpm12_extend(le, digest_len);
    পক্ষ TPM_VERSION_2:
        প্রত্যাগম tpm20_extend(le, digest_len);
    }
    প্রত্যাগম -1;
}

স্থির পূর্ণাঙ্ক
tpm20_stirrandom(শূন্য)
{
    সংরচনা tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(পরিমাণ(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(পরিমাণ(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    সংরচনা tpm_rsp_header rsp;
    u32 resp_length = পরিমাণ(rsp);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    সংরচনা tpm2_res_getrandom rsp;

    যদি (buf_len > পরিমাণ(rsp.rnd.buffer))
        প্রত্যাগম -1;

    সংরচনা tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(পরিমাণ(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = পরিমাণ(rsp);

    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.hdr.errcode)
        ret = -1;
    অন্যথা
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    সংরচনা tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(পরিমাণ(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(পরিমাণ(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    সংরচনা tpm_rsp_header rsp;
    u32 resp_length = পরিমাণ(rsp);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_hierarchychangeauth(u8 auth[20])
{
    সংরচনা tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(পরিমাণ(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(পরিমাণ(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(পরিমাণ(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, পরিমাণ(trhca.newAuth.buffer));

    সংরচনা tpm_rsp_header rsp;
    u32 resp_length = পরিমাণ(rsp);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    প্রত্যাগম ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

স্থির পূর্ণাঙ্ক TPM_has_physical_presence;
u8 TPM_working VARLOW;

স্থির পূর্ণাঙ্ক
tpm_is_working(শূন্য)
{
    প্রত্যাগম CONFIG_TCGBIOS && TPM_working;
}

স্থির শূন্য
tpm_set_failure(শূন্য)
{
    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        ভঙ্গ;
    পক্ষ TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        ভঙ্গ;
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
                           ধ্রুব বর্ণ *event, u32 event_length,
                           ধ্রুব u8 *hashdata, u32 hashdata_length)
{
    যদি (!tpm_is_working())
        প্রত্যাগম;

    সংরচনা tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    পূর্ণাঙ্ক digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    যদি (digest_len < 0)
        প্রত্যাগম;
    পূর্ণাঙ্ক ret = tpm_extend(&le, digest_len);
    যদি (ret) {
        tpm_set_failure();
        প্রত্যাগম;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
স্থির শূন্য
tpm_add_action(u32 pcrIndex, ধ্রুব বর্ণ *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
স্থির শূন্য
tpm_add_event_separators(শূন্য)
{
    স্থির ধ্রুব u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    প্রতি (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (ধ্রুব বর্ণ *)evt_separator,
                                   পরিমাণ(evt_separator),
                                   evt_separator,
                                   পরিমাণ(evt_separator));
}

স্থির শূন্য
tpm_smbios_measure(শূন্য)
{
    সংরচনা pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    শূন্য *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    যদি (!smbios_tables)
        প্রত্যাগম;

    sha1((ধ্রুব u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (ধ্রুব বর্ণ *)&pcctes, পরিমাণ(pcctes),
                               (u8 *)&pcctes, পরিমাণ(pcctes));
}

স্থির পূর্ণাঙ্ক
tpm12_assert_physical_presence(শূন্য)
{
    পূর্ণাঙ্ক ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    যদি (!ret)
        প্রত্যাগম 0;

    সংরচনা tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((বর্ণ *)&pf, পরিমাণ(pf));
    যদি (ret)
        প্রত্যাগম -1;

    /* check if hardware physical presence is supported */
    যদি (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        প্রত্যাগম 0;
    }

    যদি (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        প্রত্যাগম tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    প্রত্যাগম -1;
}

স্থির পূর্ণাঙ্ক
tpm12_startup(শূন্য)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    পূর্ণাঙ্ক ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    যদি (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    যদি (ret)
        গচ্ছ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    যদি (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    যদি (ret)
        গচ্ছ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    যদি (ret)
        গচ্ছ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    যদি (ret && ret != TPM_BAD_LOCALITY)
        গচ্ছ err_exit;

    প্রত্যাগম 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    প্রত্যাগম -1;
}

স্থির পূর্ণাঙ্ক
tpm20_startup(শূন্য)
{
    tpm20_set_timeouts();

    পূর্ণাঙ্ক ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    যদি (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    যদি (ret)
        গচ্ছ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    যদি (ret)
        গচ্ছ err_exit;

    ret = tpm20_get_pcrbanks();
    যদি (ret)
        গচ্ছ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    যদি (ret)
        গচ্ছ err_exit;

    প্রত্যাগম 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    প্রত্যাগম -1;
}

স্থির পূর্ণাঙ্ক
tpm_startup(শূন্য)
{
    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        প্রত্যাগম tpm12_startup();
    পক্ষ TPM_VERSION_2:
        প্রত্যাগম tpm20_startup();
    }
    প্রত্যাগম -1;
}

শূন্য
tpm_setup(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        প্রত্যাগম;

    পূর্ণাঙ্ক ret = tpm_tpm2_probe();
    যদি (ret) {
        ret = tpm_tcpa_probe();
        যদি (ret)
            প্রত্যাগম;
    }

    TPM_version = tpmhw_probe();
    যদি (TPM_version == TPM_VERSION_NONE)
        প্রত্যাগম;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    যদি (runningOnXen())
        প্রত্যাগম;

    ret = tpm_startup();
    যদি (ret)
        প্রত্যাগম;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

স্থির শূন্য
tpm20_prepboot(শূন্য)
{
    পূর্ণাঙ্ক ret = tpm20_stirrandom();
    যদি (ret)
         গচ্ছ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], পরিমাণ(auth));
    যদি (ret)
        গচ্ছ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    যদি (ret)
        গচ্ছ err_exit;

    প্রত্যাগম;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

শূন্য
tpm_prepboot(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        প্রত্যাগম;

    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        যদি (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        ভঙ্গ;
    পক্ষ TPM_VERSION_2:
        tpm20_prepboot();
        ভঙ্গ;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
শূন্য
tpm_option_rom(ধ্রুব শূন্য *addr, u32 len)
{
    যদি (!tpm_is_working())
        প্রত্যাগম;

    সংরচনা pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = পরিমাণ(u16) + পরিমাণ(u16) + SHA1_BUFSIZE,
    };
    sha1((ধ্রুব u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (ধ্রুব বর্ণ *)&pcctes, পরিমাণ(pcctes),
                               (u8 *)&pcctes, পরিমাণ(pcctes));
}

শূন্য
tpm_add_bcv(u32 bootdrv, ধ্রুব u8 *addr, u32 length)
{
    যদি (!tpm_is_working())
        প্রত্যাগম;

    যদি (length < 0x200)
        প্রত্যাগম;

    ধ্রুব বর্ণ *string = "Booting BCV device 00h (Floppy)";
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
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr + 0x1b8, 0x48);
}

শূন্য
tpm_add_cdrom(u32 bootdrv, ধ্রুব u8 *addr, u32 length)
{
    যদি (!tpm_is_working())
        প্রত্যাগম;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ধ্রুব বর্ণ *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

শূন্য
tpm_add_cdrom_catalog(ধ্রুব u8 *addr, u32 length)
{
    যদি (!tpm_is_working())
        প্রত্যাগম;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ধ্রুব বর্ণ *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

শূন্য
tpm_s3_resume(শূন্য)
{
    যদি (!tpm_is_working())
        প্রত্যাগম;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    পূর্ণাঙ্ক ret = -1;

    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        ভঙ্গ;
    পক্ষ TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        যদি (ret)
            গচ্ছ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        ভঙ্গ;
    }

    যদি (ret)
        গচ্ছ err_exit;

    প্রত্যাগম;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

স্থির অন্তঃস্থ শূন্য *প্রবেশ_buf32(সংরচনা bregs *regs)
{
    প্রত্যাগম MAKE_FLATPTR(regs->es, regs->di);
}

স্থির অন্তঃস্থ শূন্য *output_buf32(সংরচনা bregs *regs)
{
    প্রত্যাগম MAKE_FLATPTR(regs->ds, regs->si);
}

স্থির u32
hash_log_extend(সংরচনা pcpes *pcpes, ধ্রুব শূন্য *hashdata, u32 hashdata_length
                , শূন্য *event, পূর্ণাঙ্ক extend)
{
    যদি (pcpes->pcrindex >= 24)
        প্রত্যাগম TCG_INVALID_প্রবেশ_PARA;
    যদি (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    সংরচনা tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    পূর্ণাঙ্ক digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    যদি (digest_len < 0)
        প্রত্যাগম TCG_GENERAL_ERROR;
    যদি (extend) {
        পূর্ণাঙ্ক ret = tpm_extend(&le, digest_len);
        যদি (ret)
            প্রত্যাগম TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    পূর্ণাঙ্ক ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    যদি (ret)
        প্রত্যাগম TCG_PC_LOGOVERFLOW;
    প্রত্যাগম 0;
}

স্থির u32
hash_log_extend_event_int(ধ্রুব সংরচনা hleei_short *hleei_s,
                          সংরচনা hleeo *hleeo)
{
    u32 rc = 0;
    সংরচনা hleei_long *hleei_l = (সংরচনা hleei_long *)hleei_s;
    ধ্রুব শূন্য *logdataptr;
    u32 logdatalen;
    সংরচনা pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    বিকল্প (hleei_s->ipblength) {
    পক্ষ পরিমাণ(সংরচনা hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    ভঙ্গ;

    পক্ষ পরিমাণ(সংরচনা hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    ভঙ্গ;

    স্বভাব:
        /* bad input block */
        rc = TCG_INVALID_প্রবেশ_PARA;
        গচ্ছ err_exit;
    }

    pcpes = (সংরচনা pcpes *)logdataptr;

    যদি (pcpes->pcrindex != pcrindex
        || logdatalen != পরিমাণ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        গচ্ছ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    যদি (rc)
        গচ্ছ err_exit;

    hleeo->opblength = পরিমাণ(সংরচনা hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_গুনতি;
    memcpy(hleeo->digest, pcpes->digest, পরিমাণ(hleeo->digest));

err_exit:
    যদি (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    প্রত্যাগম rc;
}

স্থির u32
pass_through_to_tpm_int(সংরচনা pttti *pttti, সংরচনা pttto *pttto)
{
    u32 rc = 0;
    সংরচনা tpm_req_header *trh = (শূন্য*)pttti->tpmopin;

    যদি (pttti->ipblength < পরিমাণ(সংরচনা pttti) + পরিমাণ(*trh)
        || pttti->ipblength != পরিমাণ(সংরচনা pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < পরিমাণ(সংরচনা pttto)) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        গচ্ছ err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        যদি (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_প্রবেশ_PARA;
            গচ্ছ err_exit;
        }
        ভঙ্গ;
    পক্ষ TPM_VERSION_2:
        যদি (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_প্রবেশ_PARA;
            গচ্ছ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(সংরচনা pttto, tpmopout);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    যদি (ret) {
        rc = TCG_FATAL_COM_ERROR;
        গচ্ছ err_exit;
    }

    pttto->opblength = offsetof(সংরচনা pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    যদি (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    প্রত্যাগম rc;
}

স্থির u32
shutdown_preboot_interface(শূন্য)
{
    TPM_interface_shutdown = 1;
    প্রত্যাগম 0;
}

স্থির u32
hash_log_event_int(ধ্রুব সংরচনা hlei *hlei, সংরচনা hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    সংরচনা pcpes *pcpes;

    size = hlei->ipblength;
    যদি (size != পরিমাণ(*hlei)) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        গচ্ছ err_exit;
    }

    pcpes = (সংরচনা pcpes *)hlei->logdataptr;

    যদি (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != পরিমাণ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_প্রবেশ_PARA;
        গচ্ছ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    যদি (rc)
        গচ্ছ err_exit;

    /* updating the log was fine */
    hleo->opblength = পরিমাণ(সংরচনা hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_গুনতি;

err_exit:
    যদি (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    প্রত্যাগম rc;
}

স্থির u32
hash_all_int(ধ্রুব সংরচনা hai *hai, u8 *hash)
{
    যদি (hai->ipblength != পরিমাণ(সংরচনা hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        প্রত্যাগম TCG_INVALID_প্রবেশ_PARA;

    sha1((ধ্রুব u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    প্রত্যাগম 0;
}

স্থির u32
tss_int(সংরচনা ti *ti, সংরচনা to *to)
{
    to->opblength = পরিমাণ(সংরচনা to);
    to->reserved  = 0;

    প্রত্যাগম TCG_PC_UNSUPPORTED;
}

স্থির u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    সংরচনা pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = পরিমাণ(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    যদি (rc)
        প্রত্যাগম rc;

    *edx_ptr = tpm_state.entry_গুনতি;
    প্রত্যাগম 0;
}

শূন্য VISIBLE32FLAT
tpm_interrupt_handler32(সংরচনা bregs *regs)
{
    যদি (!CONFIG_TCGBIOS)
        প্রত্যাগম;

    set_cf(regs, 0);

    যদি (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        প্রত্যাগম;
    }

    বিকল্প ((গণনা irq_ids)regs->al) {
    পক্ষ TCG_StatusCheck:
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
        ভঙ্গ;

    পক্ষ TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (সংরচনা hleei_short *)প্রবেশ_buf32(regs),
                  (সংরচনা hleeo *)output_buf32(regs));
        ভঙ্গ;

    পক্ষ TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((সংরচনা pttti *)প্রবেশ_buf32(regs),
                                    (সংরচনা pttto *)output_buf32(regs));
        ভঙ্গ;

    পক্ষ TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        ভঙ্গ;

    পক্ষ TCG_HashLogEvent:
        regs->eax = hash_log_event_int((সংরচনা hlei*)প্রবেশ_buf32(regs),
                                       (সংরচনা hleo*)output_buf32(regs));
        ভঙ্গ;

    পক্ষ TCG_HashAll:
        regs->eax =
            hash_all_int((সংরচনা hai*)প্রবেশ_buf32(regs),
                          (u8 *)output_buf32(regs));
        ভঙ্গ;

    পক্ষ TCG_TSS:
        regs->eax = tss_int((সংরচনা ti*)প্রবেশ_buf32(regs),
                            (সংরচনা to*)output_buf32(regs));
        ভঙ্গ;

    পক্ষ TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)প্রবেশ_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        ভঙ্গ;

    স্বভাব:
        set_cf(regs, 1);
    }

    প্রত্যাগম;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

প্রকারনির্দেশ u8 tpm_ppi_code;

স্থির পূর্ণাঙ্ক
tpm12_read_has_owner(পূর্ণাঙ্ক *has_owner)
{
    সংরচনা tpm_res_getcap_ownerauth oauth;
    পূর্ণাঙ্ক ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, পরিমাণ(oauth));
    যদি (ret)
        প্রত্যাগম -1;

    *has_owner = oauth.flag;

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
tpm12_enable_tpm(পূর্ণাঙ্ক enable, পূর্ণাঙ্ক verbose)
{
    সংরচনা tpm_permanent_flags pf;
    পূর্ণাঙ্ক ret = tpm12_read_permanent_flags((বর্ণ *)&pf, পরিমাণ(pf));
    যদি (ret)
        প্রত্যাগম -1;

    যদি (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        প্রত্যাগম 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    যদি (ret) {
        যদি (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        অন্যথা
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm12_activate_tpm(পূর্ণাঙ্ক activate, পূর্ণাঙ্ক allow_reset, পূর্ণাঙ্ক verbose)
{
    সংরচনা tpm_permanent_flags pf;
    পূর্ণাঙ্ক ret = tpm12_read_permanent_flags((বর্ণ *)&pf, পরিমাণ(pf));
    যদি (ret)
        প্রত্যাগম -1;

    যদি (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        প্রত্যাগম 0;

    যদি (pf.flags[PERM_FLAG_IDX_DISABLE])
        প্রত্যাগম 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    যদি (ret)
        প্রত্যাগম ret;

    যদি (activate && allow_reset) {
        যদি (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
tpm12_enable_activate(পূর্ণাঙ্ক allow_reset, পূর্ণাঙ্ক verbose)
{
    পূর্ণাঙ্ক ret = tpm12_enable_tpm(1, verbose);
    যদি (ret)
        প্রত্যাগম ret;

    প্রত্যাগম tpm12_activate_tpm(1, allow_reset, verbose);
}

স্থির পূর্ণাঙ্ক
tpm12_force_clear(পূর্ণাঙ্ক enable_activate_before, পূর্ণাঙ্ক enable_activate_after,
                  পূর্ণাঙ্ক verbose)
{
    পূর্ণাঙ্ক has_owner;
    পূর্ণাঙ্ক ret = tpm12_read_has_owner(&has_owner);
    যদি (ret)
        প্রত্যাগম -1;
    যদি (!has_owner) {
        যদি (verbose)
            printf("TPM does not have an owner.\n");
        প্রত্যাগম 0;
    }

    যদি (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        যদি (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            প্রত্যাগম ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    যদি (ret)
        প্রত্যাগম ret;

    যদি (!enable_activate_after) {
        যদি (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        প্রত্যাগম 0;
    }

    প্রত্যাগম tpm12_enable_activate(1, verbose);
}

স্থির পূর্ণাঙ্ক
tpm12_set_owner_install(পূর্ণাঙ্ক allow, পূর্ণাঙ্ক verbose)
{
    পূর্ণাঙ্ক has_owner;
    পূর্ণাঙ্ক ret = tpm12_read_has_owner(&has_owner);
    যদি (ret)
        প্রত্যাগম -1;
    যদি (has_owner) {
        যদি (verbose)
            printf("Must first remove owner.\n");
        প্রত্যাগম 0;
    }

    সংরচনা tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((বর্ণ *)&pf, পরিমাণ(pf));
    যদি (ret)
        প্রত্যাগম -1;

    যদি (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        যদি (verbose)
            printf("TPM must first be enable.\n");
        প্রত্যাগম 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    যদি (ret)
        প্রত্যাগম ret;

    যদি (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
tpm12_process_cfg(tpm_ppi_code বার্তা_Code, পূর্ণাঙ্ক verbose)
{
    পূর্ণাঙ্ক ret = 0;

    বিকল্প (বার্তা_Code) {
        পক্ষ TPM_PPI_OP_NOOP: /* no-op */
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            ভঙ্গ;

        স্বভাব:
            ভঙ্গ;
    }

    যদি (ret)
        printf("Op %d: An error occurred: 0x%x\n", বার্তা_Code, ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_clearcontrol(u8 disable, পূর্ণাঙ্ক verbose)
{
    সংরচনা tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(পরিমাণ(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(পরিমাণ(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    সংরচনা tpm_rsp_header rsp;
    u32 resp_length = পরিমাণ(rsp);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_clear(শূন্য)
{
    সংরচনা tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(পরিমাণ(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(পরিমাণ(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    সংরচনা tpm_rsp_header rsp;
    u32 resp_length = পরিমাণ(rsp);
    পূর্ণাঙ্ক ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    যদি (ret || resp_length != পরিমাণ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm20_process_cfg(tpm_ppi_code বার্তা_Code, পূর্ণাঙ্ক verbose)
{
    পূর্ণাঙ্ক ret = 0;

    বিকল্প (বার্তা_Code) {
        পক্ষ TPM_PPI_OP_NOOP: /* no-op */
            ভঙ্গ;

        পক্ষ TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            যদি (!ret)
                 ret = tpm20_clear();
            ভঙ্গ;
    }

    যদি (ret)
        printf("Op %d: An error occurred: 0x%x\n", বার্তা_Code, ret);

    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক
tpm12_get_tpm_state(শূন্য)
{
    পূর্ণাঙ্ক state = 0;
    সংরচনা tpm_permanent_flags pf;
    পূর্ণাঙ্ক has_owner;

    যদি (tpm12_read_permanent_flags((বর্ণ *)&pf, পরিমাণ(pf)) ||
        tpm12_read_has_owner(&has_owner))
        প্রত্যাগম ~0;

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

    প্রত্যাগম state;
}

স্থির শূন্য
tpm12_show_tpm_menu(পূর্ণাঙ্ক state, পূর্ণাঙ্ক next_scancodes[7])
{
    পূর্ণাঙ্ক i = 0;

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
        next_scancodes[i++] = 32;

        যদি (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            যদি (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } অন্যথা {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } অন্যথা {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } অন্যথা {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    যদি (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

স্থির শূন্য
tpm12_menu(শূন্য)
{
    পূর্ণাঙ্ক scancode, next_scancodes[7];
    tpm_ppi_code বার্তা_Code;
    পূর্ণাঙ্ক state = 0, i;
    পূর্ণাঙ্ক waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    প্রতি (;;) {
        যদি ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } অন্যথা {
            printf("TPM is not working correctly.\n");
            প্রত্যাগম;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        বার্তা_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        যাবত্ (waitkey) {
            যাবত্ ((scancode = get_keystroke(1000)) == ~0)
                ;

            বিকল্প (scancode) {
            পক্ষ 1:
                // ESC
                reset();
                ভঙ্গ;
            পক্ষ 18: /* e. enable */
                বার্তা_Code = TPM_PPI_OP_ENABLE;
                ভঙ্গ;
            পক্ষ 32: /* d. disable */
                বার্তা_Code = TPM_PPI_OP_DISABLE;
                ভঙ্গ;
            পক্ষ 30: /* a. activate */
                বার্তা_Code = TPM_PPI_OP_ACTIVATE;
                ভঙ্গ;
            পক্ষ 47: /* v. deactivate */
                বার্তা_Code = TPM_PPI_OP_DEACTIVATE;
                ভঙ্গ;
            পক্ষ 46: /* c. clear owner */
                বার্তা_Code = TPM_PPI_OP_CLEAR;
                ভঙ্গ;
            পক্ষ 25: /* p. prevent ownerinstall */
                বার্তা_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                ভঙ্গ;
            পক্ষ 31: /* s. allow ownerinstall */
                বার্তা_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                ভঙ্গ;
            স্বভাব:
                অনুবর্ত;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            প্রতি (i = 0; i < পরিমাণ(next_scancodes); i++) {
                যদি (next_scancodes[i] == 0)
                    ভঙ্গ;

                যদি (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(বার্তা_Code, 1);
                    waitkey = 0;
                    ভঙ্গ;
                }
            }
        }
    }
}

স্থির পূর্ণাঙ্ক
tpm20_menu_change_active_pcrbanks(শূন্য)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    যাবত্ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        যাবত্ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ধ্রুব বর্ণ *hashname = tpm20_hashalg_flag_to_name(flag);

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
        পূর্ণাঙ্ক show = 0;
        যাবত্ (!show) {
            পূর্ণাঙ্ক scancode = get_keystroke(1000);

            বিকল্প (scancode) {
            পক্ষ ~0:
                অনুবর্ত;
            পক্ষ 1: /* ESC */
                printf("\n");
                প্রত্যাগম -1;
            পক্ষ 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                যদি (flagnum > i)
                    অনুবর্ত;
                যদি (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                ভঙ্গ;
            পক্ষ 30: /* a */
                যদি (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

স্থির শূন্য
tpm20_menu(শূন্য)
{
    পূর্ণাঙ্ক scan_code;
    tpm_ppi_code বার্তা_Code;

    প্রতি (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        বার্তা_Code = TPM_PPI_OP_NOOP;

        যাবত্ ((scan_code = get_keystroke(1000)) == ~0)
            ;

        বিকল্প (scan_code) {
        পক্ষ 1:
            // ESC
            reset();
            ভঙ্গ;
        পক্ষ 2:
            বার্তা_Code = TPM_PPI_OP_CLEAR;
            ভঙ্গ;
        পক্ষ 3:
            tpm20_menu_change_active_pcrbanks();
            অনুবর্ত;
        স্বভাব:
            অনুবর্ত;
        }

        tpm20_process_cfg(বার্তা_Code, 0);
    }
}

শূন্য
tpm_menu(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        প্রত্যাগম;

    যাবত্ (get_keystroke(0) >= 0)
        ;
    wait_threads();

    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        tpm12_menu();
        ভঙ্গ;
    পক্ষ TPM_VERSION_2:
        tpm20_menu();
        ভঙ্গ;
    }
}

পূর্ণাঙ্ক
tpm_can_show_menu(শূন্য)
{
    বিকল্প (TPM_version) {
    পক্ষ TPM_VERSION_1_2:
        প্রত্যাগম tpm_is_working() && TPM_has_physical_presence;
    পক্ষ TPM_VERSION_2:
        প্রত্যাগম tpm_is_working();
    }
    প্রত্যাগম 0;
}
