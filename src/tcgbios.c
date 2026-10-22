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

#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "byteorder.h" // cpu_to_*
#અન્તર્ભાવ "config.h" // CONFIG_TCGBIOS
#અન્તર્ભાવ "farptr.h" // MAKE_FLATPTR
#અન્તર્ભાવ "fw/paravirt.h" // runningOnXen
#અન્તર્ભાવ "hw/tpm_drivers.h" // tpm_drivers[]
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "sha.h" // sha1, sha256, ...
#અન્તર્ભાવ "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#અન્તર્ભાવ "std/smbios.h" // struct smbios_21_entry_point
#અન્તર્ભાવ "std/tcg.h" // TCG_PC_LOGOVERFLOW
#અન્તર્ભાવ "string.h" // checksum
#અન્તર્ભાવ "tcgbios.h"// tpm_*, prototypes
#અન્તર્ભાવ "util.h" // printf, get_keystroke
#અન્તર્ભાવ "stacks.h" // wait_threads, reset
#અન્તર્ભાવ "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

સંરચના {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_ગણતરી;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

સ્થિર પૂર્ણાંક tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    જો (!log_area_start_address || !log_area_minimum_length)
        પાછા -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = ખાલી;
    tpm_state.entry_ગણતરી = 0;
    પાછા 0;
}

