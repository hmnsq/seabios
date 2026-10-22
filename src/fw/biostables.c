// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "byteorder.h" // le32_to_cpu
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "hw/pci.h" // pci_config_writeb
#অন্তর্ভাব "malloc.h" // malloc_fseg
#অন্তর্ভাব "memmap.h" //প্রতীক
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_find
#অন্তর্ভাব "std/acpi.h" // struct rsdp_descriptor
#অন্তর্ভাব "std/mptable.h" // MPTABLE_SIGNATURE
#অন্তর্ভাব "std/pirtable.h" // struct pir_header
#অন্তর্ভাব "std/smbios.h" // struct smbios_21_entry_point
#অন্তর্ভাব "string.h" // memcpy
#অন্তর্ভাব "util.h" // copy_table
#অন্তর্ভাব "x86.h" // outb

কাঠামো pir_header *PirAddr VARFSEG;

স্থির শূন্য *
copy_fseg_table(ধ্রুবক অক্ষর *name, শূন্য *pos, u32 আকার_2)
{
    শূন্য *newpos = malloc_fseg(আকার_2);
    যদি (!newpos) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, আকার_2);
    ফেরত newpos;
}

শূন্য
copy_pir(শূন্য *pos)
{
    কাঠামো pir_header *p = pos;
    যদি (p->signature != PIR_SIGNATURE)
        ফেরত;
    যদি (PirAddr)
        ফেরত;
    যদি (p->আকার_2 < আকার(*p))
        ফেরত;
    যদি (checksum(pos, p->আকার_2) != 0)
        ফেরত;
    PirAddr = copy_fseg_table("PIR", pos, p->আকার_2);
}

