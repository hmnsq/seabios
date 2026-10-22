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

#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "byteorder.h" // cpu_to_*
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_TCGBIOS
#ଅନ୍ତର୍ଭାଵ "farptr.h" // MAKE_FLATPTR
#ଅନ୍ତର୍ଭାଵ "fw/paravirt.h" // runningOnXen
#ଅନ୍ତର୍ଭାଵ "hw/tpm_drivers.h" // tpm_drivers[]
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "sha.h" // sha1, sha256, ...
#ଅନ୍ତର୍ଭାଵ "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#ଅନ୍ତର୍ଭାଵ "std/smbios.h" // struct smbios_21_entry_point
#ଅନ୍ତର୍ଭାଵ "std/tcg.h" // TCG_PC_LOGOVERFLOW
#ଅନ୍ତର୍ଭାଵ "string.h" // checksum
#ଅନ୍ତର୍ଭାଵ "tcgbios.h"// tpm_*, prototypes
#ଅନ୍ତର୍ଭାଵ "util.h" // printf, get_keystroke
#ଅନ୍ତର୍ଭାଵ "stacks.h" // wait_threads, reset
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

ସଂରଚନା {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_ଗଣନା;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    ଯଦି (!log_area_start_address || !log_area_minimum_length)
        ଫେରନ୍ତୁ -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = ଖାଲି;
    tpm_state.entry_ଗଣନା = 0;
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_tcpa_probe(ଶୂନ୍ୟ)
{
    ସଂରଚନା tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    ଯଦି (!tcpa)
        ଫେରନ୍ତୁ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    ଫେରନ୍ତୁ tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_tpm2_probe(ଶୂନ୍ୟ)
{
    ସଂରଚନା tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    ଯଦି (!tpm2)
        ଫେରନ୍ତୁ -1;

    ଯଦି (tpm2->length < 76)
        ଫେରନ୍ତୁ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    ଫେରନ୍ତୁ tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_log_event(ସଂରଚନା tpm_log_header *entry, ପୂର୍ଣ୍ଣସଂଖ୍ୟା digest_len
              , ଅଚଳ ଶୂନ୍ୟ *event, ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    ଯଦି (tpm_state.log_area_next_entry == ଖାଲି)
        ଫେରନ୍ତୁ -1;

    u32 size = (ଆକାର(*entry) + digest_len
                + ଆକାର(ସଂରଚନା tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    ଯଦି (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        ଫେରନ୍ତୁ -1;
    }

    ଶୂନ୍ୟ *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, ଆକାର(*entry) + digest_len);
    ସଂରଚନା tpm_log_trailer *t = dest + ଆକାର(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_ଗଣନା++;

    ଫେରନ୍ତୁ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ସ୍ଥିର TPMVersion TPM_version;
ସ୍ଥିର u32 tpm20_pcr_selection_size;
ସ୍ଥିର ସଂରଚନା tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
ସଂରଚନା tpm_log_entry {
    ସଂରଚନା tpm_log_header hdr;
    u8 pad[ଆକାର(ସଂରଚନା tpm2_digest_ମୂଲ୍ୟଗୁଡ଼ିକ)
           + 8 * ଆକାର(ସଂରଚନା tpm2_digest_ମୂଲ୍ୟ)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ସ୍ଥିର ଅଚଳ ସଂରଚନା hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ଅଚଳ ଅକ୍ଷର *name;
    ଶୂନ୍ୟ (*hashfunc)(ଅଚଳ u8 *data, u32 length, u8 *hash);
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    ପାଇଁ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ଯଦି (hash_parameters[i].hashalg == hashAlg)
            ଫେରନ୍ତୁ hash_parameters[i].hash_buffersize;
    }
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    ପାଇଁ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ଯଦି (hash_parameters[i].hashalg == hashAlg)
            ଫେରନ୍ତୁ hash_parameters[i].hashalg_flag;
    }
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    ପାଇଁ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ଯଦି (hash_parameters[i].hashalg_flag == hashalg_flag)
            ଫେରନ୍ତୁ hash_parameters[i].hashalg;
    }
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    ପାଇଁ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ଯଦି (hash_parameters[i].hashalg_flag == hashalg_flag)
            ଫେରନ୍ତୁ hash_parameters[i].name;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ଶୂନ୍ୟ tpm2_hash_data(u16 hashAlg, ଅଚଳ u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    ପାଇଁ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ଯଦି (hash_parameters[i].hashalg == hashAlg) {
            ଯଦି (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } ଅନ୍ୟଥା {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_write_EfiSpecIdEventStruct(ଶୂନ୍ୟ)
{
    ଯଦି (!tpm20_pcr_selection)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା {
        ସଂରଚନା TCG_EfiSpecIdEventStruct hdr;
        u8 pad[ଆକାର(ସଂରଚନା tpm_log_entry) + ଆକାର(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    ସଂରଚନା tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ଶୂନ୍ୟ *nsel, *end = (ଶୂନ୍ୟ*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 ଗଣନା_2, ଅଙ୍କ_Algs = 0;
    ପାଇଁ (ଗଣନା_2 = 0; ଗଣନା_2 < be32_to_cpu(tpm20_pcr_selection->ଗଣନା_2); ଗଣନା_2++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (ଶୂନ୍ୟ*)sel + ଆକାର(*sel) + sizeOfSelect;
        ଯଦି (nsel > end)
            ଭାଙ୍ଗ;

        ଯଦି (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ଜାରି;
        }

        ପୂର୍ଣ୍ଣସଂଖ୍ୟା hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ଯଦି (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ଫେରନ୍ତୁ -1;
        }

        ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_size = offsetof(ସଂରଚନା TCG_EfiSpecIdEventStruct
                                  , digestSizes[ଗଣନା_2+1]);
        ଯଦି (event_size > ଆକାର(event) - ଆକାର(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            ଫେରନ୍ତୁ -1;
        }

        event.hdr.digestSizes[ଅଙ୍କ_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[ଅଙ୍କ_Algs].digestSize = hsize;
        ଅଙ୍କ_Algs++;

        sel = nsel;
    }

    ଯଦି (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ଫେରନ୍ତୁ -1;
    }

    event.hdr.ସଂଖ୍ୟା_Of_Algorithms = ଅଙ୍କ_Algs;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା event_size = offsetof(ସଂରଚନା TCG_EfiSpecIdEventStruct
                              , digestSizes[ଅଙ୍କ_Algs]);
    u8 *vendorInfoSize = (ଶୂନ୍ୟ*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += ଆକାର(*vendorInfoSize);

    ସଂରଚନା tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    ଫେରନ୍ତୁ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_build_digest(ସଂରଚନା tpm_log_entry *le,
                   ଅଚଳ u8 *hashdata, u32 hashdata_len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bigEndian)
{
    ଯଦି (!tpm20_pcr_selection)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ଶୂନ୍ୟ *nsel, *end = (ଶୂନ୍ୟ*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    ଶୂନ୍ୟ *dest = le->hdr.digest + ଆକାର(ସଂରଚନା tpm2_digest_ମୂଲ୍ୟଗୁଡ଼ିକ);

    u32 ଗଣନା_2, ଅଙ୍କ_Algs = 0;
    ପାଇଁ (ଗଣନା_2 = 0; ଗଣନା_2 < be32_to_cpu(tpm20_pcr_selection->ଗଣନା_2); ଗଣନା_2++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (ଶୂନ୍ୟ*)sel + ଆକାର(*sel) + sizeOfSelect;
        ଯଦି (nsel > end)
            ଭାଙ୍ଗ;

        /* PCR 0-7 unused? -- skip */
        ଯଦି (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ଜାରି;
        }

        ପୂର୍ଣ୍ଣସଂଖ୍ୟା hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ଯଦି (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ଫେରନ୍ତୁ -1;
        }

        /* buffer size sanity check before writing */
        ସଂରଚନା tpm2_digest_ମୂଲ୍ୟ *v = dest;
        ଯଦି (dest + ଆକାର(*v) + hsize > (ଶୂନ୍ୟ*)le + ଆକାର(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            ଫେରନ୍ତୁ -1;
        }

        ଯଦି (bigEndian)
            v->hashAlg = sel->hashAlg;
        ଅନ୍ୟଥା
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += ଆକାର(*v) + hsize;
        sel = nsel;

        ଅଙ୍କ_Algs++;
    }

    ଯଦି (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ଫେରନ୍ତୁ -1;
    }

    ସଂରଚନା tpm2_digest_ମୂଲ୍ୟଗୁଡ଼ିକ *v = (ଶୂନ୍ୟ*)le->hdr.digest;
    ଯଦି (bigEndian)
        v->ଗଣନା_2 = cpu_to_be32(ଅଙ୍କ_Algs);
    ଅନ୍ୟଥା
        v->ଗଣନା_2 = ଅଙ୍କ_Algs;

    ଫେରନ୍ତୁ dest - (ଶୂନ୍ୟ*)le->hdr.digest;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_build_digest(ସଂରଚନା tpm_log_entry *le,
                   ଅଚଳ u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    ଫେରନ୍ତୁ SHA1_BUFSIZE;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_build_digest_direct(ସଂରଚନା tpm_log_entry *le, ଅଚଳ u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    ଫେରନ୍ତୁ SHA1_BUFSIZE;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_build_digest(ସଂରଚନା tpm_log_entry *le, ଅଚଳ u8 *hashdata, u32 hashdata_len
                 , ପୂର୍ଣ୍ଣସଂଖ୍ୟା bigEndian)
{
    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        ଫେରନ୍ତୁ tpm12_build_digest(le, hashdata, hashdata_len);
    ସ୍ଥିତି TPM_VERSION_2:
        ଫେରନ୍ତୁ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    ଫେରନ୍ତୁ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_simple_cmd(u8 locty, u32 ordinal
               , ପୂର୍ଣ୍ଣସଂଖ୍ୟା param_size, u16 param, ଗଣନା tpmDurationType to_t)
{
    ସଂରଚନା {
        ସଂରଚନା tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(ଆକାର(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        ଭାଙ୍ଗ;
    }

    u8 obuffer[64];
    ସଂରଚନା tpm_rsp_header *trsh = (ଶୂନ୍ୟ*)obuffer;
    u32 obuffer_len = ଆକାର(obuffer);
    memset(obuffer, 0x0, ଆକାର(obuffer));

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_getcapability(u32 capability, u32 property, u32 ଗଣନା_2,
                    ସଂରଚନା tpm_rsp_header *rsp, u32 rsize)
{
    ସଂରଚନା tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(ଆକାର(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(ଗଣନା_2),
    };

    u32 resp_size = rsize;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_get_pcrbanks(ଶୂନ୍ୟ)
{
    u8 buffer[128];
    ସଂରଚନା tpm2_res_getcapability *trg =
      (ସଂରଚନା tpm2_res_getcapability *)&buffer;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  ଆକାର(buffer));
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    ଯଦି (resplen <= offsetof(ସଂରଚନା tpm2_res_getcapability, data))
        ଫେରନ୍ତୁ -1;

    u32 size = resplen - offsetof(ସଂରଚନା tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    ଯଦି (size < offsetof(ସଂରଚନା tpml_pcr_selection, selections) +
               offsetof(ସଂରଚନା tpms_pcr_selection, pcrSelect))
        ଫେରନ୍ତୁ -1;

    tpm20_pcr_selection = malloc_ଉପର(size);
    ଯଦି (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } ଅନ୍ୟଥା {
        warn_noalloc();
        ret = -1;
    }

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    ଯଦି (!tpm20_pcr_selection)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ଶୂନ୍ୟ *end = (ଶୂନ୍ୟ*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    ଯେତେବେଳେ (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        ଶୂନ୍ୟ *nsel = (ଶୂନ୍ୟ*)sel + ଆକାର(*sel) + sizeOfSelect;
        ଯଦି (nsel > end)
            ଫେରନ୍ତୁ 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        ପାଇଁ (i = 0; i < sizeOfSelect; i++) {
            ଯଦି (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                ଭାଙ୍ଗ;
            }
        }

        sel = nsel;
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_set_pcrbanks(u32 active_banks)
{
    ସଂରଚନା tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ଆକାର(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ସଂରଚନା tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    ଯେତେବେଳେ (hashalg_flag) {
        ଯଦି ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            ଯଦି (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                ଯଦି (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.ଗଣନା_2 = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * ଆକାର(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(ସଂରଚନା tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * ଆକାର(tps[0]));

    ସଂରଚନା tpm_rsp_header rsp;
    u32 resp_length = ଆକାର(rsp);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tpm20_activate_pcrbanks(u32 active_banks)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm20_set_pcrbanks(active_banks);
    ଯଦି (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    ଯଦି (!ret)
        reset();
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_get_capability(u32 cap, u32 subcap, ସଂରଚନା tpm_rsp_header *rsp, u32 rsize)
{
    ସଂରଚନା tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(ଆକାର(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(ଆକାର(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_read_permanent_flags(ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା buf_len)
{
    memset(buf, 0, buf_len);

    ସଂରଚନା tpm_res_getcap_perm_flags pf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, ଆକାର(pf));
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_determine_timeouts(ଶୂନ୍ୟ)
{
    ସଂରଚନା tpm_res_getcap_timeouts timeouts;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, ଆକାର(timeouts));
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ସଂରଚନା tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, ଆକାର(durations));
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    ପାଇଁ (i = 0; i < 4; i++)
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

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ
tpm20_set_timeouts(ଶୂନ୍ୟ)
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_extend(ସଂରଚନା tpm_log_entry *le, ପୂର୍ଣ୍ଣସଂଖ୍ୟା digest_len)
{
    ସଂରଚନା tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(ଆକାର(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, ଆକାର(tre.digest));

    ସଂରଚନା tpm_rsp_extend rsp;
    u32 resp_length = ଆକାର(rsp);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.hdr.errcode)
        ଫେରନ୍ତୁ -1;

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tpm20_extend(ସଂରଚନା tpm_log_entry *le, ପୂର୍ଣ୍ଣସଂଖ୍ୟା digest_len)
{
    ସଂରଚନା tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(ଆକାର(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[ଆକାର(tmp_tre) + ଆକାର(le->pad)];
    ସଂରଚନା tpm2_req_extend *tre = (ସଂରଚନା tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, ଆକାର(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(ଆକାର(tmp_tre) + digest_len);

    ସଂରଚନା tpm_rsp_header rsp;
    u32 resp_length = ଆକାର(rsp);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.errcode)
        ଫେରନ୍ତୁ -1;

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_extend(ସଂରଚନା tpm_log_entry *le, ପୂର୍ଣ୍ଣସଂଖ୍ୟା digest_len)
{
    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        ଫେରନ୍ତୁ tpm12_extend(le, digest_len);
    ସ୍ଥିତି TPM_VERSION_2:
        ଫେରନ୍ତୁ tpm20_extend(le, digest_len);
    }
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_stirrandom(ଶୂନ୍ୟ)
{
    ସଂରଚନା tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(ଆକାର(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(ଆକାର(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    ସଂରଚନା tpm_rsp_header rsp;
    u32 resp_length = ଆକାର(rsp);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    ସଂରଚନା tpm2_res_getrandom rsp;

    ଯଦି (buf_len > ଆକାର(rsp.rnd.buffer))
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(ଆକାର(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = ଆକାର(rsp);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.hdr.errcode)
        ret = -1;
    ଅନ୍ୟଥା
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    ସଂରଚନା tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(ଆକାର(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ଆକାର(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    ସଂରଚନା tpm_rsp_header rsp;
    u32 resp_length = ଆକାର(rsp);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_hierarchychangeauth(u8 auth[20])
{
    ସଂରଚନା tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(ଆକାର(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ଆକାର(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(ଆକାର(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, ଆକାର(trhca.newAuth.buffer));

    ସଂରଚନା tpm_rsp_header rsp;
    u32 resp_length = ଆକାର(rsp);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    ଫେରନ୍ତୁ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା TPM_has_physical_presence;
u8 TPM_working VARLOW;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_is_working(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ CONFIG_TCGBIOS && TPM_working;
}

ସ୍ଥିର ଶୂନ୍ୟ
tpm_set_failure(ଶୂନ୍ୟ)
{
    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        ଭାଙ୍ଗ;
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
ସ୍ଥିର ଶୂନ୍ୟ
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           ଅଚଳ ଅକ୍ଷର *event, u32 event_length,
                           ଅଚଳ u8 *hashdata, u32 hashdata_length)
{
    ଯଦି (!tpm_is_working())
        ଫେରନ୍ତୁ;

    ସଂରଚନା tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    ଯଦି (digest_len < 0)
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm_extend(&le, digest_len);
    ଯଦି (ret) {
        tpm_set_failure();
        ଫେରନ୍ତୁ;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
ସ୍ଥିର ଶୂନ୍ୟ
tpm_add_action(u32 pcrIndex, ଅଚଳ ଅକ୍ଷର *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ସ୍ଥିର ଶୂନ୍ୟ
tpm_add_event_separators(ଶୂନ୍ୟ)
{
    ସ୍ଥିର ଅଚଳ u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    ପାଇଁ (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (ଅଚଳ ଅକ୍ଷର *)evt_separator,
                                   ଆକାର(evt_separator),
                                   evt_separator,
                                   ଆକାର(evt_separator));
}

ସ୍ଥିର ଶୂନ୍ୟ
tpm_smbios_measure(ଶୂନ୍ୟ)
{
    ସଂରଚନା pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    ଶୂନ୍ୟ *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    ଯଦି (!smbios_tables)
        ଫେରନ୍ତୁ;

    sha1((ଅଚଳ u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (ଅଚଳ ଅକ୍ଷର *)&pcctes, ଆକାର(pcctes),
                               (u8 *)&pcctes, ଆକାର(pcctes));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_assert_physical_presence(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    ଯଦି (!ret)
        ଫେରନ୍ତୁ 0;

    ସଂରଚନା tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ଅକ୍ଷର *)&pf, ଆକାର(pf));
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;

    /* check if hardware physical presence is supported */
    ଯଦି (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        ଫେରନ୍ତୁ 0;
    }

    ଯଦି (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        ଫେରନ୍ତୁ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_startup(ଶୂନ୍ୟ)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    ଯଦି (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    ଯଦି (ret)
        ଯାଅ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    ଯଦି (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    ଯଦି (ret)
        ଯାଅ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    ଯଦି (ret)
        ଯାଅ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret && ret != TPM_BAD_LOCALITY)
        ଯାଅ err_exit;

    ଫେରନ୍ତୁ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_startup(ଶୂନ୍ୟ)
{
    tpm20_set_timeouts();

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    ଯଦି (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    ଯଦି (ret)
        ଯାଅ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    ଯଦି (ret)
        ଯାଅ err_exit;

    ret = tpm20_get_pcrbanks();
    ଯଦି (ret)
        ଯାଅ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    ଯଦି (ret)
        ଯାଅ err_exit;

    ଫେରନ୍ତୁ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_startup(ଶୂନ୍ୟ)
{
    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        ଫେରନ୍ତୁ tpm12_startup();
    ସ୍ଥିତି TPM_VERSION_2:
        ଫେରନ୍ତୁ tpm20_startup();
    }
    ଫେରନ୍ତୁ -1;
}

ଶୂନ୍ୟ
tpm_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm_tpm2_probe();
    ଯଦି (ret) {
        ret = tpm_tcpa_probe();
        ଯଦି (ret)
            ଫେରନ୍ତୁ;
    }

    TPM_version = tpmhw_probe();
    ଯଦି (TPM_version == TPM_VERSION_NONE)
        ଫେରନ୍ତୁ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    ଯଦି (runningOnXen())
        ଫେରନ୍ତୁ;

    ret = tpm_startup();
    ଯଦି (ret)
        ଫେରନ୍ତୁ;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

ସ୍ଥିର ଶୂନ୍ୟ
tpm20_prepboot(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm20_stirrandom();
    ଯଦି (ret)
         ଯାଅ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], ଆକାର(auth));
    ଯଦି (ret)
        ଯାଅ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    ଯଦି (ret)
        ଯାଅ err_exit;

    ଫେରନ୍ତୁ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

ଶୂନ୍ୟ
tpm_prepboot(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ;

    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        ଯଦି (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି TPM_VERSION_2:
        tpm20_prepboot();
        ଭାଙ୍ଗ;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
ଶୂନ୍ୟ
tpm_option_rom(ଅଚଳ ଶୂନ୍ୟ *addr, u32 len)
{
    ଯଦି (!tpm_is_working())
        ଫେରନ୍ତୁ;

    ସଂରଚନା pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = ଆକାର(u16) + ଆକାର(u16) + SHA1_BUFSIZE,
    };
    sha1((ଅଚଳ u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (ଅଚଳ ଅକ୍ଷର *)&pcctes, ଆକାର(pcctes),
                               (u8 *)&pcctes, ଆକାର(pcctes));
}

ଶୂନ୍ୟ
tpm_add_bcv(u32 bootdrv, ଅଚଳ u8 *addr, u32 length)
{
    ଯଦି (!tpm_is_working())
        ଫେରନ୍ତୁ;

    ଯଦି (length < 0x200)
        ଫେରନ୍ତୁ;

    ଅଚଳ ଅକ୍ଷର *string = "Booting BCV device 00h (Floppy)";
    ଯଦି (bootdrv == 0x80)
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

ଶୂନ୍ୟ
tpm_add_cdrom(u32 bootdrv, ଅଚଳ u8 *addr, u32 length)
{
    ଯଦି (!tpm_is_working())
        ଫେରନ୍ତୁ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ଅଚଳ ଅକ୍ଷର *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

ଶୂନ୍ୟ
tpm_add_cdrom_catalog(ଅଚଳ u8 *addr, u32 length)
{
    ଯଦି (!tpm_is_working())
        ଫେରନ୍ତୁ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ଅଚଳ ଅକ୍ଷର *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

ଶୂନ୍ୟ
tpm_s3_resume(ଶୂନ୍ୟ)
{
    ଯଦି (!tpm_is_working())
        ଫେରନ୍ତୁ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -1;

    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        ଯଦି (ret)
            ଯାଅ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        ଭାଙ୍ଗ;
    }

    ଯଦି (ret)
        ଯାଅ err_exit;

    ଫେରନ୍ତୁ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *ନିବେଶ_buf32(ସଂରଚନା bregs *regs)
{
    ଫେରନ୍ତୁ MAKE_FLATPTR(regs->es, regs->di);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *output_buf32(ସଂରଚନା bregs *regs)
{
    ଫେରନ୍ତୁ MAKE_FLATPTR(regs->ds, regs->si);
}

ସ୍ଥିର u32
hash_log_extend(ସଂରଚନା pcpes *pcpes, ଅଚଳ ଶୂନ୍ୟ *hashdata, u32 hashdata_length
                , ଶୂନ୍ୟ *event, ପୂର୍ଣ୍ଣସଂଖ୍ୟା extend)
{
    ଯଦି (pcpes->pcrindex >= 24)
        ଫେରନ୍ତୁ TCG_INVALID_ନିବେଶ_PARA;
    ଯଦି (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    ସଂରଚନା tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    ଯଦି (digest_len < 0)
        ଫେରନ୍ତୁ TCG_GENERAL_ERROR;
    ଯଦି (extend) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm_extend(&le, digest_len);
        ଯଦି (ret)
            ଫେରନ୍ତୁ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    ଯଦି (ret)
        ଫେରନ୍ତୁ TCG_PC_LOGOVERFLOW;
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32
hash_log_extend_event_int(ଅଚଳ ସଂରଚନା hleei_short *hleei_s,
                          ସଂରଚନା hleeo *hleeo)
{
    u32 rc = 0;
    ସଂରଚନା hleei_long *hleei_l = (ସଂରଚନା hleei_long *)hleei_s;
    ଅଚଳ ଶୂନ୍ୟ *logdataptr;
    u32 logdatalen;
    ସଂରଚନା pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    ବାଛ (hleei_s->ipblength) {
    ସ୍ଥିତି ଆକାର(ସଂରଚନା hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    ଭାଙ୍ଗ;

    ସ୍ଥିତି ଆକାର(ସଂରଚନା hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        /* bad input block */
        rc = TCG_INVALID_ନିବେଶ_PARA;
        ଯାଅ err_exit;
    }

    pcpes = (ସଂରଚନା pcpes *)logdataptr;

    ଯଦି (pcpes->pcrindex != pcrindex
        || logdatalen != ଆକାର(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ନିବେଶ_PARA;
        ଯାଅ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    ଯଦି (rc)
        ଯାଅ err_exit;

    hleeo->opblength = ଆକାର(ସଂରଚନା hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_ଗଣନା;
    memcpy(hleeo->digest, pcpes->digest, ଆକାର(hleeo->digest));

err_exit:
    ଯଦି (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32
pass_through_to_tpm_int(ସଂରଚନା pttti *pttti, ସଂରଚନା pttto *pttto)
{
    u32 rc = 0;
    ସଂରଚନା tpm_req_header *trh = (ଶୂନ୍ୟ*)pttti->tpmopin;

    ଯଦି (pttti->ipblength < ଆକାର(ସଂରଚନା pttti) + ଆକାର(*trh)
        || pttti->ipblength != ଆକାର(ସଂରଚନା pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < ଆକାର(ସଂରଚନା pttto)) {
        rc = TCG_INVALID_ନିବେଶ_PARA;
        ଯାଅ err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        ଯଦି (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_ନିବେଶ_PARA;
            ଯାଅ err_exit;
        }
        ଭାଙ୍ଗ;
    ସ୍ଥିତି TPM_VERSION_2:
        ଯଦି (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_ନିବେଶ_PARA;
            ଯାଅ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(ସଂରଚନା pttto, tpmopout);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    ଯଦି (ret) {
        rc = TCG_FATAL_COM_ERROR;
        ଯାଅ err_exit;
    }

    pttto->opblength = offsetof(ସଂରଚନା pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    ଯଦି (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32
shutdown_preboot_interface(ଶୂନ୍ୟ)
{
    TPM_interface_shutdown = 1;
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32
hash_log_event_int(ଅଚଳ ସଂରଚନା hlei *hlei, ସଂରଚନା hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    ସଂରଚନା pcpes *pcpes;

    size = hlei->ipblength;
    ଯଦି (size != ଆକାର(*hlei)) {
        rc = TCG_INVALID_ନିବେଶ_PARA;
        ଯାଅ err_exit;
    }

    pcpes = (ସଂରଚନା pcpes *)hlei->logdataptr;

    ଯଦି (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != ଆକାର(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ନିବେଶ_PARA;
        ଯାଅ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    ଯଦି (rc)
        ଯାଅ err_exit;

    /* updating the log was fine */
    hleo->opblength = ଆକାର(ସଂରଚନା hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_ଗଣନା;

err_exit:
    ଯଦି (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32
hash_all_int(ଅଚଳ ସଂରଚନା hai *hai, u8 *hash)
{
    ଯଦି (hai->ipblength != ଆକାର(ସଂରଚନା hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        ଫେରନ୍ତୁ TCG_INVALID_ନିବେଶ_PARA;

    sha1((ଅଚଳ u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32
tss_int(ସଂରଚନା ti *ti, ସଂରଚନା to *to)
{
    to->opblength = ଆକାର(ସଂରଚନା to);
    to->reserved  = 0;

    ଫେରନ୍ତୁ TCG_PC_UNSUPPORTED;
}

ସ୍ଥିର u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    ସଂରଚନା pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = ଆକାର(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    ଯଦି (rc)
        ଫେରନ୍ତୁ rc;

    *edx_ptr = tpm_state.entry_ଗଣନା;
    ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ VISIBLE32FLAT
tpm_interrupt_handler32(ସଂରଚନା bregs *regs)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ;

    set_cf(regs, 0);

    ଯଦି (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        ଫେରନ୍ତୁ;
    }

    ବାଛ ((ଗଣନା irq_ids)regs->al) {
    ସ୍ଥିତି TCG_StatusCheck:
        ଯଦି (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } ଅନ୍ୟଥା {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        ଭାଙ୍ଗ;

    ସ୍ଥିତି TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (ସଂରଚନା hleei_short *)ନିବେଶ_buf32(regs),
                  (ସଂରଚନା hleeo *)output_buf32(regs));
        ଭାଙ୍ଗ;

    ସ୍ଥିତି TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((ସଂରଚନା pttti *)ନିବେଶ_buf32(regs),
                                    (ସଂରଚନା pttto *)output_buf32(regs));
        ଭାଙ୍ଗ;

    ସ୍ଥିତି TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        ଭାଙ୍ଗ;

    ସ୍ଥିତି TCG_HashLogEvent:
        regs->eax = hash_log_event_int((ସଂରଚନା hlei*)ନିବେଶ_buf32(regs),
                                       (ସଂରଚନା hleo*)output_buf32(regs));
        ଭାଙ୍ଗ;

    ସ୍ଥିତି TCG_HashAll:
        regs->eax =
            hash_all_int((ସଂରଚନା hai*)ନିବେଶ_buf32(regs),
                          (u8 *)output_buf32(regs));
        ଭାଙ୍ଗ;

    ସ୍ଥିତି TCG_TSS:
        regs->eax = tss_int((ସଂରଚନା ti*)ନିବେଶ_buf32(regs),
                            (ସଂରଚନା to*)output_buf32(regs));
        ଭାଙ୍ଗ;

    ସ୍ଥିତି TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)ନିବେଶ_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        set_cf(regs, 1);
    }

    ଫେରନ୍ତୁ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

ପ୍ରକାରବ୍ୟାଖ୍ୟା u8 tpm_ppi_code;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_read_has_owner(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *has_owner)
{
    ସଂରଚନା tpm_res_getcap_ownerauth oauth;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, ଆକାର(oauth));
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;

    *has_owner = oauth.flag;

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_enable_tpm(ପୂର୍ଣ୍ଣସଂଖ୍ୟା enable, ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ସଂରଚନା tpm_permanent_flags pf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_read_permanent_flags((ଅକ୍ଷର *)&pf, ଆକାର(pf));
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;

    ଯଦି (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        ଫେରନ୍ତୁ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret) {
        ଯଦି (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        ଅନ୍ୟଥା
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_activate_tpm(ପୂର୍ଣ୍ଣସଂଖ୍ୟା activate, ପୂର୍ଣ୍ଣସଂଖ୍ୟା allow_reset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ସଂରଚନା tpm_permanent_flags pf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_read_permanent_flags((ଅକ୍ଷର *)&pf, ଆକାର(pf));
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;

    ଯଦି (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        ଫେରନ୍ତୁ 0;

    ଯଦି (pf.flags[PERM_FLAG_IDX_DISABLE])
        ଫେରନ୍ତୁ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ଯଦି (activate && allow_reset) {
        ଯଦି (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_enable_activate(ପୂର୍ଣ୍ଣସଂଖ୍ୟା allow_reset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_enable_tpm(1, verbose);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ଫେରନ୍ତୁ tpm12_activate_tpm(1, allow_reset, verbose);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_force_clear(ପୂର୍ଣ୍ଣସଂଖ୍ୟା enable_activate_before, ପୂର୍ଣ୍ଣସଂଖ୍ୟା enable_activate_after,
                  ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା has_owner;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_read_has_owner(&has_owner);
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;
    ଯଦି (!has_owner) {
        ଯଦି (verbose)
            printf("TPM does not have an owner.\n");
        ଫେରନ୍ତୁ 0;
    }

    ଯଦି (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        ଯଦି (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            ଫେରନ୍ତୁ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ଯଦି (!enable_activate_after) {
        ଯଦି (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        ଫେରନ୍ତୁ 0;
    }

    ଫେରନ୍ତୁ tpm12_enable_activate(1, verbose);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_set_owner_install(ପୂର୍ଣ୍ଣସଂଖ୍ୟା allow, ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା has_owner;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpm12_read_has_owner(&has_owner);
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;
    ଯଦି (has_owner) {
        ଯଦି (verbose)
            printf("Must first remove owner.\n");
        ଫେରନ୍ତୁ 0;
    }

    ସଂରଚନା tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ଅକ୍ଷର *)&pf, ଆକାର(pf));
    ଯଦି (ret)
        ଫେରନ୍ତୁ -1;

    ଯଦି (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        ଯଦି (verbose)
            printf("TPM must first be enable.\n");
        ଫେରନ୍ତୁ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ଯଦି (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_process_cfg(tpm_ppi_code ସନ୍ଦେଶ_Code, ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

    ବାଛ (ସନ୍ଦେଶ_Code) {
        ସ୍ଥିତି TPM_PPI_OP_NOOP: /* no-op */
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            ଭାଙ୍ଗ;

        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            ଭାଙ୍ଗ;
    }

    ଯଦି (ret)
        printf("Op %d: An error occurred: 0x%x\n", ସନ୍ଦେଶ_Code, ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_clearcontrol(u8 disable, ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ସଂରଚନା tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(ଆକାର(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ଆକାର(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    ସଂରଚନା tpm_rsp_header rsp;
    u32 resp_length = ଆକାର(rsp);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_clear(ଶୂନ୍ୟ)
{
    ସଂରଚନା tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(ଆକାର(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ଆକାର(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ସଂରଚନା tpm_rsp_header rsp;
    u32 resp_length = ଆକାର(rsp);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ଯଦି (ret || resp_length != ଆକାର(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_process_cfg(tpm_ppi_code ସନ୍ଦେଶ_Code, ପୂର୍ଣ୍ଣସଂଖ୍ୟା verbose)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

    ବାଛ (ସନ୍ଦେଶ_Code) {
        ସ୍ଥିତି TPM_PPI_OP_NOOP: /* no-op */
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            ଯଦି (!ret)
                 ret = tpm20_clear();
            ଭାଙ୍ଗ;
    }

    ଯଦି (ret)
        printf("Op %d: An error occurred: 0x%x\n", ସନ୍ଦେଶ_Code, ret);

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm12_get_tpm_state(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା state = 0;
    ସଂରଚନା tpm_permanent_flags pf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା has_owner;

    ଯଦି (tpm12_read_permanent_flags((ଅକ୍ଷର *)&pf, ଆକାର(pf)) ||
        tpm12_read_has_owner(&has_owner))
        ଫେରନ୍ତୁ ~0;

    ଯଦି (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    ଯଦି (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    ଯଦି (has_owner) {
        state |= TPM_STATE_OWNED;
    } ଅନ୍ୟଥା {
        ଯଦି (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    ଫେରନ୍ତୁ state;
}

ସ୍ଥିର ଶୂନ୍ୟ
tpm12_show_tpm_menu(ପୂର୍ଣ୍ଣସଂଖ୍ୟା state, ପୂର୍ଣ୍ଣସଂଖ୍ୟା next_scancodes[7])
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;

    printf("\nThe current state of the TPM is:\n");

    ଯଦି (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    ଅନ୍ୟଥା
        printf("  Disabled");

    ଯଦି (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    ଅନ୍ୟଥା
        printf(" and deactivated\n");

    ଯଦି (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    ଅନ୍ୟଥା {
        printf("  Ownership has not been taken\n");
        ଯଦି (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        ଅନ୍ୟଥା
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    ଯଦି ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    ଯଦି (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        ଯଦି (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            ଯଦି (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } ଅନ୍ୟଥା {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } ଅନ୍ୟଥା {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } ଅନ୍ୟଥା {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    ଯଦି (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

ସ୍ଥିର ଶୂନ୍ୟ
tpm12_menu(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା scancode, next_scancodes[7];
    tpm_ppi_code ସନ୍ଦେଶ_Code;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା state = 0, i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    ପାଇଁ (;;) {
        ଯଦି ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } ଅନ୍ୟଥା {
            printf("TPM is not working correctly.\n");
            ଫେରନ୍ତୁ;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ସନ୍ଦେଶ_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        ଯେତେବେଳେ (waitkey) {
            ଯେତେବେଳେ ((scancode = get_keystroke(1000)) == ~0)
                ;

            ବାଛ (scancode) {
            ସ୍ଥିତି 1:
                // ESC
                reset();
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 18: /* e. enable */
                ସନ୍ଦେଶ_Code = TPM_PPI_OP_ENABLE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 32: /* d. disable */
                ସନ୍ଦେଶ_Code = TPM_PPI_OP_DISABLE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 30: /* a. activate */
                ସନ୍ଦେଶ_Code = TPM_PPI_OP_ACTIVATE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 47: /* v. deactivate */
                ସନ୍ଦେଶ_Code = TPM_PPI_OP_DEACTIVATE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 46: /* c. clear owner */
                ସନ୍ଦେଶ_Code = TPM_PPI_OP_CLEAR;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 25: /* p. prevent ownerinstall */
                ସନ୍ଦେଶ_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 31: /* s. allow ownerinstall */
                ସନ୍ଦେଶ_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
                ଜାରି;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            ପାଇଁ (i = 0; i < ଆକାର(next_scancodes); i++) {
                ଯଦି (next_scancodes[i] == 0)
                    ଭାଙ୍ଗ;

                ଯଦି (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(ସନ୍ଦେଶ_Code, 1);
                    waitkey = 0;
                    ଭାଙ୍ଗ;
                }
            }
        }
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm20_menu_change_active_pcrbanks(ଶୂନ୍ୟ)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    ଯେତେବେଳେ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        ଯେତେବେଳେ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ଅଚଳ ଅକ୍ଷର *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            ଯଦି (hashname) {
                printf("  %d: %s", i, hashname);
                ଯଦି (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        ଯଦି (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା show = 0;
        ଯେତେବେଳେ (!show) {
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା scancode = get_keystroke(1000);

            ବାଛ (scancode) {
            ସ୍ଥିତି ~0:
                ଜାରି;
            ସ୍ଥିତି 1: /* ESC */
                printf("\n");
                ଫେରନ୍ତୁ -1;
            ସ୍ଥିତି 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                ଯଦି (flagnum > i)
                    ଜାରି;
                ଯଦି (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 30: /* a */
                ଯଦି (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
tpm20_menu(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା scan_code;
    tpm_ppi_code ସନ୍ଦେଶ_Code;

    ପାଇଁ (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ସନ୍ଦେଶ_Code = TPM_PPI_OP_NOOP;

        ଯେତେବେଳେ ((scan_code = get_keystroke(1000)) == ~0)
            ;

        ବାଛ (scan_code) {
        ସ୍ଥିତି 1:
            // ESC
            reset();
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 2:
            ସନ୍ଦେଶ_Code = TPM_PPI_OP_CLEAR;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 3:
            tpm20_menu_change_active_pcrbanks();
            ଜାରି;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            ଜାରି;
        }

        tpm20_process_cfg(ସନ୍ଦେଶ_Code, 0);
    }
}

ଶୂନ୍ୟ
tpm_menu(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ;

    ଯେତେବେଳେ (get_keystroke(0) >= 0)
        ;
    wait_threads();

    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        tpm12_menu();
        ଭାଙ୍ଗ;
    ସ୍ଥିତି TPM_VERSION_2:
        tpm20_menu();
        ଭାଙ୍ଗ;
    }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpm_can_show_menu(ଶୂନ୍ୟ)
{
    ବାଛ (TPM_version) {
    ସ୍ଥିତି TPM_VERSION_1_2:
        ଫେରନ୍ତୁ tpm_is_working() && TPM_has_physical_presence;
    ସ୍ଥିତି TPM_VERSION_2:
        ଫେରନ୍ତୁ tpm_is_working();
    }
    ଫେରନ୍ତୁ 0;
}
