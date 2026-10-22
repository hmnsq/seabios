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

#ضمن "bregs.h" // struct bregs
#ضمن "byteorder.h" // cpu_to_*
#ضمن "config.h" // CONFIG_TCGBIOS
#ضمن "farptr.h" // MAKE_FLATPTR
#ضمن "fw/paravirt.h" // runningOnXen
#ضمن "hw/tpm_drivers.h" // tpm_drivers[]
#ضمن "output.h" // dprintf
#ضمن "sha.h" // sha1, sha256, ...
#ضمن "std/acpi.h"  // RSDP_SIGNATURE, rsdt_descriptor
#ضمن "std/smbios.h" // struct smbios_21_entry_point
#ضمن "std/tcg.h" // TCG_PC_LOGOVERFLOW
#ضمن "string.h" // checksum
#ضمن "tcgbios.h"// tpm_*, prototypes
#ضمن "util.h" // printf, get_keystroke
#ضمن "stacks.h" // wait_threads, reset
#ضمن "malloc.h" // malloc_high


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

بنية {
    /* length of the TCPA log buffer */
    u32           log_area_minimum_length;

    /* start address of TCPA log buffer */
    u8 *          log_area_start_address;

    /* number of log entries written */
    u32           entry_count;

    /* address to write next log entry to */
    u8 *          log_area_next_entry;

    /* address of last entry written (need for TCG_StatusCheck) */
    u8 *          log_area_last_entry;
} tpm_state VARLOW;

ساكن صحيح tpm_set_log_area(u8 *log_area_start_address,
                            u32 log_area_minimum_length)
{
    ܐܢ (!log_area_start_address || !log_area_minimum_length)
        ܦܢܐ -1;

    memset(log_area_start_address, 0, log_area_minimum_length);
    tpm_state.log_area_start_address = log_area_start_address;
    tpm_state.log_area_minimum_length = log_area_minimum_length;
    tpm_state.log_area_next_entry = log_area_start_address;
    tpm_state.log_area_last_entry = عدم;
    tpm_state.entry_count = 0;
    ܦܢܐ 0;
}

ساكن صحيح
tpm_tcpa_probe(فراغ)
{
    بنية tcpa_descriptor_rev2 *tcpa = find_acpi_table(TCPA_SIGNATURE);
    ܐܢ (!tcpa)
        ܦܢܐ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TCPA: LASA = %p, LAML = %u\n",
            (u8 *)(long)tcpa->log_area_start_address,
            tcpa->log_area_minimum_length);

    ܦܢܐ tpm_set_log_area((u8*)(long)tcpa->log_area_start_address,
                            tcpa->log_area_minimum_length);
}

