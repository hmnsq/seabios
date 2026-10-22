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

#समावेश "bregs.h" // struct bregs
#समावेश "byteorder.h" // cpu_to_*
#समावेश "config.h" // CONFIG_TCGBIOS
#समावेश "farptr.h" // MAKE_FLATPTR
#समावेश "fw/paravirt.h" // runningOnXen
#समावेश "hw/tpm_drivers.h" // tpm_drivers[]
#समावेश "output.h" // dprintf
#समावेश "sha.h" // sha1, sha256, ...
#समावेश "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#समावेश "std/smbios.h" // struct smbios_21_entry_point
#समावेश "std/tcg.h" // TCG_PC_LOGOVERFLOW
#समावेश "string.h" // checksum
#समावेश "tcgbios.h"// tpm_*, prototypes
#समावेश "util.h" // printf, get_keystroke
#समावेश "stacks.h" // wait_threads, reset
#समावेश "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

काष्ठा {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_लंबाई;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_गिनती;

    /* address to write next log entry to */
    u8 *          log_area_अग्र_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

अटल पूर्णांक tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_लंबाई)
{
    अगर (!log_area_start_address || !log_area_minimum_लंबाई)
        वापस -1;

    स_रखो(log_area_start_address, 0, log_area_minimum_लंबाई);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_लंबाई = log_area_minimum_लंबाई;
    tpm_state.log_area_अग्र_entry = log_area_start_address;
    tpm_state.log_area_last_entry = शून्य;
    tpm_state.entry_गिनती = 0;
    वापस 0;
}

अटल पूर्णांक
tpm_tcpa_probe(व्योम)
{
    काष्ठा tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    अगर (!tcpa)
        वापस -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(दीर्घ)tcpa->log_area_start_address,
            tcpa->log_area_minimum_लंबाई);

    वापस tpm_set_log_area((u8*)(दीर्घ)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_लंबाई);
}

अटल पूर्णांक
tpm_tpm2_probe(व्योम)
{
    काष्ठा tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    अगर (!tpm2)
        वापस -1;

    अगर (tpm2->लंबाई < 76)
        वापस -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(दीर्घ)tpm2->log_area_start_address,
            tpm2->log_area_minimum_लंबाई);

    वापस tpm_set_log_area((u8*)(दीर्घ)tpm2->log_area_start_address,
                            tpm2->log_area_minimum_लंबाई);
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
अटल पूर्णांक
tpm_log_event(काष्ठा tpm_log_header *entry, पूर्णांक digest_len
              , स्थिर व्योम *event, पूर्णांक event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_अग्र_entry);

    अगर (tpm_state.log_area_अग्र_entry == शून्य)
        वापस -1;

    u32 आकार = (माप(*entry) + digest_len
                + माप(काष्ठा tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_अग्र_entry + आकार
                   - tpm_state.log_area_start_address);
    अगर (logsize > tpm_state.log_area_minimum_लंबाई) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", आकार);
        वापस -1;
    }

    व्योम *dest = tpm_state.log_area_अग्र_entry;
    स_नकल(dest, entry, माप(*entry) + digest_len);
    काष्ठा tpm_log_trailer *t = dest + माप(*entry) + digest_len;
    t->eventdatasize = event_len;
    स_नकल(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_अग्र_entry;
    tpm_state.log_area_अग्र_entry += आकार;
    tpm_state.entry_गिनती++;

    वापस 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

अटल TPMVersion TPM_version;
अटल u32 tpm20_pcr_selection_आकार;
अटल काष्ठा tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
काष्ठा tpm_log_entry {
    काष्ठा tpm_log_header hdr;
    u8 pad[माप(काष्ठा tpm2_digest_मूल्यावली)
           + 8 * माप(काष्ठा tpm2_digest_मूल्य)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

अटल स्थिर काष्ठा hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    स्थिर अक्षर *name;
    व्योम (*hashfunc)(स्थिर u8 *तथ्य, u32 लंबाई, u8 *hash);
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

अटल पूर्णांक
tpm20_get_hash_buffersize(u16 hashAlg)
{
    अचिन्हित i;

    क्रम (i = 0; i < ARRAY_आकार(hash_parameters); i++) {
        अगर (hash_parameters[i].hashalg == hashAlg)
            वापस hash_parameters[i].hash_buffersize;
    }
    वापस -1;
}

अटल u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    अचिन्हित i;

    क्रम (i = 0; i < ARRAY_आकार(hash_parameters); i++) {
        अगर (hash_parameters[i].hashalg == hashAlg)
            वापस hash_parameters[i].hashalg_flag;
    }
    वापस 0;
}

अटल u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    अचिन्हित i;

    क्रम (i = 0; i < ARRAY_आकार(hash_parameters); i++) {
        अगर (hash_parameters[i].hashalg_flag == hashalg_flag)
            वापस hash_parameters[i].hashalg;
    }
    वापस 0;
}

अटल स्थिर अक्षर *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    अचिन्हित i;

    क्रम (i = 0; i < ARRAY_आकार(hash_parameters); i++) {
        अगर (hash_parameters[i].hashalg_flag == hashalg_flag)
            वापस hash_parameters[i].name;
    }
    वापस शून्य;
}

