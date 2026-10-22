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

#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // cpu_to_*
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_TCGBIOS
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // MAKE_FLATPTR
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // runningOnXen
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/tpm_drivers.h" // tpm_drivers[]
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "sha.h" // sha1, sha256, ...
#ਅਨ੍ਤਰ੍ਭਾਵ "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#ਅਨ੍ਤਰ੍ਭਾਵ "std/smbios.h" // struct smbios_21_entry_point
#ਅਨ੍ਤਰ੍ਭਾਵ "std/tcg.h" // TCG_PC_LOGOVERFLOW
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // checksum
#ਅਨ੍ਤਰ੍ਭਾਵ "tcgbios.h"// tpm_*, prototypes
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // printf, get_keystroke
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // wait_threads, reset
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

ਬਣਤਰ {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_ਲੰਮਾਈ;

    /* start address of TCPA log buffer */
    u8 *          log_area_ਸ਼ੁਰੂ_ਪਤਾ;

    /* number of log entries written */
    u32           ਇੰਦਰਾਜ_ਗਿਣਤੀ;

    /* address to write next log entry to */
    u8 *          log_area_ਅਗਲਾ_ਇੰਦਰਾਜ;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_ਆਖਰੀ_ਇੰਦਰਾਜ;
} tpm_state VARLOW;

ਸਥਿਰ ਪੂਰਨ tpm_ਸੈੱਟ_log_area(u8 *log_area_ਸ਼ੁਰੂ_ਪਤਾ,
                            u32 log_area_minimum_ਲੰਮਾਈ)
{
    ਜੇ (!log_area_ਸ਼ੁਰੂ_ਪਤਾ || !log_area_minimum_ਲੰਮਾਈ)
        ਵਾਪਸ -1;

    memset(log_area_ਸ਼ੁਰੂ_ਪਤਾ, 0, log_area_minimum_ਲੰਮਾਈ);
    tpm_state.log_area_ਸ਼ੁਰੂ_ਪਤਾ = log_area_ਸ਼ੁਰੂ_ਪਤਾ;
    tpm_state.log_area_minimum_ਲੰਮਾਈ = log_area_minimum_ਲੰਮਾਈ;
    tpm_state.log_area_ਅਗਲਾ_ਇੰਦਰਾਜ = log_area_ਸ਼ੁਰੂ_ਪਤਾ;
    tpm_state.log_area_ਆਖਰੀ_ਇੰਦਰਾਜ = ਨਿਰਾ;
    tpm_state.ਇੰਦਰਾਜ_ਗਿਣਤੀ = 0;
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
tpm_tcpa_probe(ਖਾਲੀ)
{
    ਬਣਤਰ tcpa_descriptor_rev2 *tcpa = ਲੱਭੋ_acpi_ਸਾਰਣੀ(TCPA_SIGNATURE);
    ਜੇ (!tcpa)
        ਵਾਪਸ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(ਲੰਮਾ)tcpa->log_area_ਸ਼ੁਰੂ_ਪਤਾ,
            tcpa->log_area_minimum_ਲੰਮਾਈ);

    ਵਾਪਸ tpm_ਸੈੱਟ_log_area((u8*)(ਲੰਮਾ)tcpa->log_area_ਸ਼ੁਰੂ_ਪਤਾ,
                            tcpa->log_area_minimum_ਲੰਮਾਈ);
}

ਸਥਿਰ ਪੂਰਨ
tpm_tpm2_probe(ਖਾਲੀ)
{
    ਬਣਤਰ tpm2_descriptor_rev2 *tpm2 = ਲੱਭੋ_acpi_ਸਾਰਣੀ(TPM2_SIGNATURE);
    ਜੇ (!tpm2)
        ਵਾਪਸ -1;

    ਜੇ (tpm2->ਲੰਮਾਈ < 76)
        ਵਾਪਸ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(ਲੰਮਾ)tpm2->log_area_ਸ਼ੁਰੂ_ਪਤਾ,
            tpm2->log_area_minimum_ਲੰਮਾਈ);

    ਵਾਪਸ tpm_ਸੈੱਟ_log_area((u8*)(ਲੰਮਾ)tpm2->log_area_ਸ਼ੁਰੂ_ਪਤਾ,
                            tpm2->log_area_minimum_ਲੰਮਾਈ);
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
ਸਥਿਰ ਪੂਰਨ
tpm_log_event(ਬਣਤਰ tpm_log_ਸਿਰਲੇਖ *ਇੰਦਰਾਜ, ਪੂਰਨ digest_len
              , ਅਚਲ ਖਾਲੀ *event, ਪੂਰਨ event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_ਸ਼ੁਰੂ_ਪਤਾ, tpm_state.log_area_ਅਗਲਾ_ਇੰਦਰਾਜ);

    ਜੇ (tpm_state.log_area_ਅਗਲਾ_ਇੰਦਰਾਜ == ਨਿਰਾ)
        ਵਾਪਸ -1;

    u32 ਮਾਪ = (ਆਕਾਰ(*ਇੰਦਰਾਜ) + digest_len
                + ਆਕਾਰ(ਬਣਤਰ tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_ਅਗਲਾ_ਇੰਦਰਾਜ + ਮਾਪ
                   - tpm_state.log_area_ਸ਼ੁਰੂ_ਪਤਾ);
    ਜੇ (logsize > tpm_state.log_area_minimum_ਲੰਮਾਈ) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", ਮਾਪ);
        ਵਾਪਸ -1;
    }

    ਖਾਲੀ *dest = tpm_state.log_area_ਅਗਲਾ_ਇੰਦਰਾਜ;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, ਇੰਦਰਾਜ, ਆਕਾਰ(*ਇੰਦਰਾਜ) + digest_len);
    ਬਣਤਰ tpm_log_trailer *t = dest + ਆਕਾਰ(*ਇੰਦਰਾਜ) + digest_len;
    t->eventdatasize = event_len;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(t->event, event, event_len);

    tpm_state.log_area_ਆਖਰੀ_ਇੰਦਰਾਜ = tpm_state.log_area_ਅਗਲਾ_ਇੰਦਰਾਜ;
    tpm_state.log_area_ਅਗਲਾ_ਇੰਦਰਾਜ += ਮਾਪ;
    tpm_state.ਇੰਦਰਾਜ_ਗਿਣਤੀ++;

    ਵਾਪਸ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ਸਥਿਰ TPMVersion TPM_ਸੰਸਕਰਣ;
ਸਥਿਰ u32 tpm20_pcr_selection_ਮਾਪ;
ਸਥਿਰ ਬਣਤਰ tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ {
    ਬਣਤਰ tpm_log_ਸਿਰਲੇਖ hdr;
    u8 pad[ਆਕਾਰ(ਬਣਤਰ tpm2_digest_ਮੁੱਲਾਂ)
           + 8 * ਆਕਾਰ(ਬਣਤਰ tpm2_digest_ਮੁੱਲ)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ਸਥਿਰ ਅਚਲ ਬਣਤਰ hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ਅਚਲ ਅੱਖਰ *name;
    ਖਾਲੀ (*hashfunc)(ਅਚਲ u8 *ਅੰਕੜੇ, u32 ਲੰਮਾਈ, u8 *hash);
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

ਸਥਿਰ ਪੂਰਨ
tpm20_get_hash_buffersize(u16 hashAlg)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ i;

    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(hash_parameters); i++) {
        ਜੇ (hash_parameters[i].hashalg == hashAlg)
            ਵਾਪਸ hash_parameters[i].hash_buffersize;
    }
    ਵਾਪਸ -1;
}

ਸਥਿਰ u8
tpm20_hashalg_ਮੰਜ਼ਿਲ_flag(u16 hashAlg)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ i;

    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(hash_parameters); i++) {
        ਜੇ (hash_parameters[i].hashalg == hashAlg)
            ਵਾਪਸ hash_parameters[i].hashalg_flag;
    }
    ਵਾਪਸ 0;
}