ساكن صحيح
tpm_tpm2_probe(فراغ)
{
    بنية tpm2_descriptor_rev2 *tpm2 = find_acpi_table(TPM2_SIGNATURE);
    ܐܢ (!tpm2)
        ܦܢܐ -1;

    ܐܢ (tpm2->length < 76)
        ܦܢܐ -1;

    dprintf(DEBUG_tcg, "TCGBIOS: TPM2: LASA = %p, LAML = %u\n",
            (u8 *)(long)tpm2->log_area_start_address,
            tpm2->log_area_minimum_length);

    ܦܢܐ tpm_set_log_area((u8*)(long)tpm2->log_area_start_address,
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
ساكن صحيح
tpm_log_event(بنية tpm_log_header *entry, صحيح digest_len
              , ثابت فراغ *event, صحيح event_len)
{
    dprintf(DEBUG_tcg, "TCGBIOS: LASA = %p, next entry = %p\n",
            tpm_state.log_area_start_address, tpm_state.log_area_next_entry);

    ܐܢ (tpm_state.log_area_next_entry == عدم)
        ܦܢܐ -1;

    u32 size = (حجم(*entry) + digest_len
                + حجم(بنية tpm_log_trailer) + event_len);
    u32 logsize = (tpm_state.log_area_next_entry + size
                   - tpm_state.log_area_start_address);
    ܐܢ (logsize > tpm_state.log_area_minimum_length) {
        dprintf(DEBUG_tcg, "TCGBIOS: LOG OVERFLOW: size = %d\n", size);
        ܦܢܐ -1;
    }

    فراغ *dest = tpm_state.log_area_next_entry;
    memcpy(dest, entry, حجم(*entry) + digest_len);
    بنية tpm_log_trailer *t = dest + حجم(*entry) + digest_len;
    t->eventdatasize = event_len;
    memcpy(t->event, event, event_len);

    tpm_state.log_area_last_entry = tpm_state.log_area_next_entry;
    tpm_state.log_area_next_entry += size;
    tpm_state.entry_count++;

    ܦܢܐ 0;
}


/****************************************************************
 * Digest formatting
 ****************************************************************/

ساكن TPMVersion TPM_version;
ساكن u32 tpm20_pcr_selection_size;
ساكن بنية tpml_pcr_selection *tpm20_pcr_selection;

// A 'struct tpm_log_entry' is a local data structure containing a
// 'tpm_log_header' followed by space for the maximum supported
// digest.  (The digest is a sha1 hash on tpm1.2 or a series of
// tpm2_digest_value structs on tpm2.0)
بنية tpm_log_entry {
    بنية tpm_log_header hdr;
    u8 pad[حجم(بنية tpm2_digest_values)
           + 8 * حجم(بنية tpm2_digest_value)
           + SHA1_BUFSIZE + SHA256_BUFSIZE + SHA384_BUFSIZE
           + SHA512_BUFSIZE + SM3_256_BUFSIZE + SHA3_256_BUFSIZE
           + SHA3_384_BUFSIZE + SHA3_512_BUFSIZE];
} PACKED;

ساكن ثابت بنية hash_parameters {
    u16 hashalg;
    u8  hashalg_flag;
    u8  hash_buffersize;
    ثابت حرف *name;
    فراغ (*hashfunc)(ثابت u8 *data, u32 length, u8 *hash);
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

ساكن صحيح
tpm20_get_hash_buffersize(u16 hashAlg)
{
    unsigned i;

    لكل (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ܐܢ (hash_parameters[i].hashalg == hashAlg)
            ܦܢܐ hash_parameters[i].hash_buffersize;
    }
    ܦܢܐ -1;
}

ساكن u8
tpm20_hashalg_to_flag(u16 hashAlg)
{
    unsigned i;

    لكل (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ܐܢ (hash_parameters[i].hashalg == hashAlg)
            ܦܢܐ hash_parameters[i].hashalg_flag;
    }
    ܦܢܐ 0;
}

ساكن u16
tpm20_hashalg_flag_to_hashalg(u8 hashalg_flag)
{
    unsigned i;

    لكل (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ܐܢ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ܦܢܐ hash_parameters[i].hashalg;
    }
    ܦܢܐ 0;
}

ساكن ثابت حرف *
tpm20_hashalg_flag_to_name(u8 hashalg_flag)
{
    unsigned i;

    لكل (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ܐܢ (hash_parameters[i].hashalg_flag == hashalg_flag)
            ܦܢܐ hash_parameters[i].name;
    }
    ܦܢܐ عدم;
}

ساكن فراغ tpm2_hash_data(u16 hashAlg, ثابت u8 *data, u32 data_len, u8 *hash)
{
    unsigned i;

    لكل (i = 0; i < ARRAY_SIZE(hash_parameters); i++) {
        ܐܢ (hash_parameters[i].hashalg == hashAlg) {
            ܐܢ (hash_parameters[i].hashfunc) {
                hash_parameters[i].hashfunc(data, data_len, hash);
            } ܐܚܪܢܐ {
                memset(hash, 0xff, hash_parameters[i].hash_buffersize);
            }
        }
    }
}

// Add an entry at the start of the log describing digest formats
ساكن صحيح
tpm20_write_EfiSpecIdEventStruct(فراغ)
{
    ܐܢ (!tpm20_pcr_selection)
        ܦܢܐ -1;

    بنية {
        بنية TCG_EfiSpecIdEventStruct hdr;
        u8 pad[حجم(بنية tpm_log_entry) + حجم(u8)];
    } event = {
        .hdr.signature = "Spec ID Event03",
        .hdr.platformClass = TPM_TCPA_ACPI_CLASS_CLIENT,
        .hdr.specVersionMinor = 0,
        .hdr.specVersionMajor = 2,
        .hdr.specErrata = 2,
        .hdr.uintnSize = 2,
    };

    بنية tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    فراغ *nsel, *end = (فراغ*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    u32 count, numAlgs = 0;
    لكل (count = 0; count < be32_to_cpu(tpm20_pcr_selection->count); count++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (فراغ*)sel + حجم(*sel) + sizeOfSelect;
        ܐܢ (nsel > end)
            اكسر;

        ܐܢ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            استمر;
        }

        صحيح hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ܐܢ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ܦܢܐ -1;
        }

        صحيح event_size = offsetof(بنية TCG_EfiSpecIdEventStruct
                                  , digestSizes[count+1]);
        ܐܢ (event_size > حجم(event) - حجم(u8)) {
            dprintf(DEBUG_tcg, "EfiSpecIdEventStruct pad too small\n");
            ܦܢܐ -1;
        }

        event.hdr.digestSizes[numAlgs].algorithmId = be16_to_cpu(sel->hashAlg);
        event.hdr.digestSizes[numAlgs].digestSize = hsize;
        numAlgs++;

        sel = nsel;
    }

    ܐܢ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ܦܢܐ -1;
    }

    event.hdr.numberOfAlgorithms = numAlgs;
    صحيح event_size = offsetof(بنية TCG_EfiSpecIdEventStruct
                              , digestSizes[numAlgs]);
    u8 *vendorInfoSize = (فراغ*)&event + event_size;
    *vendorInfoSize = 0;
    event_size += حجم(*vendorInfoSize);

    بنية tpm_log_entry le = {
        .hdr.eventtype = EV_NO_ACTION,
    };
    ܦܢܐ tpm_log_event(&le.hdr, SHA1_BUFSIZE, &event, event_size);
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
ساكن صحيح
tpm20_build_digest(بنية tpm_log_entry *le,
                   ثابت u8 *hashdata, u32 hashdata_len, صحيح bigEndian)
{
    ܐܢ (!tpm20_pcr_selection)
        ܦܢܐ -1;

    بنية tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    فراغ *nsel, *end = (فراغ*)tpm20_pcr_selection + tpm20_pcr_selection_size;
    فراغ *dest = le->hdr.digest + حجم(بنية tpm2_digest_values);

    u32 count, numAlgs = 0;
    لكل (count = 0; count < be32_to_cpu(tpm20_pcr_selection->count); count++) {
        u8 sizeOfSelect = sel->sizeOfSelect;

        nsel = (فراغ*)sel + حجم(*sel) + sizeOfSelect;
        ܐܢ (nsel > end)
            اكسر;

        /* PCR 0-7 unused? -- skip */
        ܐܢ (!sizeOfSelect || sel->pcrSelect[0] == 0) {
            sel = nsel;
            استمر;
        }

        صحيح hsize = tpm20_get_hash_buffersize(be16_to_cpu(sel->hashAlg));
        ܐܢ (hsize < 0) {
            dprintf(DEBUG_tcg, "TPM is using an unsupported hash: %d\n",
                    be16_to_cpu(sel->hashAlg));
            ܦܢܐ -1;
        }

        /* buffer size sanity check before writing */
        بنية tpm2_digest_value *v = dest;
        ܐܢ (dest + حجم(*v) + hsize > (فراغ*)le + حجم(*le)) {
            dprintf(DEBUG_tcg, "tpm_log_entry is too small\n");
            ܦܢܐ -1;
        }

        ܐܢ (bigEndian)
            v->hashAlg = sel->hashAlg;
        ܐܚܪܢܐ
            v->hashAlg = be16_to_cpu(sel->hashAlg);

        tpm2_hash_data(be16_to_cpu(sel->hashAlg), hashdata, hashdata_len,
                       v->hash);

        dest += حجم(*v) + hsize;
        sel = nsel;

        numAlgs++;
    }

    ܐܢ (sel != end) {
        dprintf(DEBUG_tcg, "Malformed pcr selection structure fron TPM\n");
        ܦܢܐ -1;
    }

    بنية tpm2_digest_values *v = (فراغ*)le->hdr.digest;
    ܐܢ (bigEndian)
        v->count = cpu_to_be32(numAlgs);
    ܐܚܪܢܐ
        v->count = numAlgs;

    ܦܢܐ dest - (فراغ*)le->hdr.digest;
}

ساكن صحيح
tpm12_build_digest(بنية tpm_log_entry *le,
                   ثابت u8 *hashdata, u32 hashdata_len)
{
    sha1(hashdata, hashdata_len, le->hdr.digest);
    ܦܢܐ SHA1_BUFSIZE;
}

ساكن صحيح
tpm12_build_digest_direct(بنية tpm_log_entry *le, ثابت u8 *sha1)
{
    // On TPM 1.2 the digest contains just the SHA1 hash
    memcpy(le->hdr.digest, sha1, SHA1_BUFSIZE);
    ܦܢܐ SHA1_BUFSIZE;
}

ساكن صحيح
tpm_build_digest(بنية tpm_log_entry *le, ثابت u8 *hashdata, u32 hashdata_len
                 , صحيح bigEndian)
{
    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        ܦܢܐ tpm12_build_digest(le, hashdata, hashdata_len);
    حالة TPM_VERSION_2:
        ܦܢܐ tpm20_build_digest(le, hashdata, hashdata_len, bigEndian);
    }
    ܦܢܐ -1;
}


/****************************************************************
 * TPM hardware command wrappers
 ****************************************************************/

// Helper function for sending tpm commands that take a single
// optional parameter (0, 1, or 2 bytes) and have no special response.
ساكن صحيح
tpm_simple_cmd(u8 locty, u32 ordinal
               , صحيح param_size, u16 param, تعداد tpmDurationType to_t)
{
    بنية {
        بنية tpm_req_header trqh;
        u16 param;
    } PACKED req = {
        .trqh.totlen = cpu_to_be32(حجم(req.trqh) + param_size),
        .trqh.ordinal = cpu_to_be32(ordinal),
        .param = param_size == 2 ? cpu_to_be16(param) : param,
    };
    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        req.trqh.tag = cpu_to_be16(TPM_TAG_RQU_CMD);
        اكسر;
    حالة TPM_VERSION_2:
        req.trqh.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        اكسر;
    }

    u8 obuffer[64];
    بنية tpm_rsp_header *trsh = (فراغ*)obuffer;
    u32 obuffer_len = حجم(obuffer);
    memset(obuffer, 0x0, حجم(obuffer));

    صحيح ret = tpmhw_transmit(locty, &req.trqh, obuffer, &obuffer_len, to_t);
    ret = ret ? -1 : be32_to_cpu(trsh->errcode);
    dprintf(DEBUG_tcg, "Return from tpm_simple_cmd(%x, %x) = %x\n",
            ordinal, param, ret);
    ܦܢܐ ret;
}

