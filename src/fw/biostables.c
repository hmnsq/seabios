// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "byteorder.h" // le32_to_cpu
#شامل "config.h" // CONFIG_*
#شامل "hw/pci.h" // pci_config_writeb
#شامل "malloc.h" // malloc_fseg
#شامل "memmap.h" //نماد
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_find
#شامل "std/acpi.h" // struct rsdp_descriptor
#شامل "std/mptable.h" // MPTABLE_SIGNATURE
#شامل "std/pirtable.h" // struct pir_header
#شامل "std/smbios.h" // struct smbios_21_entry_point
#شامل "string.h" // memcpy
#شامل "util.h" // copy_table
#شامل "x86.h" // outb

ساختار pir_header *PirAddr VARFSEG;

ایستا تهی *
copy_fseg_table(ثابت نویسه *name, تهی *pos, u32 size)
{
    تهی *newpos = malloc_fseg(size);
    اگر (!newpos) {
        warn_noalloc();
        بازگشت پوچ;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    بازگشت newpos;
}

تهی
copy_pir(تهی *pos)
{
    ساختار pir_header *p = pos;
    اگر (p->signature != PIR_SIGNATURE)
        بازگشت;
    اگر (PirAddr)
        بازگشت;
    اگر (p->size < اندازه(*p))
        بازگشت;
    اگر (checksum(pos, p->size) != 0)
        بازگشت;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

تهی
copy_mptable(تهی *pos)
{
    ساختار mptable_floating_s *p = pos;
    اگر (p->signature != MPTABLE_SIGNATURE)
        بازگشت;
    اگر (!p->physaddr)
        بازگشت;
    اگر (checksum(pos, اندازه(*p)) != 0)
        بازگشت;
    u32 length = p->length * 16;
    u16 mpclength = ((ساختار mptable_config_s *)p->physaddr)->length;
    اگر (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        بازگشت;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    ساختار mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    اگر (!newpos) {
        warn_noalloc();
        بازگشت;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, اندازه(*newpos));
    memcpy((تهی*)newpos + length, (تهی*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ایستا صحیح
get_acpi_rsdp_length(تهی *pos, unsigned size)
{
    ساختار rsdp_descriptor *p = pos;
    اگر (p->signature != RSDP_SIGNATURE)
        بازگشت -1;
    u32 length = 20;
    اگر (length > size)
        بازگشت -1;
    اگر (checksum(pos, length) != 0)
        بازگشت -1;
    اگر (p->revision > 1) {
        length = p->length;
        اگر (length > size)
            بازگشت -1;
        اگر (checksum(pos, length) != 0)
            بازگشت -1;
    }
    بازگشت length;
}

ساختار rsdp_descriptor *RsdpAddr;

تهی
copy_acpi_rsdp(تهی *pos)
{
    اگر (RsdpAddr)
        بازگشت;
    صحیح length = get_acpi_rsdp_length(pos, -1);
    اگر (length < 0)
        بازگشت;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

تهی *find_acpi_rsdp(تهی)
{
    unsigned long start = نماد(zonefseg_start);
    unsigned long end = نماد(zonefseg_end);
    unsigned long pos;

    برای (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        اگر (get_acpi_rsdp_length((تهی *)pos, end - pos) >= 0)
            بازگشت (تهی *)pos;

    بازگشت پوچ;
}

تهی *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    اگر (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        بازگشت پوچ;
    ساختار rsdt_descriptor_rev1 *rsdt = (تهی*)RsdpAddr->rsdt_physical_address;
    ساختار xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? پوچ : (تهی*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    اگر (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        تهی *end = (تهی*)xsdt + xsdt->length;
        صحیح i;
        برای (i=0; (تهی*)&xsdt->table_offset_entry[i] < end; i++) {
            اگر (xsdt->table_offset_entry[i] >= 0x100000000)
                ادامه; /* above 4G */
            ساختار acpi_table_header *tbl = (تهی*)(u32)xsdt->table_offset_entry[i];
            اگر (!tbl || tbl->signature != signature)
                ادامه;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            بازگشت tbl;
        }
    }

    اگر (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        تهی *end = (تهی*)rsdt + rsdt->length;
        صحیح i;
        برای (i=0; (تهی*)&rsdt->table_offset_entry[i] < end; i++) {
            ساختار acpi_table_header *tbl = (تهی*)rsdt->table_offset_entry[i];
            اگر (!tbl || tbl->signature != signature)
                ادامه;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            بازگشت tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    بازگشت پوچ;
}

u32
find_resume_vector(تهی)
{
    ساختار fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    اگر (!fadt)
        بازگشت 0;
    ساختار facs_descriptor_rev1 *facs = (تهی*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    اگر (! facs || facs->signature != FACS_SIGNATURE)
        بازگشت 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    بازگشت facs->firmware_waking_vector;
}

ایستا ساختار acpi_20_generic_address acpi_reset_reg;
ایستا u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

تهی
acpi_reboot(تهی)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    اگر (acpi_reset_reg.register_bit_width != 8)
        بازگشت;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    انتخاب (acpi_reset_reg.address_space_id) {
    حالت 0: // System Memory
        writeb((تهی *)(u32)addr, acpi_reset_val);
        بشکن;
    حالت 1: // System I/O
        outb(acpi_reset_val, addr);
        بشکن;
    حالت 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        بشکن;
    }
}

ایستا تهی
acpi_set_reset_reg(ساختار acpi_20_generic_address *reg, u8 val)
{
    اگر (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        بازگشت;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

تهی
find_acpi_features(تهی)
{
    ساختار fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    اگر (!fadt)
        بازگشت;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    اگر (pm_tmr)
        pmtimer_setup(pm_tmr);
    اگر (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    اگر (fadt->length >= 129) {
        تهی *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
تهی *
smbios_next(تهی *start, u32 length, تهی *prev)
{
    اگر (!start)
        بازگشت پوچ;
    تهی *end = start + length;

    اگر (!prev) {
        prev = start;
    } وگرنه {
        ساختار smbios_structure_header *hdr = prev;
        اگر (prev + اندازه(*hdr) > end)
            بازگشت پوچ;
        prev += hdr->length + 2;
        وقتی (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    ساختار smbios_structure_header *hdr = prev;
    اگر (prev >= end || prev + اندازه(*hdr) >= end || prev + hdr->length >= end)
        بازگشت پوچ;
    بازگشت prev;
}

تهی *
smbios_21_next(ساختار smbios_21_entry_point *smbios, تهی *prev)
{
    اگر (!smbios)
        بازگشت پوچ;
    بازگشت smbios_next((تهی*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

ایستا ساختار smbios_21_entry_point *SMBios21Addr;

تهی
copy_smbios_21(تهی *pos)
{
    اگر (SMBios21Addr)
        بازگشت;
    ساختار smbios_21_entry_point *p = pos;
    اگر (p->signature != SMBIOS_21_SIGNATURE)
        بازگشت;
    اگر (checksum(pos, 0x10) != 0)
        بازگشت;
    اگر (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        بازگشت;
    اگر (checksum(pos+0x10, p->length-0x10) != 0)
        بازگشت;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

ایستا ساختار smbios_30_entry_point *SMBios30Addr;

ایستا صحیح
valid_smbios_30_signature(ساختار smbios_30_entry_point *p)
{
    بازگشت !memcmp(p->signature, "_SM3_", 5);
}

تهی
copy_smbios_30(تهی *pos)
{
    اگر (SMBios30Addr)
        بازگشت;
    ساختار smbios_30_entry_point *p = pos;
    اگر (!valid_smbios_30_signature(p))
        بازگشت;
    اگر (checksum(pos, p->length) != 0)
        بازگشت;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

تهی *smbios_get_tables(u32 *length)
{
    اگر (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        اگر (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            بازگشت (تهی *)addr32;
        }
    }
    اگر (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        بازگشت (تهی *)SMBios21Addr->structure_table_address;
    }
    بازگشت پوچ;
}

ایستا صحیح
smbios_major_version(تهی)
{
    اگر (SMBios30Addr)
        بازگشت SMBios30Addr->smbios_major_version;
    وگرنه اگر (SMBios21Addr)
        بازگشت SMBios21Addr->smbios_major_version;
    وگرنه
        بازگشت 0;
}

ایستا صحیح
smbios_minor_version(تهی)
{
    اگر (SMBios30Addr)
        بازگشت SMBios30Addr->smbios_minor_version;
    وگرنه اگر (SMBios21Addr)
        بازگشت SMBios21Addr->smbios_minor_version;
    وگرنه
        بازگشت 0;
}

تهی
display_uuid(تهی)
{
    u32 smbios_len = 0;
    تهی *smbios_tables = smbios_get_tables(&smbios_len);
    ساختار smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, پوچ);
    صحیح minlen = offsetof(ساختار smbios_type_1, uuid) + اندازه(tbl->uuid);
    برای (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        اگر (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[اندازه(tbl->uuid)] = { 0 };
            اگر (memcmp(uuid, empty_uuid, اندازه(empty_uuid)) == 0)
                بازگشت;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            اگر (smbios_major_version() > 2 ||
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
            } وگرنه {
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

            بازگشت;
        }
}

#define set_str_field_or_skip(type, field, مقدار)                       \
    انجامبده {                                                                \
        صحیح size = (مقدار != پوچ) ? strlen(مقدار) + 1 : 0;             \
        اگر (size > 1) {                                                 \
            memcpy(end, مقدار, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } وگرنه {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } وقتی (0)

ایستا تهی *
smbios_new_type_0(تهی *start,
                  ثابت نویسه *vendor, ثابت نویسه *version, ثابت نویسه *date)
{
    ساختار smbios_type_0 *p = (ساختار smbios_type_0 *)start;
    نویسه *end = (نویسه *)start + اندازه(ساختار smbios_type_0);
    صحیح str_index = 0;

    p->header.type = 0;
    p->header.length = اندازه(ساختار smbios_type_0);
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
    اگر (!str_index) {
        *end = 0;
        end++;
    }

    بازگشت end;
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
ایستا صحیح
smbios_build_tables(ساختار romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *عدد_of_structures)
{
    ساختار smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    اگر (f_tables->size != *length)
        بازگشت 0;

    qtables = malloc_tmphigh(f_tables->size);
    اگر (!qtables) {
        warn_noalloc();
        بازگشت 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    برای (t0 = smbios_next(qtables, qtables_len, پوچ); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        اگر (t0->header.type == 0) {
            need_t0 = 0;
            بشکن;
        }
    }

    اگر (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = اندازه(ساختار smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        اگر (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } وگرنه {
            *length += t0_len;
            اگر (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            اگر (عدد_of_structures)
                (*عدد_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    اگر (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_بالا(*length);
    وگرنه
        tables = malloc_fseg(*length);
    اگر (!tables) {
        warn_noalloc();
        آزاد(qtables);
        بازگشت 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    اگر (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    آزاد(qtables);
    بازگشت 1;
}

ایستا صحیح
smbios_21_setup_entry_point(ساختار romfile_s *f_tables,
                            ساختار smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    اگر (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->عدد_of_structures))
        بازگشت 0;

    اگر ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        بازگشت 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((تهی *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    بازگشت 1;
}

ایستا صحیح
smbios_30_setup_entry_point(ساختار romfile_s *f_tables,
                            ساختار smbios_30_entry_point *ep)
{
    اگر (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             پوچ, پوچ))
        بازگشت 0;

    ep->checksum -= checksum(ep, اندازه(*ep));
    copy_smbios_30(ep);
    بازگشت 1;
}

ایستا صحیح
smbios_romfile_setup(تهی)
{
    ساختار romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    ساختار romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    اجتماع {
        ساختار smbios_21_entry_point ep21;
        ساختار smbios_30_entry_point ep30;
    } ep;

    اگر (!f_anchor || !f_tables || f_anchor->size > اندازه(ep))
        بازگشت 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    اگر (f_anchor->size == اندازه(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        بازگشت smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } وگرنه اگر (f_anchor->size == اندازه(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        بازگشت smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } وگرنه {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        بازگشت 0;
    }
}

تهی
smbios_setup(تهی)
{
    اگر (smbios_romfile_setup())
        بازگشت;
    smbios_legacy_setup();
}

تهی
copy_table(تهی *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