ਸਥਿਰ u16
tpm20_hashalg_flag_ਮੰਜ਼ਿਲ_hashalg(u8 hashalg_flag)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ i;

    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(hash_parameters); i++) {
        ਜੇ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ਵਾਪਸ hash_parameters[i].hashalg;
    }
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
tpm20_hashalg_flag_ਮੰਜ਼ਿਲ_name(u8 hashalg_flag)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ i;

    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(hash_parameters); i++) {
        ਜੇ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ਵਾਪਸ hash_parameters[i].name;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਖਾਲੀ tpm2_hash_ਅੰਕੜੇ(u16 hashAlg, ਅਚਲ u8 *ਅੰਕੜੇ, u32 ਅੰਕੜੇ_len, u8 *hash)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ i;

    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(hash_parameters); i++) {
        ਜੇ (hash_parameters[i].hashalg == hashAlg) {
            ਜੇ (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(ਅੰਕੜੇ, ਅੰਕੜੇ_len, hash);
            } ਨਹੀਂਤਾਂ {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ਸਥਿਰ ਪੂਰਨ
tpm20_write_EfiSpecIdEventStruct(ਖਾਲੀ)
{
    ਜੇ (!tpm20_pcr_selection)
        ਵਾਪਸ -1;

    ਬਣਤਰ {
        ਬਣਤਰ TCG_EfiSpecIdEventStruct hdr;
        u8 pad[ਆਕਾਰ(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ) + ਆਕਾਰ(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.spec_ਸੰਸਕਰਣ_Minor = 0,
        .hdr.spec_ਸੰਸਕਰਣ_Major = 2,
        .hdr.specErrata = 2,
        .hdr.uintn_ਮਾਪ = 2,
    };

    ਬਣਤਰ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ਖਾਲੀ *nsel, *ਸਿਰਾ = (ਖਾਲੀ*)tpm20_pcr_selection + tpm20_pcr_selection_ਮਾਪ;

    u32 ਗਿਣਤੀ_2, ਅੰਕ_Algs = 0;
    ਲਈ (ਗਿਣਤੀ_2 = 0; ਗਿਣਤੀ_2 < be32_ਮੰਜ਼ਿਲ_cpu(tpm20_pcr_selection->ਗਿਣਤੀ_2); ਗਿਣਤੀ_2++) {
        u8 ਮਾਪ_ਦਾ_Select = sel->ਮਾਪ_ਦਾ_Select;

        nsel = (ਖਾਲੀ*)sel + ਆਕਾਰ(*sel) + ਮਾਪ_ਦਾ_Select;
        ਜੇ (nsel > ਸਿਰਾ)
            ਰੋਕੋ;

        ਜੇ (!ਮਾਪ_ਦਾ_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ਜਾਰੀ;
        }

        ਪੂਰਨ hsize = tpm20_get_hash_buffersize(be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg));
        ਜੇ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg));
            ਵਾਪਸ -1;
        }

        ਪੂਰਨ event_ਮਾਪ = offsetof(ਬਣਤਰ TCG_EfiSpecIdEventStruct
                                  , digestSizes[ਗਿਣਤੀ_2+1]);
        ਜੇ (event_ਮਾਪ > ਆਕਾਰ(event) - ਆਕਾਰ(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            ਵਾਪਸ -1;
        }

        event.hdr.digestSizes[ਅੰਕ_Algs].algorithmId = be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg);
        event.hdr.digestSizes[ਅੰਕ_Algs].digest_ਮਾਪ = hsize;
        ਅੰਕ_Algs++;

        sel = nsel;
    }

    ਜੇ (sel != ਸਿਰਾ) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ਵਾਪਸ -1;
    }

    event.hdr.ਸੰਖਿਆ_ਦਾ_Algorithms = ਅੰਕ_Algs;
    ਪੂਰਨ event_ਮਾਪ = offsetof(ਬਣਤਰ TCG_EfiSpecIdEventStruct
                              , digestSizes[ਅੰਕ_Algs]);
    u8 *vendor_Info_ਮਾਪ = (ਖਾਲੀ*)&event + event_ਮਾਪ;
    *vendor_Info_ਮਾਪ = 0;
    event_ਮਾਪ += ਆਕਾਰ(*vendor_Info_ਮਾਪ);

    ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    ਵਾਪਸ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_ਮਾਪ);
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
ਸਥਿਰ ਪੂਰਨ
tpm20_build_digest(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ *le,
                   ਅਚਲ u8 *hashdata, u32 hashdata_len, ਪੂਰਨ bigEndian)
{
    ਜੇ (!tpm20_pcr_selection)
        ਵਾਪਸ -1;

    ਬਣਤਰ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ਖਾਲੀ *nsel, *ਸਿਰਾ = (ਖਾਲੀ*)tpm20_pcr_selection + tpm20_pcr_selection_ਮਾਪ;
    ਖਾਲੀ *dest = le->hdr.digest + ਆਕਾਰ(ਬਣਤਰ tpm2_digest_ਮੁੱਲਾਂ);

    u32 ਗਿਣਤੀ_2, ਅੰਕ_Algs = 0;
    ਲਈ (ਗਿਣਤੀ_2 = 0; ਗਿਣਤੀ_2 < be32_ਮੰਜ਼ਿਲ_cpu(tpm20_pcr_selection->ਗਿਣਤੀ_2); ਗਿਣਤੀ_2++) {
        u8 ਮਾਪ_ਦਾ_Select = sel->ਮਾਪ_ਦਾ_Select;

        nsel = (ਖਾਲੀ*)sel + ਆਕਾਰ(*sel) + ਮਾਪ_ਦਾ_Select;
        ਜੇ (nsel > ਸਿਰਾ)
            ਰੋਕੋ;

        /* PCR 0-7 unused? -- skip */
        ਜੇ (!ਮਾਪ_ਦਾ_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ਜਾਰੀ;
        }

        ਪੂਰਨ hsize = tpm20_get_hash_buffersize(be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg));
        ਜੇ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg));
            ਵਾਪਸ -1;
        }

        /* buffer size sanity check before writing */
        ਬਣਤਰ tpm2_digest_ਮੁੱਲ *v = dest;
        ਜੇ (dest + ਆਕਾਰ(*v) + hsize > (ਖਾਲੀ*)le + ਆਕਾਰ(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            ਵਾਪਸ -1;
        }

        ਜੇ (bigEndian)
            v->hashAlg = sel->hashAlg;
        ਨਹੀਂਤਾਂ
            v->hashAlg = be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg);

        tpm2_hash_ਅੰਕੜੇ(be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += ਆਕਾਰ(*v) + hsize;
        sel = nsel;

        ਅੰਕ_Algs++;
    }

    ਜੇ (sel != ਸਿਰਾ) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ਵਾਪਸ -1;
    }

    ਬਣਤਰ tpm2_digest_ਮੁੱਲਾਂ *v = (ਖਾਲੀ*)le->hdr.digest;
    ਜੇ (bigEndian)
        v->ਗਿਣਤੀ_2 = cpu_ਮੰਜ਼ਿਲ_be32(ਅੰਕ_Algs);
    ਨਹੀਂਤਾਂ
        v->ਗਿਣਤੀ_2 = ਅੰਕ_Algs;

    ਵਾਪਸ dest - (ਖਾਲੀ*)le->hdr.digest;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_build_digest(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ *le,
                   ਅਚਲ u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    ਵਾਪਸ SHA1_BUFSIZE;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_build_digest_ਸਿੱਧਾ(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ *le, ਅਚਲ u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(le->hdr.digest, sha1, SHA1_BUFSIZE);
    ਵਾਪਸ SHA1_BUFSIZE;
}

ਸਥਿਰ ਪੂਰਨ
tpm_build_digest(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ *le, ਅਚਲ u8 *hashdata, u32 hashdata_len
                 , ਪੂਰਨ bigEndian)
{
    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        ਵਾਪਸ tpm12_build_digest(le, hashdata, hashdata_len);
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        ਵਾਪਸ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    ਵਾਪਸ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ਸਥਿਰ ਪੂਰਨ
tpm_simple_cmd(u8 locty, u32 ordinal
               , ਪੂਰਨ param_ਮਾਪ, u16 param, ਗਿਣਤੀ tpmDurationType ਮੰਜ਼ਿਲ_t)
{
    ਬਣਤਰ {
        ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(req.trqh) + param_ਮਾਪ),
        .trqh.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(ordinal),
        .param = param_ਮਾਪ == 2 ? cpu_ਮੰਜ਼ਿਲ_be16(param) : param,
    };
    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        req.trqh.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM_ਟੈਗ_RQU_CMD);
        ਰੋਕੋ;
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        req.trqh.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_NO_SESSIONS);
        ਰੋਕੋ;
    }

    u8 obuffer[64];
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ *trsh = (ਖਾਲੀ*)obuffer;
    u32 obuffer_len = ਆਕਾਰ(obuffer);
    memset(obuffer, 0x0, ਆਕਾਰ(obuffer));

    ਪੂਰਨ ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, ਮੰਜ਼ਿਲ_t);
    ret = ret ? -1 : be32_ਮੰਜ਼ਿਲ_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_getcapability(u32 capability, u32 property, u32 ਗਿਣਤੀ_2,
                    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ *rsp, u32 rsize)
{
    ਬਣਤਰ tpm2_req_getcapability trg = {
        .hdr.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trg)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_GetCapability),
        .capability = cpu_ਮੰਜ਼ਿਲ_be32(capability),
        .property = cpu_ਮੰਜ਼ਿਲ_be32(property),
        .propertycount = cpu_ਮੰਜ਼ਿਲ_be32(ਗਿਣਤੀ_2),
    };

    u32 resp_ਮਾਪ = rsize;
    ਪੂਰਨ ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_ਮਾਪ,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_ਮੰਜ਼ਿਲ_cpu(rsp->totlen)) ? -1 : be32_ਮੰਜ਼ਿਲ_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_get_pcrbanks(ਖਾਲੀ)
{
    u8 ਬਫਰ[128];
    ਬਣਤਰ tpm2_res_getcapability *trg =
      (ਬਣਤਰ tpm2_res_getcapability *)&ਬਫਰ;

    ਪੂਰਨ ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  ਆਕਾਰ(ਬਫਰ));
    ਜੇ (ret)
        ਵਾਪਸ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_ਮੰਜ਼ਿਲ_cpu(trg->hdr.totlen);
    ਜੇ (resplen <= offsetof(ਬਣਤਰ tpm2_res_getcapability, ਅੰਕੜੇ))
        ਵਾਪਸ -1;

    u32 ਮਾਪ = resplen - offsetof(ਬਣਤਰ tpm2_res_getcapability, ਅੰਕੜੇ);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    ਜੇ (ਮਾਪ < offsetof(ਬਣਤਰ tpml_pcr_selection, selections) +
               offsetof(ਬਣਤਰ tpms_pcr_selection, pcrSelect))
        ਵਾਪਸ -1;

    tpm20_pcr_selection = malloc_ਉੱਚਾ(ਮਾਪ);
    ਜੇ (tpm20_pcr_selection) {
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(tpm20_pcr_selection, &trg->ਅੰਕੜੇ, ਮਾਪ);
        tpm20_pcr_selection_ਮਾਪ = ਮਾਪ;
    } ਨਹੀਂਤਾਂ {
        warn_noalloc();
        ret = -1;
    }

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *ਸਰਗਰਮ_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *ਸਰਗਰਮ_pcrbanks = 0;

    ਜੇ (!tpm20_pcr_selection)
        ਵਾਪਸ -1;

    ਬਣਤਰ tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    ਖਾਲੀ *ਸਿਰਾ = (ਖਾਲੀ*)tpm20_pcr_selection + tpm20_pcr_selection_ਮਾਪ;

    ਜਦੋਂ (1) {
        u8 ਮਾਪ_ਦਾ_Select = sel->ਮਾਪ_ਦਾ_Select;
        ਖਾਲੀ *nsel = (ਖਾਲੀ*)sel + ਆਕਾਰ(*sel) + ਮਾਪ_ਦਾ_Select;
        ਜੇ (nsel > ਸਿਰਾ)
            ਵਾਪਸ 0;

        u16 hashalg = be16_ਮੰਜ਼ਿਲ_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_ਮੰਜ਼ਿਲ_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        ਬਿਨਾਨਿਸ਼ਾਨੀ i;
        ਲਈ (i = 0; i < ਮਾਪ_ਦਾ_Select; i++) {
            ਜੇ (sel->pcrSelect[i]) {
                *ਸਰਗਰਮ_pcrbanks |= hashalg_flag;
                ਰੋਕੋ;
            }
        }

        sel = nsel;
    }
}

