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

#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "byteorder.h" // cpu_to_*
#അന്തര്ഭാവ "config.h" // CONFIG_TCGBIOS
#അന്തര്ഭാവ "farptr.h" // MAKE_FLATPTR
#അന്തര്ഭാവ "fw/paravirt.h" // runningOnXen
#അന്തര്ഭാവ "hw/tpm_drivers.h" // tpm_drivers[]
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "sha.h" // sha1, sha256, ...
#അന്തര്ഭാവ "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#അന്തര്ഭാവ "std/smbios.h" // struct smbios_21_entry_point
#അന്തര്ഭാവ "std/tcg.h" // TCG_PC_LOGOVERFLOW
#അന്തര്ഭാവ "string.h" // checksum
#അന്തര്ഭാവ "tcgbios.h"// tpm_*, prototypes
#അന്തര്ഭാവ "util.h" // printf, get_keystroke
#അന്തര്ഭാവ "stacks.h" // wait_threads, reset
#അന്തര്ഭാവ "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

ഘടന {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_എണ്ണം;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

സ്ഥിതം പൂർണ്ണസംഖ്യ tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    എങ്കിൽ (!log_area_start_address || !log_area_minimum_length)
        തിരികെ -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = ശൂന്യ;
    tpm_state.entry_എണ്ണം = 0;
    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_tcpa_probe(ശൂന്യം)
{
    ഘടന tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    എങ്കിൽ (!tcpa)
        തിരികെ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    തിരികെ tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_tpm2_probe(ശൂന്യം)
{
    ഘടന tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    എങ്കിൽ (!tpm2)
        തിരികെ -1;

    എങ്കിൽ (tpm2->length < 76)
        തിരികെ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    തിരികെ tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_log_event(ഘടന tpm_log_header *entry, പൂർണ്ണസംഖ്യ digest_len
              , സ്ഥിരം ശൂന്യം *event, പൂർണ്ണസംഖ്യ event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    എങ്കിൽ (tpm_state.log_area_next_entry == ശൂന്യ)
        തിരികെ -1;

    u32 size = (വലുപ്പം(*entry) + digest_len
                + വലുപ്പം(ഘടന tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    എങ്കിൽ (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        തിരികെ -1;
    }

    ശൂന്യം *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, വലുപ്പം(*entry) + digest_len);
    ഘടന tpm_log_trailer *t = dest + വലുപ്പം(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_എണ്ണം++;

    തിരികെ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

സ്ഥിതം TPMVersion TPM_version;
സ്ഥിതം u32 tpm20_pcr_selection_size;
സ്ഥിതം ഘടന tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
ഘടന tpm_log_entry {
    ഘടന tpm_log_header hdr;
    u8 pad[വലുപ്പം(ഘടന tpm2_digest_മൂല്യങ്ങൾ)
           + 8 * വലുപ്പം(ഘടന tpm2_digest_മൂല്യം)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

സ്ഥിതം സ്ഥിരം ഘടന hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    സ്ഥിരം അക്ഷരം *name;
    ശൂന്യം (*hashfunc)(സ്ഥിരം u8 *data, u32 length, u8 *hash);
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

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    വേണ്ടി (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        എങ്കിൽ (hash_parameters[i].hashalg == hashAlg)
            തിരികെ hash_parameters[i].hash_buffersize;
    }
    തിരികെ -1;
}

സ്ഥിതം u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    വേണ്ടി (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        എങ്കിൽ (hash_parameters[i].hashalg == hashAlg)
            തിരികെ hash_parameters[i].hashalg_flag;
    }
    തിരികെ 0;
}

സ്ഥിതം u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    വേണ്ടി (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        എങ്കിൽ (hash_parameters[i].hashalg_flag == hashalg_flag)
            തിരികെ hash_parameters[i].hashalg;
    }
    തിരികെ 0;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    വേണ്ടി (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        എങ്കിൽ (hash_parameters[i].hashalg_flag == hashalg_flag)
            തിരികെ hash_parameters[i].name;
    }
    തിരികെ ശൂന്യ;
}

സ്ഥിതം ശൂന്യം tpm2_hash_data(u16 hashAlg, സ്ഥിരം u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    വേണ്ടി (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        എങ്കിൽ (hash_parameters[i].hashalg == hashAlg) {
            എങ്കിൽ (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } അല്ലെങ്കിൽ {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_write_EfiSpecIdEventStruct(ശൂന്യം)
{
    എങ്കിൽ (!tpm20_pcr_selection)
        തിരികെ -1;

    ഘടന {
        ഘടന TCG_EfiSpecIdEventStruct hdr;
        u8 pad[വലുപ്പം(ഘടന tpm_log_entry) + വലുപ്പം(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    ഘടന tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ശൂന്യം *nsel, *end = (ശൂന്യം*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 എണ്ണം, അക്കം_Algs = 0;
    വേണ്ടി (എണ്ണം = 0; എണ്ണം < be32_to_cpu(tpm20_pcr_selection->എണ്ണം); എണ്ണം++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (ശൂന്യം*)sel + വലുപ്പം(*sel) + sizeOfSelect;
        എങ്കിൽ (nsel > end)
            നിർത്തുക;

        എങ്കിൽ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            തുടരുക;
        }

        പൂർണ്ണസംഖ്യ hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        എങ്കിൽ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            തിരികെ -1;
        }

        പൂർണ്ണസംഖ്യ event_size = offsetof(ഘടന TCG_EfiSpecIdEventStruct
                                  , digestSizes[എണ്ണം+1]);
        എങ്കിൽ (event_size > വലുപ്പം(event) - വലുപ്പം(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            തിരികെ -1;
        }

        event.hdr.digestSizes[അക്കം_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[അക്കം_Algs].digestSize = hsize;
        അക്കം_Algs++;

        sel = nsel;
    }

    എങ്കിൽ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        തിരികെ -1;
    }

    event.hdr.സംഖ്യ_Of_Algorithms = അക്കം_Algs;
    പൂർണ്ണസംഖ്യ event_size = offsetof(ഘടന TCG_EfiSpecIdEventStruct
                              , digestSizes[അക്കം_Algs]);
    u8 *vendorInfoSize = (ശൂന്യം*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += വലുപ്പം(*vendorInfoSize);

    ഘടന tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    തിരികെ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_build_digest(ഘടന tpm_log_entry *le,
                   സ്ഥിരം u8 *hashdata, u32 hashdata_len, പൂർണ്ണസംഖ്യ bigEndian)
{
    എങ്കിൽ (!tpm20_pcr_selection)
        തിരികെ -1;

    ഘടന tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ശൂന്യം *nsel, *end = (ശൂന്യം*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    ശൂന്യം *dest = le->hdr.digest + വലുപ്പം(ഘടന tpm2_digest_മൂല്യങ്ങൾ);

    u32 എണ്ണം, അക്കം_Algs = 0;
    വേണ്ടി (എണ്ണം = 0; എണ്ണം < be32_to_cpu(tpm20_pcr_selection->എണ്ണം); എണ്ണം++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (ശൂന്യം*)sel + വലുപ്പം(*sel) + sizeOfSelect;
        എങ്കിൽ (nsel > end)
            നിർത്തുക;

        /* PCR 0-7 unused? -- skip */
        എങ്കിൽ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            തുടരുക;
        }

        പൂർണ്ണസംഖ്യ hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        എങ്കിൽ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            തിരികെ -1;
        }

        /* buffer size sanity check before writing */
        ഘടന tpm2_digest_മൂല്യം *v = dest;
        എങ്കിൽ (dest + വലുപ്പം(*v) + hsize > (ശൂന്യം*)le + വലുപ്പം(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            തിരികെ -1;
        }

        എങ്കിൽ (bigEndian)
            v->hashAlg = sel->hashAlg;
        അല്ലെങ്കിൽ
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += വലുപ്പം(*v) + hsize;
        sel = nsel;

        അക്കം_Algs++;
    }

    എങ്കിൽ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        തിരികെ -1;
    }

    ഘടന tpm2_digest_മൂല്യങ്ങൾ *v = (ശൂന്യം*)le->hdr.digest;
    എങ്കിൽ (bigEndian)
        v->എണ്ണം = cpu_to_be32(അക്കം_Algs);
    അല്ലെങ്കിൽ
        v->എണ്ണം = അക്കം_Algs;

    തിരികെ dest - (ശൂന്യം*)le->hdr.digest;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_build_digest(ഘടന tpm_log_entry *le,
                   സ്ഥിരം u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    തിരികെ SHA1_BUFSIZE;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_build_digest_direct(ഘടന tpm_log_entry *le, സ്ഥിരം u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    തിരികെ SHA1_BUFSIZE;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_build_digest(ഘടന tpm_log_entry *le, സ്ഥിരം u8 *hashdata, u32 hashdata_len
                 , പൂർണ്ണസംഖ്യ bigEndian)
{
    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        തിരികെ tpm12_build_digest(le, hashdata, hashdata_len);
    അവസ്ഥ TPM_VERSION_2:
        തിരികെ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    തിരികെ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_simple_cmd(u8 locty, u32 ordinal
               , പൂർണ്ണസംഖ്യ param_size, u16 param, എണ്ണൽ tpmDurationType to_t)
{
    ഘടന {
        ഘടന tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(വലുപ്പം(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        നിർത്തുക;
    അവസ്ഥ TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        നിർത്തുക;
    }

    u8 obuffer[64];
    ഘടന tpm_rsp_header *trsh = (ശൂന്യം*)obuffer;
    u32 obuffer_len = വലുപ്പം(obuffer);
    memset(obuffer, 0x0, വലുപ്പം(obuffer));

    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_getcapability(u32 capability, u32 property, u32 എണ്ണം,
                    ഘടന tpm_rsp_header *rsp, u32 rsize)
{
    ഘടന tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(വലുപ്പം(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(എണ്ണം),
    };

    u32 resp_size = rsize;
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_get_pcrbanks(ശൂന്യം)
{
    u8 buffer[128];
    ഘടന tpm2_res_getcapability *trg =
      (ഘടന tpm2_res_getcapability *)&buffer;

    പൂർണ്ണസംഖ്യ ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  വലുപ്പം(buffer));
    എങ്കിൽ (ret)
        തിരികെ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    എങ്കിൽ (resplen <= offsetof(ഘടന tpm2_res_getcapability, data))
        തിരികെ -1;

    u32 size = resplen - offsetof(ഘടന tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    എങ്കിൽ (size < offsetof(ഘടന tpml_pcr_selection, selections) +
               offsetof(ഘടന tpms_pcr_selection, pcrSelect))
        തിരികെ -1;

    tpm20_pcr_selection = malloc_മുകളിലെ(size);
    എങ്കിൽ (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } അല്ലെങ്കിൽ {
        warn_noalloc();
        ret = -1;
    }

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    എങ്കിൽ (!tpm20_pcr_selection)
        തിരികെ -1;

    ഘടന tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ശൂന്യം *end = (ശൂന്യം*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    ആകുമ്പോൾ (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        ശൂന്യം *nsel = (ശൂന്യം*)sel + വലുപ്പം(*sel) + sizeOfSelect;
        എങ്കിൽ (nsel > end)
            തിരികെ 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        വേണ്ടി (i = 0; i < sizeOfSelect; i++) {
            എങ്കിൽ (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                നിർത്തുക;
            }
        }

        sel = nsel;
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_set_pcrbanks(u32 active_banks)
{
    ഘടന tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(വലുപ്പം(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ഘടന tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    പൂർണ്ണസംഖ്യ i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    ആകുമ്പോൾ (hashalg_flag) {
        എങ്കിൽ ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            എങ്കിൽ (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                എങ്കിൽ (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.എണ്ണം = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * വലുപ്പം(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(ഘടന tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * വലുപ്പം(tps[0]));

    ഘടന tpm_rsp_header rsp;
    u32 resp_length = വലുപ്പം(rsp);

    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tpm20_activate_pcrbanks(u32 active_banks)
{
    പൂർണ്ണസംഖ്യ ret = tpm20_set_pcrbanks(active_banks);
    എങ്കിൽ (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (!ret)
        reset();
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_get_capability(u32 cap, u32 subcap, ഘടന tpm_rsp_header *rsp, u32 rsize)
{
    ഘടന tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(വലുപ്പം(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(വലുപ്പം(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_read_permanent_flags(അക്ഷരം *buf, പൂർണ്ണസംഖ്യ buf_len)
{
    memset(buf, 0, buf_len);

    ഘടന tpm_res_getcap_perm_flags pf;
    പൂർണ്ണസംഖ്യ ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, വലുപ്പം(pf));
    എങ്കിൽ (ret)
        തിരികെ -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_determine_timeouts(ശൂന്യം)
{
    ഘടന tpm_res_getcap_timeouts timeouts;
    പൂർണ്ണസംഖ്യ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, വലുപ്പം(timeouts));
    എങ്കിൽ (ret)
        തിരികെ ret;

    ഘടന tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, വലുപ്പം(durations));
    എങ്കിൽ (ret)
        തിരികെ ret;

    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    വേണ്ടി (i = 0; i < 4; i++)
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

    തിരികെ 0;
}

സ്ഥിതം ശൂന്യം
tpm20_set_timeouts(ശൂന്യം)
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

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_extend(ഘടന tpm_log_entry *le, പൂർണ്ണസംഖ്യ digest_len)
{
    ഘടന tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(വലുപ്പം(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, വലുപ്പം(tre.digest));

    ഘടന tpm_rsp_extend rsp;
    u32 resp_length = വലുപ്പം(rsp);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.hdr.errcode)
        തിരികെ -1;

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tpm20_extend(ഘടന tpm_log_entry *le, പൂർണ്ണസംഖ്യ digest_len)
{
    ഘടന tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(വലുപ്പം(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[വലുപ്പം(tmp_tre) + വലുപ്പം(le->pad)];
    ഘടന tpm2_req_extend *tre = (ഘടന tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, വലുപ്പം(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(വലുപ്പം(tmp_tre) + digest_len);

    ഘടന tpm_rsp_header rsp;
    u32 resp_length = വലുപ്പം(rsp);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.errcode)
        തിരികെ -1;

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_extend(ഘടന tpm_log_entry *le, പൂർണ്ണസംഖ്യ digest_len)
{
    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        തിരികെ tpm12_extend(le, digest_len);
    അവസ്ഥ TPM_VERSION_2:
        തിരികെ tpm20_extend(le, digest_len);
    }
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_stirrandom(ശൂന്യം)
{
    ഘടന tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(വലുപ്പം(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(വലുപ്പം(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    ഘടന tpm_rsp_header rsp;
    u32 resp_length = വലുപ്പം(rsp);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    ഘടന tpm2_res_getrandom rsp;

    എങ്കിൽ (buf_len > വലുപ്പം(rsp.rnd.buffer))
        തിരികെ -1;

    ഘടന tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(വലുപ്പം(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = വലുപ്പം(rsp);

    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.hdr.errcode)
        ret = -1;
    അല്ലെങ്കിൽ
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    ഘടന tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(വലുപ്പം(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(വലുപ്പം(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    ഘടന tpm_rsp_header rsp;
    u32 resp_length = വലുപ്പം(rsp);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_hierarchychangeauth(u8 auth[20])
{
    ഘടന tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(വലുപ്പം(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(വലുപ്പം(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(വലുപ്പം(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, വലുപ്പം(trhca.newAuth.buffer));

    ഘടന tpm_rsp_header rsp;
    u32 resp_length = വലുപ്പം(rsp);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    തിരികെ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

സ്ഥിതം പൂർണ്ണസംഖ്യ TPM_has_physical_presence;
u8 TPM_working VARLOW;

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_is_working(ശൂന്യം)
{
    തിരികെ CONFIG_TCGBIOS && TPM_working;
}

സ്ഥിതം ശൂന്യം
tpm_set_failure(ശൂന്യം)
{
    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        നിർത്തുക;
    അവസ്ഥ TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        നിർത്തുക;
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
സ്ഥിതം ശൂന്യം
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           സ്ഥിരം അക്ഷരം *event, u32 event_length,
                           സ്ഥിരം u8 *hashdata, u32 hashdata_length)
{
    എങ്കിൽ (!tpm_is_working())
        തിരികെ;

    ഘടന tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    പൂർണ്ണസംഖ്യ digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    എങ്കിൽ (digest_len < 0)
        തിരികെ;
    പൂർണ്ണസംഖ്യ ret = tpm_extend(&le, digest_len);
    എങ്കിൽ (ret) {
        tpm_set_failure();
        തിരികെ;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
സ്ഥിതം ശൂന്യം
tpm_add_action(u32 pcrIndex, സ്ഥിരം അക്ഷരം *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
സ്ഥിതം ശൂന്യം
tpm_add_event_separators(ശൂന്യം)
{
    സ്ഥിതം സ്ഥിരം u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    വേണ്ടി (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (സ്ഥിരം അക്ഷരം *)evt_separator,
                                   വലുപ്പം(evt_separator),
                                   evt_separator,
                                   വലുപ്പം(evt_separator));
}

സ്ഥിതം ശൂന്യം
tpm_smbios_measure(ശൂന്യം)
{
    ഘടന pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    ശൂന്യം *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    എങ്കിൽ (!smbios_tables)
        തിരികെ;

    sha1((സ്ഥിരം u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (സ്ഥിരം അക്ഷരം *)&pcctes, വലുപ്പം(pcctes),
                               (u8 *)&pcctes, വലുപ്പം(pcctes));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_assert_physical_presence(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (!ret)
        തിരികെ 0;

    ഘടന tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((അക്ഷരം *)&pf, വലുപ്പം(pf));
    എങ്കിൽ (ret)
        തിരികെ -1;

    /* check if hardware physical presence is supported */
    എങ്കിൽ (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        തിരികെ 0;
    }

    എങ്കിൽ (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        തിരികെ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_startup(ശൂന്യം)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    പൂർണ്ണസംഖ്യ ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    എങ്കിൽ (ret)
        പോകുക err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    എങ്കിൽ (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    എങ്കിൽ (ret)
        പോകുക err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    എങ്കിൽ (ret)
        പോകുക err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret && ret != TPM_BAD_LOCALITY)
        പോകുക err_exit;

    തിരികെ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_startup(ശൂന്യം)
{
    tpm20_set_timeouts();

    പൂർണ്ണസംഖ്യ ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    എങ്കിൽ (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    എങ്കിൽ (ret)
        പോകുക err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    എങ്കിൽ (ret)
        പോകുക err_exit;

    ret = tpm20_get_pcrbanks();
    എങ്കിൽ (ret)
        പോകുക err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    എങ്കിൽ (ret)
        പോകുക err_exit;

    തിരികെ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm_startup(ശൂന്യം)
{
    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        തിരികെ tpm12_startup();
    അവസ്ഥ TPM_VERSION_2:
        തിരികെ tpm20_startup();
    }
    തിരികെ -1;
}

ശൂന്യം
tpm_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ;

    പൂർണ്ണസംഖ്യ ret = tpm_tpm2_probe();
    എങ്കിൽ (ret) {
        ret = tpm_tcpa_probe();
        എങ്കിൽ (ret)
            തിരികെ;
    }

    TPM_version = tpmhw_probe();
    എങ്കിൽ (TPM_version == TPM_VERSION_NONE)
        തിരികെ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    എങ്കിൽ (runningOnXen())
        തിരികെ;

    ret = tpm_startup();
    എങ്കിൽ (ret)
        തിരികെ;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

സ്ഥിതം ശൂന്യം
tpm20_prepboot(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ ret = tpm20_stirrandom();
    എങ്കിൽ (ret)
         പോകുക err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], വലുപ്പം(auth));
    എങ്കിൽ (ret)
        പോകുക err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    എങ്കിൽ (ret)
        പോകുക err_exit;

    തിരികെ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

ശൂന്യം
tpm_prepboot(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ;

    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        എങ്കിൽ (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        നിർത്തുക;
    അവസ്ഥ TPM_VERSION_2:
        tpm20_prepboot();
        നിർത്തുക;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
ശൂന്യം
tpm_option_rom(സ്ഥിരം ശൂന്യം *addr, u32 len)
{
    എങ്കിൽ (!tpm_is_working())
        തിരികെ;

    ഘടന pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = വലുപ്പം(u16) + വലുപ്പം(u16) + SHA1_BUFSIZE,
    };
    sha1((സ്ഥിരം u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (സ്ഥിരം അക്ഷരം *)&pcctes, വലുപ്പം(pcctes),
                               (u8 *)&pcctes, വലുപ്പം(pcctes));
}

ശൂന്യം
tpm_add_bcv(u32 bootdrv, സ്ഥിരം u8 *addr, u32 length)
{
    എങ്കിൽ (!tpm_is_working())
        തിരികെ;

    എങ്കിൽ (length < 0x200)
        തിരികെ;

    സ്ഥിരം അക്ഷരം *string = "Booting BCV device 00h (Floppy)";
    എങ്കിൽ (bootdrv == 0x80)
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

ശൂന്യം
tpm_add_cdrom(u32 bootdrv, സ്ഥിരം u8 *addr, u32 length)
{
    എങ്കിൽ (!tpm_is_working())
        തിരികെ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    സ്ഥിരം അക്ഷരം *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

ശൂന്യം
tpm_add_cdrom_catalog(സ്ഥിരം u8 *addr, u32 length)
{
    എങ്കിൽ (!tpm_is_working())
        തിരികെ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    സ്ഥിരം അക്ഷരം *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

ശൂന്യം
tpm_s3_resume(ശൂന്യം)
{
    എങ്കിൽ (!tpm_is_working())
        തിരികെ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    പൂർണ്ണസംഖ്യ ret = -1;

    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        നിർത്തുക;
    അവസ്ഥ TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        എങ്കിൽ (ret)
            പോകുക err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        നിർത്തുക;
    }

    എങ്കിൽ (ret)
        പോകുക err_exit;

    തിരികെ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

സ്ഥിതം വരിയിൽ ശൂന്യം *നിവേശം_buf32(ഘടന bregs *regs)
{
    തിരികെ MAKE_FLATPTR(regs->es, regs->di);
}

സ്ഥിതം വരിയിൽ ശൂന്യം *output_buf32(ഘടന bregs *regs)
{
    തിരികെ MAKE_FLATPTR(regs->ds, regs->si);
}

സ്ഥിതം u32
hash_log_extend(ഘടന pcpes *pcpes, സ്ഥിരം ശൂന്യം *hashdata, u32 hashdata_length
                , ശൂന്യം *event, പൂർണ്ണസംഖ്യ extend)
{
    എങ്കിൽ (pcpes->pcrindex >= 24)
        തിരികെ TCG_INVALID_നിവേശം_PARA;
    എങ്കിൽ (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    ഘടന tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    പൂർണ്ണസംഖ്യ digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    എങ്കിൽ (digest_len < 0)
        തിരികെ TCG_GENERAL_ERROR;
    എങ്കിൽ (extend) {
        പൂർണ്ണസംഖ്യ ret = tpm_extend(&le, digest_len);
        എങ്കിൽ (ret)
            തിരികെ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    പൂർണ്ണസംഖ്യ ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    എങ്കിൽ (ret)
        തിരികെ TCG_PC_LOGOVERFLOW;
    തിരികെ 0;
}

സ്ഥിതം u32
hash_log_extend_event_int(സ്ഥിരം ഘടന hleei_short *hleei_s,
                          ഘടന hleeo *hleeo)
{
    u32 rc = 0;
    ഘടന hleei_long *hleei_l = (ഘടന hleei_long *)hleei_s;
    സ്ഥിരം ശൂന്യം *logdataptr;
    u32 logdatalen;
    ഘടന pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    തിരഞ്ഞെടുപ്പ് (hleei_s->ipblength) {
    അവസ്ഥ വലുപ്പം(ഘടന hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    നിർത്തുക;

    അവസ്ഥ വലുപ്പം(ഘടന hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
        /* bad input block */
        rc = TCG_INVALID_നിവേശം_PARA;
        പോകുക err_exit;
    }

    pcpes = (ഘടന pcpes *)logdataptr;

    എങ്കിൽ (pcpes->pcrindex != pcrindex
        || logdatalen != വലുപ്പം(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_നിവേശം_PARA;
        പോകുക err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    എങ്കിൽ (rc)
        പോകുക err_exit;

    hleeo->opblength = വലുപ്പം(ഘടന hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_എണ്ണം;
    memcpy(hleeo->digest, pcpes->digest, വലുപ്പം(hleeo->digest));

err_exit:
    എങ്കിൽ (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    തിരികെ rc;
}

സ്ഥിതം u32
pass_through_to_tpm_int(ഘടന pttti *pttti, ഘടന pttto *pttto)
{
    u32 rc = 0;
    ഘടന tpm_req_header *trh = (ശൂന്യം*)pttti->tpmopin;

    എങ്കിൽ (pttti->ipblength < വലുപ്പം(ഘടന pttti) + വലുപ്പം(*trh)
        || pttti->ipblength != വലുപ്പം(ഘടന pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < വലുപ്പം(ഘടന pttto)) {
        rc = TCG_INVALID_നിവേശം_PARA;
        പോകുക err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        എങ്കിൽ (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_നിവേശം_PARA;
            പോകുക err_exit;
        }
        നിർത്തുക;
    അവസ്ഥ TPM_VERSION_2:
        എങ്കിൽ (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_നിവേശം_PARA;
            പോകുക err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(ഘടന pttto, tpmopout);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    എങ്കിൽ (ret) {
        rc = TCG_FATAL_COM_ERROR;
        പോകുക err_exit;
    }

    pttto->opblength = offsetof(ഘടന pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    എങ്കിൽ (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    തിരികെ rc;
}

സ്ഥിതം u32
shutdown_preboot_interface(ശൂന്യം)
{
    TPM_interface_shutdown = 1;
    തിരികെ 0;
}

സ്ഥിതം u32
hash_log_event_int(സ്ഥിരം ഘടന hlei *hlei, ഘടന hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    ഘടന pcpes *pcpes;

    size = hlei->ipblength;
    എങ്കിൽ (size != വലുപ്പം(*hlei)) {
        rc = TCG_INVALID_നിവേശം_PARA;
        പോകുക err_exit;
    }

    pcpes = (ഘടന pcpes *)hlei->logdataptr;

    എങ്കിൽ (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != വലുപ്പം(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_നിവേശം_PARA;
        പോകുക err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    എങ്കിൽ (rc)
        പോകുക err_exit;

    /* updating the log was fine */
    hleo->opblength = വലുപ്പം(ഘടന hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_എണ്ണം;

err_exit:
    എങ്കിൽ (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    തിരികെ rc;
}

സ്ഥിതം u32
hash_all_int(സ്ഥിരം ഘടന hai *hai, u8 *hash)
{
    എങ്കിൽ (hai->ipblength != വലുപ്പം(ഘടന hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        തിരികെ TCG_INVALID_നിവേശം_PARA;

    sha1((സ്ഥിരം u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    തിരികെ 0;
}

സ്ഥിതം u32
tss_int(ഘടന ti *ti, ഘടന to *to)
{
    to->opblength = വലുപ്പം(ഘടന to);
    to->reserved  = 0;

    തിരികെ TCG_PC_UNSUPPORTED;
}

സ്ഥിതം u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    ഘടന pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = വലുപ്പം(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    എങ്കിൽ (rc)
        തിരികെ rc;

    *edx_ptr = tpm_state.entry_എണ്ണം;
    തിരികെ 0;
}

ശൂന്യം VISIBLE32FLAT
tpm_interrupt_handler32(ഘടന bregs *regs)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ;

    set_cf(regs, 0);

    എങ്കിൽ (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        തിരികെ;
    }

    തിരഞ്ഞെടുപ്പ് ((എണ്ണൽ irq_ids)regs->al) {
    അവസ്ഥ TCG_StatusCheck:
        എങ്കിൽ (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } അല്ലെങ്കിൽ {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        നിർത്തുക;

    അവസ്ഥ TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (ഘടന hleei_short *)നിവേശം_buf32(regs),
                  (ഘടന hleeo *)output_buf32(regs));
        നിർത്തുക;

    അവസ്ഥ TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((ഘടന pttti *)നിവേശം_buf32(regs),
                                    (ഘടന pttto *)output_buf32(regs));
        നിർത്തുക;

    അവസ്ഥ TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        നിർത്തുക;

    അവസ്ഥ TCG_HashLogEvent:
        regs->eax = hash_log_event_int((ഘടന hlei*)നിവേശം_buf32(regs),
                                       (ഘടന hleo*)output_buf32(regs));
        നിർത്തുക;

    അവസ്ഥ TCG_HashAll:
        regs->eax =
            hash_all_int((ഘടന hai*)നിവേശം_buf32(regs),
                          (u8 *)output_buf32(regs));
        നിർത്തുക;

    അവസ്ഥ TCG_TSS:
        regs->eax = tss_int((ഘടന ti*)നിവേശം_buf32(regs),
                            (ഘടന to*)output_buf32(regs));
        നിർത്തുക;

    അവസ്ഥ TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)നിവേശം_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
        set_cf(regs, 1);
    }

    തിരികെ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

തരനിർവചനം u8 tpm_ppi_code;

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_read_has_owner(പൂർണ്ണസംഖ്യ *has_owner)
{
    ഘടന tpm_res_getcap_ownerauth oauth;
    പൂർണ്ണസംഖ്യ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, വലുപ്പം(oauth));
    എങ്കിൽ (ret)
        തിരികെ -1;

    *has_owner = oauth.flag;

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_enable_tpm(പൂർണ്ണസംഖ്യ enable, പൂർണ്ണസംഖ്യ verbose)
{
    ഘടന tpm_permanent_flags pf;
    പൂർണ്ണസംഖ്യ ret = tpm12_read_permanent_flags((അക്ഷരം *)&pf, വലുപ്പം(pf));
    എങ്കിൽ (ret)
        തിരികെ -1;

    എങ്കിൽ (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        തിരികെ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret) {
        എങ്കിൽ (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        അല്ലെങ്കിൽ
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_activate_tpm(പൂർണ്ണസംഖ്യ activate, പൂർണ്ണസംഖ്യ allow_reset, പൂർണ്ണസംഖ്യ verbose)
{
    ഘടന tpm_permanent_flags pf;
    പൂർണ്ണസംഖ്യ ret = tpm12_read_permanent_flags((അക്ഷരം *)&pf, വലുപ്പം(pf));
    എങ്കിൽ (ret)
        തിരികെ -1;

    എങ്കിൽ (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        തിരികെ 0;

    എങ്കിൽ (pf.flags[PERM_FLAG_IDX_DISABLE])
        തിരികെ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret)
        തിരികെ ret;

    എങ്കിൽ (activate && allow_reset) {
        എങ്കിൽ (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_enable_activate(പൂർണ്ണസംഖ്യ allow_reset, പൂർണ്ണസംഖ്യ verbose)
{
    പൂർണ്ണസംഖ്യ ret = tpm12_enable_tpm(1, verbose);
    എങ്കിൽ (ret)
        തിരികെ ret;

    തിരികെ tpm12_activate_tpm(1, allow_reset, verbose);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_force_clear(പൂർണ്ണസംഖ്യ enable_activate_before, പൂർണ്ണസംഖ്യ enable_activate_after,
                  പൂർണ്ണസംഖ്യ verbose)
{
    പൂർണ്ണസംഖ്യ has_owner;
    പൂർണ്ണസംഖ്യ ret = tpm12_read_has_owner(&has_owner);
    എങ്കിൽ (ret)
        തിരികെ -1;
    എങ്കിൽ (!has_owner) {
        എങ്കിൽ (verbose)
            printf("TPM does not have an owner.\n");
        തിരികെ 0;
    }

    എങ്കിൽ (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        എങ്കിൽ (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            തിരികെ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret)
        തിരികെ ret;

    എങ്കിൽ (!enable_activate_after) {
        എങ്കിൽ (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        തിരികെ 0;
    }

    തിരികെ tpm12_enable_activate(1, verbose);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_set_owner_install(പൂർണ്ണസംഖ്യ allow, പൂർണ്ണസംഖ്യ verbose)
{
    പൂർണ്ണസംഖ്യ has_owner;
    പൂർണ്ണസംഖ്യ ret = tpm12_read_has_owner(&has_owner);
    എങ്കിൽ (ret)
        തിരികെ -1;
    എങ്കിൽ (has_owner) {
        എങ്കിൽ (verbose)
            printf("Must first remove owner.\n");
        തിരികെ 0;
    }

    ഘടന tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((അക്ഷരം *)&pf, വലുപ്പം(pf));
    എങ്കിൽ (ret)
        തിരികെ -1;

    എങ്കിൽ (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        എങ്കിൽ (verbose)
            printf("TPM must first be enable.\n");
        തിരികെ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret)
        തിരികെ ret;

    എങ്കിൽ (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_process_cfg(tpm_ppi_code സന്ദേശം_Code, പൂർണ്ണസംഖ്യ verbose)
{
    പൂർണ്ണസംഖ്യ ret = 0;

    തിരഞ്ഞെടുപ്പ് (സന്ദേശം_Code) {
        അവസ്ഥ TPM_PPI_OP_NOOP: /* no-op */
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            നിർത്തുക;

        സ്ഥിരസ്ഥിതി:
            നിർത്തുക;
    }

    എങ്കിൽ (ret)
        printf("Op %d: An error occurred: 0x%x\n", സന്ദേശം_Code, ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_clearcontrol(u8 disable, പൂർണ്ണസംഖ്യ verbose)
{
    ഘടന tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(വലുപ്പം(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(വലുപ്പം(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    ഘടന tpm_rsp_header rsp;
    u32 resp_length = വലുപ്പം(rsp);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_clear(ശൂന്യം)
{
    ഘടന tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(വലുപ്പം(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(വലുപ്പം(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ഘടന tpm_rsp_header rsp;
    u32 resp_length = വലുപ്പം(rsp);
    പൂർണ്ണസംഖ്യ ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    എങ്കിൽ (ret || resp_length != വലുപ്പം(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_process_cfg(tpm_ppi_code സന്ദേശം_Code, പൂർണ്ണസംഖ്യ verbose)
{
    പൂർണ്ണസംഖ്യ ret = 0;

    തിരഞ്ഞെടുപ്പ് (സന്ദേശം_Code) {
        അവസ്ഥ TPM_PPI_OP_NOOP: /* no-op */
            നിർത്തുക;

        അവസ്ഥ TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            എങ്കിൽ (!ret)
                 ret = tpm20_clear();
            നിർത്തുക;
    }

    എങ്കിൽ (ret)
        printf("Op %d: An error occurred: 0x%x\n", സന്ദേശം_Code, ret);

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm12_get_tpm_state(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ state = 0;
    ഘടന tpm_permanent_flags pf;
    പൂർണ്ണസംഖ്യ has_owner;

    എങ്കിൽ (tpm12_read_permanent_flags((അക്ഷരം *)&pf, വലുപ്പം(pf)) ||
        tpm12_read_has_owner(&has_owner))
        തിരികെ ~0;

    എങ്കിൽ (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    എങ്കിൽ (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    എങ്കിൽ (has_owner) {
        state |= TPM_STATE_OWNED;
    } അല്ലെങ്കിൽ {
        എങ്കിൽ (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    തിരികെ state;
}

സ്ഥിതം ശൂന്യം
tpm12_show_tpm_menu(പൂർണ്ണസംഖ്യ state, പൂർണ്ണസംഖ്യ next_scancodes[7])
{
    പൂർണ്ണസംഖ്യ i = 0;

    printf("\nThe current state of the TPM is:\n");

    എങ്കിൽ (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    അല്ലെങ്കിൽ
        printf("  Disabled");

    എങ്കിൽ (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    അല്ലെങ്കിൽ
        printf(" and deactivated\n");

    എങ്കിൽ (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    അല്ലെങ്കിൽ {
        printf("  Ownership has not been taken\n");
        എങ്കിൽ (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        അല്ലെങ്കിൽ
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    എങ്കിൽ ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    എങ്കിൽ (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        എങ്കിൽ (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            എങ്കിൽ (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } അല്ലെങ്കിൽ {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } അല്ലെങ്കിൽ {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } അല്ലെങ്കിൽ {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    എങ്കിൽ (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

സ്ഥിതം ശൂന്യം
tpm12_menu(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ scancode, next_scancodes[7];
    tpm_ppi_code സന്ദേശം_Code;
    പൂർണ്ണസംഖ്യ state = 0, i;
    പൂർണ്ണസംഖ്യ waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    വേണ്ടി (;;) {
        എങ്കിൽ ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } അല്ലെങ്കിൽ {
            printf("TPM is not working correctly.\n");
            തിരികെ;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        സന്ദേശം_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        ആകുമ്പോൾ (waitkey) {
            ആകുമ്പോൾ ((scancode = get_keystroke(1000)) == ~0)
                ;

            തിരഞ്ഞെടുപ്പ് (scancode) {
            അവസ്ഥ 1:
                // ESC
                reset();
                നിർത്തുക;
            അവസ്ഥ 18: /* e. enable */
                സന്ദേശം_Code = TPM_PPI_OP_ENABLE;
                നിർത്തുക;
            അവസ്ഥ 32: /* d. disable */
                സന്ദേശം_Code = TPM_PPI_OP_DISABLE;
                നിർത്തുക;
            അവസ്ഥ 30: /* a. activate */
                സന്ദേശം_Code = TPM_PPI_OP_ACTIVATE;
                നിർത്തുക;
            അവസ്ഥ 47: /* v. deactivate */
                സന്ദേശം_Code = TPM_PPI_OP_DEACTIVATE;
                നിർത്തുക;
            അവസ്ഥ 46: /* c. clear owner */
                സന്ദേശം_Code = TPM_PPI_OP_CLEAR;
                നിർത്തുക;
            അവസ്ഥ 25: /* p. prevent ownerinstall */
                സന്ദേശം_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                നിർത്തുക;
            അവസ്ഥ 31: /* s. allow ownerinstall */
                സന്ദേശം_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
                തുടരുക;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            വേണ്ടി (i = 0; i < വലുപ്പം(next_scancodes); i++) {
                എങ്കിൽ (next_scancodes[i] == 0)
                    നിർത്തുക;

                എങ്കിൽ (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(സന്ദേശം_Code, 1);
                    waitkey = 0;
                    നിർത്തുക;
                }
            }
        }
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
tpm20_menu_change_active_pcrbanks(ശൂന്യം)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    ആകുമ്പോൾ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        ആകുമ്പോൾ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            സ്ഥിരം അക്ഷരം *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            എങ്കിൽ (hashname) {
                printf("  %d: %s", i, hashname);
                എങ്കിൽ (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        എങ്കിൽ (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        പൂർണ്ണസംഖ്യ show = 0;
        ആകുമ്പോൾ (!show) {
            പൂർണ്ണസംഖ്യ scancode = get_keystroke(1000);

            തിരഞ്ഞെടുപ്പ് (scancode) {
            അവസ്ഥ ~0:
                തുടരുക;
            അവസ്ഥ 1: /* ESC */
                printf("\n");
                തിരികെ -1;
            അവസ്ഥ 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                എങ്കിൽ (flagnum > i)
                    തുടരുക;
                എങ്കിൽ (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                നിർത്തുക;
            അവസ്ഥ 30: /* a */
                എങ്കിൽ (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

സ്ഥിതം ശൂന്യം
tpm20_menu(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ scan_code;
    tpm_ppi_code സന്ദേശം_Code;

    വേണ്ടി (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        സന്ദേശം_Code = TPM_PPI_OP_NOOP;

        ആകുമ്പോൾ ((scan_code = get_keystroke(1000)) == ~0)
            ;

        തിരഞ്ഞെടുപ്പ് (scan_code) {
        അവസ്ഥ 1:
            // ESC
            reset();
            നിർത്തുക;
        അവസ്ഥ 2:
            സന്ദേശം_Code = TPM_PPI_OP_CLEAR;
            നിർത്തുക;
        അവസ്ഥ 3:
            tpm20_menu_change_active_pcrbanks();
            തുടരുക;
        സ്ഥിരസ്ഥിതി:
            തുടരുക;
        }

        tpm20_process_cfg(സന്ദേശം_Code, 0);
    }
}

ശൂന്യം
tpm_menu(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ;

    ആകുമ്പോൾ (get_keystroke(0) >= 0)
        ;
    wait_threads();

    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        tpm12_menu();
        നിർത്തുക;
    അവസ്ഥ TPM_VERSION_2:
        tpm20_menu();
        നിർത്തുക;
    }
}

പൂർണ്ണസംഖ്യ
tpm_can_show_menu(ശൂന്യം)
{
    തിരഞ്ഞെടുപ്പ് (TPM_version) {
    അവസ്ഥ TPM_VERSION_1_2:
        തിരികെ tpm_is_working() && TPM_has_physical_presence;
    അവസ്ഥ TPM_VERSION_2:
        തിരികെ tpm_is_working();
    }
    തിരികെ 0;
}