अटल व्योम tpm2_hash_तथ्य(u16 hashAlg, स्थिर u8 *तथ्य, u32 तथ्य_len, u8 *hash)
{
    अचिन्हित i;

    क्रम (i = 0; i < ARRAY_आकार(hash_parameters); i++) {
        अगर (hash_parameters[i].hashalg == hashAlg) {
            अगर (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(तथ्य, तथ्य_len, hash);
            } अन्यथा {
                स_रखो(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
अटल पूर्णांक
tpm20_write_EfiSpecIdEventStruct(व्योम)
{
    अगर (!tpm20_pcr_selection)
        वापस -1;

    काष्ठा {
        काष्ठा TCG_EfiSpecIdEventStruct hdr;
        u8 pad[माप(काष्ठा tpm_log_entry) + माप(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintn_आकार = 2,
    };

    काष्ठा tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    व्योम *nsel, *end = (व्योम*)tpm20_pcr_selection + tpm20_pcr_selection_आकार;

    u32 गिनती, अंक_Algs = 0;
    क्रम (गिनती = 0; गिनती < be32_to_cpu(tpm20_pcr_selection->गिनती); गिनती++) {
        u8 आकार_Of_Select = sel->आकार_Of_Select;

        nsel = (व्योम*)sel + माप(*sel) + आकार_Of_Select;
        अगर (nsel > end)
            अवरोध;

        अगर (!आकार_Of_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            जारी;
        }

        पूर्णांक hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        अगर (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            वापस -1;
        }

        पूर्णांक event_आकार = दुरत्व(काष्ठा TCG_EfiSpecIdEventStruct
                                  , digestSizes[गिनती+1]);
        अगर (event_आकार > माप(event) - माप(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            वापस -1;
        }

        event.hdr.digestSizes[अंक_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[अंक_Algs].digest_आकार = hsize;
        अंक_Algs++;

        sel = nsel;
    }

    अगर (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        वापस -1;
    }

    event.hdr.संख्या_Of_Algorithms = अंक_Algs;
    पूर्णांक event_आकार = दुरत्व(काष्ठा TCG_EfiSpecIdEventStruct
                              , digestSizes[अंक_Algs]);
    u8 *vendor_Info_आकार = (व्योम*)&event + event_आकार;
    *vendor_Info_आकार = 0;
    event_आकार += माप(*vendor_Info_आकार);

    काष्ठा tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    वापस tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_आकार);
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
अटल पूर्णांक
tpm20_build_digest(काष्ठा tpm_log_entry *le,
                   स्थिर u8 *hashdata, u32 hashdata_len, पूर्णांक bigEndian)
{
    अगर (!tpm20_pcr_selection)
        वापस -1;

    काष्ठा tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    व्योम *nsel, *end = (व्योम*)tpm20_pcr_selection + tpm20_pcr_selection_आकार;
    व्योम *dest = le->hdr.digest + माप(काष्ठा tpm2_digest_मूल्यावली);

    u32 गिनती, अंक_Algs = 0;
    क्रम (गिनती = 0; गिनती < be32_to_cpu(tpm20_pcr_selection->गिनती); गिनती++) {
        u8 आकार_Of_Select = sel->आकार_Of_Select;

        nsel = (व्योम*)sel + माप(*sel) + आकार_Of_Select;
        अगर (nsel > end)
            अवरोध;

        /* PCR 0-7 unused? -- skip */
        अगर (!आकार_Of_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            जारी;
        }

        पूर्णांक hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        अगर (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            वापस -1;
        }

        /* buffer size sanity check before writing */
        काष्ठा tpm2_digest_मूल्य *v = dest;
        अगर (dest + माप(*v) + hsize > (व्योम*)le + माप(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            वापस -1;
        }

        अगर (bigEndian)
            v->hashAlg = sel->hashAlg;
        अन्यथा
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_तथ्य(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += माप(*v) + hsize;
        sel = nsel;

        अंक_Algs++;
    }

    अगर (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        वापस -1;
    }

    काष्ठा tpm2_digest_मूल्यावली *v = (व्योम*)le->hdr.digest;
    अगर (bigEndian)
        v->गिनती = cpu_to_be32(अंक_Algs);
    अन्यथा
        v->गिनती = अंक_Algs;

    वापस dest - (व्योम*)le->hdr.digest;
}

अटल पूर्णांक
tpm12_build_digest(काष्ठा tpm_log_entry *le,
                   स्थिर u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    वापस SHA1_BUFSIZE;
}

अटल पूर्णांक
tpm12_build_digest_direct(काष्ठा tpm_log_entry *le, स्थिर u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    स_नकल(le->hdr.digest, sha1, SHA1_BUFSIZE);
    वापस SHA1_BUFSIZE;
}

अटल पूर्णांक
tpm_build_digest(काष्ठा tpm_log_entry *le, स्थिर u8 *hashdata, u32 hashdata_len
                 , पूर्णांक bigEndian)
{
    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        वापस tpm12_build_digest(le, hashdata, hashdata_len);
    हाल TPM_VERSION_2:
        वापस tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    वापस -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
अटल पूर्णांक
tpm_simple_cmd(u8 locty, u32 ordinal
               , पूर्णांक param_आकार, u16 param, क्रमागत tpmDurationType to_t)
{
    काष्ठा {
        काष्ठा tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(माप(req.trqh) + param_आकार),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_आकार == 2 ? cpu_to_be16(param) : param,
    };
    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        अवरोध;
    हाल TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        अवरोध;
    }

    u8 obuffer[64];
    काष्ठा tpm_rsp_header *trsh = (व्योम*)obuffer;
    u32 obuffer_len = माप(obuffer);
    स_रखो(obuffer, 0x0, माप(obuffer));

    पूर्णांक ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    वापस ret;
}

अटल पूर्णांक
tpm20_getcapability(u32 capability, u32 property, u32 गिनती,
                    काष्ठा tpm_rsp_header *rsp, u32 rsize)
{
    काष्ठा tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(माप(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(गिनती),
    };

    u32 resp_आकार = rsize;
    पूर्णांक ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_आकार,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    वापस ret;
}

अटल पूर्णांक
tpm20_get_pcrbanks(व्योम)
{
    u8 बफर[128];
    काष्ठा tpm2_res_getcapability *trg =
      (काष्ठा tpm2_res_getcapability *)&बफर;

    पूर्णांक ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  माप(बफर));
    अगर (ret)
        वापस ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    अगर (resplen <= दुरत्व(काष्ठा tpm2_res_getcapability, तथ्य))
        वापस -1;

    u32 आकार = resplen - दुरत्व(काष्ठा tpm2_res_getcapability, तथ्य);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    अगर (आकार < दुरत्व(काष्ठा tpml_pcr_selection, selections) +
               दुरत्व(काष्ठा tpms_pcr_selection, pcrSelect))
        वापस -1;

    tpm20_pcr_selection = malloc_ऊपरी(आकार);
    अगर (tpm20_pcr_selection) {
        स_नकल(tpm20_pcr_selection, &trg->तथ्य, आकार);
        tpm20_pcr_selection_आकार = आकार;
    } अन्यथा {
        warn_noalloc();
        ret = -1;
    }

    वापस ret;
}

अटल पूर्णांक
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    अगर (!tpm20_pcr_selection)
        वापस -1;

    काष्ठा tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    व्योम *end = (व्योम*)tpm20_pcr_selection + tpm20_pcr_selection_आकार;

    जबतक (1) {
        u8 आकार_Of_Select = sel->आकार_Of_Select;
        व्योम *nsel = (व्योम*)sel + माप(*sel) + आकार_Of_Select;
        अगर (nsel > end)
            वापस 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        अचिन्हित i;
        क्रम (i = 0; i < आकार_Of_Select; i++) {
            अगर (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                अवरोध;
            }
        }

        sel = nsel;
    }
}

अटल पूर्णांक
tpm20_set_pcrbanks(u32 active_banks)
{
    काष्ठा tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(माप(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    काष्ठा tpms_pcr_selection3 {
        u16 hashAlg;
        u8 आकार_Of_Select;
        u8 pcrSelect[3];
    } tps[ARRAY_आकार(trpa.tpms_pcr_selections)];
    पूर्णांक i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    जबतक (hashalg_flag) {
        अगर ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            अगर (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].आकार_Of_Select = 3;

                अगर (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.गिनती = cpu_to_be32(i);
    स_नकल(trpa.tpms_pcr_selections, tps, i * माप(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(दुरत्व(काष्ठा tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * माप(tps[0]));

    काष्ठा tpm_rsp_header rsp;
    u32 resp_लंबाई = माप(rsp);

    पूर्णांक ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    वापस ret;
}

अटल पूर्णांक tpm20_activate_pcrbanks(u32 active_banks)
{
    पूर्णांक ret = tpm20_set_pcrbanks(active_banks);
    अगर (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    अगर (!ret)
        reset();
    वापस ret;
}

अटल पूर्णांक
tpm12_get_capability(u32 cap, u32 subcap, काष्ठा tpm_rsp_header *rsp, u32 rsize)
{
    काष्ठा tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(माप(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .sub_Cap_आकार = cpu_to_be32(माप(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_आकार = rsize;
    पूर्णांक ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_आकार,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_आकार != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    वापस ret;
}

अटल पूर्णांक
tpm12_read_permanent_flags(अक्षर *buf, पूर्णांक buf_len)
{
    स_रखो(buf, 0, buf_len);

    काष्ठा tpm_res_getcap_perm_flags pf;
    पूर्णांक ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, माप(pf));
    अगर (ret)
        वापस -1;

    स_नकल(buf, &pf.perm_flags, buf_len);

    वापस 0;
}

अटल पूर्णांक
tpm12_determine_timeouts(व्योम)
{
    काष्ठा tpm_res_getcap_timeouts timeouts;
    पूर्णांक ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, माप(timeouts));
    अगर (ret)
        वापस ret;

    काष्ठा tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, माप(durations));
    अगर (ret)
        वापस ret;

    पूर्णांक i;
    क्रम (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    क्रम (i = 0; i < 4; i++)
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

    वापस 0;
}

अटल व्योम
tpm20_set_timeouts(व्योम)
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

अटल पूर्णांक
tpm12_extend(काष्ठा tpm_log_entry *le, पूर्णांक digest_len)
{
    काष्ठा tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(माप(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    स_नकल(tre.digest, le->hdr.digest, माप(tre.digest));

    काष्ठा tpm_rsp_extend rsp;
    u32 resp_लंबाई = माप(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_SHORT);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.hdr.errcode)
        वापस -1;

    वापस 0;
}

अटल पूर्णांक tpm20_extend(काष्ठा tpm_log_entry *le, पूर्णांक digest_len)
{
    काष्ठा tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(माप(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 बफर[माप(tmp_tre) + माप(le->pad)];
    काष्ठा tpm2_req_extend *tre = (काष्ठा tpm2_req_extend *)बफर;

    स_नकल(tre, &tmp_tre, माप(tmp_tre));
    स_नकल(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(माप(tmp_tre) + digest_len);

    काष्ठा tpm_rsp_header rsp;
    u32 resp_लंबाई = माप(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_SHORT);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.errcode)
        वापस -1;

    वापस 0;
}

अटल पूर्णांक
tpm_extend(काष्ठा tpm_log_entry *le, पूर्णांक digest_len)
{
    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        वापस tpm12_extend(le, digest_len);
    हाल TPM_VERSION_2:
        वापस tpm20_extend(le, digest_len);
    }
    वापस -1;
}

अटल पूर्णांक
tpm20_stirrandom(व्योम)
{
    काष्ठा tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(माप(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .आकार = cpu_to_be16(माप(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    काष्ठा tpm_rsp_header rsp;
    u32 resp_लंबाई = माप(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_SHORT);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    वापस ret;
}

अटल पूर्णांक
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    काष्ठा tpm2_res_getrandom rsp;

    अगर (buf_len > माप(rsp.rnd.बफर))
        वापस -1;

    काष्ठा tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(माप(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_लंबाई = माप(rsp);

    पूर्णांक ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_MEDIUM);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.hdr.errcode)
        ret = -1;
    अन्यथा
        स_नकल(buf, rsp.rnd.बफर, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    वापस ret;
}

अटल पूर्णांक
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    काष्ठा tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(माप(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(माप(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    काष्ठा tpm_rsp_header rsp;
    u32 resp_लंबाई = माप(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_MEDIUM);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    वापस ret;
}

अटल पूर्णांक
tpm20_hierarchychangeauth(u8 auth[20])
{
    काष्ठा tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(माप(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(माप(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .आकार = cpu_to_be16(माप(trhca.newAuth.बफर)),
        },
    };
    स_नकल(trhca.newAuth.बफर, auth, माप(trhca.newAuth.बफर));

    काष्ठा tpm_rsp_header rsp;
    u32 resp_लंबाई = माप(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_MEDIUM);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    वापस ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

अटल पूर्णांक TPM_has_physical_presence;
u8 TPM_working VARLOW;

अटल पूर्णांक
tpm_is_working(व्योम)
{
    वापस CONFIG_TCGBIOS && TPM_working;
}

अटल व्योम
tpm_set_failure(व्योम)
{
    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_Set_अस्थायी_Deactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        अवरोध;
    हाल TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        अवरोध;
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
अटल व्योम
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           स्थिर अक्षर *event, u32 event_लंबाई,
                           स्थिर u8 *hashdata, u32 hashdata_लंबाई)
{
    अगर (!tpm_is_working())
        वापस;

    काष्ठा tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    पूर्णांक digest_len = tpm_build_digest(&le, hashdata, hashdata_लंबाई, 1);
    अगर (digest_len < 0)
        वापस;
    पूर्णांक ret = tpm_extend(&le, digest_len);
    अगर (ret) {
        tpm_set_failure();
        वापस;
    }
    tpm_build_digest(&le, hashdata, hashdata_लंबाई, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_लंबाई);
}

// Add an EV_ACTION measurement to the list of measurements
अटल व्योम
tpm_add_action(u32 pcr_सूचकांक, स्थिर अक्षर *string)
{
    u32 len = म_माप(string);
    tpm_add_measurement_to_log(pcr_सूचकांक, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
अटल व्योम
tpm_add_event_separators(व्योम)
{
    अटल स्थिर u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcr_सूचकांक;
    क्रम (pcr_सूचकांक = 0; pcr_सूचकांक <= 7; pcr_सूचकांक++)
        tpm_add_measurement_to_log(pcr_सूचकांक, EV_SEPARATOR,
                                   (स्थिर अक्षर *)evt_separator,
                                   माप(evt_separator),
                                   evt_separator,
                                   माप(evt_separator));
}

अटल व्योम
tpm_smbios_measure(व्योम)
{
    काष्ठा pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    व्योम *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    अगर (!smbios_tables)
        वापस;

    sha1((स्थिर u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (स्थिर अक्षर *)&pcctes, माप(pcctes),
                               (u8 *)&pcctes, माप(pcctes));
}

अटल पूर्णांक
tpm12_assert_physical_presence(व्योम)
{
    पूर्णांक ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    अगर (!ret)
        वापस 0;

    काष्ठा tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((अक्षर *)&pf, माप(pf));
    अगर (ret)
        वापस -1;

    /* check if hardware physical presence is supported */
    अगर (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        वापस 0;
    }

    अगर (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        वापस tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    वापस -1;
}

अटल पूर्णांक
tpm12_startup(व्योम)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    पूर्णांक ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    अगर (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    अगर (ret)
        जाओ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    अगर (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    अगर (ret)
        जाओ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    अगर (ret)
        जाओ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    अगर (ret && ret != TPM_BAD_LOCALITY)
        जाओ err_exit;

    वापस 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    वापस -1;
}

अटल पूर्णांक
tpm20_startup(व्योम)
{
    tpm20_set_timeouts();

    पूर्णांक ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    अगर (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    अगर (ret)
        जाओ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    अगर (ret)
        जाओ err_exit;

    ret = tpm20_get_pcrbanks();
    अगर (ret)
        जाओ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    अगर (ret)
        जाओ err_exit;

    वापस 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    वापस -1;
}

अटल पूर्णांक
tpm_startup(व्योम)
{
    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        वापस tpm12_startup();
    हाल TPM_VERSION_2:
        वापस tpm20_startup();
    }
    वापस -1;
}

व्योम
tpm_setup(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस;

    पूर्णांक ret = tpm_tpm2_probe();
    अगर (ret) {
        ret = tpm_tcpa_probe();
        अगर (ret)
            वापस;
    }

    TPM_version = tpmhw_probe();
    अगर (TPM_version == TPM_VERSION_NONE)
        वापस;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    अगर (runningOnXen())
        वापस;

    ret = tpm_startup();
    अगर (ret)
        वापस;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

अटल व्योम
tpm20_prepboot(व्योम)
{
    पूर्णांक ret = tpm20_stirrandom();
    अगर (ret)
         जाओ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], माप(auth));
    अगर (ret)
        जाओ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    अगर (ret)
        जाओ err_exit;

    वापस;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

व्योम
tpm_prepboot(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस;

    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        अगर (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        अवरोध;
    हाल TPM_VERSION_2:
        tpm20_prepboot();
        अवरोध;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
व्योम
tpm_option_rom(स्थिर व्योम *addr, u32 len)
{
    अगर (!tpm_is_working())
        वापस;

    काष्ठा pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = माप(u16) + माप(u16) + SHA1_BUFSIZE,
    };
    sha1((स्थिर u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (स्थिर अक्षर *)&pcctes, माप(pcctes),
                               (u8 *)&pcctes, माप(pcctes));
}

व्योम
tpm_add_bcv(u32 bootdrv, स्थिर u8 *addr, u32 लंबाई)
{
    अगर (!tpm_is_working())
        वापस;

    अगर (लंबाई < 0x200)
        वापस;

    स्थिर अक्षर *string = "Booting BCV device 00h (Floppy)";
    अगर (bootdrv == 0x80)
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
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_तथ्य,
                               string, म_माप(string),
                               addr + 0x1b8, 0x48);
}

व्योम
tpm_add_cdrom(u32 bootdrv, स्थिर u8 *addr, u32 लंबाई)
{
    अगर (!tpm_is_working())
        वापस;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    स्थिर अक्षर *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, म_माप(string),
                               addr, लंबाई);
}

व्योम
tpm_add_cdrom_catalog(स्थिर u8 *addr, u32 लंबाई)
{
    अगर (!tpm_is_working())
        वापस;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    स्थिर अक्षर *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_तथ्य,
                               string, म_माप(string),
                               addr, लंबाई);
}

व्योम
tpm_s3_resume(व्योम)
{
    अगर (!tpm_is_working())
        वापस;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    पूर्णांक ret = -1;

    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        अवरोध;
    हाल TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        अगर (ret)
            जाओ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        अवरोध;
    }

    अगर (ret)
        जाओ err_exit;

    वापस;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

अटल अंतरभूत व्योम *निवेश_buf32(काष्ठा bregs *regs)
{
    वापस MAKE_FLATPTR(regs->es, regs->di);
}

अटल अंतरभूत व्योम *output_buf32(काष्ठा bregs *regs)
{
    वापस MAKE_FLATPTR(regs->ds, regs->si);
}

अटल u32
hash_log_extend(काष्ठा pcpes *pcpes, स्थिर व्योम *hashdata, u32 hashdata_लंबाई
                , व्योम *event, पूर्णांक extend)
{
    अगर (pcpes->pcrindex >= 24)
        वापस TCG_INVALID_निवेश_PARA;
    अगर (hashdata)
        sha1(hashdata, hashdata_लंबाई, pcpes->digest);

    काष्ठा tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    पूर्णांक digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    अगर (digest_len < 0)
        वापस TCG_GENERAL_ERROR;
    अगर (extend) {
        पूर्णांक ret = tpm_extend(&le, digest_len);
        अगर (ret)
            वापस TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    पूर्णांक ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    अगर (ret)
        वापस TCG_PC_LOGOVERFLOW;
    वापस 0;
}

अटल u32
hash_log_extend_event_int(स्थिर काष्ठा hleei_short *hleei_s,
                          काष्ठा hleeo *hleeo)
{
    u32 rc = 0;
    काष्ठा hleei_long *hleei_l = (काष्ठा hleei_long *)hleei_s;
    स्थिर व्योम *logdataptr;
    u32 logdatalen;
    काष्ठा pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    चयन (hleei_s->ipblength) {
    हाल माप(काष्ठा hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    अवरोध;

    हाल माप(काष्ठा hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    अवरोध;

    शेष:
        /* bad input block */
        rc = TCG_INVALID_निवेश_PARA;
        जाओ err_exit;
    }

    pcpes = (काष्ठा pcpes *)logdataptr;

    अगर (pcpes->pcrindex != pcrindex
        || logdatalen != माप(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_निवेश_PARA;
        जाओ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    अगर (rc)
        जाओ err_exit;

    hleeo->opblength = माप(काष्ठा hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_गिनती;
    स_नकल(hleeo->digest, pcpes->digest, माप(hleeo->digest));

err_exit:
    अगर (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    वापस rc;
}

अटल u32
pass_through_to_tpm_int(काष्ठा pttti *pttti, काष्ठा pttto *pttto)
{
    u32 rc = 0;
    काष्ठा tpm_req_header *trh = (व्योम*)pttti->tpmopin;

    अगर (pttti->ipblength < माप(काष्ठा pttti) + माप(*trh)
        || pttti->ipblength != माप(काष्ठा pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < माप(काष्ठा pttto)) {
        rc = TCG_INVALID_निवेश_PARA;
        जाओ err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        अगर (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_निवेश_PARA;
            जाओ err_exit;
        }
        अवरोध;
    हाल TPM_VERSION_2:
        अगर (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_निवेश_PARA;
            जाओ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - दुरत्व(काष्ठा pttto, tpmopout);
    पूर्णांक ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    अगर (ret) {
        rc = TCG_FATAL_COM_ERROR;
        जाओ err_exit;
    }

    pttto->opblength = दुरत्व(काष्ठा pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    अगर (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    वापस rc;
}

अटल u32
shutdown_preboot_interface(व्योम)
{
    TPM_interface_shutdown = 1;
    वापस 0;
}

अटल u32
hash_log_event_int(स्थिर काष्ठा hlei *hlei, काष्ठा hleo *hleo)
{
    u32 rc = 0;
    u16 आकार;
    काष्ठा pcpes *pcpes;

    आकार = hlei->ipblength;
    अगर (आकार != माप(*hlei)) {
        rc = TCG_INVALID_निवेश_PARA;
        जाओ err_exit;
    }

    pcpes = (काष्ठा pcpes *)hlei->logdataptr;

    अगर (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != माप(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_निवेश_PARA;
        जाओ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    अगर (rc)
        जाओ err_exit;

    /* updating the log was fine */
    hleo->opblength = माप(काष्ठा hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_गिनती;

err_exit:
    अगर (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    वापस rc;
}

अटल u32
hash_all_int(स्थिर काष्ठा hai *hai, u8 *hash)
{
    अगर (hai->ipblength != माप(काष्ठा hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        वापस TCG_INVALID_निवेश_PARA;

    sha1((स्थिर u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    वापस 0;
}

अटल u32
tss_int(काष्ठा ti *ti, काष्ठा to *to)
{
    to->opblength = माप(काष्ठा to);
    to->reserved  = 0;

    वापस TCG_PC_UNSUPPORTED;
}

अटल u32
compact_hash_log_extend_event_int(u8 *बफर,
                                  u32 info,
                                  u32 लंबाई,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    काष्ठा pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = माप(info),
    };
    u32 rc = hash_log_extend(&pcpes, बफर, लंबाई, &info, 1);
    अगर (rc)
        वापस rc;

    *edx_ptr = tpm_state.entry_गिनती;
    वापस 0;
}

व्योम VISIBLE32FLAT
tpm_interrupt_handler32(काष्ठा bregs *regs)
{
    अगर (!CONFIG_TCGBIOS)
        वापस;

    set_cf(regs, 0);

    अगर (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        वापस;
    }

    चयन ((क्रमागत irq_ids)regs->al) {
    हाल TCG_StatusCheck:
        अगर (!tpmhw_is_present()) {
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
        अवरोध;

    हाल TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (काष्ठा hleei_short *)निवेश_buf32(regs),
                  (काष्ठा hleeo *)output_buf32(regs));
        अवरोध;

    हाल TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((काष्ठा pttti *)निवेश_buf32(regs),
                                    (काष्ठा pttto *)output_buf32(regs));
        अवरोध;

    हाल TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        अवरोध;

    हाल TCG_HashLogEvent:
        regs->eax = hash_log_event_int((काष्ठा hlei*)निवेश_buf32(regs),
                                       (काष्ठा hleo*)output_buf32(regs));
        अवरोध;

    हाल TCG_HashAll:
        regs->eax =
            hash_all_int((काष्ठा hai*)निवेश_buf32(regs),
                          (u8 *)output_buf32(regs));
        अवरोध;

    हाल TCG_TSS:
        regs->eax = tss_int((काष्ठा ti*)निवेश_buf32(regs),
                            (काष्ठा to*)output_buf32(regs));
        अवरोध;

    हाल TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)निवेश_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        अवरोध;

    शेष:
        set_cf(regs, 1);
    }

    वापस;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

प्रकार u8 tpm_ppi_code;

अटल पूर्णांक
tpm12_read_has_owner(पूर्णांक *has_owner)
{
    काष्ठा tpm_res_getcap_ownerauth oauth;
    पूर्णांक ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, माप(oauth));
    अगर (ret)
        वापस -1;

    *has_owner = oauth.flag;

    वापस 0;
}

अटल पूर्णांक
tpm12_enable_tpm(पूर्णांक enable, पूर्णांक verbose)
{
    काष्ठा tpm_permanent_flags pf;
    पूर्णांक ret = tpm12_read_permanent_flags((अक्षर *)&pf, माप(pf));
    अगर (ret)
        वापस -1;

    अगर (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        वापस 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    अगर (ret) {
        अगर (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        अन्यथा
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    वापस ret;
}

अटल पूर्णांक
tpm12_activate_tpm(पूर्णांक activate, पूर्णांक allow_reset, पूर्णांक verbose)
{
    काष्ठा tpm_permanent_flags pf;
    पूर्णांक ret = tpm12_read_permanent_flags((अक्षर *)&pf, माप(pf));
    अगर (ret)
        वापस -1;

    अगर (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        वापस 0;

    अगर (pf.flags[PERM_FLAG_IDX_DISABLE])
        वापस 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    अगर (ret)
        वापस ret;

    अगर (activate && allow_reset) {
        अगर (verbose) {
            म_लिखो("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    वापस 0;
}

अटल पूर्णांक
tpm12_enable_activate(पूर्णांक allow_reset, पूर्णांक verbose)
{
    पूर्णांक ret = tpm12_enable_tpm(1, verbose);
    अगर (ret)
        वापस ret;

    वापस tpm12_activate_tpm(1, allow_reset, verbose);
}

अटल पूर्णांक
tpm12_force_clear(पूर्णांक enable_activate_before, पूर्णांक enable_activate_after,
                  पूर्णांक verbose)
{
    पूर्णांक has_owner;
    पूर्णांक ret = tpm12_read_has_owner(&has_owner);
    अगर (ret)
        वापस -1;
    अगर (!has_owner) {
        अगर (verbose)
            म_लिखो("TPM does not have an owner.\n");
        वापस 0;
    }

    अगर (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        अगर (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            वापस ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    अगर (ret)
        वापस ret;

    अगर (!enable_activate_after) {
        अगर (verbose)
            म_लिखो("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        वापस 0;
    }

    वापस tpm12_enable_activate(1, verbose);
}

अटल पूर्णांक
tpm12_set_owner_install(पूर्णांक allow, पूर्णांक verbose)
{
    पूर्णांक has_owner;
    पूर्णांक ret = tpm12_read_has_owner(&has_owner);
    अगर (ret)
        वापस -1;
    अगर (has_owner) {
        अगर (verbose)
            म_लिखो("Must first remove owner.\n");
        वापस 0;
    }

    काष्ठा tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((अक्षर *)&pf, माप(pf));
    अगर (ret)
        वापस -1;

    अगर (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        अगर (verbose)
            म_लिखो("TPM must first be enable.\n");
        वापस 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    अगर (ret)
        वापस ret;

    अगर (verbose)
        म_लिखो("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    वापस 0;
}

अटल पूर्णांक
tpm12_process_cfg(tpm_ppi_code संदेश_Code, पूर्णांक verbose)
{
    पूर्णांक ret = 0;

    चयन (संदेश_Code) {
        हाल TPM_PPI_OP_NOOP: /* no-op */
            अवरोध;

        हाल TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            अवरोध;

        हाल TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            अवरोध;

        हाल TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            अवरोध;

        हाल TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            अवरोध;

        हाल TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            अवरोध;

        हाल TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            अवरोध;

        हाल TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            अवरोध;

        शेष:
            अवरोध;
    }

    अगर (ret)
        म_लिखो("Op %d: An error occurred: 0x%x\n", संदेश_Code, ret);

    वापस ret;
}

अटल पूर्णांक
tpm20_clearcontrol(u8 disable, पूर्णांक verbose)
{
    काष्ठा tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(माप(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(माप(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    काष्ठा tpm_rsp_header rsp;
    u32 resp_लंबाई = माप(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_SHORT);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    वापस ret;
}

अटल पूर्णांक
tpm20_clear(व्योम)
{
    काष्ठा tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(माप(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(माप(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    काष्ठा tpm_rsp_header rsp;
    u32 resp_लंबाई = माप(rsp);
    पूर्णांक ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_लंबाई,
                             TPM_DURATION_TYPE_MEDIUM);
    अगर (ret || resp_लंबाई != माप(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    वापस ret;
}

अटल पूर्णांक
tpm20_process_cfg(tpm_ppi_code संदेश_Code, पूर्णांक verbose)
{
    पूर्णांक ret = 0;

    चयन (संदेश_Code) {
        हाल TPM_PPI_OP_NOOP: /* no-op */
            अवरोध;

        हाल TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            अगर (!ret)
                 ret = tpm20_clear();
            अवरोध;
    }

    अगर (ret)
        म_लिखो("Op %d: An error occurred: 0x%x\n", संदेश_Code, ret);

    वापस ret;
}

अटल पूर्णांक
tpm12_get_tpm_state(व्योम)
{
    पूर्णांक state = 0;
    काष्ठा tpm_permanent_flags pf;
    पूर्णांक has_owner;

    अगर (tpm12_read_permanent_flags((अक्षर *)&pf, माप(pf)) ||
        tpm12_read_has_owner(&has_owner))
        वापस ~0;

    अगर (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    अगर (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    अगर (has_owner) {
        state |= TPM_STATE_OWNED;
    } अन्यथा {
        अगर (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    वापस state;
}

अटल व्योम
tpm12_show_tpm_menu(पूर्णांक state, पूर्णांक अग्र_scancodes[7])
{
    पूर्णांक i = 0;

    म_लिखो("\nThe current state of the TPM is:\n");

    अगर (state & TPM_STATE_ENABLED)
        म_लिखो("  Enabled");
    अन्यथा
        म_लिखो("  Disabled");

    अगर (state & TPM_STATE_ACTIVE)
        म_लिखो(" and active\n");
    अन्यथा
        म_लिखो(" and deactivated\n");

    अगर (state & TPM_STATE_OWNED)
        म_लिखो("  Ownership has been taken\n");
    अन्यथा {
        म_लिखो("  Ownership has not been taken\n");
        अगर (state & TPM_STATE_OWNERINSTALL)
            म_लिखो("  A user can take ownership of the TPM\n");
        अन्यथा
            म_लिखो("  Taking ownership of the TPM has been disabled\n");
    }

    अगर ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        म_लिखो("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    म_लिखो("\nAvailable options are:\n");
    अगर (state & TPM_STATE_ENABLED) {
        म_लिखो(" d. Disable the TPM\n");
        अग्र_scancodes[i++] = 32;

        अगर (state & TPM_STATE_ACTIVE) {
            म_लिखो(" v. Deactivate the TPM\n");
            अग्र_scancodes[i++] = 47;

            अगर (state & TPM_STATE_OWNERINSTALL) {
                म_लिखो(" p. Prevent installation of an owner\n");
                अग्र_scancodes[i++] = 25;
            } अन्यथा {
                म_लिखो(" s. Allow installation of an owner\n");
                अग्र_scancodes[i++] = 31;
            }
        } अन्यथा {
            म_लिखो(" a. Activate the TPM\n");
            अग्र_scancodes[i++] = 30;
        }

    } अन्यथा {
        म_लिखो(" e. Enable the TPM\n");
        अग्र_scancodes[i++] = 18;
    }

    अगर (state & TPM_STATE_OWNED) {
        म_लिखो(" c. Clear ownership\n");
        अग्र_scancodes[i++] = 46;
    }

    अग्र_scancodes[i++] = 0;
}

अटल व्योम
tpm12_menu(व्योम)
{
    पूर्णांक scancode, अग्र_scancodes[7];
    tpm_ppi_code संदेश_Code;
    पूर्णांक state = 0, i;
    पूर्णांक waitkey;

    म_लिखो("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    क्रम (;;) {
        अगर ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, अग्र_scancodes);
        } अन्यथा {
            म_लिखो("TPM is not working correctly.\n");
            वापस;
        }

        म_लिखो("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        संदेश_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        जबतक (waitkey) {
            जबतक ((scancode = get_keystroke(1000)) == ~0)
                ;

            चयन (scancode) {
            हाल 1:
                // ESC
                reset();
                अवरोध;
            हाल 18: /* e. enable */
                संदेश_Code = TPM_PPI_OP_ENABLE;
                अवरोध;
            हाल 32: /* d. disable */
                संदेश_Code = TPM_PPI_OP_DISABLE;
                अवरोध;
            हाल 30: /* a. activate */
                संदेश_Code = TPM_PPI_OP_ACTIVATE;
                अवरोध;
            हाल 47: /* v. deactivate */
                संदेश_Code = TPM_PPI_OP_DEACTIVATE;
                अवरोध;
            हाल 46: /* c. clear owner */
                संदेश_Code = TPM_PPI_OP_CLEAR;
                अवरोध;
            हाल 25: /* p. prevent ownerinstall */
                संदेश_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                अवरोध;
            हाल 31: /* s. allow ownerinstall */
                संदेश_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                अवरोध;
            शेष:
                जारी;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            क्रम (i = 0; i < माप(अग्र_scancodes); i++) {
                अगर (अग्र_scancodes[i] == 0)
                    अवरोध;

                अगर (अग्र_scancodes[i] == scancode) {
                    tpm12_process_cfg(संदेश_Code, 1);
                    waitkey = 0;
                    अवरोध;
                }
            }
        }
    }
}

अटल पूर्णांक
tpm20_menu_change_active_pcrbanks(व्योम)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    जबतक (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        म_लिखो("\nToggle active PCR banks by pressing number key\n\n");

        जबतक (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            स्थिर अक्षर *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            अगर (hashname) {
                म_लिखो("  %d: %s", i, hashname);
                अगर (activate_banks & hashalg_flag)
                    म_लिखो(" (enabled)");
                म_लिखो("\n");
            }

            hashalg_flag <<= 1;
        }
        म_लिखो("\n"
               "ESC: return to previous menu without changes\n");
        अगर (activate_banks)
            म_लिखो("A  : activate selection\n");

        u8 flagnum;
        पूर्णांक show = 0;
        जबतक (!show) {
            पूर्णांक scancode = get_keystroke(1000);

            चयन (scancode) {
            हाल ~0:
                जारी;
            हाल 1: /* ESC */
                म_लिखो("\n");
                वापस -1;
            हाल 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                अगर (flagnum > i)
                    जारी;
                अगर (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                अवरोध;
            हाल 30: /* a */
                अगर (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

अटल व्योम
tpm20_menu(व्योम)
{
    पूर्णांक scan_code;
    tpm_ppi_code संदेश_Code;

    क्रम (;;) {
        म_लिखो("1. Clear TPM\n");
        म_लिखो("2. Change active PCR banks\n");

        म_लिखो("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        संदेश_Code = TPM_PPI_OP_NOOP;

        जबतक ((scan_code = get_keystroke(1000)) == ~0)
            ;

        चयन (scan_code) {
        हाल 1:
            // ESC
            reset();
            अवरोध;
        हाल 2:
            संदेश_Code = TPM_PPI_OP_CLEAR;
            अवरोध;
        हाल 3:
            tpm20_menu_change_active_pcrbanks();
            जारी;
        शेष:
            जारी;
        }

        tpm20_process_cfg(संदेश_Code, 0);
    }
}

व्योम
tpm_menu(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस;

    जबतक (get_keystroke(0) >= 0)
        ;
    wait_threads();

    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        tpm12_menu();
        अवरोध;
    हाल TPM_VERSION_2:
        tpm20_menu();
        अवरोध;
    }
}

पूर्णांक
tpm_can_show_menu(व्योम)
{
    चयन (TPM_version) {
    हाल TPM_VERSION_1_2:
        वापस tpm_is_working() && TPM_has_physical_presence;
    हाल TPM_VERSION_2:
        वापस tpm_is_working();
    }
    वापस 0;
}