শূন্য
copy_mptable(শূন্য *pos)
{
    কাঠামো mptable_floating_s *p = pos;
    যদি (p->signature != MPTABLE_SIGNATURE)
        ফেরত;
    যদি (!p->physaddr)
        ফেরত;
    যদি (checksum(pos, আকার(*p)) != 0)
        ফেরত;
    u32 দৈর্ঘ্য = p->দৈর্ঘ্য * 16;
    u16 mpclength = ((কাঠামো mptable_config_s *)p->physaddr)->দৈর্ঘ্য;
    যদি (দৈর্ঘ্য + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , দৈর্ঘ্য + mpclength);
        ফেরত;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    কাঠামো mptable_floating_s *newpos = malloc_fseg(দৈর্ঘ্য + mpclength);
    যদি (!newpos) {
        warn_noalloc();
        ফেরত;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, দৈর্ঘ্য);
    newpos->physaddr = (u32)newpos + দৈর্ঘ্য;
    newpos->checksum -= checksum(newpos, আকার(*newpos));
    memcpy((শূন্য*)newpos + দৈর্ঘ্য, (শূন্য*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

স্থির পূর্ণসংখ্যা
get_acpi_rsdp_দৈর্ঘ্য(শূন্য *pos, unsigned আকার_2)
{
    কাঠামো rsdp_descriptor *p = pos;
    যদি (p->signature != RSDP_SIGNATURE)
        ফেরত -1;
    u32 দৈর্ঘ্য = 20;
    যদি (দৈর্ঘ্য > আকার_2)
        ফেরত -1;
    যদি (checksum(pos, দৈর্ঘ্য) != 0)
        ফেরত -1;
    যদি (p->revision > 1) {
        দৈর্ঘ্য = p->দৈর্ঘ্য;
        যদি (দৈর্ঘ্য > আকার_2)
            ফেরত -1;
        যদি (checksum(pos, দৈর্ঘ্য) != 0)
            ফেরত -1;
    }
    ফেরত দৈর্ঘ্য;
}

কাঠামো rsdp_descriptor *RsdpAddr;

শূন্য
copy_acpi_rsdp(শূন্য *pos)
{
    যদি (RsdpAddr)
        ফেরত;
    পূর্ণসংখ্যা দৈর্ঘ্য = get_acpi_rsdp_দৈর্ঘ্য(pos, -1);
    যদি (দৈর্ঘ্য < 0)
        ফেরত;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, দৈর্ঘ্য);
}

শূন্য *find_acpi_rsdp(শূন্য)
{
    unsigned long start = প্রতীক(zonefseg_start);
    unsigned long end = প্রতীক(zonefseg_end);
    unsigned long pos;

    জন্য (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        যদি (get_acpi_rsdp_দৈর্ঘ্য((শূন্য *)pos, end - pos) >= 0)
            ফেরত (শূন্য *)pos;

    ফেরত ফাঁকা;
}

শূন্য *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    যদি (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        ফেরত ফাঁকা;
    কাঠামো rsdt_descriptor_rev1 *rsdt = (শূন্য*)RsdpAddr->rsdt_physical_address;
    কাঠামো xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? ফাঁকা : (শূন্য*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    যদি (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        শূন্য *end = (শূন্য*)xsdt + xsdt->দৈর্ঘ্য;
        পূর্ণসংখ্যা i;
        জন্য (i=0; (শূন্য*)&xsdt->table_offset_entry[i] < end; i++) {
            যদি (xsdt->table_offset_entry[i] >= 0x100000000)
                চালু; /* above 4G */
            কাঠামো acpi_table_header *tbl = (শূন্য*)(u32)xsdt->table_offset_entry[i];
            যদি (!tbl || tbl->signature != signature)
                চালু;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            ফেরত tbl;
        }
    }

    যদি (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        শূন্য *end = (শূন্য*)rsdt + rsdt->দৈর্ঘ্য;
        পূর্ণসংখ্যা i;
        জন্য (i=0; (শূন্য*)&rsdt->table_offset_entry[i] < end; i++) {
            কাঠামো acpi_table_header *tbl = (শূন্য*)rsdt->table_offset_entry[i];
            যদি (!tbl || tbl->signature != signature)
                চালু;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            ফেরত tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    ফেরত ফাঁকা;
}

u32
find_resume_vector(শূন্য)
{
    কাঠামো fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    যদি (!fadt)
        ফেরত 0;
    কাঠামো facs_descriptor_rev1 *facs = (শূন্য*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    যদি (! facs || facs->signature != FACS_SIGNATURE)
        ফেরত 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    ফেরত facs->firmware_waking_vector;
}

স্থির কাঠামো acpi_20_generic_address acpi_reset_reg;
স্থির u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

শূন্য
acpi_reboot(শূন্য)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    যদি (acpi_reset_reg.register_bit_width != 8)
        ফেরত;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    নির্বাচন (acpi_reset_reg.address_space_id) {
    ক্ষেত্র 0: // System Memory
        writeb((শূন্য *)(u32)addr, acpi_reset_val);
        বিরতি;
    ক্ষেত্র 1: // System I/O
        outb(acpi_reset_val, addr);
        বিরতি;
    ক্ষেত্র 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        বিরতি;
    }
}

স্থির শূন্য
acpi_set_reset_reg(কাঠামো acpi_20_generic_address *reg, u8 val)
{
    যদি (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        ফেরত;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

শূন্য
find_acpi_features(শূন্য)
{
    কাঠামো fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    যদি (!fadt)
        ফেরত;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    যদি (pm_tmr)
        pmtimer_setup(pm_tmr);
    যদি (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    যদি (fadt->দৈর্ঘ্য >= 129) {
        শূন্য *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
শূন্য *
smbios_পরবর্তী(শূন্য *start, u32 দৈর্ঘ্য, শূন্য *পূর্ব)
{
    যদি (!start)
        ফেরত ফাঁকা;
    শূন্য *end = start + দৈর্ঘ্য;

    যদি (!পূর্ব) {
        পূর্ব = start;
    } অন্যথা {
        কাঠামো smbios_structure_header *hdr = পূর্ব;
        যদি (পূর্ব + আকার(*hdr) > end)
            ফেরত ফাঁকা;
        পূর্ব += hdr->দৈর্ঘ্য + 2;
        যখন (পূর্ব < end && (*(u8*)(পূর্ব-1) != '\0' || *(u8*)(পূর্ব-2) != '\0'))
            পূর্ব++;
    }
    কাঠামো smbios_structure_header *hdr = পূর্ব;
    যদি (পূর্ব >= end || পূর্ব + আকার(*hdr) >= end || পূর্ব + hdr->দৈর্ঘ্য >= end)
        ফেরত ফাঁকা;
    ফেরত পূর্ব;
}

শূন্য *
smbios_21_পরবর্তী(কাঠামো smbios_21_entry_point *smbios, শূন্য *পূর্ব)
{
    যদি (!smbios)
        ফেরত ফাঁকা;
    ফেরত smbios_পরবর্তী((শূন্য*)smbios->structure_table_address,
                       smbios->structure_table_দৈর্ঘ্য, পূর্ব);
}

স্থির কাঠামো smbios_21_entry_point *SMBios21Addr;

শূন্য
copy_smbios_21(শূন্য *pos)
{
    যদি (SMBios21Addr)
        ফেরত;
    কাঠামো smbios_21_entry_point *p = pos;
    যদি (p->signature != SMBIOS_21_SIGNATURE)
        ফেরত;
    যদি (checksum(pos, 0x10) != 0)
        ফেরত;
    যদি (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        ফেরত;
    যদি (checksum(pos+0x10, p->দৈর্ঘ্য-0x10) != 0)
        ফেরত;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->দৈর্ঘ্য);
}

স্থির কাঠামো smbios_30_entry_point *SMBios30Addr;

স্থির পূর্ণসংখ্যা
valid_smbios_30_signature(কাঠামো smbios_30_entry_point *p)
{
    ফেরত !memcmp(p->signature, "_SM3_", 5);
}

শূন্য
copy_smbios_30(শূন্য *pos)
{
    যদি (SMBios30Addr)
        ফেরত;
    কাঠামো smbios_30_entry_point *p = pos;
    যদি (!valid_smbios_30_signature(p))
        ফেরত;
    যদি (checksum(pos, p->দৈর্ঘ্য) != 0)
        ফেরত;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->দৈর্ঘ্য);
}

শূন্য *smbios_get_tables(u32 *দৈর্ঘ্য)
{
    যদি (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        যদি (addr32 == SMBios30Addr->structure_table_address) {
            *দৈর্ঘ্য = SMBios30Addr->structure_table_max_আকার;
            ফেরত (শূন্য *)addr32;
        }
    }
    যদি (SMBios21Addr) {
        *দৈর্ঘ্য = SMBios21Addr->structure_table_দৈর্ঘ্য;
        ফেরত (শূন্য *)SMBios21Addr->structure_table_address;
    }
    ফেরত ফাঁকা;
}

স্থির পূর্ণসংখ্যা
smbios_major_version(শূন্য)
{
    যদি (SMBios30Addr)
        ফেরত SMBios30Addr->smbios_major_version;
    অন্যথা যদি (SMBios21Addr)
        ফেরত SMBios21Addr->smbios_major_version;
    অন্যথা
        ফেরত 0;
}

স্থির পূর্ণসংখ্যা
smbios_minor_version(শূন্য)
{
    যদি (SMBios30Addr)
        ফেরত SMBios30Addr->smbios_minor_version;
    অন্যথা যদি (SMBios21Addr)
        ফেরত SMBios21Addr->smbios_minor_version;
    অন্যথা
        ফেরত 0;
}

শূন্য
display_uuid(শূন্য)
{
    u32 smbios_len = 0;
    শূন্য *smbios_tables = smbios_get_tables(&smbios_len);
    কাঠামো smbios_type_1 *tbl = smbios_পরবর্তী(smbios_tables, smbios_len, ফাঁকা);
    পূর্ণসংখ্যা minlen = offsetof(কাঠামো smbios_type_1, uuid) + আকার(tbl->uuid);
    জন্য (; tbl; tbl = smbios_পরবর্তী(smbios_tables, smbios_len, tbl))
        যদি (tbl->header.type == 1 && tbl->header.দৈর্ঘ্য >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[আকার(tbl->uuid)] = { 0 };
            যদি (memcmp(uuid, empty_uuid, আকার(empty_uuid)) == 0)
                ফেরত;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            যদি (smbios_major_version() > 2 ||
                (smbios_major_version() == 2 &&
                 smbios_minor_version() >= 6)) {
                printf("Machine UUID"
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
            } অন্যথা {
                printf("Machine UUID"
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

            ফেরত;
        }
}

#define set_str_field_or_skip(type, field, মান)                       \
    করো {                                                                \
        পূর্ণসংখ্যা আকার_2 = (মান != ফাঁকা) ? strlen(মান) + 1 : 0;             \
        যদি (আকার_2 > 1) {                                                 \
            memcpy(end, মান, আকার_2);                                   \
            end += আকার_2;                                                \
            p->field = ++str_সূচক;                                     \
        } অন্যথা {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } যখন (0)

স্থির শূন্য *
smbios_new_type_0(শূন্য *start,
                  ধ্রুবক অক্ষর *vendor, ধ্রুবক অক্ষর *version, ধ্রুবক অক্ষর *date)
{
    কাঠামো smbios_type_0 *p = (কাঠামো smbios_type_0 *)start;
    অক্ষর *end = (অক্ষর *)start + আকার(কাঠামো smbios_type_0);
    পূর্ণসংখ্যা str_সূচক = 0;

    p->header.type = 0;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_0);
    p->header.handle = 0;

    set_str_field_or_skip(0, vendor_str, vendor);
    set_str_field_or_skip(0, bios_version_str, version);
    p->bios_starting_address_segment = 0xe800;
    set_str_field_or_skip(0, bios_release_date_str, date);

    p->bios_rom_আকার = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_bytes[0] = 0;
    p->bios_characteristics_extension_bytes[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *end = 0;
    end++;
    যদি (!str_সূচক) {
        *end = 0;
        end++;
    }

    ফেরত end;
}

#define BIOS_NAME "SeaBIOS"
#define BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
স্থির পূর্ণসংখ্যা
smbios_build_tables(কাঠামো romfile_s *f_tables,
                    u64 *address, u32 *দৈর্ঘ্য,
                    u16 *max_structure_আকার,
                    u16 *সংখ্যা_of_structures)
{
    কাঠামো smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    যদি (f_tables->আকার_2 != *দৈর্ঘ্য)
        ফেরত 0;

    qtables = malloc_tmphigh(f_tables->আকার_2);
    যদি (!qtables) {
        warn_noalloc();
        ফেরত 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->আকার_2);
    qtables_len = f_tables->আকার_2;

    /* did we get a type 0 structure ? */
    জন্য (t0 = smbios_পরবর্তী(qtables, qtables_len, ফাঁকা); t0;
         t0 = smbios_পরবর্তী(qtables, qtables_len, t0)) {
        যদি (t0->header.type == 0) {
            need_t0 = 0;
            বিরতি;
        }
    }

    যদি (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = আকার(কাঠামো smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        যদি (t0_len > (0xffff - *দৈর্ঘ্য)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *দৈর্ঘ্য, t0_len);
            need_t0 = 0;
        } অন্যথা {
            *দৈর্ঘ্য += t0_len;
            যদি (max_structure_আকার && t0_len > *max_structure_আকার)
                *max_structure_আকার = t0_len;
            যদি (সংখ্যা_of_structures)
                (*সংখ্যা_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    যদি (*দৈর্ঘ্য > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_উপরের(*দৈর্ঘ্য);
    অন্যথা
        tables = malloc_fseg(*দৈর্ঘ্য);
    যদি (!tables) {
        warn_noalloc();
        মুক্ত(qtables);
        ফেরত 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    যদি (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    মুক্ত(qtables);
    ফেরত 1;
}

স্থির পূর্ণসংখ্যা
smbios_21_setup_entry_point(কাঠামো romfile_s *f_tables,
                            কাঠামো smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 দৈর্ঘ্য = ep->structure_table_দৈর্ঘ্য;

    যদি (!smbios_build_tables(f_tables,
                             &address,
                             &দৈর্ঘ্য,
                             &ep->max_structure_আকার,
                             &ep->সংখ্যা_of_structures))
        ফেরত 0;

    যদি ((u32)address != address || (u16)দৈর্ঘ্য != দৈর্ঘ্য) {
        warn_internalerror();
        ফেরত 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_দৈর্ঘ্য = দৈর্ঘ্য;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((শূন্য *)ep + 0x10, ep->দৈর্ঘ্য - 0x10);

    copy_smbios_21(ep);
    ফেরত 1;
}

স্থির পূর্ণসংখ্যা
smbios_30_setup_entry_point(কাঠামো romfile_s *f_tables,
                            কাঠামো smbios_30_entry_point *ep)
{
    যদি (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_আকার,
                             ফাঁকা, ফাঁকা))
        ফেরত 0;

    ep->checksum -= checksum(ep, আকার(*ep));
    copy_smbios_30(ep);
    ফেরত 1;
}

স্থির পূর্ণসংখ্যা
smbios_romfile_setup(শূন্য)
{
    কাঠামো romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    কাঠামো romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    সংঘ {
        কাঠামো smbios_21_entry_point ep21;
        কাঠামো smbios_30_entry_point ep30;
    } ep;

    যদি (!f_anchor || !f_tables || f_anchor->আকার_2 > আকার(ep))
        ফেরত 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->আকার_2);

    যদি (f_anchor->আকার_2 == আকার(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        ফেরত smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } অন্যথা যদি (f_anchor->আকার_2 == আকার(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        ফেরত smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } অন্যথা {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        ফেরত 0;
    }
}

শূন্য
smbios_setup(শূন্য)
{
    যদি (smbios_romfile_setup())
        ফেরত;
    smbios_legacy_setup();
}

শূন্য
copy_table(শূন্য *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