ساكن صحيح
tpm20_getcapability(u32 capability, u32 property, u32 count,
                    بنية tpm_rsp_header *rsp, u32 rsize)
{
    بنية tpm2_req_getcapability trg = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(حجم(trg)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetCapability),
        .capability = cpu_to_be32(capability),
        .property = cpu_to_be32(property),
        .propertycount = cpu_to_be32(count),
    };

    u32 resp_size = rsize;
    صحيح ret = tpmhw_transmit(0, &trg.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret ||
           rsize < be32_to_cpu(rsp->totlen)) ? -1 : be32_to_cpu(rsp->errcode);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetCapability = 0x%08x\n",
            ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_get_pcrbanks(فراغ)
{
    u8 buffer[128];
    بنية tpm2_res_getcapability *trg =
      (بنية tpm2_res_getcapability *)&buffer;

    صحيح ret = tpm20_getcapability(TPM2_CAP_PCRS, 0, 8, &trg->hdr,
                                  حجم(buffer));
    ܐܢ (ret)
        ܦܢܐ ret;

    /* defend against (broken) TPM sending packets that are too short */
    u32 resplen = be32_to_cpu(trg->hdr.totlen);
    ܐܢ (resplen <= offsetof(بنية tpm2_res_getcapability, data))
        ܦܢܐ -1;

    u32 size = resplen - offsetof(بنية tpm2_res_getcapability, data);
    /* we need a valid tpml_pcr_selection up to and including sizeOfSelect */
    ܐܢ (size < offsetof(بنية tpml_pcr_selection, selections) +
               offsetof(بنية tpms_pcr_selection, pcrSelect))
        ܦܢܐ -1;

    tpm20_pcr_selection = malloc_high(size);
    ܐܢ (tpm20_pcr_selection) {
        memcpy(tpm20_pcr_selection, &trg->data, size);
        tpm20_pcr_selection_size = size;
    } ܐܚܪܢܐ {
        warn_noalloc();
        ret = -1;
    }

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_get_suppt_pcrbanks(u8 *suppt_pcrbanks, u8 *active_pcrbanks)
{
    *suppt_pcrbanks = 0;
    *active_pcrbanks = 0;

    ܐܢ (!tpm20_pcr_selection)
        ܦܢܐ -1;

    بنية tpms_pcr_selection *sel = tpm20_pcr_selection->selections;
    فراغ *end = (فراغ*)tpm20_pcr_selection + tpm20_pcr_selection_size;

    ܟܡܐ (1) {
        u8 sizeOfSelect = sel->sizeOfSelect;
        فراغ *nsel = (فراغ*)sel + حجم(*sel) + sizeOfSelect;
        ܐܢ (nsel > end)
            ܦܢܐ 0;

        u16 hashalg = be16_to_cpu(sel->hashAlg);
        u8 hashalg_flag = tpm20_hashalg_to_flag(hashalg);

        *suppt_pcrbanks |= hashalg_flag;

        unsigned i;
        لكل (i = 0; i < sizeOfSelect; i++) {
            ܐܢ (sel->pcrSelect[i]) {
                *active_pcrbanks |= hashalg_flag;
                اكسر;
            }
        }

        sel = nsel;
    }
}

ساكن صحيح
tpm20_set_pcrbanks(u32 active_banks)
{
    بنية tpm2_req_pcr_allocate trpa = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Allocate),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(حجم(trpa.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    بنية tpms_pcr_selection3 {
        u16 hashAlg;
        u8 sizeOfSelect;
        u8 pcrSelect[3];
    } tps[ARRAY_SIZE(trpa.tpms_pcr_selections)];
    صحيح i = 0;
    u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
    u8 dontcare, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &dontcare);

    ܟܡܐ (hashalg_flag) {
        ܐܢ ((hashalg_flag & suppt_banks)) {
            u16 hashalg = tpm20_hashalg_flag_to_hashalg(hashalg_flag);

            ܐܢ (hashalg) {
                u8 mask = 0;
                tps[i].hashAlg = cpu_to_be16(hashalg);
                tps[i].sizeOfSelect = 3;

                ܐܢ (active_banks & hashalg_flag)
                    mask = 0xff;

                tps[i].pcrSelect[0] = mask;
                tps[i].pcrSelect[1] = mask;
                tps[i].pcrSelect[2] = mask;
                i++;
            }
        }
        hashalg_flag <<= 1;
    }

    trpa.count = cpu_to_be32(i);
    memcpy(trpa.tpms_pcr_selections, tps, i * حجم(tps[0]));
    trpa.hdr.totlen = cpu_to_be32(offsetof(بنية tpm2_req_pcr_allocate,
                                           tpms_pcr_selections) +
                                  i * حجم(tps[0]));

    بنية tpm_rsp_header rsp;
    u32 resp_length = حجم(rsp);

    صحيح ret = tpmhw_transmit(0, &trpa.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ret = ret ? -1 : be32_to_cpu(rsp.errcode);

    ܦܢܐ ret;
}

ساكن صحيح tpm20_activate_pcrbanks(u32 active_banks)
{
    صحيح ret = tpm20_set_pcrbanks(active_banks);
    ܐܢ (!ret)
        ret = tpm_simple_cmd(0, TPM2_CC_Shutdown,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);
    ܐܢ (!ret)
        reset();
    ܦܢܐ ret;
}

ساكن صحيح
tpm12_get_capability(u32 cap, u32 subcap, بنية tpm_rsp_header *rsp, u32 rsize)
{
    بنية tpm_req_getcap trgc = {
        .hdr.tag = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen = cpu_to_be32(حجم(trgc)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_GetCapability),
        .capArea = cpu_to_be32(cap),
        .subCapSize = cpu_to_be32(حجم(trgc.subCap)),
        .subCap = cpu_to_be32(subcap)
    };
    u32 resp_size = rsize;
    صحيح ret = tpmhw_transmit(0, &trgc.hdr, rsp, &resp_size,
                             TPM_DURATION_TYPE_SHORT);
    ret = (ret || resp_size != rsize) ? -1 : be32_to_cpu(rsp->errcode);
    dprintf(DEBUG_tcg, "TCGBIOS: Return code from TPM_GetCapability(%d, %d)"
            " = %x\n", cap, subcap, ret);
    ܦܢܐ ret;
}

ساكن صحيح
tpm12_read_permanent_flags(حرف *buf, صحيح buf_len)
{
    memset(buf, 0, buf_len);

    بنية tpm_res_getcap_perm_flags pf;
    صحيح ret = tpm12_get_capability(TPM_CAP_FLAG, TPM_CAP_FLAG_PERMANENT
                                   , &pf.hdr, حجم(pf));
    ܐܢ (ret)
        ܦܢܐ -1;

    memcpy(buf, &pf.perm_flags, buf_len);

    ܦܢܐ 0;
}

ساكن صحيح
tpm12_determine_timeouts(فراغ)
{
    بنية tpm_res_getcap_timeouts timeouts;
    صحيح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_TIS_TIMEOUT
                                   , &timeouts.hdr, حجم(timeouts));
    ܐܢ (ret)
        ܦܢܐ ret;

    بنية tpm_res_getcap_durations durations;
    ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_DURATION
                               , &durations.hdr, حجم(durations));
    ܐܢ (ret)
        ܦܢܐ ret;

    صحيح i;
    لكل (i = 0; i < 3; i++)
        durations.durations[i] = be32_to_cpu(durations.durations[i]);

    لكل (i = 0; i < 4; i++)
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

    ܦܢܐ 0;
}

ساكن فراغ
tpm20_set_timeouts(فراغ)
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

ساكن صحيح
tpm12_extend(بنية tpm_log_entry *le, صحيح digest_len)
{
    بنية tpm_req_extend tre = {
        .hdr.tag     = cpu_to_be16(TPM_TAG_RQU_CMD),
        .hdr.totlen  = cpu_to_be32(حجم(tre)),
        .hdr.ordinal = cpu_to_be32(TPM_ORD_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
    };
    memcpy(tre.digest, le->hdr.digest, حجم(tre.digest));

    بنية tpm_rsp_extend rsp;
    u32 resp_length = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &tre.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.hdr.errcode)
        ܦܢܐ -1;

    ܦܢܐ 0;
}

ساكن صحيح tpm20_extend(بنية tpm_log_entry *le, صحيح digest_len)
{
    بنية tpm2_req_extend tmp_tre = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(0),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_PCR_Extend),
        .pcrindex    = cpu_to_be32(le->hdr.pcrindex),
        .authblocksize = cpu_to_be32(حجم(tmp_tre.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    u8 buffer[حجم(tmp_tre) + حجم(le->pad)];
    بنية tpm2_req_extend *tre = (بنية tpm2_req_extend *)buffer;

    memcpy(tre, &tmp_tre, حجم(tmp_tre));
    memcpy(&tre->digest[0], le->hdr.digest, digest_len);

    tre->hdr.totlen = cpu_to_be32(حجم(tmp_tre) + digest_len);

    بنية tpm_rsp_header rsp;
    u32 resp_length = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &tre->hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.errcode)
        ܦܢܐ -1;

    ܦܢܐ 0;
}

ساكن صحيح
tpm_extend(بنية tpm_log_entry *le, صحيح digest_len)
{
    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        ܦܢܐ tpm12_extend(le, digest_len);
    حالة TPM_VERSION_2:
        ܦܢܐ tpm20_extend(le, digest_len);
    }
    ܦܢܐ -1;
}

