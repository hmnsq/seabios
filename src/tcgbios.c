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

ರಚನೆ {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_ಉದ್ದಳತೆ;

    /* start address of TCPA log buffer */
    u8 *          log_area_ಆರಂಭ_ವಿಳಾಸ;

    /* number of log entries written */
    u32           ನಮೂದು_ಎಣಿಕೆ;

    /* address to write next log entry to */
    u8 *          log_area_ಮುಂದಿನ_ನಮೂದು;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_ಕೊನೆಯ_ನಮೂದು;
} tpm_state VARLOW;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tpm_ಗಣ_log_area(u8 *log_area_ಆರಂಭ_ವಿಳಾಸ,
                            u32 log_area_minimum_ಉದ್ದಳತೆ)
{
    ಆದರೆ (!log_area_ಆರಂಭ_ವಿಳಾಸ || !log_area_minimum_ಉದ್ದಳತೆ)
        ಹಿಂತಿರುಗಿ -1;

    memset(log_area_ಆರಂಭ_ವಿಳಾಸ, 0, log_area_minimum_ಉದ್ದಳತೆ);
    tpm_state.log_area_ಆರಂಭ_ವಿಳಾಸ = log_area_ಆರಂಭ_ವಿಳಾಸ;
    tpm_state.log_area_minimum_ಉದ್ದಳತೆ = log_area_minimum_ಉದ್ದಳತೆ;
    tpm_state.log_area_ಮುಂದಿನ_ನಮೂದು = log_area_ಆರಂಭ_ವಿಳಾಸ;
    tpm_state.log_area_ಕೊನೆಯ_ನಮೂದು = ಖಾಲಿ;
    tpm_state.ನಮೂದು_ಎಣಿಕೆ = 0;
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_tcpa_probe(ಶೂನ್ಯ)
{
    ರಚನೆ tcpa_descriptor_rev2 *tcpa = ಕಂಡುಹಿಡಿ_acpi_ಕೋಷ್ಟಕ(TCPA_SIGNATURE);
    ಆದರೆ (!tcpa)
        ಹಿಂತಿರುಗಿ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(ದೀರ್ಘ)tcpa->log_area_ಆರಂಭ_ವಿಳಾಸ,
            tcpa->log_area_minimum_ಉದ್ದಳತೆ);

    ಹಿಂತಿರುಗಿ tpm_ಗಣ_log_area((u8*)(ದೀರ್ಘ)tcpa->log_area_ಆರಂಭ_ವಿಳಾಸ,
                            tcpa->log_area_minimum_ಉದ್ದಳತೆ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_tpm2_probe(ಶೂನ್ಯ)
{
    ರಚನೆ tpm2_descriptor_rev2 *tpm2 = ಕಂಡುಹಿಡಿ_acpi_ಕೋಷ್ಟಕ(TPM2_SIGNATURE);
    ಆದರೆ (!tpm2)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (tpm2->ಉದ್ದಳತೆ < 76)
        ಹಿಂತಿರುಗಿ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(ದೀರ್ಘ)tpm2->log_area_ಆರಂಭ_ವಿಳಾಸ,
            tpm2->log_area_minimum_ಉದ್ದಳತೆ);

    ಹಿಂತಿರುಗಿ tpm_ಗಣ_log_area((u8*)(ದೀರ್ಘ)tpm2->log_area_ಆರಂಭ_ವಿಳಾಸ,
                            tpm2->log_area_minimum_ಉದ್ದಳತೆ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_log_event(ರಚನೆ tpm_log_ಶೀರ್ಷಿಕೆ *ನಮೂದು, ಪೂರ್ಣಾಂಕ digest_len
              , ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *event, ಪೂರ್ಣಾಂಕ event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_ಆರಂಭ_ವಿಳಾಸ, tpm_state.log_area_ಮುಂದಿನ_ನಮೂದು);

    ಆದರೆ (tpm_state.log_area_ಮುಂದಿನ_ನಮೂದು == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;

    u32 ಗಾತ್ರ_2 = (ಗಾತ್ರ(*ನಮೂದು) + digest_len
                + ಗಾತ್ರ(ರಚನೆ tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_ಮುಂದಿನ_ನಮೂದು + ಗಾತ್ರ_2
                   - tpm_state.log_area_ಆರಂಭ_ವಿಳಾಸ);
    ಆದರೆ (logsize > tpm_state.log_area_minimum_ಉದ್ದಳತೆ) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", ಗಾತ್ರ_2);
        ಹಿಂತಿರುಗಿ -1;
    }

    ಶೂನ್ಯ *dest = tpm_state.log_area_ಮುಂದಿನ_ನಮೂದು;
    ಸ್ಮರಣೆ_ನಕಲು(dest, ನಮೂದು, ಗಾತ್ರ(*ನಮೂದು) + digest_len);
    ರಚನೆ tpm_log_trailer *t = dest + ಗಾತ್ರ(*ನಮೂದು) + digest_len;
    t->eventdatasize = event_len;
    ಸ್ಮರಣೆ_ನಕಲು(t->event, event, event_len);

    tpm_state.log_area_ಕೊನೆಯ_ನಮೂದು = tpm_state.log_area_ಮುಂದಿನ_ನಮೂದು;
    tpm_state.log_area_ಮುಂದಿನ_ನಮೂದು += ಗಾತ್ರ_2;
    tpm_state.ನಮೂದು_ಎಣಿಕೆ++;

    ಹಿಂತಿರುಗಿ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ಸ್ಥಿರ TPMVersion TPM_ಆವೃತ್ತಿ;
ಸ್ಥಿರ u32 tpm20_pcr_selection_ಗಾತ್ರ;
ಸ್ಥಿರ ರಚನೆ tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
ರಚನೆ tpm_log_ನಮೂದು {
    ರಚನೆ tpm_log_ಶೀರ್ಷಿಕೆ hdr;
    u8 pad[ಗಾತ್ರ(ರಚನೆ tpm2_digest_ಮೌಲ್ಯಗಳು)
           + 8 * ಗಾತ್ರ(ರಚನೆ tpm2_digest_ಮೌಲ್ಯ)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
    ಶೂನ್ಯ (*hashfunc)(ಸ್ಥಿರಾಂಕ u8 *ದತ್ತಾಂಶ, u32 ಉದ್ದಳತೆ, u8 *hash);
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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_get_hash_buffersize(u16 hashAlg)
{
    ಚಿಹ್ನೆರಹಿತ i;

    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(hash_parameters); i++) {
        ಆದರೆ (hash_parameters[i].hashalg == hashAlg)
            ಹಿಂತಿರುಗಿ hash_parameters[i].hash_buffersize;
    }
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ u8
tpm20_hashalg_ಗಮ್ಯ_flag(u16 hashAlg)
{
    ಚಿಹ್ನೆರಹಿತ i;

    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(hash_parameters); i++) {
        ಆದರೆ (hash_parameters[i].hashalg == hashAlg)
            ಹಿಂತಿರುಗಿ hash_parameters[i].hashalg_flag;
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u16
tpm20_hashalg_flag_ಗಮ್ಯ_hashalg(u8 hashalg_flag)
{
    ಚಿಹ್ನೆರಹಿತ i;

    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(hash_parameters); i++) {
        ಆದರೆ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ಹಿಂತಿರುಗಿ hash_parameters[i].hashalg;
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
tpm20_hashalg_flag_ಗಮ್ಯ_name(u8 hashalg_flag)
{
    ಚಿಹ್ನೆರಹಿತ i;

    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(hash_parameters); i++) {
        ಆದರೆ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ಹಿಂತಿರುಗಿ hash_parameters[i].name;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ tpm2_hash_ದತ್ತಾಂಶ(u16 hashAlg, ಸ್ಥಿರಾಂಕ u8 *ದತ್ತಾಂಶ, u32 ದತ್ತಾಂಶ_len, u8 *hash)
{
    ಚಿಹ್ನೆರಹಿತ i;

    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(hash_parameters); i++) {
        ಆದರೆ (hash_parameters[i].hashalg == hashAlg) {
            ಆದರೆ (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(ದತ್ತಾಂಶ, ದತ್ತಾಂಶ_len, hash);
            } ಇಲ್ಲದಿದ್ದರೆ {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_write_EfiSpecIdEventStruct(ಶೂನ್ಯ)
{
    ಆದರೆ (!tpm20_pcr_selection)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ {
        ರಚನೆ TCG_EfiSpecIdEventStruct hdr;
        u8 pad[ಗಾತ್ರ(ರಚನೆ tpm_log_ನಮೂದು) + ಗಾತ್ರ(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.spec_ಆವೃತ್ತಿ_Minor = 0,
        .hdr.spec_ಆವೃತ್ತಿ_Major = 2,
        .hdr.specErrata = 2,
        .hdr.uintn_ಗಾತ್ರ = 2,
    };

    ರಚನೆ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ಶೂನ್ಯ *nsel, *ಕೊನೆ = (ಶೂನ್ಯ*)tpm20_pcr_selection + tpm20_pcr_selection_ಗಾತ್ರ;

    u32 ಎಣಿಕೆ_2, ಅಂಕೆ_Algs = 0;
    ಗಾಗಿ (ಎಣಿಕೆ_2 = 0; ಎಣಿಕೆ_2 < be32_ಗಮ್ಯ_cpu(tpm20_pcr_selection->ಎಣಿಕೆ_2); ಎಣಿಕೆ_2++) {
        u8 ಗಾತ್ರ_ದ_Select = sel->ಗಾತ್ರ_ದ_Select;

        nsel = (ಶೂನ್ಯ*)sel + ಗಾತ್ರ(*sel) + ಗಾತ್ರ_ದ_Select;
        ಆದರೆ (nsel > ಕೊನೆ)
            ನಿಲ್ಲಿಸು;

        ಆದರೆ (!ಗಾತ್ರ_ದ_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ಮುಂದುವರಿ;
        }

        ಪೂರ್ಣಾಂಕ hsize = tpm20_get_hash_buffersize(be16_ಗಮ್ಯ_cpu(sel->hashAlg));
        ಆದರೆ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_ಗಮ್ಯ_cpu(sel->hashAlg));
            ಹಿಂತಿರುಗಿ -1;
        }

        ಪೂರ್ಣಾಂಕ event_ಗಾತ್ರ = offsetof(ರಚನೆ TCG_EfiSpecIdEventStruct
                                  , digestSizes[ಎಣಿಕೆ_2+1]);
        ಆದರೆ (event_ಗಾತ್ರ > ಗಾತ್ರ(event) - ಗಾತ್ರ(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            ಹಿಂತಿರುಗಿ -1;
        }

        event.hdr.digestSizes[ಅಂಕೆ_Algs].algorithmId = be16_ಗಮ್ಯ_cpu(sel->hashAlg);
        event.hdr.digestSizes[ಅಂಕೆ_Algs].digest_ಗಾತ್ರ = hsize;
        ಅಂಕೆ_Algs++;

        sel = nsel;
    }

    ಆದರೆ (sel != ಕೊನೆ) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ಹಿಂತಿರುಗಿ -1;
    }

    event.hdr.ಸಂಖ್ಯೆ_ದ_Algorithms = ಅಂಕೆ_Algs;
    ಪೂರ್ಣಾಂಕ event_ಗಾತ್ರ = offsetof(ರಚನೆ TCG_EfiSpecIdEventStruct
                              , digestSizes[ಅಂಕೆ_Algs]);
    u8 *vendor_Info_ಗಾತ್ರ = (ಶೂನ್ಯ*)&event + event_ಗಾತ್ರ;
    *vendor_Info_ಗಾತ್ರ = 0;
    event_ಗಾತ್ರ += ಗಾತ್ರ(*vendor_Info_ಗಾತ್ರ);

    ರಚನೆ tpm_log_ನಮೂದು le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    ಹಿಂತಿರುಗಿ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_ಗಾತ್ರ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_build_digest(ರಚನೆ tpm_log_ನಮೂದು *le,
                   ಸ್ಥಿರಾಂಕ u8 *hashdata, u32 hashdata_len, ಪೂರ್ಣಾಂಕ bigEndian)
{
    ಆದರೆ (!tpm20_pcr_selection)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ಶೂನ್ಯ *nsel, *ಕೊನೆ = (ಶೂನ್ಯ*)tpm20_pcr_selection + tpm20_pcr_selection_ಗಾತ್ರ;
    ಶೂನ್ಯ *dest = le->hdr.digest + ಗಾತ್ರ(ರಚನೆ tpm2_digest_ಮೌಲ್ಯಗಳು);

    u32 ಎಣಿಕೆ_2, ಅಂಕೆ_Algs = 0;
    ಗಾಗಿ (ಎಣಿಕೆ_2 = 0; ಎಣಿಕೆ_2 < be32_ಗಮ್ಯ_cpu(tpm20_pcr_selection->ಎಣಿಕೆ_2); ಎಣಿಕೆ_2++) {
        u8 ಗಾತ್ರ_ದ_Select = sel->ಗಾತ್ರ_ದ_Select;

        nsel = (ಶೂನ್ಯ*)sel + ಗಾತ್ರ(*sel) + ಗಾತ್ರ_ದ_Select;
        ಆದರೆ (nsel > ಕೊನೆ)
            ನಿಲ್ಲಿಸು;

        /* PCR 0-7 unused? -- skip */
        ಆದರೆ (!ಗಾತ್ರ_ದ_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ಮುಂದುವರಿ;
        }

        ಪೂರ್ಣಾಂಕ hsize = tpm20_get_hash_buffersize(be16_ಗಮ್ಯ_cpu(sel->hashAlg));
        ಆದರೆ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_ಗಮ್ಯ_cpu(sel->hashAlg));
            ಹಿಂತಿರುಗಿ -1;
        }

        /* buffer size sanity check before writing */
        ರಚನೆ tpm2_digest_ಮೌಲ್ಯ *v = dest;
        ಆದರೆ (dest + ಗಾತ್ರ(*v) + hsize > (ಶೂನ್ಯ*)le + ಗಾತ್ರ(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            ಹಿಂತಿರುಗಿ -1;
        }

        ಆದರೆ (bigEndian)
            v->hashAlg = sel->hashAlg;
        ಇಲ್ಲದಿದ್ದರೆ
            v->hashAlg = be16_ಗಮ್ಯ_cpu(sel->hashAlg);

        tpm2_hash_ದತ್ತಾಂಶ(be16_ಗಮ್ಯ_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += ಗಾತ್ರ(*v) + hsize;
        sel = nsel;

        ಅಂಕೆ_Algs++;
    }

    ಆದರೆ (sel != ಕೊನೆ) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ಹಿಂತಿರುಗಿ -1;
    }

    ರಚನೆ tpm2_digest_ಮೌಲ್ಯಗಳು *v = (ಶೂನ್ಯ*)le->hdr.digest;
    ಆದರೆ (bigEndian)
        v->ಎಣಿಕೆ_2 = cpu_ಗಮ್ಯ_be32(ಅಂಕೆ_Algs);
    ಇಲ್ಲದಿದ್ದರೆ
        v->ಎಣಿಕೆ_2 = ಅಂಕೆ_Algs;

    ಹಿಂತಿರುಗಿ dest - (ಶೂನ್ಯ*)le->hdr.digest;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_build_digest(ರಚನೆ tpm_log_ನಮೂದು *le,
                   ಸ್ಥಿರಾಂಕ u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    ಹಿಂತಿರುಗಿ SHA1_BUFSIZE;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_build_digest_ನೇರ(ರಚನೆ tpm_log_ನಮೂದು *le, ಸ್ಥಿರಾಂಕ u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    ಸ್ಮರಣೆ_ನಕಲು(le->hdr.digest, sha1, SHA1_BUFSIZE);
    ಹಿಂತಿರುಗಿ SHA1_BUFSIZE;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_build_digest(ರಚನೆ tpm_log_ನಮೂದು *le, ಸ್ಥಿರಾಂಕ u8 *hashdata, u32 hashdata_len
                 , ಪೂರ್ಣಾಂಕ bigEndian)
{
    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        ಹಿಂತಿರುಗಿ tpm12_build_digest(le, hashdata, hashdata_len);
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        ಹಿಂತಿರುಗಿ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    ಹಿಂತಿರುಗಿ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_simple_cmd(u8 locty, u32 ordinal
               , ಪೂರ್ಣಾಂಕ param_ಗಾತ್ರ, u16 param, ಎಣಿಕೆ tpmDurationType ಗಮ್ಯ_t)
{
    ರಚನೆ {
        ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(req.trqh) + param_ಗಾತ್ರ),
        .trqh.ordinal = cpu_ಗಮ್ಯ_be32(ordinal),
        .param = param_ಗಾತ್ರ == 2 ? cpu_ಗಮ್ಯ_be16(param) : param,
    };
    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        req.trqh.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM_ಟ್ಯಾಗ್_RQU_CMD);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        req.trqh.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM2_ST_NO_SESSIONS);
        ನಿಲ್ಲಿಸು;
    }

    u8 obuffer[64];
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ *trsh = (ಶೂನ್ಯ*)obuffer;
    u32 obuffer_len = ಗಾತ್ರ(obuffer);
    memset(obuffer, 0x0, ಗಾತ್ರ(obuffer));

    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, ಗಮ್ಯ_t);
    ret = ret ? -1 : be32_ಗಮ್ಯ_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_getcapability(u32 capability, u32 property, u32 ಎಣಿಕೆ_2,
                    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ *rsp, u32 rsize)
{
    ರಚನೆ tpm2_req_getcapability trg = {
        .hdr.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trg)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_GetCapability),
        .capability = cpu_ಗಮ್ಯ_be32(capability),
        .property = cpu_ಗಮ್ಯ_be32(property),
        .propertycount = cpu_ಗಮ್ಯ_be32(ಎಣಿಕೆ_2),
    };

    u32 resp_ಗಾತ್ರ = rsize;
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_ಗಾತ್ರ,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_ಗಮ್ಯ_cpu(rsp->totlen)) ? -1 : be32_ಗಮ್ಯ_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_get_pcrbanks(ಶೂನ್ಯ)
{
    u8 ಬಫರ್[128];
    ರಚನೆ tpm2_res_getcapability *trg =
      (ರಚನೆ tpm2_res_getcapability *)&ಬಫರ್;

    ಪೂರ್ಣಾಂಕ ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  ಗಾತ್ರ(ಬಫರ್));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_ಗಮ್ಯ_cpu(trg->hdr.totlen);
    ಆದರೆ (resplen <= offsetof(ರಚನೆ tpm2_res_getcapability, ದತ್ತಾಂಶ))
        ಹಿಂತಿರುಗಿ -1;

    u32 ಗಾತ್ರ_2 = resplen - offsetof(ರಚನೆ tpm2_res_getcapability, ದತ್ತಾಂಶ);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    ಆದರೆ (ಗಾತ್ರ_2 < offsetof(ರಚನೆ tpml_pcr_selection, selections) +
               offsetof(ರಚನೆ tpms_pcr_selection, pcrSelect))
        ಹಿಂತಿರುಗಿ -1;

    tpm20_pcr_selection = malloc_ಮೇಲಿನ(ಗಾತ್ರ_2);
    ಆದರೆ (tpm20_pcr_selection) {
        ಸ್ಮರಣೆ_ನಕಲು(tpm20_pcr_selection, &trg->ದತ್ತಾಂಶ, ಗಾತ್ರ_2);
        tpm20_pcr_selection_ಗಾತ್ರ = ಗಾತ್ರ_2;
    } ಇಲ್ಲದಿದ್ದರೆ {
        warn_noalloc();
        ret = -1;
    }

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *ಸಕ್ರಿಯ_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *ಸಕ್ರಿಯ_pcrbanks = 0;

    ಆದರೆ (!tpm20_pcr_selection)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ಶೂನ್ಯ *ಕೊನೆ = (ಶೂನ್ಯ*)tpm20_pcr_selection + tpm20_pcr_selection_ಗಾತ್ರ;

    ಆಗ (1) {
        u8 ಗಾತ್ರ_ದ_Select = sel->ಗಾತ್ರ_ದ_Select;
        ಶೂನ್ಯ *nsel = (ಶೂನ್ಯ*)sel + ಗಾತ್ರ(*sel) + ಗಾತ್ರ_ದ_Select;
        ಆದರೆ (nsel > ಕೊನೆ)
            ಹಿಂತಿರುಗಿ 0;

        u16 hashalg = be16_ಗಮ್ಯ_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_ಗಮ್ಯ_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        ಚಿಹ್ನೆರಹಿತ i;
        ಗಾಗಿ (i = 0; i < ಗಾತ್ರ_ದ_Select; i++) {
            ಆದರೆ (sel->pcrSelect[i]) {
                *ಸಕ್ರಿಯ_pcrbanks |= hashalg_flag;
                ನಿಲ್ಲಿಸು;
            }
        }

        sel = nsel;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_ಗಣ_pcrbanks(u32 ಸಕ್ರಿಯ_banks)
{
    ರಚನೆ tpm2_req_pcr_allocate trpa = {
        .hdr.ಟ್ಯಾಗ್     = cpu_ಗಮ್ಯ_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_ಗಮ್ಯ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trpa.authblock)),
        .authblock = {
            .handle = cpu_ಗಮ್ಯ_be32(TPM2_RS_PW),
            .noncesize = cpu_ಗಮ್ಯ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ಗಮ್ಯ_be16(0),
        },
    };
    ರಚನೆ tpms_pcr_selection3 {
        u16 hashAlg;
        u8 ಗಾತ್ರ_ದ_Select;
        u8 pcrSelect[3];
    } tps[ARRAY_ಗಾತ್ರ(trpa.tpms_pcr_selections)];
    ಪೂರ್ಣಾಂಕ i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    ಆಗ (hashalg_flag) {
        ಆದರೆ ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_ಗಮ್ಯ_hashalg(hashalg_flag);

            ಆದರೆ (hashalg) {
                u8 ಮುಖವಾಡ = 0;
                tps[i].hashAlg = cpu_ಗಮ್ಯ_be16(hashalg);
                tps[i].ಗಾತ್ರ_ದ_Select = 3;

                ಆದರೆ (ಸಕ್ರಿಯ_banks & hashalg_flag)
                    ಮುಖವಾಡ = 0xff;

                tps[i].pcrSelect[0] = ಮುಖವಾಡ;
                tps[i].pcrSelect[1] = ಮುಖವಾಡ;
                tps[i].pcrSelect[2] = ಮುಖವಾಡ;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.ಎಣಿಕೆ_2 = cpu_ಗಮ್ಯ_be32(i);
    ಸ್ಮರಣೆ_ನಕಲು(trpa.tpms_pcr_selections, tps, i * ಗಾತ್ರ(tps[0]));
    trpa.hdr.totlen = cpu_ಗಮ್ಯ_be32(offsetof(ರಚನೆ tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * ಗಾತ್ರ(tps[0]));

    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);

    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_ಗಮ್ಯ_cpu(rsp.errcode);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tpm20_activate_pcrbanks(u32 ಸಕ್ರಿಯ_banks)
{
    ಪೂರ್ಣಾಂಕ ret = tpm20_ಗಣ_pcrbanks(ಸಕ್ರಿಯ_banks);
    ಆದರೆ (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (!ret)
        reset();
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_get_capability(u32 cap, u32 subcap, ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ *rsp, u32 rsize)
{
    ರಚನೆ tpm_req_getcap trgc = {
        .hdr.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM_ಟ್ಯಾಗ್_RQU_CMD),
        .hdr.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trgc)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM_ORD_GetCapability),
        .capArea = cpu_ಗಮ್ಯ_be32(cap),
        .sub_Cap_ಗಾತ್ರ = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trgc.subCap)),
        .subCap = cpu_ಗಮ್ಯ_be32(subcap)
    };
    u32 resp_ಗಾತ್ರ = rsize;
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_ಗಾತ್ರ,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_ಗಾತ್ರ != rsize) ? -1 : be32_ಗಮ್ಯ_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_read_permanent_flags(ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ buf_len)
{
    memset(buf, 0, buf_len);

    ರಚನೆ tpm_res_getcap_perm_flags pf;
    ಪೂರ್ಣಾಂಕ ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, ಗಾತ್ರ(pf));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;

    ಸ್ಮರಣೆ_ನಕಲು(buf, &pf.perm_flags, buf_len);

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_determine_timeouts(ಶೂನ್ಯ)
{
    ರಚನೆ tpm_res_getcap_timeouts timeouts;
    ಪೂರ್ಣಾಂಕ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, ಗಾತ್ರ(timeouts));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ರಚನೆ tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, ಗಾತ್ರ(durations));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < 3; i++)
        durations.durations[i] = be32_ಗಮ್ಯ_cpu(durations.durations[i]);

    ಗಾಗಿ (i = 0; i < 4; i++)
        timeouts.timeouts[i] = be32_ಗಮ್ಯ_cpu(timeouts.timeouts[i]);

    dprintf(DEBUG_tcg, "TCGBIOS: timeouts: %u %u %u %u\n",
            timeouts.timeouts[0],
            timeouts.timeouts[1],
            timeouts.timeouts[2],
            timeouts.timeouts[3]);

    dprintf(DEBUG_tcg, "TCGBIOS: durations: %u %u %u\n",
            durations.durations[0],
            durations.durations[1],
            durations.durations[2]);

    tpmhw_ಗಣ_timeouts(timeouts.timeouts, durations.durations);

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm20_ಗಣ_timeouts(ಶೂನ್ಯ)
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

    tpmhw_ಗಣ_timeouts(timeouts, durations);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_extend(ರಚನೆ tpm_log_ನಮೂದು *le, ಪೂರ್ಣಾಂಕ digest_len)
{
    ರಚನೆ tpm_req_extend tre = {
        .hdr.ಟ್ಯಾಗ್     = cpu_ಗಮ್ಯ_be16(TPM_ಟ್ಯಾಗ್_RQU_CMD),
        .hdr.totlen  = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(tre)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_ಗಮ್ಯ_be32(le->hdr.pcrindex),
    };
    ಸ್ಮರಣೆ_ನಕಲು(tre.digest, le->hdr.digest, ಗಾತ್ರ(tre.digest));

    ರಚನೆ tpm_rsp_extend rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.hdr.errcode)
        ಹಿಂತಿರುಗಿ -1;

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tpm20_extend(ರಚನೆ tpm_log_ನಮೂದು *le, ಪೂರ್ಣಾಂಕ digest_len)
{
    ರಚನೆ tpm2_req_extend tmp_tre = {
        .hdr.ಟ್ಯಾಗ್     = cpu_ಗಮ್ಯ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_ಗಮ್ಯ_be32(0),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_ಗಮ್ಯ_be32(le->hdr.pcrindex),
        .authblocksize = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_ಗಮ್ಯ_be32(TPM2_RS_PW),
            .noncesize = cpu_ಗಮ್ಯ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ಗಮ್ಯ_be16(0),
        },
    };
    u8 ಬಫರ್[ಗಾತ್ರ(tmp_tre) + ಗಾತ್ರ(le->pad)];
    ರಚನೆ tpm2_req_extend *tre = (ರಚನೆ tpm2_req_extend *)ಬಫರ್;

    ಸ್ಮರಣೆ_ನಕಲು(tre, &tmp_tre, ಗಾತ್ರ(tmp_tre));
    ಸ್ಮರಣೆ_ನಕಲು(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(tmp_tre) + digest_len);

    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.errcode)
        ಹಿಂತಿರುಗಿ -1;

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_extend(ರಚನೆ tpm_log_ನಮೂದು *le, ಪೂರ್ಣಾಂಕ digest_len)
{
    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        ಹಿಂತಿರುಗಿ tpm12_extend(le, digest_len);
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        ಹಿಂತಿರುಗಿ tpm20_extend(le, digest_len);
    }
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_stirrandom(ಶೂನ್ಯ)
{
    ರಚನೆ tpm2_req_stirrandom stir = {
        .hdr.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(stir)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_StirRandom),
        .ಗಾತ್ರ_2 = cpu_ಗಮ್ಯ_be16(ಗಾತ್ರ(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    ರಚನೆ tpm2_res_getrandom rsp;

    ಆದರೆ (buf_len > ಗಾತ್ರ(rsp.rnd.ಬಫರ್))
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ tpm2_req_getrandom trgr = {
        .hdr.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trgr)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_GetRandom),
        .ಬೈಟುಗಳು_Requested = cpu_ಗಮ್ಯ_be16(buf_len),
    };
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);

    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_MEDIUM);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.hdr.errcode)
        ret = -1;
    ಇಲ್ಲದಿದ್ದರೆ
        ಸ್ಮರಣೆ_ನಕಲು(buf, rsp.rnd.ಬಫರ್, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    ರಚನೆ tpm2_req_hierarchycontrol trh = {
        .hdr.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trh)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_ಗಮ್ಯ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trh.authblock)),
        .authblock = {
            .handle = cpu_ಗಮ್ಯ_be32(TPM2_RS_PW),
            .noncesize = cpu_ಗಮ್ಯ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ಗಮ್ಯ_be16(0),
        },
        .enable = cpu_ಗಮ್ಯ_be32(hierarchy),
        .state = state,
    };
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_MEDIUM);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_hierarchychangeauth(u8 auth[20])
{
    ರಚನೆ tpm2_req_hierarchychangeauth trhca = {
        .hdr.ಟ್ಯಾಗ್ = cpu_ಗಮ್ಯ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trhca)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_ಗಮ್ಯ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trhca.authblock)),
        .authblock = {
            .handle = cpu_ಗಮ್ಯ_be32(TPM2_RS_PW),
            .noncesize = cpu_ಗಮ್ಯ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ಗಮ್ಯ_be16(0),
        },
        .newAuth = {
            .ಗಾತ್ರ_2 = cpu_ಗಮ್ಯ_be16(ಗಾತ್ರ(trhca.newAuth.ಬಫರ್)),
        },
    };
    ಸ್ಮರಣೆ_ನಕಲು(trhca.newAuth.ಬಫರ್, auth, ಗಾತ್ರ(trhca.newAuth.ಬಫರ್));

    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_MEDIUM);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    ಹಿಂತಿರುಗಿ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ TPM_has_physical_presence;