ਸਥਿਰ ਪੂਰਨ
tpm20_ਸੈੱਟ_pcrbanks(u32 ਸਰਗਰਮ_banks)
{
    ਬਣਤਰ tpm2_req_pcr_allocate trpa = {
        .hdr.ਟੈਗ     = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trpa.authblock)),
        .authblock = {
            .handle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RS_PW),
            .noncesize = cpu_ਮੰਜ਼ਿਲ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ਮੰਜ਼ਿਲ_be16(0),
        },
    };
    ਬਣਤਰ tpms_pcr_selection3 {
        u16 hashAlg;
        u8 ਮਾਪ_ਦਾ_Select;
        u8 pcrSelect[3];
    } tps[ARRAY_ਮਾਪ(trpa.tpms_pcr_selections)];
    ਪੂਰਨ i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    ਜਦੋਂ (hashalg_flag) {
        ਜੇ ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_ਮੰਜ਼ਿਲ_hashalg(hashalg_flag);

            ਜੇ (hashalg) {
                u8 ਨਕਾਬ = 0;
                tps[i].hashAlg = cpu_ਮੰਜ਼ਿਲ_be16(hashalg);
                tps[i].ਮਾਪ_ਦਾ_Select = 3;

                ਜੇ (ਸਰਗਰਮ_banks & hashalg_flag)
                    ਨਕਾਬ = 0xff;

                tps[i].pcrSelect[0] = ਨਕਾਬ;
                tps[i].pcrSelect[1] = ਨਕਾਬ;
                tps[i].pcrSelect[2] = ਨਕਾਬ;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.ਗਿਣਤੀ_2 = cpu_ਮੰਜ਼ਿਲ_be32(i);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(trpa.tpms_pcr_selections, tps, i * ਆਕਾਰ(tps[0]));
    trpa.hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(offsetof(ਬਣਤਰ tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * ਆਕਾਰ(tps[0]));

    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);

    ਪੂਰਨ ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_ਮੰਜ਼ਿਲ_cpu(rsp.errcode);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ tpm20_activate_pcrbanks(u32 ਸਰਗਰਮ_banks)
{
    ਪੂਰਨ ret = tpm20_ਸੈੱਟ_pcrbanks(ਸਰਗਰਮ_banks);
    ਜੇ (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    ਜੇ (!ret)
        reset();
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_get_capability(u32 cap, u32 subcap, ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ *rsp, u32 rsize)
{
    ਬਣਤਰ tpm_req_getcap trgc = {
        .hdr.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM_ਟੈਗ_RQU_CMD),
        .hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trgc)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM_ORD_GetCapability),
        .capArea = cpu_ਮੰਜ਼ਿਲ_be32(cap),
        .sub_Cap_ਮਾਪ = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trgc.subCap)),
        .subCap = cpu_ਮੰਜ਼ਿਲ_be32(subcap)
    };
    u32 resp_ਮਾਪ = rsize;
    ਪੂਰਨ ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_ਮਾਪ,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_ਮਾਪ != rsize) ? -1 : be32_ਮੰਜ਼ਿਲ_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_read_permanent_flags(ਅੱਖਰ *buf, ਪੂਰਨ buf_len)
{
    memset(buf, 0, buf_len);

    ਬਣਤਰ tpm_res_getcap_perm_flags pf;
    ਪੂਰਨ ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, ਆਕਾਰ(pf));
    ਜੇ (ret)
        ਵਾਪਸ -1;

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, &pf.perm_flags, buf_len);

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_determine_timeouts(ਖਾਲੀ)
{
    ਬਣਤਰ tpm_res_getcap_timeouts timeouts;
    ਪੂਰਨ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, ਆਕਾਰ(timeouts));
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਬਣਤਰ tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, ਆਕਾਰ(durations));
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਪੂਰਨ i;
    ਲਈ (i = 0; i < 3; i++)
        durations.durations[i] = be32_ਮੰਜ਼ਿਲ_cpu(durations.durations[i]);

    ਲਈ (i = 0; i < 4; i++)
        timeouts.timeouts[i] = be32_ਮੰਜ਼ਿਲ_cpu(timeouts.timeouts[i]);

    dprintf(DEBUG_tcg, "TCGBIOS: timeouts: %u %u %u %u\n",
            timeouts.timeouts[0],
            timeouts.timeouts[1],
            timeouts.timeouts[2],
            timeouts.timeouts[3]);

    dprintf(DEBUG_tcg, "TCGBIOS: durations: %u %u %u\n",
            durations.durations[0],
            durations.durations[1],
            durations.durations[2]);

    tpmhw_ਸੈੱਟ_timeouts(timeouts.timeouts, durations.durations);

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ
tpm20_ਸੈੱਟ_timeouts(ਖਾਲੀ)
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

    tpmhw_ਸੈੱਟ_timeouts(timeouts, durations);
}