ساكن صحيح
tpm20_stirrandom(فراغ)
{
    بنية tpm2_req_stirrandom stir = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(حجم(stir)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_StirRandom),
        .size = cpu_to_be16(حجم(stir.stir)),
        .stir = rdtscll(),
    };
    /* set more bits to stir with */
    stir.stir += swab64(rdtscll());

    بنية tpm_rsp_header rsp;
    u32 resp_length = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &stir.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_StirRandom = 0x%08x\n",
            ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_getrandom(u8 *buf, u16 buf_len)
{
    بنية tpm2_res_getrandom rsp;

    ܐܢ (buf_len > حجم(rsp.rnd.buffer))
        ܦܢܐ -1;

    بنية tpm2_req_getrandom trgr = {
        .hdr.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
        .hdr.totlen = cpu_to_be32(حجم(trgr)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_GetRandom),
        .bytesRequested = cpu_to_be16(buf_len),
    };
    u32 resp_length = حجم(rsp);

    صحيح ret = tpmhw_transmit(0, &trgr.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.hdr.errcode)
        ret = -1;
    ܐܚܪܢܐ
        memcpy(buf, rsp.rnd.buffer, buf_len);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_GetRandom = 0x%08x\n",
            ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_hierarchycontrol(u32 hierarchy, u8 state)
{
    /* we will try to deactivate the TPM now - ignoring all errors */
    بنية tpm2_req_hierarchycontrol trh = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(حجم(trh)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(حجم(trh.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .enable = cpu_to_be32(hierarchy),
        .state = state,
    };
    بنية tpm_rsp_header rsp;
    u32 resp_length = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trh.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyControl = 0x%08x\n",
            ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_hierarchychangeauth(u8 auth[20])
{
    بنية tpm2_req_hierarchychangeauth trhca = {
        .hdr.tag = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen = cpu_to_be32(حجم(trhca)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_HierarchyChangeAuth),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(حجم(trhca.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .newAuth = {
            .size = cpu_to_be16(حجم(trhca.newAuth.buffer)),
        },
    };
    memcpy(trhca.newAuth.buffer, auth, حجم(trhca.newAuth.buffer));

    بنية tpm_rsp_header rsp;
    u32 resp_length = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trhca.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_HierarchyChangeAuth = 0x%08x\n",
            ret);

    ܦܢܐ ret;
}


/****************************************************************
 * Setup and Measurements
 ****************************************************************/

ساكن صحيح TPM_has_physical_presence;
u8 TPM_working VARLOW;

ساكن صحيح
tpm_is_working(فراغ)
{
    ܦܢܐ CONFIG_TCGBIOS && TPM_working;
}

ساكن فراغ
tpm_set_failure(فراغ)
{
    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        /*
         * We will try to deactivate the TPM now - ignoring all errors
         * Physical presence is asserted.
         */

        tpm_simple_cmd(0, TPM_ORD_SetTempDeactivated,
                       0, 0, TPM_DURATION_TYPE_SHORT);
        اكسر;
    حالة TPM_VERSION_2:
        tpm20_hierarchycontrol(TPM2_RH_ENDORSEMENT, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_OWNER, TPM2_NO);
        tpm20_hierarchycontrol(TPM2_RH_PLATFORM, TPM2_NO);
        اكسر;
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
ساكن فراغ
tpm_add_measurement_to_log(u32 pcrindex, u32 event_type,
                           ثابت حرف *event, u32 event_length,
                           ثابت u8 *hashdata, u32 hashdata_length)
{
    ܐܢ (!tpm_is_working())
        ܦܢܐ;

    بنية tpm_log_entry le = {
        .hdr.pcrindex = pcrindex,
        .hdr.eventtype = event_type,
    };
    صحيح digest_len = tpm_build_digest(&le, hashdata, hashdata_length, 1);
    ܐܢ (digest_len < 0)
        ܦܢܐ;
    صحيح ret = tpm_extend(&le, digest_len);
    ܐܢ (ret) {
        tpm_set_failure();
        ܦܢܐ;
    }
    tpm_build_digest(&le, hashdata, hashdata_length, 0);
    tpm_log_event(&le.hdr, digest_len, event, event_length);
}

// Add an EV_ACTION measurement to the list of measurements
ساكن فراغ
tpm_add_action(u32 pcrIndex, ثابت حرف *string)
{
    u32 len = strlen(string);
    tpm_add_measurement_to_log(pcrIndex, EV_ACTION,
                               string, len, (u8 *)string, len);
}

/*
 * Add event separators for PCRs 0 to 7; specs on 'Measuring Boot Events'
 */
ساكن فراغ
tpm_add_event_separators(فراغ)
{
    ساكن ثابت u8 evt_separator[] = {0xff,0xff,0xff,0xff};
    u32 pcrIndex;
    لكل (pcrIndex = 0; pcrIndex <= 7; pcrIndex++)
        tpm_add_measurement_to_log(pcrIndex, EV_SEPARATOR,
                                   (ثابت حرف *)evt_separator,
                                   حجم(evt_separator),
                                   evt_separator,
                                   حجم(evt_separator));
}

ساكن فراغ
tpm_smbios_measure(فراغ)
{
    بنية pcctes pcctes = {
        .eventid = 1,
        .eventdatasize = SHA1_BUFSIZE,
    };
    u32 smbios_len;
    فراغ *smbios_tables = smbios_get_tables(&smbios_len);

    dprintf(DEBUG_tcg, "TCGBIOS: SMBIOS tables at %p\n", smbios_tables);

    ܐܢ (!smbios_tables)
        ܦܢܐ;

    sha1((ثابت u8 *)smbios_tables, smbios_len, pcctes.digest);
    tpm_add_measurement_to_log(1,
                               EV_EVENT_TAG,
                               (ثابت حرف *)&pcctes, حجم(pcctes),
                               (u8 *)&pcctes, حجم(pcctes));
}

ساكن صحيح
tpm12_assert_physical_presence(فراغ)
{
    صحيح ret = tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                             2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    ܐܢ (!ret)
        ܦܢܐ 0;

    بنية tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    ܐܢ (ret)
        ܦܢܐ -1;

    /* check if hardware physical presence is supported */
    ܐܢ (pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE]) {
        /* HW phys. presence may not be asserted... */
        ܦܢܐ 0;
    }

    ܐܢ (!pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_LOCK]
        && !pf.flags[PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE]) {
        tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                       2, TPM_PP_CMD_ENABLE, TPM_DURATION_TYPE_SHORT);

        ܦܢܐ tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                              2, TPM_PP_PRESENT, TPM_DURATION_TYPE_SHORT);
    }
    ܦܢܐ -1;
}

ساكن صحيح
tpm12_startup(فراغ)
{
    dprintf(DEBUG_tcg, "TCGBIOS: Starting with TPM_Startup(ST_CLEAR)\n");
    صحيح ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_CLEAR, TPM_DURATION_TYPE_SHORT);
    ܐܢ (CONFIG_COREBOOT && ret == TPM_INVALID_POSTINIT)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;
    ܐܢ (ret)
        اذهب err_exit;

    /* assertion of physical presence is only possible after startup */
    ret = tpm12_assert_physical_presence();
    ܐܢ (!ret)
        TPM_has_physical_presence = 1;

    ret = tpm12_determine_timeouts();
    ܐܢ (ret)
        اذهب err_exit;

    ret = tpm_simple_cmd(0, TPM_ORD_SelfTestFull,
                         0, 0, TPM_DURATION_TYPE_LONG);
    ܐܢ (ret)
        اذهب err_exit;

    ret = tpm_simple_cmd(3, TSC_ORD_ResetEstablishmentBit,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret && ret != TPM_BAD_LOCALITY)
        اذهب err_exit;

    ܦܢܐ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ܦܢܐ -1;
}

ساكن صحيح
tpm20_startup(فراغ)
{
    tpm20_set_timeouts();

    صحيح ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_CLEAR, TPM_DURATION_TYPE_SHORT);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_CLEAR) = 0x%08x\n",
            ret);

    ܐܢ (CONFIG_COREBOOT && ret == TPM2_RC_INITIALIZE)
        /* with other firmware on the system the TPM may already have been
         * initialized
         */
        ret = 0;

    ܐܢ (ret)
        اذهب err_exit;

    ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                         1, TPM2_YES, TPM_DURATION_TYPE_LONG);

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest = 0x%08x\n",
            ret);

    ܐܢ (ret)
        اذهب err_exit;

    ret = tpm20_get_pcrbanks();
    ܐܢ (ret)
        اذهب err_exit;

    ret = tpm20_write_EfiSpecIdEventStruct();
    ܐܢ (ret)
        اذهب err_exit;

    ܦܢܐ 0;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
    ܦܢܐ -1;
}

ساكن صحيح
tpm_startup(فراغ)
{
    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        ܦܢܐ tpm12_startup();
    حالة TPM_VERSION_2:
        ܦܢܐ tpm20_startup();
    }
    ܦܢܐ -1;
}

