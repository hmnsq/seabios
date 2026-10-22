// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "byteorder.h" // le32_to_cpu
#ضمن "config.h" // CONFIG_*
#ضمن "hw/pci.h" // pci_config_writeb
#ضمن "malloc.h" // malloc_fseg
#ضمن "memmap.h" // SYMBOL
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_find
#ضمن "std/acpi.h" // struct rsdp_descriptor
#ضمن "std/mptable.h" // MPTABLE_SIGNATURE
#ضمن "std/pirtable.h" // struct pir_header
#ضمن "std/smbios.h" // struct smbios_21_entry_point
#ضمن "string.h" // memcpy
#ضمن "util.h" // copy_table
#ضمن "x86.h" // outb

بنية pir_header *PirAddr VARFSEG;

ساكن فراغ *
copy_fseg_table(ثابت حرف *name, فراغ *pos, u32 size)
{
    فراغ *newpos = malloc_fseg(size);
    אם (!newpos) {
        warn_noalloc();
        החזר عدم;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    החזר newpos;
}

فراغ
copy_pir(فراغ *pos)
{
    بنية pir_header *p = pos;
    אם (p->signature != PIR_SIGNATURE)
        החזר;
    אם (PirAddr)
        החזר;
    אם (p->size < حجم(*p))
        החזר;
    אם (checksum(pos, p->size) != 0)
        החזר;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

فراغ
copy_mptable(فراغ *pos)
{
    بنية mptable_floating_s *p = pos;
    אם (p->signature != MPTABLE_SIGNATURE)
        החזר;
    אם (!p->physaddr)
        החזר;
    אם (checksum(pos, حجم(*p)) != 0)
        החזר;
    u32 length = p->length * 16;
    u16 mpclength = ((بنية mptable_config_s *)p->physaddr)->length;
    אם (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        החזר;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    بنية mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    אם (!newpos) {
        warn_noalloc();
        החזר;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, حجم(*newpos));
    memcpy((فراغ*)newpos + length, (فراغ*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ساكن صحيح
get_acpi_rsdp_length(فراغ *pos, unsigned size)
{
    بنية rsdp_descriptor *p = pos;
    אם (p->signature != RSDP_SIGNATURE)
        החזר -1;
    u32 length = 20;
    אם (length > size)
        החזר -1;
    אם (checksum(pos, length) != 0)
        החזר -1;
    אם (p->revision > 1) {
        length = p->length;
        אם (length > size)
            החזר -1;
        אם (checksum(pos, length) != 0)
            החזר -1;
    }
    החזר length;
}

بنية rsdp_descriptor *RsdpAddr;

فراغ
copy_acpi_rsdp(فراغ *pos)
{
    אם (RsdpAddr)
        החזר;
    صحيح length = get_acpi_rsdp_length(pos, -1);
    אם (length < 0)
        החזר;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

فراغ *find_acpi_rsdp(فراغ)
{
    unsigned long start = SYMBOL(zonefseg_start);
    unsigned long end = SYMBOL(zonefseg_end);
    unsigned long pos;

    עבור (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        אם (get_acpi_rsdp_length((فراغ *)pos, end - pos) >= 0)
            החזר (فراغ *)pos;

    החזר عدم;
}

فراغ *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    אם (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        החזר عدم;
    بنية rsdt_descriptor_rev1 *rsdt = (فراغ*)RsdpAddr->rsdt_physical_address;
    بنية xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? عدم : (فراغ*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    אם (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        فراغ *end = (فراغ*)xsdt + xsdt->length;
        صحيح i;
        עבור (i=0; (فراغ*)&xsdt->table_offset_entry[i] < end; i++) {
            אם (xsdt->table_offset_entry[i] >= 0x100000000)
                استمر; /* above 4G */
            بنية acpi_table_header *tbl = (فراغ*)(u32)xsdt->table_offset_entry[i];
            אם (!tbl || tbl->signature != signature)
                استمر;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            החזר tbl;
        }
    }

    אם (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        فراغ *end = (فراغ*)rsdt + rsdt->length;
        صحيح i;
        עבור (i=0; (فراغ*)&rsdt->table_offset_entry[i] < end; i++) {
            بنية acpi_table_header *tbl = (فراغ*)rsdt->table_offset_entry[i];
            אם (!tbl || tbl->signature != signature)
                استمر;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            החזר tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    החזר عدم;
}

u32
find_resume_vector(فراغ)
{
    بنية fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    אם (!fadt)
        החזר 0;
    بنية facs_descriptor_rev1 *facs = (فراغ*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    אם (! facs || facs->signature != FACS_SIGNATURE)
        החזר 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    החזר facs->firmware_waking_vector;
}

ساكن بنية acpi_20_generic_address acpi_reset_reg;
ساكن u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

فراغ
acpi_reboot(فراغ)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    אם (acpi_reset_reg.register_bit_width != 8)
        החזר;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    بدل (acpi_reset_reg.address_space_id) {
    حالة 0: // System Memory
        writeb((فراغ *)(u32)addr, acpi_reset_val);
        اكسر;
    حالة 1: // System I/O
        outb(acpi_reset_val, addr);
        اكسر;
    حالة 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        اكسر;
    }
}

ساكن فراغ
acpi_set_reset_reg(بنية acpi_20_generic_address *reg, u8 val)
{
    אם (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        החזר;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

فراغ
find_acpi_features(فراغ)
{
    بنية fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    אם (!fadt)
        החזר;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    אם (pm_tmr)
        pmtimer_setup(pm_tmr);
    אם (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    אם (fadt->length >= 129) {
        فراغ *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
فراغ *
smbios_next(فراغ *start, u32 length, فراغ *prev)
{
    אם (!start)
        החזר عدم;
    فراغ *end = start + length;

    אם (!prev) {
        prev = start;
    } אחרת {
        بنية smbios_structure_header *hdr = prev;
        אם (prev + حجم(*hdr) > end)
            החזר عدم;
        prev += hdr->length + 2;
        כלעוד (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    بنية smbios_structure_header *hdr = prev;
    אם (prev >= end || prev + حجم(*hdr) >= end || prev + hdr->length >= end)
        החזר عدم;
    החזר prev;
}

فراغ *
smbios_21_next(بنية smbios_21_entry_point *smbios, فراغ *prev)
{
    אם (!smbios)
        החזר عدم;
    החזר smbios_next((فراغ*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

ساكن بنية smbios_21_entry_point *SMBios21Addr;

فراغ
copy_smbios_21(فراغ *pos)
{
    אם (SMBios21Addr)
        החזר;
    بنية smbios_21_entry_point *p = pos;
    אם (p->signature != SMBIOS_21_SIGNATURE)
        החזר;
    אם (checksum(pos, 0x10) != 0)
        החזר;
    אם (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        החזר;
    אם (checksum(pos+0x10, p->length-0x10) != 0)
        החזר;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

ساكن بنية smbios_30_entry_point *SMBios30Addr;

ساكن صحيح
valid_smbios_30_signature(بنية smbios_30_entry_point *p)
{
    החזר !memcmp(p->signature, "_SM3_", 5);
}

فراغ
copy_smbios_30(فراغ *pos)
{
    אם (SMBios30Addr)
        החזר;
    بنية smbios_30_entry_point *p = pos;
    אם (!valid_smbios_30_signature(p))
        החזר;
    אם (checksum(pos, p->length) != 0)
        החזר;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

فراغ *smbios_get_tables(u32 *length)
{
    אם (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        אם (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            החזר (فراغ *)addr32;
        }
    }
    אם (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        החזר (فراغ *)SMBios21Addr->structure_table_address;
    }
    החזר عدم;
}

ساكن صحيح
smbios_major_version(فراغ)
{
    אם (SMBios30Addr)
        החזר SMBios30Addr->smbios_major_version;
    אחרת אם (SMBios21Addr)
        החזר SMBios21Addr->smbios_major_version;
    אחרת
        החזר 0;
}

ساكن صحيح
smbios_minor_version(فراغ)
{
    אם (SMBios30Addr)
        החזר SMBios30Addr->smbios_minor_version;
    אחרת אם (SMBios21Addr)
        החזר SMBios21Addr->smbios_minor_version;
    אחרת
        החזר 0;
}

فراغ
display_uuid(فراغ)
{
    u32 smbios_len = 0;
    فراغ *smbios_tables = smbios_get_tables(&smbios_len);
    بنية smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, عدم);
    صحيح minlen = offsetof(بنية smbios_type_1, uuid) + حجم(tbl->uuid);
    עבור (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        אם (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[حجم(tbl->uuid)] = { 0 };
            אם (memcmp(uuid, empty_uuid, حجم(empty_uuid)) == 0)
                החזר;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            אם (smbios_major_version() > 2 ||
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
            } אחרת {
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

            החזר;
        }
}

#define set_str_field_or_skip(type, field, value)                       \
    עשה {                                                                \
        صحيح size = (value != عدم) ? strlen(value) + 1 : 0;             \
        אם (size > 1) {                                                 \
            memcpy(end, value, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } אחרת {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } כלעוד (0)

ساكن فراغ *
smbios_new_type_0(فراغ *start,
                  ثابت حرف *vendor, ثابت حرف *version, ثابت حرف *date)
{
    بنية smbios_type_0 *p = (بنية smbios_type_0 *)start;
    حرف *end = (حرف *)start + حجم(بنية smbios_type_0);
    صحيح str_index = 0;

    p->header.type = 0;
    p->header.length = حجم(بنية smbios_type_0);
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
    אם (!str_index) {
        *end = 0;
        end++;
    }

    החזר end;
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
ساكن صحيح
smbios_build_tables(بنية romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *number_of_structures)
{
    بنية smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    אם (f_tables->size != *length)
        החזר 0;

    qtables = malloc_tmphigh(f_tables->size);
    אם (!qtables) {
        warn_noalloc();
        החזר 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    עבור (t0 = smbios_next(qtables, qtables_len, عدم); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        אם (t0->header.type == 0) {
            need_t0 = 0;
            اكسر;
        }
    }

    אם (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = حجم(بنية smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        אם (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } אחרת {
            *length += t0_len;
            אם (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            אם (number_of_structures)
                (*number_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    אם (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_high(*length);
    אחרת
        tables = malloc_fseg(*length);
    אם (!tables) {
        warn_noalloc();
        حرر(qtables);
        החזר 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    אם (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    حرر(qtables);
    החזר 1;
}

ساكن صحيح
smbios_21_setup_entry_point(بنية romfile_s *f_tables,
                            بنية smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    אם (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->number_of_structures))
        החזר 0;

    אם ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        החזר 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((فراغ *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    החזר 1;
}

ساكن صحيح
smbios_30_setup_entry_point(بنية romfile_s *f_tables,
                            بنية smbios_30_entry_point *ep)
{
    אם (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             عدم, عدم))
        החזר 0;

    ep->checksum -= checksum(ep, حجم(*ep));
    copy_smbios_30(ep);
    החזר 1;
}

ساكن صحيح
smbios_romfile_setup(فراغ)
{
    بنية romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    بنية romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    اتحاد {
        بنية smbios_21_entry_point ep21;
        بنية smbios_30_entry_point ep30;
    } ep;

    אם (!f_anchor || !f_tables || f_anchor->size > حجم(ep))
        החזר 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    אם (f_anchor->size == حجم(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        החזר smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } אחרת אם (f_anchor->size == حجم(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        החזר smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } אחרת {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        החזר 0;
    }
}

فراغ
smbios_setup(فراغ)
{
    אם (smbios_romfile_setup())
        החזר;
    smbios_legacy_setup();
}

فراغ
copy_table(فراغ *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
