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

#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "byteorder.h" // cpu_to_*
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_TCGBIOS
#ಅನ್ತರ್ಭಾವ "farptr.h" // MAKE_FLATPTR
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // runningOnXen
#ಅನ್ತರ್ಭಾವ "hw/tpm_drivers.h" // tpm_drivers[]
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "sha.h" // sha1, sha256, ...
#ಅನ್ತರ್ಭಾವ "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#ಅನ್ತರ್ಭಾವ "std/smbios.h" // struct smbios_21_entry_point
#ಅನ್ತರ್ಭಾವ "std/tcg.h" // TCG_PC_LOGOVERFLOW
#ಅನ್ತರ್ಭಾವ "string.h" // checksum
#ಅನ್ತರ್ಭಾವ "tcgbios.h"// tpm_*, prototypes
#ಅನ್ತರ್ಭಾವ "util.h" // printf, get_keystroke
#ಅನ್ತರ್ಭಾವ "stacks.h" // wait_threads, reset
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

ಸಂರಚನಾ {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_ಎಣಿಕೆ;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    ಯದಿ (!log_area_start_address || !log_area_minimum_length)
        ಪ್ರತ್ಯಾಗಮ -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = ರಿಕ್ತ;
    tpm_state.entry_ಎಣಿಕೆ = 0;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_tcpa_probe(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    ಯದಿ (!tcpa)
        ಪ್ರತ್ಯಾಗಮ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    ಪ್ರತ್ಯಾಗಮ tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_tpm2_probe(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    ಯದಿ (!tpm2)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (tpm2->length < 76)
        ಪ್ರತ್ಯಾಗಮ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    ಪ್ರತ್ಯಾಗಮ tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_log_event(ಸಂರಚನಾ tpm_log_header *entry, ಪೂರ್ಣಾಙ್ಕ digest_len
              , ಧ್ರುವ ಶೂನ್ಯ *event, ಪೂರ್ಣಾಙ್ಕ event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    ಯದಿ (tpm_state.log_area_next_entry == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;

    u32 size = (ಪರಿಮಾಣ(*entry) + digest_len
                + ಪರಿಮಾಣ(ಸಂರಚನಾ tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    ಯದಿ (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಶೂನ್ಯ *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, ಪರಿಮಾಣ(*entry) + digest_len);
    ಸಂರಚನಾ tpm_log_trailer *t = dest + ಪರಿಮಾಣ(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_ಎಣಿಕೆ++;

    ಪ್ರತ್ಯಾಗಮ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ಸ್ಥಿರ TPMVersion TPM_version;
ಸ್ಥಿರ u32 tpm20_pcr_selection_size;
ಸ್ಥಿರ ಸಂರಚನಾ tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
ಸಂರಚನಾ tpm_log_entry {
    ಸಂರಚನಾ tpm_log_header hdr;
    u8 pad[ಪರಿಮಾಣ(ಸಂರಚನಾ tpm2_digest_ಮೌಲ್ಯಗಳು)
           + 8 * ಪರಿಮಾಣ(ಸಂರಚನಾ tpm2_digest_ಮೌಲ್ಯ)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ಧ್ರುವ ವರ್ಣ *name;
    ಶೂನ್ಯ (*hashfunc)(ಧ್ರುವ u8 *data, u32 length, u8 *hash);
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

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ಯದಿ (hash_parameters[i].hashalg == hashAlg)
            ಪ್ರತ್ಯಾಗಮ hash_parameters[i].hash_buffersize;
    }
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ಯದಿ (hash_parameters[i].hashalg == hashAlg)
            ಪ್ರತ್ಯಾಗಮ hash_parameters[i].hashalg_flag;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ಯದಿ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ಪ್ರತ್ಯಾಗಮ hash_parameters[i].hashalg;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ಯದಿ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ಪ್ರತ್ಯಾಗಮ hash_parameters[i].name;
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಶೂನ್ಯ tpm2_hash_data(u16 hashAlg, ಧ್ರುವ u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ಯದಿ (hash_parameters[i].hashalg == hashAlg) {
            ಯದಿ (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } ಅನ್ಯಥಾ {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_write_EfiSpecIdEventStruct(ಶೂನ್ಯ)
{
    ಯದಿ (!tpm20_pcr_selection)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ {
        ಸಂರಚನಾ TCG_EfiSpecIdEventStruct hdr;
        u8 pad[ಪರಿಮಾಣ(ಸಂರಚನಾ tpm_log_entry) + ಪರಿಮಾಣ(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    ಸಂರಚನಾ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ಶೂನ್ಯ *nsel, *end = (ಶೂನ್ಯ*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 ಎಣಿಕೆ, ಅಂಕೆ_Algs = 0;
    ಪ್ರತಿ (ಎಣಿಕೆ = 0; ಎಣಿಕೆ < be32_to_cpu(tpm20_pcr_selection->ಎಣಿಕೆ); ಎಣಿಕೆ++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (ಶೂನ್ಯ*)sel + ಪರಿಮಾಣ(*sel) + sizeOfSelect;
        ಯದಿ (nsel > end)
            ಭಙ್ಗ;

        ಯದಿ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ಅನುವರ್ತ;
        }

        ಪೂರ್ಣಾಙ್ಕ hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ಯದಿ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        ಪೂರ್ಣಾಙ್ಕ event_size = offsetof(ಸಂರಚನಾ TCG_EfiSpecIdEventStruct
                                  , digestSizes[ಎಣಿಕೆ+1]);
        ಯದಿ (event_size > ಪರಿಮಾಣ(event) - ಪರಿಮಾಣ(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        event.hdr.digestSizes[ಅಂಕೆ_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[ಅಂಕೆ_Algs].digestSize = hsize;
        ಅಂಕೆ_Algs++;

        sel = nsel;
    }

    ಯದಿ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    event.hdr.ಸಂಖ್ಯೆ_Of_Algorithms = ಅಂಕೆ_Algs;
    ಪೂರ್ಣಾಙ್ಕ event_size = offsetof(ಸಂರಚನಾ TCG_EfiSpecIdEventStruct
                              , digestSizes[ಅಂಕೆ_Algs]);
    u8 *vendorInfoSize = (ಶೂನ್ಯ*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += ಪರಿಮಾಣ(*vendorInfoSize);

    ಸಂರಚನಾ tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    ಪ್ರತ್ಯಾಗಮ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_build_digest(ಸಂರಚನಾ tpm_log_entry *le,
                   ಧ್ರುವ u8 *hashdata, u32 hashdata_len, ಪೂರ್ಣಾಙ್ಕ bigEndian)
{
    ಯದಿ (!tpm20_pcr_selection)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ಶೂನ್ಯ *nsel, *end = (ಶೂನ್ಯ*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    ಶೂನ್ಯ *dest = le->hdr.digest + ಪರಿಮಾಣ(ಸಂರಚನಾ tpm2_digest_ಮೌಲ್ಯಗಳು);

    u32 ಎಣಿಕೆ, ಅಂಕೆ_Algs = 0;
    ಪ್ರತಿ (ಎಣಿಕೆ = 0; ಎಣಿಕೆ < be32_to_cpu(tpm20_pcr_selection->ಎಣಿಕೆ); ಎಣಿಕೆ++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (ಶೂನ್ಯ*)sel + ಪರಿಮಾಣ(*sel) + sizeOfSelect;
        ಯದಿ (nsel > end)
            ಭಙ್ಗ;

        /* PCR 0-7 unused? -- skip */
        ಯದಿ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ಅನುವರ್ತ;
        }

        ಪೂರ್ಣಾಙ್ಕ hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ಯದಿ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        /* buffer size sanity check before writing */
        ಸಂರಚನಾ tpm2_digest_ಮೌಲ್ಯ *v = dest;
        ಯದಿ (dest + ಪರಿಮಾಣ(*v) + hsize > (ಶೂನ್ಯ*)le + ಪರಿಮಾಣ(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        ಯದಿ (bigEndian)
            v->hashAlg = sel->hashAlg;
        ಅನ್ಯಥಾ
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += ಪರಿಮಾಣ(*v) + hsize;
        sel = nsel;

        ಅಂಕೆ_Algs++;
    }

    ಯದಿ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಸಂರಚನಾ tpm2_digest_ಮೌಲ್ಯಗಳು *v = (ಶೂನ್ಯ*)le->hdr.digest;
    ಯದಿ (bigEndian)
        v->ಎಣಿಕೆ = cpu_to_be32(ಅಂಕೆ_Algs);
    ಅನ್ಯಥಾ
        v->ಎಣಿಕೆ = ಅಂಕೆ_Algs;

    ಪ್ರತ್ಯಾಗಮ dest - (ಶೂನ್ಯ*)le->hdr.digest;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_build_digest(ಸಂರಚನಾ tpm_log_entry *le,
                   ಧ್ರುವ u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    ಪ್ರತ್ಯಾಗಮ SHA1_BUFSIZE;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_build_digest_direct(ಸಂರಚನಾ tpm_log_entry *le, ಧ್ರುವ u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    ಪ್ರತ್ಯಾಗಮ SHA1_BUFSIZE;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_build_digest(ಸಂರಚನಾ tpm_log_entry *le, ಧ್ರುವ u8 *hashdata, u32 hashdata_len
                 , ಪೂರ್ಣಾಙ್ಕ bigEndian)
{
    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        ಪ್ರತ್ಯಾಗಮ tpm12_build_digest(le, hashdata, hashdata_len);
    ಪಕ್ಷ TPM_VERSION_2:
        ಪ್ರತ್ಯಾಗಮ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    ಪ್ರತ್ಯಾಗಮ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_simple_cmd(u8 locty, u32 ordinal
               , ಪೂರ್ಣಾಙ್ಕ param_size, u16 param, ಗಣನಾ tpmDurationType to_t)
{
    ಸಂರಚನಾ {
        ಸಂರಚನಾ tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(ಪರಿಮಾಣ(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        ಭಙ್ಗ;
    ಪಕ್ಷ TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        ಭಙ್ಗ;
    }

    u8 obuffer[64];
    ಸಂರಚನಾ tpm_rsp_header *trsh = (ಶೂನ್ಯ*)obuffer;
    u32 obuffer_len = ಪರಿಮಾಣ(obuffer);
    memset(obuffer, 0x0, ಪರಿಮಾಣ(obuffer));

    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_getcapability(u32 capability, u32 property, u32 ಎಣಿಕೆ,
                    ಸಂರಚನಾ tpm_rsp_header *rsp, u32 rsize)
{
    ಸಂರಚನಾ tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(ಪರಿಮಾಣ(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(ಎಣಿಕೆ),
    };

    u32 resp_size = rsize;
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_get_pcrbanks(ಶೂನ್ಯ)
{
    u8 buffer[128];
    ಸಂರಚನಾ tpm2_res_getcapability *trg =
      (ಸಂರಚನಾ tpm2_res_getcapability *)&buffer;

    ಪೂರ್ಣಾಙ್ಕ ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  ಪರಿಮಾಣ(buffer));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    ಯದಿ (resplen <= offsetof(ಸಂರಚನಾ tpm2_res_getcapability, data))
        ಪ್ರತ್ಯಾಗಮ -1;

    u32 size = resplen - offsetof(ಸಂರಚನಾ tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    ಯದಿ (size < offsetof(ಸಂರಚನಾ tpml_pcr_selection, selections) +
               offsetof(ಸಂರಚನಾ tpms_pcr_selection, pcrSelect))
        ಪ್ರತ್ಯಾಗಮ -1;

    tpm20_pcr_selection = malloc_ಮೇಲಿನ(size);
    ಯದಿ (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } ಅನ್ಯಥಾ {
        warn_noalloc();
        ret = -1;
    }

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    ಯದಿ (!tpm20_pcr_selection)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ಶೂನ್ಯ *end = (ಶೂನ್ಯ*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    ಯಾವತ್ (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        ಶೂನ್ಯ *nsel = (ಶೂನ್ಯ*)sel + ಪರಿಮಾಣ(*sel) + sizeOfSelect;
        ಯದಿ (nsel > end)
            ಪ್ರತ್ಯಾಗಮ 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        ಪ್ರತಿ (i = 0; i < sizeOfSelect; i++) {
            ಯದಿ (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                ಭಙ್ಗ;
            }
        }

        sel = nsel;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_set_pcrbanks(u32 active_banks)
{
    ಸಂರಚನಾ tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ಪರಿಮಾಣ(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ಸಂರಚನಾ tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    ಪೂರ್ಣಾಙ್ಕ i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    ಯಾವತ್ (hashalg_flag) {
        ಯದಿ ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            ಯದಿ (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                ಯದಿ (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.ಎಣಿಕೆ = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * ಪರಿಮಾಣ(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(ಸಂರಚನಾ tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * ಪರಿಮಾಣ(tps[0]));

    ಸಂರಚನಾ tpm_rsp_header rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);

    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tpm20_activate_pcrbanks(u32 active_banks)
{
    ಪೂರ್ಣಾಙ್ಕ ret = tpm20_set_pcrbanks(active_banks);
    ಯದಿ (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (!ret)
        reset();
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_get_capability(u32 cap, u32 subcap, ಸಂರಚನಾ tpm_rsp_header *rsp, u32 rsize)
{
    ಸಂರಚನಾ tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(ಪರಿಮಾಣ(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(ಪರಿಮಾಣ(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_read_permanent_flags(ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ buf_len)
{
    memset(buf, 0, buf_len);

    ಸಂರಚನಾ tpm_res_getcap_perm_flags pf;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, ಪರಿಮಾಣ(pf));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_determine_timeouts(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ tpm_res_getcap_timeouts timeouts;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, ಪರಿಮಾಣ(timeouts));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಸಂರಚನಾ tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, ಪರಿಮಾಣ(durations));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    ಪ್ರತಿ (i = 0; i < 4; i++)
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

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm20_set_timeouts(ಶೂನ್ಯ)
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

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_extend(ಸಂರಚನಾ tpm_log_entry *le, ಪೂರ್ಣಾಙ್ಕ digest_len)
{
    ಸಂರಚನಾ tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(ಪರಿಮಾಣ(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, ಪರಿಮಾಣ(tre.digest));

    ಸಂರಚನಾ tpm_rsp_extend rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.hdr.errcode)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tpm20_extend(ಸಂರಚನಾ tpm_log_entry *le, ಪೂರ್ಣಾಙ್ಕ digest_len)
{
    ಸಂರಚನಾ tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(ಪರಿಮಾಣ(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[ಪರಿಮಾಣ(tmp_tre) + ಪರಿಮಾಣ(le->pad)];
    ಸಂರಚನಾ tpm2_req_extend *tre = (ಸಂರಚನಾ tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, ಪರಿಮಾಣ(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(ಪರಿಮಾಣ(tmp_tre) + digest_len);

    ಸಂರಚನಾ tpm_rsp_header rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.errcode)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_extend(ಸಂರಚನಾ tpm_log_entry *le, ಪೂರ್ಣಾಙ್ಕ digest_len)
{
    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        ಪ್ರತ್ಯಾಗಮ tpm12_extend(le, digest_len);
    ಪಕ್ಷ TPM_VERSION_2:
        ಪ್ರತ್ಯಾಗಮ tpm20_extend(le, digest_len);
    }
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_stirrandom(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(ಪರಿಮಾಣ(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(ಪರಿಮಾಣ(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    ಸಂರಚನಾ tpm_rsp_header rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    ಸಂರಚನಾ tpm2_res_getrandom rsp;

    ಯದಿ (buf_len > ಪರಿಮಾಣ(rsp.rnd.buffer))
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(ಪರಿಮಾಣ(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = ಪರಿಮಾಣ(rsp);

    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.hdr.errcode)
        ret = -1;
    ಅನ್ಯಥಾ
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    ಸಂರಚನಾ tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(ಪರಿಮಾಣ(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ಪರಿಮಾಣ(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    ಸಂರಚನಾ tpm_rsp_header rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_hierarchychangeauth(u8 auth[20])
{
    ಸಂರಚನಾ tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(ಪರಿಮಾಣ(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ಪರಿಮಾಣ(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(ಪರಿಮಾಣ(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, ಪರಿಮಾಣ(trhca.newAuth.buffer));

    ಸಂರಚನಾ tpm_rsp_header rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ TPM_has_physical_presence;
u8 TPM_working VARLOW;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_is_working(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ CONFIG_TCGBIOS && TPM_working;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm_set_failure(ಶೂನ್ಯ)
{
    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        ಭಙ್ಗ;
    ಪಕ್ಷ TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        ಭಙ್ಗ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           ಧ್ರುವ ವರ್ಣ *event, u32 event_length,
                           ಧ್ರುವ u8 *hashdata, u32 hashdata_length)
{
    ಯದಿ (!tpm_is_working())
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    ಪೂರ್ಣಾಙ್ಕ digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    ಯದಿ (digest_len < 0)
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm_extend(&le, digest_len);
    ಯದಿ (ret) {
        tpm_set_failure();
        ಪ್ರತ್ಯಾಗಮ;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
ಸ್ಥಿರ ಶೂನ್ಯ
tpm_add_action(u32 pcrIndex, ಧ್ರುವ ವರ್ಣ *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tpm_add_event_separators(ಶೂನ್ಯ)
{
    ಸ್ಥಿರ ಧ್ರುವ u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    ಪ್ರತಿ (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (ಧ್ರುವ ವರ್ಣ *)evt_separator,
                                   ಪರಿಮಾಣ(evt_separator),
                                   evt_separator,
                                   ಪರಿಮಾಣ(evt_separator));
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm_smbios_measure(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    ಶೂನ್ಯ *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    ಯದಿ (!smbios_tables)
        ಪ್ರತ್ಯಾಗಮ;

    sha1((ಧ್ರುವ u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (ಧ್ರುವ ವರ್ಣ *)&pcctes, ಪರಿಮಾಣ(pcctes),
                               (u8 *)&pcctes, ಪರಿಮಾಣ(pcctes));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_assert_physical_presence(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (!ret)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಸಂರಚನಾ tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ವರ್ಣ *)&pf, ಪರಿಮಾಣ(pf));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* check if hardware physical presence is supported */
    ಯದಿ (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    ಯದಿ (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        ಪ್ರತ್ಯಾಗಮ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_startup(ಶೂನ್ಯ)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    ಪೂರ್ಣಾಙ್ಕ ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    ಯದಿ (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret && ret != TPM_BAD_LOCALITY)
        ಗಚ್ಛ err_exit;

    ಪ್ರತ್ಯಾಗಮ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_startup(ಶೂನ್ಯ)
{
    tpm20_set_timeouts();

    ಪೂರ್ಣಾಙ್ಕ ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    ಯದಿ (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ret = tpm20_get_pcrbanks();
    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ಪ್ರತ್ಯಾಗಮ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm_startup(ಶೂನ್ಯ)
{
    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        ಪ್ರತ್ಯಾಗಮ tpm12_startup();
    ಪಕ್ಷ TPM_VERSION_2:
        ಪ್ರತ್ಯಾಗಮ tpm20_startup();
    }
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಶೂನ್ಯ
tpm_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ;

    ಪೂರ್ಣಾಙ್ಕ ret = tpm_tpm2_probe();
    ಯದಿ (ret) {
        ret = tpm_tcpa_probe();
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ;
    }

    TPM_version = tpmhw_probe();
    ಯದಿ (TPM_version == TPM_VERSION_NONE)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    ಯದಿ (runningOnXen())
        ಪ್ರತ್ಯಾಗಮ;

    ret = tpm_startup();
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm20_prepboot(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ ret = tpm20_stirrandom();
    ಯದಿ (ret)
         ಗಚ್ಛ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], ಪರಿಮಾಣ(auth));
    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ಪ್ರತ್ಯಾಗಮ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

ಶೂನ್ಯ
tpm_prepboot(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ;

    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        ಯದಿ (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        ಭಙ್ಗ;
    ಪಕ್ಷ TPM_VERSION_2:
        tpm20_prepboot();
        ಭಙ್ಗ;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
ಶೂನ್ಯ
tpm_option_rom(ಧ್ರುವ ಶೂನ್ಯ *addr, u32 len)
{
    ಯದಿ (!tpm_is_working())
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = ಪರಿಮಾಣ(u16) + ಪರಿಮಾಣ(u16) + SHA1_BUFSIZE,
    };
    sha1((ಧ್ರುವ u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (ಧ್ರುವ ವರ್ಣ *)&pcctes, ಪರಿಮಾಣ(pcctes),
                               (u8 *)&pcctes, ಪರಿಮಾಣ(pcctes));
}

ಶೂನ್ಯ
tpm_add_bcv(u32 bootdrv, ಧ್ರುವ u8 *addr, u32 length)
{
    ಯದಿ (!tpm_is_working())
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (length < 0x200)
        ಪ್ರತ್ಯಾಗಮ;

    ಧ್ರುವ ವರ್ಣ *string = "Booting BCV device 00h (Floppy)";
    ಯದಿ (bootdrv == 0x80)
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

ಶೂನ್ಯ
tpm_add_cdrom(u32 bootdrv, ಧ್ರುವ u8 *addr, u32 length)
{
    ಯದಿ (!tpm_is_working())
        ಪ್ರತ್ಯಾಗಮ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ಧ್ರುವ ವರ್ಣ *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

ಶೂನ್ಯ
tpm_add_cdrom_catalog(ಧ್ರುವ u8 *addr, u32 length)
{
    ಯದಿ (!tpm_is_working())
        ಪ್ರತ್ಯಾಗಮ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ಧ್ರುವ ವರ್ಣ *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

ಶೂನ್ಯ
tpm_s3_resume(ಶೂನ್ಯ)
{
    ಯದಿ (!tpm_is_working())
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    ಪೂರ್ಣಾಙ್ಕ ret = -1;

    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        ಭಙ್ಗ;
    ಪಕ್ಷ TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        ಯದಿ (ret)
            ಗಚ್ಛ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        ಭಙ್ಗ;
    }

    ಯದಿ (ret)
        ಗಚ್ಛ err_exit;

    ಪ್ರತ್ಯಾಗಮ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *ಒಳಹರಿವು_buf32(ಸಂರಚನಾ bregs *regs)
{
    ಪ್ರತ್ಯಾಗಮ MAKE_FLATPTR(regs->es, regs->di);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *output_buf32(ಸಂರಚನಾ bregs *regs)
{
    ಪ್ರತ್ಯಾಗಮ MAKE_FLATPTR(regs->ds, regs->si);
}

ಸ್ಥಿರ u32
hash_log_extend(ಸಂರಚನಾ pcpes *pcpes, ಧ್ರುವ ಶೂನ್ಯ *hashdata, u32 hashdata_length
                , ಶೂನ್ಯ *event, ಪೂರ್ಣಾಙ್ಕ extend)
{
    ಯದಿ (pcpes->pcrindex >= 24)
        ಪ್ರತ್ಯಾಗಮ TCG_INVALID_ಒಳಹರಿವು_PARA;
    ಯದಿ (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    ಸಂರಚನಾ tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    ಪೂರ್ಣಾಙ್ಕ digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    ಯದಿ (digest_len < 0)
        ಪ್ರತ್ಯಾಗಮ TCG_GENERAL_ERROR;
    ಯದಿ (extend) {
        ಪೂರ್ಣಾಙ್ಕ ret = tpm_extend(&le, digest_len);
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    ಪೂರ್ಣಾಙ್ಕ ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ TCG_PC_LOGOVERFLOW;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32
hash_log_extend_event_int(ಧ್ರುವ ಸಂರಚನಾ hleei_short *hleei_s,
                          ಸಂರಚನಾ hleeo *hleeo)
{
    u32 rc = 0;
    ಸಂರಚನಾ hleei_long *hleei_l = (ಸಂರಚನಾ hleei_long *)hleei_s;
    ಧ್ರುವ ಶೂನ್ಯ *logdataptr;
    u32 logdatalen;
    ಸಂರಚನಾ pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    ವಿಕಲ್ಪ (hleei_s->ipblength) {
    ಪಕ್ಷ ಪರಿಮಾಣ(ಸಂರಚನಾ hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    ಭಙ್ಗ;

    ಪಕ್ಷ ಪರಿಮಾಣ(ಸಂರಚನಾ hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    ಭಙ್ಗ;

    ಸ್ವಭಾವ:
        /* bad input block */
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಗಚ್ಛ err_exit;
    }

    pcpes = (ಸಂರಚನಾ pcpes *)logdataptr;

    ಯದಿ (pcpes->pcrindex != pcrindex
        || logdatalen != ಪರಿಮಾಣ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಗಚ್ಛ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    ಯದಿ (rc)
        ಗಚ್ಛ err_exit;

    hleeo->opblength = ಪರಿಮಾಣ(ಸಂರಚನಾ hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_ಎಣಿಕೆ;
    memcpy(hleeo->digest, pcpes->digest, ಪರಿಮಾಣ(hleeo->digest));

err_exit:
    ಯದಿ (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32
pass_through_to_tpm_int(ಸಂರಚನಾ pttti *pttti, ಸಂರಚನಾ pttto *pttto)
{
    u32 rc = 0;
    ಸಂರಚನಾ tpm_req_header *trh = (ಶೂನ್ಯ*)pttti->tpmopin;

    ಯದಿ (pttti->ipblength < ಪರಿಮಾಣ(ಸಂರಚನಾ pttti) + ಪರಿಮಾಣ(*trh)
        || pttti->ipblength != ಪರಿಮಾಣ(ಸಂರಚನಾ pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < ಪರಿಮಾಣ(ಸಂರಚನಾ pttto)) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಗಚ್ಛ err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        ಯದಿ (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
            ಗಚ್ಛ err_exit;
        }
        ಭಙ್ಗ;
    ಪಕ್ಷ TPM_VERSION_2:
        ಯದಿ (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
            ಗಚ್ಛ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(ಸಂರಚನಾ pttto, tpmopout);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    ಯದಿ (ret) {
        rc = TCG_FATAL_COM_ERROR;
        ಗಚ್ಛ err_exit;
    }

    pttto->opblength = offsetof(ಸಂರಚನಾ pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    ಯದಿ (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32
shutdown_preboot_interface(ಶೂನ್ಯ)
{
    TPM_interface_shutdown = 1;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32
hash_log_event_int(ಧ್ರುವ ಸಂರಚನಾ hlei *hlei, ಸಂರಚನಾ hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    ಸಂರಚನಾ pcpes *pcpes;

    size = hlei->ipblength;
    ಯದಿ (size != ಪರಿಮಾಣ(*hlei)) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಗಚ್ಛ err_exit;
    }

    pcpes = (ಸಂರಚನಾ pcpes *)hlei->logdataptr;

    ಯದಿ (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != ಪರಿಮಾಣ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಗಚ್ಛ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    ಯದಿ (rc)
        ಗಚ್ಛ err_exit;

    /* updating the log was fine */
    hleo->opblength = ಪರಿಮಾಣ(ಸಂರಚನಾ hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_ಎಣಿಕೆ;

err_exit:
    ಯದಿ (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32
hash_all_int(ಧ್ರುವ ಸಂರಚನಾ hai *hai, u8 *hash)
{
    ಯದಿ (hai->ipblength != ಪರಿಮಾಣ(ಸಂರಚನಾ hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        ಪ್ರತ್ಯಾಗಮ TCG_INVALID_ಒಳಹರಿವು_PARA;

    sha1((ಧ್ರುವ u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32
tss_int(ಸಂರಚನಾ ti *ti, ಸಂರಚನಾ to *to)
{
    to->opblength = ಪರಿಮಾಣ(ಸಂರಚನಾ to);
    to->reserved  = 0;

    ಪ್ರತ್ಯಾಗಮ TCG_PC_UNSUPPORTED;
}

ಸ್ಥಿರ u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    ಸಂರಚನಾ pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = ಪರಿಮಾಣ(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    ಯದಿ (rc)
        ಪ್ರತ್ಯಾಗಮ rc;

    *edx_ptr = tpm_state.entry_ಎಣಿಕೆ;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ VISIBLE32FLAT
tpm_interrupt_handler32(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ;

    set_cf(regs, 0);

    ಯದಿ (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        ಪ್ರತ್ಯಾಗಮ;
    }

    ವಿಕಲ್ಪ ((ಗಣನಾ irq_ids)regs->al) {
    ಪಕ್ಷ TCG_StatusCheck:
        ಯದಿ (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } ಅನ್ಯಥಾ {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        ಭಙ್ಗ;

    ಪಕ್ಷ TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (ಸಂರಚನಾ hleei_short *)ಒಳಹರಿವು_buf32(regs),
                  (ಸಂರಚನಾ hleeo *)output_buf32(regs));
        ಭಙ್ಗ;

    ಪಕ್ಷ TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((ಸಂರಚನಾ pttti *)ಒಳಹರಿವು_buf32(regs),
                                    (ಸಂರಚನಾ pttto *)output_buf32(regs));
        ಭಙ್ಗ;

    ಪಕ್ಷ TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        ಭಙ್ಗ;

    ಪಕ್ಷ TCG_HashLogEvent:
        regs->eax = hash_log_event_int((ಸಂರಚನಾ hlei*)ಒಳಹರಿವು_buf32(regs),
                                       (ಸಂರಚನಾ hleo*)output_buf32(regs));
        ಭಙ್ಗ;

    ಪಕ್ಷ TCG_HashAll:
        regs->eax =
            hash_all_int((ಸಂರಚನಾ hai*)ಒಳಹರಿವು_buf32(regs),
                          (u8 *)output_buf32(regs));
        ಭಙ್ಗ;

    ಪಕ್ಷ TCG_TSS:
        regs->eax = tss_int((ಸಂರಚನಾ ti*)ಒಳಹರಿವು_buf32(regs),
                            (ಸಂರಚನಾ to*)output_buf32(regs));
        ಭಙ್ಗ;

    ಪಕ್ಷ TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)ಒಳಹರಿವು_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        ಭಙ್ಗ;

    ಸ್ವಭಾವ:
        set_cf(regs, 1);
    }

    ಪ್ರತ್ಯಾಗಮ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

ಪ್ರಕಾರನಿರ್ದೇಶ u8 tpm_ppi_code;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_read_has_owner(ಪೂರ್ಣಾಙ್ಕ *has_owner)
{
    ಸಂರಚನಾ tpm_res_getcap_ownerauth oauth;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, ಪರಿಮಾಣ(oauth));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;

    *has_owner = oauth.flag;

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_enable_tpm(ಪೂರ್ಣಾಙ್ಕ enable, ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಸಂರಚನಾ tpm_permanent_flags pf;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_read_permanent_flags((ವರ್ಣ *)&pf, ಪರಿಮಾಣ(pf));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        ಪ್ರತ್ಯಾಗಮ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret) {
        ಯದಿ (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        ಅನ್ಯಥಾ
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_activate_tpm(ಪೂರ್ಣಾಙ್ಕ activate, ಪೂರ್ಣಾಙ್ಕ allow_reset, ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಸಂರಚನಾ tpm_permanent_flags pf;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_read_permanent_flags((ವರ್ಣ *)&pf, ಪರಿಮಾಣ(pf));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ (pf.flags[PERM_FLAG_IDX_DISABLE])
        ಪ್ರತ್ಯಾಗಮ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಯದಿ (activate && allow_reset) {
        ಯದಿ (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_enable_activate(ಪೂರ್ಣಾಙ್ಕ allow_reset, ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_enable_tpm(1, verbose);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಪ್ರತ್ಯಾಗಮ tpm12_activate_tpm(1, allow_reset, verbose);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_force_clear(ಪೂರ್ಣಾಙ್ಕ enable_activate_before, ಪೂರ್ಣಾಙ್ಕ enable_activate_after,
                  ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಪೂರ್ಣಾಙ್ಕ has_owner;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_read_has_owner(&has_owner);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!has_owner) {
        ಯದಿ (verbose)
            printf("TPM does not have an owner.\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    ಯದಿ (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        ಯದಿ (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            ಪ್ರತ್ಯಾಗಮ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಯದಿ (!enable_activate_after) {
        ಯದಿ (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    ಪ್ರತ್ಯಾಗಮ tpm12_enable_activate(1, verbose);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_set_owner_install(ಪೂರ್ಣಾಙ್ಕ allow, ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಪೂರ್ಣಾಙ್ಕ has_owner;
    ಪೂರ್ಣಾಙ್ಕ ret = tpm12_read_has_owner(&has_owner);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (has_owner) {
        ಯದಿ (verbose)
            printf("Must first remove owner.\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    ಸಂರಚನಾ tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ವರ್ಣ *)&pf, ಪರಿಮಾಣ(pf));
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        ಯದಿ (verbose)
            printf("TPM must first be enable.\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಯದಿ (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_process_cfg(tpm_ppi_code ಸಂದೇಶ_Code, ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಪೂರ್ಣಾಙ್ಕ ret = 0;

    ವಿಕಲ್ಪ (ಸಂದೇಶ_Code) {
        ಪಕ್ಷ TPM_PPI_OP_NOOP: /* no-op */
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            ಭಙ್ಗ;

        ಸ್ವಭಾವ:
            ಭಙ್ಗ;
    }

    ಯದಿ (ret)
        printf("Op %d: An error occurred: 0x%x\n", ಸಂದೇಶ_Code, ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_clearcontrol(u8 disable, ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಸಂರಚನಾ tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(ಪರಿಮಾಣ(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ಪರಿಮಾಣ(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    ಸಂರಚನಾ tpm_rsp_header rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_clear(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(ಪರಿಮಾಣ(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(ಪರಿಮಾಣ(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    ಸಂರಚನಾ tpm_rsp_header rsp;
    u32 resp_length = ಪರಿಮಾಣ(rsp);
    ಪೂರ್ಣಾಙ್ಕ ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ಯದಿ (ret || resp_length != ಪರಿಮಾಣ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_process_cfg(tpm_ppi_code ಸಂದೇಶ_Code, ಪೂರ್ಣಾಙ್ಕ verbose)
{
    ಪೂರ್ಣಾಙ್ಕ ret = 0;

    ವಿಕಲ್ಪ (ಸಂದೇಶ_Code) {
        ಪಕ್ಷ TPM_PPI_OP_NOOP: /* no-op */
            ಭಙ್ಗ;

        ಪಕ್ಷ TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            ಯದಿ (!ret)
                 ret = tpm20_clear();
            ಭಙ್ಗ;
    }

    ಯದಿ (ret)
        printf("Op %d: An error occurred: 0x%x\n", ಸಂದೇಶ_Code, ret);

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm12_get_tpm_state(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ state = 0;
    ಸಂರಚನಾ tpm_permanent_flags pf;
    ಪೂರ್ಣಾಙ್ಕ has_owner;

    ಯದಿ (tpm12_read_permanent_flags((ವರ್ಣ *)&pf, ಪರಿಮಾಣ(pf)) ||
        tpm12_read_has_owner(&has_owner))
        ಪ್ರತ್ಯಾಗಮ ~0;

    ಯದಿ (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    ಯದಿ (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    ಯದಿ (has_owner) {
        state |= TPM_STATE_OWNED;
    } ಅನ್ಯಥಾ {
        ಯದಿ (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    ಪ್ರತ್ಯಾಗಮ state;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm12_show_tpm_menu(ಪೂರ್ಣಾಙ್ಕ state, ಪೂರ್ಣಾಙ್ಕ next_scancodes[7])
{
    ಪೂರ್ಣಾಙ್ಕ i = 0;

    printf("\nThe current state of the TPM is:\n");

    ಯದಿ (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    ಅನ್ಯಥಾ
        printf("  Disabled");

    ಯದಿ (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    ಅನ್ಯಥಾ
        printf(" and deactivated\n");

    ಯದಿ (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    ಅನ್ಯಥಾ {
        printf("  Ownership has not been taken\n");
        ಯದಿ (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        ಅನ್ಯಥಾ
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    ಯದಿ ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    ಯದಿ (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        ಯದಿ (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            ಯದಿ (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } ಅನ್ಯಥಾ {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } ಅನ್ಯಥಾ {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } ಅನ್ಯಥಾ {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    ಯದಿ (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm12_menu(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ scancode, next_scancodes[7];
    tpm_ppi_code ಸಂದೇಶ_Code;
    ಪೂರ್ಣಾಙ್ಕ state = 0, i;
    ಪೂರ್ಣಾಙ್ಕ waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    ಪ್ರತಿ (;;) {
        ಯದಿ ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } ಅನ್ಯಥಾ {
            printf("TPM is not working correctly.\n");
            ಪ್ರತ್ಯಾಗಮ;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ಸಂದೇಶ_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        ಯಾವತ್ (waitkey) {
            ಯಾವತ್ ((scancode = get_keystroke(1000)) == ~0)
                ;

            ವಿಕಲ್ಪ (scancode) {
            ಪಕ್ಷ 1:
                // ESC
                reset();
                ಭಙ್ಗ;
            ಪಕ್ಷ 18: /* e. enable */
                ಸಂದೇಶ_Code = TPM_PPI_OP_ENABLE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 32: /* d. disable */
                ಸಂದೇಶ_Code = TPM_PPI_OP_DISABLE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 30: /* a. activate */
                ಸಂದೇಶ_Code = TPM_PPI_OP_ACTIVATE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 47: /* v. deactivate */
                ಸಂದೇಶ_Code = TPM_PPI_OP_DEACTIVATE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 46: /* c. clear owner */
                ಸಂದೇಶ_Code = TPM_PPI_OP_CLEAR;
                ಭಙ್ಗ;
            ಪಕ್ಷ 25: /* p. prevent ownerinstall */
                ಸಂದೇಶ_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 31: /* s. allow ownerinstall */
                ಸಂದೇಶ_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                ಭಙ್ಗ;
            ಸ್ವಭಾವ:
                ಅನುವರ್ತ;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            ಪ್ರತಿ (i = 0; i < ಪರಿಮಾಣ(next_scancodes); i++) {
                ಯದಿ (next_scancodes[i] == 0)
                    ಭಙ್ಗ;

                ಯದಿ (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(ಸಂದೇಶ_Code, 1);
                    waitkey = 0;
                    ಭಙ್ಗ;
                }
            }
        }
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tpm20_menu_change_active_pcrbanks(ಶೂನ್ಯ)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    ಯಾವತ್ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        ಯಾವತ್ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ಧ್ರುವ ವರ್ಣ *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            ಯದಿ (hashname) {
                printf("  %d: %s", i, hashname);
                ಯದಿ (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        ಯದಿ (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        ಪೂರ್ಣಾಙ್ಕ show = 0;
        ಯಾವತ್ (!show) {
            ಪೂರ್ಣಾಙ್ಕ scancode = get_keystroke(1000);

            ವಿಕಲ್ಪ (scancode) {
            ಪಕ್ಷ ~0:
                ಅನುವರ್ತ;
            ಪಕ್ಷ 1: /* ESC */
                printf("\n");
                ಪ್ರತ್ಯಾಗಮ -1;
            ಪಕ್ಷ 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                ಯದಿ (flagnum > i)
                    ಅನುವರ್ತ;
                ಯದಿ (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                ಭಙ್ಗ;
            ಪಕ್ಷ 30: /* a */
                ಯದಿ (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm20_menu(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ scan_code;
    tpm_ppi_code ಸಂದೇಶ_Code;

    ಪ್ರತಿ (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ಸಂದೇಶ_Code = TPM_PPI_OP_NOOP;

        ಯಾವತ್ ((scan_code = get_keystroke(1000)) == ~0)
            ;

        ವಿಕಲ್ಪ (scan_code) {
        ಪಕ್ಷ 1:
            // ESC
            reset();
            ಭಙ್ಗ;
        ಪಕ್ಷ 2:
            ಸಂದೇಶ_Code = TPM_PPI_OP_CLEAR;
            ಭಙ್ಗ;
        ಪಕ್ಷ 3:
            tpm20_menu_change_active_pcrbanks();
            ಅನುವರ್ತ;
        ಸ್ವಭಾವ:
            ಅನುವರ್ತ;
        }

        tpm20_process_cfg(ಸಂದೇಶ_Code, 0);
    }
}

ಶೂನ್ಯ
tpm_menu(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ;

    ಯಾವತ್ (get_keystroke(0) >= 0)
        ;
    wait_threads();

    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        tpm12_menu();
        ಭಙ್ಗ;
    ಪಕ್ಷ TPM_VERSION_2:
        tpm20_menu();
        ಭಙ್ಗ;
    }
}

ಪೂರ್ಣಾಙ್ಕ
tpm_can_show_menu(ಶೂನ್ಯ)
{
    ವಿಕಲ್ಪ (TPM_version) {
    ಪಕ್ಷ TPM_VERSION_1_2:
        ಪ್ರತ್ಯಾಗಮ tpm_is_working() && TPM_has_physical_presence;
    ಪಕ್ಷ TPM_VERSION_2:
        ಪ್ರತ್ಯಾಗಮ tpm_is_working();
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}