فراغ
tpm_setup(فراغ)
{
    ܐܢ (!CONFIG_TCGBIOS)
        ܦܢܐ;

    صحيح ret = tpm_tpm2_probe();
    ܐܢ (ret) {
        ret = tpm_tcpa_probe();
        ܐܢ (ret)
            ܦܢܐ;
    }

    TPM_version = tpmhw_probe();
    ܐܢ (TPM_version == TPM_VERSION_NONE)
        ܦܢܐ;

    dprintf(DEBUG_tcg,
            "TCGBIOS: Detected a TPM %s.\n",
             (TPM_version == TPM_VERSION_1_2) ? "1.2" : "2");

    TPM_working = 1;

    ܐܢ (runningOnXen())
        ܦܢܐ;

    ret = tpm_startup();
    ܐܢ (ret)
        ܦܢܐ;

    tpm_smbios_measure();
    tpm_add_action(2, "Start Option ROM Scan");
}

ساكن فراغ
tpm20_prepboot(فراغ)
{
    صحيح ret = tpm20_stirrandom();
    ܐܢ (ret)
         اذهب err_exit;

    u8 auth[20];
    ret = tpm20_getrandom(&auth[0], حجم(auth));
    ܐܢ (ret)
        اذهب err_exit;

    ret = tpm20_hierarchychangeauth(auth);
    ܐܢ (ret)
        اذهب err_exit;

    ܦܢܐ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}

فراغ
tpm_prepboot(فراغ)
{
    ܐܢ (!CONFIG_TCGBIOS)
        ܦܢܐ;

    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        ܐܢ (TPM_has_physical_presence)
            tpm_simple_cmd(0, TPM_ORD_PhysicalPresence,
                           2, TPM_PP_NOT_PRESENT_LOCK, TPM_DURATION_TYPE_SHORT);
        اكسر;
    حالة TPM_VERSION_2:
        tpm20_prepboot();
        اكسر;
    }

    tpm_add_action(4, "Calling INT 19h");
    tpm_add_event_separators();
}

/*
 * Add measurement to the log about an option rom
 */
فراغ
tpm_option_rom(ثابت فراغ *addr, u32 len)
{
    ܐܢ (!tpm_is_working())
        ܦܢܐ;

    بنية pcctes_romex pcctes = {
        .eventid = 7,
        .eventdatasize = حجم(u16) + حجم(u16) + SHA1_BUFSIZE,
    };
    sha1((ثابت u8 *)addr, len, pcctes.digest);
    tpm_add_measurement_to_log(2,
                               EV_EVENT_TAG,
                               (ثابت حرف *)&pcctes, حجم(pcctes),
                               (u8 *)&pcctes, حجم(pcctes));
}

فراغ
tpm_add_bcv(u32 bootdrv, ثابت u8 *addr, u32 length)
{
    ܐܢ (!tpm_is_working())
        ܦܢܐ;

    ܐܢ (length < 0x200)
        ܦܢܐ;

    ثابت حرف *string = "Booting BCV device 00h (Floppy)";
    ܐܢ (bootdrv == 0x80)
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

فراغ
tpm_add_cdrom(u32 bootdrv, ثابت u8 *addr, u32 length)
{
    ܐܢ (!tpm_is_working())
        ܦܢܐ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت حرف *string = "EL TORITO IPL";
    tpm_add_measurement_to_log(4, EV_IPL,
                               string, strlen(string),
                               addr, length);
}

فراغ
tpm_add_cdrom_catalog(ثابت u8 *addr, u32 length)
{
    ܐܢ (!tpm_is_working())
        ܦܢܐ;

    tpm_add_action(4, "Booting from CD ROM device");

    /* specs: see section 'El Torito' */
    ثابت حرف *string = "BOOT CATALOG";
    tpm_add_measurement_to_log(5, EV_IPL_PARTITION_DATA,
                               string, strlen(string),
                               addr, length);
}

فراغ
tpm_s3_resume(فراغ)
{
    ܐܢ (!tpm_is_working())
        ܦܢܐ;

    dprintf(DEBUG_tcg, "TCGBIOS: Resuming with TPM_Startup(ST_STATE)\n");

    صحيح ret = -1;

    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        ret = tpm_simple_cmd(0, TPM_ORD_Startup,
                             2, TPM_ST_STATE, TPM_DURATION_TYPE_SHORT);
        اكسر;
    حالة TPM_VERSION_2:
        ret = tpm_simple_cmd(0, TPM2_CC_Startup,
                             2, TPM2_SU_STATE, TPM_DURATION_TYPE_SHORT);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Startup(SU_STATE) = 0x%08x\n",
                ret);

        ܐܢ (ret)
            اذهب err_exit;


        ret = tpm_simple_cmd(0, TPM2_CC_SelfTest,
                             1, TPM2_YES, TPM_DURATION_TYPE_LONG);

        dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_SelfTest() = 0x%08x\n",
                ret);

        اكسر;
    }

    ܐܢ (ret)
        اذهب err_exit;

    ܦܢܐ;

err_exit:
    dprintf(DEBUG_tcg, "TCGBIOS: TPM malfunctioning (line %d).\n", __LINE__);

    tpm_set_failure();
}


/****************************************************************
 * BIOS interface
 ****************************************************************/

u8 TPM_interface_shutdown VARLOW;

ساكن مضمن فراغ *input_buf32(بنية bregs *regs)
{
    ܦܢܐ MAKE_FLATPTR(regs->es, regs->di);
}

ساكن مضمن فراغ *output_buf32(بنية bregs *regs)
{
    ܦܢܐ MAKE_FLATPTR(regs->ds, regs->si);
}

ساكن u32
hash_log_extend(بنية pcpes *pcpes, ثابت فراغ *hashdata, u32 hashdata_length
                , فراغ *event, صحيح extend)
{
    ܐܢ (pcpes->pcrindex >= 24)
        ܦܢܐ TCG_INVALID_INPUT_PARA;
    ܐܢ (hashdata)
        sha1(hashdata, hashdata_length, pcpes->digest);

    بنية tpm_log_entry le = {
        .hdr.pcrindex = pcpes->pcrindex,
        .hdr.eventtype = pcpes->eventtype,
    };
    صحيح digest_len = tpm12_build_digest_direct(&le, pcpes->digest);
    ܐܢ (digest_len < 0)
        ܦܢܐ TCG_GENERAL_ERROR;
    ܐܢ (extend) {
        صحيح ret = tpm_extend(&le, digest_len);
        ܐܢ (ret)
            ܦܢܐ TCG_TCG_COMMAND_ERROR;
    }
    tpm12_build_digest_direct(&le, pcpes->digest);
    صحيح ret = tpm_log_event(&le.hdr, digest_len
                            , pcpes->event, pcpes->eventdatasize);
    ܐܢ (ret)
        ܦܢܐ TCG_PC_LOGOVERFLOW;
    ܦܢܐ 0;
}

