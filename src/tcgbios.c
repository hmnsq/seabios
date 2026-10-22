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

#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "byteorder.h" // cpu_to_*
#అన్తర్భావ "config.h" // CONFIG_TCGBIOS
#అన్తర్భావ "farptr.h" // MAKE_FLATPTR
#అన్తర్భావ "fw/paravirt.h" // runningOnXen
#అన్తర్భావ "hw/tpm_drivers.h" // tpm_drivers[]
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "sha.h" // sha1, sha256, ...
#అన్తర్భావ "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#అన్తర్భావ "std/smbios.h" // struct smbios_21_entry_point
#అన్తర్భావ "std/tcg.h" // TCG_PC_LOGOVERFLOW
#అన్తర్భావ "string.h" // checksum
#అన్తర్భావ "tcgbios.h"// tpm_*, prototypes
#అన్తర్భావ "util.h" // printf, get_keystroke
#అన్తర్భావ "stacks.h" // wait_threads, reset
#అన్తర్భావ "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

నిర్మాణం {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_నిడివి;

    /* start address of TCPA log buffer */
    u8 *          log_area_ప్రారంభం_చిరునామా;

    /* number of log entries written */
    u32           నమోదు_లెక్క;

    /* address to write next log entry to */
    u8 *          log_area_తదుపరి_నమోదు;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_చివరి_నమోదు;
} tpm_state VARLOW;

స్థిర పూర్ణాంకం tpm_సమితి_log_area(u8 *log_area_ప్రారంభం_చిరునామా,
                            u32 log_area_minimum_నిడివి)
{
    ఒకవేళ (!log_area_ప్రారంభం_చిరునామా || !log_area_minimum_నిడివి)
        తిరిగి -1;

    memset(log_area_ప్రారంభం_చిరునామా, 0, log_area_minimum_నిడివి);
    tpm_state.log_area_ప్రారంభం_చిరునామా = log_area_ప్రారంభం_చిరునామా;
    tpm_state.log_area_minimum_నిడివి = log_area_minimum_నిడివి;
    tpm_state.log_area_తదుపరి_నమోదు = log_area_ప్రారంభం_చిరునామా;
    tpm_state.log_area_చివరి_నమోదు = ఖాళీ;
    tpm_state.నమోదు_లెక్క = 0;
    తిరిగి 0;
}

స్థిర పూర్ణాంకం
tpm_tcpa_probe(శూన్య)
{
    నిర్మాణం tcpa_descriptor_rev2 *tcpa = కనుగొను_acpi_పట్టిక(TCPA_SIGNATURE);
    ఒకవేళ (!tcpa)
        తిరిగి -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(దీర్ఘ)tcpa->log_area_ప్రారంభం_చిరునామా,
            tcpa->log_area_minimum_నిడివి);

    తిరిగి tpm_సమితి_log_area((u8*)(దీర్ఘ)tcpa->log_area_ప్రారంభం_చిరునామా,
                            tcpa->log_area_minimum_నిడివి);
}

స్థిర పూర్ణాంకం
tpm_tpm2_probe(శూన్య)
{
    నిర్మాణం tpm2_descriptor_rev2 *tpm2 = కనుగొను_acpi_పట్టిక(TPM2_SIGNATURE);
    ఒకవేళ (!tpm2)
        తిరిగి -1;

    ఒకవేళ (tpm2->నిడివి < 76)
        తిరిగి -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(దీర్ఘ)tpm2->log_area_ప్రారంభం_చిరునామా,
            tpm2->log_area_minimum_నిడివి);

    తిరిగి tpm_సమితి_log_area((u8*)(దీర్ఘ)tpm2->log_area_ప్రారంభం_చిరునామా,
                            tpm2->log_area_minimum_నిడివి);
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
స్థిర పూర్ణాంకం
tpm_log_event(నిర్మాణం tpm_log_శీర్షిక *నమోదు_2, పూర్ణాంకం digest_len
              , స్థిరరాశి శూన్య *event, పూర్ణాంకం event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_ప్రారంభం_చిరునామా, tpm_state.log_area_తదుపరి_నమోదు);

    ఒకవేళ (tpm_state.log_area_తదుపరి_నమోదు == ఖాళీ)
        తిరిగి -1;

    u32 కొలత = (పరిమాణం(*నమోదు_2) + digest_len
                + పరిమాణం(నిర్మాణం tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_తదుపరి_నమోదు + కొలత
                   - tpm_state.log_area_ప్రారంభం_చిరునామా);
    ఒకవేళ (logsize > tpm_state.log_area_minimum_నిడివి) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", కొలత);
        తిరిగి -1;
    }

    శూన్య *dest = tpm_state.log_area_తదుపరి_నమోదు;
    జ్ఞాపకం_నకలు(dest, నమోదు_2, పరిమాణం(*నమోదు_2) + digest_len);
    నిర్మాణం tpm_log_trailer *t = dest + పరిమాణం(*నమోదు_2) + digest_len;
    t->eventdatasize = event_len;
    జ్ఞాపకం_నకలు(t->event, event, event_len);

    tpm_state.log_area_చివరి_నమోదు = tpm_state.log_area_తదుపరి_నమోదు;
    tpm_state.log_area_తదుపరి_నమోదు += కొలత;
    tpm_state.నమోదు_లెక్క++;

    తిరిగి 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