u8 TPM_working VARLOW;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_is_working(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ CONFIG_TCGBIOS && TPM_working;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm_ಗಣ_failure(ಶೂನ್ಯ)
{
    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_ಗಣ_ತಾತ್ಕಾಲಿಕ_Deactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        ನಿಲ್ಲಿಸು;
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
tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(u32 pcrindex, u32 event_type,
                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *event, u32 event_ಉದ್ದಳತೆ,
                           ಸ್ಥಿರಾಂಕ u8 *hashdata, u32 hashdata_ಉದ್ದಳತೆ)
{
    ಆದರೆ (!tpm_is_working())
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ tpm_log_ನಮೂದು le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    ಪೂರ್ಣಾಂಕ digest_len = tpm_build_digest(&le, hashdata, hashdata_ಉದ್ದಳತೆ, 1);
    ಆದರೆ (digest_len < 0)
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ ret = tpm_extend(&le, digest_len);
    ಆದರೆ (ret) {
        tpm_ಗಣ_failure();
        ಹಿಂತಿರುಗಿ;
    }
    tpm_build_digest(&le, hashdata, hashdata_ಉದ್ದಳತೆ, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_ಉದ್ದಳತೆ);
}

// Add an EV_ACTION measurement to the list of measurements
ಸ್ಥಿರ ಶೂನ್ಯ
tpm_ಕೂಡಿಸು_action(u32 pcr_ಸೂಚ್ಯಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string)
{
    u32 len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string);
    tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(pcr_ಸೂಚ್ಯಂಕ, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tpm_ಕೂಡಿಸು_event_separators(ಶೂನ್ಯ)
{
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcr_ಸೂಚ್ಯಂಕ;
    ಗಾಗಿ (pcr_ಸೂಚ್ಯಂಕ = 0; pcr_ಸೂಚ್ಯಂಕ <= 7; pcr_ಸೂಚ್ಯಂಕ++)
        tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(pcr_ಸೂಚ್ಯಂಕ, EV_SEPARATOR,
                                   (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)evt_separator,
                                   ಗಾತ್ರ(evt_separator),
                                   evt_separator,
                                   ಗಾತ್ರ(evt_separator));
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm_smbios_measure(ಶೂನ್ಯ)
{
    ರಚನೆ pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    ಶೂನ್ಯ *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    ಆದರೆ (!smbios_tables)
        ಹಿಂತಿರುಗಿ;

    sha1((ಸ್ಥಿರಾಂಕ u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(1,
                               EV_EVENT_ಟ್ಯಾಗ್,
                               (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)&pcctes, ಗಾತ್ರ(pcctes),
                               (u8 *)&pcctes, ಗಾತ್ರ(pcctes));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_assert_physical_presence(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (!ret)
        ಹಿಂತಿರುಗಿ 0;

    ರಚನೆ tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ಅಕ್ಷರ *)&pf, ಗಾತ್ರ(pf));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;

    /* check if hardware physical presence is supported */
    ಆದರೆ (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        ಹಿಂತಿರುಗಿ 0;
    }

    ಆದರೆ (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_ಬೀಗ]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        ಹಿಂತಿರುಗಿ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_startup(ಶೂನ್ಯ)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    ಪೂರ್ಣಾಂಕ ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    ಆದರೆ (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_Self_Test_ತುಂಬಿದ,
                         0, 0, TPM_DURATION_TYPE_LONG);
    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_Reset_Establishment_ಬಿಟ್,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret && ret != TPM_BAD_LOCALITY)
        ಹೋಗು err_exit;

    ಹಿಂತಿರುಗಿ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ಗಣ_failure();
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_startup(ಶೂನ್ಯ)
{
    tpm20_ಗಣ_timeouts();

    ಪೂರ್ಣಾಂಕ ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    ಆದರೆ (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ret = tpm20_get_pcrbanks();
    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ಹಿಂತಿರುಗಿ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ಗಣ_failure();
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm_startup(ಶೂನ್ಯ)
{
    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        ಹಿಂತಿರುಗಿ tpm12_startup();
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        ಹಿಂತಿರುಗಿ tpm20_startup();
    }
    ಹಿಂತಿರುಗಿ -1;
}

ಶೂನ್ಯ
tpm_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ;

    ಪೂರ್ಣಾಂಕ ret = tpm_tpm2_probe();
    ಆದರೆ (ret) {
        ret = tpm_tcpa_probe();
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ;
    }

    TPM_ಆವೃತ್ತಿ = tpmhw_probe();
    ಆದರೆ (TPM_ಆವೃತ್ತಿ == TPM_ಆವೃತ್ತಿ_NONE)
        ಹಿಂತಿರುಗಿ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_ಆವೃತ್ತಿ == TPM_ಆವೃತ್ತಿ_1_2) ? "1.2" : "2");

    TPM_working = 1;

    ಆದರೆ (runningOnXen())
        ಹಿಂತಿರುಗಿ;

    ret = tpm_startup();
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;

    tpm_smbios_measure();
    tpm_ಕೂಡಿಸು_action(2, "Start Option ROM Scan");
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm20_prepboot(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ret = tpm20_stirrandom();
    ಆದರೆ (ret)
         ಹೋಗು err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], ಗಾತ್ರ(auth));
    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ಹಿಂತಿರುಗಿ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ಗಣ_failure();
}

ಶೂನ್ಯ
tpm_prepboot(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ;

    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        ಆದರೆ (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_ಬೀಗ, TPM_DURATION_TYPE_SHORT);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        tpm20_prepboot();
        ನಿಲ್ಲಿಸು;
    }

    tpm_ಕೂಡಿಸು_action(4, "Calling INT 19h");
    tpm_ಕೂಡಿಸು_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
ಶೂನ್ಯ
tpm_option_rom(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *addr, u32 len)
{
    ಆದರೆ (!tpm_is_working())
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = ಗಾತ್ರ(u16) + ಗಾತ್ರ(u16) + SHA1_BUFSIZE,
    };
    sha1((ಸ್ಥಿರಾಂಕ u8 *)addr, len, pcctes.digest);
    tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(2,
                               EV_EVENT_ಟ್ಯಾಗ್,
                               (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)&pcctes, ಗಾತ್ರ(pcctes),
                               (u8 *)&pcctes, ಗಾತ್ರ(pcctes));
}

ಶೂನ್ಯ
tpm_ಕೂಡಿಸು_bcv(u32 bootdrv, ಸ್ಥಿರಾಂಕ u8 *addr, u32 ಉದ್ದಳತೆ)
{
    ಆದರೆ (!tpm_is_working())
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (ಉದ್ದಳತೆ < 0x200)
        ಹಿಂತಿರುಗಿ;

    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string = "Booting BCV device 00h (Floppy)";
    ಆದರೆ (bootdrv == 0x80)
        string = "Booting BCV device 80h (HDD)";
    tpm_ಕೂಡಿಸು_action(4, string);

    /* specs: see section 'Hard Disk Device or Hard Disk-Like Devices' */
    /* equivalent to: dd if=/dev/hda ibs=1 count=440 | sha1sum */
    string = "MBR";
    tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(4, EV_IPL,
                               string, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string),
                               addr, 0x1b8);

    /* equivalent to: dd if=/dev/hda ibs=1 count=72 skip=440 | sha1sum */
    string = "MBR PARTITION_TABLE";
    tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(5, EV_IPL_ವಿಭಜನೆ_ದತ್ತಾಂಶ,
                               string, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string),
                               addr + 0x1b8, 0x48);
}