ساكن u32
hash_log_extend_event_int(ثابت بنية hleei_short *hleei_s,
                          بنية hleeo *hleeo)
{
    u32 rc = 0;
    بنية hleei_long *hleei_l = (بنية hleei_long *)hleei_s;
    ثابت فراغ *logdataptr;
    u32 logdatalen;
    بنية pcpes *pcpes;
    u32 pcrindex;

    /* short or long version? */
    بدل (hleei_s->ipblength) {
    حالة حجم(بنية hleei_short):
        /* short */
        logdataptr = hleei_s->logdataptr;
        logdatalen = hleei_s->logdatalen;
        pcrindex = hleei_s->pcrindex;
    اكسر;

    حالة حجم(بنية hleei_long):
        /* long */
        logdataptr = hleei_l->logdataptr;
        logdatalen = hleei_l->logdatalen;
        pcrindex = hleei_l->pcrindex;
    اكسر;

    افتراضي:
        /* bad input block */
        rc = TCG_INVALID_INPUT_PARA;
        اذهب err_exit;
    }

    pcpes = (بنية pcpes *)logdataptr;

    ܐܢ (pcpes->pcrindex != pcrindex
        || logdatalen != حجم(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_INPUT_PARA;
        اذهب err_exit;
    }
    rc = hash_log_extend(pcpes, hleei_s->hashdataptr, hleei_s->hashdatalen
                         , pcpes->event, 1);
    ܐܢ (rc)
        اذهب err_exit;

    hleeo->opblength = حجم(بنية hleeo);
    hleeo->reserved  = 0;
    hleeo->eventnumber = tpm_state.entry_count;
    memcpy(hleeo->digest, pcpes->digest, حجم(hleeo->digest));

err_exit:
    ܐܢ (rc != 0) {
        hleeo->opblength = 4;
        hleeo->reserved  = 0;
    }

    ܦܢܐ rc;
}

ساكن u32
pass_through_to_tpm_int(بنية pttti *pttti, بنية pttto *pttto)
{
    u32 rc = 0;
    بنية tpm_req_header *trh = (فراغ*)pttti->tpmopin;

    ܐܢ (pttti->ipblength < حجم(بنية pttti) + حجم(*trh)
        || pttti->ipblength != حجم(بنية pttti) + be32_to_cpu(trh->totlen)
        || pttti->opblength < حجم(بنية pttto)) {
        rc = TCG_INVALID_INPUT_PARA;
        اذهب err_exit;
    }

    u16 tag = be16_to_cpu(trh->tag);

    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        ܐܢ (tag != TPM_TAG_RQU_CMD && tag != TPM_TAG_RQU_AUTH1_CMD
            && tag != TPM_TAG_RQU_AUTH2_CMD) {
            rc = TCG_INVALID_INPUT_PARA;
            اذهب err_exit;
        }
        اكسر;
    حالة TPM_VERSION_2:
        ܐܢ (tag != TPM2_ST_NO_SESSIONS && tag != TPM2_ST_SESSIONS) {
            rc = TCG_INVALID_INPUT_PARA;
            اذهب err_exit;
        }
    }

    u32 resbuflen = pttti->opblength - offsetof(بنية pttto, tpmopout);
    صحيح ret = tpmhw_transmit(0, trh, pttto->tpmopout, &resbuflen,
                             TPM_DURATION_TYPE_LONG /* worst case */);
    ܐܢ (ret) {
        rc = TCG_FATAL_COM_ERROR;
        اذهب err_exit;
    }

    pttto->opblength = offsetof(بنية pttto, tpmopout) + resbuflen;
    pttto->reserved  = 0;

err_exit:
    ܐܢ (rc != 0) {
        pttto->opblength = 4;
        pttto->reserved = 0;
    }

    ܦܢܐ rc;
}

ساكن u32
shutdown_preboot_interface(فراغ)
{
    TPM_interface_shutdown = 1;
    ܦܢܐ 0;
}

ساكن u32
hash_log_event_int(ثابت بنية hlei *hlei, بنية hleo *hleo)
{
    u32 rc = 0;
    u16 size;
    بنية pcpes *pcpes;

    size = hlei->ipblength;
    ܐܢ (size != حجم(*hlei)) {
        rc = TCG_INVALID_INPUT_PARA;
        اذهب err_exit;
    }

    pcpes = (بنية pcpes *)hlei->logdataptr;

    ܐܢ (pcpes->pcrindex != hlei->pcrindex
        || pcpes->eventtype != hlei->logeventtype
        || hlei->logdatalen != حجم(*pcpes) + pcpes->eventdatasize) {
        rc = TCG_INVALID_INPUT_PARA;
        اذهب err_exit;
    }
    rc = hash_log_extend(pcpes, hlei->hashdataptr, hlei->hashdatalen
                         , pcpes->event, 0);
    ܐܢ (rc)
        اذهب err_exit;

    /* updating the log was fine */
    hleo->opblength = حجم(بنية hleo);
    hleo->reserved  = 0;
    hleo->eventnumber = tpm_state.entry_count;

err_exit:
    ܐܢ (rc != 0) {
        hleo->opblength = 2;
        hleo->reserved = 0;
    }

    ܦܢܐ rc;
}

ساكن u32
hash_all_int(ثابت بنية hai *hai, u8 *hash)
{
    ܐܢ (hai->ipblength != حجم(بنية hai) ||
        hai->hashdataptr == 0 ||
        hai->hashdatalen == 0 ||
        hai->algorithmid != TPM_ALG_SHA)
        ܦܢܐ TCG_INVALID_INPUT_PARA;

    sha1((ثابت u8 *)hai->hashdataptr, hai->hashdatalen, hash);
    ܦܢܐ 0;
}

ساكن u32
tss_int(بنية ti *ti, بنية to *to)
{
    to->opblength = حجم(بنية to);
    to->reserved  = 0;

    ܦܢܐ TCG_PC_UNSUPPORTED;
}

ساكن u32
compact_hash_log_extend_event_int(u8 *buffer,
                                  u32 info,
                                  u32 length,
                                  u32 pcrindex,
                                  u32 *edx_ptr)
{
    بنية pcpes pcpes = {
        .pcrindex      = pcrindex,
        .eventtype     = EV_COMPACT_HASH,
        .eventdatasize = حجم(info),
    };
    u32 rc = hash_log_extend(&pcpes, buffer, length, &info, 1);
    ܐܢ (rc)
        ܦܢܐ rc;

    *edx_ptr = tpm_state.entry_count;
    ܦܢܐ 0;
}

فراغ VISIBLE32FLAT
tpm_interrupt_handler32(بنية bregs *regs)
{
    ܐܢ (!CONFIG_TCGBIOS)
        ܦܢܐ;

    set_cf(regs, 0);

    ܐܢ (TPM_interface_shutdown && regs->al) {
        regs->eax = TCG_INTERFACE_SHUTDOWN;
        ܦܢܐ;
    }

    بدل ((تعداد irq_ids)regs->al) {
    حالة TCG_StatusCheck:
        ܐܢ (!tpmhw_is_present()) {
            /* no TPM available */
            regs->eax = TCG_PC_TPM_NOT_PRESENT;
        } ܐܚܪܢܐ {
            regs->eax = 0;
            regs->ebx = TCG_MAGIC;
            regs->ch = TCG_VERSION_MAJOR;
            regs->cl = TCG_VERSION_MINOR;
            regs->edx = 0x0;
            regs->esi = (u32)tpm_state.log_area_start_address;
            regs->edi = (u32)tpm_state.log_area_last_entry;
        }
        اكسر;

    حالة TCG_HashLogExtendEvent:
        regs->eax =
            hash_log_extend_event_int(
                  (بنية hleei_short *)input_buf32(regs),
                  (بنية hleeo *)output_buf32(regs));
        اكسر;

    حالة TCG_PassThroughToTPM:
        regs->eax =
            pass_through_to_tpm_int((بنية pttti *)input_buf32(regs),
                                    (بنية pttto *)output_buf32(regs));
        اكسر;

    حالة TCG_ShutdownPreBootInterface:
        regs->eax = shutdown_preboot_interface();
        اكسر;

    حالة TCG_HashLogEvent:
        regs->eax = hash_log_event_int((بنية hlei*)input_buf32(regs),
                                       (بنية hleo*)output_buf32(regs));
        اكسر;

    حالة TCG_HashAll:
        regs->eax =
            hash_all_int((بنية hai*)input_buf32(regs),
                          (u8 *)output_buf32(regs));
        اكسر;

    حالة TCG_TSS:
        regs->eax = tss_int((بنية ti*)input_buf32(regs),
                            (بنية to*)output_buf32(regs));
        اكسر;

    حالة TCG_CompactHashLogExtendEvent:
        regs->eax =
          compact_hash_log_extend_event_int((u8 *)input_buf32(regs),
                                            regs->esi,
                                            regs->ecx,
                                            regs->edx,
                                            &regs->edx);
        اكسر;

    افتراضي:
        set_cf(regs, 1);
    }

    ܦܢܐ;
}


/****************************************************************
 * TPM Configuration Menu
 ****************************************************************/

تعريفنوع u8 tpm_ppi_code;

ساكن صحيح
tpm12_read_has_owner(صحيح *has_owner)
{
    بنية tpm_res_getcap_ownerauth oauth;
    صحيح ret = tpm12_get_capability(TPM_CAP_PROPERTY, TPM_CAP_PROP_OWNER
                                   , &oauth.hdr, حجم(oauth));
    ܐܢ (ret)
        ܦܢܐ -1;

    *has_owner = oauth.flag;

    ܦܢܐ 0;
}