స్థిర TPMVersion TPM_సంస్కరణ;
స్థిర u32 tpm20_pcr_selection_కొలత;
స్థిర నిర్మాణం tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
నిర్మాణం tpm_log_నమోదు {
    నిర్మాణం tpm_log_శీర్షిక hdr;
    u8 pad[పరిమాణం(నిర్మాణం tpm2_digest_విలువలు)
           + 8 * పరిమాణం(నిర్మాణం tpm2_digest_విలువ)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

స్థిర స్థిరరాశి నిర్మాణం hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    స్థిరరాశి అక్షరం *name;
    శూన్య (*hashfunc)(స్థిరరాశి u8 *దత్తాంశం, u32 నిడివి, u8 *hash);
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

స్థిర పూర్ణాంకం
tpm20_get_hash_buffersize(u16 hashAlg)
{
    గుర్తులేని i;

    కొరకు (i = 0; i < ARRAY_కొలత(hash_parameters); i++) {
        ఒకవేళ (hash_parameters[i].hashalg == hashAlg)
            తిరిగి hash_parameters[i].hash_buffersize;
    }
    తిరిగి -1;
}

స్థిర u8
tpm20_hashalg_గమ్యం_flag(u16 hashAlg)
{
    గుర్తులేని i;

    కొరకు (i = 0; i < ARRAY_కొలత(hash_parameters); i++) {
        ఒకవేళ (hash_parameters[i].hashalg == hashAlg)
            తిరిగి hash_parameters[i].hashalg_flag;
    }
    తిరిగి 0;
}

స్థిర u16
tpm20_hashalg_flag_గమ్యం_hashalg(u8 hashalg_flag)
{
    గుర్తులేని i;

    కొరకు (i = 0; i < ARRAY_కొలత(hash_parameters); i++) {
        ఒకవేళ (hash_parameters[i].hashalg_flag == hashalg_flag)
            తిరిగి hash_parameters[i].hashalg;
    }
    తిరిగి 0;
}

స్థిర స్థిరరాశి అక్షరం *
tpm20_hashalg_flag_గమ్యం_name(u8 hashalg_flag)
{
    గుర్తులేని i;

    కొరకు (i = 0; i < ARRAY_కొలత(hash_parameters); i++) {
        ఒకవేళ (hash_parameters[i].hashalg_flag == hashalg_flag)
            తిరిగి hash_parameters[i].name;
    }
    తిరిగి ఖాళీ;
}

స్థిర శూన్య tpm2_hash_దత్తాంశం(u16 hashAlg, స్థిరరాశి u8 *దత్తాంశం, u32 దత్తాంశం_len, u8 *hash)
{
    గుర్తులేని i;

    కొరకు (i = 0; i < ARRAY_కొలత(hash_parameters); i++) {
        ఒకవేళ (hash_parameters[i].hashalg == hashAlg) {
            ఒకవేళ (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(దత్తాంశం, దత్తాంశం_len, hash);
            } లేకపోతే {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
స్థిర పూర్ణాంకం
tpm20_write_EfiSpecIdEventStruct(శూన్య)
{
    ఒకవేళ (!tpm20_pcr_selection)
        తిరిగి -1;

    నిర్మాణం {
        నిర్మాణం TCG_EfiSpecIdEventStruct hdr;
        u8 pad[పరిమాణం(నిర్మాణం tpm_log_నమోదు) + పరిమాణం(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.spec_సంస్కరణ_Minor = 0,
        .hdr.spec_సంస్కరణ_Major = 2,
        .hdr.specErrata = 2,
        .hdr.uintn_కొలత = 2,
    };

    నిర్మాణం tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    శూన్య *nsel, *చివర = (శూన్య*)tpm20_pcr_selection + tpm20_pcr_selection_కొలత;

    u32 లెక్క, అంకె_Algs = 0;
    కొరకు (లెక్క = 0; లెక్క < be32_గమ్యం_cpu(tpm20_pcr_selection->లెక్క); లెక్క++) {
        u8 కొలత_యొక్క_Select = sel->కొలత_యొక్క_Select;

        nsel = (శూన్య*)sel + పరిమాణం(*sel) + కొలత_యొక్క_Select;
        ఒకవేళ (nsel > చివర)
            ఆపు;

        ఒకవేళ (!కొలత_యొక్క_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            కొనసాగు;
        }

        పూర్ణాంకం hsize = tpm20_get_hash_buffersize(be16_గమ్యం_cpu(sel->hashAlg));
        ఒకవేళ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_గమ్యం_cpu(sel->hashAlg));
            తిరిగి -1;
        }

        పూర్ణాంకం event_కొలత = offsetof(నిర్మాణం TCG_EfiSpecIdEventStruct
                                  , digestSizes[లెక్క+1]);
        ఒకవేళ (event_కొలత > పరిమాణం(event) - పరిమాణం(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            తిరిగి -1;
        }

        event.hdr.digestSizes[అంకె_Algs].algorithmId = be16_గమ్యం_cpu(sel->hashAlg);
        event.hdr.digestSizes[అంకె_Algs].digest_కొలత = hsize;
        అంకె_Algs++;

        sel = nsel;
    }

    ఒకవేళ (sel != చివర) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        తిరిగి -1;
    }

    event.hdr.సంఖ్య_యొక్క_Algorithms = అంకె_Algs;
    పూర్ణాంకం event_కొలత = offsetof(నిర్మాణం TCG_EfiSpecIdEventStruct
                              , digestSizes[అంకె_Algs]);
    u8 *vendor_Info_కొలత = (శూన్య*)&event + event_కొలత;
    *vendor_Info_కొలత = 0;
    event_కొలత += పరిమాణం(*vendor_Info_కొలత);

    నిర్మాణం tpm_log_నమోదు le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    తిరిగి tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_కొలత);
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
స్థిర పూర్ణాంకం
tpm20_build_digest(నిర్మాణం tpm_log_నమోదు *le,
                   స్థిరరాశి u8 *hashdata, u32 hashdata_len, పూర్ణాంకం bigEndian)
{
    ఒకవేళ (!tpm20_pcr_selection)
        తిరిగి -1;

    నిర్మాణం tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    శూన్య *nsel, *చివర = (శూన్య*)tpm20_pcr_selection + tpm20_pcr_selection_కొలత;
    శూన్య *dest = le->hdr.digest + పరిమాణం(నిర్మాణం tpm2_digest_విలువలు);

    u32 లెక్క, అంకె_Algs = 0;
    కొరకు (లెక్క = 0; లెక్క < be32_గమ్యం_cpu(tpm20_pcr_selection->లెక్క); లెక్క++) {
        u8 కొలత_యొక్క_Select = sel->కొలత_యొక్క_Select;

        nsel = (శూన్య*)sel + పరిమాణం(*sel) + కొలత_యొక్క_Select;
        ఒకవేళ (nsel > చివర)
            ఆపు;

        /* PCR 0-7 unused? -- skip */
        ఒకవేళ (!కొలత_యొక్క_Select || sel->pcrSelect[0] == 0) {
            sel = nsel;
            కొనసాగు;
        }

        పూర్ణాంకం hsize = tpm20_get_hash_buffersize(be16_గమ్యం_cpu(sel->hashAlg));
        ఒకవేళ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_గమ్యం_cpu(sel->hashAlg));
            తిరిగి -1;
        }

        /* buffer size sanity check before writing */
        నిర్మాణం tpm2_digest_విలువ *v = dest;
        ఒకవేళ (dest + పరిమాణం(*v) + hsize > (శూన్య*)le + పరిమాణం(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            తిరిగి -1;
        }

        ఒకవేళ (bigEndian)
            v->hashAlg = sel->hashAlg;
        లేకపోతే
            v->hashAlg = be16_గమ్యం_cpu(sel->hashAlg);

        tpm2_hash_దత్తాంశం(be16_గమ్యం_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += పరిమాణం(*v) + hsize;
        sel = nsel;

        అంకె_Algs++;
    }

    ఒకవేళ (sel != చివర) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        తిరిగి -1;
    }

    నిర్మాణం tpm2_digest_విలువలు *v = (శూన్య*)le->hdr.digest;
    ఒకవేళ (bigEndian)
        v->లెక్క = cpu_గమ్యం_be32(అంకె_Algs);
    లేకపోతే
        v->లెక్క = అంకె_Algs;

    తిరిగి dest - (శూన్య*)le->hdr.digest;
}

స్థిర పూర్ణాంకం
tpm12_build_digest(నిర్మాణం tpm_log_నమోదు *le,
                   స్థిరరాశి u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    తిరిగి SHA1_BUFSIZE;
}

స్థిర పూర్ణాంకం
tpm12_build_digest_ప్రత్యక్ష(నిర్మాణం tpm_log_నమోదు *le, స్థిరరాశి u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    జ్ఞాపకం_నకలు(le->hdr.digest, sha1, SHA1_BUFSIZE);
    తిరిగి SHA1_BUFSIZE;
}

స్థిర పూర్ణాంకం
tpm_build_digest(నిర్మాణం tpm_log_నమోదు *le, స్థిరరాశి u8 *hashdata, u32 hashdata_len
                 , పూర్ణాంకం bigEndian)
{
    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        తిరిగి tpm12_build_digest(le, hashdata, hashdata_len);
    స్థితి TPM_సంస్కరణ_2:
        తిరిగి tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    తిరిగి -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
స్థిర పూర్ణాంకం
tpm_simple_cmd(u8 locty, u32 ordinal
               , పూర్ణాంకం param_కొలత, u16 param, గణన tpmDurationType గమ్యం_t)
{
    నిర్మాణం {
        నిర్మాణం tpm_req_శీర్షిక trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_గమ్యం_be32(పరిమాణం(req.trqh) + param_కొలత),
        .trqh.ordinal = cpu_గమ్యం_be32(ordinal),
        .param = param_కొలత == 2 ? cpu_గమ్యం_be16(param) : param,
    };
    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        req.trqh.ట్యాగ్ = cpu_గమ్యం_be16(TPM_ట్యాగ్_RQU_CMD);
        ఆపు;
    స్థితి TPM_సంస్కరణ_2:
        req.trqh.ట్యాగ్ = cpu_గమ్యం_be16(TPM2_ST_NO_SESSIONS);
        ఆపు;
    }

    u8 obuffer[64];
    నిర్మాణం tpm_rsp_శీర్షిక *trsh = (శూన్య*)obuffer;
    u32 obuffer_len = పరిమాణం(obuffer);
    memset(obuffer, 0x0, పరిమాణం(obuffer));

    పూర్ణాంకం ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, గమ్యం_t);
    ret = ret ? -1 : be32_గమ్యం_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_getcapability(u32 capability, u32 property, u32 లెక్క,
                    నిర్మాణం tpm_rsp_శీర్షిక *rsp, u32 rsize)
{
    నిర్మాణం tpm2_req_getcapability trg = {
        .hdr.ట్యాగ్ = cpu_గమ్యం_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_గమ్యం_be32(పరిమాణం(trg)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_GetCapability),
        .capability = cpu_గమ్యం_be32(capability),
        .property = cpu_గమ్యం_be32(property),
        .propertycount = cpu_గమ్యం_be32(లెక్క),
    };

    u32 resp_కొలత = rsize;
    పూర్ణాంకం ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_కొలత,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_గమ్యం_cpu(rsp->totlen)) ? -1 : be32_గమ్యం_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_get_pcrbanks(శూన్య)
{
    u8 బఫరు[128];
    నిర్మాణం tpm2_res_getcapability *trg =
      (నిర్మాణం tpm2_res_getcapability *)&బఫరు;

    పూర్ణాంకం ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  పరిమాణం(బఫరు));
    ఒకవేళ (ret)
        తిరిగి ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_గమ్యం_cpu(trg->hdr.totlen);
    ఒకవేళ (resplen <= offsetof(నిర్మాణం tpm2_res_getcapability, దత్తాంశం))
        తిరిగి -1;

    u32 కొలత = resplen - offsetof(నిర్మాణం tpm2_res_getcapability, దత్తాంశం);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    ఒకవేళ (కొలత < offsetof(నిర్మాణం tpml_pcr_selection, selections) +
               offsetof(నిర్మాణం tpms_pcr_selection, pcrSelect))
        తిరిగి -1;

    tpm20_pcr_selection = malloc_ఎగువ(కొలత);
    ఒకవేళ (tpm20_pcr_selection) {
        జ్ఞాపకం_నకలు(tpm20_pcr_selection, &trg->దత్తాంశం, కొలత);
        tpm20_pcr_selection_కొలత = కొలత;
    } లేకపోతే {
        warn_noalloc();
        ret = -1;
    }

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *చురుకైన_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *చురుకైన_pcrbanks = 0;

    ఒకవేళ (!tpm20_pcr_selection)
        తిరిగి -1;

    నిర్మాణం tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    శూన్య *చివర = (శూన్య*)tpm20_pcr_selection + tpm20_pcr_selection_కొలత;

    అయితే (1) {
        u8 కొలత_యొక్క_Select = sel->కొలత_యొక్క_Select;
        శూన్య *nsel = (శూన్య*)sel + పరిమాణం(*sel) + కొలత_యొక్క_Select;
        ఒకవేళ (nsel > చివర)
            తిరిగి 0;

        u16 hashalg = be16_గమ్యం_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_గమ్యం_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        గుర్తులేని i;
        కొరకు (i = 0; i < కొలత_యొక్క_Select; i++) {
            ఒకవేళ (sel->pcrSelect[i]) {
                *చురుకైన_pcrbanks |= hashalg_flag;
                ఆపు;
            }
        }

        sel = nsel;
    }
}

స్థిర పూర్ణాంకం
tpm20_సమితి_pcrbanks(u32 చురుకైన_banks)
{
    నిర్మాణం tpm2_req_pcr_allocate trpa = {
        .hdr.ట్యాగ్     = cpu_గమ్యం_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_గమ్యం_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_గమ్యం_be32(పరిమాణం(trpa.authblock)),
        .authblock = {
            .handle = cpu_గమ్యం_be32(TPM2_RS_PW),
            .noncesize = cpu_గమ్యం_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_గమ్యం_be16(0),
        },
    };
    నిర్మాణం tpms_pcr_selection3 {
        u16 hashAlg;
        u8 కొలత_యొక్క_Select;
        u8 pcrSelect[3];
    } tps[ARRAY_కొలత(trpa.tpms_pcr_selections)];
    పూర్ణాంకం i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    అయితే (hashalg_flag) {
        ఒకవేళ ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_గమ్యం_hashalg(hashalg_flag);

            ఒకవేళ (hashalg) {
                u8 ముసుగు = 0;
                tps[i].hashAlg = cpu_గమ్యం_be16(hashalg);
                tps[i].కొలత_యొక్క_Select = 3;

                ఒకవేళ (చురుకైన_banks & hashalg_flag)
                    ముసుగు = 0xff;

                tps[i].pcrSelect[0] = ముసుగు;
                tps[i].pcrSelect[1] = ముసుగు;
                tps[i].pcrSelect[2] = ముసుగు;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.లెక్క = cpu_గమ్యం_be32(i);
    జ్ఞాపకం_నకలు(trpa.tpms_pcr_selections, tps, i * పరిమాణం(tps[0]));
    trpa.hdr.totlen = cpu_గమ్యం_be32(offsetof(నిర్మాణం tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * పరిమాణం(tps[0]));

    నిర్మాణం tpm_rsp_శీర్షిక rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);

    పూర్ణాంకం ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_గమ్యం_cpu(rsp.errcode);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం tpm20_activate_pcrbanks(u32 చురుకైన_banks)
{
    పూర్ణాంకం ret = tpm20_సమితి_pcrbanks(చురుకైన_banks);
    ఒకవేళ (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (!ret)
        reset();
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm12_get_capability(u32 cap, u32 subcap, నిర్మాణం tpm_rsp_శీర్షిక *rsp, u32 rsize)
{
    నిర్మాణం tpm_req_getcap trgc = {
        .hdr.ట్యాగ్ = cpu_గమ్యం_be16(TPM_ట్యాగ్_RQU_CMD),
        .hdr.totlen = cpu_గమ్యం_be32(పరిమాణం(trgc)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM_ORD_GetCapability),
        .capArea = cpu_గమ్యం_be32(cap),
        .sub_Cap_కొలత = cpu_గమ్యం_be32(పరిమాణం(trgc.subCap)),
        .subCap = cpu_గమ్యం_be32(subcap)
    };
    u32 resp_కొలత = rsize;
    పూర్ణాంకం ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_కొలత,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_కొలత != rsize) ? -1 : be32_గమ్యం_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm12_read_permanent_flags(అక్షరం *buf, పూర్ణాంకం buf_len)
{
    memset(buf, 0, buf_len);

    నిర్మాణం tpm_res_getcap_perm_flags pf;
    పూర్ణాంకం ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, పరిమాణం(pf));
    ఒకవేళ (ret)
        తిరిగి -1;

    జ్ఞాపకం_నకలు(buf, &pf.perm_flags, buf_len);

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
tpm12_determine_timeouts(శూన్య)
{
    నిర్మాణం tpm_res_getcap_timeouts timeouts;
    పూర్ణాంకం ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, పరిమాణం(timeouts));
    ఒకవేళ (ret)
        తిరిగి ret;

    నిర్మాణం tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, పరిమాణం(durations));
    ఒకవేళ (ret)
        తిరిగి ret;

    పూర్ణాంకం i;
    కొరకు (i = 0; i < 3; i++)
        durations.durations[i] = be32_గమ్యం_cpu(durations.durations[i]);

    కొరకు (i = 0; i < 4; i++)
        timeouts.timeouts[i] = be32_గమ్యం_cpu(timeouts.timeouts[i]);

    dprintf(DEBUG_tcg, "TCGBIOS: timeouts: %u %u %u %u\n",
            timeouts.timeouts[0],
            timeouts.timeouts[1],
            timeouts.timeouts[2],
            timeouts.timeouts[3]);

    dprintf(DEBUG_tcg, "TCGBIOS: durations: %u %u %u\n",
            durations.durations[0],
            durations.durations[1],
            durations.durations[2]);

    tpmhw_సమితి_timeouts(timeouts.timeouts, durations.durations);

    తిరిగి 0;
}

స్థిర శూన్య
tpm20_సమితి_timeouts(శూన్య)
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

    tpmhw_సమితి_timeouts(timeouts, durations);
}

స్థిర పూర్ణాంకం
tpm12_extend(నిర్మాణం tpm_log_నమోదు *le, పూర్ణాంకం digest_len)
{
    నిర్మాణం tpm_req_extend tre = {
        .hdr.ట్యాగ్     = cpu_గమ్యం_be16(TPM_ట్యాగ్_RQU_CMD),
        .hdr.totlen  = cpu_గమ్యం_be32(పరిమాణం(tre)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_గమ్యం_be32(le->hdr.pcrindex),
    };
    జ్ఞాపకం_నకలు(tre.digest, le->hdr.digest, పరిమాణం(tre.digest));

    నిర్మాణం tpm_rsp_extend rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);
    పూర్ణాంకం ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.hdr.errcode)
        తిరిగి -1;

    తిరిగి 0;
}

స్థిర పూర్ణాంకం tpm20_extend(నిర్మాణం tpm_log_నమోదు *le, పూర్ణాంకం digest_len)
{
    నిర్మాణం tpm2_req_extend tmp_tre = {
        .hdr.ట్యాగ్     = cpu_గమ్యం_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_గమ్యం_be32(0),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_గమ్యం_be32(le->hdr.pcrindex),
        .authblocksize = cpu_గమ్యం_be32(పరిమాణం(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_గమ్యం_be32(TPM2_RS_PW),
            .noncesize = cpu_గమ్యం_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_గమ్యం_be16(0),
        },
    };
    u8 బఫరు[పరిమాణం(tmp_tre) + పరిమాణం(le->pad)];
    నిర్మాణం tpm2_req_extend *tre = (నిర్మాణం tpm2_req_extend *)బఫరు;

    జ్ఞాపకం_నకలు(tre, &tmp_tre, పరిమాణం(tmp_tre));
    జ్ఞాపకం_నకలు(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_గమ్యం_be32(పరిమాణం(tmp_tre) + digest_len);

    నిర్మాణం tpm_rsp_శీర్షిక rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);
    పూర్ణాంకం ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.errcode)
        తిరిగి -1;

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
tpm_extend(నిర్మాణం tpm_log_నమోదు *le, పూర్ణాంకం digest_len)
{
    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        తిరిగి tpm12_extend(le, digest_len);
    స్థితి TPM_సంస్కరణ_2:
        తిరిగి tpm20_extend(le, digest_len);
    }
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
tpm20_stirrandom(శూన్య)
{
    నిర్మాణం tpm2_req_stirrandom stir = {
        .hdr.ట్యాగ్ = cpu_గమ్యం_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_గమ్యం_be32(పరిమాణం(stir)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_StirRandom),
        .కొలత = cpu_గమ్యం_be16(పరిమాణం(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    నిర్మాణం tpm_rsp_శీర్షిక rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);
    పూర్ణాంకం ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    నిర్మాణం tpm2_res_getrandom rsp;

    ఒకవేళ (buf_len > పరిమాణం(rsp.rnd.బఫరు))
        తిరిగి -1;

    నిర్మాణం tpm2_req_getrandom trgr = {
        .hdr.ట్యాగ్ = cpu_గమ్యం_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_గమ్యం_be32(పరిమాణం(trgr)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_GetRandom),
        .బైట్లు_Requested = cpu_గమ్యం_be16(buf_len),
    };
    u32 resp_నిడివి = పరిమాణం(rsp);

    పూర్ణాంకం ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_MEDIUM);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.hdr.errcode)
        ret = -1;
    లేకపోతే
        జ్ఞాపకం_నకలు(buf, rsp.rnd.బఫరు, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    నిర్మాణం tpm2_req_hierarchycontrol trh = {
        .hdr.ట్యాగ్ = cpu_గమ్యం_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_గమ్యం_be32(పరిమాణం(trh)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_గమ్యం_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_గమ్యం_be32(పరిమాణం(trh.authblock)),
        .authblock = {
            .handle = cpu_గమ్యం_be32(TPM2_RS_PW),
            .noncesize = cpu_గమ్యం_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_గమ్యం_be16(0),
        },
        .enable = cpu_గమ్యం_be32(hierarchy),
        .state = state,
    };
    నిర్మాణం tpm_rsp_శీర్షిక rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);
    పూర్ణాంకం ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_MEDIUM);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_hierarchychangeauth(u8 auth[20])
{
    నిర్మాణం tpm2_req_hierarchychangeauth trhca = {
        .hdr.ట్యాగ్ = cpu_గమ్యం_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_గమ్యం_be32(పరిమాణం(trhca)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_గమ్యం_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_గమ్యం_be32(పరిమాణం(trhca.authblock)),
        .authblock = {
            .handle = cpu_గమ్యం_be32(TPM2_RS_PW),
            .noncesize = cpu_గమ్యం_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_గమ్యం_be16(0),
        },
        .newAuth = {
            .కొలత = cpu_గమ్యం_be16(పరిమాణం(trhca.newAuth.బఫరు)),
        },
    };
    జ్ఞాపకం_నకలు(trhca.newAuth.బఫరు, auth, పరిమాణం(trhca.newAuth.బఫరు));

    నిర్మాణం tpm_rsp_శీర్షిక rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);
    పూర్ణాంకం ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_MEDIUM);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    తిరిగి ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

స్థిర పూర్ణాంకం TPM_has_physical_presence;
u8 TPM_working VARLOW;

స్థిర పూర్ణాంకం
tpm_is_working(శూన్య)
{
    తిరిగి CONFIG_TCGBIOS && TPM_working;
}

స్థిర శూన్య
tpm_సమితి_failure(శూన్య)
{
    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_సమితి_తాత్కాలిక_Deactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        ఆపు;
    స్థితి TPM_సంస్కరణ_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        ఆపు;
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
స్థిర శూన్య
tpm_కూడు_measurement_గమ్యం_log(u32 pcrindex, u32 event_type,
                           స్థిరరాశి అక్షరం *event, u32 event_నిడివి,
                           స్థిరరాశి u8 *hashdata, u32 hashdata_నిడివి)
{
    ఒకవేళ (!tpm_is_working())
        తిరిగి;

    నిర్మాణం tpm_log_నమోదు le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    పూర్ణాంకం digest_len = tpm_build_digest(&le, hashdata, hashdata_నిడివి, 1);
    ఒకవేళ (digest_len < 0)
        తిరిగి;
    పూర్ణాంకం ret = tpm_extend(&le, digest_len);
    ఒకవేళ (ret) {
        tpm_సమితి_failure();
        తిరిగి;
    }
    tpm_build_digest(&le, hashdata, hashdata_నిడివి, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_నిడివి);
}

// Add an EV_ACTION measurement to the list of measurements
స్థిర శూన్య
tpm_కూడు_action(u32 pcr_సూచీ, స్థిరరాశి అక్షరం *string)
{
    u32 len = అక్షరశ్రేణి_పొడవు(string);
    tpm_కూడు_measurement_గమ్యం_log(pcr_సూచీ, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
స్థిర శూన్య
tpm_కూడు_event_separators(శూన్య)
{
    స్థిర స్థిరరాశి u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcr_సూచీ;
    కొరకు (pcr_సూచీ = 0; pcr_సూచీ <= 7; pcr_సూచీ++)
        tpm_కూడు_measurement_గమ్యం_log(pcr_సూచీ, EV_SEPARATOR,
                                   (స్థిరరాశి అక్షరం *)evt_separator,
                                   పరిమాణం(evt_separator),
                                   evt_separator,
                                   పరిమాణం(evt_separator));
}

స్థిర శూన్య
tpm_smbios_measure(శూన్య)
{
    నిర్మాణం pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    శూన్య *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    ఒకవేళ (!smbios_tables)
        తిరిగి;

    sha1((స్థిరరాశి u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_కూడు_measurement_గమ్యం_log(1,
                               EV_EVENT_ట్యాగ్,
                               (స్థిరరాశి అక్షరం *)&pcctes, పరిమాణం(pcctes),
                               (u8 *)&pcctes, పరిమాణం(pcctes));
}

స్థిర పూర్ణాంకం
tpm12_assert_physical_presence(శూన్య)
{
    పూర్ణాంకం ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (!ret)
        తిరిగి 0;

    నిర్మాణం tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((అక్షరం *)&pf, పరిమాణం(pf));
    ఒకవేళ (ret)
        తిరిగి -1;

    /* check if hardware physical presence is supported */
    ఒకవేళ (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        తిరిగి 0;
    }

    ఒకవేళ (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_తాళం]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        తిరిగి tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
tpm12_startup(శూన్య)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    పూర్ణాంకం ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    ఒకవేళ (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_Self_Test_నిండిన,
                         0, 0, TPM_DURATION_TYPE_LONG);
    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_Reset_Establishment_బిట్టు,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret && ret != TPM_BAD_LOCALITY)
        వెళ్ళు err_exit;

    తిరిగి 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_సమితి_failure();
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
tpm20_startup(శూన్య)
{
    tpm20_సమితి_timeouts();

    పూర్ణాంకం ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    ఒకవేళ (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    ret = tpm20_get_pcrbanks();
    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    తిరిగి 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_సమితి_failure();
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
tpm_startup(శూన్య)
{
    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        తిరిగి tpm12_startup();
    స్థితి TPM_సంస్కరణ_2:
        తిరిగి tpm20_startup();
    }
    తిరిగి -1;
}

శూన్య
tpm_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి;

    పూర్ణాంకం ret = tpm_tpm2_probe();
    ఒకవేళ (ret) {
        ret = tpm_tcpa_probe();
        ఒకవేళ (ret)
            తిరిగి;
    }

    TPM_సంస్కరణ = tpmhw_probe();
    ఒకవేళ (TPM_సంస్కరణ == TPM_సంస్కరణ_NONE)
        తిరిగి;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_సంస్కరణ == TPM_సంస్కరణ_1_2) ? "1.2" : "2");

    TPM_working = 1;

    ఒకవేళ (runningOnXen())
        తిరిగి;

    ret = tpm_startup();
    ఒకవేళ (ret)
        తిరిగి;

    tpm_smbios_measure();
    tpm_కూడు_action(2, "Start Option ROM Scan");
}

స్థిర శూన్య
tpm20_prepboot(శూన్య)
{
    పూర్ణాంకం ret = tpm20_stirrandom();
    ఒకవేళ (ret)
         వెళ్ళు err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], పరిమాణం(auth));
    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    తిరిగి;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_సమితి_failure();
}

శూన్య
tpm_prepboot(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి;

    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        ఒకవేళ (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_తాళం, TPM_DURATION_TYPE_SHORT);
        ఆపు;
    స్థితి TPM_సంస్కరణ_2:
        tpm20_prepboot();
        ఆపు;
    }

    tpm_కూడు_action(4, "Calling INT 19h");
    tpm_కూడు_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
శూన్య
tpm_option_rom(స్థిరరాశి శూన్య *addr, u32 len)
{
    ఒకవేళ (!tpm_is_working())
        తిరిగి;

    నిర్మాణం pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = పరిమాణం(u16) + పరిమాణం(u16) + SHA1_BUFSIZE,
    };
    sha1((స్థిరరాశి u8 *)addr, len, pcctes.digest);
    tpm_కూడు_measurement_గమ్యం_log(2,
                               EV_EVENT_ట్యాగ్,
                               (స్థిరరాశి అక్షరం *)&pcctes, పరిమాణం(pcctes),
                               (u8 *)&pcctes, పరిమాణం(pcctes));
}

శూన్య
tpm_కూడు_bcv(u32 bootdrv, స్థిరరాశి u8 *addr, u32 నిడివి)
{
    ఒకవేళ (!tpm_is_working())
        తిరిగి;

    ఒకవేళ (నిడివి < 0x200)
        తిరిగి;

    స్థిరరాశి అక్షరం *string = "Booting BCV device 00h (Floppy)";
    ఒకవేళ (bootdrv == 0x80)
        string = "Booting BCV device 80h (HDD)";
    tpm_కూడు_action(4, string);

    /* specs: see section 'Hard Disk Device or Hard Disk-Like Devices' */
    /* equivalent to: dd if=/dev/hda ibs=1 count=440 | sha1sum */
    string = "MBR";
    tpm_కూడు_measurement_గమ్యం_log(4, EV_IPL,
                               string, అక్షరశ్రేణి_పొడవు(string),
                               addr, 0x1b8);

    /* equivalent to: dd if=/dev/hda ibs=1 count=72 skip=440 | sha1sum */
    string = "MBR PARTITION_TABLE";
    tpm_కూడు_measurement_గమ్యం_log(5, EV_IPL_విభజన_దత్తాంశం,
                               string, అక్షరశ్రేణి_పొడవు(string),
                               addr + 0x1b8, 0x48);
}

శూన్య
tpm_కూడు_cdrom(u32 bootdrv, స్థిరరాశి u8 *addr, u32 నిడివి)
{
    ఒకవేళ (!tpm_is_working())
        తిరిగి;

    tpm_కూడు_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    స్థిరరాశి అక్షరం *string = "EL TORITO IPL";
    tpm_కూడు_measurement_గమ్యం_log(4, EV_IPL,
                               string, అక్షరశ్రేణి_పొడవు(string),
                               addr, నిడివి);
}

శూన్య
tpm_కూడు_cdrom_catalog(స్థిరరాశి u8 *addr, u32 నిడివి)
{
    ఒకవేళ (!tpm_is_working())
        తిరిగి;

    tpm_కూడు_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    స్థిరరాశి అక్షరం *string = "BOOT CATALOG";
    tpm_కూడు_measurement_గమ్యం_log(5, EV_IPL_విభజన_దత్తాంశం,
                               string, అక్షరశ్రేణి_పొడవు(string),
                               addr, నిడివి);
}

శూన్య
tpm_s3_resume(శూన్య)
{
    ఒకవేళ (!tpm_is_working())
        తిరిగి;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    పూర్ణాంకం ret = -1;

    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        ఆపు;
    స్థితి TPM_సంస్కరణ_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        ఒకవేళ (ret)
            వెళ్ళు err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        ఆపు;
    }

    ఒకవేళ (ret)
        వెళ్ళు err_exit;

    తిరిగి;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_సమితి_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

స్థిర అంతఃవరుస శూన్య *ఆదానం_buf32(నిర్మాణం bregs *regs)
{
    తిరిగి MAKE_FLATPTR(regs->es, regs->di);
}

స్థిర అంతఃవరుస శూన్య *output_buf32(నిర్మాణం bregs *regs)
{
    తిరిగి MAKE_FLATPTR(regs->ds, regs->si);
}

స్థిర u32
hash_log_extend(నిర్మాణం pcpes *pcpes, స్థిరరాశి శూన్య *hashdata, u32 hashdata_నిడివి
                , శూన్య *event, పూర్ణాంకం extend)
{
    ఒకవేళ (pcpes->pcrindex >= 24)
        తిరిగి TCG_INVALID_ఆదానం_PARA;
    ఒకవేళ (hashdata)
        sha1(hashdata, hashdata_నిడివి, pcpes->digest);

    నిర్మాణం tpm_log_నమోదు le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    పూర్ణాంకం digest_len = tpm12_build_digest_ప్రత్యక్ష(&le, pcpes->digest);
    ఒకవేళ (digest_len < 0)
        తిరిగి TCG_GENERAL_ERROR;
    ఒకవేళ (extend) {
        పూర్ణాంకం ret = tpm_extend(&le, digest_len);
        ఒకవేళ (ret)
            తిరిగి TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_ప్రత్యక్ష(&le, pcpes->digest);
    పూర్ణాంకం ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    ఒకవేళ (ret)
        తిరిగి TCG_PC_LOGOVERFLOW;
    తిరిగి 0;
}

స్థిర u32
hash_log_extend_event_int(స్థిరరాశి నిర్మాణం hleei_short *hleei_s,
                          నిర్మాణం hleeo *hleeo)
{
    u32 rc = 0;
    నిర్మాణం hleei_long *hleei_l = (నిర్మాణం hleei_long *)hleei_s;
    స్థిరరాశి శూన్య *logdataptr;
    u32 logdatalen;
    నిర్మాణం pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    ఎంపిక (hleei_s->ipblength) {
    స్థితి పరిమాణం(నిర్మాణం hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    ఆపు;

    స్థితి పరిమాణం(నిర్మాణం hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    ఆపు;

    అప్రమేయ:
        /* bad input block */
        rc = TCG_INVALID_ఆదానం_PARA;
        వెళ్ళు err_exit;
    }

    pcpes = (నిర్మాణం pcpes *)logdataptr;

    ఒకవేళ (pcpes->pcrindex != pcrindex
        || logdatalen != పరిమాణం(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ఆదానం_PARA;
        వెళ్ళు err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    ఒకవేళ (rc)
        వెళ్ళు err_exit;

    hleeo->opblength = పరిమాణం(నిర్మాణం hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.నమోదు_లెక్క;
    జ్ఞాపకం_నకలు(hleeo->digest, pcpes->digest, పరిమాణం(hleeo->digest));

err_exit:
    ఒకవేళ (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    తిరిగి rc;
}

స్థిర u32
pass_through_గమ్యం_tpm_int(నిర్మాణం pttti *pttti, నిర్మాణం pttto *pttto)
{
    u32 rc = 0;
    నిర్మాణం tpm_req_శీర్షిక *trh = (శూన్య*)pttti->tpmopin;

    ఒకవేళ (pttti->ipblength < పరిమాణం(నిర్మాణం pttti) + పరిమాణం(*trh)
        || pttti->ipblength != పరిమాణం(నిర్మాణం pttti) + be32_గమ్యం_cpu(trh->totlen)
        || pttti->opblength < పరిమాణం(నిర్మాణం pttto)) {
        rc = TCG_INVALID_ఆదానం_PARA;
        వెళ్ళు err_exit;
    }

    u16 ట్యాగ్ = be16_గమ్యం_cpu(trh->ట్యాగ్);

    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        ఒకవేళ (ట్యాగ్ != TPM_ట్యాగ్_RQU_CMD && ట్యాగ్ != TPM_ట్యాగ్_RQU_AUTH1_CMD
            && ట్యాగ్ != TPM_ట్యాగ్_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_ఆదానం_PARA;
            వెళ్ళు err_exit;
        }
        ఆపు;
    స్థితి TPM_సంస్కరణ_2:
        ఒకవేళ (ట్యాగ్ != TPM2_ST_NO_SESSIONS && ట్యాగ్ != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_ఆదానం_PARA;
            వెళ్ళు err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(నిర్మాణం pttto, tpmopout);
    పూర్ణాంకం ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    ఒకవేళ (ret) {
        rc = TCG_FATAL_COM_ERROR;
        వెళ్ళు err_exit;
    }

    pttto->opblength = offsetof(నిర్మాణం pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    ఒకవేళ (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    తిరిగి rc;
}

స్థిర u32
shutdown_preboot_interface(శూన్య)
{
    TPM_interface_shutdown = 1;
    తిరిగి 0;
}

స్థిర u32
hash_log_event_int(స్థిరరాశి నిర్మాణం hlei *hlei, నిర్మాణం hleo *hleo)
{
    u32 rc = 0;
    u16 కొలత;
    నిర్మాణం pcpes *pcpes;

    కొలత = hlei->ipblength;
    ఒకవేళ (కొలత != పరిమాణం(*hlei)) {
        rc = TCG_INVALID_ఆదానం_PARA;
        వెళ్ళు err_exit;
    }

    pcpes = (నిర్మాణం pcpes *)hlei->logdataptr;

    ఒకవేళ (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != పరిమాణం(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_ఆదానం_PARA;
        వెళ్ళు err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    ఒకవేళ (rc)
        వెళ్ళు err_exit;

    /* updating the log was fine */
    hleo->opblength = పరిమాణం(నిర్మాణం hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.నమోదు_లెక్క;

err_exit:
    ఒకవేళ (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    తిరిగి rc;
}

స్థిర u32
hash_all_int(స్థిరరాశి నిర్మాణం hai *hai, u8 *hash)
{
    ఒకవేళ (hai->ipblength != పరిమాణం(నిర్మాణం hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        తిరిగి TCG_INVALID_ఆదానం_PARA;

    sha1((స్థిరరాశి u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    తిరిగి 0;
}

స్థిర u32
tss_int(నిర్మాణం ti *ti, నిర్మాణం గమ్యం *గమ్యం)
{
    గమ్యం->opblength = పరిమాణం(నిర్మాణం గమ్యం);
    గమ్యం->reserved  = 0;

    తిరిగి TCG_PC_UNSUPPORTED;
}

స్థిర u32
compact_hash_log_extend_event_int(u8 *బఫరు,
                                  u32 info,
                                  u32 నిడివి,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    నిర్మాణం pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = పరిమాణం(info),
    };
    u32 rc = hash_log_extend(&pcpes, బఫరు, నిడివి, &info, 1);
    ఒకవేళ (rc)
        తిరిగి rc;

    *edx_ptr = tpm_state.నమోదు_లెక్క;
    తిరిగి 0;
}

శూన్య VISIBLE32FLAT
tpm_అంతరాయం_handler32(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి;

    సమితి_cf(regs, 0);

    ఒకవేళ (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        తిరిగి;
    }

    ఎంపిక ((గణన irq_ids)regs->al) {
    స్థితి TCG_స్థితి_Check:
        ఒకవేళ (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } లేకపోతే {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_సంస్కరణ_MAJOR;
            regs->cl = TCG_సంస్కరణ_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_ప్రారంభం_చిరునామా;
            regs->edi = (u32)tpm_state.log_area_చివరి_నమోదు;
        }
        ఆపు;

    స్థితి TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (నిర్మాణం hleei_short *)ఆదానం_buf32(regs),
                  (నిర్మాణం hleeo *)output_buf32(regs));
        ఆపు;

    స్థితి TCG_Pass_Through_గమ్యం_TPM:
        regs->eax =
            pass_through_గమ్యం_tpm_int((నిర్మాణం pttti *)ఆదానం_buf32(regs),
                                    (నిర్మాణం pttto *)output_buf32(regs));
        ఆపు;

    స్థితి TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        ఆపు;

    స్థితి TCG_HashLogEvent:
        regs->eax = hash_log_event_int((నిర్మాణం hlei*)ఆదానం_buf32(regs),
                                       (నిర్మాణం hleo*)output_buf32(regs));
        ఆపు;

    స్థితి TCG_HashAll:
        regs->eax =
            hash_all_int((నిర్మాణం hai*)ఆదానం_buf32(regs),
                          (u8 *)output_buf32(regs));
        ఆపు;

    స్థితి TCG_TSS:
        regs->eax = tss_int((నిర్మాణం ti*)ఆదానం_buf32(regs),
                            (నిర్మాణం గమ్యం*)output_buf32(regs));
        ఆపు;

    స్థితి TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)ఆదానం_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        ఆపు;

    అప్రమేయ:
        సమితి_cf(regs, 1);
    }

    తిరిగి;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

రకనిర్వచనం u8 tpm_ppi_సంకేతం;

స్థిర పూర్ణాంకం
tpm12_read_has_owner(పూర్ణాంకం *has_owner)
{
    నిర్మాణం tpm_res_getcap_ownerauth oauth;
    పూర్ణాంకం ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, పరిమాణం(oauth));
    ఒకవేళ (ret)
        తిరిగి -1;

    *has_owner = oauth.flag;

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
tpm12_enable_tpm(పూర్ణాంకం enable, పూర్ణాంకం verbose)
{
    నిర్మాణం tpm_permanent_flags pf;
    పూర్ణాంకం ret = tpm12_read_permanent_flags((అక్షరం *)&pf, పరిమాణం(pf));
    ఒకవేళ (ret)
        తిరిగి -1;

    ఒకవేళ (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        తిరిగి 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret) {
        ఒకవేళ (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        లేకపోతే
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm12_activate_tpm(పూర్ణాంకం activate, పూర్ణాంకం allow_reset, పూర్ణాంకం verbose)
{
    నిర్మాణం tpm_permanent_flags pf;
    పూర్ణాంకం ret = tpm12_read_permanent_flags((అక్షరం *)&pf, పరిమాణం(pf));
    ఒకవేళ (ret)
        తిరిగి -1;

    ఒకవేళ (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        తిరిగి 0;

    ఒకవేళ (pf.flags[PERM_FLAG_IDX_DISABLE])
        తిరిగి 0;

    ret = tpm_simple_cmd(0, TPM_ORD_Physical_సమితి_Deactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret)
        తిరిగి ret;

    ఒకవేళ (activate && allow_reset) {
        ఒకవేళ (verbose) {
            రాయి("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
tpm12_enable_activate(పూర్ణాంకం allow_reset, పూర్ణాంకం verbose)
{
    పూర్ణాంకం ret = tpm12_enable_tpm(1, verbose);
    ఒకవేళ (ret)
        తిరిగి ret;

    తిరిగి tpm12_activate_tpm(1, allow_reset, verbose);
}

స్థిర పూర్ణాంకం
tpm12_force_clear(పూర్ణాంకం enable_activate_before, పూర్ణాంకం enable_activate_after,
                  పూర్ణాంకం verbose)
{
    పూర్ణాంకం has_owner;
    పూర్ణాంకం ret = tpm12_read_has_owner(&has_owner);
    ఒకవేళ (ret)
        తిరిగి -1;
    ఒకవేళ (!has_owner) {
        ఒకవేళ (verbose)
            రాయి("TPM does not have an owner.\n");
        తిరిగి 0;
    }

    ఒకవేళ (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        ఒకవేళ (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            తిరిగి ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret)
        తిరిగి ret;

    ఒకవేళ (!enable_activate_after) {
        ఒకవేళ (verbose)
            రాయి("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        తిరిగి 0;
    }

    తిరిగి tpm12_enable_activate(1, verbose);
}

స్థిర పూర్ణాంకం
tpm12_సమితి_owner_install(పూర్ణాంకం allow, పూర్ణాంకం verbose)
{
    పూర్ణాంకం has_owner;
    పూర్ణాంకం ret = tpm12_read_has_owner(&has_owner);
    ఒకవేళ (ret)
        తిరిగి -1;
    ఒకవేళ (has_owner) {
        ఒకవేళ (verbose)
            రాయి("Must first remove owner.\n");
        తిరిగి 0;
    }

    నిర్మాణం tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((అక్షరం *)&pf, పరిమాణం(pf));
    ఒకవేళ (ret)
        తిరిగి -1;

    ఒకవేళ (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        ఒకవేళ (verbose)
            రాయి("TPM must first be enable.\n");
        తిరిగి 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_సమితి_Owner_Install,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret)
        తిరిగి ret;

    ఒకవేళ (verbose)
        రాయి("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
tpm12_process_cfg(tpm_ppi_సంకేతం సందేశం_సంకేతం, పూర్ణాంకం verbose)
{
    పూర్ణాంకం ret = 0;

    ఎంపిక (సందేశం_సంకేతం) {
        స్థితి TPM_PPI_OP_NOOP: /* no-op */
            ఆపు;

        స్థితి TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            ఆపు;

        స్థితి TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            ఆపు;

        స్థితి TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            ఆపు;

        స్థితి TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            ఆపు;

        స్థితి TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            ఆపు;

        స్థితి TPM_PPI_OP_సమితి_OWNERINSTALL_TRUE:
            ret = tpm12_సమితి_owner_install(1, verbose);
            ఆపు;

        స్థితి TPM_PPI_OP_సమితి_OWNERINSTALL_FALSE:
            ret = tpm12_సమితి_owner_install(0, verbose);
            ఆపు;

        అప్రమేయ:
            ఆపు;
    }

    ఒకవేళ (ret)
        రాయి("Op %d: An error occurred: 0x%x\n", సందేశం_సంకేతం, ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_clearcontrol(u8 disable, పూర్ణాంకం verbose)
{
    నిర్మాణం tpm2_req_clearcontrol trc = {
        .hdr.ట్యాగ్     = cpu_గమ్యం_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_గమ్యం_be32(పరిమాణం(trc)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_గమ్యం_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_గమ్యం_be32(పరిమాణం(trc.authblock)),
        .authblock = {
            .handle = cpu_గమ్యం_be32(TPM2_RS_PW),
            .noncesize = cpu_గమ్యం_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_గమ్యం_be16(0),
        },
        .disable = disable,
    };
    నిర్మాణం tpm_rsp_శీర్షిక rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);
    పూర్ణాంకం ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_SHORT);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_clear(శూన్య)
{
    నిర్మాణం tpm2_req_clear trq = {
        .hdr.ట్యాగ్     = cpu_గమ్యం_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_గమ్యం_be32(పరిమాణం(trq)),
        .hdr.ordinal = cpu_గమ్యం_be32(TPM2_CC_Clear),
        .authhandle = cpu_గమ్యం_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_గమ్యం_be32(పరిమాణం(trq.authblock)),
        .authblock = {
            .handle = cpu_గమ్యం_be32(TPM2_RS_PW),
            .noncesize = cpu_గమ్యం_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_గమ్యం_be16(0),
        },
    };
    నిర్మాణం tpm_rsp_శీర్షిక rsp;
    u32 resp_నిడివి = పరిమాణం(rsp);
    పూర్ణాంకం ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_నిడివి,
                             TPM_DURATION_TYPE_MEDIUM);
    ఒకవేళ (ret || resp_నిడివి != పరిమాణం(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm20_process_cfg(tpm_ppi_సంకేతం సందేశం_సంకేతం, పూర్ణాంకం verbose)
{
    పూర్ణాంకం ret = 0;

    ఎంపిక (సందేశం_సంకేతం) {
        స్థితి TPM_PPI_OP_NOOP: /* no-op */
            ఆపు;

        స్థితి TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            ఒకవేళ (!ret)
                 ret = tpm20_clear();
            ఆపు;
    }

    ఒకవేళ (ret)
        రాయి("Op %d: An error occurred: 0x%x\n", సందేశం_సంకేతం, ret);

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
tpm12_get_tpm_state(శూన్య)
{
    పూర్ణాంకం state = 0;
    నిర్మాణం tpm_permanent_flags pf;
    పూర్ణాంకం has_owner;

    ఒకవేళ (tpm12_read_permanent_flags((అక్షరం *)&pf, పరిమాణం(pf)) ||
        tpm12_read_has_owner(&has_owner))
        తిరిగి ~0;

    ఒకవేళ (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    ఒకవేళ (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_చురుకైన;

    ఒకవేళ (has_owner) {
        state |= TPM_STATE_OWNED;
    } లేకపోతే {
        ఒకవేళ (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    తిరిగి state;
}

స్థిర శూన్య
tpm12_show_tpm_menu(పూర్ణాంకం state, పూర్ణాంకం తదుపరి_scancodes[7])
{
    పూర్ణాంకం i = 0;

    రాయి("\nThe current state of the TPM is:\n");

    ఒకవేళ (state & TPM_STATE_ENABLED)
        రాయి("  Enabled");
    లేకపోతే
        రాయి("  Disabled");

    ఒకవేళ (state & TPM_STATE_చురుకైన)
        రాయి(" and active\n");
    లేకపోతే
        రాయి(" and deactivated\n");

    ఒకవేళ (state & TPM_STATE_OWNED)
        రాయి("  Ownership has been taken\n");
    లేకపోతే {
        రాయి("  Ownership has not been taken\n");
        ఒకవేళ (state & TPM_STATE_OWNERINSTALL)
            రాయి("  A user can take ownership of the TPM\n");
        లేకపోతే
            రాయి("  Taking ownership of the TPM has been disabled\n");
    }

    ఒకవేళ ((state & (TPM_STATE_ENABLED | TPM_STATE_చురుకైన)) !=
        (TPM_STATE_ENABLED | TPM_STATE_చురుకైన)) {
        రాయి("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    రాయి("\nAvailable options are:\n");
    ఒకవేళ (state & TPM_STATE_ENABLED) {
        రాయి(" d. Disable the TPM\n");
        తదుపరి_scancodes[i++] = 32;

        ఒకవేళ (state & TPM_STATE_చురుకైన) {
            రాయి(" v. Deactivate the TPM\n");
            తదుపరి_scancodes[i++] = 47;

            ఒకవేళ (state & TPM_STATE_OWNERINSTALL) {
                రాయి(" p. Prevent installation of an owner\n");
                తదుపరి_scancodes[i++] = 25;
            } లేకపోతే {
                రాయి(" s. Allow installation of an owner\n");
                తదుపరి_scancodes[i++] = 31;
            }
        } లేకపోతే {
            రాయి(" a. Activate the TPM\n");
            తదుపరి_scancodes[i++] = 30;
        }

    } లేకపోతే {
        రాయి(" e. Enable the TPM\n");
        తదుపరి_scancodes[i++] = 18;
    }

    ఒకవేళ (state & TPM_STATE_OWNED) {
        రాయి(" c. Clear ownership\n");
        తదుపరి_scancodes[i++] = 46;
    }

    తదుపరి_scancodes[i++] = 0;
}

స్థిర శూన్య
tpm12_menu(శూన్య)
{
    పూర్ణాంకం scancode, తదుపరి_scancodes[7];
    tpm_ppi_సంకేతం సందేశం_సంకేతం;
    పూర్ణాంకం state = 0, i;
    పూర్ణాంకం waitkey;

    రాయి("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    కొరకు (;;) {
        ఒకవేళ ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, తదుపరి_scancodes);
        } లేకపోతే {
            రాయి("TPM is not working correctly.\n");
            తిరిగి;
        }

        రాయి("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        సందేశం_సంకేతం = TPM_PPI_OP_NOOP;

        waitkey = 1;

        అయితే (waitkey) {
            అయితే ((scancode = get_keystroke(1000)) == ~0)
                ;

            ఎంపిక (scancode) {
            స్థితి 1:
                // ESC
                reset();
                ఆపు;
            స్థితి 18: /* e. enable */
                సందేశం_సంకేతం = TPM_PPI_OP_ENABLE;
                ఆపు;
            స్థితి 32: /* d. disable */
                సందేశం_సంకేతం = TPM_PPI_OP_DISABLE;
                ఆపు;
            స్థితి 30: /* a. activate */
                సందేశం_సంకేతం = TPM_PPI_OP_ACTIVATE;
                ఆపు;
            స్థితి 47: /* v. deactivate */
                సందేశం_సంకేతం = TPM_PPI_OP_DEACTIVATE;
                ఆపు;
            స్థితి 46: /* c. clear owner */
                సందేశం_సంకేతం = TPM_PPI_OP_CLEAR;
                ఆపు;
            స్థితి 25: /* p. prevent ownerinstall */
                సందేశం_సంకేతం = TPM_PPI_OP_సమితి_OWNERINSTALL_FALSE;
                ఆపు;
            స్థితి 31: /* s. allow ownerinstall */
                సందేశం_సంకేతం = TPM_PPI_OP_సమితి_OWNERINSTALL_TRUE;
                ఆపు;
            అప్రమేయ:
                కొనసాగు;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            కొరకు (i = 0; i < పరిమాణం(తదుపరి_scancodes); i++) {
                ఒకవేళ (తదుపరి_scancodes[i] == 0)
                    ఆపు;

                ఒకవేళ (తదుపరి_scancodes[i] == scancode) {
                    tpm12_process_cfg(సందేశం_సంకేతం, 1);
                    waitkey = 0;
                    ఆపు;
                }
            }
        }
    }
}

స్థిర పూర్ణాంకం
tpm20_menu_change_చురుకైన_pcrbanks(శూన్య)
{
    u8 చురుకైన_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &చురుకైన_banks);

    u8 activate_banks = చురుకైన_banks;

    అయితే (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        రాయి("\nToggle active PCR banks by pressing number key\n\n");

        అయితే (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            స్థిరరాశి అక్షరం *hashname = tpm20_hashalg_flag_గమ్యం_name(flag);

            i++;
            ఒకవేళ (hashname) {
                రాయి("  %d: %s", i, hashname);
                ఒకవేళ (activate_banks & hashalg_flag)
                    రాయి(" (enabled)");
                రాయి("\n");
            }

            hashalg_flag <<= 1;
        }
        రాయి("\n"
               "ESC: return to previous menu without changes\n");
        ఒకవేళ (activate_banks)
            రాయి("A  : activate selection\n");

        u8 flagnum;
        పూర్ణాంకం show = 0;
        అయితే (!show) {
            పూర్ణాంకం scancode = get_keystroke(1000);

            ఎంపిక (scancode) {
            స్థితి ~0:
                కొనసాగు;
            స్థితి 1: /* ESC */
                రాయి("\n");
                తిరిగి -1;
            స్థితి 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                ఒకవేళ (flagnum > i)
                    కొనసాగు;
                ఒకవేళ (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                ఆపు;
            స్థితి 30: /* a */
                ఒకవేళ (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

స్థిర శూన్య
tpm20_menu(శూన్య)
{
    పూర్ణాంకం పరిశీలన_సంకేతం;
    tpm_ppi_సంకేతం సందేశం_సంకేతం;

    కొరకు (;;) {
        రాయి("1. Clear TPM\n");
        రాయి("2. Change active PCR banks\n");

        రాయి("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        సందేశం_సంకేతం = TPM_PPI_OP_NOOP;

        అయితే ((పరిశీలన_సంకేతం = get_keystroke(1000)) == ~0)
            ;

        ఎంపిక (పరిశీలన_సంకేతం) {
        స్థితి 1:
            // ESC
            reset();
            ఆపు;
        స్థితి 2:
            సందేశం_సంకేతం = TPM_PPI_OP_CLEAR;
            ఆపు;
        స్థితి 3:
            tpm20_menu_change_చురుకైన_pcrbanks();
            కొనసాగు;
        అప్రమేయ:
            కొనసాగు;
        }

        tpm20_process_cfg(సందేశం_సంకేతం, 0);
    }
}

శూన్య
tpm_menu(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి;

    అయితే (get_keystroke(0) >= 0)
        ;
    wait_దారాలు();

    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        tpm12_menu();
        ఆపు;
    స్థితి TPM_సంస్కరణ_2:
        tpm20_menu();
        ఆపు;
    }
}

పూర్ణాంకం
tpm_can_show_menu(శూన్య)
{
    ఎంపిక (TPM_సంస్కరణ) {
    స్థితి TPM_సంస్కరణ_1_2:
        తిరిగి tpm_is_working() && TPM_has_physical_presence;
    స్థితి TPM_సంస్కరణ_2:
        తిరిగి tpm_is_working();
    }
    తిరిగి 0;
}
