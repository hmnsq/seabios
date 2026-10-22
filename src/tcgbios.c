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

#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "byteorder.h" // cpu_to_*
#அந்தர்பாவ "config.h" // CONFIG_TCGBIOS
#அந்தர்பாவ "farptr.h" // MAKE_FLATPTR
#அந்தர்பாவ "fw/paravirt.h" // runningOnXen
#அந்தர்பாவ "hw/tpm_drivers.h" // tpm_drivers[]
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "sha.h" // sha1, sha256, ...
#அந்தர்பாவ "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#அந்தர்பாவ "std/smbios.h" // struct smbios_21_entry_point
#அந்தர்பாவ "std/tcg.h" // TCG_PC_LOGOVERFLOW
#அந்தர்பாவ "string.h" // checksum
#அந்தர்பாவ "tcgbios.h"// tpm_*, prototypes
#அந்தர்பாவ "util.h" // printf, get_keystroke
#அந்தர்பாவ "stacks.h" // wait_threads, reset
#அந்தர்பாவ "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

கட்டமைப்பு {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_மொத்தம்;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

நிலையான முழு tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    எனில் (!log_area_start_address || !log_area_minimum_length)
        திரும்பு -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = வெற்று;
    tpm_state.entry_மொத்தம் = 0;
    திரும்பு 0;
}