ساكن صحيح
tpm12_enable_tpm(صحيح enable, صحيح verbose)
{
    بنية tpm_permanent_flags pf;
    صحيح ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    ܐܢ (ret)
        ܦܢܐ -1;

    ܐܢ (pf.flags[PERM_FLAG_IDX_DISABLE] && !enable)
        ܦܢܐ 0;

    ret = tpm_simple_cmd(0, enable ? TPM_ORD_PhysicalEnable
                                   : TPM_ORD_PhysicalDisable,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret) {
        ܐܢ (enable)
            dprintf(DEBUG_tcg, "TCGBIOS: Enabling the TPM failed.\n");
        ܐܚܪܢܐ
            dprintf(DEBUG_tcg, "TCGBIOS: Disabling the TPM failed.\n");
    }
    ܦܢܐ ret;
}

ساكن صحيح
tpm12_activate_tpm(صحيح activate, صحيح allow_reset, صحيح verbose)
{
    بنية tpm_permanent_flags pf;
    صحيح ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    ܐܢ (ret)
        ܦܢܐ -1;

    ܐܢ (pf.flags[PERM_FLAG_IDX_DEACTIVATED] && !activate)
        ܦܢܐ 0;

    ܐܢ (pf.flags[PERM_FLAG_IDX_DISABLE])
        ܦܢܐ 0;

    ret = tpm_simple_cmd(0, TPM_ORD_PhysicalSetDeactivated,
                         1, activate ? 0x00 : 0x01, TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret)
        ܦܢܐ ret;

    ܐܢ (activate && allow_reset) {
        ܐܢ (verbose) {
            printf("Requiring a reboot to activate the TPM.\n");

            msleep(2000);
        }
        reset();
    }

    ܦܢܐ 0;
}

ساكن صحيح
tpm12_enable_activate(صحيح allow_reset, صحيح verbose)
{
    صحيح ret = tpm12_enable_tpm(1, verbose);
    ܐܢ (ret)
        ܦܢܐ ret;

    ܦܢܐ tpm12_activate_tpm(1, allow_reset, verbose);
}

ساكن صحيح
tpm12_force_clear(صحيح enable_activate_before, صحيح enable_activate_after,
                  صحيح verbose)
{
    صحيح has_owner;
    صحيح ret = tpm12_read_has_owner(&has_owner);
    ܐܢ (ret)
        ܦܢܐ -1;
    ܐܢ (!has_owner) {
        ܐܢ (verbose)
            printf("TPM does not have an owner.\n");
        ܦܢܐ 0;
    }

    ܐܢ (enable_activate_before) {
        ret = tpm12_enable_activate(0, verbose);
        ܐܢ (ret) {
            dprintf(DEBUG_tcg,
                    "TCGBIOS: Enabling/activating the TPM failed.\n");
            ܦܢܐ ret;
        }
    }

    ret = tpm_simple_cmd(0, TPM_ORD_ForceClear,
                         0, 0, TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret)
        ܦܢܐ ret;

    ܐܢ (!enable_activate_after) {
        ܐܢ (verbose)
            printf("Owner successfully cleared.\n"
                   "You will need to enable/activate the TPM again.\n\n");
        ܦܢܐ 0;
    }

    ܦܢܐ tpm12_enable_activate(1, verbose);
}

ساكن صحيح
tpm12_set_owner_install(صحيح allow, صحيح verbose)
{
    صحيح has_owner;
    صحيح ret = tpm12_read_has_owner(&has_owner);
    ܐܢ (ret)
        ܦܢܐ -1;
    ܐܢ (has_owner) {
        ܐܢ (verbose)
            printf("Must first remove owner.\n");
        ܦܢܐ 0;
    }

    بنية tpm_permanent_flags pf;
    ret = tpm12_read_permanent_flags((حرف *)&pf, حجم(pf));
    ܐܢ (ret)
        ܦܢܐ -1;

    ܐܢ (pf.flags[PERM_FLAG_IDX_DISABLE]) {
        ܐܢ (verbose)
            printf("TPM must first be enable.\n");
        ܦܢܐ 0;
    }

    ret = tpm_simple_cmd(0, TPM_ORD_SetOwnerInstall,
                         1, allow ? 0x01 : 0x00, TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret)
        ܦܢܐ ret;

    ܐܢ (verbose)
        printf("Installation of owner %s.\n", allow ? "enabled" : "disabled");

    ܦܢܐ 0;
}

