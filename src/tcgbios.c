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

#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "byteorder.h" // cpu_to_*
#अन्तर्भाव "config.h" // CONFIG_TCGBIOS
#अन्तर्भाव "farptr.h" // MAKE_FLATPTR
#अन्तर्भाव "fw/paravirt.h" // runningOnXen
#अन्तर्भाव "hw/tpm_drivers.h" // tpm_drivers[]
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "sha.h" // sha1, sha256, ...
#अन्तर्भाव "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#अन्तर्भाव "std/smbios.h" // struct smbios_21_entry_point
#अन्तर्भाव "std/tcg.h" // TCG_PC_LOGOVERFLOW
#अन्तर्भाव "string.h" // checksum
#अन्तर्भाव "tcgbios.h"// tpm_*, prototypes
#अन्तर्भाव "util.h" // printf, get_keystroke
#अन्तर्भाव "stacks.h" // wait_threads, reset
#अन्तर्भाव "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

संरचना {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_गन्ती;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

स्थिर पूर्णाङ्क tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    यदि (!log_area_start_address || !log_area_minimum_length)
        फर्काऊ -1;

    स_रखो(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = रिक्त;
    tpm_state.entry_गन्ती = 0;
    फर्काऊ 0;
}

स्थिर पूर्णाङ्क
tpm_tcpa_probe(शून्य)
{
    संरचना tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    यदि (!tcpa)
        फर्काऊ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(दीर्घ)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    फर्काऊ tpm_set_log_area((u8*)(दीर्घ)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

स्थिर पूर्णाङ्क
tpm_tpm2_probe(शून्य)
{
    संरचना tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    यदि (!tpm2)
        फर्काऊ -1;

    यदि (tpm2->length < 76)
        फर्काऊ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(दीर्घ)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    फर्काऊ tpm_set_log_area((u8*)(दीर्घ)tpm2->log_area_start_address,
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
स्थिर पूर्णाङ्क
tpm_log_event(संरचना tpm_log_header *entry, पूर्णाङ्क digest_len
              , अचर शून्य *event, पूर्णाङ्क event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    यदि (tpm_state.log_area_next_entry == रिक्त)
        फर्काऊ -1;

    u32 size = (परिमाण(*entry) + digest_len
                + परिमाण(संरचना tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    यदि (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        फर्काऊ -1;
    }

    शून्य *dest = tpm_state.log_area_next_entry;
    स_नकल(dest, entry, परिमाण(*entry) + digest_len);
    संरचना tpm_log_trailer *t = dest + परिमाण(*entry) + digest_len;
    t->eventdatasize = event_len;
    स_नकल(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_गन्ती++;

    फर्काऊ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

स्थिर TPMVersion TPM_version;
स्थिर u32 tpm20_pcr_selection_size;
स्थिर संरचना tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
संरचना tpm_log_entry {
    संरचना tpm_log_header hdr;
    u8 pad[परिमाण(संरचना tpm2_digest_मानहरू)
           + 8 * परिमाण(संरचना tpm2_digest_मान)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

स्थिर अचर संरचना hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    अचर वर्ण *name;
    शून्य (*hashfunc)(अचर u8 *data, u32 length, u8 *hash);
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

स्थिर पूर्णाङ्क
tpm20_get_hash_buffersize(u16 hashAlg)
{
    अचिन्हित i;

    लागि (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        यदि (hash_parameters[i].hashalg == hashAlg)
            फर्काऊ hash_parameters[i].hash_buffersize;
    }
    फर्काऊ -1;
}

स्थिर u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    अचिन्हित i;

    लागि (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        यदि (hash_parameters[i].hashalg == hashAlg)
            फर्काऊ hash_parameters[i].hashalg_flag;
    }
    फर्काऊ 0;
}

स्थिर u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    अचिन्हित i;

    लागि (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        यदि (hash_parameters[i].hashalg_flag == hashalg_flag)
            फर्काऊ hash_parameters[i].hashalg;
    }
    फर्काऊ 0;
}

स्थिर अचर वर्ण *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    अचिन्हित i;

    लागि (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        यदि (hash_parameters[i].hashalg_flag == hashalg_flag)
            फर्काऊ hash_parameters[i].name;
    }
    फर्काऊ रिक्त;
}

स्थिर शून्य tpm2_hash_data(u16 hashAlg, अचर u8 *data, u32 data_len, u8 *hash)
{
    अचिन्हित i;

    लागि (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        यदि (hash_parameters[i].hashalg == hashAlg) {
            यदि (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } अन्यथा {
                स_रखो(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
स्थिर पूर्णाङ्क
tpm20_write_EfiSpecIdEventStruct(शून्य)
{
    यदि (!tpm20_pcr_selection)
        फर्काऊ -1;

    संरचना {
        संरचना TCG_EfiSpecIdEventStruct hdr;
        u8 pad[परिमाण(संरचना tpm_log_entry) + परिमाण(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    संरचना tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    शून्य *nsel, *end = (शून्य*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 गन्ती, अङ्क_Algs = 0;
    लागि (गन्ती = 0; गन्ती < be32_to_cpu(tpm20_pcr_selection->गन्ती); गन्ती++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (शून्य*)sel + परिमाण(*sel) + sizeOfSelect;
        यदि (nsel > end)
            रोक;

        यदि (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            जारी;
        }

        पूर्णाङ्क hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        यदि (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            फर्काऊ -1;
        }

        पूर्णाङ्क event_size = दुरत्व(संरचना TCG_EfiSpecIdEventStruct
                                  , digestSizes[गन्ती+1]);
        यदि (event_size > परिमाण(event) - परिमाण(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            फर्काऊ -1;
        }

        event.hdr.digestSizes[अङ्क_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[अङ्क_Algs].digestSize = hsize;
        अङ्क_Algs++;

        sel = nsel;
    }

    यदि (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        फर्काऊ -1;
    }

    event.hdr.सङ्ख्या_Of_Algorithms = अङ्क_Algs;
    पूर्णाङ्क event_size = दुरत्व(संरचना TCG_EfiSpecIdEventStruct
                              , digestSizes[अङ्क_Algs]);
    u8 *vendorInfoSize = (शून्य*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += परिमाण(*vendorInfoSize);

    संरचना tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    फर्काऊ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
स्थिर पूर्णाङ्क
tpm20_build_digest(संरचना tpm_log_entry *le,
                   अचर u8 *hashdata, u32 hashdata_len, पूर्णाङ्क bigEndian)
{
    यदि (!tpm20_pcr_selection)
        फर्काऊ -1;

    संरचना tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    शून्य *nsel, *end = (शून्य*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    शून्य *dest = le->hdr.digest + परिमाण(संरचना tpm2_digest_मानहरू);

    u32 गन्ती, अङ्क_Algs = 0;
    लागि (गन्ती = 0; गन्ती < be32_to_cpu(tpm20_pcr_selection->गन्ती); गन्ती++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (शून्य*)sel + परिमाण(*sel) + sizeOfSelect;
        यदि (nsel > end)
            रोक;

        /* PCR 0-7 unused? -- skip */
        यदि (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            जारी;
        }

        पूर्णाङ्क hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        यदि (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            फर्काऊ -1;
        }

        /* buffer size sanity check before writing */
        संरचना tpm2_digest_मान *v = dest;
        यदि (dest + परिमाण(*v) + hsize > (शून्य*)le + परिमाण(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            फर्काऊ -1;
        }

        यदि (bigEndian)
            v->hashAlg = sel->hashAlg;
        अन्यथा
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += परिमाण(*v) + hsize;
        sel = nsel;

        अङ्क_Algs++;
    }

    यदि (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        फर्काऊ -1;
    }

    संरचना tpm2_digest_मानहरू *v = (शून्य*)le->hdr.digest;
    यदि (bigEndian)
        v->गन्ती = cpu_to_be32(अङ्क_Algs);
    अन्यथा
        v->गन्ती = अङ्क_Algs;

    फर्काऊ dest - (शून्य*)le->hdr.digest;
}

स्थिर पूर्णाङ्क
tpm12_build_digest(संरचना tpm_log_entry *le,
                   अचर u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    फर्काऊ SHA1_BUFSIZE;
}

स्थिर पूर्णाङ्क
tpm12_build_digest_direct(संरचना tpm_log_entry *le, अचर u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    स_नकल(le->hdr.digest, sha1, SHA1_BUFSIZE);
    फर्काऊ SHA1_BUFSIZE;
}

स्थिर पूर्णाङ्क
tpm_build_digest(संरचना tpm_log_entry *le, अचर u8 *hashdata, u32 hashdata_len
                 , पूर्णाङ्क bigEndian)
{
    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        फर्काऊ tpm12_build_digest(le, hashdata, hashdata_len);
    पक्ष TPM_VERSION_2:
        फर्काऊ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    फर्काऊ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
स्थिर पूर्णाङ्क
tpm_simple_cmd(u8 locty, u32 ordinal
               , पूर्णाङ्क param_size, u16 param, गणना tpmDurationType to_t)
{
    संरचना {
        संरचना tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(परिमाण(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        रोक;
    पक्ष TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        रोक;
    }

    u8 obuffer[64];
    संरचना tpm_rsp_header *trsh = (शून्य*)obuffer;
    u32 obuffer_len = परिमाण(obuffer);
    स_रखो(obuffer, 0x0, परिमाण(obuffer));

    पूर्णाङ्क ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_getcapability(u32 capability, u32 property, u32 गन्ती,
                    संरचना tpm_rsp_header *rsp, u32 rsize)
{
    संरचना tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(परिमाण(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(गन्ती),
    };

    u32 resp_size = rsize;
    पूर्णाङ्क ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_get_pcrbanks(शून्य)
{
    u8 buffer[128];
    संरचना tpm2_res_getcapability *trg =
      (संरचना tpm2_res_getcapability *)&buffer;

    पूर्णाङ्क ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  परिमाण(buffer));
    यदि (ret)
        फर्काऊ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    यदि (resplen <= दुरत्व(संरचना tpm2_res_getcapability, data))
        फर्काऊ -1;

    u32 size = resplen - दुरत्व(संरचना tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    यदि (size < दुरत्व(संरचना tpml_pcr_selection, selections) +
               दुरत्व(संरचना tpms_pcr_selection, pcrSelect))
        फर्काऊ -1;

    tpm20_pcr_selection = malloc_माथिल्लो(size);
    यदि (tpm20_pcr_selection) {
        स_नकल(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } अन्यथा {
        warn_noalloc();
        ret = -1;
    }

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    यदि (!tpm20_pcr_selection)
        फर्काऊ -1;

    संरचना tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    शून्य *end = (शून्य*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    जबसम्म (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        शून्य *nsel = (शून्य*)sel + परिमाण(*sel) + sizeOfSelect;
        यदि (nsel > end)
            फर्काऊ 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        अचिन्हित i;
        लागि (i = 0; i < sizeOfSelect; i++) {
            यदि (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                रोक;
            }
        }

        sel = nsel;
    }
}

स्थिर पूर्णाङ्क
tpm20_set_pcrbanks(u32 active_banks)
{
    संरचना tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(परिमाण(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    संरचना tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    पूर्णाङ्क i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    जबसम्म (hashalg_flag) {
        यदि ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            यदि (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                यदि (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.गन्ती = cpu_to_be32(i);
    स_नकल(trpa.tpms_pcr_selections, tps, i * परिमाण(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(दुरत्व(संरचना tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * परिमाण(tps[0]));

    संरचना tpm_rsp_header rsp;
    u32 resp_length = परिमाण(rsp);

    पूर्णाङ्क ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क tpm20_activate_pcrbanks(u32 active_banks)
{
    पूर्णाङ्क ret = tpm20_set_pcrbanks(active_banks);
    यदि (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    यदि (!ret)
        reset();
    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm12_get_capability(u32 cap, u32 subcap, संरचना tpm_rsp_header *rsp, u32 rsize)
{
    संरचना tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(परिमाण(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(परिमाण(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    पूर्णाङ्क ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm12_read_permanent_flags(वर्ण *buf, पूर्णाङ्क buf_len)
{
    स_रखो(buf, 0, buf_len);

    संरचना tpm_res_getcap_perm_flags pf;
    पूर्णाङ्क ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, परिमाण(pf));
    यदि (ret)
        फर्काऊ -1;

    स_नकल(buf, &pf.perm_flags, buf_len);

    फर्काऊ 0;
}

स्थिर पूर्णाङ्क
tpm12_determine_timeouts(शून्य)
{
    संरचना tpm_res_getcap_timeouts timeouts;
    पूर्णाङ्क ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, परिमाण(timeouts));
    यदि (ret)
        फर्काऊ ret;

    संरचना tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, परिमाण(durations));
    यदि (ret)
        फर्काऊ ret;

    पूर्णाङ्क i;
    लागि (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    लागि (i = 0; i < 4; i++)
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

    फर्काऊ 0;
}

स्थिर शून्य
tpm20_set_timeouts(शून्य)
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

स्थिर पूर्णाङ्क
tpm12_extend(संरचना tpm_log_entry *le, पूर्णाङ्क digest_len)
{
    संरचना tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(परिमाण(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    स_नकल(tre.digest, le->hdr.digest, परिमाण(tre.digest));

    संरचना tpm_rsp_extend rsp;
    u32 resp_length = परिमाण(rsp);
    पूर्णाङ्क ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.hdr.errcode)
        फर्काऊ -1;

    फर्काऊ 0;
}

स्थिर पूर्णाङ्क tpm20_extend(संरचना tpm_log_entry *le, पूर्णाङ्क digest_len)
{
    संरचना tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(परिमाण(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[परिमाण(tmp_tre) + परिमाण(le->pad)];
    संरचना tpm2_req_extend *tre = (संरचना tpm2_req_extend *)buffer;

    स_नकल(tre, &tmp_tre, परिमाण(tmp_tre));
    स_नकल(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(परिमाण(tmp_tre) + digest_len);

    संरचना tpm_rsp_header rsp;
    u32 resp_length = परिमाण(rsp);
    पूर्णाङ्क ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.errcode)
        फर्काऊ -1;

    फर्काऊ 0;
}

स्थिर पूर्णाङ्क
tpm_extend(संरचना tpm_log_entry *le, पूर्णाङ्क digest_len)
{
    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        फर्काऊ tpm12_extend(le, digest_len);
    पक्ष TPM_VERSION_2:
        फर्काऊ tpm20_extend(le, digest_len);
    }
    फर्काऊ -1;
}

स्थिर पूर्णाङ्क
tpm20_stirrandom(शून्य)
{
    संरचना tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(परिमाण(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(परिमाण(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    संरचना tpm_rsp_header rsp;
    u32 resp_length = परिमाण(rsp);
    पूर्णाङ्क ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    संरचना tpm2_res_getrandom rsp;

    यदि (buf_len > परिमाण(rsp.rnd.buffer))
        फर्काऊ -1;

    संरचना tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(परिमाण(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = परिमाण(rsp);

    पूर्णाङ्क ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.hdr.errcode)
        ret = -1;
    अन्यथा
        स_नकल(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    संरचना tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(परिमाण(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(परिमाण(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    संरचना tpm_rsp_header rsp;
    u32 resp_length = परिमाण(rsp);
    पूर्णाङ्क ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_hierarchychangeauth(u8 auth[20])
{
    संरचना tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(परिमाण(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(परिमाण(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(परिमाण(trhca.newAuth.buffer)),
        },
    };
    स_नकल(trhca.newAuth.buffer, auth, परिमाण(trhca.newAuth.buffer));

    संरचना tpm_rsp_header rsp;
    u32 resp_length = परिमाण(rsp);
    पूर्णाङ्क ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    फर्काऊ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

स्थिर पूर्णाङ्क TPM_has_physical_presence;
u8 TPM_working VARLOW;

स्थिर पूर्णाङ्क
tpm_is_working(शून्य)
{
    फर्काऊ CONFIG_TCGBIOS && TPM_working;
}

स्थिर शून्य
tpm_set_failure(शून्य)
{
    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        रोक;
    पक्ष TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        रोक;
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
स्थिर शून्य
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           अचर वर्ण *event, u32 event_length,
                           अचर u8 *hashdata, u32 hashdata_length)
{
    यदि (!tpm_is_working())
        फर्काऊ;

    संरचना tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    पूर्णाङ्क digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    यदि (digest_len < 0)
        फर्काऊ;
    पूर्णाङ्क ret = tpm_extend(&le, digest_len);
    यदि (ret) {
        tpm_set_failure();
        फर्काऊ;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
स्थिर शून्य
tpm_add_action(u32 pcrIndex, अचर वर्ण *string)
{
    u32 len = म_माप(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
स्थिर शून्य
tpm_add_event_separators(शून्य)
{
    स्थिर अचर u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    लागि (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (अचर वर्ण *)evt_separator,
                                   परिमाण(evt_separator),
                                   evt_separator,
                                   परिमाण(evt_separator));
}

स्थिर शून्य
tpm_smbios_measure(शून्य)
{
    संरचना pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    शून्य *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    यदि (!smbios_tables)
        फर्काऊ;

    sha1((अचर u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (अचर वर्ण *)&pcctes, परिमाण(pcctes),
                               (u8 *)&pcctes, परिमाण(pcctes));
}

स्थिर पूर्णाङ्क
tpm12_assert_physical_presence(शून्य)
{
    पूर्णाङ्क ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    यदि (!ret)
        फर्काऊ 0;

    संरचना tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((वर्ण *)&pf, परिमाण(pf));
    यदि (ret)
        फर्काऊ -1;

    /* check if hardware physical presence is supported */
    यदि (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        फर्काऊ 0;
    }

    यदि (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        फर्काऊ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    फर्काऊ -1;
}

स्थिर पूर्णाङ्क
tpm12_startup(शून्य)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    पूर्णाङ्क ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    यदि (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    यदि (ret)
        गच्छ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    यदि (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    यदि (ret)
        गच्छ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    यदि (ret)
        गच्छ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    यदि (ret && ret != TPM_BAD_LOCALITY)
        गच्छ err_exit;

    फर्काऊ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    फर्काऊ -1;
}

स्थिर पूर्णाङ्क
tpm20_startup(शून्य)
{
    tpm20_set_timeouts();

    पूर्णाङ्क ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    यदि (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    यदि (ret)
        गच्छ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    यदि (ret)
        गच्छ err_exit;

    ret = tpm20_get_pcrbanks();
    यदि (ret)
        गच्छ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    यदि (ret)
        गच्छ err_exit;

    फर्काऊ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    फर्काऊ -1;
}

स्थिर पूर्णाङ्क
tpm_startup(शून्य)
{
    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        फर्काऊ tpm12_startup();
    पक्ष TPM_VERSION_2:
        फर्काऊ tpm20_startup();
    }
    फर्काऊ -1;
}

शून्य
tpm_setup(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        फर्काऊ;

    पूर्णाङ्क ret = tpm_tpm2_probe();
    यदि (ret) {
        ret = tpm_tcpa_probe();
        यदि (ret)
            फर्काऊ;
    }

    TPM_version = tpmhw_probe();
    यदि (TPM_version == TPM_VERSION_NONE)
        फर्काऊ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    यदि (runningOnXen())
        फर्काऊ;

    ret = tpm_startup();
    यदि (ret)
        फर्काऊ;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

स्थिर शून्य
tpm20_prepboot(शून्य)
{
    पूर्णाङ्क ret = tpm20_stirrandom();
    यदि (ret)
         गच्छ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], परिमाण(auth));
    यदि (ret)
        गच्छ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    यदि (ret)
        गच्छ err_exit;

    फर्काऊ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

शून्य
tpm_prepboot(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        फर्काऊ;

    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        यदि (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        रोक;
    पक्ष TPM_VERSION_2:
        tpm20_prepboot();
        रोक;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
शून्य
tpm_option_rom(अचर शून्य *addr, u32 len)
{
    यदि (!tpm_is_working())
        फर्काऊ;

    संरचना pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = परिमाण(u16) + परिमाण(u16) + SHA1_BUFSIZE,
    };
    sha1((अचर u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (अचर वर्ण *)&pcctes, परिमाण(pcctes),
                               (u8 *)&pcctes, परिमाण(pcctes));
}

शून्य
tpm_add_bcv(u32 bootdrv, अचर u8 *addr, u32 length)
{
    यदि (!tpm_is_working())
        फर्काऊ;

    यदि (length < 0x200)
        फर्काऊ;

    अचर वर्ण *string = "Booting BCV device 00h (Floppy)";
    यदि (bootdrv == 0x80)
        string = "Booting BCV device 80h (HDD)";
    tpm_add_action(4, string);

    /* specs: see section 'Hard Disk Device or Hard Disk-Like Devices' */
    /* equivalent to: dd if=/dev/hda ibs=1 count=440 | sha1sum */
    string = "MBR";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, म_माप(string),
                               addr, 0x1b8);

    /* equivalent to: dd if=/dev/hda ibs=1 count=72 skip=440 | sha1sum */
    string = "MBR PARTITION_TABLE";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, म_माप(string),
                               addr + 0x1b8, 0x48);
}

शून्य
tpm_add_cdrom(u32 bootdrv, अचर u8 *addr, u32 length)
{
    यदि (!tpm_is_working())
        फर्काऊ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    अचर वर्ण *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, म_माप(string),
                               addr, length);
}

शून्य
tpm_add_cdrom_catalog(अचर u8 *addr, u32 length)
{
    यदि (!tpm_is_working())
        फर्काऊ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    अचर वर्ण *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, म_माप(string),
                               addr, length);
}

शून्य
tpm_s3_resume(शून्य)
{
    यदि (!tpm_is_working())
        फर्काऊ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    पूर्णाङ्क ret = -1;

    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        रोक;
    पक्ष TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        यदि (ret)
            गच्छ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        रोक;
    }

    यदि (ret)
        गच्छ err_exit;

    फर्काऊ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

स्थिर अन्तःस्थ शून्य *आगत_buf32(संरचना bregs *regs)
{
    फर्काऊ MAKE_FLATPTR(regs->es, regs->di);
}

स्थिर अन्तःस्थ शून्य *output_buf32(संरचना bregs *regs)
{
    फर्काऊ MAKE_FLATPTR(regs->ds, regs->si);
}

स्थिर u32
hash_log_extend(संरचना pcpes *pcpes, अचर शून्य *hashdata, u32 hashdata_length
                , शून्य *event, पूर्णाङ्क extend)
{
    यदि (pcpes->pcrindex >= 24)
        फर्काऊ TCG_INVALID_आगत_PARA;
    यदि (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    संरचना tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    पूर्णाङ्क digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    यदि (digest_len < 0)
        फर्काऊ TCG_GENERAL_ERROR;
    यदि (extend) {
        पूर्णाङ्क ret = tpm_extend(&le, digest_len);
        यदि (ret)
            फर्काऊ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    पूर्णाङ्क ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    यदि (ret)
        फर्काऊ TCG_PC_LOGOVERFLOW;
    फर्काऊ 0;
}

स्थिर u32
hash_log_extend_event_int(अचर संरचना hleei_short *hleei_s,
                          संरचना hleeo *hleeo)
{
    u32 rc = 0;
    संरचना hleei_long *hleei_l = (संरचना hleei_long *)hleei_s;
    अचर शून्य *logdataptr;
    u32 logdatalen;
    संरचना pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    विकल्प (hleei_s->ipblength) {
    पक्ष परिमाण(संरचना hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    रोक;

    पक्ष परिमाण(संरचना hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    रोक;

    स्वभाव:
        /* bad input block */
        rc = TCG_INVALID_आगत_PARA;
        गच्छ err_exit;
    }

    pcpes = (संरचना pcpes *)logdataptr;

    यदि (pcpes->pcrindex != pcrindex
        || logdatalen != परिमाण(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_आगत_PARA;
        गच्छ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    यदि (rc)
        गच्छ err_exit;

    hleeo->opblength = परिमाण(संरचना hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_गन्ती;
    स_नकल(hleeo->digest, pcpes->digest, परिमाण(hleeo->digest));

err_exit:
    यदि (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    फर्काऊ rc;
}

स्थिर u32
pass_through_to_tpm_int(संरचना pttti *pttti, संरचना pttto *pttto)
{
    u32 rc = 0;
    संरचना tpm_req_header *trh = (शून्य*)pttti->tpmopin;

    यदि (pttti->ipblength < परिमाण(संरचना pttti) + परिमाण(*trh)
        || pttti->ipblength != परिमाण(संरचना pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < परिमाण(संरचना pttto)) {
        rc = TCG_INVALID_आगत_PARA;
        गच्छ err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        यदि (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_आगत_PARA;
            गच्छ err_exit;
        }
        रोक;
    पक्ष TPM_VERSION_2:
        यदि (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_आगत_PARA;
            गच्छ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - दुरत्व(संरचना pttto, tpmopout);
    पूर्णाङ्क ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    यदि (ret) {
        rc = TCG_FATAL_COM_ERROR;
        गच्छ err_exit;
    }

    pttto->opblength = दुरत्व(संरचना pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    यदि (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    फर्काऊ rc;
}

स्थिर u32
shutdown_preboot_interface(शून्य)
{
    TPM_interface_shutdown = 1;
    फर्काऊ 0;
}

स्थिर u32
hash_log_event_int(अचर संरचना hlei *hlei, संरचना hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    संरचना pcpes *pcpes;

    size = hlei->ipblength;
    यदि (size != परिमाण(*hlei)) {
        rc = TCG_INVALID_आगत_PARA;
        गच्छ err_exit;
    }

    pcpes = (संरचना pcpes *)hlei->logdataptr;

    यदि (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != परिमाण(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_आगत_PARA;
        गच्छ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    यदि (rc)
        गच्छ err_exit;

    /* updating the log was fine */
    hleo->opblength = परिमाण(संरचना hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_गन्ती;

err_exit:
    यदि (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    फर्काऊ rc;
}

स्थिर u32
hash_all_int(अचर संरचना hai *hai, u8 *hash)
{
    यदि (hai->ipblength != परिमाण(संरचना hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        फर्काऊ TCG_INVALID_आगत_PARA;

    sha1((अचर u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    फर्काऊ 0;
}

स्थिर u32
tss_int(संरचना ti *ti, संरचना to *to)
{
    to->opblength = परिमाण(संरचना to);
    to->reserved  = 0;

    फर्काऊ TCG_PC_UNSUPPORTED;
}

स्थिर u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    संरचना pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = परिमाण(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    यदि (rc)
        फर्काऊ rc;

    *edx_ptr = tpm_state.entry_गन्ती;
    फर्काऊ 0;
}

शून्य VISIBLE32FLAT
tpm_interrupt_handler32(संरचना bregs *regs)
{
    यदि (!CONFIG_TCGBIOS)
        फर्काऊ;

    set_cf(regs, 0);

    यदि (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        फर्काऊ;
    }

    विकल्प ((गणना irq_ids)regs->al) {
    पक्ष TCG_StatusCheck:
        यदि (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } अन्यथा {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        रोक;

    पक्ष TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (संरचना hleei_short *)आगत_buf32(regs),
                  (संरचना hleeo *)output_buf32(regs));
        रोक;

    पक्ष TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((संरचना pttti *)आगत_buf32(regs),
                                    (संरचना pttto *)output_buf32(regs));
        रोक;

    पक्ष TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        रोक;

    पक्ष TCG_HashLogEvent:
        regs->eax = hash_log_event_int((संरचना hlei*)आगत_buf32(regs),
                                       (संरचना hleo*)output_buf32(regs));
        रोक;

    पक्ष TCG_HashAll:
        regs->eax =
            hash_all_int((संरचना hai*)आगत_buf32(regs),
                          (u8 *)output_buf32(regs));
        रोक;

    पक्ष TCG_TSS:
        regs->eax = tss_int((संरचना ti*)आगत_buf32(regs),
                            (संरचना to*)output_buf32(regs));
        रोक;

    पक्ष TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)आगत_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        रोक;

    स्वभाव:
        set_cf(regs, 1);
    }

    फर्काऊ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

प्रकारनिर्देश u8 tpm_ppi_code;

स्थिर पूर्णाङ्क
tpm12_read_has_owner(पूर्णाङ्क *has_owner)
{
    संरचना tpm_res_getcap_ownerauth oauth;
    पूर्णाङ्क ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, परिमाण(oauth));
    यदि (ret)
        फर्काऊ -1;

    *has_owner = oauth.flag;

    फर्काऊ 0;
}

स्थिर पूर्णाङ्क
tpm12_enable_tpm(पूर्णाङ्क enable, पूर्णाङ्क verbose)
{
    संरचना tpm_permanent_flags pf;
    पूर्णाङ्क ret = tpm12_read_permanent_flags((वर्ण *)&pf, परिमाण(pf));
    यदि (ret)
        फर्काऊ -1;

    यदि (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        फर्काऊ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    यदि (ret) {
        यदि (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        अन्यथा
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm12_activate_tpm(पूर्णाङ्क activate, पूर्णाङ्क allow_reset, पूर्णाङ्क verbose)
{
    संरचना tpm_permanent_flags pf;
    पूर्णाङ्क ret = tpm12_read_permanent_flags((वर्ण *)&pf, परिमाण(pf));
    यदि (ret)
        फर्काऊ -1;

    यदि (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        फर्काऊ 0;

    यदि (pf.flags[PERM_FLAG_IDX_DISABLE])
        फर्काऊ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    यदि (ret)
        फर्काऊ ret;

    यदि (activate && allow_reset) {
        यदि (verbose) {
            म_लिखो("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    फर्काऊ 0;
}

स्थिर पूर्णाङ्क
tpm12_enable_activate(पूर्णाङ्क allow_reset, पूर्णाङ्क verbose)
{
    पूर्णाङ्क ret = tpm12_enable_tpm(1, verbose);
    यदि (ret)
        फर्काऊ ret;

    फर्काऊ tpm12_activate_tpm(1, allow_reset, verbose);
}

स्थिर पूर्णाङ्क
tpm12_force_clear(पूर्णाङ्क enable_activate_before, पूर्णाङ्क enable_activate_after,
                  पूर्णाङ्क verbose)
{
    पूर्णाङ्क has_owner;
    पूर्णाङ्क ret = tpm12_read_has_owner(&has_owner);
    यदि (ret)
        फर्काऊ -1;
    यदि (!has_owner) {
        यदि (verbose)
            म_लिखो("TPM does not have an owner.\n");
        फर्काऊ 0;
    }

    यदि (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        यदि (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            फर्काऊ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    यदि (ret)
        फर्काऊ ret;

    यदि (!enable_activate_after) {
        यदि (verbose)
            म_लिखो("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        फर्काऊ 0;
    }

    फर्काऊ tpm12_enable_activate(1, verbose);
}

स्थिर पूर्णाङ्क
tpm12_set_owner_install(पूर्णाङ्क allow, पूर्णाङ्क verbose)
{
    पूर्णाङ्क has_owner;
    पूर्णाङ्क ret = tpm12_read_has_owner(&has_owner);
    यदि (ret)
        फर्काऊ -1;
    यदि (has_owner) {
        यदि (verbose)
            म_लिखो("Must first remove owner.\n");
        फर्काऊ 0;
    }

    संरचना tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((वर्ण *)&pf, परिमाण(pf));
    यदि (ret)
        फर्काऊ -1;

    यदि (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        यदि (verbose)
            म_लिखो("TPM must first be enable.\n");
        फर्काऊ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    यदि (ret)
        फर्काऊ ret;

    यदि (verbose)
        म_लिखो("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    फर्काऊ 0;
}

स्थिर पूर्णाङ्क
tpm12_process_cfg(tpm_ppi_code सन्देश_Code, पूर्णाङ्क verbose)
{
    पूर्णाङ्क ret = 0;

    विकल्प (सन्देश_Code) {
        पक्ष TPM_PPI_OP_NOOP: /* no-op */
            रोक;

        पक्ष TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            रोक;

        पक्ष TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            रोक;

        पक्ष TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            रोक;

        पक्ष TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            रोक;

        पक्ष TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            रोक;

        पक्ष TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            रोक;

        पक्ष TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            रोक;

        स्वभाव:
            रोक;
    }

    यदि (ret)
        म_लिखो("Op %d: An error occurred: 0x%x\n", सन्देश_Code, ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_clearcontrol(u8 disable, पूर्णाङ्क verbose)
{
    संरचना tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(परिमाण(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(परिमाण(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    संरचना tpm_rsp_header rsp;
    u32 resp_length = परिमाण(rsp);
    पूर्णाङ्क ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_clear(शून्य)
{
    संरचना tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(परिमाण(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(परिमाण(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    संरचना tpm_rsp_header rsp;
    u32 resp_length = परिमाण(rsp);
    पूर्णाङ्क ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    यदि (ret || resp_length != परिमाण(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm20_process_cfg(tpm_ppi_code सन्देश_Code, पूर्णाङ्क verbose)
{
    पूर्णाङ्क ret = 0;

    विकल्प (सन्देश_Code) {
        पक्ष TPM_PPI_OP_NOOP: /* no-op */
            रोक;

        पक्ष TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            यदि (!ret)
                 ret = tpm20_clear();
            रोक;
    }

    यदि (ret)
        म_लिखो("Op %d: An error occurred: 0x%x\n", सन्देश_Code, ret);

    फर्काऊ ret;
}

स्थिर पूर्णाङ्क
tpm12_get_tpm_state(शून्य)
{
    पूर्णाङ्क state = 0;
    संरचना tpm_permanent_flags pf;
    पूर्णाङ्क has_owner;

    यदि (tpm12_read_permanent_flags((वर्ण *)&pf, परिमाण(pf)) ||
        tpm12_read_has_owner(&has_owner))
        फर्काऊ ~0;

    यदि (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    यदि (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    यदि (has_owner) {
        state |= TPM_STATE_OWNED;
    } अन्यथा {
        यदि (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    फर्काऊ state;
}

स्थिर शून्य
tpm12_show_tpm_menu(पूर्णाङ्क state, पूर्णाङ्क next_scancodes[7])
{
    पूर्णाङ्क i = 0;

    म_लिखो("\nThe current state of the TPM is:\n");

    यदि (state & TPM_STATE_ENABLED)
        म_लिखो("  Enabled");
    अन्यथा
        म_लिखो("  Disabled");

    यदि (state & TPM_STATE_ACTIVE)
        म_लिखो(" and active\n");
    अन्यथा
        म_लिखो(" and deactivated\n");

    यदि (state & TPM_STATE_OWNED)
        म_लिखो("  Ownership has been taken\n");
    अन्यथा {
        म_लिखो("  Ownership has not been taken\n");
        यदि (state & TPM_STATE_OWNERINSTALL)
            म_लिखो("  A user can take ownership of the TPM\n");
        अन्यथा
            म_लिखो("  Taking ownership of the TPM has been disabled\n");
    }

    यदि ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        म_लिखो("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    म_लिखो("\nAvailable options are:\n");
    यदि (state & TPM_STATE_ENABLED) {
        म_लिखो(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        यदि (state & TPM_STATE_ACTIVE) {
            म_लिखो(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            यदि (state & TPM_STATE_OWNERINSTALL) {
                म_लिखो(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } अन्यथा {
                म_लिखो(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } अन्यथा {
            म_लिखो(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } अन्यथा {
        म_लिखो(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    यदि (state & TPM_STATE_OWNED) {
        म_लिखो(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

स्थिर शून्य
tpm12_menu(शून्य)
{
    पूर्णाङ्क scancode, next_scancodes[7];
    tpm_ppi_code सन्देश_Code;
    पूर्णाङ्क state = 0, i;
    पूर्णाङ्क waitkey;

    म_लिखो("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    लागि (;;) {
        यदि ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } अन्यथा {
            म_लिखो("TPM is not working correctly.\n");
            फर्काऊ;
        }

        म_लिखो("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        सन्देश_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        जबसम्म (waitkey) {
            जबसम्म ((scancode = get_keystroke(1000)) == ~0)
                ;

            विकल्प (scancode) {
            पक्ष 1:
                // ESC
                reset();
                रोक;
            पक्ष 18: /* e. enable */
                सन्देश_Code = TPM_PPI_OP_ENABLE;
                रोक;
            पक्ष 32: /* d. disable */
                सन्देश_Code = TPM_PPI_OP_DISABLE;
                रोक;
            पक्ष 30: /* a. activate */
                सन्देश_Code = TPM_PPI_OP_ACTIVATE;
                रोक;
            पक्ष 47: /* v. deactivate */
                सन्देश_Code = TPM_PPI_OP_DEACTIVATE;
                रोक;
            पक्ष 46: /* c. clear owner */
                सन्देश_Code = TPM_PPI_OP_CLEAR;
                रोक;
            पक्ष 25: /* p. prevent ownerinstall */
                सन्देश_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                रोक;
            पक्ष 31: /* s. allow ownerinstall */
                सन्देश_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                रोक;
            स्वभाव:
                जारी;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            लागि (i = 0; i < परिमाण(next_scancodes); i++) {
                यदि (next_scancodes[i] == 0)
                    रोक;

                यदि (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(सन्देश_Code, 1);
                    waitkey = 0;
                    रोक;
                }
            }
        }
    }
}

स्थिर पूर्णाङ्क
tpm20_menu_change_active_pcrbanks(शून्य)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    जबसम्म (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        म_लिखो("\nToggle active PCR banks by pressing number key\n\n");

        जबसम्म (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            अचर वर्ण *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            यदि (hashname) {
                म_लिखो("  %d: %s", i, hashname);
                यदि (activate_banks & hashalg_flag)
                    म_लिखो(" (enabled)");
                म_लिखो("\n");
            }

            hashalg_flag <<= 1;
        }
        म_लिखो("\n"
               "ESC: return to previous menu without changes\n");
        यदि (activate_banks)
            म_लिखो("A  : activate selection\n");

        u8 flagnum;
        पूर्णाङ्क show = 0;
        जबसम्म (!show) {
            पूर्णाङ्क scancode = get_keystroke(1000);

            विकल्प (scancode) {
            पक्ष ~0:
                जारी;
            पक्ष 1: /* ESC */
                म_लिखो("\n");
                फर्काऊ -1;
            पक्ष 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                यदि (flagnum > i)
                    जारी;
                यदि (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                रोक;
            पक्ष 30: /* a */
                यदि (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

स्थिर शून्य
tpm20_menu(शून्य)
{
    पूर्णाङ्क scan_code;
    tpm_ppi_code सन्देश_Code;

    लागि (;;) {
        म_लिखो("1. Clear TPM\n");
        म_लिखो("2. Change active PCR banks\n");

        म_लिखो("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        सन्देश_Code = TPM_PPI_OP_NOOP;

        जबसम्म ((scan_code = get_keystroke(1000)) == ~0)
            ;

        विकल्प (scan_code) {
        पक्ष 1:
            // ESC
            reset();
            रोक;
        पक्ष 2:
            सन्देश_Code = TPM_PPI_OP_CLEAR;
            रोक;
        पक्ष 3:
            tpm20_menu_change_active_pcrbanks();
            जारी;
        स्वभाव:
            जारी;
        }

        tpm20_process_cfg(सन्देश_Code, 0);
    }
}

शून्य
tpm_menu(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        फर्काऊ;

    जबसम्म (get_keystroke(0) >= 0)
        ;
    wait_threads();

    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        tpm12_menu();
        रोक;
    पक्ष TPM_VERSION_2:
        tpm20_menu();
        रोक;
    }
}

पूर्णाङ्क
tpm_can_show_menu(शून्य)
{
    विकल्प (TPM_version) {
    पक्ष TPM_VERSION_1_2:
        फर्काऊ tpm_is_working() && TPM_has_physical_presence;
    पक्ष TPM_VERSION_2:
        फर्काऊ tpm_is_working();
    }
    फर्काऊ 0;
}