સ્થિર પૂર્ણાંક
tpm_tcpa_probe(શૂન્ય)
{
    સંરચના tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    જો (!tcpa)
        પાછા -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    પાછા tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

સ્થિર પૂર્ણાંક
tpm_tpm2_probe(શૂન્ય)
{
    સંરચના tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    જો (!tpm2)
        પાછા -1;

    જો (tpm2->length < 76)
        પાછા -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    પાછા tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
સ્થિર પૂર્ણાંક
tpm_log_event(સંરચના tpm_log_header *entry, પૂર્ણાંક digest_len
              , અચલ શૂન્ય *event, પૂર્ણાંક event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    જો (tpm_state.log_area_next_entry == ખાલી)
        પાછા -1;

    u32 size = (કદ(*entry) + digest_len
                + કદ(સંરચના tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    જો (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        પાછા -1;
    }

    શૂન્ય *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, કદ(*entry) + digest_len);
    સંરચના tpm_log_trailer *t = dest + કદ(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_ગણતરી++;

    પાછા 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

સ્થિર TPMVersion TPM_version;
સ્થિર u32 tpm20_pcr_selection_size;
સ્થિર સંરચના tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
સંરચના tpm_log_entry {
    સંરચના tpm_log_header hdr;
    u8 pad[કદ(સંરચના tpm2_digest_કિંમતો)
           + 8 * કદ(સંરચના tpm2_digest_કિંમત)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

સ્થિર અચલ સંરચના hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    અચલ અક્ષર *name;
    શૂન્ય (*hashfunc)(અચલ u8 *data, u32 length, u8 *hash);
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

સ્થિર પૂર્ણાંક
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    માટે (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        જો (hash_parameters[i].hashalg == hashAlg)
            પાછા hash_parameters[i].hash_buffersize;
    }
    પાછા -1;
}

સ્થિર u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    માટે (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        જો (hash_parameters[i].hashalg == hashAlg)
            પાછા hash_parameters[i].hashalg_flag;
    }
    પાછા 0;
}

સ્થિર u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    માટે (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        જો (hash_parameters[i].hashalg_flag == hashalg_flag)
            પાછા hash_parameters[i].hashalg;
    }
    પાછા 0;
}

સ્થિર અચલ અક્ષર *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    માટે (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        જો (hash_parameters[i].hashalg_flag == hashalg_flag)
            પાછા hash_parameters[i].name;
    }
    પાછા ખાલી;
}

સ્થિર શૂન્ય tpm2_hash_data(u16 hashAlg, અચલ u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    માટે (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        જો (hash_parameters[i].hashalg == hashAlg) {
            જો (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } નહીંતર {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
સ્થિર પૂર્ણાંક
tpm20_write_EfiSpecIdEventStruct(શૂન્ય)
{
    જો (!tpm20_pcr_selection)
        પાછા -1;

    સંરચના {
        સંરચના TCG_EfiSpecIdEventStruct hdr;
        u8 pad[કદ(સંરચના tpm_log_entry) + કદ(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    સંરચના tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    શૂન્ય *nsel, *end = (શૂન્ય*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 ગણતરી, અંક_Algs = 0;
    માટે (ગણતરી = 0; ગણતરી < be32_to_cpu(tpm20_pcr_selection->ગણતરી); ગણતરી++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (શૂન્ય*)sel + કદ(*sel) + sizeOfSelect;
        જો (nsel > end)
            તોડો;

        જો (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ચાલુ;
        }

        પૂર્ણાંક hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        જો (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            પાછા -1;
        }

        પૂર્ણાંક event_size = offsetof(સંરચના TCG_EfiSpecIdEventStruct
                                  , digestSizes[ગણતરી+1]);
        જો (event_size > કદ(event) - કદ(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            પાછા -1;
        }

        event.hdr.digestSizes[અંક_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[અંક_Algs].digestSize = hsize;
        અંક_Algs++;

        sel = nsel;
    }

    જો (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        પાછા -1;
    }

    event.hdr.સંખ્યા_Of_Algorithms = અંક_Algs;
    પૂર્ણાંક event_size = offsetof(સંરચના TCG_EfiSpecIdEventStruct
                              , digestSizes[અંક_Algs]);
    u8 *vendorInfoSize = (શૂન્ય*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += કદ(*vendorInfoSize);

    સંરચના tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    પાછા tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
સ્થિર પૂર્ણાંક
tpm20_build_digest(સંરચના tpm_log_entry *le,
                   અચલ u8 *hashdata, u32 hashdata_len, પૂર્ણાંક bigEndian)
{
    જો (!tpm20_pcr_selection)
        પાછા -1;

    સંરચના tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    શૂન્ય *nsel, *end = (શૂન્ય*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    શૂન્ય *dest = le->hdr.digest + કદ(સંરચના tpm2_digest_કિંમતો);

    u32 ગણતરી, અંક_Algs = 0;
    માટે (ગણતરી = 0; ગણતરી < be32_to_cpu(tpm20_pcr_selection->ગણતરી); ગણતરી++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (શૂન્ય*)sel + કદ(*sel) + sizeOfSelect;
        જો (nsel > end)
            તોડો;

        /* PCR 0-7 unused? -- skip */
        જો (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            ચાલુ;
        }

        પૂર્ણાંક hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        જો (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            પાછા -1;
        }

        /* buffer size sanity check before writing */
        સંરચના tpm2_digest_કિંમત *v = dest;
        જો (dest + કદ(*v) + hsize > (શૂન્ય*)le + કદ(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            પાછા -1;
        }

        જો (bigEndian)
            v->hashAlg = sel->hashAlg;
        નહીંતર
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += કદ(*v) + hsize;
        sel = nsel;

        અંક_Algs++;
    }

    જો (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        પાછા -1;
    }

    સંરચના tpm2_digest_કિંમતો *v = (શૂન્ય*)le->hdr.digest;
    જો (bigEndian)
        v->ગણતરી = cpu_to_be32(અંક_Algs);
    નહીંતર
        v->ગણતરી = અંક_Algs;

    પાછા dest - (શૂન્ય*)le->hdr.digest;
}

સ્થિર પૂર્ણાંક
tpm12_build_digest(સંરચના tpm_log_entry *le,
                   અચલ u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    પાછા SHA1_BUFSIZE;
}

સ્થિર પૂર્ણાંક
tpm12_build_digest_direct(સંરચના tpm_log_entry *le, અચલ u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    પાછા SHA1_BUFSIZE;
}

સ્થિર પૂર્ણાંક
tpm_build_digest(સંરચના tpm_log_entry *le, અચલ u8 *hashdata, u32 hashdata_len
                 , પૂર્ણાંક bigEndian)
{
    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        પાછા tpm12_build_digest(le, hashdata, hashdata_len);
    સ્થિતિ TPM_VERSION_2:
        પાછા tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    પાછા -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
સ્થિર પૂર્ણાંક
tpm_simple_cmd(u8 locty, u32 ordinal
               , પૂર્ણાંક param_size, u16 param, ગણના tpmDurationType to_t)
{
    સંરચના {
        સંરચના tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(કદ(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        તોડો;
    સ્થિતિ TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        તોડો;
    }

    u8 obuffer[64];
    સંરચના tpm_rsp_header *trsh = (શૂન્ય*)obuffer;
    u32 obuffer_len = કદ(obuffer);
    memset(obuffer, 0x0, કદ(obuffer));

    પૂર્ણાંક ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_getcapability(u32 capability, u32 property, u32 ગણતરી,
                    સંરચના tpm_rsp_header *rsp, u32 rsize)
{
    સંરચના tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(કદ(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(ગણતરી),
    };

    u32 resp_size = rsize;
    પૂર્ણાંક ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_get_pcrbanks(શૂન્ય)
{
    u8 buffer[128];
    સંરચના tpm2_res_getcapability *trg =
      (સંરચના tpm2_res_getcapability *)&buffer;

    પૂર્ણાંક ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  કદ(buffer));
    જો (ret)
        પાછા ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    જો (resplen <= offsetof(સંરચના tpm2_res_getcapability, data))
        પાછા -1;

    u32 size = resplen - offsetof(સંરચના tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    જો (size < offsetof(સંરચના tpml_pcr_selection, selections) +
               offsetof(સંરચના tpms_pcr_selection, pcrSelect))
        પાછા -1;

    tpm20_pcr_selection = malloc_ઉપલું(size);
    જો (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } નહીંતર {
        warn_noalloc();
        ret = -1;
    }

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    જો (!tpm20_pcr_selection)
        પાછા -1;

    સંરચના tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    શૂન્ય *end = (શૂન્ય*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    જ્યારે (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        શૂન્ય *nsel = (શૂન્ય*)sel + કદ(*sel) + sizeOfSelect;
        જો (nsel > end)
            પાછા 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        માટે (i = 0; i < sizeOfSelect; i++) {
            જો (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                તોડો;
            }
        }

        sel = nsel;
    }
}

સ્થિર પૂર્ણાંક
tpm20_set_pcrbanks(u32 active_banks)
{
    સંરચના tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(કદ(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    સંરચના tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    પૂર્ણાંક i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    જ્યારે (hashalg_flag) {
        જો ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            જો (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                જો (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.ગણતરી = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * કદ(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(સંરચના tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * કદ(tps[0]));

    સંરચના tpm_rsp_header rsp;
    u32 resp_length = કદ(rsp);

    પૂર્ણાંક ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    પાછા ret;
}

સ્થિર પૂર્ણાંક tpm20_activate_pcrbanks(u32 active_banks)
{
    પૂર્ણાંક ret = tpm20_set_pcrbanks(active_banks);
    જો (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    જો (!ret)
        reset();
    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm12_get_capability(u32 cap, u32 subcap, સંરચના tpm_rsp_header *rsp, u32 rsize)
{
    સંરચના tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(કદ(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(કદ(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    પૂર્ણાંક ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm12_read_permanent_flags(અક્ષર *buf, પૂર્ણાંક buf_len)
{
    memset(buf, 0, buf_len);

    સંરચના tpm_res_getcap_perm_flags pf;
    પૂર્ણાંક ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, કદ(pf));
    જો (ret)
        પાછા -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    પાછા 0;
}

સ્થિર પૂર્ણાંક
tpm12_determine_timeouts(શૂન્ય)
{
    સંરચના tpm_res_getcap_timeouts timeouts;
    પૂર્ણાંક ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, કદ(timeouts));
    જો (ret)
        પાછા ret;

    સંરચના tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, કદ(durations));
    જો (ret)
        પાછા ret;

    પૂર્ણાંક i;
    માટે (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    માટે (i = 0; i < 4; i++)
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

    પાછા 0;
}

સ્થિર શૂન્ય
tpm20_set_timeouts(શૂન્ય)
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

સ્થિર પૂર્ણાંક
tpm12_extend(સંરચના tpm_log_entry *le, પૂર્ણાંક digest_len)
{
    સંરચના tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(કદ(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, કદ(tre.digest));

    સંરચના tpm_rsp_extend rsp;
    u32 resp_length = કદ(rsp);
    પૂર્ણાંક ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    જો (ret || resp_length != કદ(rsp) || rsp.hdr.errcode)
        પાછા -1;

    પાછા 0;
}

સ્થિર પૂર્ણાંક tpm20_extend(સંરચના tpm_log_entry *le, પૂર્ણાંક digest_len)
{
    સંરચના tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(કદ(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[કદ(tmp_tre) + કદ(le->pad)];
    સંરચના tpm2_req_extend *tre = (સંરચના tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, કદ(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(કદ(tmp_tre) + digest_len);

    સંરચના tpm_rsp_header rsp;
    u32 resp_length = કદ(rsp);
    પૂર્ણાંક ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    જો (ret || resp_length != કદ(rsp) || rsp.errcode)
        પાછા -1;

    પાછા 0;
}

સ્થિર પૂર્ણાંક
tpm_extend(સંરચના tpm_log_entry *le, પૂર્ણાંક digest_len)
{
    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        પાછા tpm12_extend(le, digest_len);
    સ્થિતિ TPM_VERSION_2:
        પાછા tpm20_extend(le, digest_len);
    }
    પાછા -1;
}

સ્થિર પૂર્ણાંક
tpm20_stirrandom(શૂન્ય)
{
    સંરચના tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(કદ(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(કદ(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    સંરચના tpm_rsp_header rsp;
    u32 resp_length = કદ(rsp);
    પૂર્ણાંક ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    જો (ret || resp_length != કદ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    સંરચના tpm2_res_getrandom rsp;

    જો (buf_len > કદ(rsp.rnd.buffer))
        પાછા -1;

    સંરચના tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(કદ(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = કદ(rsp);

    પૂર્ણાંક ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    જો (ret || resp_length != કદ(rsp) || rsp.hdr.errcode)
        ret = -1;
    નહીંતર
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    સંરચના tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(કદ(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(કદ(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    સંરચના tpm_rsp_header rsp;
    u32 resp_length = કદ(rsp);
    પૂર્ણાંક ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    જો (ret || resp_length != કદ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_hierarchychangeauth(u8 auth[20])
{
    સંરચના tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(કદ(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(કદ(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(કદ(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, કદ(trhca.newAuth.buffer));

    સંરચના tpm_rsp_header rsp;
    u32 resp_length = કદ(rsp);
    પૂર્ણાંક ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    જો (ret || resp_length != કદ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    પાછા ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

સ્થિર પૂર્ણાંક TPM_has_physical_presence;
u8 TPM_working VARLOW;

સ્થિર પૂર્ણાંક
tpm_is_working(શૂન્ય)
{
    પાછા CONFIG_TCGBIOS && TPM_working;
}

સ્થિર શૂન્ય
tpm_set_failure(શૂન્ય)
{
    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        તોડો;
    સ્થિતિ TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        તોડો;
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
સ્થિર શૂન્ય
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           અચલ અક્ષર *event, u32 event_length,
                           અચલ u8 *hashdata, u32 hashdata_length)
{
    જો (!tpm_is_working())
        પાછા;

    સંરચના tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    પૂર્ણાંક digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    જો (digest_len < 0)
        પાછા;
    પૂર્ણાંક ret = tpm_extend(&le, digest_len);
    જો (ret) {
        tpm_set_failure();
        પાછા;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
સ્થિર શૂન્ય
tpm_add_action(u32 pcrIndex, અચલ અક્ષર *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
સ્થિર શૂન્ય
tpm_add_event_separators(શૂન્ય)
{
    સ્થિર અચલ u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    માટે (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (અચલ અક્ષર *)evt_separator,
                                   કદ(evt_separator),
                                   evt_separator,
                                   કદ(evt_separator));
}

સ્થિર શૂન્ય
tpm_smbios_measure(શૂન્ય)
{
    સંરચના pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    શૂન્ય *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    જો (!smbios_tables)
        પાછા;

    sha1((અચલ u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (અચલ અક્ષર *)&pcctes, કદ(pcctes),
                               (u8 *)&pcctes, કદ(pcctes));
}

સ્થિર પૂર્ણાંક
tpm12_assert_physical_presence(શૂન્ય)
{
    પૂર્ણાંક ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    જો (!ret)
        પાછા 0;

    સંરચના tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((અક્ષર *)&pf, કદ(pf));
    જો (ret)
        પાછા -1;

    /* check if hardware physical presence is supported */
    જો (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        પાછા 0;
    }

    જો (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        પાછા tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    પાછા -1;
}

સ્થિર પૂર્ણાંક
tpm12_startup(શૂન્ય)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    પૂર્ણાંક ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    જો (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    જો (ret)
        જાઓ err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    જો (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    જો (ret)
        જાઓ err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    જો (ret)
        જાઓ err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    જો (ret && ret != TPM_BAD_LOCALITY)
        જાઓ err_exit;

    પાછા 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    પાછા -1;
}

સ્થિર પૂર્ણાંક
tpm20_startup(શૂન્ય)
{
    tpm20_set_timeouts();

    પૂર્ણાંક ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    જો (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    જો (ret)
        જાઓ err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    જો (ret)
        જાઓ err_exit;

    ret = tpm20_get_pcrbanks();
    જો (ret)
        જાઓ err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    જો (ret)
        જાઓ err_exit;

    પાછા 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    પાછા -1;
}

સ્થિર પૂર્ણાંક
tpm_startup(શૂન્ય)
{
    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        પાછા tpm12_startup();
    સ્થિતિ TPM_VERSION_2:
        પાછા tpm20_startup();
    }
    પાછા -1;
}

શૂન્ય
tpm_setup(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા;

    પૂર્ણાંક ret = tpm_tpm2_probe();
    જો (ret) {
        ret = tpm_tcpa_probe();
        જો (ret)
            પાછા;
    }

    TPM_version = tpmhw_probe();
    જો (TPM_version == TPM_VERSION_NONE)
        પાછા;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    જો (runningOnXen())
        પાછા;

    ret = tpm_startup();
    જો (ret)
        પાછા;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

સ્થિર શૂન્ય
tpm20_prepboot(શૂન્ય)
{
    પૂર્ણાંક ret = tpm20_stirrandom();
    જો (ret)
         જાઓ err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], કદ(auth));
    જો (ret)
        જાઓ err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    જો (ret)
        જાઓ err_exit;

    પાછા;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

શૂન્ય
tpm_prepboot(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા;

    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        જો (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        તોડો;
    સ્થિતિ TPM_VERSION_2:
        tpm20_prepboot();
        તોડો;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
શૂન્ય
tpm_option_rom(અચલ શૂન્ય *addr, u32 len)
{
    જો (!tpm_is_working())
        પાછા;

    સંરચના pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = કદ(u16) + કદ(u16) + SHA1_BUFSIZE,
    };
    sha1((અચલ u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (અચલ અક્ષર *)&pcctes, કદ(pcctes),
                               (u8 *)&pcctes, કદ(pcctes));
}

શૂન્ય
tpm_add_bcv(u32 bootdrv, અચલ u8 *addr, u32 length)
{
    જો (!tpm_is_working())
        પાછા;

    જો (length < 0x200)
        પાછા;

    અચલ અક્ષર *string = "Booting BCV device 00h (Floppy)";
    જો (bootdrv == 0x80)
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

શૂન્ય
tpm_add_cdrom(u32 bootdrv, અચલ u8 *addr, u32 length)
{
    જો (!tpm_is_working())
        પાછા;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    અચલ અક્ષર *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

શૂન્ય
tpm_add_cdrom_catalog(અચલ u8 *addr, u32 length)
{
    જો (!tpm_is_working())
        પાછા;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    અચલ અક્ષર *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

શૂન્ય
tpm_s3_resume(શૂન્ય)
{
    જો (!tpm_is_working())
        પાછા;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    પૂર્ણાંક ret = -1;

    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        તોડો;
    સ્થિતિ TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        જો (ret)
            જાઓ err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        તોડો;
    }

    જો (ret)
        જાઓ err_exit;

    પાછા;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

સ્થિર અંતઃરેખ શૂન્ય *આગત_buf32(સંરચના bregs *regs)
{
    પાછા MAKE_FLATPTR(regs->es, regs->di);
}

સ્થિર અંતઃરેખ શૂન્ય *output_buf32(સંરચના bregs *regs)
{
    પાછા MAKE_FLATPTR(regs->ds, regs->si);
}

સ્થિર u32
hash_log_extend(સંરચના pcpes *pcpes, અચલ શૂન્ય *hashdata, u32 hashdata_length
                , શૂન્ય *event, પૂર્ણાંક extend)
{
    જો (pcpes->pcrindex >= 24)
        પાછા TCG_INVALID_આગત_PARA;
    જો (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    સંરચના tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    પૂર્ણાંક digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    જો (digest_len < 0)
        પાછા TCG_GENERAL_ERROR;
    જો (extend) {
        પૂર્ણાંક ret = tpm_extend(&le, digest_len);
        જો (ret)
            પાછા TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    પૂર્ણાંક ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    જો (ret)
        પાછા TCG_PC_LOGOVERFLOW;
    પાછા 0;
}

સ્થિર u32
hash_log_extend_event_int(અચલ સંરચના hleei_short *hleei_s,
                          સંરચના hleeo *hleeo)
{
    u32 rc = 0;
    સંરચના hleei_long *hleei_l = (સંરચના hleei_long *)hleei_s;
    અચલ શૂન્ય *logdataptr;
    u32 logdatalen;
    સંરચના pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    પસંદગી (hleei_s->ipblength) {
    સ્થિતિ કદ(સંરચના hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    તોડો;

    સ્થિતિ કદ(સંરચના hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    તોડો;

    મૂળભૂત:
        /* bad input block */
        rc = TCG_INVALID_આગત_PARA;
        જાઓ err_exit;
    }

    pcpes = (સંરચના pcpes *)logdataptr;

    જો (pcpes->pcrindex != pcrindex
        || logdatalen != કદ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_આગત_PARA;
        જાઓ err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    જો (rc)
        જાઓ err_exit;

    hleeo->opblength = કદ(સંરચના hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_ગણતરી;
    memcpy(hleeo->digest, pcpes->digest, કદ(hleeo->digest));

err_exit:
    જો (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    પાછા rc;
}

સ્થિર u32
pass_through_to_tpm_int(સંરચના pttti *pttti, સંરચના pttto *pttto)
{
    u32 rc = 0;
    સંરચના tpm_req_header *trh = (શૂન્ય*)pttti->tpmopin;

    જો (pttti->ipblength < કદ(સંરચના pttti) + કદ(*trh)
        || pttti->ipblength != કદ(સંરચના pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < કદ(સંરચના pttto)) {
        rc = TCG_INVALID_આગત_PARA;
        જાઓ err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        જો (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_આગત_PARA;
            જાઓ err_exit;
        }
        તોડો;
    સ્થિતિ TPM_VERSION_2:
        જો (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_આગત_PARA;
            જાઓ err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(સંરચના pttto, tpmopout);
    પૂર્ણાંક ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    જો (ret) {
        rc = TCG_FATAL_COM_ERROR;
        જાઓ err_exit;
    }

    pttto->opblength = offsetof(સંરચના pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    જો (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    પાછા rc;
}

સ્થિર u32
shutdown_preboot_interface(શૂન્ય)
{
    TPM_interface_shutdown = 1;
    પાછા 0;
}

સ્થિર u32
hash_log_event_int(અચલ સંરચના hlei *hlei, સંરચના hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    સંરચના pcpes *pcpes;

    size = hlei->ipblength;
    જો (size != કદ(*hlei)) {
        rc = TCG_INVALID_આગત_PARA;
        જાઓ err_exit;
    }

    pcpes = (સંરચના pcpes *)hlei->logdataptr;

    જો (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != કદ(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_આગત_PARA;
        જાઓ err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    જો (rc)
        જાઓ err_exit;

    /* updating the log was fine */
    hleo->opblength = કદ(સંરચના hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_ગણતરી;

err_exit:
    જો (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    પાછા rc;
}

સ્થિર u32
hash_all_int(અચલ સંરચના hai *hai, u8 *hash)
{
    જો (hai->ipblength != કદ(સંરચના hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        પાછા TCG_INVALID_આગત_PARA;

    sha1((અચલ u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    પાછા 0;
}

સ્થિર u32
tss_int(સંરચના ti *ti, સંરચના to *to)
{
    to->opblength = કદ(સંરચના to);
    to->reserved  = 0;

    પાછા TCG_PC_UNSUPPORTED;
}

સ્થિર u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    સંરચના pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = કદ(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    જો (rc)
        પાછા rc;

    *edx_ptr = tpm_state.entry_ગણતરી;
    પાછા 0;
}

શૂન્ય VISIBLE32FLAT
tpm_interrupt_handler32(સંરચના bregs *regs)
{
    જો (!CONFIG_TCGBIOS)
        પાછા;

    set_cf(regs, 0);

    જો (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        પાછા;
    }

    પસંદગી ((ગણના irq_ids)regs->al) {
    સ્થિતિ TCG_StatusCheck:
        જો (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } નહીંતર {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        તોડો;

    સ્થિતિ TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (સંરચના hleei_short *)આગત_buf32(regs),
                  (સંરચના hleeo *)output_buf32(regs));
        તોડો;

    સ્થિતિ TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((સંરચના pttti *)આગત_buf32(regs),
                                    (સંરચના pttto *)output_buf32(regs));
        તોડો;

    સ્થિતિ TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        તોડો;

    સ્થિતિ TCG_HashLogEvent:
        regs->eax = hash_log_event_int((સંરચના hlei*)આગત_buf32(regs),
                                       (સંરચના hleo*)output_buf32(regs));
        તોડો;

    સ્થિતિ TCG_HashAll:
        regs->eax =
            hash_all_int((સંરચના hai*)આગત_buf32(regs),
                          (u8 *)output_buf32(regs));
        તોડો;

    સ્થિતિ TCG_TSS:
        regs->eax = tss_int((સંરચના ti*)આગત_buf32(regs),
                            (સંરચના to*)output_buf32(regs));
        તોડો;

    સ્થિતિ TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)આગત_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        તોડો;

    મૂળભૂત:
        set_cf(regs, 1);
    }

    પાછા;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

પ્રકારવ્યાખ્યા u8 tpm_ppi_code;

સ્થિર પૂર્ણાંક
tpm12_read_has_owner(પૂર્ણાંક *has_owner)
{
    સંરચના tpm_res_getcap_ownerauth oauth;
    પૂર્ણાંક ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, કદ(oauth));
    જો (ret)
        પાછા -1;

    *has_owner = oauth.flag;

    પાછા 0;
}

સ્થિર પૂર્ણાંક
tpm12_enable_tpm(પૂર્ણાંક enable, પૂર્ણાંક verbose)
{
    સંરચના tpm_permanent_flags pf;
    પૂર્ણાંક ret = tpm12_read_permanent_flags((અક્ષર *)&pf, કદ(pf));
    જો (ret)
        પાછા -1;

    જો (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        પાછા 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    જો (ret) {
        જો (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        નહીંતર
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm12_activate_tpm(પૂર્ણાંક activate, પૂર્ણાંક allow_reset, પૂર્ણાંક verbose)
{
    સંરચના tpm_permanent_flags pf;
    પૂર્ણાંક ret = tpm12_read_permanent_flags((અક્ષર *)&pf, કદ(pf));
    જો (ret)
        પાછા -1;

    જો (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        પાછા 0;

    જો (pf.flags[PERM_FLAG_IDX_DISABLE])
        પાછા 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    જો (ret)
        પાછા ret;

    જો (activate && allow_reset) {
        જો (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    પાછા 0;
}

સ્થિર પૂર્ણાંક
tpm12_enable_activate(પૂર્ણાંક allow_reset, પૂર્ણાંક verbose)
{
    પૂર્ણાંક ret = tpm12_enable_tpm(1, verbose);
    જો (ret)
        પાછા ret;

    પાછા tpm12_activate_tpm(1, allow_reset, verbose);
}

સ્થિર પૂર્ણાંક
tpm12_force_clear(પૂર્ણાંક enable_activate_before, પૂર્ણાંક enable_activate_after,
                  પૂર્ણાંક verbose)
{
    પૂર્ણાંક has_owner;
    પૂર્ણાંક ret = tpm12_read_has_owner(&has_owner);
    જો (ret)
        પાછા -1;
    જો (!has_owner) {
        જો (verbose)
            printf("TPM does not have an owner.\n");
        પાછા 0;
    }

    જો (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        જો (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            પાછા ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    જો (ret)
        પાછા ret;

    જો (!enable_activate_after) {
        જો (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        પાછા 0;
    }

    પાછા tpm12_enable_activate(1, verbose);
}

સ્થિર પૂર્ણાંક
tpm12_set_owner_install(પૂર્ણાંક allow, પૂર્ણાંક verbose)
{
    પૂર્ણાંક has_owner;
    પૂર્ણાંક ret = tpm12_read_has_owner(&has_owner);
    જો (ret)
        પાછા -1;
    જો (has_owner) {
        જો (verbose)
            printf("Must first remove owner.\n");
        પાછા 0;
    }

    સંરચના tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((અક્ષર *)&pf, કદ(pf));
    જો (ret)
        પાછા -1;

    જો (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        જો (verbose)
            printf("TPM must first be enable.\n");
        પાછા 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    જો (ret)
        પાછા ret;

    જો (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    પાછા 0;
}

સ્થિર પૂર્ણાંક
tpm12_process_cfg(tpm_ppi_code સંદેશ_Code, પૂર્ણાંક verbose)
{
    પૂર્ણાંક ret = 0;

    પસંદગી (સંદેશ_Code) {
        સ્થિતિ TPM_PPI_OP_NOOP: /* no-op */
            તોડો;

        સ્થિતિ TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            તોડો;

        સ્થિતિ TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            તોડો;

        સ્થિતિ TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            તોડો;

        સ્થિતિ TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            તોડો;

        સ્થિતિ TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            તોડો;

        સ્થિતિ TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            તોડો;

        સ્થિતિ TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            તોડો;

        મૂળભૂત:
            તોડો;
    }

    જો (ret)
        printf("Op %d: An error occurred: 0x%x\n", સંદેશ_Code, ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_clearcontrol(u8 disable, પૂર્ણાંક verbose)
{
    સંરચના tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(કદ(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(કદ(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    સંરચના tpm_rsp_header rsp;
    u32 resp_length = કદ(rsp);
    પૂર્ણાંક ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    જો (ret || resp_length != કદ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_clear(શૂન્ય)
{
    સંરચના tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(કદ(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(કદ(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    સંરચના tpm_rsp_header rsp;
    u32 resp_length = કદ(rsp);
    પૂર્ણાંક ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    જો (ret || resp_length != કદ(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm20_process_cfg(tpm_ppi_code સંદેશ_Code, પૂર્ણાંક verbose)
{
    પૂર્ણાંક ret = 0;

    પસંદગી (સંદેશ_Code) {
        સ્થિતિ TPM_PPI_OP_NOOP: /* no-op */
            તોડો;

        સ્થિતિ TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            જો (!ret)
                 ret = tpm20_clear();
            તોડો;
    }

    જો (ret)
        printf("Op %d: An error occurred: 0x%x\n", સંદેશ_Code, ret);

    પાછા ret;
}

સ્થિર પૂર્ણાંક
tpm12_get_tpm_state(શૂન્ય)
{
    પૂર્ણાંક state = 0;
    સંરચના tpm_permanent_flags pf;
    પૂર્ણાંક has_owner;

    જો (tpm12_read_permanent_flags((અક્ષર *)&pf, કદ(pf)) ||
        tpm12_read_has_owner(&has_owner))
        પાછા ~0;

    જો (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    જો (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    જો (has_owner) {
        state |= TPM_STATE_OWNED;
    } નહીંતર {
        જો (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    પાછા state;
}

સ્થિર શૂન્ય
tpm12_show_tpm_menu(પૂર્ણાંક state, પૂર્ણાંક next_scancodes[7])
{
    પૂર્ણાંક i = 0;

    printf("\nThe current state of the TPM is:\n");

    જો (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    નહીંતર
        printf("  Disabled");

    જો (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    નહીંતર
        printf(" and deactivated\n");

    જો (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    નહીંતર {
        printf("  Ownership has not been taken\n");
        જો (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        નહીંતર
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    જો ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    જો (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        જો (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            જો (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } નહીંતર {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } નહીંતર {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } નહીંતર {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    જો (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

સ્થિર શૂન્ય
tpm12_menu(શૂન્ય)
{
    પૂર્ણાંક scancode, next_scancodes[7];
    tpm_ppi_code સંદેશ_Code;
    પૂર્ણાંક state = 0, i;
    પૂર્ણાંક waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    માટે (;;) {
        જો ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } નહીંતર {
            printf("TPM is not working correctly.\n");
            પાછા;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        સંદેશ_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        જ્યારે (waitkey) {
            જ્યારે ((scancode = get_keystroke(1000)) == ~0)
                ;

            પસંદગી (scancode) {
            સ્થિતિ 1:
                // ESC
                reset();
                તોડો;
            સ્થિતિ 18: /* e. enable */
                સંદેશ_Code = TPM_PPI_OP_ENABLE;
                તોડો;
            સ્થિતિ 32: /* d. disable */
                સંદેશ_Code = TPM_PPI_OP_DISABLE;
                તોડો;
            સ્થિતિ 30: /* a. activate */
                સંદેશ_Code = TPM_PPI_OP_ACTIVATE;
                તોડો;
            સ્થિતિ 47: /* v. deactivate */
                સંદેશ_Code = TPM_PPI_OP_DEACTIVATE;
                તોડો;
            સ્થિતિ 46: /* c. clear owner */
                સંદેશ_Code = TPM_PPI_OP_CLEAR;
                તોડો;
            સ્થિતિ 25: /* p. prevent ownerinstall */
                સંદેશ_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                તોડો;
            સ્થિતિ 31: /* s. allow ownerinstall */
                સંદેશ_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                તોડો;
            મૂળભૂત:
                ચાલુ;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            માટે (i = 0; i < કદ(next_scancodes); i++) {
                જો (next_scancodes[i] == 0)
                    તોડો;

                જો (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(સંદેશ_Code, 1);
                    waitkey = 0;
                    તોડો;
                }
            }
        }
    }
}

સ્થિર પૂર્ણાંક
tpm20_menu_change_active_pcrbanks(શૂન્ય)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    જ્યારે (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        જ્યારે (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            અચલ અક્ષર *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            જો (hashname) {
                printf("  %d: %s", i, hashname);
                જો (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        જો (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        પૂર્ણાંક show = 0;
        જ્યારે (!show) {
            પૂર્ણાંક scancode = get_keystroke(1000);

            પસંદગી (scancode) {
            સ્થિતિ ~0:
                ચાલુ;
            સ્થિતિ 1: /* ESC */
                printf("\n");
                પાછા -1;
            સ્થિતિ 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                જો (flagnum > i)
                    ચાલુ;
                જો (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                તોડો;
            સ્થિતિ 30: /* a */
                જો (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

સ્થિર શૂન્ય
tpm20_menu(શૂન્ય)
{
    પૂર્ણાંક scan_code;
    tpm_ppi_code સંદેશ_Code;

    માટે (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        સંદેશ_Code = TPM_PPI_OP_NOOP;

        જ્યારે ((scan_code = get_keystroke(1000)) == ~0)
            ;

        પસંદગી (scan_code) {
        સ્થિતિ 1:
            // ESC
            reset();
            તોડો;
        સ્થિતિ 2:
            સંદેશ_Code = TPM_PPI_OP_CLEAR;
            તોડો;
        સ્થિતિ 3:
            tpm20_menu_change_active_pcrbanks();
            ચાલુ;
        મૂળભૂત:
            ચાલુ;
        }

        tpm20_process_cfg(સંદેશ_Code, 0);
    }
}

શૂન્ય
tpm_menu(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા;

    જ્યારે (get_keystroke(0) >= 0)
        ;
    wait_threads();

    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        tpm12_menu();
        તોડો;
    સ્થિતિ TPM_VERSION_2:
        tpm20_menu();
        તોડો;
    }
}

પૂર્ણાંક
tpm_can_show_menu(શૂન્ય)
{
    પસંદગી (TPM_version) {
    સ્થિતિ TPM_VERSION_1_2:
        પાછા tpm_is_working() && TPM_has_physical_presence;
    સ્થિતિ TPM_VERSION_2:
        પાછા tpm_is_working();
    }
    પાછા 0;
}