நிலையான முழு
tpm_tcpa_probe(வெறுமை)
{
    கட்டமைப்பு tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    எனில் (!tcpa)
        திரும்பு -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    திரும்பு tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

நிலையான முழு
tpm_tpm2_probe(வெறுமை)
{
    கட்டமைப்பு tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    எனில் (!tpm2)
        திரும்பு -1;

    எனில் (tpm2->length < 76)
        திரும்பு -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    திரும்பு tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
நிலையான முழு
tpm_log_event(கட்டமைப்பு tpm_log_header *entry, முழு digest_len
              , மாறிலி வெறுமை *event, முழு event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    எனில் (tpm_state.log_area_next_entry == வெற்று)
        திரும்பு -1;

    u32 size = (அளவு(*entry) + digest_len
                + அளவு(கட்டமைப்பு tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    எனில் (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        திரும்பு -1;
    }

    வெறுமை *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, அளவு(*entry) + digest_len);
    கட்டமைப்பு tpm_log_trailer *t = dest + அளவு(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_மொத்தம்++;

    திரும்பு 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

நிலையான TPMVersion TPM_version;
நிலையான u32 tpm20_pcr_selection_size;
நிலையான கட்டமைப்பு tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
கட்டமைப்பு tpm_log_entry {
    கட்டமைப்பு tpm_log_header hdr;
    u8 pad[அளவு(கட்டமைப்பு tpm2_digest_மதிப்புகள்)
           + 8 * அளவு(கட்டமைப்பு tpm2_digest_மதிப்பு)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

நிலையான மாறிலி கட்டமைப்பு hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    மாறிலி எழுத்து *name;
    வெறுமை (*hashfunc)(மாறிலி u8 *data, u32 length, u8 *hash);
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

நிலையான முழு
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    க்கு (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        எனில் (hash_parameters[i].hashalg == hashAlg)
            திரும்பு hash_parameters[i].hash_buffersize;
    }
    திரும்பு -1;
}

நிலையான u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    க்கு (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        எனில் (hash_parameters[i].hashalg == hashAlg)
            திரும்பு hash_parameters[i].hashalg_flag;
    }
    திரும்பு 0;
}

நிலையான u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    க்கு (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        எனில் (hash_parameters[i].hashalg_flag == hashalg_flag)
            திரும்பு hash_parameters[i].hashalg;
    }
    திரும்பு 0;
}

நிலையான மாறிலி எழுத்து *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    க்கு (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        எனில் (hash_parameters[i].hashalg_flag == hashalg_flag)
            திரும்பு hash_parameters[i].name;
    }
    திரும்பு வெற்று;
}

நிலையான வெறுமை tpm2_hash_data(u16 hashAlg, மாறிலி u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    க்கு (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        எனில் (hash_parameters[i].hashalg == hashAlg) {
            எனில் (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } இல்லையெனில் {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
நிலையான முழு
tpm20_write_EfiSpecIdEventStruct(வெறுமை)
{
    எனில் (!tpm20_pcr_selection)
        திரும்பு -1;

    கட்டமைப்பு {
        கட்டமைப்பு TCG_EfiSpecIdEventStruct hdr;
        u8 pad[அளவு(கட்டமைப்பு tpm_log_entry) + அளவு(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    கட்டமைப்பு tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    வெறுமை *nsel, *end = (வெறுமை*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 மொத்தம், இலக்கம்_Algs = 0;
    க்கு (மொத்தம் = 0; மொத்தம் < be32_to_cpu(tpm20_pcr_selection->மொத்தம்); மொத்தம்++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (வெறுமை*)sel + அளவு(*sel) + sizeOfSelect;
        எனில் (nsel > end)
            நிறுத்து;

        எனில் (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            தொடர்;
        }

        முழு hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        எனில் (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            திரும்பு -1;
        }

        முழு event_size = offsetof(கட்டமைப்பு TCG_EfiSpecIdEventStruct
                                  , digestSizes[மொத்தம்+1]);
        எனில் (event_size > அளவு(event) - அளவு(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            திரும்பு -1;
        }

        event.hdr.digestSizes[இலக்கம்_Algs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[இலக்கம்_Algs].digestSize = hsize;
        இலக்கம்_Algs++;

        sel = nsel;
    }

    எனில் (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        திரும்பு -1;
    }

    event.hdr.எண்_Of_Algorithms = இலக்கம்_Algs;
    முழு event_size = offsetof(கட்டமைப்பு TCG_EfiSpecIdEventStruct
                              , digestSizes[இலக்கம்_Algs]);
    u8 *vendorInfoSize = (வெறுமை*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += அளவு(*vendorInfoSize);

    கட்டமைப்பு tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    திரும்பு tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
நிலையான முழு
tpm20_build_digest(கட்டமைப்பு tpm_log_entry *le,
                   மாறிலி u8 *hashdata, u32 hashdata_len, முழு bigEndian)
{
    எனில் (!tpm20_pcr_selection)
        திரும்பு -1;

    கட்டமைப்பு tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    வெறுமை *nsel, *end = (வெறுமை*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    வெறுமை *dest = le->hdr.digest + அளவு(கட்டமைப்பு tpm2_digest_மதிப்புகள்);

    u32 மொத்தம், இலக்கம்_Algs = 0;
    க்கு (மொத்தம் = 0; மொத்தம் < be32_to_cpu(tpm20_pcr_selection->மொத்தம்); மொத்தம்++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (வெறுமை*)sel + அளவு(*sel) + sizeOfSelect;
        எனில் (nsel > end)
            நிறுத்து;

        /* PCR 0-7 unused? -- skip */
        எனில் (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            தொடர்;
        }

        முழு hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        எனில் (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            திரும்பு -1;
        }

        /* buffer size sanity check before writing */
        கட்டமைப்பு tpm2_digest_மதிப்பு *v = dest;
        எனில் (dest + அளவு(*v) + hsize > (வெறுமை*)le + அளவு(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            திரும்பு -1;
        }

        எனில் (bigEndian)
            v->hashAlg = sel->hashAlg;
        இல்லையெனில்
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += அளவு(*v) + hsize;
        sel = nsel;

        இலக்கம்_Algs++;
    }

    எனில் (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        திரும்பு -1;
    }

    கட்டமைப்பு tpm2_digest_மதிப்புகள் *v = (வெறுமை*)le->hdr.digest;
    எனில் (bigEndian)
        v->மொத்தம் = cpu_to_be32(இலக்கம்_Algs);
    இல்லையெனில்
        v->மொத்தம் = இலக்கம்_Algs;

    திரும்பு dest - (வெறுமை*)le->hdr.digest;
}

நிலையான முழு
tpm12_build_digest(கட்டமைப்பு tpm_log_entry *le,
                   மாறிலி u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    திரும்பு SHA1_BUFSIZE;
}

நிலையான முழு
tpm12_build_digest_direct(கட்டமைப்பு tpm_log_entry *le, மாறிலி u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    திரும்பு SHA1_BUFSIZE;
}

நிலையான முழு
tpm_build_digest(கட்டமைப்பு tpm_log_entry *le, மாறிலி u8 *hashdata, u32 hashdata_len
                 , முழு bigEndian)
{
    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        திரும்பு tpm12_build_digest(le, hashdata, hashdata_len);
    நிலை TPM_VERSION_2:
        திரும்பு tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    திரும்பு -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
நிலையான முழு
tpm_simple_cmd(u8 locty, u32 ordinal
               , முழு param_size, u16 param, எண்ணிக்கை tpmDurationType to_t)
{
    கட்டமைப்பு {
        கட்டமைப்பு tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(அளவு(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        நிறுத்து;
    நிலை TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        நிறுத்து;
    }

    u8 obuffer[64];
    கட்டமைப்பு tpm_rsp_header *trsh = (வெறுமை*)obuffer;
    u32 obuffer_len = அளவு(obuffer);
    memset(obuffer, 0x0, அளவு(obuffer));

    முழு ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    திரும்பு ret;
}

நிலையான முழு
tpm20_getcapability(u32 capability, u32 property, u32 மொத்தம்,
                    கட்டமைப்பு tpm_rsp_header *rsp, u32 rsize)
{
    கட்டமைப்பு tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(அளவு(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(மொத்தம்),
    };

    u32 resp_size = rsize;
    முழு ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    திரும்பு ret;
}

நிலையான முழு
tpm20_get_pcrbanks(வெறுமை)
{
    u8 buffer[128];
    கட்டமைப்பு tpm2_res_getcapability *trg =
      (கட்டமைப்பு tpm2_res_getcapability *)&buffer;

    முழு ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  அளவு(buffer));
    எனில் (ret)
        திரும்பு ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    எனில் (resplen <= offsetof(கட்டமைப்பு tpm2_res_getcapability, data))
        திரும்பு -1;

    u32 size = resplen - offsetof(கட்டமைப்பு tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    எனில் (size < offsetof(கட்டமைப்பு tpml_pcr_selection, selections) +
               offsetof(கட்டமைப்பு tpms_pcr_selection, pcrSelect))
        திரும்பு -1;

    tpm20_pcr_selection = malloc_உயர்வு(size);
    எனில் (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } இல்லையெனில் {
        warn_noalloc();
        ret = -1;
    }

    திரும்பு ret;
}

நிலையான முழு
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    எனில் (!tpm20_pcr_selection)
        திரும்பு -1;

    கட்டமைப்பு tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    வெறுமை *end = (வெறுமை*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    போது (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        வெறுமை *nsel = (வெறுமை*)sel + அளவு(*sel) + sizeOfSelect;
        எனில் (nsel > end)
            திரும்பு 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        க்கு (i = 0; i < sizeOfSelect; i++) {
            எனில் (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                நிறுத்து;
            }
        }

        sel = nsel;
    }
}

நிலையான முழு
tpm20_set_pcrbanks(u32 active_banks)
{
    கட்டமைப்பு tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(அளவு(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    கட்டமைப்பு tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    முழு i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    போது (hashalg_flag) {
        எனில் ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            எனில் (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                எனில் (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.மொத்தம் = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * அளவு(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(கட்டமைப்பு tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * அளவு(tps[0]));

    கட்டமைப்பு tpm_rsp_header rsp;
    u32 resp_length = அளவு(rsp);

    முழு ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    திரும்பு ret;
}

நிலையான முழு tpm20_activate_pcrbanks(u32 active_banks)
{
    முழு ret = tpm20_set_pcrbanks(active_banks);
    எனில் (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    எனில் (!ret)
        reset();
    திரும்பு ret;
}

நிலையான முழு
tpm12_get_capability(u32 cap, u32 subcap, கட்டமைப்பு tpm_rsp_header *rsp, u32 rsize)
{
    கட்டமைப்பு tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(அளவு(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(அளவு(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    முழு ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    திரும்பு ret;
}

நிலையான முழு
tpm12_read_permanent_flags(எழுத்து *buf, முழு buf_len)
{
    memset(buf, 0, buf_len);

    கட்டமைப்பு tpm_res_getcap_perm_flags pf;
    முழு ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, அளவு(pf));
    எனில் (ret)
        திரும்பு -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    திரும்பு 0;
}

நிலையான முழு
tpm12_determine_timeouts(வெறுமை)
{
    கட்டமைப்பு tpm_res_getcap_timeouts timeouts;
    முழு ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, அளவு(timeouts));
    எனில் (ret)
        திரும்பு ret;

    கட்டமைப்பு tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, அளவு(durations));
    எனில் (ret)
        திரும்பு ret;

    முழு i;
    க்கு (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    க்கு (i = 0; i < 4; i++)
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

    திரும்பு 0;
}

நிலையான வெறுமை
tpm20_set_timeouts(வெறுமை)
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

நிலையான முழு
tpm12_extend(கட்டமைப்பு tpm_log_entry *le, முழு digest_len)
{
    கட்டமைப்பு tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(அளவு(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, அளவு(tre.digest));

    கட்டமைப்பு tpm_rsp_extend rsp;
    u32 resp_length = அளவு(rsp);
    முழு ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.hdr.errcode)
        திரும்பு -1;

    திரும்பு 0;
}

நிலையான முழு tpm20_extend(கட்டமைப்பு tpm_log_entry *le, முழு digest_len)
{
    கட்டமைப்பு tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(அளவு(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[அளவு(tmp_tre) + அளவு(le->pad)];
    கட்டமைப்பு tpm2_req_extend *tre = (கட்டமைப்பு tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, அளவு(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(அளவு(tmp_tre) + digest_len);

    கட்டமைப்பு tpm_rsp_header rsp;
    u32 resp_length = அளவு(rsp);
    முழு ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.errcode)
        திரும்பு -1;

    திரும்பு 0;
}

நிலையான முழு
tpm_extend(கட்டமைப்பு tpm_log_entry *le, முழு digest_len)
{
    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        திரும்பு tpm12_extend(le, digest_len);
    நிலை TPM_VERSION_2:
        திரும்பு tpm20_extend(le, digest_len);
    }
    திரும்பு -1;
}

நிலையான முழு
tpm20_stirrandom(வெறுமை)
{
    கட்டமைப்பு tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(அளவு(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(அளவு(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    கட்டமைப்பு tpm_rsp_header rsp;
    u32 resp_length = அளவு(rsp);
    முழு ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    திரும்பு ret;
}

நிலையான முழு
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    கட்டமைப்பு tpm2_res_getrandom rsp;

    எனில் (buf_len > அளவு(rsp.rnd.buffer))
        திரும்பு -1;

    கட்டமைப்பு tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(அளவு(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = அளவு(rsp);

    முழு ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.hdr.errcode)
        ret = -1;
    இல்லையெனில்
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    திரும்பு ret;
}

நிலையான முழு
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    கட்டமைப்பு tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(அளவு(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(அளவு(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    கட்டமைப்பு tpm_rsp_header rsp;
    u32 resp_length = அளவு(rsp);
    முழு ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    திரும்பு ret;
}

நிலையான முழு
tpm20_hierarchychangeauth(u8 auth[20])
{
    கட்டமைப்பு tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(அளவு(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(அளவு(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(அளவு(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, அளவு(trhca.newAuth.buffer));

    கட்டமைப்பு tpm_rsp_header rsp;
    u32 resp_length = அளவு(rsp);
    முழு ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    திரும்பு ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

நிலையான முழு TPM_has_physical_presence;
u8 TPM_working VARLOW;

நிலையான முழு
tpm_is_working(வெறுமை)
{
    திரும்பு CONFIG_TCGBIOS && TPM_working;
}

நிலையான வெறுமை
tpm_set_failure(வெறுமை)
{
    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        நிறுத்து;
    நிலை TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        நிறுத்து;
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
நிலையான வெறுமை
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           மாறிலி எழுத்து *event, u32 event_length,
                           மாறிலி u8 *hashdata, u32 hashdata_length)
{
    எனில் (!tpm_is_working())
        திரும்பு;

    கட்டமைப்பு tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    முழு digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    எனில் (digest_len < 0)
        திரும்பு;
    முழு ret = tpm_extend(&le, digest_len);
    எனில் (ret) {
        tpm_set_failure();
        திரும்பு;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
நிலையான வெறுமை
tpm_add_action(u32 pcrIndex, மாறிலி எழுத்து *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
நிலையான வெறுமை
tpm_add_event_separators(வெறுமை)
{
    நிலையான மாறிலி u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    க்கு (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (மாறிலி எழுத்து *)evt_separator,
                                   அளவு(evt_separator),
                                   evt_separator,
                                   அளவு(evt_separator));
}

நிலையான வெறுமை
tpm_smbios_measure(வெறுமை)
{
    கட்டமைப்பு pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    வெறுமை *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    எனில் (!smbios_tables)
        திரும்பு;

    sha1((மாறிலி u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (மாறிலி எழுத்து *)&pcctes, அளவு(pcctes),
                               (u8 *)&pcctes, அளவு(pcctes));
}

நிலையான முழு
tpm12_assert_physical_presence(வெறுமை)
{
    முழு ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    எனில் (!ret)
        திரும்பு 0;

    கட்டமைப்பு tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((எழுத்து *)&pf, அளவு(pf));
    எனில் (ret)
        திரும்பு -1;

    /* check if hardware physical presence is supported */
    எனில் (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        திரும்பு 0;
    }

    எனில் (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        திரும்பு tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    திரும்பு -1;
}

நிலையான முழு
tpm12_startup(வெறுமை)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    முழு ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    எனில் (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    எனில் (ret)
        செல் err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    எனில் (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    எனில் (ret)
        செல் err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    எனில் (ret)
        செல் err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    எனில் (ret && ret != TPM_BAD_LOCALITY)
        செல் err_exit;

    திரும்பு 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    திரும்பு -1;
}

நிலையான முழு
tpm20_startup(வெறுமை)
{
    tpm20_set_timeouts();

    முழு ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    எனில் (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    எனில் (ret)
        செல் err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    எனில் (ret)
        செல் err_exit;

    ret = tpm20_get_pcrbanks();
    எனில் (ret)
        செல் err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    எனில் (ret)
        செல் err_exit;

    திரும்பு 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    திரும்பு -1;
}

நிலையான முழு
tpm_startup(வெறுமை)
{
    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        திரும்பு tpm12_startup();
    நிலை TPM_VERSION_2:
        திரும்பு tpm20_startup();
    }
    திரும்பு -1;
}

வெறுமை
tpm_setup(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு;

    முழு ret = tpm_tpm2_probe();
    எனில் (ret) {
        ret = tpm_tcpa_probe();
        எனில் (ret)
            திரும்பு;
    }

    TPM_version = tpmhw_probe();
    எனில் (TPM_version == TPM_VERSION_NONE)
        திரும்பு;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    எனில் (runningOnXen())
        திரும்பு;

    ret = tpm_startup();
    எனில் (ret)
        திரும்பு;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

நிலையான வெறுமை
tpm20_prepboot(வெறுமை)
{
    முழு ret = tpm20_stirrandom();
    எனில் (ret)
         செல் err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], அளவு(auth));
    எனில் (ret)
        செல் err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    எனில் (ret)
        செல் err_exit;

    திரும்பு;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

வெறுமை
tpm_prepboot(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு;

    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        எனில் (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        நிறுத்து;
    நிலை TPM_VERSION_2:
        tpm20_prepboot();
        நிறுத்து;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
வெறுமை
tpm_option_rom(மாறிலி வெறுமை *addr, u32 len)
{
    எனில் (!tpm_is_working())
        திரும்பு;

    கட்டமைப்பு pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = அளவு(u16) + அளவு(u16) + SHA1_BUFSIZE,
    };
    sha1((மாறிலி u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (மாறிலி எழுத்து *)&pcctes, அளவு(pcctes),
                               (u8 *)&pcctes, அளவு(pcctes));
}

வெறுமை
tpm_add_bcv(u32 bootdrv, மாறிலி u8 *addr, u32 length)
{
    எனில் (!tpm_is_working())
        திரும்பு;

    எனில் (length < 0x200)
        திரும்பு;

    மாறிலி எழுத்து *string = "Booting BCV device 00h (Floppy)";
    எனில் (bootdrv == 0x80)
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

வெறுமை
tpm_add_cdrom(u32 bootdrv, மாறிலி u8 *addr, u32 length)
{
    எனில் (!tpm_is_working())
        திரும்பு;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    மாறிலி எழுத்து *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

வெறுமை
tpm_add_cdrom_catalog(மாறிலி u8 *addr, u32 length)
{
    எனில் (!tpm_is_working())
        திரும்பு;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    மாறிலி எழுத்து *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

வெறுமை
tpm_s3_resume(வெறுமை)
{
    எனில் (!tpm_is_working())
        திரும்பு;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    முழு ret = -1;

    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        நிறுத்து;
    நிலை TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        எனில் (ret)
            செல் err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        நிறுத்து;
    }

    எனில் (ret)
        செல் err_exit;

    திரும்பு;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

நிலையான உள்வரி வெறுமை *உள்ளீடு_buf32(கட்டமைப்பு bregs *regs)
{
    திரும்பு MAKE_FLATPTR(regs->es, regs->di);
}

நிலையான உள்வரி வெறுமை *output_buf32(கட்டமைப்பு bregs *regs)
{
    திரும்பு MAKE_FLATPTR(regs->ds, regs->si);
}

நிலையான u32
hash_log_extend(கட்டமைப்பு pcpes *pcpes, மாறிலி வெறுமை *hashdata, u32 hashdata_length
                , வெறுமை *event, முழு extend)
{
    எனில் (pcpes->pcrindex >= 24)
        திரும்பு TCG_INVALID_உள்ளீடு_PARA;
    எனில் (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    கட்டமைப்பு tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    முழு digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    எனில் (digest_len < 0)
        திரும்பு TCG_GENERAL_ERROR;
    எனில் (extend) {
        முழு ret = tpm_extend(&le, digest_len);
        எனில் (ret)
            திரும்பு TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    முழு ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    எனில் (ret)
        திரும்பு TCG_PC_LOGOVERFLOW;
    திரும்பு 0;
}

நிலையான u32
hash_log_extend_event_int(மாறிலி கட்டமைப்பு hleei_short *hleei_s,
                          கட்டமைப்பு hleeo *hleeo)
{
    u32 rc = 0;
    கட்டமைப்பு hleei_long *hleei_l = (கட்டமைப்பு hleei_long *)hleei_s;
    மாறிலி வெறுமை *logdataptr;
    u32 logdatalen;
    கட்டமைப்பு pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    தேர்வு (hleei_s->ipblength) {
    நிலை அளவு(கட்டமைப்பு hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    நிறுத்து;

    நிலை அளவு(கட்டமைப்பு hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    நிறுத்து;

    இயல்பு:
        /* bad input block */
        rc = TCG_INVALID_உள்ளீடு_PARA;
        செல் err_exit;
    }

    pcpes = (கட்டமைப்பு pcpes *)logdataptr;

    எனில் (pcpes->pcrindex != pcrindex
        || logdatalen != அளவு(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_உள்ளீடு_PARA;
        செல் err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    எனில் (rc)
        செல் err_exit;

    hleeo->opblength = அளவு(கட்டமைப்பு hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_மொத்தம்;
    memcpy(hleeo->digest, pcpes->digest, அளவு(hleeo->digest));

err_exit:
    எனில் (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    திரும்பு rc;
}

நிலையான u32
pass_through_to_tpm_int(கட்டமைப்பு pttti *pttti, கட்டமைப்பு pttto *pttto)
{
    u32 rc = 0;
    கட்டமைப்பு tpm_req_header *trh = (வெறுமை*)pttti->tpmopin;

    எனில் (pttti->ipblength < அளவு(கட்டமைப்பு pttti) + அளவு(*trh)
        || pttti->ipblength != அளவு(கட்டமைப்பு pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < அளவு(கட்டமைப்பு pttto)) {
        rc = TCG_INVALID_உள்ளீடு_PARA;
        செல் err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        எனில் (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_உள்ளீடு_PARA;
            செல் err_exit;
        }
        நிறுத்து;
    நிலை TPM_VERSION_2:
        எனில் (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_உள்ளீடு_PARA;
            செல் err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(கட்டமைப்பு pttto, tpmopout);
    முழு ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    எனில் (ret) {
        rc = TCG_FATAL_COM_ERROR;
        செல் err_exit;
    }

    pttto->opblength = offsetof(கட்டமைப்பு pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    எனில் (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    திரும்பு rc;
}

நிலையான u32
shutdown_preboot_interface(வெறுமை)
{
    TPM_interface_shutdown = 1;
    திரும்பு 0;
}

நிலையான u32
hash_log_event_int(மாறிலி கட்டமைப்பு hlei *hlei, கட்டமைப்பு hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    கட்டமைப்பு pcpes *pcpes;

    size = hlei->ipblength;
    எனில் (size != அளவு(*hlei)) {
        rc = TCG_INVALID_உள்ளீடு_PARA;
        செல் err_exit;
    }

    pcpes = (கட்டமைப்பு pcpes *)hlei->logdataptr;

    எனில் (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != அளவு(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_உள்ளீடு_PARA;
        செல் err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    எனில் (rc)
        செல் err_exit;

    /* updating the log was fine */
    hleo->opblength = அளவு(கட்டமைப்பு hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_மொத்தம்;

err_exit:
    எனில் (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    திரும்பு rc;
}

நிலையான u32
hash_all_int(மாறிலி கட்டமைப்பு hai *hai, u8 *hash)
{
    எனில் (hai->ipblength != அளவு(கட்டமைப்பு hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        திரும்பு TCG_INVALID_உள்ளீடு_PARA;

    sha1((மாறிலி u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    திரும்பு 0;
}

நிலையான u32
tss_int(கட்டமைப்பு ti *ti, கட்டமைப்பு to *to)
{
    to->opblength = அளவு(கட்டமைப்பு to);
    to->reserved  = 0;

    திரும்பு TCG_PC_UNSUPPORTED;
}

நிலையான u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    கட்டமைப்பு pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = அளவு(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    எனில் (rc)
        திரும்பு rc;

    *edx_ptr = tpm_state.entry_மொத்தம்;
    திரும்பு 0;
}

வெறுமை VISIBLE32FLAT
tpm_interrupt_handler32(கட்டமைப்பு bregs *regs)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு;

    set_cf(regs, 0);

    எனில் (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        திரும்பு;
    }

    தேர்வு ((எண்ணிக்கை irq_ids)regs->al) {
    நிலை TCG_StatusCheck:
        எனில் (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } இல்லையெனில் {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        நிறுத்து;

    நிலை TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (கட்டமைப்பு hleei_short *)உள்ளீடு_buf32(regs),
                  (கட்டமைப்பு hleeo *)output_buf32(regs));
        நிறுத்து;

    நிலை TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((கட்டமைப்பு pttti *)உள்ளீடு_buf32(regs),
                                    (கட்டமைப்பு pttto *)output_buf32(regs));
        நிறுத்து;

    நிலை TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        நிறுத்து;

    நிலை TCG_HashLogEvent:
        regs->eax = hash_log_event_int((கட்டமைப்பு hlei*)உள்ளீடு_buf32(regs),
                                       (கட்டமைப்பு hleo*)output_buf32(regs));
        நிறுத்து;

    நிலை TCG_HashAll:
        regs->eax =
            hash_all_int((கட்டமைப்பு hai*)உள்ளீடு_buf32(regs),
                          (u8 *)output_buf32(regs));
        நிறுத்து;

    நிலை TCG_TSS:
        regs->eax = tss_int((கட்டமைப்பு ti*)உள்ளீடு_buf32(regs),
                            (கட்டமைப்பு to*)output_buf32(regs));
        நிறுத்து;

    நிலை TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)உள்ளீடு_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        நிறுத்து;

    இயல்பு:
        set_cf(regs, 1);
    }

    திரும்பு;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

வகைவரை u8 tpm_ppi_code;

நிலையான முழு
tpm12_read_has_owner(முழு *has_owner)
{
    கட்டமைப்பு tpm_res_getcap_ownerauth oauth;
    முழு ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, அளவு(oauth));
    எனில் (ret)
        திரும்பு -1;

    *has_owner = oauth.flag;

    திரும்பு 0;
}

நிலையான முழு
tpm12_enable_tpm(முழு enable, முழு verbose)
{
    கட்டமைப்பு tpm_permanent_flags pf;
    முழு ret = tpm12_read_permanent_flags((எழுத்து *)&pf, அளவு(pf));
    எனில் (ret)
        திரும்பு -1;

    எனில் (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        திரும்பு 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    எனில் (ret) {
        எனில் (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        இல்லையெனில்
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    திரும்பு ret;
}

நிலையான முழு
tpm12_activate_tpm(முழு activate, முழு allow_reset, முழு verbose)
{
    கட்டமைப்பு tpm_permanent_flags pf;
    முழு ret = tpm12_read_permanent_flags((எழுத்து *)&pf, அளவு(pf));
    எனில் (ret)
        திரும்பு -1;

    எனில் (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        திரும்பு 0;

    எனில் (pf.flags[PERM_FLAG_IDX_DISABLE])
        திரும்பு 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    எனில் (ret)
        திரும்பு ret;

    எனில் (activate && allow_reset) {
        எனில் (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    திரும்பு 0;
}

நிலையான முழு
tpm12_enable_activate(முழு allow_reset, முழு verbose)
{
    முழு ret = tpm12_enable_tpm(1, verbose);
    எனில் (ret)
        திரும்பு ret;

    திரும்பு tpm12_activate_tpm(1, allow_reset, verbose);
}

நிலையான முழு
tpm12_force_clear(முழு enable_activate_before, முழு enable_activate_after,
                  முழு verbose)
{
    முழு has_owner;
    முழு ret = tpm12_read_has_owner(&has_owner);
    எனில் (ret)
        திரும்பு -1;
    எனில் (!has_owner) {
        எனில் (verbose)
            printf("TPM does not have an owner.\n");
        திரும்பு 0;
    }

    எனில் (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        எனில் (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            திரும்பு ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    எனில் (ret)
        திரும்பு ret;

    எனில் (!enable_activate_after) {
        எனில் (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        திரும்பு 0;
    }

    திரும்பு tpm12_enable_activate(1, verbose);
}

நிலையான முழு
tpm12_set_owner_install(முழு allow, முழு verbose)
{
    முழு has_owner;
    முழு ret = tpm12_read_has_owner(&has_owner);
    எனில் (ret)
        திரும்பு -1;
    எனில் (has_owner) {
        எனில் (verbose)
            printf("Must first remove owner.\n");
        திரும்பு 0;
    }

    கட்டமைப்பு tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((எழுத்து *)&pf, அளவு(pf));
    எனில் (ret)
        திரும்பு -1;

    எனில் (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        எனில் (verbose)
            printf("TPM must first be enable.\n");
        திரும்பு 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    எனில் (ret)
        திரும்பு ret;

    எனில் (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    திரும்பு 0;
}

நிலையான முழு
tpm12_process_cfg(tpm_ppi_code செய்தி_Code, முழு verbose)
{
    முழு ret = 0;

    தேர்வு (செய்தி_Code) {
        நிலை TPM_PPI_OP_NOOP: /* no-op */
            நிறுத்து;

        நிலை TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            நிறுத்து;

        நிலை TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            நிறுத்து;

        நிலை TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            நிறுத்து;

        நிலை TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            நிறுத்து;

        நிலை TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            நிறுத்து;

        நிலை TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            நிறுத்து;

        நிலை TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            நிறுத்து;

        இயல்பு:
            நிறுத்து;
    }

    எனில் (ret)
        printf("Op %d: An error occurred: 0x%x\n", செய்தி_Code, ret);

    திரும்பு ret;
}

நிலையான முழு
tpm20_clearcontrol(u8 disable, முழு verbose)
{
    கட்டமைப்பு tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(அளவு(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(அளவு(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    கட்டமைப்பு tpm_rsp_header rsp;
    u32 resp_length = அளவு(rsp);
    முழு ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    திரும்பு ret;
}

நிலையான முழு
tpm20_clear(வெறுமை)
{
    கட்டமைப்பு tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(அளவு(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(அளவு(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    கட்டமைப்பு tpm_rsp_header rsp;
    u32 resp_length = அளவு(rsp);
    முழு ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    எனில் (ret || resp_length != அளவு(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    திரும்பு ret;
}

நிலையான முழு
tpm20_process_cfg(tpm_ppi_code செய்தி_Code, முழு verbose)
{
    முழு ret = 0;

    தேர்வு (செய்தி_Code) {
        நிலை TPM_PPI_OP_NOOP: /* no-op */
            நிறுத்து;

        நிலை TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            எனில் (!ret)
                 ret = tpm20_clear();
            நிறுத்து;
    }

    எனில் (ret)
        printf("Op %d: An error occurred: 0x%x\n", செய்தி_Code, ret);

    திரும்பு ret;
}

நிலையான முழு
tpm12_get_tpm_state(வெறுமை)
{
    முழு state = 0;
    கட்டமைப்பு tpm_permanent_flags pf;
    முழு has_owner;

    எனில் (tpm12_read_permanent_flags((எழுத்து *)&pf, அளவு(pf)) ||
        tpm12_read_has_owner(&has_owner))
        திரும்பு ~0;

    எனில் (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    எனில் (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    எனில் (has_owner) {
        state |= TPM_STATE_OWNED;
    } இல்லையெனில் {
        எனில் (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    திரும்பு state;
}

நிலையான வெறுமை
tpm12_show_tpm_menu(முழு state, முழு next_scancodes[7])
{
    முழு i = 0;

    printf("\nThe current state of the TPM is:\n");

    எனில் (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    இல்லையெனில்
        printf("  Disabled");

    எனில் (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    இல்லையெனில்
        printf(" and deactivated\n");

    எனில் (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    இல்லையெனில் {
        printf("  Ownership has not been taken\n");
        எனில் (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        இல்லையெனில்
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    எனில் ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    எனில் (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        எனில் (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            எனில் (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } இல்லையெனில் {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } இல்லையெனில் {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } இல்லையெனில் {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    எனில் (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

நிலையான வெறுமை
tpm12_menu(வெறுமை)
{
    முழு scancode, next_scancodes[7];
    tpm_ppi_code செய்தி_Code;
    முழு state = 0, i;
    முழு waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    க்கு (;;) {
        எனில் ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } இல்லையெனில் {
            printf("TPM is not working correctly.\n");
            திரும்பு;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        செய்தி_Code = TPM_PPI_OP_NOOP;

        waitkey = 1;

        போது (waitkey) {
            போது ((scancode = get_keystroke(1000)) == ~0)
                ;

            தேர்வு (scancode) {
            நிலை 1:
                // ESC
                reset();
                நிறுத்து;
            நிலை 18: /* e. enable */
                செய்தி_Code = TPM_PPI_OP_ENABLE;
                நிறுத்து;
            நிலை 32: /* d. disable */
                செய்தி_Code = TPM_PPI_OP_DISABLE;
                நிறுத்து;
            நிலை 30: /* a. activate */
                செய்தி_Code = TPM_PPI_OP_ACTIVATE;
                நிறுத்து;
            நிலை 47: /* v. deactivate */
                செய்தி_Code = TPM_PPI_OP_DEACTIVATE;
                நிறுத்து;
            நிலை 46: /* c. clear owner */
                செய்தி_Code = TPM_PPI_OP_CLEAR;
                நிறுத்து;
            நிலை 25: /* p. prevent ownerinstall */
                செய்தி_Code = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                நிறுத்து;
            நிலை 31: /* s. allow ownerinstall */
                செய்தி_Code = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                நிறுத்து;
            இயல்பு:
                தொடர்;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            க்கு (i = 0; i < அளவு(next_scancodes); i++) {
                எனில் (next_scancodes[i] == 0)
                    நிறுத்து;

                எனில் (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(செய்தி_Code, 1);
                    waitkey = 0;
                    நிறுத்து;
                }
            }
        }
    }
}

நிலையான முழு
tpm20_menu_change_active_pcrbanks(வெறுமை)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    போது (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        போது (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            மாறிலி எழுத்து *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            எனில் (hashname) {
                printf("  %d: %s", i, hashname);
                எனில் (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        எனில் (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        முழு show = 0;
        போது (!show) {
            முழு scancode = get_keystroke(1000);

            தேர்வு (scancode) {
            நிலை ~0:
                தொடர்;
            நிலை 1: /* ESC */
                printf("\n");
                திரும்பு -1;
            நிலை 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                எனில் (flagnum > i)
                    தொடர்;
                எனில் (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                நிறுத்து;
            நிலை 30: /* a */
                எனில் (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

நிலையான வெறுமை
tpm20_menu(வெறுமை)
{
    முழு scan_code;
    tpm_ppi_code செய்தி_Code;

    க்கு (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        செய்தி_Code = TPM_PPI_OP_NOOP;

        போது ((scan_code = get_keystroke(1000)) == ~0)
            ;

        தேர்வு (scan_code) {
        நிலை 1:
            // ESC
            reset();
            நிறுத்து;
        நிலை 2:
            செய்தி_Code = TPM_PPI_OP_CLEAR;
            நிறுத்து;
        நிலை 3:
            tpm20_menu_change_active_pcrbanks();
            தொடர்;
        இயல்பு:
            தொடர்;
        }

        tpm20_process_cfg(செய்தி_Code, 0);
    }
}

வெறுமை
tpm_menu(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு;

    போது (get_keystroke(0) >= 0)
        ;
    wait_threads();

    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        tpm12_menu();
        நிறுத்து;
    நிலை TPM_VERSION_2:
        tpm20_menu();
        நிறுத்து;
    }
}

முழு
tpm_can_show_menu(வெறுமை)
{
    தேர்வு (TPM_version) {
    நிலை TPM_VERSION_1_2:
        திரும்பு tpm_is_working() && TPM_has_physical_presence;
    நிலை TPM_VERSION_2:
        திரும்பு tpm_is_working();
    }
    திரும்பு 0;
}
