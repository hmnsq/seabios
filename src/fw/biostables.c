// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "byteorder.h" // le32_to_cpu
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "hw/pci.h" // pci_config_writeb
#অন্তর্ভাৱ "malloc.h" // malloc_fseg
#অন্তর্ভাৱ "memmap.h" //প্ৰতীক
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "romfile.h" // romfile_find
#অন্তর্ভাৱ "std/acpi.h" // struct rsdp_descriptor
#অন্তর্ভাৱ "std/mptable.h" // MPTABLE_SIGNATURE
#অন্তর্ভাৱ "std/pirtable.h" // struct pir_header
#অন্তর্ভাৱ "std/smbios.h" // struct smbios_21_entry_point
#অন্তর্ভাৱ "string.h" // memcpy
#অন্তর্ভাৱ "util.h" // copy_table
#অন্তর্ভাৱ "x86.h" // outb

সংরচনা pir_header *PirAddr VARFSEG;

স্থির শূন্য *
copy_fseg_table(ধ্রুৱ ৱর্ণ *name, শূন্য *pos, u32 size)
{
    শূন্য *newpos = malloc_fseg(size);
    যদি (!newpos) {
        warn_noalloc();
        ঘূৰাই রিক্ত;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    ঘূৰাই newpos;
}

শূন্য
copy_pir(শূন্য *pos)
{
    সংরচনা pir_header *p = pos;
    যদি (p->signature != PIR_SIGNATURE)
        ঘূৰাই;
    যদি (PirAddr)
        ঘূৰাই;
    যদি (p->size < পরিমাণ(*p))
        ঘূৰাই;
    যদি (checksum(pos, p->size) != 0)
        ঘূৰাই;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

শূন্য
copy_mptable(শূন্য *pos)
{
    সংরচনা mptable_floating_s *p = pos;
    যদি (p->signature != MPTABLE_SIGNATURE)
        ঘূৰাই;
    যদি (!p->physaddr)
        ঘূৰাই;
    যদি (checksum(pos, পরিমাণ(*p)) != 0)
        ঘূৰাই;
    u32 length = p->length * 16;
    u16 mpclength = ((সংরচনা mptable_config_s *)p->physaddr)->length;
    যদি (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        ঘূৰাই;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    সংরচনা mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    যদি (!newpos) {
        warn_noalloc();
        ঘূৰাই;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, পরিমাণ(*newpos));
    memcpy((শূন্য*)newpos + length, (শূন্য*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

স্থির পূর্ণাঙ্ক
get_acpi_rsdp_length(শূন্য *pos, unsigned size)
{
    সংরচনা rsdp_descriptor *p = pos;
    যদি (p->signature != RSDP_SIGNATURE)
        ঘূৰাই -1;
    u32 length = 20;
    যদি (length > size)
        ঘূৰাই -1;
    যদি (checksum(pos, length) != 0)
        ঘূৰাই -1;
    যদি (p->revision > 1) {
        length = p->length;
        যদি (length > size)
            ঘূৰাই -1;
        যদি (checksum(pos, length) != 0)
            ঘূৰাই -1;
    }
    ঘূৰাই length;
}

সংরচনা rsdp_descriptor *RsdpAddr;

শূন্য
copy_acpi_rsdp(শূন্য *pos)
{
    যদি (RsdpAddr)
        ঘূৰাই;
    পূর্ণাঙ্ক length = get_acpi_rsdp_length(pos, -1);
    যদি (length < 0)
        ঘূৰাই;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

শূন্য *find_acpi_rsdp(শূন্য)
{
    unsigned long start = প্ৰতীক(zonefseg_start);
    unsigned long end = প্ৰতীক(zonefseg_end);
    unsigned long pos;

    বাবে (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        যদি (get_acpi_rsdp_length((শূন্য *)pos, end - pos) >= 0)
            ঘূৰাই (শূন্য *)pos;

    ঘূৰাই রিক্ত;
}

শূন্য *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    যদি (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        ঘূৰাই রিক্ত;
    সংরচনা rsdt_descriptor_rev1 *rsdt = (শূন্য*)RsdpAddr->rsdt_physical_address;
    সংরচনা xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? রিক্ত : (শূন্য*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    যদি (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        শূন্য *end = (শূন্য*)xsdt + xsdt->length;
        পূর্ণাঙ্ক i;
        বাবে (i=0; (শূন্য*)&xsdt->table_offset_entry[i] < end; i++) {
            যদি (xsdt->table_offset_entry[i] >= 0x100000000)
                অনুৱর্ত; /* above 4G */
            সংরচনা acpi_table_header *tbl = (শূন্য*)(u32)xsdt->table_offset_entry[i];
            যদি (!tbl || tbl->signature != signature)
                অনুৱর্ত;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            ঘূৰাই tbl;
        }
    }

    যদি (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        শূন্য *end = (শূন্য*)rsdt + rsdt->length;
        পূর্ণাঙ্ক i;
        বাবে (i=0; (শূন্য*)&rsdt->table_offset_entry[i] < end; i++) {
            সংরচনা acpi_table_header *tbl = (শূন্য*)rsdt->table_offset_entry[i];
            যদি (!tbl || tbl->signature != signature)
                অনুৱর্ত;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            ঘূৰাই tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    ঘূৰাই রিক্ত;
}

u32
find_resume_vector(শূন্য)
{
    সংরচনা fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    যদি (!fadt)
        ঘূৰাই 0;
    সংরচনা facs_descriptor_rev1 *facs = (শূন্য*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    যদি (! facs || facs->signature != FACS_SIGNATURE)
        ঘূৰাই 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    ঘূৰাই facs->firmware_waking_vector;
}

স্থির সংরচনা acpi_20_generic_address acpi_reset_reg;
স্থির u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

শূন্য
acpi_reboot(শূন্য)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    যদি (acpi_reset_reg.register_bit_width != 8)
        ঘূৰাই;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    ৱিকল্প (acpi_reset_reg.address_space_id) {
    পক্ষ 0: // System Memory
        writeb((শূন্য *)(u32)addr, acpi_reset_val);
        ভঙ্গ;
    পক্ষ 1: // System I/O
        outb(acpi_reset_val, addr);
        ভঙ্গ;
    পক্ষ 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        ভঙ্গ;
    }
}

স্থির শূন্য
acpi_set_reset_reg(সংরচনা acpi_20_generic_address *reg, u8 val)
{
    যদি (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        ঘূৰাই;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

শূন্য
find_acpi_features(শূন্য)
{
    সংরচনা fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    যদি (!fadt)
        ঘূৰাই;
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
    যদি (fadt->length >= 129) {
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
smbios_next(শূন্য *start, u32 length, শূন্য *prev)
{
    যদি (!start)
        ঘূৰাই রিক্ত;
    শূন্য *end = start + length;

    যদি (!prev) {
        prev = start;
    } অন্যথা {
        সংরচনা smbios_structure_header *hdr = prev;
        যদি (prev + পরিমাণ(*hdr) > end)
            ঘূৰাই রিক্ত;
        prev += hdr->length + 2;
        যেতিয়ালৈ (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    সংরচনা smbios_structure_header *hdr = prev;
    যদি (prev >= end || prev + পরিমাণ(*hdr) >= end || prev + hdr->length >= end)
        ঘূৰাই রিক্ত;
    ঘূৰাই prev;
}

শূন্য *
smbios_21_next(সংরচনা smbios_21_entry_point *smbios, শূন্য *prev)
{
    যদি (!smbios)
        ঘূৰাই রিক্ত;
    ঘূৰাই smbios_next((শূন্য*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

স্থির সংরচনা smbios_21_entry_point *SMBios21Addr;

শূন্য
copy_smbios_21(শূন্য *pos)
{
    যদি (SMBios21Addr)
        ঘূৰাই;
    সংরচনা smbios_21_entry_point *p = pos;
    যদি (p->signature != SMBIOS_21_SIGNATURE)
        ঘূৰাই;
    যদি (checksum(pos, 0x10) != 0)
        ঘূৰাই;
    যদি (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        ঘূৰাই;
    যদি (checksum(pos+0x10, p->length-0x10) != 0)
        ঘূৰাই;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

স্থির সংরচনা smbios_30_entry_point *SMBios30Addr;

স্থির পূর্ণাঙ্ক
valid_smbios_30_signature(সংরচনা smbios_30_entry_point *p)
{
    ঘূৰাই !memcmp(p->signature, "_SM3_", 5);
}

শূন্য
copy_smbios_30(শূন্য *pos)
{
    যদি (SMBios30Addr)
        ঘূৰাই;
    সংরচনা smbios_30_entry_point *p = pos;
    যদি (!valid_smbios_30_signature(p))
        ঘূৰাই;
    যদি (checksum(pos, p->length) != 0)
        ঘূৰাই;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

শূন্য *smbios_get_tables(u32 *length)
{
    যদি (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        যদি (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            ঘূৰাই (শূন্য *)addr32;
        }
    }
    যদি (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        ঘূৰাই (শূন্য *)SMBios21Addr->structure_table_address;
    }
    ঘূৰাই রিক্ত;
}

স্থির পূর্ণাঙ্ক
smbios_major_version(শূন্য)
{
    যদি (SMBios30Addr)
        ঘূৰাই SMBios30Addr->smbios_major_version;
    অন্যথা যদি (SMBios21Addr)
        ঘূৰাই SMBios21Addr->smbios_major_version;
    অন্যথা
        ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক
smbios_minor_version(শূন্য)
{
    যদি (SMBios30Addr)
        ঘূৰাই SMBios30Addr->smbios_minor_version;
    অন্যথা যদি (SMBios21Addr)
        ঘূৰাই SMBios21Addr->smbios_minor_version;
    অন্যথা
        ঘূৰাই 0;
}

শূন্য
display_uuid(শূন্য)
{
    u32 smbios_len = 0;
    শূন্য *smbios_tables = smbios_get_tables(&smbios_len);
    সংরচনা smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, রিক্ত);
    পূর্ণাঙ্ক minlen = offsetof(সংরচনা smbios_type_1, uuid) + পরিমাণ(tbl->uuid);
    বাবে (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        যদি (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[পরিমাণ(tbl->uuid)] = { 0 };
            যদি (memcmp(uuid, empty_uuid, পরিমাণ(empty_uuid)) == 0)
                ঘূৰাই;

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

            ঘূৰাই;
        }
}

#define set_str_field_or_skip(type, field, মান)                       \
    কৰা {                                                                \
        পূর্ণাঙ্ক size = (মান != রিক্ত) ? strlen(মান) + 1 : 0;             \
        যদি (size > 1) {                                                 \
            memcpy(end, মান, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } অন্যথা {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } যেতিয়ালৈ (0)

স্থির শূন্য *
smbios_new_type_0(শূন্য *start,
                  ধ্রুৱ ৱর্ণ *vendor, ধ্রুৱ ৱর্ণ *version, ধ্রুৱ ৱর্ণ *date)
{
    সংরচনা smbios_type_0 *p = (সংরচনা smbios_type_0 *)start;
    ৱর্ণ *end = (ৱর্ণ *)start + পরিমাণ(সংরচনা smbios_type_0);
    পূর্ণাঙ্ক str_index = 0;

    p->header.type = 0;
    p->header.length = পরিমাণ(সংরচনা smbios_type_0);
    p->header.handle = 0;

    set_str_field_or_skip(0, vendor_str, vendor);
    set_str_field_or_skip(0, bios_version_str, version);
    p->bios_starting_address_segment = 0xe800;
    set_str_field_or_skip(0, bios_release_date_str, date);

    p->bios_rom_size = 0; /* FIXME */

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
    যদি (!str_index) {
        *end = 0;
        end++;
    }

    ঘূৰাই end;
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
স্থির পূর্ণাঙ্ক
smbios_build_tables(সংরচনা romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *সংখ্যা_of_structures)
{
    সংরচনা smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    যদি (f_tables->size != *length)
        ঘূৰাই 0;

    qtables = malloc_tmphigh(f_tables->size);
    যদি (!qtables) {
        warn_noalloc();
        ঘূৰাই 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    বাবে (t0 = smbios_next(qtables, qtables_len, রিক্ত); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        যদি (t0->header.type == 0) {
            need_t0 = 0;
            ভঙ্গ;
        }
    }

    যদি (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = পরিমাণ(সংরচনা smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        যদি (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } অন্যথা {
            *length += t0_len;
            যদি (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            যদি (সংখ্যা_of_structures)
                (*সংখ্যা_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    যদি (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_উপৰেৰ(*length);
    অন্যথা
        tables = malloc_fseg(*length);
    যদি (!tables) {
        warn_noalloc();
        মুক্ত(qtables);
        ঘূৰাই 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    যদি (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    মুক্ত(qtables);
    ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক
smbios_21_setup_entry_point(সংরচনা romfile_s *f_tables,
                            সংরচনা smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    যদি (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->সংখ্যা_of_structures))
        ঘূৰাই 0;

    যদি ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        ঘূৰাই 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((শূন্য *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক
smbios_30_setup_entry_point(সংরচনা romfile_s *f_tables,
                            সংরচনা smbios_30_entry_point *ep)
{
    যদি (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             রিক্ত, রিক্ত))
        ঘূৰাই 0;

    ep->checksum -= checksum(ep, পরিমাণ(*ep));
    copy_smbios_30(ep);
    ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক
smbios_romfile_setup(শূন্য)
{
    সংরচনা romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    সংরচনা romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    সঙ্ঘ {
        সংরচনা smbios_21_entry_point ep21;
        সংরচনা smbios_30_entry_point ep30;
    } ep;

    যদি (!f_anchor || !f_tables || f_anchor->size > পরিমাণ(ep))
        ঘূৰাই 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    যদি (f_anchor->size == পরিমাণ(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        ঘূৰাই smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } অন্যথা যদি (f_anchor->size == পরিমাণ(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        ঘূৰাই smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } অন্যথা {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        ঘূৰাই 0;
    }
}

শূন্য
smbios_setup(শূন্য)
{
    যদি (smbios_romfile_setup())
        ঘূৰাই;
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