ساكن صحيح
tpm12_process_cfg(tpm_ppi_code msgCode, صحيح verbose)
{
    صحيح ret = 0;

    بدل (msgCode) {
        حالة TPM_PPI_OP_NOOP: /* no-op */
            اكسر;

        حالة TPM_PPI_OP_ENABLE:
            ret = tpm12_enable_tpm(1, verbose);
            اكسر;

        حالة TPM_PPI_OP_DISABLE:
            ret = tpm12_enable_tpm(0, verbose);
            اكسر;

        حالة TPM_PPI_OP_ACTIVATE:
            ret = tpm12_activate_tpm(1, 1, verbose);
            اكسر;

        حالة TPM_PPI_OP_DEACTIVATE:
            ret = tpm12_activate_tpm(0, 1, verbose);
            اكسر;

        حالة TPM_PPI_OP_CLEAR:
            ret = tpm12_force_clear(1, 0, verbose);
            اكسر;

        حالة TPM_PPI_OP_SET_OWNERINSTALL_TRUE:
            ret = tpm12_set_owner_install(1, verbose);
            اكسر;

        حالة TPM_PPI_OP_SET_OWNERINSTALL_FALSE:
            ret = tpm12_set_owner_install(0, verbose);
            اكسر;

        افتراضي:
            اكسر;
    }

    ܐܢ (ret)
        printf("Op %d: An error occurred: 0x%x\n", msgCode, ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_clearcontrol(u8 disable, صحيح verbose)
{
    بنية tpm2_req_clearcontrol trc = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(حجم(trc)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_ClearControl),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(حجم(trc.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
        .disable = disable,
    };
    بنية tpm_rsp_header rsp;
    u32 resp_length = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trc.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_SHORT);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_ClearControl = 0x%08x\n",
            ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_clear(فراغ)
{
    بنية tpm2_req_clear trq = {
        .hdr.tag     = cpu_to_be16(TPM2_ST_SESSIONS),
        .hdr.totlen  = cpu_to_be32(حجم(trq)),
        .hdr.ordinal = cpu_to_be32(TPM2_CC_Clear),
        .authhandle = cpu_to_be32(TPM2_RH_PLATFORM),
        .authblocksize = cpu_to_be32(حجم(trq.authblock)),
        .authblock = {
            .handle = cpu_to_be32(TPM2_RS_PW),
            .noncesize = cpu_to_be16(0),
            .contsession = TPM2_YES,
            .pwdsize = cpu_to_be16(0),
        },
    };
    بنية tpm_rsp_header rsp;
    u32 resp_length = حجم(rsp);
    صحيح ret = tpmhw_transmit(0, &trq.hdr, &rsp, &resp_length,
                             TPM_DURATION_TYPE_MEDIUM);
    ܐܢ (ret || resp_length != حجم(rsp) || rsp.errcode)
        ret = -1;

    dprintf(DEBUG_tcg, "TCGBIOS: Return value from sending TPM2_CC_Clear = 0x%08x\n",
            ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm20_process_cfg(tpm_ppi_code msgCode, صحيح verbose)
{
    صحيح ret = 0;

    بدل (msgCode) {
        حالة TPM_PPI_OP_NOOP: /* no-op */
            اكسر;

        حالة TPM_PPI_OP_CLEAR:
            ret = tpm20_clearcontrol(0, verbose);
            ܐܢ (!ret)
                 ret = tpm20_clear();
            اكسر;
    }

    ܐܢ (ret)
        printf("Op %d: An error occurred: 0x%x\n", msgCode, ret);

    ܦܢܐ ret;
}

ساكن صحيح
tpm12_get_tpm_state(فراغ)
{
    صحيح state = 0;
    بنية tpm_permanent_flags pf;
    صحيح has_owner;

    ܐܢ (tpm12_read_permanent_flags((حرف *)&pf, حجم(pf)) ||
        tpm12_read_has_owner(&has_owner))
        ܦܢܐ ~0;

    ܐܢ (!pf.flags[PERM_FLAG_IDX_DISABLE])
        state |= TPM_STATE_ENABLED;

    ܐܢ (!pf.flags[PERM_FLAG_IDX_DEACTIVATED])
        state |= TPM_STATE_ACTIVE;

    ܐܢ (has_owner) {
        state |= TPM_STATE_OWNED;
    } ܐܚܪܢܐ {
        ܐܢ (pf.flags[PERM_FLAG_IDX_OWNERSHIP])
            state |= TPM_STATE_OWNERINSTALL;
    }

    ܦܢܐ state;
}

ساكن فراغ
tpm12_show_tpm_menu(صحيح state, صحيح next_scancodes[7])
{
    صحيح i = 0;

    printf("\nThe current state of the TPM is:\n");

    ܐܢ (state & TPM_STATE_ENABLED)
        printf("  Enabled");
    ܐܚܪܢܐ
        printf("  Disabled");

    ܐܢ (state & TPM_STATE_ACTIVE)
        printf(" and active\n");
    ܐܚܪܢܐ
        printf(" and deactivated\n");

    ܐܢ (state & TPM_STATE_OWNED)
        printf("  Ownership has been taken\n");
    ܐܚܪܢܐ {
        printf("  Ownership has not been taken\n");
        ܐܢ (state & TPM_STATE_OWNERINSTALL)
            printf("  A user can take ownership of the TPM\n");
        ܐܚܪܢܐ
            printf("  Taking ownership of the TPM has been disabled\n");
    }

    ܐܢ ((state & (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) !=
        (TPM_STATE_ENABLED | TPM_STATE_ACTIVE)) {
        printf("\nNote: To make use of all functionality, the TPM must be "
               "enabled and active.\n");
    }

    printf("\nAvailable options are:\n");
    ܐܢ (state & TPM_STATE_ENABLED) {
        printf(" d. Disable the TPM\n");
        next_scancodes[i++] = 32;

        ܐܢ (state & TPM_STATE_ACTIVE) {
            printf(" v. Deactivate the TPM\n");
            next_scancodes[i++] = 47;

            ܐܢ (state & TPM_STATE_OWNERINSTALL) {
                printf(" p. Prevent installation of an owner\n");
                next_scancodes[i++] = 25;
            } ܐܚܪܢܐ {
                printf(" s. Allow installation of an owner\n");
                next_scancodes[i++] = 31;
            }
        } ܐܚܪܢܐ {
            printf(" a. Activate the TPM\n");
            next_scancodes[i++] = 30;
        }

    } ܐܚܪܢܐ {
        printf(" e. Enable the TPM\n");
        next_scancodes[i++] = 18;
    }

    ܐܢ (state & TPM_STATE_OWNED) {
        printf(" c. Clear ownership\n");
        next_scancodes[i++] = 46;
    }

    next_scancodes[i++] = 0;
}

ساكن فراغ
tpm12_menu(فراغ)
{
    صحيح scancode, next_scancodes[7];
    tpm_ppi_code msgCode;
    صحيح state = 0, i;
    صحيح waitkey;

    printf("The Trusted Platform Module (TPM) is a hardware device in "
           "this machine.\n"
           "It can help verify the integrity of system software.\n\n");

    لكل (;;) {
        ܐܢ ((state = tpm12_get_tpm_state()) != ~0) {
            tpm12_show_tpm_menu(state, next_scancodes);
        } ܐܚܪܢܐ {
            printf("TPM is not working correctly.\n");
            ܦܢܐ;
        }

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        msgCode = TPM_PPI_OP_NOOP;

        waitkey = 1;

        ܟܡܐ (waitkey) {
            ܟܡܐ ((scancode = get_keystroke(1000)) == ~0)
                ;

            بدل (scancode) {
            حالة 1:
                // ESC
                reset();
                اكسر;
            حالة 18: /* e. enable */
                msgCode = TPM_PPI_OP_ENABLE;
                اكسر;
            حالة 32: /* d. disable */
                msgCode = TPM_PPI_OP_DISABLE;
                اكسر;
            حالة 30: /* a. activate */
                msgCode = TPM_PPI_OP_ACTIVATE;
                اكسر;
            حالة 47: /* v. deactivate */
                msgCode = TPM_PPI_OP_DEACTIVATE;
                اكسر;
            حالة 46: /* c. clear owner */
                msgCode = TPM_PPI_OP_CLEAR;
                اكسر;
            حالة 25: /* p. prevent ownerinstall */
                msgCode = TPM_PPI_OP_SET_OWNERINSTALL_FALSE;
                اكسر;
            حالة 31: /* s. allow ownerinstall */
                msgCode = TPM_PPI_OP_SET_OWNERINSTALL_TRUE;
                اكسر;
            افتراضي:
                استمر;
            }

            /*
             * Using the next_scancodes array, check whether the
             * pressed key is currently a valid option.
             */
            لكل (i = 0; i < حجم(next_scancodes); i++) {
                ܐܢ (next_scancodes[i] == 0)
                    اكسر;

                ܐܢ (next_scancodes[i] == scancode) {
                    tpm12_process_cfg(msgCode, 1);
                    waitkey = 0;
                    اكسر;
                }
            }
        }
    }
}

ساكن صحيح
tpm20_menu_change_active_pcrbanks(فراغ)
{
    u8 active_banks, suppt_banks;

    tpm20_get_suppt_pcrbanks(&suppt_banks, &active_banks);

    u8 activate_banks = active_banks;

    ܟܡܐ (1) {
        u8 hashalg_flag = TPM2_ALG_SHA1_FLAG;
        u8 i = 0;

        printf("\nToggle active PCR banks by pressing number key\n\n");

        ܟܡܐ (hashalg_flag) {
            u8 flag = hashalg_flag & suppt_banks;
            ثابت حرف *hashname = tpm20_hashalg_flag_to_name(flag);

            i++;
            ܐܢ (hashname) {
                printf("  %d: %s", i, hashname);
                ܐܢ (activate_banks & hashalg_flag)
                    printf(" (enabled)");
                printf("\n");
            }

            hashalg_flag <<= 1;
        }
        printf("\n"
               "ESC: return to previous menu without changes\n");
        ܐܢ (activate_banks)
            printf("A  : activate selection\n");

        u8 flagnum;
        صحيح show = 0;
        ܟܡܐ (!show) {
            صحيح scancode = get_keystroke(1000);

            بدل (scancode) {
            حالة ~0:
                استمر;
            حالة 1: /* ESC */
                printf("\n");
                ܦܢܐ -1;
            حالة 2 ... 6: /* keys 1 .. 5 */
                flagnum = scancode - 1;
                ܐܢ (flagnum > i)
                    استمر;
                ܐܢ (suppt_banks & (1 << (flagnum - 1))) {
                    activate_banks ^= 1 << (flagnum - 1);
                    show = 1;
                }
                اكسر;
            حالة 30: /* a */
                ܐܢ (activate_banks)
                    tpm20_activate_pcrbanks(activate_banks);
            }
        }
    }
}

ساكن فراغ
tpm20_menu(فراغ)
{
    صحيح scan_code;
    tpm_ppi_code msgCode;

    لكل (;;) {
        printf("1. Clear TPM\n");
        printf("2. Change active PCR banks\n");

        printf("\nIf no change is desired or if this menu was reached by "
               "mistake, press ESC to\n"
               "reboot the machine.\n");

        msgCode = TPM_PPI_OP_NOOP;

        ܟܡܐ ((scan_code = get_keystroke(1000)) == ~0)
            ;

        بدل (scan_code) {
        حالة 1:
            // ESC
            reset();
            اكسر;
        حالة 2:
            msgCode = TPM_PPI_OP_CLEAR;
            اكسر;
        حالة 3:
            tpm20_menu_change_active_pcrbanks();
            استمر;
        افتراضي:
            استمر;
        }

        tpm20_process_cfg(msgCode, 0);
    }
}

فراغ
tpm_menu(فراغ)
{
    ܐܢ (!CONFIG_TCGBIOS)
        ܦܢܐ;

    ܟܡܐ (get_keystroke(0) >= 0)
        ;
    wait_threads();

    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        tpm12_menu();
        اكسر;
    حالة TPM_VERSION_2:
        tpm20_menu();
        اكسر;
    }
}

صحيح
tpm_can_show_menu(فراغ)
{
    بدل (TPM_version) {
    حالة TPM_VERSION_1_2:
        ܦܢܐ tpm_is_working() && TPM_has_physical_presence;
    حالة TPM_VERSION_2:
        ܦܢܐ tpm_is_working();
    }
    ܦܢܐ 0;
}
