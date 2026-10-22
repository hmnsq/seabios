// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "byteorder.h" // le32_to_cpu
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "hw/pci.h" // pci_config_writeb
#अन्तर्भाव "malloc.h" // malloc_fseg
#अन्तर्भाव "memmap.h" //चिह्नम्
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_find
#अन्तर्भाव "std/acpi.h" // struct rsdp_descriptor
#अन्तर्भाव "std/mptable.h" // MPTABLE_SIGNATURE
#अन्तर्भाव "std/pirtable.h" // struct pir_header
#अन्तर्भाव "std/smbios.h" // struct smbios_21_entry_point
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "util.h" // copy_table
#अन्तर्भाव "x86.h" // outb

संरचना pir_शीर्षकम् *PirAddr VARFSEG;

स्थिर शून्य *
copy_fseg_सारणी(ध्रुव वर्ण *name, शून्य *pos, u32 आकार)
{
    शून्य *newpos = malloc_fseg(आकार);
    यदि (!newpos) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    स्मृतिं_प्रतिलिख(newpos, pos, आकार);
    प्रत्यागम newpos;
}

शून्य
copy_pir(शून्य *pos)
{
    संरचना pir_शीर्षकम् *p = pos;
    यदि (p->signature != PIR_SIGNATURE)
        प्रत्यागम;
    यदि (PirAddr)
        प्रत्यागम;
    यदि (p->आकार < परिमाण(*p))
        प्रत्यागम;
    यदि (checksum(pos, p->आकार) != 0)
        प्रत्यागम;
    PirAddr = copy_fseg_सारणी("PIR", pos, p->आकार);
}

