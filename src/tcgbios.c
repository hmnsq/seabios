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
    u32           entry_मोजणी;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

स्थिर पूर्णांक tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    जर (!log_area_start_address || !log_area_minimum_length)
        परत -1;

    स_रखो(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = रिक्त;
    tpm_state.entry_मोजणी = 0;
    परत 0;
}

स्थिर पूर्णांक
tpm_tcpa_probe(शून्य)
{
    संरचना tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    जर (!tcpa)
        परत -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(दीर्घ)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    परत tpm_set_log_area((u8*)(दीर्घ)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

स्थिर पूर्णांक
tpm_tpm2_probe(शून्य)
{
    संरचना tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    जर (!tpm2)
        परत -1;

    जर (tpm2->length < 76)
        परत -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(दीर्घ)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    परत tpm_set_log_area((u8*)(दीर्घ)tpm2->log_area_start_address,
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
स्थिर पूर्णांक
tpm_log_event(संरचना tpm_log_header *entry, पूर्णांक digest_len
              , स्थिरांक शून्य *event, पूर्णांक event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    जर (tpm_state.log_area_next_entry == रिक्त)
        परत -1;

    u32 size = (आकार(*entry) + digest_len
                + आकार(संरचना tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    जर (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        परत -1;
    }

    शून्य *dest = tpm_state.log_area_next_entry;
    स_नकल(dest, entry, आकार(*entry) + digest_len);
    संरचना tpm_log_trailer *t = dest + आकार(*entry) + digest_len;
    t->eventdatasize = event_len;
    स_नकल(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_मोजणी++;

    परत 0;
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
    u8 pad[आकार(संरचना tpm2_digest_मूल्ये)
           + 8 * आकार(संरचना tpm2_digest_मूल्य)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

स्थिर स्थिरांक संरचना hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    स्थिरांक अक्षर *name;
    शून्य (*hashfunc)(स्थिरांक u8 *data, u32 length, u8 *hash);
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

स्थिर पूर्णांक
tpm20_get_hash_buffersize(u16 hashAlg)
{
    अचिन्हित i;

    साठी (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        जर (hash_parameters[i].hashalg == hashAlg)
            परत hash_parameters[i].hash_buffersize;
    }
    परत -1;
}

स्थिर u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    अचिन्हित i;

    साठी (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        जर (hash_parameters[i].hashalg == hashAlg)
            परत hash_parameters[i].hashalg_flag;
    }
    परत 0;
}

स्थिर u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    अचिन्हित i;

    साठी (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        जर (hash_parameters[i].hashalg_flag == hashalg_flag)
            परत hash_parameters[i].hashalg;
    }
    परत 0;
}

स्थिर स्थिरांक अक्षर *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    अचिन्हित i;

    साठी (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        जर (hash_parameters[i].hashalg_flag == hashalg_flag)
            परत hash_parameters[i].name;
    }
    परत रिक्त;
}

स्थिर शून्य tpm2_hash_data(u16 hashAlg, स्थिरांक u8 *data, u32 data_len, u8 *hash)
{
    अचिन्हित i;

    साठी (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        जर (hash_parameters[i].hashalg == hashAlg) {
            जर (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } अन्यथा {
                स_रखो(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
स्थिर पूर्णांक
tpm20_write_EfiSpecIdEventStruct(शून्य)
{
    जर (!tpm20_pcr_selection)
        परत -1;

    संरचना {
        संरचना TCG_EfiSpecIdEventStruct hdr;
        u8 pad[आकार(संरचना tpm_log_entry) + आकार(u8)];
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

    u32 मोजणी, अंक_Algs = 0;
    साठी (मोजणी = 0; मोजणी < be32_to_cpu(tpm20_pcr_selection->मोजणी); मोजणी++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (शून्य*)sel + आकार(*sel) + sizeOfSelect;
        जर (nsel > end)
            थांबा;

        जर (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            सुरूठेवा;
        }

        पूर्णांक hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        जर (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            परत -1;
        }

        पूर्णांक event_size = दुरत्व(संरचना TCG_EfiSpecIdEventStruct
                                  , digestSizes[मोजणी+1]);
        जर (event_size > आकार(event) - आकार(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            परत -1;
        }

        event.hdr.digestSizes[अंक_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[अंक_Algs].digestSize = hsize;
        अंक_Algs++;

        sel = nsel;
    }

    जर (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        परत -1;
    }

    event.hdr.संख्या_Of_Algorithms = अंक_Algs;
    पूर्णांक event_size = दुरत्व(संरचना TCG_EfiSpecIdEventStruct
                              , digestSizes[अंक_Algs]);
    u8 *vendorInfoSize = (शून्य*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += आकार(*vendorInfoSize);

    संरचना tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    परत tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
स्थिर पूर्णांक
tpm20_build_digest(संरचना tpm_log_entry *le,
                   स्थिरांक u8 *hashdata, u32 hashdata_len, पूर्णांक bigEndian)
{
    जर (!tpm20_pcr_selection)
        परत -1;

    संरचना tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    शून्य *nsel, *end = (शून्य*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    शून्य *dest = le->hdr.digest + आकार(संरचना tpm2_digest_मूल्ये);

    u32 मोजणी, अंक_Algs = 0;
    साठी (मोजणी = 0; मोजणी < be32_to_cpu(tpm20_pcr_selection->मोजणी); मोजणी++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (शून्य*)sel + आकार(*sel) + sizeOfSelect;
        जर (nsel > end)
            थांबा;

        /* PCR 0-7 unused? -- skip */
        जर (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            सुरूठेवा;
        }

        पूर्णांक hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        जर (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            परत -1;
        }

        /* buffer size sanity check before writing */
        संरचना tpm2_digest_मूल्य *v = dest;
        जर (dest + आकार(*v) + hsize > (शून्य*)le + आकार(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            परत -1;
        }

        जर (bigEndian)
            v->hashAlg = sel->hashAlg;
        अन्यथा
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += आकार(*v) + hsize;
        sel = nsel;

        अंक_Algs++;
    }

    जर (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        परत -1;
    }

    संरचना tpm2_digest_मूल्ये *v = (शून्य*)le->hdr.digest;
    जर (bigEndian)
        v->मोजणी = cpu_to_be32(अंक_Algs);
    अन्यथा
        v->मोजणी = अंक_Algs;

    परत dest - (शून्य*)le->hdr.digest;
}

स्थिर पूर्णांक
tpm12_build_digest(संरचना tpm_log_entry *le,
                   स्थिरांक u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    परत SHA1_BUFSIZE;
}

स्थिर पूर्णांक
tpm12_build_digest_direct(संरचना tpm_log_entry *le, स्थिरांक u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    स_नकल(le->hdr.digest, sha1, SHA1_BUFSIZE);
    परत SHA1_BUFSIZE;
}

स्थिर पूर्णांक
tpm_build_digest(संरचना tpm_log_entry *le, स्थिरांक u8 *hashdata, u32 hashdata_len
                 , पूर्णांक bigEndian)
{
    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        परत tpm12_build_digest(le, hashdata, hashdata_len);
    स्थिती TPM_VERSION_2:
        परत tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    परत -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
स्थिर पूर्णांक
tpm_simple_cmd(u8 locty, u32 ordinal
               , पूर्णांक param_size, u16 param, गणना tpmDurationType to_t)
{
    संरचना {
        संरचना tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(आकार(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        थांबा;
    स्थिती TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        थांबा;
    }

    u8 obuffer[64];
    संरचना tpm_rsp_header *trsh = (शून्य*)obuffer;
    u32 obuffer_len = आकार(obuffer);
    स_रखो(obuffer, 0x0, आकार(obuffer));

    पूर्णांक ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    परत ret;
}

स्थिर पूर्णांक
tpm20_getcapability(u32 capability, u32 property, u32 मोजणी,
                    संरचना tpm_rsp_header *rsp, u32 rsize)
{
    संरचना tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(आकार(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(मोजणी),
    };

    u32 resp_size = rsize;
    पूर्णांक ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    परत ret;
}

स्थिर पूर्णांक
tpm20_get_pcrbanks(शून्य)
{
    u8 buffer[128];
    संरचना tpm2_res_getcapability *trg =
      (संरचना tpm2_res_getcapability *)&buffer;

    पूर्णांक ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  आकार(buffer));
    जर (ret)
        परत ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    जर (resplen <= दुरत्व(संरचना tpm2_res_getcapability, data))
        परत -1;

    u32 size = resplen - दुरत्व(संरचना tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    जर (size < दुरत्व(संरचना tpml_pcr_selection, selections) +
               दुरत्व(संरचना tpms_pcr_selection, pcrSelect))
        परत -1;

    tpm20_pcr_selection = malloc_वरचा(size);
    जर (tpm20_pcr_selection) {
        स_नकल(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } अन्यथा {
        warn_noalloc();
        ret = -1;
    }

    परत ret;
}

स्थिर पूर्णांक
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    जर (!tpm20_pcr_selection)
        परत -1;

    संरचना tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    शून्य *end = (शून्य*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    पर्यंत (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        शून्य *nsel = (शून्य*)sel + आकार(*sel) + sizeOfSelect;
        जर (nsel > end)
            परत 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        अचिन्हित i;
        साठी (i = 0; i < sizeOfSelect; i++) {
            जर (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                थांबा;
            }
        }

        sel = nsel;
    }
}

स्थिर पूर्णांक
tpm20_set_pcrbanks(u32 active_banks)
{
    संरचना tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(आकार(trpa.authblock)),
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
    पूर्णांक i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    पर्यंत (hashalg_flag) {
        जर ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            जर (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                जर (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.मोजणी = cpu_to_be32(i);
    स_नकल(trpa.tpms_pcr_selections, tps, i * आकार(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(दुरत्व(संरचना tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * आकार(tps[0]));

    संरचना tpm_rsp_header rsp;
    u32 resp_length = आकार(rsp);

    पूर्णांक ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    परत ret;
}

स्थिर पूर्णांक tpm20_activate_pcrbanks(u32 active_banks)
{
    पूर्णांक ret = tpm20_set_pcrbanks(active_banks);
    जर (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    जर (!ret)
        reset();
    परत ret;
}

स्थिर पूर्णांक
tpm12_get_capability(u32 cap, u32 subcap, संरचना tpm_rsp_header *rsp, u32 rsize)
{
    संरचना tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(आकार(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(आकार(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    पूर्णांक ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    परत ret;
}

स्थिर पूर्णांक
tpm12_read_permanent_flags(अक्षर *buf, पूर्णांक buf_len)
{
    स_रखो(buf, 0, buf_len);

    संरचना tpm_res_getcap_perm_flags pf;
    पूर्णांक ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, आकार(pf));
    जर (ret)
        परत -1;

    स_नकल(buf, &pf.perm_flags, buf_len);

    परत 0;
}

स्थिर पूर्णांक
tpm12_determine_timeouts(शून्य)
{
    संरचना tpm_res_getcap_timeouts timeouts;
    पूर्णांक ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, आकार(timeouts));
    जर (ret)
        परत ret;

    संरचना tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, आकार(durations));
    जर (ret)
        परत ret;

    पूर्णांक i;
    साठी (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    साठी (i = 0; i < 4; i++)
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

    परत 0;
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

स्थिर पूर्णांक
tpm12_extend(संरचना tpm_log_entry *le, पूर्णांक digest_len)
{
    संरचना tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(आकार(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    स_नकल(tre.digest, le->hdr.digest, आकार(tre.digest));

    संरचना tpm_rsp_extend rsp;
    u32 resp_length = आकार(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    जर (ret || resp_length != आकार(rsp) || rsp.hdr.errcode)
        परत -1;

    परत 0;
}

स्थिर पूर्णांक tpm20_extend(संरचना tpm_log_entry *le, पूर्णांक digest_len)
{
    संरचना tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(आकार(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[आकार(tmp_tre) + आकार(le->pad)];
    संरचना tpm2_req_extend *tre = (संरचना tpm2_req_extend *)buffer;

    स_नकल(tre, &tmp_tre, आकार(tmp_tre));
    स_नकल(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(आकार(tmp_tre) + digest_len);

    संरचना tpm_rsp_header rsp;
    u32 resp_length = आकार(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    जर (ret || resp_length != आकार(rsp) || rsp.errcode)
        परत -1;

    परत 0;
}

स्थिर पूर्णांक
tpm_extend(संरचना tpm_log_entry *le, पूर्णांक digest_len)
{
    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        परत tpm12_extend(le, digest_len);
    स्थिती TPM_VERSION_2:
        परत tpm20_extend(le, digest_len);
    }
    परत -1;
}

स्थिर पूर्णांक
tpm20_stirrandom(शून्य)
{
    संरचना tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(आकार(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(आकार(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    संरचना tpm_rsp_header rsp;
    u32 resp_length = आकार(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    जर (ret || resp_length != आकार(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    परत ret;
}

स्थिर पूर्णांक
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    संरचना tpm2_res_getrandom rsp;

    जर (buf_len > आकार(rsp.rnd.buffer))
        परत -1;

    संरचना tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(आकार(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = आकार(rsp);

    पूर्णांक ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    जर (ret || resp_length != आकार(rsp) || rsp.hdr.errcode)
        ret = -1;
    अन्यथा
        स_नकल(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    परत ret;
}

स्थिर पूर्णांक
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    संरचना tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(आकार(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(आकार(trh.authblock)),
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
    u32 resp_length = आकार(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    जर (ret || resp_length != आकार(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    परत ret;
}

स्थिर पूर्णांक
tpm20_hierarchychangeauth(u8 auth[20])
{
    संरचना tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(आकार(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(आकार(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(आकार(trhca.newAuth.buffer)),
        },
    };
    स_नकल(trhca.newAuth.buffer, auth, आकार(trhca.newAuth.buffer));

    संरचना tpm_rsp_header rsp;
    u32 resp_length = आकार(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    जर (ret || resp_length != आकार(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    परत ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

स्थिर पूर्णांक TPM_has_physical_presence;
u8 TPM_working VARLOW;

स्थिर पूर्णांक
tpm_is_working(शून्य)
{
    परत CONFIG_TCGBIOS && TPM_working;
}

स्थिर शून्य
tpm_set_failure(शून्य)
{
    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        थांबा;
    स्थिती TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        थांबा;
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
                           स्थिरांक अक्षर *event, u32 event_length,
                           स्थिरांक u8 *hashdata, u32 hashdata_length)
{
    जर (!tpm_is_working())
        परत;

    संरचना tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    पूर्णांक digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    जर (digest_len < 0)
        परत;
    पूर्णांक ret = tpm_extend(&le, digest_len);
    जर (ret) {
        tpm_set_failure();
        परत;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
स्थिर शून्य
tpm_add_action(u32 pcrIndex, स्थिरांक अक्षर *string)
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
    स्थिर स्थिरांक u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    साठी (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (स्थिरांक अक्षर *)evt_separator,
                                   आकार(evt_separator),
                                   evt_separator,
                                   आकार(evt_separator));
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

    जर (!smbios_tables)
        परत;

    sha1((स्थिरांक u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (स्थिरांक अक्षर *)&pcctes, आकार(pcctes),
                               (u8 *)&pcctes, आकार(pcctes));
}

स्थिर पूर्णांक
tpm12_assert_physical_presence(शून्य)
{
    पूर्णांक ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    जर (!ret)
        परत 0;

    संरचना tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((अक्षर *)&pf, आकार(pf));
    जर (ret)
        परत -1;

    /* check if hardware physical presence is supported */
    जर (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        परत 0;
    }

    जर (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        परत tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    परत -1;
}

स्थिर पूर्णांक
tpm12_startup(शून्य)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    पूर्णांक ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    जर (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    जर (ret)
        जा err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    जर (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    जर (ret)
        जा err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    जर (ret)
        जा err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    जर (ret && ret != TPM_BAD_LOCALITY)
        जा err_exit;

    परत 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    परत -1;
}

स्थिर पूर्णांक
tpm20_startup(शून्य)
{
    tpm20_set_timeouts();

    पूर्णांक ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    जर (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    जर (ret)
        जा err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    जर (ret)
        जा err_exit;

    ret = tpm20_get_pcrbanks();
    जर (ret)
        जा err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    जर (ret)
        जा err_exit;

    परत 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    परत -1;
}

स्थिर पूर्णांक
tpm_startup(शून्य)
{
    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        परत tpm12_startup();
    स्थिती TPM_VERSION_2:
        परत tpm20_startup();
    }
    परत -1;
}

शून्य
tpm_setup(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत;

    पूर्णांक ret = tpm_tpm2_probe();
    जर (ret) {
        ret = tpm_tcpa_probe();
        जर (ret)
            परत;
    }

    TPM_version = tpmhw_probe();
    जर (TPM_version == TPM_VERSION_NONE)
        परत;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    जर (runningOnXen())
        परत;

    ret = tpm_startup();
    जर (ret)
        परत;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

स्थिर शून्य
tpm20_prepboot(शून्य)
{
    पूर्णांक ret = tpm20_stirrandom();
    जर (ret)
         जा err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], आकार(auth));
    जर (ret)
        जा err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    जर (ret)
        जा err_exit;

    परत;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

शून्य
tpm_prepboot(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत;

    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        जर (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        थांबा;
    स्थिती TPM_VERSION_2:
        tpm20_prepboot();
        थांबा;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
शून्य
tpm_option_rom(स्थिरांक शून्य *addr, u32 len)
{
    जर (!tpm_is_working())
        परत;

    संरचना pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = आकार(u16) + आकार(u16) + SHA1_BUFSIZE,
    };
    sha1((स्थिरांक u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (स्थिरांक अक्षर *)&pcctes, आकार(pcctes),
                               (u8 *)&pcctes, आकार(pcctes));
}

शून्य
tpm_add_bcv(u32 bootdrv, स्थिरांक u8 *addr, u32 length)
{
    जर (!tpm_is_working())
        परत;

    जर (length < 0x200)
        परत;

    स्थिरांक अक्षर *string = "Booting BCV device 00h (Floppy)";
    जर (bootdrv == 0x80)
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
tpm_add_cdrom(u32 bootdrv, स्थिरांक u8 *addr, u32 length)
{
    जर (!tpm_is_working())
        परत;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    स्थिरांक अक्षर *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, म_माप(string),
                               addr, length);
}

शून्य
tpm_add_cdrom_catalog(स्थिरांक u8 *addr, u32 length)
{
    जर (!tpm_is_working())
        परत;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    स्थिरांक अक्षर *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, म_माप(string),
                               addr, length);
}

शून्य
tpm_s3_resume(शून्य)
{
    जर (!tpm_is_working())
        परत;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    पूर्णांक ret = -1;

    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        थांबा;
    स्थिती TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        जर (ret)
            जा err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        थांबा;
    }

    जर (ret)
        जा err_exit;

    परत;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

स्थिर ओळीत शून्य *आदान_buf32(संरचना bregs *regs)
{
    परत MAKE_FLATPTR(regs->es, regs->di);
}

स्थिर ओळीत शून्य *output_buf32(संरचना bregs *regs)
{
    परत MAKE_FLATPTR(regs->ds, regs->si);
}

स्थिर u32
hash_log_extend(संरचना pcpes *pcpes, स्थिरांक शून्य *hashdata, u32 hashdata_length
                , शून्य *event, पूर्णांक extend)
{
    जर (pcpes->pcrindex >= 24)
        परत TCG_INVALID_आदान_PARA;
    जर (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    संरचना tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    पूर्णांक digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    जर (digest_len < 0)
        परत TCG_GENERAL_ERROR;
    जर (extend) {
        पूर्णांक ret = tpm_extend(&le, digest_len);
        जर (ret)
            परत TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    पूर्णांक ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    जर (ret)
        परत TCG_PC_LOGOVERFLOW;
    परत 0;
}

स्थिर u32
hash_log_extend_event_int(स्थिरांक संरचना hleei_short *hleei_s,
                          संरचना hleeo *hleeo)
{
    u32 rc = 0;
    संरचना hleei_long *hleei_l = (संरचना hleei_long *)hleei_s;
    स्थिरांक शून्य *logdataptr;
    u32 logdatalen;
    संरचना pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    निवड (hleei_s->ipblength) {
    स्थिती आकार(संरचना hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    थांबा;

    स्थिती आकार(संरचना hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    थांबा;

    पूर्वनिर्धारित:
        /* bad input block */
        rc = TCG_INVALID_आदान_PARA;
        जा err_exit;
    }

    pcpes = (संरचना pcpes *)logdataptr;

    जर (pcpes->pcrindex != pcrindex
        || logdatalen != आकार(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_आदान_PARA;
        जा err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    जर (rc)
        जा err_exit;

    hleeo->opblength = आकार(संरचना hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_मोजणी;
    स_नकल(hleeo->digest, pcpes->digest, आकार(hleeo->digest));

err_exit:
    जर (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    परत rc;
}

स्थिर u32
pass_through_to_tpm_int(संरचना pttti *pttti, संरचना pttto *pttto)
{
    u32 rc = 0;
    संरचना tpm_req_header *trh = (शून्य*)pttti->tpmopin;

    जर (pttti->ipblength < आकार(संरचना pttti) + आकार(*trh)
        || pttti->ipblength != आकार(संरचना pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < आकार(संरचना pttto)) {
        rc = TCG_INVALID_आदान_PARA;
        जा err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        जर (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_आदान_PARA;
            जा err_exit;
        }
        थांबा;
    स्थिती TPM_VERSION_2:
        जर (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_आदान_PARA;
            जा err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - दुरत्व(संरचना pttto, tpmopout);
    पूर्णांक ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    जर (ret) {
        rc = TCG_FATAL_COM_ERROR;
        जा err_exit;
    }

    pttto->opblength = दुरत्व(संरचना pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    जर (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    परत rc;
}

स्थिर u32
shutdown_preboot_interface(शून्य)
{
    TPM_interface_shutdown = 1;
    परत 0;
}

स्थिर u32
hash_log_event_int(स्थिरांक संरचना hlei *hlei, संरचना hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    संरचना pcpes *pcpes;

    size = hlei->ipblength;
    जर (size != आकार(*hlei)) {
        rc = TCG_INVALID_आदान_PARA;
        जा err_exit;
    }

    pcpes = (संरचना pcpes *)hlei->logdataptr;

    जर (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != आकार(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_आदान_PARA;
        जा err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    जर (rc)
        जा err_exit;

    /* updating the log was fine */
    hleo->opblength = आकार(संरचना hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_मोजणी;

err_exit:
    जर (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    परत rc;
}

स्थिर u32
hash_all_int(स्थिरांक संरचना hai *hai, u8 *hash)
{
    जर (hai->ipblength != आकार(संरचना hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        परत TCG_INVALID_आदान_PARA;

    sha1((स्थिरांक u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    परत 0;
}

स्थिर u32
tss_int(संरचना ti *ti, संरचना to *to)
{
    to->opblength = आकार(संरचना to);
    to->reserved  = 0;

    परत TCG_PC_UNSUPPORTED;
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
        .eventdatasize = आकार(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    जर (rc)
        परत rc;

    *edx_ptr = tpm_state.entry_मोजणी;
    परत 0;
}

शून्य VISIBLE32FLAT
tpm_interrupt_handler32(संरचना bregs *regs)
{
    जर (!CONFIG_TCGBIOS)
        परत;

    set_cf(regs, 0);

    जर (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        परत;
    }

    निवड ((गणना irq_ids)regs->al) {
    स्थिती TCG_StatusCheck:
        जर (!tpmhw_is_present()) {
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
        थांबा;

    स्थिती TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (संरचना hleei_short *)आदान_buf32(regs),
                  (संरचना hleeo *)output_buf32(regs));
        थांबा;

    स्थिती TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((संरचना pttti *)आदान_buf32(regs),
                                    (संरचना pttto *)output_buf32(regs));
        थांबा;

    स्थिती TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        थांबा;

    स्थिती TCG_HashLogEvent:
        regs->eax = hash_log_event_int((संरचना hlei*)आदान_buf32(regs),
                                       (संरचना hleo*)output_buf32(regs));
        थांबा;

    स्थिती TCG_HashAll:
        regs->eax =
            hash_all_int((संरचना hai*)आदान_buf32(regs),
                          (u8 *)output_buf32(regs));
        थांबा;

    स्थिती TCG_TSS:
        regs->eax = tss_int((संरचना ti*)आदान_buf32(regs),
                            (संरचना to*)output_buf32(regs));
        थांबा;

    स्थिती TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)आदान_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        थांबा;

    पूर्वनिर्धारित:
        set_cf(regs, 1);
    }

    परत;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

प्रकारव्याख्या u8 tpm_ppi_code;

स्थिर पूर्णांक
tpm12_read_has_owner(पूर्णांक *has_owner)
{
    संरचना tpm_res_getcap_ownerauth oauth;
    पूर्णांक ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, आकार(oauth));
    जर (ret)
        परत -1;

    *has_owner = oauth.flag;

    परत 0;
}

स्थिर पूर्णांक
tpm12_enable_tpm(पूर्णांक enable, पूर्णांक verbose)
{
    संरचना tpm_permanent_flags pf;
    पूर्णांक ret = tpm12_read_permanent_flags((अक्षर *)&pf, आकार(pf));
    जर (ret)
        परत -1;

    जर (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        परत 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    जर (ret) {
        जर (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        अन्यथा
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    परत ret;
}

स्थिर पूर्णांक
tpm12_activate_tpm(पूर्णांक activate, पूर्णांक allow_reset, पूर्णांक verbose)
{
    संरचना tpm_permanent_flags pf;
    पूर्णांक ret = tpm12_read_permanent_flags((अक्षर *)&pf, आकार(pf));
    जर (ret)
        परत -1;

    जर (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        परत 0;

    जर (pf.flags[PERM_FLAG_IDX_DISABLE])
        परत 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    जर (ret)
        परत ret;

    जर (activate && allow_reset) {
        जर (verbose) {
            म_लिखो("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    परत 0;
}

स्थिर पूर्णांक
tpm12_enable_activate(पूर्णांक allow_reset, पूर्णांक verbose)
{
    पूर्णांक ret = tpm12_enable_tpm(1, verbose);
    जर (ret)
        परत ret;

    परत tpm12_activate_tpm(1, allow_reset, verbose);
}

स्थिर पूर्णांक
tpm12_force_clear(पूर्णांक enable_activate_before, पूर्णांक enable_activate_after,
                  पूर्णांक verbose)
{
    पूर्णांक has_owner;
    पूर्णांक ret = tpm12_read_has_owner(&has_owner);
    जर (ret)
        परत -1;
    जर (!has_owner) {
        जर (verbose)
            म_लिखो("TPM does not have an owner.\n");
        परत 0;
    }

    जर (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        जर (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            परत ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    जर (ret)
        परत ret;

    जर (!enable_activate_after) {
        जर (verbose)
            म_लिखो("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        परत 0;
    }

    परत tpm12_enable_activate(1, verbose);
}

स्थिर पूर्णांक
tpm12_set_owner_install(पूर्णांक allow, पूर्णांक verbose)
{
    पूर्णांक has_owner;
    पूर्णांक ret = tpm12_read_has_owner(&has_owner);
    जर (ret)
        परत -1;
    जर (has_owner) {
        जर (verbose)
            म_लिखो("Must first remove owner.\n");
        परत 0;
    }

    संरचना tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((अक्षर *)&pf, आकार(pf));
    जर (ret)
        परत -1;

    जर (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        जर (verbose)
            म_लिखो("TPM must first be enable.\n");
        परत 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    जर (ret)
        परत ret;

    जर (verbose)
        म_लिखो("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    परत 0;
}

स्थिर पूर्णांक
tpm12_process_cfg(tpm_ppi_code संदेश_Code, पूर्णांक verbose)
{
    पूर्णांक ret = 0;

    निवड (संदेश_Code) {
        स्थिती TPM_PPI_OP_NOOP: /* no-op */
            थांबा;

        स्थिती TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            थांबा;

        स्थिती TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            थांबा;

        स्थिती TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            थांबा;

        स्थिती TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            थांबा;

        स्थिती TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            थांबा;

        स्थिती TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            थांबा;

        स्थिती TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            थांबा;

        पूर्वनिर्धारित:
            थांबा;
    }

    जर (ret)
        म_लिखो("Op %d: An error occurred: 0x%x\n", संदेश_Code, ret);

    परत ret;
}

स्थिर पूर्णांक
tpm20_clearcontrol(u8 disable, पूर्णांक verbose)
{
    संरचना tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(आकार(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(आकार(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    संरचना tpm_rsp_header rsp;
    u32 resp_length = आकार(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    जर (ret || resp_length != आकार(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    परत ret;
}

स्थिर पूर्णांक
tpm20_clear(शून्य)
{
    संरचना tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(आकार(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(आकार(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    संरचना tpm_rsp_header rsp;
    u32 resp_length = आकार(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    जर (ret || resp_length != आकार(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    परत ret;
}

स्थिर पूर्णांक
tpm20_process_cfg(tpm_ppi_code संदेश_Code, पूर्णांक verbose)
{
    पूर्णांक ret = 0;

    निवड (संदेश_Code) {
        स्थिती TPM_PPI_OP_NOOP: /* no-op */
            थांबा;

        स्थिती TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            जर (!ret)
                 ret = tpm20_clear();
            थांबा;
    }

    जर (ret)
        म_लिखो("Op %d: An error occurred: 0x%x\n", संदेश_Code, ret);

    परत ret;
}

स्थिर पूर्णांक
tpm12_get_tpm_state(शून्य)
{
    पूर्णांक state = 0;
    संरचना tpm_permanent_flags pf;
    पूर्णांक has_owner;

    जर (tpm12_read_permanent_flags((अक्षर *)&pf, आकार(pf)) ||
        tpm12_read_has_owner(&has_owner))
        परत ~0;

    जर (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    जर (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    जर (has_owner) {
        state |= TPM_STATE_OWNED;
    } अन्यथा {
        जर (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    परत state;
}

स्थिर शून्य
tpm12_show_tpm_menu(पूर्णांक state, पूर्णांक next_scancodes[7])
{
    पूर्णांक i = 0;

    म_लिखो("\nThe current state of the TPM is:\n");

    जर (state & TPM_STATE_ENABLED)
        म_लिखो("  Enabled");
    अन्यथा
        म_लिखो("  Disabled");

    जर (state & TPM_STATE_ACTIVE)
        म_लिखो(" and active\n");
    अन्यथा
        म_लिखो(" and deactivated\n");

    जर (state & TPM_STATE_OWNED)
        म_लिखो("  Ownership has been taken\n");
    अन्यथा {
        म_लिखो("  Ownership has not been taken\n");
        जर (state & TPM_STATE_OWNERINSTALL)
            म_लिखो("  A user can take ownership of the TPM\n");
        अन्यथा
            म_लिखो("  Taking ownership of the TPM has been disabled\n");
    }

    जर ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        म_लिखो("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    म_लिखो("\nAvailable options are:\n");
    जर (state & TPM_STATE_ENABLED) {
        म_लिखो(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        जर (state & TPM_STATE_ACTIVE) {
            म_लिखो(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            जर (state & TPM_STATE_OWNERINSTALL) {
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

    जर (state & TPM_STATE_OWNED) {
        म_लिखो(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

स्थिर शून्य
tpm12_menu(शून्य)
{
    पूर्णांक scancode, next_scancodes[7];
    tpm_ppi_code संदेश_Code;
    पूर्णांक state = 0, i;
    पूर्णांक waitkey;

    म_लिखो("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    साठी (;;) {
        जर ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } अन्यथा {
            म_लिखो("TPM is not working correctly.\n");
            परत;
        }

        म_लिखो("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        संदेश_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        पर्यंत (waitkey) {
            पर्यंत ((scancode = get_keystroke(1000)) == ~0)
                ;

            निवड (scancode) {
            स्थिती 1:
                // ESC
                reset();
                थांबा;
            स्थिती 18: /* e. enable */
                संदेश_Code = TPM_PPI_OP_ENABLE;
                थांबा;
            स्थिती 32: /* d. disable */
                संदेश_Code = TPM_PPI_OP_DISABLE;
                थांबा;
            स्थिती 30: /* a. activate */
                संदेश_Code = TPM_PPI_OP_ACTIVATE;
                थांबा;
            स्थिती 47: /* v. deactivate */
                संदेश_Code = TPM_PPI_OP_DEACTIVATE;
                थांबा;
            स्थिती 46: /* c. clear owner */
                संदेश_Code = TPM_PPI_OP_CLEAR;
                थांबा;
            स्थिती 25: /* p. prevent ownerinstall */
                संदेश_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                थांबा;
            स्थिती 31: /* s. allow ownerinstall */
                संदेश_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                थांबा;
            पूर्वनिर्धारित:
                सुरूठेवा;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            साठी (i = 0; i < आकार(next_scancodes); i++) {
                जर (next_scancodes[i] == 0)
                    थांबा;

                जर (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(संदेश_Code, 1);
                    waitkey = 0;
                    थांबा;
                }
            }
        }
    }
}

स्थिर पूर्णांक
tpm20_menu_change_active_pcrbanks(शून्य)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    पर्यंत (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        म_लिखो("\nToggle active PCR banks by pressing number key\n\n");

        पर्यंत (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            स्थिरांक अक्षर *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            जर (hashname) {
                म_लिखो("  %d: %s", i, hashname);
                जर (activate_banks & hashalg_flag)
                    म_लिखो(" (enabled)");
                म_लिखो("\n");
            }

            hashalg_flag <<= 1;
        }
        म_लिखो("\n"
               "ESC: return to previous menu without changes\n");
        जर (activate_banks)
            म_लिखो("A  : activate selection\n");

        u8 flagnum;
        पूर्णांक show = 0;
        पर्यंत (!show) {
            पूर्णांक scancode = get_keystroke(1000);

            निवड (scancode) {
            स्थिती ~0:
                सुरूठेवा;
            स्थिती 1: /* ESC */
                म_लिखो("\n");
                परत -1;
            स्थिती 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                जर (flagnum > i)
                    सुरूठेवा;
                जर (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                थांबा;
            स्थिती 30: /* a */
                जर (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

स्थिर शून्य
tpm20_menu(शून्य)
{
    पूर्णांक scan_code;
    tpm_ppi_code संदेश_Code;

    साठी (;;) {
        म_लिखो("1. Clear TPM\n");
        म_लिखो("2. Change active PCR banks\n");

        म_लिखो("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        संदेश_Code = TPM_PPI_OP_NOOP;

        पर्यंत ((scan_code = get_keystroke(1000)) == ~0)
            ;

        निवड (scan_code) {
        स्थिती 1:
            // ESC
            reset();
            थांबा;
        स्थिती 2:
            संदेश_Code = TPM_PPI_OP_CLEAR;
            थांबा;
        स्थिती 3:
            tpm20_menu_change_active_pcrbanks();
            सुरूठेवा;
        पूर्वनिर्धारित:
            सुरूठेवा;
        }

        tpm20_process_cfg(संदेश_Code, 0);
    }
}

शून्य
tpm_menu(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत;

    पर्यंत (get_keystroke(0) >= 0)
        ;
    wait_threads();

    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        tpm12_menu();
        थांबा;
    स्थिती TPM_VERSION_2:
        tpm20_menu();
        थांबा;
    }
}

पूर्णांक
tpm_can_show_menu(शून्य)
{
    निवड (TPM_version) {
    स्थिती TPM_VERSION_1_2:
        परत tpm_is_working() && TPM_has_physical_presence;
    स्थिती TPM_VERSION_2:
        परत tpm_is_working();
    }
    परत 0;
}