ಶೂನ್ಯ
tpm_ಕೂಡಿಸು_cdrom(u32 bootdrv, ಸ್ಥಿರಾಂಕ u8 *addr, u32 ಉದ್ದಳತೆ)
{
    ಆದರೆ (!tpm_is_working())
        ಹಿಂತಿರುಗಿ;

    tpm_ಕೂಡಿಸು_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string = "EL TORITO IPL";
    tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(4, EV_IPL,
                               string, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string),
                               addr, ಉದ್ದಳತೆ);
}

ಶೂನ್ಯ
tpm_ಕೂಡಿಸು_cdrom_catalog(ಸ್ಥಿರಾಂಕ u8 *addr, u32 ಉದ್ದಳತೆ)
{
    ಆದರೆ (!tpm_is_working())
        ಹಿಂತಿರುಗಿ;

    tpm_ಕೂಡಿಸು_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string = "BOOT CATALOG";
    tpm_ಕೂಡಿಸು_measurement_ಗಮ್ಯ_log(5, EV_IPL_ವಿಭಜನೆ_ದತ್ತಾಂಶ,
                               string, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string),
                               addr, ಉದ್ದಳತೆ);
}

ಶೂನ್ಯ
tpm_s3_resume(ಶೂನ್ಯ)
{
    ಆದರೆ (!tpm_is_working())
        ಹಿಂತಿರುಗಿ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    ಪೂರ್ಣಾಂಕ ret = -1;

    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        ಆದರೆ (ret)
            ಹೋಗು err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ (ret)
        ಹೋಗು err_exit;

    ಹಿಂತಿರುಗಿ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ಗಣ_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *ಒಳಹರಿವು_buf32(ರಚನೆ bregs *regs)
{
    ಹಿಂತಿರುಗಿ MAKE_FLATPTR(regs->es, regs->di);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *output_buf32(ರಚನೆ bregs *regs)
{
    ಹಿಂತಿರುಗಿ MAKE_FLATPTR(regs->ds, regs->si);
}

ಸ್ಥಿರ u32
hash_log_extend(ರಚನೆ pcpes *pcpes, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *hashdata, u32 hashdata_ಉದ್ದಳತೆ
                , ಶೂನ್ಯ *event, ಪೂರ್ಣಾಂಕ extend)
{
    ಆದರೆ (pcpes->pcrindex >= 24)
        ಹಿಂತಿರುಗಿ TCG_INVALID_ಒಳಹರಿವು_PARA;
    ಆದರೆ (hashdata)
        sha1(hashdata, hashdata_ಉದ್ದಳತೆ, pcpes->digest);

    ರಚನೆ tpm_log_ನಮೂದು le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    ಪೂರ್ಣಾಂಕ digest_len = tpm12_build_digest_ನೇರ(&le, pcpes->digest);
    ಆದರೆ (digest_len < 0)
        ಹಿಂತಿರುಗಿ TCG_GENERAL_ERROR;
    ಆದರೆ (extend) {
        ಪೂರ್ಣಾಂಕ ret = tpm_extend(&le, digest_len);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_ನೇರ(&le, pcpes->digest);
    ಪೂರ್ಣಾಂಕ ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ TCG_PC_LOGOVERFLOW;
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32
hash_log_extend_event_int(ಸ್ಥಿರಾಂಕ ರಚನೆ hleei_short *hleei_s,
                          ರಚನೆ hleeo *hleeo)
{
    u32 rc = 0;
    ರಚನೆ hleei_long *hleei_l = (ರಚನೆ hleei_long *)hleei_s;
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *logdataptr;
    u32 logdatalen;
    ರಚನೆ pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    ಆಯ್ಕೆ (hleei_s->ipblength) {
    ಸ್ಥಿತಿ ಗಾತ್ರ(ರಚನೆ hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ ಗಾತ್ರ(ರಚನೆ hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
        /* bad input block */
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಹೋಗು err_exit;
    }

    pcpes = (ರಚನೆ pcpes *)logdataptr;

    ಆದರೆ (pcpes->pcrindex != pcrindex
        || logdatalen != ಗಾತ್ರ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಹೋಗು err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    ಆದರೆ (rc)
        ಹೋಗು err_exit;

    hleeo->opblength = ಗಾತ್ರ(ರಚನೆ hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.ನಮೂದು_ಎಣಿಕೆ;
    ಸ್ಮರಣೆ_ನಕಲು(hleeo->digest, pcpes->digest, ಗಾತ್ರ(hleeo->digest));

err_exit:
    ಆದರೆ (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32
pass_through_ಗಮ್ಯ_tpm_int(ರಚನೆ pttti *pttti, ರಚನೆ pttto *pttto)
{
    u32 rc = 0;
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ *trh = (ಶೂನ್ಯ*)pttti->tpmopin;

    ಆದರೆ (pttti->ipblength < ಗಾತ್ರ(ರಚನೆ pttti) + ಗಾತ್ರ(*trh)
        || pttti->ipblength != ಗಾತ್ರ(ರಚನೆ pttti) + be32_ಗಮ್ಯ_cpu(trh->totlen)
        || pttti->opblength < ಗಾತ್ರ(ರಚನೆ pttto)) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಹೋಗು err_exit;
    }

    u16 ಟ್ಯಾಗ್ = be16_ಗಮ್ಯ_cpu(trh->ಟ್ಯಾಗ್);

    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        ಆದರೆ (ಟ್ಯಾಗ್ != TPM_ಟ್ಯಾಗ್_RQU_CMD && ಟ್ಯಾಗ್ != TPM_ಟ್ಯಾಗ್_RQU_AUTH1_CMD
            && ಟ್ಯಾಗ್ != TPM_ಟ್ಯಾಗ್_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
            ಹೋಗು err_exit;
        }
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        ಆದರೆ (ಟ್ಯಾಗ್ != TPM2_ST_NO_SESSIONS && ಟ್ಯಾಗ್ != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
            ಹೋಗು err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(ರಚನೆ pttto, tpmopout);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    ಆದರೆ (ret) {
        rc = TCG_FATAL_COM_ERROR;
        ಹೋಗು err_exit;
    }

    pttto->opblength = offsetof(ರಚನೆ pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    ಆದರೆ (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32
shutdown_preboot_interface(ಶೂನ್ಯ)
{
    TPM_interface_shutdown = 1;
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32
hash_log_event_int(ಸ್ಥಿರಾಂಕ ರಚನೆ hlei *hlei, ರಚನೆ hleo *hleo)
{
    u32 rc = 0;
    u16 ಗಾತ್ರ_2;
    ರಚನೆ pcpes *pcpes;

    ಗಾತ್ರ_2 = hlei->ipblength;
    ಆದರೆ (ಗಾತ್ರ_2 != ಗಾತ್ರ(*hlei)) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಹೋಗು err_exit;
    }

    pcpes = (ರಚನೆ pcpes *)hlei->logdataptr;

    ಆದರೆ (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != ಗಾತ್ರ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ಒಳಹರಿವು_PARA;
        ಹೋಗು err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    ಆದರೆ (rc)
        ಹೋಗು err_exit;

    /* updating the log was fine */
    hleo->opblength = ಗಾತ್ರ(ರಚನೆ hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.ನಮೂದು_ಎಣಿಕೆ;

err_exit:
    ಆದರೆ (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32
hash_all_int(ಸ್ಥಿರಾಂಕ ರಚನೆ hai *hai, u8 *hash)
{
    ಆದರೆ (hai->ipblength != ಗಾತ್ರ(ರಚನೆ hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        ಹಿಂತಿರುಗಿ TCG_INVALID_ಒಳಹರಿವು_PARA;

    sha1((ಸ್ಥಿರಾಂಕ u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32
tss_int(ರಚನೆ ti *ti, ರಚನೆ ಗಮ್ಯ *ಗಮ್ಯ)
{
    ಗಮ್ಯ->opblength = ಗಾತ್ರ(ರಚನೆ ಗಮ್ಯ);
    ಗಮ್ಯ->reserved  = 0;

    ಹಿಂತಿರುಗಿ TCG_PC_UNSUPPORTED;
}

ಸ್ಥಿರ u32
compact_hash_log_extend_event_int(u8 *ಬಫರ್,
                                  u32 info,
                                  u32 ಉದ್ದಳತೆ,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    ರಚನೆ pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = ಗಾತ್ರ(info),
    };
    u32 rc = hash_log_extend(&pcpes, ಬಫರ್, ಉದ್ದಳತೆ, &info, 1);
    ಆದರೆ (rc)
        ಹಿಂತಿರುಗಿ rc;

    *edx_ptr = tpm_state.ನಮೂದು_ಎಣಿಕೆ;
    ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ VISIBLE32FLAT
tpm_ಅಡಚಣೆ_handler32(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ;

    ಗಣ_cf(regs, 0);

    ಆದರೆ (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        ಹಿಂತಿರುಗಿ;
    }

    ಆಯ್ಕೆ ((ಎಣಿಕೆ irq_ids)regs->al) {
    ಸ್ಥಿತಿ TCG_ಸ್ಥಿತಿ_Check:
        ಆದರೆ (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } ಇಲ್ಲದಿದ್ದರೆ {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_ಆವೃತ್ತಿ_MAJOR;
            regs->cl = TCG_ಆವೃತ್ತಿ_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_ಆರಂಭ_ವಿಳಾಸ;
            regs->edi = (u32)tpm_state.log_area_ಕೊನೆಯ_ನಮೂದು;
        }
        ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (ರಚನೆ hleei_short *)ಒಳಹರಿವು_buf32(regs),
                  (ರಚನೆ hleeo *)output_buf32(regs));
        ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TCG_Pass_Through_ಗಮ್ಯ_TPM:
        regs->eax =
            pass_through_ಗಮ್ಯ_tpm_int((ರಚನೆ pttti *)ಒಳಹರಿವು_buf32(regs),
                                    (ರಚನೆ pttto *)output_buf32(regs));
        ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TCG_HashLogEvent:
        regs->eax = hash_log_event_int((ರಚನೆ hlei*)ಒಳಹರಿವು_buf32(regs),
                                       (ರಚನೆ hleo*)output_buf32(regs));
        ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TCG_HashAll:
        regs->eax =
            hash_all_int((ರಚನೆ hai*)ಒಳಹರಿವು_buf32(regs),
                          (u8 *)output_buf32(regs));
        ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TCG_TSS:
        regs->eax = tss_int((ರಚನೆ ti*)ಒಳಹರಿವು_buf32(regs),
                            (ರಚನೆ ಗಮ್ಯ*)output_buf32(regs));
        ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)ಒಳಹರಿವು_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
        ಗಣ_cf(regs, 1);
    }

    ಹಿಂತಿರುಗಿ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u8 tpm_ppi_ಸಂಕೇತ;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_read_has_owner(ಪೂರ್ಣಾಂಕ *has_owner)
{
    ರಚನೆ tpm_res_getcap_ownerauth oauth;
    ಪೂರ್ಣಾಂಕ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, ಗಾತ್ರ(oauth));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;

    *has_owner = oauth.flag;

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_enable_tpm(ಪೂರ್ಣಾಂಕ enable, ಪೂರ್ಣಾಂಕ verbose)
{
    ರಚನೆ tpm_permanent_flags pf;
    ಪೂರ್ಣಾಂಕ ret = tpm12_read_permanent_flags((ಅಕ್ಷರ *)&pf, ಗಾತ್ರ(pf));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        ಹಿಂತಿರುಗಿ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret) {
        ಆದರೆ (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        ಇಲ್ಲದಿದ್ದರೆ
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_activate_tpm(ಪೂರ್ಣಾಂಕ activate, ಪೂರ್ಣಾಂಕ allow_reset, ಪೂರ್ಣಾಂಕ verbose)
{
    ರಚನೆ tpm_permanent_flags pf;
    ಪೂರ್ಣಾಂಕ ret = tpm12_read_permanent_flags((ಅಕ್ಷರ *)&pf, ಗಾತ್ರ(pf));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ (pf.flags[PERM_FLAG_IDX_DISABLE])
        ಹಿಂತಿರುಗಿ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_Physical_ಗಣ_Deactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಆದರೆ (activate && allow_reset) {
        ಆದರೆ (verbose) {
            ಬರೆ("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_enable_activate(ಪೂರ್ಣಾಂಕ allow_reset, ಪೂರ್ಣಾಂಕ verbose)
{
    ಪೂರ್ಣಾಂಕ ret = tpm12_enable_tpm(1, verbose);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಹಿಂತಿರುಗಿ tpm12_activate_tpm(1, allow_reset, verbose);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_force_clear(ಪೂರ್ಣಾಂಕ enable_activate_before, ಪೂರ್ಣಾಂಕ enable_activate_after,
                  ಪೂರ್ಣಾಂಕ verbose)
{
    ಪೂರ್ಣಾಂಕ has_owner;
    ಪೂರ್ಣಾಂಕ ret = tpm12_read_has_owner(&has_owner);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!has_owner) {
        ಆದರೆ (verbose)
            ಬರೆ("TPM does not have an owner.\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    ಆದರೆ (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        ಆದರೆ (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            ಹಿಂತಿರುಗಿ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಆದರೆ (!enable_activate_after) {
        ಆದರೆ (verbose)
            ಬರೆ("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    ಹಿಂತಿರುಗಿ tpm12_enable_activate(1, verbose);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_ಗಣ_owner_install(ಪೂರ್ಣಾಂಕ allow, ಪೂರ್ಣಾಂಕ verbose)
{
    ಪೂರ್ಣಾಂಕ has_owner;
    ಪೂರ್ಣಾಂಕ ret = tpm12_read_has_owner(&has_owner);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (has_owner) {
        ಆದರೆ (verbose)
            ಬರೆ("Must first remove owner.\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    ರಚನೆ tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ಅಕ್ಷರ *)&pf, ಗಾತ್ರ(pf));
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        ಆದರೆ (verbose)
            ಬರೆ("TPM must first be enable.\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ಗಣ_Owner_Install,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಆದರೆ (verbose)
        ಬರೆ("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_process_cfg(tpm_ppi_ಸಂಕೇತ ಸಂದೇಶ_ಸಂಕೇತ, ಪೂರ್ಣಾಂಕ verbose)
{
    ಪೂರ್ಣಾಂಕ ret = 0;

    ಆಯ್ಕೆ (ಸಂದೇಶ_ಸಂಕೇತ) {
        ಸ್ಥಿತಿ TPM_PPI_OP_NOOP: /* no-op */
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_ಗಣ_OWNERINSTALL_TRUE:
            ret = tpm12_ಗಣ_owner_install(1, verbose);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_ಗಣ_OWNERINSTALL_FALSE:
            ret = tpm12_ಗಣ_owner_install(0, verbose);
            ನಿಲ್ಲಿಸು;

        ಪೂರ್ವನಿಯೋಜಿತ:
            ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ (ret)
        ಬರೆ("Op %d: An error occurred: 0x%x\n", ಸಂದೇಶ_ಸಂಕೇತ, ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_clearcontrol(u8 disable, ಪೂರ್ಣಾಂಕ verbose)
{
    ರಚನೆ tpm2_req_clearcontrol trc = {
        .hdr.ಟ್ಯಾಗ್     = cpu_ಗಮ್ಯ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trc)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_ಗಮ್ಯ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trc.authblock)),
        .authblock = {
            .handle = cpu_ಗಮ್ಯ_be32(TPM2_RS_PW),
            .noncesize = cpu_ಗಮ್ಯ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ಗಮ್ಯ_be16(0),
        },
        .disable = disable,
    };
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_SHORT);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_clear(ಶೂನ್ಯ)
{
    ರಚನೆ tpm2_req_clear trq = {
        .hdr.ಟ್ಯಾಗ್     = cpu_ಗಮ್ಯ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trq)),
        .hdr.ordinal = cpu_ಗಮ್ಯ_be32(TPM2_CC_Clear),
        .authhandle = cpu_ಗಮ್ಯ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(trq.authblock)),
        .authblock = {
            .handle = cpu_ಗಮ್ಯ_be32(TPM2_RS_PW),
            .noncesize = cpu_ಗಮ್ಯ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ಗಮ್ಯ_be16(0),
        },
    };
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ rsp;
    u32 resp_ಉದ್ದಳತೆ = ಗಾತ್ರ(rsp);
    ಪೂರ್ಣಾಂಕ ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_ಉದ್ದಳತೆ,
                             TPM_DURATION_TYPE_MEDIUM);
    ಆದರೆ (ret || resp_ಉದ್ದಳತೆ != ಗಾತ್ರ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_process_cfg(tpm_ppi_ಸಂಕೇತ ಸಂದೇಶ_ಸಂಕೇತ, ಪೂರ್ಣಾಂಕ verbose)
{
    ಪೂರ್ಣಾಂಕ ret = 0;

    ಆಯ್ಕೆ (ಸಂದೇಶ_ಸಂಕೇತ) {
        ಸ್ಥಿತಿ TPM_PPI_OP_NOOP: /* no-op */
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            ಆದರೆ (!ret)
                 ret = tpm20_clear();
            ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ (ret)
        ಬರೆ("Op %d: An error occurred: 0x%x\n", ಸಂದೇಶ_ಸಂಕೇತ, ret);

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm12_get_tpm_state(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ state = 0;
    ರಚನೆ tpm_permanent_flags pf;
    ಪೂರ್ಣಾಂಕ has_owner;

    ಆದರೆ (tpm12_read_permanent_flags((ಅಕ್ಷರ *)&pf, ಗಾತ್ರ(pf)) ||
        tpm12_read_has_owner(&has_owner))
        ಹಿಂತಿರುಗಿ ~0;

    ಆದರೆ (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    ಆದರೆ (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ಸಕ್ರಿಯ;

    ಆದರೆ (has_owner) {
        state |= TPM_STATE_OWNED;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    ಹಿಂತಿರುಗಿ state;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm12_show_tpm_menu(ಪೂರ್ಣಾಂಕ state, ಪೂರ್ಣಾಂಕ ಮುಂದಿನ_scancodes[7])
{
    ಪೂರ್ಣಾಂಕ i = 0;

    ಬರೆ("\nThe current state of the TPM is:\n");

    ಆದರೆ (state & TPM_STATE_ENABLED)
        ಬರೆ("  Enabled");
    ಇಲ್ಲದಿದ್ದರೆ
        ಬರೆ("  Disabled");

    ಆದರೆ (state & TPM_STATE_ಸಕ್ರಿಯ)
        ಬರೆ(" and active\n");
    ಇಲ್ಲದಿದ್ದರೆ
        ಬರೆ(" and deactivated\n");

    ಆದರೆ (state & TPM_STATE_OWNED)
        ಬರೆ("  Ownership has been taken\n");
    ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ("  Ownership has not been taken\n");
        ಆದರೆ (state & TPM_STATE_OWNERINSTALL)
            ಬರೆ("  A user can take ownership of the TPM\n");
        ಇಲ್ಲದಿದ್ದರೆ
            ಬರೆ("  Taking ownership of the TPM has been disabled\n");
    }

    ಆದರೆ ((state & (TPM_STATE_ENABLED | TPM_STATE_ಸಕ್ರಿಯ)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ಸಕ್ರಿಯ)) {
        ಬರೆ("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    ಬರೆ("\nAvailable options are:\n");
    ಆದರೆ (state & TPM_STATE_ENABLED) {
        ಬರೆ(" d. Disable the TPM\n");
        ಮುಂದಿನ_scancodes[i++] = 32;

        ಆದರೆ (state & TPM_STATE_ಸಕ್ರಿಯ) {
            ಬರೆ(" v. Deactivate the TPM\n");
            ಮುಂದಿನ_scancodes[i++] = 47;

            ಆದರೆ (state & TPM_STATE_OWNERINSTALL) {
                ಬರೆ(" p. Prevent installation of an owner\n");
                ಮುಂದಿನ_scancodes[i++] = 25;
            } ಇಲ್ಲದಿದ್ದರೆ {
                ಬರೆ(" s. Allow installation of an owner\n");
                ಮುಂದಿನ_scancodes[i++] = 31;
            }
        } ಇಲ್ಲದಿದ್ದರೆ {
            ಬರೆ(" a. Activate the TPM\n");
            ಮುಂದಿನ_scancodes[i++] = 30;
        }

    } ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ(" e. Enable the TPM\n");
        ಮುಂದಿನ_scancodes[i++] = 18;
    }

    ಆದರೆ (state & TPM_STATE_OWNED) {
        ಬರೆ(" c. Clear ownership\n");
        ಮುಂದಿನ_scancodes[i++] = 46;
    }

    ಮುಂದಿನ_scancodes[i++] = 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm12_menu(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ scancode, ಮುಂದಿನ_scancodes[7];
    tpm_ppi_ಸಂಕೇತ ಸಂದೇಶ_ಸಂಕೇತ;
    ಪೂರ್ಣಾಂಕ state = 0, i;
    ಪೂರ್ಣಾಂಕ waitkey;

    ಬರೆ("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    ಗಾಗಿ (;;) {
        ಆದರೆ ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, ಮುಂದಿನ_scancodes);
        } ಇಲ್ಲದಿದ್ದರೆ {
            ಬರೆ("TPM is not working correctly.\n");
            ಹಿಂತಿರುಗಿ;
        }

        ಬರೆ("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_NOOP;

        waitkey = 1;

        ಆಗ (waitkey) {
            ಆಗ ((scancode = get_keystroke(1000)) == ~0)
                ;

            ಆಯ್ಕೆ (scancode) {
            ಸ್ಥಿತಿ 1:
                // ESC
                reset();
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 18: /* e. enable */
                ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_ENABLE;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 32: /* d. disable */
                ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_DISABLE;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 30: /* a. activate */
                ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_ACTIVATE;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 47: /* v. deactivate */
                ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_DEACTIVATE;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 46: /* c. clear owner */
                ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_CLEAR;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 25: /* p. prevent ownerinstall */
                ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_ಗಣ_OWNERINSTALL_FALSE;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 31: /* s. allow ownerinstall */
                ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_ಗಣ_OWNERINSTALL_TRUE;
                ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:
                ಮುಂದುವರಿ;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            ಗಾಗಿ (i = 0; i < ಗಾತ್ರ(ಮುಂದಿನ_scancodes); i++) {
                ಆದರೆ (ಮುಂದಿನ_scancodes[i] == 0)
                    ನಿಲ್ಲಿಸು;

                ಆದರೆ (ಮುಂದಿನ_scancodes[i] == scancode) {
                    tpm12_process_cfg(ಸಂದೇಶ_ಸಂಕೇತ, 1);
                    waitkey = 0;
                    ನಿಲ್ಲಿಸು;
                }
            }
        }
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tpm20_menu_change_ಸಕ್ರಿಯ_pcrbanks(ಶೂನ್ಯ)
{
    u8 ಸಕ್ರಿಯ_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &ಸಕ್ರಿಯ_banks);

    u8 activate_banks = ಸಕ್ರಿಯ_banks;

    ಆಗ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        ಬರೆ("\nToggle active PCR banks by pressing number key\n\n");

        ಆಗ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hashname = tpm20_hashalg_flag_ಗಮ್ಯ_name(flag);

            i++;
            ಆದರೆ (hashname) {
                ಬರೆ("  %d: %s", i, hashname);
                ಆದರೆ (activate_banks & hashalg_flag)
                    ಬರೆ(" (enabled)");
                ಬರೆ("\n");
            }

            hashalg_flag <<= 1;
        }
        ಬರೆ("\n"
               "ESC: return to previous menu without changes\n");
        ಆದರೆ (activate_banks)
            ಬರೆ("A  : activate selection\n");

        u8 flagnum;
        ಪೂರ್ಣಾಂಕ show = 0;
        ಆಗ (!show) {
            ಪೂರ್ಣಾಂಕ scancode = get_keystroke(1000);

            ಆಯ್ಕೆ (scancode) {
            ಸ್ಥಿತಿ ~0:
                ಮುಂದುವರಿ;
            ಸ್ಥಿತಿ 1: /* ESC */
                ಬರೆ("\n");
                ಹಿಂತಿರುಗಿ -1;
            ಸ್ಥಿತಿ 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                ಆದರೆ (flagnum > i)
                    ಮುಂದುವರಿ;
                ಆದರೆ (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 30: /* a */
                ಆದರೆ (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
tpm20_menu(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ಪರಿಶೀಲನೆ_ಸಂಕೇತ;
    tpm_ppi_ಸಂಕೇತ ಸಂದೇಶ_ಸಂಕೇತ;

    ಗಾಗಿ (;;) {
        ಬರೆ("1. Clear TPM\n");
        ಬರೆ("2. Change active PCR banks\n");

        ಬರೆ("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_NOOP;

        ಆಗ ((ಪರಿಶೀಲನೆ_ಸಂಕೇತ = get_keystroke(1000)) == ~0)
            ;

        ಆಯ್ಕೆ (ಪರಿಶೀಲನೆ_ಸಂಕೇತ) {
        ಸ್ಥಿತಿ 1:
            // ESC
            reset();
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 2:
            ಸಂದೇಶ_ಸಂಕೇತ = TPM_PPI_OP_CLEAR;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 3:
            tpm20_menu_change_ಸಕ್ರಿಯ_pcrbanks();
            ಮುಂದುವರಿ;
        ಪೂರ್ವನಿಯೋಜಿತ:
            ಮುಂದುವರಿ;
        }

        tpm20_process_cfg(ಸಂದೇಶ_ಸಂಕೇತ, 0);
    }
}

ಶೂನ್ಯ
tpm_menu(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ;

    ಆಗ (get_keystroke(0) >= 0)
        ;
    wait_ಎಳೆಗಳು();

    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        tpm12_menu();
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        tpm20_menu();
        ನಿಲ್ಲಿಸು;
    }
}

ಪೂರ್ಣಾಂಕ
tpm_can_show_menu(ಶೂನ್ಯ)
{
    ಆಯ್ಕೆ (TPM_ಆವೃತ್ತಿ) {
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_1_2:
        ಹಿಂತಿರುಗಿ tpm_is_working() && TPM_has_physical_presence;
    ಸ್ಥಿತಿ TPM_ಆವೃತ್ತಿ_2:
        ಹಿಂತಿರುಗಿ tpm_is_working();
    }
    ಹಿಂತಿರುಗಿ 0;
}