ਸਥਿਰ ਪੂਰਨ
tpm12_extend(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ *le, ਪੂਰਨ digest_len)
{
    ਬਣਤਰ tpm_req_extend tre = {
        .hdr.ਟੈਗ     = cpu_ਮੰਜ਼ਿਲ_be16(TPM_ਟੈਗ_RQU_CMD),
        .hdr.totlen  = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(tre)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_ਮੰਜ਼ਿਲ_be32(le->hdr.pcrindex),
    };
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(tre.digest, le->hdr.digest, ਆਕਾਰ(tre.digest));

    ਬਣਤਰ tpm_rsp_extend rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);
    ਪੂਰਨ ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.hdr.errcode)
        ਵਾਪਸ -1;

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ tpm20_extend(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ *le, ਪੂਰਨ digest_len)
{
    ਬਣਤਰ tpm2_req_extend tmp_tre = {
        .hdr.ਟੈਗ     = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_ਮੰਜ਼ਿਲ_be32(0),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_ਮੰਜ਼ਿਲ_be32(le->hdr.pcrindex),
        .authblocksize = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RS_PW),
            .noncesize = cpu_ਮੰਜ਼ਿਲ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ਮੰਜ਼ਿਲ_be16(0),
        },
    };
    u8 ਬਫਰ[ਆਕਾਰ(tmp_tre) + ਆਕਾਰ(le->pad)];
    ਬਣਤਰ tpm2_req_extend *tre = (ਬਣਤਰ tpm2_req_extend *)ਬਫਰ;

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(tre, &tmp_tre, ਆਕਾਰ(tmp_tre));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(tmp_tre) + digest_len);

    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);
    ਪੂਰਨ ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.errcode)
        ਵਾਪਸ -1;

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
tpm_extend(ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ *le, ਪੂਰਨ digest_len)
{
    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        ਵਾਪਸ tpm12_extend(le, digest_len);
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        ਵਾਪਸ tpm20_extend(le, digest_len);
    }
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_stirrandom(ਖਾਲੀ)
{
    ਬਣਤਰ tpm2_req_stirrandom stir = {
        .hdr.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(stir)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_StirRandom),
        .ਮਾਪ = cpu_ਮੰਜ਼ਿਲ_be16(ਆਕਾਰ(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);
    ਪੂਰਨ ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    ਬਣਤਰ tpm2_res_getrandom rsp;

    ਜੇ (buf_len > ਆਕਾਰ(rsp.rnd.ਬਫਰ))
        ਵਾਪਸ -1;

    ਬਣਤਰ tpm2_req_getrandom trgr = {
        .hdr.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trgr)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_GetRandom),
        .ਬਾਈਟਾਂ_Requested = cpu_ਮੰਜ਼ਿਲ_be16(buf_len),
    };
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);

    ਪੂਰਨ ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_MEDIUM);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.hdr.errcode)
        ret = -1;
    ਨਹੀਂਤਾਂ
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, rsp.rnd.ਬਫਰ, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    ਬਣਤਰ tpm2_req_hierarchycontrol trh = {
        .hdr.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trh)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trh.authblock)),
        .authblock = {
            .handle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RS_PW),
            .noncesize = cpu_ਮੰਜ਼ਿਲ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ਮੰਜ਼ਿਲ_be16(0),
        },
        .enable = cpu_ਮੰਜ਼ਿਲ_be32(hierarchy),
        .state = state,
    };
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);
    ਪੂਰਨ ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_MEDIUM);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_hierarchychangeauth(u8 auth[20])
{
    ਬਣਤਰ tpm2_req_hierarchychangeauth trhca = {
        .hdr.ਟੈਗ = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trhca)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trhca.authblock)),
        .authblock = {
            .handle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RS_PW),
            .noncesize = cpu_ਮੰਜ਼ਿਲ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ਮੰਜ਼ਿਲ_be16(0),
        },
        .newAuth = {
            .ਮਾਪ = cpu_ਮੰਜ਼ਿਲ_be16(ਆਕਾਰ(trhca.newAuth.ਬਫਰ)),
        },
    };
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(trhca.newAuth.ਬਫਰ, auth, ਆਕਾਰ(trhca.newAuth.ਬਫਰ));

    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);
    ਪੂਰਨ ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_MEDIUM);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    ਵਾਪਸ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ TPM_has_physical_presence;
u8 TPM_working VARLOW;

ਸਥਿਰ ਪੂਰਨ
tpm_is_working(ਖਾਲੀ)
{
    ਵਾਪਸ CONFIG_TCGBIOS && TPM_working;
}

ਸਥਿਰ ਖਾਲੀ
tpm_ਸੈੱਟ_failure(ਖਾਲੀ)
{
    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_ਸੈੱਟ_ਆਰਜ਼ੀ_Deactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        ਰੋਕੋ;
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        ਰੋਕੋ;
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
ਸਥਿਰ ਖਾਲੀ
tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(u32 pcrindex, u32 event_type,
                           ਅਚਲ ਅੱਖਰ *event, u32 event_ਲੰਮਾਈ,
                           ਅਚਲ u8 *hashdata, u32 hashdata_ਲੰਮਾਈ)
{
    ਜੇ (!tpm_is_working())
        ਵਾਪਸ;

    ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    ਪੂਰਨ digest_len = tpm_build_digest(&le, hashdata, hashdata_ਲੰਮਾਈ, 1);
    ਜੇ (digest_len < 0)
        ਵਾਪਸ;
    ਪੂਰਨ ret = tpm_extend(&le, digest_len);
    ਜੇ (ret) {
        tpm_ਸੈੱਟ_failure();
        ਵਾਪਸ;
    }
    tpm_build_digest(&le, hashdata, hashdata_ਲੰਮਾਈ, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_ਲੰਮਾਈ);
}

// Add an EV_ACTION measurement to the list of measurements
ਸਥਿਰ ਖਾਲੀ
tpm_ਜੋੜੋ_action(u32 pcr_ਸੂਚਕਾਂਕ, ਅਚਲ ਅੱਖਰ *string)
{
    u32 len = ਲੜੀ_ਲੰਬਾਈ(string);
    tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(pcr_ਸੂਚਕਾਂਕ, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ਸਥਿਰ ਖਾਲੀ
tpm_ਜੋੜੋ_event_separators(ਖਾਲੀ)
{
    ਸਥਿਰ ਅਚਲ u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcr_ਸੂਚਕਾਂਕ;
    ਲਈ (pcr_ਸੂਚਕਾਂਕ = 0; pcr_ਸੂਚਕਾਂਕ <= 7; pcr_ਸੂਚਕਾਂਕ++)
        tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(pcr_ਸੂਚਕਾਂਕ, EV_SEPARATOR,
                                   (ਅਚਲ ਅੱਖਰ *)evt_separator,
                                   ਆਕਾਰ(evt_separator),
                                   evt_separator,
                                   ਆਕਾਰ(evt_separator));
}

ਸਥਿਰ ਖਾਲੀ
tpm_smbios_measure(ਖਾਲੀ)
{
    ਬਣਤਰ pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    ਖਾਲੀ *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    ਜੇ (!smbios_tables)
        ਵਾਪਸ;

    sha1((ਅਚਲ u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(1,
                               EV_EVENT_ਟੈਗ,
                               (ਅਚਲ ਅੱਖਰ *)&pcctes, ਆਕਾਰ(pcctes),
                               (u8 *)&pcctes, ਆਕਾਰ(pcctes));
}

ਸਥਿਰ ਪੂਰਨ
tpm12_assert_physical_presence(ਖਾਲੀ)
{
    ਪੂਰਨ ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    ਜੇ (!ret)
        ਵਾਪਸ 0;

    ਬਣਤਰ tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ਅੱਖਰ *)&pf, ਆਕਾਰ(pf));
    ਜੇ (ret)
        ਵਾਪਸ -1;

    /* check if hardware physical presence is supported */
    ਜੇ (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        ਵਾਪਸ 0;
    }

    ਜੇ (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_ਤਾਲਾ]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        ਵਾਪਸ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_startup(ਖਾਲੀ)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    ਪੂਰਨ ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    ਜੇ (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    ਜੇ (ret)
        ਜਾਓ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    ਜੇ (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    ਜੇ (ret)
        ਜਾਓ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_Self_Test_ਭਰਿਆ,
                         0, 0, TPM_DURATION_TYPE_LONG);
    ਜੇ (ret)
        ਜਾਓ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_Reset_Establishment_ਦੋਆਧਾਰੀ_ਅੰਕ,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret && ret != TPM_BAD_LOCALITY)
        ਜਾਓ err_exit;

    ਵਾਪਸ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ਸੈੱਟ_failure();
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_startup(ਖਾਲੀ)
{
    tpm20_ਸੈੱਟ_timeouts();

    ਪੂਰਨ ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    ਜੇ (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    ਜੇ (ret)
        ਜਾਓ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    ਜੇ (ret)
        ਜਾਓ err_exit;

    ret = tpm20_get_pcrbanks();
    ਜੇ (ret)
        ਜਾਓ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    ਜੇ (ret)
        ਜਾਓ err_exit;

    ਵਾਪਸ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ਸੈੱਟ_failure();
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
tpm_startup(ਖਾਲੀ)
{
    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        ਵਾਪਸ tpm12_startup();
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        ਵਾਪਸ tpm20_startup();
    }
    ਵਾਪਸ -1;
}

ਖਾਲੀ
tpm_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ;

    ਪੂਰਨ ret = tpm_tpm2_probe();
    ਜੇ (ret) {
        ret = tpm_tcpa_probe();
        ਜੇ (ret)
            ਵਾਪਸ;
    }

    TPM_ਸੰਸਕਰਣ = tpmhw_probe();
    ਜੇ (TPM_ਸੰਸਕਰਣ == TPM_ਸੰਸਕਰਣ_NONE)
        ਵਾਪਸ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_ਸੰਸਕਰਣ == TPM_ਸੰਸਕਰਣ_1_2) ? "1.2" : "2");

    TPM_working = 1;

    ਜੇ (runningOnXen())
        ਵਾਪਸ;

    ret = tpm_startup();
    ਜੇ (ret)
        ਵਾਪਸ;

    tpm_smbios_measure();
    tpm_ਜੋੜੋ_action(2, "Start Option ROM Scan");
}

ਸਥਿਰ ਖਾਲੀ
tpm20_prepboot(ਖਾਲੀ)
{
    ਪੂਰਨ ret = tpm20_stirrandom();
    ਜੇ (ret)
         ਜਾਓ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], ਆਕਾਰ(auth));
    ਜੇ (ret)
        ਜਾਓ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    ਜੇ (ret)
        ਜਾਓ err_exit;

    ਵਾਪਸ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ਸੈੱਟ_failure();
}