शून्य
copy_mptable(शून्य *pos)
{
    संरचना mptable_floating_s *p = pos;
    यदि (p->signature != MPTABLE_SIGNATURE)
        प्रत्यागम;
    यदि (!p->physaddr)
        प्रत्यागम;
    यदि (checksum(pos, परिमाण(*p)) != 0)
        प्रत्यागम;
    u32 दीर्घता = p->दीर्घता * 16;
    u16 mpclength = ((संरचना mptable_config_s *)p->physaddr)->दीर्घता;
    यदि (दीर्घता + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , दीर्घता + mpclength);
        प्रत्यागम;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    संरचना mptable_floating_s *newpos = malloc_fseg(दीर्घता + mpclength);
    यदि (!newpos) {
        warn_noalloc();
        प्रत्यागम;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    स्मृतिं_प्रतिलिख(newpos, pos, दीर्घता);
    newpos->physaddr = (u32)newpos + दीर्घता;
    newpos->checksum -= checksum(newpos, परिमाण(*newpos));
    स्मृतिं_प्रतिलिख((शून्य*)newpos + दीर्घता, (शून्य*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

स्थिर पूर्णाङ्क
get_acpi_rsdp_दीर्घता(शून्य *pos, अचिह्नित आकार)
{
    संरचना rsdp_descriptor *p = pos;
    यदि (p->signature != RSDP_SIGNATURE)
        प्रत्यागम -1;
    u32 दीर्घता = 20;
    यदि (दीर्घता > आकार)
        प्रत्यागम -1;
    यदि (checksum(pos, दीर्घता) != 0)
        प्रत्यागम -1;
    यदि (p->revision > 1) {
        दीर्घता = p->दीर्घता;
        यदि (दीर्घता > आकार)
            प्रत्यागम -1;
        यदि (checksum(pos, दीर्घता) != 0)
            प्रत्यागम -1;
    }
    प्रत्यागम दीर्घता;
}

संरचना rsdp_descriptor *RsdpAddr;

शून्य
copy_acpi_rsdp(शून्य *pos)
{
    यदि (RsdpAddr)
        प्रत्यागम;
    पूर्णाङ्क दीर्घता = get_acpi_rsdp_दीर्घता(pos, -1);
    यदि (दीर्घता < 0)
        प्रत्यागम;
    RsdpAddr = copy_fseg_सारणी("ACPI RSDP", pos, दीर्घता);
}

शून्य *find_acpi_rsdp(शून्य)
{
    अचिह्नित दीर्घ start = चिह्नम्(zonefseg_start);
    अचिह्नित दीर्घ अन्तः = चिह्नम्(zonefseg_अन्तः);
    अचिह्नित दीर्घ pos;

    प्रति (pos = ALIGN(start, 0x10); pos <= ALIGN_अधस्तात्(अन्तः, 0x10); pos += 0x10)
        यदि (get_acpi_rsdp_दीर्घता((शून्य *)pos, अन्तः - pos) >= 0)
            प्रत्यागम (शून्य *)pos;

    प्रत्यागम रिक्त;
}

शून्य *
find_acpi_सारणी(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    यदि (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        प्रत्यागम रिक्त;
    संरचना rsdt_descriptor_rev1 *rsdt = (शून्य*)RsdpAddr->rsdt_physical_स्थानाङ्कः;
    संरचना xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_स्थानाङ्कः >= 0x100000000
        ? रिक्त : (शून्य*)(u32)(RsdpAddr->xsdt_physical_स्थानाङ्कः);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    यदि (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        शून्य *अन्तः = (शून्य*)xsdt + xsdt->दीर्घता;
        पूर्णाङ्क i;
        प्रति (i=0; (शून्य*)&xsdt->सारणी_विस्थापनम्_प्रविष्टिः[i] < अन्तः; i++) {
            यदि (xsdt->सारणी_विस्थापनम्_प्रविष्टिः[i] >= 0x100000000)
                अनुवर्त; /* above 4G */
            संरचना acpi_सारणी_शीर्षकम् *tbl = (शून्य*)(u32)xsdt->सारणी_विस्थापनम्_प्रविष्टिः[i];
            यदि (!tbl || tbl->signature != signature)
                अनुवर्त;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            प्रत्यागम tbl;
        }
    }

    यदि (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        शून्य *अन्तः = (शून्य*)rsdt + rsdt->दीर्घता;
        पूर्णाङ्क i;
        प्रति (i=0; (शून्य*)&rsdt->सारणी_विस्थापनम्_प्रविष्टिः[i] < अन्तः; i++) {
            संरचना acpi_सारणी_शीर्षकम् *tbl = (शून्य*)rsdt->सारणी_विस्थापनम्_प्रविष्टिः[i];
            यदि (!tbl || tbl->signature != signature)
                अनुवर्त;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            प्रत्यागम tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    प्रत्यागम रिक्त;
}

u32
find_resume_सदिशम्(शून्य)
{
    संरचना fadt_descriptor_rev1 *fadt = find_acpi_सारणी(FACP_SIGNATURE);
    यदि (!fadt)
        प्रत्यागम 0;
    संरचना facs_descriptor_rev1 *facs = (शून्य*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    यदि (! facs || facs->signature != FACS_SIGNATURE)
        प्रत्यागम 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_सदिशम्);
    प्रत्यागम facs->firmware_waking_सदिशम्;
}

स्थिर संरचना acpi_20_generic_स्थानाङ्कः acpi_reset_reg;
स्थिर u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_आधारः = 0xb000;

#परिभाषय acpi_ga_गन्तव्यम्_bdf(addr) pci_गन्तव्यम्_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

शून्य
acpi_reboot(शून्य)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    यदि (acpi_reset_reg.पञ्जिकाकोशः_bit_विस्तारः != 8)
        प्रत्यागम;

    u64 addr = le64_गन्तव्यम्_cpu(acpi_reset_reg.स्थानाङ्कः);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.स्थानाङ्कः_space_id, addr, acpi_reset_val);

    विकल्प (acpi_reset_reg.स्थानाङ्कः_space_id) {
    पक्ष 0: // System Memory
        writeb((शून्य *)(u32)addr, acpi_reset_val);
        भङ्ग;
    पक्ष 1: // System I/O
        outb(acpi_reset_val, addr);
        भङ्ग;
    पक्ष 2: // PCI config space
        pci_config_writeb(acpi_ga_गन्तव्यम्_bdf(addr), addr & 0xffff, acpi_reset_val);
        भङ्ग;
    }
}

स्थिर शून्य
acpi_समुच्चयः_reset_reg(संरचना acpi_20_generic_स्थानाङ्कः *reg, u8 val)
{
    यदि (!reg || reg->स्थानाङ्कः_space_id > 2 ||
        reg->पञ्जिकाकोशः_bit_विस्तारः != 8 || reg->पञ्जिकाकोशः_bit_विस्थापनम्)
        प्रत्यागम;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

शून्य
find_acpi_features(शून्य)
{
    संरचना fadt_descriptor_rev1 *fadt = find_acpi_सारणी(FACP_SIGNATURE);
    यदि (!fadt)
        प्रत्यागम;
    u32 pm_tmr = le32_गन्तव्यम्_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_गन्तव्यम्_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    यदि (pm_tmr)
        pmtimer_सज्जता(pm_tmr);
    यदि (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    यदि (fadt->दीर्घता >= 129) {
        शून्य *p = fadt;
        acpi_समुच्चयः_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
शून्य *
smbios_अग्र(शून्य *start, u32 दीर्घता, शून्य *पूर्व)
{
    यदि (!start)
        प्रत्यागम रिक्त;
    शून्य *अन्तः = start + दीर्घता;

    यदि (!पूर्व) {
        पूर्व = start;
    } अन्यथा {
        संरचना smbios_structure_शीर्षकम् *hdr = पूर्व;
        यदि (पूर्व + परिमाण(*hdr) > अन्तः)
            प्रत्यागम रिक्त;
        पूर्व += hdr->दीर्घता + 2;
        यावत् (पूर्व < अन्तः && (*(u8*)(पूर्व-1) != '\0' || *(u8*)(पूर्व-2) != '\0'))
            पूर्व++;
    }
    संरचना smbios_structure_शीर्षकम् *hdr = पूर्व;
    यदि (पूर्व >= अन्तः || पूर्व + परिमाण(*hdr) >= अन्तः || पूर्व + hdr->दीर्घता >= अन्तः)
        प्रत्यागम रिक्त;
    प्रत्यागम पूर्व;
}

शून्य *
smbios_21_अग्र(संरचना smbios_21_प्रविष्टिः_point *smbios, शून्य *पूर्व)
{
    यदि (!smbios)
        प्रत्यागम रिक्त;
    प्रत्यागम smbios_अग्र((शून्य*)smbios->structure_सारणी_स्थानाङ्कः,
                       smbios->structure_सारणी_दीर्घता, पूर्व);
}

स्थिर संरचना smbios_21_प्रविष्टिः_point *SMBios21Addr;

शून्य
copy_smbios_21(शून्य *pos)
{
    यदि (SMBios21Addr)
        प्रत्यागम;
    संरचना smbios_21_प्रविष्टिः_point *p = pos;
    यदि (p->signature != SMBIOS_21_SIGNATURE)
        प्रत्यागम;
    यदि (checksum(pos, 0x10) != 0)
        प्रत्यागम;
    यदि (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        प्रत्यागम;
    यदि (checksum(pos+0x10, p->दीर्घता-0x10) != 0)
        प्रत्यागम;
    SMBios21Addr = copy_fseg_सारणी("SMBIOS", pos, p->दीर्घता);
}

स्थिर संरचना smbios_30_प्रविष्टिः_point *SMBios30Addr;

स्थिर पूर्णाङ्क
valid_smbios_30_signature(संरचना smbios_30_प्रविष्टिः_point *p)
{
    प्रत्यागम !memcmp(p->signature, "_SM3_", 5);
}

शून्य
copy_smbios_30(शून्य *pos)
{
    यदि (SMBios30Addr)
        प्रत्यागम;
    संरचना smbios_30_प्रविष्टिः_point *p = pos;
    यदि (!valid_smbios_30_signature(p))
        प्रत्यागम;
    यदि (checksum(pos, p->दीर्घता) != 0)
        प्रत्यागम;
    SMBios30Addr = copy_fseg_सारणी("SMBIOS 3.0", pos, p->दीर्घता);
}

शून्य *smbios_get_tables(u32 *दीर्घता)
{
    यदि (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_सारणी_स्थानाङ्कः;
        यदि (addr32 == SMBios30Addr->structure_सारणी_स्थानाङ्कः) {
            *दीर्घता = SMBios30Addr->structure_सारणी_max_आकार;
            प्रत्यागम (शून्य *)addr32;
        }
    }
    यदि (SMBios21Addr) {
        *दीर्घता = SMBios21Addr->structure_सारणी_दीर्घता;
        प्रत्यागम (शून्य *)SMBios21Addr->structure_सारणी_स्थानाङ्कः;
    }
    प्रत्यागम रिक्त;
}

स्थिर पूर्णाङ्क
smbios_major_संस्करणम्(शून्य)
{
    यदि (SMBios30Addr)
        प्रत्यागम SMBios30Addr->smbios_major_संस्करणम्;
    अन्यथा यदि (SMBios21Addr)
        प्रत्यागम SMBios21Addr->smbios_major_संस्करणम्;
    अन्यथा
        प्रत्यागम 0;
}

स्थिर पूर्णाङ्क
smbios_minor_संस्करणम्(शून्य)
{
    यदि (SMBios30Addr)
        प्रत्यागम SMBios30Addr->smbios_minor_संस्करणम्;
    अन्यथा यदि (SMBios21Addr)
        प्रत्यागम SMBios21Addr->smbios_minor_संस्करणम्;
    अन्यथा
        प्रत्यागम 0;
}

शून्य
display_uuid(शून्य)
{
    u32 smbios_len = 0;
    शून्य *smbios_tables = smbios_get_tables(&smbios_len);
    संरचना smbios_type_1 *tbl = smbios_अग्र(smbios_tables, smbios_len, रिक्त);
    पूर्णाङ्क minlen = offsetof(संरचना smbios_type_1, uuid) + परिमाण(tbl->uuid);
    प्रति (; tbl; tbl = smbios_अग्र(smbios_tables, smbios_len, tbl))
        यदि (tbl->शीर्षकम्.type == 1 && tbl->शीर्षकम्.दीर्घता >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 रिक्तानि_uuid[परिमाण(tbl->uuid)] = { 0 };
            यदि (memcmp(uuid, रिक्तानि_uuid, परिमाण(रिक्तानि_uuid)) == 0)
                प्रत्यागम;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            यदि (smbios_major_संस्करणम्() > 2 ||
                (smbios_major_संस्करणम्() == 2 &&
                 smbios_minor_संस्करणम्() >= 6)) {
                प्रारूप_लिख("Machine UUID"
                       " %02x%02x%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x%02x%02x%02x%02x\n"
                       , uuid[ 3], uuid[ 2], uuid[ 1], uuid[ 0]
                       , uuid[ 5], uuid[ 4]
                       , uuid[ 7], uuid[ 6]
                       , uuid[ 8], uuid[ 9]
                       , uuid[10], uuid[11], uuid[12]
                       , uuid[13], uuid[14], uuid[15]);
            } अन्यथा {
                प्रारूप_लिख("Machine UUID"
                       " %02x%02x%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x%02x%02x%02x%02x\n"
                       , uuid[ 0], uuid[ 1], uuid[ 2], uuid[ 3]
                       , uuid[ 4], uuid[ 5]
                       , uuid[ 6], uuid[ 7]
                       , uuid[ 8], uuid[ 9]
                       , uuid[10], uuid[11], uuid[12]
                       , uuid[13], uuid[14], uuid[15]);
            }

            प्रत्यागम;
        }
}

#परिभाषय समुच्चयः_str_क्षेत्रम्_or_skip(type, क्षेत्रम्, मानम्)                       \
    कुरु {                                                                \
        पूर्णाङ्क आकार = (मानम् != रिक्त) ? वाक्यदैर्घ्यम्(मानम्) + 1 : 0;             \
        यदि (आकार > 1) {                                                 \
            स्मृतिं_प्रतिलिख(अन्तः, मानम्, आकार);                                   \
            अन्तः += आकार;                                                \
            p->क्षेत्रम् = ++str_सूचकांक;                                     \
        } अन्यथा {                                                        \
            p->क्षेत्रम् = 0;                                               \
        }                                                               \
    } यावत् (0)

स्थिर शून्य *
smbios_new_type_0(शून्य *start,
                  ध्रुव वर्ण *vendor, ध्रुव वर्ण *संस्करणम्_2, ध्रुव वर्ण *date)
{
    संरचना smbios_type_0 *p = (संरचना smbios_type_0 *)start;
    वर्ण *अन्तः = (वर्ण *)start + परिमाण(संरचना smbios_type_0);
    पूर्णाङ्क str_सूचकांक = 0;

    p->शीर्षकम्.type = 0;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_0);
    p->शीर्षकम्.handle = 0;

    समुच्चयः_str_क्षेत्रम्_or_skip(0, vendor_str, vendor);
    समुच्चयः_str_क्षेत्रम्_or_skip(0, bios_संस्करणम्_str, संस्करणम्_2);
    p->bios_starting_स्थानाङ्कः_segment = 0xe800;
    समुच्चयः_str_क्षेत्रम्_or_skip(0, bios_release_date_str, date);

    p->bios_rom_आकार = 0; /* FIXME */

    /* BIOS characteristics not supported */
    स्मृतिं_पूरय(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_अष्टकानि[0] = 0;
    p->bios_characteristics_extension_अष्टकानि[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *अन्तः = 0;
    अन्तः++;
    यदि (!str_सूचकांक) {
        *अन्तः = 0;
        अन्तः++;
    }

    प्रत्यागम अन्तः;
}

#परिभाषय BIOS_NAME "SeaBIOS"
#परिभाषय BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
स्थिर पूर्णाङ्क
smbios_build_tables(संरचना romfile_s *f_tables,
                    u64 *स्थानाङ्कः, u32 *दीर्घता,
                    u16 *max_structure_आकार,
                    u16 *सङ्ख्या_सम्बन्धि_structures)
{
    संरचना smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    यदि (f_tables->आकार != *दीर्घता)
        प्रत्यागम 0;

    qtables = malloc_tmphigh(f_tables->आकार);
    यदि (!qtables) {
        warn_noalloc();
        प्रत्यागम 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->आकार);
    qtables_len = f_tables->आकार;

    /* did we get a type 0 structure ? */
    प्रति (t0 = smbios_अग्र(qtables, qtables_len, रिक्त); t0;
         t0 = smbios_अग्र(qtables, qtables_len, t0)) {
        यदि (t0->शीर्षकम्.type == 0) {
            need_t0 = 0;
            भङ्ग;
        }
    }

    यदि (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = परिमाण(संरचना smbios_type_0) + वाक्यदैर्घ्यम्(BIOS_NAME) +
                     वाक्यदैर्घ्यम्(संस्करणम्) + वाक्यदैर्घ्यम्(BIOS_DATE) + 4;
        यदि (t0_len > (0xffff - *दीर्घता)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *दीर्घता, t0_len);
            need_t0 = 0;
        } अन्यथा {
            *दीर्घता += t0_len;
            यदि (max_structure_आकार && t0_len > *max_structure_आकार)
                *max_structure_आकार = t0_len;
            यदि (सङ्ख्या_सम्बन्धि_structures)
                (*सङ्ख्या_सम्बन्धि_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    यदि (*दीर्घता > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_उच्चः(*दीर्घता);
    अन्यथा
        tables = malloc_fseg(*दीर्घता);
    यदि (!tables) {
        warn_noalloc();
        मुक्त(qtables);
        प्रत्यागम 0;
    }
    *स्थानाङ्कः = (u32)tables;

    /* populate final blob */
    यदि (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, संस्करणम्, BIOS_DATE);
    स्मृतिं_प्रतिलिख(tables, qtables, qtables_len);
    मुक्त(qtables);
    प्रत्यागम 1;
}

स्थिर पूर्णाङ्क
smbios_21_सज्जता_प्रविष्टिः_point(संरचना romfile_s *f_tables,
                            संरचना smbios_21_प्रविष्टिः_point *ep)
{
    u64 स्थानाङ्कः = ep->structure_सारणी_स्थानाङ्कः;
    u32 दीर्घता = ep->structure_सारणी_दीर्घता;

    यदि (!smbios_build_tables(f_tables,
                             &स्थानाङ्कः,
                             &दीर्घता,
                             &ep->max_structure_आकार,
                             &ep->सङ्ख्या_सम्बन्धि_structures))
        प्रत्यागम 0;

    यदि ((u32)स्थानाङ्कः != स्थानाङ्कः || (u16)दीर्घता != दीर्घता) {
        warn_internalerror();
        प्रत्यागम 0;
    }

    /* finalize entry point */
    ep->structure_सारणी_स्थानाङ्कः = स्थानाङ्कः;
    ep->structure_सारणी_दीर्घता = दीर्घता;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((शून्य *)ep + 0x10, ep->दीर्घता - 0x10);

    copy_smbios_21(ep);
    प्रत्यागम 1;
}

स्थिर पूर्णाङ्क
smbios_30_सज्जता_प्रविष्टिः_point(संरचना romfile_s *f_tables,
                            संरचना smbios_30_प्रविष्टिः_point *ep)
{
    यदि (!smbios_build_tables(f_tables,
                             &ep->structure_सारणी_स्थानाङ्कः,
                             &ep->structure_सारणी_max_आकार,
                             रिक्त, रिक्त))
        प्रत्यागम 0;

    ep->checksum -= checksum(ep, परिमाण(*ep));
    copy_smbios_30(ep);
    प्रत्यागम 1;
}

स्थिर पूर्णाङ्क
smbios_romfile_सज्जता(शून्य)
{
    संरचना romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    संरचना romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    सङ्घ {
        संरचना smbios_21_प्रविष्टिः_point ep21;
        संरचना smbios_30_प्रविष्टिः_point ep30;
    } ep;

    यदि (!f_anchor || !f_tables || f_anchor->आकार > परिमाण(ep))
        प्रत्यागम 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->आकार);

    यदि (f_anchor->आकार == परिमाण(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        प्रत्यागम smbios_21_सज्जता_प्रविष्टिः_point(f_tables, &ep.ep21);
    } अन्यथा यदि (f_anchor->आकार == परिमाण(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        प्रत्यागम smbios_30_सज्जता_प्रविष्टिः_point(f_tables, &ep.ep30);
    } अन्यथा {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        प्रत्यागम 0;
    }
}

शून्य
smbios_सज्जता(शून्य)
{
    यदि (smbios_romfile_सज्जता())
        प्रत्यागम;
    smbios_legacy_सज्जता();
}

शून्य
copy_सारणी(शून्य *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
