// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "byteorder.h" // le32_to_cpu
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "hw/pci.h" // pci_config_writeb
#അന്തര്ഭാവ "malloc.h" // malloc_fseg
#അന്തര്ഭാവ "memmap.h" //ചിഹ്നം
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_find
#അന്തര്ഭാവ "std/acpi.h" // struct rsdp_descriptor
#അന്തര്ഭാവ "std/mptable.h" // MPTABLE_SIGNATURE
#അന്തര്ഭാവ "std/pirtable.h" // struct pir_header
#അന്തര്ഭാവ "std/smbios.h" // struct smbios_21_entry_point
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "util.h" // copy_table
#അന്തര്ഭാവ "x86.h" // outb

ഘടന pir_header *PirAddr VARFSEG;

സ്ഥിതം ശൂന്യം *
copy_fseg_table(സ്ഥിരം അക്ഷരം *name, ശൂന്യം *pos, u32 size)
{
    ശൂന്യം *newpos = malloc_fseg(size);
    എങ്കിൽ (!newpos) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    തിരികെ newpos;
}

ശൂന്യം
copy_pir(ശൂന്യം *pos)
{
    ഘടന pir_header *p = pos;
    എങ്കിൽ (p->signature != PIR_SIGNATURE)
        തിരികെ;
    എങ്കിൽ (PirAddr)
        തിരികെ;
    എങ്കിൽ (p->size < വലുപ്പം(*p))
        തിരികെ;
    എങ്കിൽ (checksum(pos, p->size) != 0)
        തിരികെ;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

ശൂന്യം
copy_mptable(ശൂന്യം *pos)
{
    ഘടന mptable_floating_s *p = pos;
    എങ്കിൽ (p->signature != MPTABLE_SIGNATURE)
        തിരികെ;
    എങ്കിൽ (!p->physaddr)
        തിരികെ;
    എങ്കിൽ (checksum(pos, വലുപ്പം(*p)) != 0)
        തിരികെ;
    u32 length = p->length * 16;
    u16 mpclength = ((ഘടന mptable_config_s *)p->physaddr)->length;
    എങ്കിൽ (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        തിരികെ;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    ഘടന mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    എങ്കിൽ (!newpos) {
        warn_noalloc();
        തിരികെ;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, വലുപ്പം(*newpos));
    memcpy((ശൂന്യം*)newpos + length, (ശൂന്യം*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

സ്ഥിതം പൂർണ്ണസംഖ്യ
get_acpi_rsdp_length(ശൂന്യം *pos, unsigned size)
{
    ഘടന rsdp_descriptor *p = pos;
    എങ്കിൽ (p->signature != RSDP_SIGNATURE)
        തിരികെ -1;
    u32 length = 20;
    എങ്കിൽ (length > size)
        തിരികെ -1;
    എങ്കിൽ (checksum(pos, length) != 0)
        തിരികെ -1;
    എങ്കിൽ (p->revision > 1) {
        length = p->length;
        എങ്കിൽ (length > size)
            തിരികെ -1;
        എങ്കിൽ (checksum(pos, length) != 0)
            തിരികെ -1;
    }
    തിരികെ length;
}

ഘടന rsdp_descriptor *RsdpAddr;

ശൂന്യം
copy_acpi_rsdp(ശൂന്യം *pos)
{
    എങ്കിൽ (RsdpAddr)
        തിരികെ;
    പൂർണ്ണസംഖ്യ length = get_acpi_rsdp_length(pos, -1);
    എങ്കിൽ (length < 0)
        തിരികെ;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

ശൂന്യം *find_acpi_rsdp(ശൂന്യം)
{
    unsigned long start = ചിഹ്നം(zonefseg_start);
    unsigned long end = ചിഹ്നം(zonefseg_end);
    unsigned long pos;

    വേണ്ടി (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        എങ്കിൽ (get_acpi_rsdp_length((ശൂന്യം *)pos, end - pos) >= 0)
            തിരികെ (ശൂന്യം *)pos;

    തിരികെ ശൂന്യ;
}

ശൂന്യം *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    എങ്കിൽ (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        തിരികെ ശൂന്യ;
    ഘടന rsdt_descriptor_rev1 *rsdt = (ശൂന്യം*)RsdpAddr->rsdt_physical_address;
    ഘടന xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? ശൂന്യ : (ശൂന്യം*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    എങ്കിൽ (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        ശൂന്യം *end = (ശൂന്യം*)xsdt + xsdt->length;
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i=0; (ശൂന്യം*)&xsdt->table_offset_entry[i] < end; i++) {
            എങ്കിൽ (xsdt->table_offset_entry[i] >= 0x100000000)
                തുടരുക; /* above 4G */
            ഘടന acpi_table_header *tbl = (ശൂന്യം*)(u32)xsdt->table_offset_entry[i];
            എങ്കിൽ (!tbl || tbl->signature != signature)
                തുടരുക;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            തിരികെ tbl;
        }
    }

    എങ്കിൽ (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        ശൂന്യം *end = (ശൂന്യം*)rsdt + rsdt->length;
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i=0; (ശൂന്യം*)&rsdt->table_offset_entry[i] < end; i++) {
            ഘടന acpi_table_header *tbl = (ശൂന്യം*)rsdt->table_offset_entry[i];
            എങ്കിൽ (!tbl || tbl->signature != signature)
                തുടരുക;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            തിരികെ tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    തിരികെ ശൂന്യ;
}

u32
find_resume_vector(ശൂന്യം)
{
    ഘടന fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    എങ്കിൽ (!fadt)
        തിരികെ 0;
    ഘടന facs_descriptor_rev1 *facs = (ശൂന്യം*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    എങ്കിൽ (! facs || facs->signature != FACS_SIGNATURE)
        തിരികെ 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    തിരികെ facs->firmware_waking_vector;
}

സ്ഥിതം ഘടന acpi_20_generic_address acpi_reset_reg;
സ്ഥിതം u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

ശൂന്യം
acpi_reboot(ശൂന്യം)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    എങ്കിൽ (acpi_reset_reg.register_bit_width != 8)
        തിരികെ;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    തിരഞ്ഞെടുപ്പ് (acpi_reset_reg.address_space_id) {
    അവസ്ഥ 0: // System Memory
        writeb((ശൂന്യം *)(u32)addr, acpi_reset_val);
        നിർത്തുക;
    അവസ്ഥ 1: // System I/O
        outb(acpi_reset_val, addr);
        നിർത്തുക;
    അവസ്ഥ 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        നിർത്തുക;
    }
}

സ്ഥിതം ശൂന്യം
acpi_set_reset_reg(ഘടന acpi_20_generic_address *reg, u8 val)
{
    എങ്കിൽ (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        തിരികെ;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

ശൂന്യം
find_acpi_features(ശൂന്യം)
{
    ഘടന fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    എങ്കിൽ (!fadt)
        തിരികെ;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    എങ്കിൽ (pm_tmr)
        pmtimer_setup(pm_tmr);
    എങ്കിൽ (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    എങ്കിൽ (fadt->length >= 129) {
        ശൂന്യം *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
ശൂന്യം *
smbios_next(ശൂന്യം *start, u32 length, ശൂന്യം *prev)
{
    എങ്കിൽ (!start)
        തിരികെ ശൂന്യ;
    ശൂന്യം *end = start + length;

    എങ്കിൽ (!prev) {
        prev = start;
    } അല്ലെങ്കിൽ {
        ഘടന smbios_structure_header *hdr = prev;
        എങ്കിൽ (prev + വലുപ്പം(*hdr) > end)
            തിരികെ ശൂന്യ;
        prev += hdr->length + 2;
        ആകുമ്പോൾ (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    ഘടന smbios_structure_header *hdr = prev;
    എങ്കിൽ (prev >= end || prev + വലുപ്പം(*hdr) >= end || prev + hdr->length >= end)
        തിരികെ ശൂന്യ;
    തിരികെ prev;
}

ശൂന്യം *
smbios_21_next(ഘടന smbios_21_entry_point *smbios, ശൂന്യം *prev)
{
    എങ്കിൽ (!smbios)
        തിരികെ ശൂന്യ;
    തിരികെ smbios_next((ശൂന്യം*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

സ്ഥിതം ഘടന smbios_21_entry_point *SMBios21Addr;

ശൂന്യം
copy_smbios_21(ശൂന്യം *pos)
{
    എങ്കിൽ (SMBios21Addr)
        തിരികെ;
    ഘടന smbios_21_entry_point *p = pos;
    എങ്കിൽ (p->signature != SMBIOS_21_SIGNATURE)
        തിരികെ;
    എങ്കിൽ (checksum(pos, 0x10) != 0)
        തിരികെ;
    എങ്കിൽ (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        തിരികെ;
    എങ്കിൽ (checksum(pos+0x10, p->length-0x10) != 0)
        തിരികെ;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

സ്ഥിതം ഘടന smbios_30_entry_point *SMBios30Addr;

സ്ഥിതം പൂർണ്ണസംഖ്യ
valid_smbios_30_signature(ഘടന smbios_30_entry_point *p)
{
    തിരികെ !memcmp(p->signature, "_SM3_", 5);
}

ശൂന്യം
copy_smbios_30(ശൂന്യം *pos)
{
    എങ്കിൽ (SMBios30Addr)
        തിരികെ;
    ഘടന smbios_30_entry_point *p = pos;
    എങ്കിൽ (!valid_smbios_30_signature(p))
        തിരികെ;
    എങ്കിൽ (checksum(pos, p->length) != 0)
        തിരികെ;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

ശൂന്യം *smbios_get_tables(u32 *length)
{
    എങ്കിൽ (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        എങ്കിൽ (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            തിരികെ (ശൂന്യം *)addr32;
        }
    }
    എങ്കിൽ (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        തിരികെ (ശൂന്യം *)SMBios21Addr->structure_table_address;
    }
    തിരികെ ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
smbios_major_version(ശൂന്യം)
{
    എങ്കിൽ (SMBios30Addr)
        തിരികെ SMBios30Addr->smbios_major_version;
    അല്ലെങ്കിൽ എങ്കിൽ (SMBios21Addr)
        തിരികെ SMBios21Addr->smbios_major_version;
    അല്ലെങ്കിൽ
        തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
smbios_minor_version(ശൂന്യം)
{
    എങ്കിൽ (SMBios30Addr)
        തിരികെ SMBios30Addr->smbios_minor_version;
    അല്ലെങ്കിൽ എങ്കിൽ (SMBios21Addr)
        തിരികെ SMBios21Addr->smbios_minor_version;
    അല്ലെങ്കിൽ
        തിരികെ 0;
}

ശൂന്യം
display_uuid(ശൂന്യം)
{
    u32 smbios_len = 0;
    ശൂന്യം *smbios_tables = smbios_get_tables(&smbios_len);
    ഘടന smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, ശൂന്യ);
    പൂർണ്ണസംഖ്യ minlen = offsetof(ഘടന smbios_type_1, uuid) + വലുപ്പം(tbl->uuid);
    വേണ്ടി (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        എങ്കിൽ (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[വലുപ്പം(tbl->uuid)] = { 0 };
            എങ്കിൽ (memcmp(uuid, empty_uuid, വലുപ്പം(empty_uuid)) == 0)
                തിരികെ;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            എങ്കിൽ (smbios_major_version() > 2 ||
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
            } അല്ലെങ്കിൽ {
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

            തിരികെ;
        }
}

#define set_str_field_or_skip(type, field, മൂല്യം)                       \
    ചെയ്യുക {                                                                \
        പൂർണ്ണസംഖ്യ size = (മൂല്യം != ശൂന്യ) ? strlen(മൂല്യം) + 1 : 0;             \
        എങ്കിൽ (size > 1) {                                                 \
            memcpy(end, മൂല്യം, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } അല്ലെങ്കിൽ {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } ആകുമ്പോൾ (0)

സ്ഥിതം ശൂന്യം *
smbios_new_type_0(ശൂന്യം *start,
                  സ്ഥിരം അക്ഷരം *vendor, സ്ഥിരം അക്ഷരം *version, സ്ഥിരം അക്ഷരം *date)
{
    ഘടന smbios_type_0 *p = (ഘടന smbios_type_0 *)start;
    അക്ഷരം *end = (അക്ഷരം *)start + വലുപ്പം(ഘടന smbios_type_0);
    പൂർണ്ണസംഖ്യ str_index = 0;

    p->header.type = 0;
    p->header.length = വലുപ്പം(ഘടന smbios_type_0);
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
    എങ്കിൽ (!str_index) {
        *end = 0;
        end++;
    }

    തിരികെ end;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
smbios_build_tables(ഘടന romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *സംഖ്യ_of_structures)
{
    ഘടന smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    എങ്കിൽ (f_tables->size != *length)
        തിരികെ 0;

    qtables = malloc_tmphigh(f_tables->size);
    എങ്കിൽ (!qtables) {
        warn_noalloc();
        തിരികെ 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    വേണ്ടി (t0 = smbios_next(qtables, qtables_len, ശൂന്യ); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        എങ്കിൽ (t0->header.type == 0) {
            need_t0 = 0;
            നിർത്തുക;
        }
    }

    എങ്കിൽ (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = വലുപ്പം(ഘടന smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        എങ്കിൽ (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } അല്ലെങ്കിൽ {
            *length += t0_len;
            എങ്കിൽ (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            എങ്കിൽ (സംഖ്യ_of_structures)
                (*സംഖ്യ_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    എങ്കിൽ (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_മുകളിലെ(*length);
    അല്ലെങ്കിൽ
        tables = malloc_fseg(*length);
    എങ്കിൽ (!tables) {
        warn_noalloc();
        സ്വതന്ത്രം(qtables);
        തിരികെ 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    എങ്കിൽ (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    സ്വതന്ത്രം(qtables);
    തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
smbios_21_setup_entry_point(ഘടന romfile_s *f_tables,
                            ഘടന smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    എങ്കിൽ (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->സംഖ്യ_of_structures))
        തിരികെ 0;

    എങ്കിൽ ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        തിരികെ 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((ശൂന്യം *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
smbios_30_setup_entry_point(ഘടന romfile_s *f_tables,
                            ഘടന smbios_30_entry_point *ep)
{
    എങ്കിൽ (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             ശൂന്യ, ശൂന്യ))
        തിരികെ 0;

    ep->checksum -= checksum(ep, വലുപ്പം(*ep));
    copy_smbios_30(ep);
    തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
smbios_romfile_setup(ശൂന്യം)
{
    ഘടന romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    ഘടന romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    സംഘം {
        ഘടന smbios_21_entry_point ep21;
        ഘടന smbios_30_entry_point ep30;
    } ep;

    എങ്കിൽ (!f_anchor || !f_tables || f_anchor->size > വലുപ്പം(ep))
        തിരികെ 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    എങ്കിൽ (f_anchor->size == വലുപ്പം(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        തിരികെ smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } അല്ലെങ്കിൽ എങ്കിൽ (f_anchor->size == വലുപ്പം(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        തിരികെ smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } അല്ലെങ്കിൽ {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        തിരികെ 0;
    }
}

ശൂന്യം
smbios_setup(ശൂന്യം)
{
    എങ്കിൽ (smbios_romfile_setup())
        തിരികെ;
    smbios_legacy_setup();
}

ശൂന്യം
copy_table(ശൂന്യം *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