ਖਾਲੀ
tpm_prepboot(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ;

    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        ਜੇ (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_ਤਾਲਾ, TPM_DURATION_TYPE_SHORT);
        ਰੋਕੋ;
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        tpm20_prepboot();
        ਰੋਕੋ;
    }

    tpm_ਜੋੜੋ_action(4, "Calling INT 19h");
    tpm_ਜੋੜੋ_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
ਖਾਲੀ
tpm_option_rom(ਅਚਲ ਖਾਲੀ *addr, u32 len)
{
    ਜੇ (!tpm_is_working())
        ਵਾਪਸ;

    ਬਣਤਰ pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = ਆਕਾਰ(u16) + ਆਕਾਰ(u16) + SHA1_BUFSIZE,
    };
    sha1((ਅਚਲ u8 *)addr, len, pcctes.digest);
    tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(2,
                               EV_EVENT_ਟੈਗ,
                               (ਅਚਲ ਅੱਖਰ *)&pcctes, ਆਕਾਰ(pcctes),
                               (u8 *)&pcctes, ਆਕਾਰ(pcctes));
}

ਖਾਲੀ
tpm_ਜੋੜੋ_bcv(u32 bootdrv, ਅਚਲ u8 *addr, u32 ਲੰਮਾਈ)
{
    ਜੇ (!tpm_is_working())
        ਵਾਪਸ;

    ਜੇ (ਲੰਮਾਈ < 0x200)
        ਵਾਪਸ;

    ਅਚਲ ਅੱਖਰ *string = "Booting BCV device 00h (Floppy)";
    ਜੇ (bootdrv == 0x80)
        string = "Booting BCV device 80h (HDD)";
    tpm_ਜੋੜੋ_action(4, string);

    /* specs: see section 'Hard Disk Device or Hard Disk-Like Devices' */
    /* equivalent to: dd if=/dev/hda ibs=1 count=440 | sha1sum */
    string = "MBR";
    tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(4, EV_IPL,
                               string, ਲੜੀ_ਲੰਬਾਈ(string),
                               addr, 0x1b8);

    /* equivalent to: dd if=/dev/hda ibs=1 count=72 skip=440 | sha1sum */
    string = "MBR PARTITION_TABLE";
    tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(5, EV_IPL_ਹਿੱਸੇ_ਕਰੋ_ਅੰਕੜੇ,
                               string, ਲੜੀ_ਲੰਬਾਈ(string),
                               addr + 0x1b8, 0x48);
}

ਖਾਲੀ
tpm_ਜੋੜੋ_cdrom(u32 bootdrv, ਅਚਲ u8 *addr, u32 ਲੰਮਾਈ)
{
    ਜੇ (!tpm_is_working())
        ਵਾਪਸ;

    tpm_ਜੋੜੋ_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ਅਚਲ ਅੱਖਰ *string = "EL TORITO IPL";
    tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(4, EV_IPL,
                               string, ਲੜੀ_ਲੰਬਾਈ(string),
                               addr, ਲੰਮਾਈ);
}

ਖਾਲੀ
tpm_ਜੋੜੋ_cdrom_catalog(ਅਚਲ u8 *addr, u32 ਲੰਮਾਈ)
{
    ਜੇ (!tpm_is_working())
        ਵਾਪਸ;

    tpm_ਜੋੜੋ_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ਅਚਲ ਅੱਖਰ *string = "BOOT CATALOG";
    tpm_ਜੋੜੋ_measurement_ਮੰਜ਼ਿਲ_log(5, EV_IPL_ਹਿੱਸੇ_ਕਰੋ_ਅੰਕੜੇ,
                               string, ਲੜੀ_ਲੰਬਾਈ(string),
                               addr, ਲੰਮਾਈ);
}

ਖਾਲੀ
tpm_s3_resume(ਖਾਲੀ)
{
    ਜੇ (!tpm_is_working())
        ਵਾਪਸ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    ਪੂਰਨ ret = -1;

    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        ਰੋਕੋ;
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        ਜੇ (ret)
            ਜਾਓ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        ਰੋਕੋ;
    }

    ਜੇ (ret)
        ਜਾਓ err_exit;

    ਵਾਪਸ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_ਸੈੱਟ_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *ਨਿਵੇਸ਼_buf32(ਬਣਤਰ bregs *regs)
{
    ਵਾਪਸ MAKE_FLATPTR(regs->es, regs->di);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *output_buf32(ਬਣਤਰ bregs *regs)
{
    ਵਾਪਸ MAKE_FLATPTR(regs->ds, regs->si);
}

ਸਥਿਰ u32
hash_log_extend(ਬਣਤਰ pcpes *pcpes, ਅਚਲ ਖਾਲੀ *hashdata, u32 hashdata_ਲੰਮਾਈ
                , ਖਾਲੀ *event, ਪੂਰਨ extend)
{
    ਜੇ (pcpes->pcrindex >= 24)
        ਵਾਪਸ TCG_INVALID_ਨਿਵੇਸ਼_PARA;
    ਜੇ (hashdata)
        sha1(hashdata, hashdata_ਲੰਮਾਈ, pcpes->digest);

    ਬਣਤਰ tpm_log_ਇੰਦਰਾਜ le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    ਪੂਰਨ digest_len = tpm12_build_digest_ਸਿੱਧਾ(&le, pcpes->digest);
    ਜੇ (digest_len < 0)
        ਵਾਪਸ TCG_GENERAL_ERROR;
    ਜੇ (extend) {
        ਪੂਰਨ ret = tpm_extend(&le, digest_len);
        ਜੇ (ret)
            ਵਾਪਸ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_ਸਿੱਧਾ(&le, pcpes->digest);
    ਪੂਰਨ ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    ਜੇ (ret)
        ਵਾਪਸ TCG_PC_LOGOVERFLOW;
    ਵਾਪਸ 0;
}

ਸਥਿਰ u32
hash_log_extend_event_int(ਅਚਲ ਬਣਤਰ hleei_short *hleei_s,
                          ਬਣਤਰ hleeo *hleeo)
{
    u32 rc = 0;
    ਬਣਤਰ hleei_long *hleei_l = (ਬਣਤਰ hleei_long *)hleei_s;
    ਅਚਲ ਖਾਲੀ *logdataptr;
    u32 logdatalen;
    ਬਣਤਰ pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    ਚੋਣ (hleei_s->ipblength) {
    ਹਾਲਤ ਆਕਾਰ(ਬਣਤਰ hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    ਰੋਕੋ;

    ਹਾਲਤ ਆਕਾਰ(ਬਣਤਰ hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    ਰੋਕੋ;

    ਮੂਲ:
        /* bad input block */
        rc = TCG_INVALID_ਨਿਵੇਸ਼_PARA;
        ਜਾਓ err_exit;
    }

    pcpes = (ਬਣਤਰ pcpes *)logdataptr;

    ਜੇ (pcpes->pcrindex != pcrindex
        || logdatalen != ਆਕਾਰ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ਨਿਵੇਸ਼_PARA;
        ਜਾਓ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    ਜੇ (rc)
        ਜਾਓ err_exit;

    hleeo->opblength = ਆਕਾਰ(ਬਣਤਰ hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.ਇੰਦਰਾਜ_ਗਿਣਤੀ;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(hleeo->digest, pcpes->digest, ਆਕਾਰ(hleeo->digest));

err_exit:
    ਜੇ (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32
pass_through_ਮੰਜ਼ਿਲ_tpm_int(ਬਣਤਰ pttti *pttti, ਬਣਤਰ pttto *pttto)
{
    u32 rc = 0;
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ *trh = (ਖਾਲੀ*)pttti->tpmopin;

    ਜੇ (pttti->ipblength < ਆਕਾਰ(ਬਣਤਰ pttti) + ਆਕਾਰ(*trh)
        || pttti->ipblength != ਆਕਾਰ(ਬਣਤਰ pttti) + be32_ਮੰਜ਼ਿਲ_cpu(trh->totlen)
        || pttti->opblength < ਆਕਾਰ(ਬਣਤਰ pttto)) {
        rc = TCG_INVALID_ਨਿਵੇਸ਼_PARA;
        ਜਾਓ err_exit;
    }

    u16 ਟੈਗ = be16_ਮੰਜ਼ਿਲ_cpu(trh->ਟੈਗ);

    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        ਜੇ (ਟੈਗ != TPM_ਟੈਗ_RQU_CMD && ਟੈਗ != TPM_ਟੈਗ_RQU_AUTH1_CMD
            && ਟੈਗ != TPM_ਟੈਗ_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_ਨਿਵੇਸ਼_PARA;
            ਜਾਓ err_exit;
        }
        ਰੋਕੋ;
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        ਜੇ (ਟੈਗ != TPM2_ST_NO_SESSIONS && ਟੈਗ != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_ਨਿਵੇਸ਼_PARA;
            ਜਾਓ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(ਬਣਤਰ pttto, tpmopout);
    ਪੂਰਨ ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    ਜੇ (ret) {
        rc = TCG_FATAL_COM_ERROR;
        ਜਾਓ err_exit;
    }

    pttto->opblength = offsetof(ਬਣਤਰ pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    ਜੇ (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32
shutdown_preboot_interface(ਖਾਲੀ)
{
    TPM_interface_shutdown = 1;
    ਵਾਪਸ 0;
}

ਸਥਿਰ u32
hash_log_event_int(ਅਚਲ ਬਣਤਰ hlei *hlei, ਬਣਤਰ hleo *hleo)
{
    u32 rc = 0;
    u16 ਮਾਪ;
    ਬਣਤਰ pcpes *pcpes;

    ਮਾਪ = hlei->ipblength;
    ਜੇ (ਮਾਪ != ਆਕਾਰ(*hlei)) {
        rc = TCG_INVALID_ਨਿਵੇਸ਼_PARA;
        ਜਾਓ err_exit;
    }

    pcpes = (ਬਣਤਰ pcpes *)hlei->logdataptr;

    ਜੇ (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != ਆਕਾਰ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ਨਿਵੇਸ਼_PARA;
        ਜਾਓ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    ਜੇ (rc)
        ਜਾਓ err_exit;

    /* updating the log was fine */
    hleo->opblength = ਆਕਾਰ(ਬਣਤਰ hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.ਇੰਦਰਾਜ_ਗਿਣਤੀ;

err_exit:
    ਜੇ (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32
hash_all_int(ਅਚਲ ਬਣਤਰ hai *hai, u8 *hash)
{
    ਜੇ (hai->ipblength != ਆਕਾਰ(ਬਣਤਰ hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        ਵਾਪਸ TCG_INVALID_ਨਿਵੇਸ਼_PARA;

    sha1((ਅਚਲ u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    ਵਾਪਸ 0;
}

ਸਥਿਰ u32
tss_int(ਬਣਤਰ ti *ti, ਬਣਤਰ ਮੰਜ਼ਿਲ *ਮੰਜ਼ਿਲ)
{
    ਮੰਜ਼ਿਲ->opblength = ਆਕਾਰ(ਬਣਤਰ ਮੰਜ਼ਿਲ);
    ਮੰਜ਼ਿਲ->reserved  = 0;

    ਵਾਪਸ TCG_PC_UNSUPPORTED;
}

ਸਥਿਰ u32
compact_hash_log_extend_event_int(u8 *ਬਫਰ,
                                  u32 info,
                                  u32 ਲੰਮਾਈ,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    ਬਣਤਰ pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = ਆਕਾਰ(info),
    };
    u32 rc = hash_log_extend(&pcpes, ਬਫਰ, ਲੰਮਾਈ, &info, 1);
    ਜੇ (rc)
        ਵਾਪਸ rc;

    *edx_ptr = tpm_state.ਇੰਦਰਾਜ_ਗਿਣਤੀ;
    ਵਾਪਸ 0;
}

ਖਾਲੀ VISIBLE32FLAT
tpm_ਰੁਕਾਵਟ_handler32(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ;

    ਸੈੱਟ_cf(regs, 0);

    ਜੇ (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        ਵਾਪਸ;
    }

    ਚੋਣ ((ਗਿਣਤੀ irq_ids)regs->al) {
    ਹਾਲਤ TCG_ਸਥਿਤੀ_Check:
        ਜੇ (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } ਨਹੀਂਤਾਂ {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_ਸੰਸਕਰਣ_MAJOR;
            regs->cl = TCG_ਸੰਸਕਰਣ_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_ਸ਼ੁਰੂ_ਪਤਾ;
            regs->edi = (u32)tpm_state.log_area_ਆਖਰੀ_ਇੰਦਰਾਜ;
        }
        ਰੋਕੋ;

    ਹਾਲਤ TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (ਬਣਤਰ hleei_short *)ਨਿਵੇਸ਼_buf32(regs),
                  (ਬਣਤਰ hleeo *)output_buf32(regs));
        ਰੋਕੋ;

    ਹਾਲਤ TCG_Pass_Through_ਮੰਜ਼ਿਲ_TPM:
        regs->eax =
            pass_through_ਮੰਜ਼ਿਲ_tpm_int((ਬਣਤਰ pttti *)ਨਿਵੇਸ਼_buf32(regs),
                                    (ਬਣਤਰ pttto *)output_buf32(regs));
        ਰੋਕੋ;

    ਹਾਲਤ TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        ਰੋਕੋ;

    ਹਾਲਤ TCG_HashLogEvent:
        regs->eax = hash_log_event_int((ਬਣਤਰ hlei*)ਨਿਵੇਸ਼_buf32(regs),
                                       (ਬਣਤਰ hleo*)output_buf32(regs));
        ਰੋਕੋ;

    ਹਾਲਤ TCG_HashAll:
        regs->eax =
            hash_all_int((ਬਣਤਰ hai*)ਨਿਵੇਸ਼_buf32(regs),
                          (u8 *)output_buf32(regs));
        ਰੋਕੋ;

    ਹਾਲਤ TCG_TSS:
        regs->eax = tss_int((ਬਣਤਰ ti*)ਨਿਵੇਸ਼_buf32(regs),
                            (ਬਣਤਰ ਮੰਜ਼ਿਲ*)output_buf32(regs));
        ਰੋਕੋ;

    ਹਾਲਤ TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)ਨਿਵੇਸ਼_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        ਰੋਕੋ;

    ਮੂਲ:
        ਸੈੱਟ_cf(regs, 1);
    }

    ਵਾਪਸ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

ਕਿਸਮਪਰਿਭਾਸ਼ਾ u8 tpm_ppi_ਕੋਡ;

ਸਥਿਰ ਪੂਰਨ
tpm12_read_has_owner(ਪੂਰਨ *has_owner)
{
    ਬਣਤਰ tpm_res_getcap_ownerauth oauth;
    ਪੂਰਨ ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, ਆਕਾਰ(oauth));
    ਜੇ (ret)
        ਵਾਪਸ -1;

    *has_owner = oauth.flag;

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_enable_tpm(ਪੂਰਨ enable, ਪੂਰਨ verbose)
{
    ਬਣਤਰ tpm_permanent_flags pf;
    ਪੂਰਨ ret = tpm12_read_permanent_flags((ਅੱਖਰ *)&pf, ਆਕਾਰ(pf));
    ਜੇ (ret)
        ਵਾਪਸ -1;

    ਜੇ (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        ਵਾਪਸ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret) {
        ਜੇ (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        ਨਹੀਂਤਾਂ
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_activate_tpm(ਪੂਰਨ activate, ਪੂਰਨ allow_reset, ਪੂਰਨ verbose)
{
    ਬਣਤਰ tpm_permanent_flags pf;
    ਪੂਰਨ ret = tpm12_read_permanent_flags((ਅੱਖਰ *)&pf, ਆਕਾਰ(pf));
    ਜੇ (ret)
        ਵਾਪਸ -1;

    ਜੇ (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        ਵਾਪਸ 0;

    ਜੇ (pf.flags[PERM_FLAG_IDX_DISABLE])
        ਵਾਪਸ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_Physical_ਸੈੱਟ_Deactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਜੇ (activate && allow_reset) {
        ਜੇ (verbose) {
            ਲਿਖੋ("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_enable_activate(ਪੂਰਨ allow_reset, ਪੂਰਨ verbose)
{
    ਪੂਰਨ ret = tpm12_enable_tpm(1, verbose);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਵਾਪਸ tpm12_activate_tpm(1, allow_reset, verbose);
}

ਸਥਿਰ ਪੂਰਨ
tpm12_force_clear(ਪੂਰਨ enable_activate_before, ਪੂਰਨ enable_activate_after,
                  ਪੂਰਨ verbose)
{
    ਪੂਰਨ has_owner;
    ਪੂਰਨ ret = tpm12_read_has_owner(&has_owner);
    ਜੇ (ret)
        ਵਾਪਸ -1;
    ਜੇ (!has_owner) {
        ਜੇ (verbose)
            ਲਿਖੋ("TPM does not have an owner.\n");
        ਵਾਪਸ 0;
    }

    ਜੇ (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        ਜੇ (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            ਵਾਪਸ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਜੇ (!enable_activate_after) {
        ਜੇ (verbose)
            ਲਿਖੋ("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        ਵਾਪਸ 0;
    }

    ਵਾਪਸ tpm12_enable_activate(1, verbose);
}

ਸਥਿਰ ਪੂਰਨ
tpm12_ਸੈੱਟ_owner_install(ਪੂਰਨ allow, ਪੂਰਨ verbose)
{
    ਪੂਰਨ has_owner;
    ਪੂਰਨ ret = tpm12_read_has_owner(&has_owner);
    ਜੇ (ret)
        ਵਾਪਸ -1;
    ਜੇ (has_owner) {
        ਜੇ (verbose)
            ਲਿਖੋ("Must first remove owner.\n");
        ਵਾਪਸ 0;
    }

    ਬਣਤਰ tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((ਅੱਖਰ *)&pf, ਆਕਾਰ(pf));
    ਜੇ (ret)
        ਵਾਪਸ -1;

    ਜੇ (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        ਜੇ (verbose)
            ਲਿਖੋ("TPM must first be enable.\n");
        ਵਾਪਸ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ਸੈੱਟ_Owner_Install,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਜੇ (verbose)
        ਲਿਖੋ("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_process_cfg(tpm_ppi_ਕੋਡ ਸੁਨੇਹਾ_ਕੋਡ, ਪੂਰਨ verbose)
{
    ਪੂਰਨ ret = 0;

    ਚੋਣ (ਸੁਨੇਹਾ_ਕੋਡ) {
        ਹਾਲਤ TPM_PPI_OP_NOOP: /* no-op */
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_ਸੈੱਟ_OWNERINSTALL_TRUE:
            ret = tpm12_ਸੈੱਟ_owner_install(1, verbose);
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_ਸੈੱਟ_OWNERINSTALL_FALSE:
            ret = tpm12_ਸੈੱਟ_owner_install(0, verbose);
            ਰੋਕੋ;

        ਮੂਲ:
            ਰੋਕੋ;
    }

    ਜੇ (ret)
        ਲਿਖੋ("Op %d: An error occurred: 0x%x\n", ਸੁਨੇਹਾ_ਕੋਡ, ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_clearcontrol(u8 disable, ਪੂਰਨ verbose)
{
    ਬਣਤਰ tpm2_req_clearcontrol trc = {
        .hdr.ਟੈਗ     = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trc)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trc.authblock)),
        .authblock = {
            .handle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RS_PW),
            .noncesize = cpu_ਮੰਜ਼ਿਲ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ਮੰਜ਼ਿਲ_be16(0),
        },
        .disable = disable,
    };
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);
    ਪੂਰਨ ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_SHORT);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_clear(ਖਾਲੀ)
{
    ਬਣਤਰ tpm2_req_clear trq = {
        .hdr.ਟੈਗ     = cpu_ਮੰਜ਼ਿਲ_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trq)),
        .hdr.ordinal = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_CC_Clear),
        .authhandle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(trq.authblock)),
        .authblock = {
            .handle = cpu_ਮੰਜ਼ਿਲ_be32(TPM2_RS_PW),
            .noncesize = cpu_ਮੰਜ਼ਿਲ_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_ਮੰਜ਼ਿਲ_be16(0),
        },
    };
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ rsp;
    u32 resp_ਲੰਮਾਈ = ਆਕਾਰ(rsp);
    ਪੂਰਨ ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_ਲੰਮਾਈ,
                             TPM_DURATION_TYPE_MEDIUM);
    ਜੇ (ret || resp_ਲੰਮਾਈ != ਆਕਾਰ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm20_process_cfg(tpm_ppi_ਕੋਡ ਸੁਨੇਹਾ_ਕੋਡ, ਪੂਰਨ verbose)
{
    ਪੂਰਨ ret = 0;

    ਚੋਣ (ਸੁਨੇਹਾ_ਕੋਡ) {
        ਹਾਲਤ TPM_PPI_OP_NOOP: /* no-op */
            ਰੋਕੋ;

        ਹਾਲਤ TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            ਜੇ (!ret)
                 ret = tpm20_clear();
            ਰੋਕੋ;
    }

    ਜੇ (ret)
        ਲਿਖੋ("Op %d: An error occurred: 0x%x\n", ਸੁਨੇਹਾ_ਕੋਡ, ret);

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
tpm12_get_tpm_state(ਖਾਲੀ)
{
    ਪੂਰਨ state = 0;
    ਬਣਤਰ tpm_permanent_flags pf;
    ਪੂਰਨ has_owner;

    ਜੇ (tpm12_read_permanent_flags((ਅੱਖਰ *)&pf, ਆਕਾਰ(pf)) ||
        tpm12_read_has_owner(&has_owner))
        ਵਾਪਸ ~0;

    ਜੇ (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    ਜੇ (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ਸਰਗਰਮ;

    ਜੇ (has_owner) {
        state |= TPM_STATE_OWNED;
    } ਨਹੀਂਤਾਂ {
        ਜੇ (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    ਵਾਪਸ state;
}

ਸਥਿਰ ਖਾਲੀ
tpm12_show_tpm_menu(ਪੂਰਨ state, ਪੂਰਨ ਅਗਲਾ_scancodes[7])
{
    ਪੂਰਨ i = 0;

    ਲਿਖੋ("\nThe current state of the TPM is:\n");

    ਜੇ (state & TPM_STATE_ENABLED)
        ਲਿਖੋ("  Enabled");
    ਨਹੀਂਤਾਂ
        ਲਿਖੋ("  Disabled");

    ਜੇ (state & TPM_STATE_ਸਰਗਰਮ)
        ਲਿਖੋ(" and active\n");
    ਨਹੀਂਤਾਂ
        ਲਿਖੋ(" and deactivated\n");

    ਜੇ (state & TPM_STATE_OWNED)
        ਲਿਖੋ("  Ownership has been taken\n");
    ਨਹੀਂਤਾਂ {
        ਲਿਖੋ("  Ownership has not been taken\n");
        ਜੇ (state & TPM_STATE_OWNERINSTALL)
            ਲਿਖੋ("  A user can take ownership of the TPM\n");
        ਨਹੀਂਤਾਂ
            ਲਿਖੋ("  Taking ownership of the TPM has been disabled\n");
    }

    ਜੇ ((state & (TPM_STATE_ENABLED | TPM_STATE_ਸਰਗਰਮ)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ਸਰਗਰਮ)) {
        ਲਿਖੋ("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    ਲਿਖੋ("\nAvailable options are:\n");
    ਜੇ (state & TPM_STATE_ENABLED) {
        ਲਿਖੋ(" d. Disable the TPM\n");
        ਅਗਲਾ_scancodes[i++] = 32;

        ਜੇ (state & TPM_STATE_ਸਰਗਰਮ) {
            ਲਿਖੋ(" v. Deactivate the TPM\n");
            ਅਗਲਾ_scancodes[i++] = 47;

            ਜੇ (state & TPM_STATE_OWNERINSTALL) {
                ਲਿਖੋ(" p. Prevent installation of an owner\n");
                ਅਗਲਾ_scancodes[i++] = 25;
            } ਨਹੀਂਤਾਂ {
                ਲਿਖੋ(" s. Allow installation of an owner\n");
                ਅਗਲਾ_scancodes[i++] = 31;
            }
        } ਨਹੀਂਤਾਂ {
            ਲਿਖੋ(" a. Activate the TPM\n");
            ਅਗਲਾ_scancodes[i++] = 30;
        }

    } ਨਹੀਂਤਾਂ {
        ਲਿਖੋ(" e. Enable the TPM\n");
        ਅਗਲਾ_scancodes[i++] = 18;
    }

    ਜੇ (state & TPM_STATE_OWNED) {
        ਲਿਖੋ(" c. Clear ownership\n");
        ਅਗਲਾ_scancodes[i++] = 46;
    }

    ਅਗਲਾ_scancodes[i++] = 0;
}

ਸਥਿਰ ਖਾਲੀ
tpm12_menu(ਖਾਲੀ)
{
    ਪੂਰਨ scancode, ਅਗਲਾ_scancodes[7];
    tpm_ppi_ਕੋਡ ਸੁਨੇਹਾ_ਕੋਡ;
    ਪੂਰਨ state = 0, i;
    ਪੂਰਨ waitkey;

    ਲਿਖੋ("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    ਲਈ (;;) {
        ਜੇ ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, ਅਗਲਾ_scancodes);
        } ਨਹੀਂਤਾਂ {
            ਲਿਖੋ("TPM is not working correctly.\n");
            ਵਾਪਸ;
        }

        ਲਿਖੋ("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_NOOP;

        waitkey = 1;

        ਜਦੋਂ (waitkey) {
            ਜਦੋਂ ((scancode = get_keystroke(1000)) == ~0)
                ;

            ਚੋਣ (scancode) {
            ਹਾਲਤ 1:
                // ESC
                reset();
                ਰੋਕੋ;
            ਹਾਲਤ 18: /* e. enable */
                ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_ENABLE;
                ਰੋਕੋ;
            ਹਾਲਤ 32: /* d. disable */
                ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_DISABLE;
                ਰੋਕੋ;
            ਹਾਲਤ 30: /* a. activate */
                ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_ACTIVATE;
                ਰੋਕੋ;
            ਹਾਲਤ 47: /* v. deactivate */
                ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_DEACTIVATE;
                ਰੋਕੋ;
            ਹਾਲਤ 46: /* c. clear owner */
                ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_CLEAR;
                ਰੋਕੋ;
            ਹਾਲਤ 25: /* p. prevent ownerinstall */
                ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_ਸੈੱਟ_OWNERINSTALL_FALSE;
                ਰੋਕੋ;
            ਹਾਲਤ 31: /* s. allow ownerinstall */
                ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_ਸੈੱਟ_OWNERINSTALL_TRUE;
                ਰੋਕੋ;
            ਮੂਲ:
                ਜਾਰੀ;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            ਲਈ (i = 0; i < ਆਕਾਰ(ਅਗਲਾ_scancodes); i++) {
                ਜੇ (ਅਗਲਾ_scancodes[i] == 0)
                    ਰੋਕੋ;

                ਜੇ (ਅਗਲਾ_scancodes[i] == scancode) {
                    tpm12_process_cfg(ਸੁਨੇਹਾ_ਕੋਡ, 1);
                    waitkey = 0;
                    ਰੋਕੋ;
                }
            }
        }
    }
}

ਸਥਿਰ ਪੂਰਨ
tpm20_menu_change_ਸਰਗਰਮ_pcrbanks(ਖਾਲੀ)
{
    u8 ਸਰਗਰਮ_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &ਸਰਗਰਮ_banks);

    u8 activate_banks = ਸਰਗਰਮ_banks;

    ਜਦੋਂ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        ਲਿਖੋ("\nToggle active PCR banks by pressing number key\n\n");

        ਜਦੋਂ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ਅਚਲ ਅੱਖਰ *hashname = tpm20_hashalg_flag_ਮੰਜ਼ਿਲ_name(flag);

            i++;
            ਜੇ (hashname) {
                ਲਿਖੋ("  %d: %s", i, hashname);
                ਜੇ (activate_banks & hashalg_flag)
                    ਲਿਖੋ(" (enabled)");
                ਲਿਖੋ("\n");
            }

            hashalg_flag <<= 1;
        }
        ਲਿਖੋ("\n"
               "ESC: return to previous menu without changes\n");
        ਜੇ (activate_banks)
            ਲਿਖੋ("A  : activate selection\n");

        u8 flagnum;
        ਪੂਰਨ show = 0;
        ਜਦੋਂ (!show) {
            ਪੂਰਨ scancode = get_keystroke(1000);

            ਚੋਣ (scancode) {
            ਹਾਲਤ ~0:
                ਜਾਰੀ;
            ਹਾਲਤ 1: /* ESC */
                ਲਿਖੋ("\n");
                ਵਾਪਸ -1;
            ਹਾਲਤ 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                ਜੇ (flagnum > i)
                    ਜਾਰੀ;
                ਜੇ (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                ਰੋਕੋ;
            ਹਾਲਤ 30: /* a */
                ਜੇ (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ਸਥਿਰ ਖਾਲੀ
tpm20_menu(ਖਾਲੀ)
{
    ਪੂਰਨ ਜਾਂਚ_ਕੋਡ;
    tpm_ppi_ਕੋਡ ਸੁਨੇਹਾ_ਕੋਡ;

    ਲਈ (;;) {
        ਲਿਖੋ("1. Clear TPM\n");
        ਲਿਖੋ("2. Change active PCR banks\n");

        ਲਿਖੋ("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_NOOP;

        ਜਦੋਂ ((ਜਾਂਚ_ਕੋਡ = get_keystroke(1000)) == ~0)
            ;

        ਚੋਣ (ਜਾਂਚ_ਕੋਡ) {
        ਹਾਲਤ 1:
            // ESC
            reset();
            ਰੋਕੋ;
        ਹਾਲਤ 2:
            ਸੁਨੇਹਾ_ਕੋਡ = TPM_PPI_OP_CLEAR;
            ਰੋਕੋ;
        ਹਾਲਤ 3:
            tpm20_menu_change_ਸਰਗਰਮ_pcrbanks();
            ਜਾਰੀ;
        ਮੂਲ:
            ਜਾਰੀ;
        }

        tpm20_process_cfg(ਸੁਨੇਹਾ_ਕੋਡ, 0);
    }
}

ਖਾਲੀ
tpm_menu(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ;

    ਜਦੋਂ (get_keystroke(0) >= 0)
        ;
    wait_ਧਾਗੇ();

    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        tpm12_menu();
        ਰੋਕੋ;
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        tpm20_menu();
        ਰੋਕੋ;
    }
}

ਪੂਰਨ
tpm_can_show_menu(ਖਾਲੀ)
{
    ਚੋਣ (TPM_ਸੰਸਕਰਣ) {
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_1_2:
        ਵਾਪਸ tpm_is_working() && TPM_has_physical_presence;
    ਹਾਲਤ TPM_ਸੰਸਕਰਣ_2:
        ਵਾਪਸ tpm_is_working();
    }
    ਵਾਪਸ 0;
}
