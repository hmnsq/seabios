// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "byteorder.h" // le32_to_cpu
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "hw/pci.h" // pci_config_writeb
#அந்தர்பாவ "malloc.h" // malloc_fseg
#அந்தர்பாவ "memmap.h" //குறியீடு
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_find
#அந்தர்பாவ "std/acpi.h" // struct rsdp_descriptor
#அந்தர்பாவ "std/mptable.h" // MPTABLE_SIGNATURE
#அந்தர்பாவ "std/pirtable.h" // struct pir_header
#அந்தர்பாவ "std/smbios.h" // struct smbios_21_entry_point
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "util.h" // copy_table
#அந்தர்பாவ "x86.h" // outb

கட்டமைப்பு pir_header *PirAddr VARFSEG;

நிலையான வெறுமை *
copy_fseg_table(மாறிலி எழுத்து *name, வெறுமை *pos, u32 size)
{
    வெறுமை *newpos = malloc_fseg(size);
    எனில் (!newpos) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    திரும்பு newpos;
}

வெறுமை
copy_pir(வெறுமை *pos)
{
    கட்டமைப்பு pir_header *p = pos;
    எனில் (p->signature != PIR_SIGNATURE)
        திரும்பு;
    எனில் (PirAddr)
        திரும்பு;
    எனில் (p->size < அளவு(*p))
        திரும்பு;
    எனில் (checksum(pos, p->size) != 0)
        திரும்பு;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

வெறுமை
copy_mptable(வெறுமை *pos)
{
    கட்டமைப்பு mptable_floating_s *p = pos;
    எனில் (p->signature != MPTABLE_SIGNATURE)
        திரும்பு;
    எனில் (!p->physaddr)
        திரும்பு;
    எனில் (checksum(pos, அளவு(*p)) != 0)
        திரும்பு;
    u32 length = p->length * 16;
    u16 mpclength = ((கட்டமைப்பு mptable_config_s *)p->physaddr)->length;
    எனில் (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        திரும்பு;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    கட்டமைப்பு mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    எனில் (!newpos) {
        warn_noalloc();
        திரும்பு;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, அளவு(*newpos));
    memcpy((வெறுமை*)newpos + length, (வெறுமை*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

நிலையான முழு
get_acpi_rsdp_length(வெறுமை *pos, unsigned size)
{
    கட்டமைப்பு rsdp_descriptor *p = pos;
    எனில் (p->signature != RSDP_SIGNATURE)
        திரும்பு -1;
    u32 length = 20;
    எனில் (length > size)
        திரும்பு -1;
    எனில் (checksum(pos, length) != 0)
        திரும்பு -1;
    எனில் (p->revision > 1) {
        length = p->length;
        எனில் (length > size)
            திரும்பு -1;
        எனில் (checksum(pos, length) != 0)
            திரும்பு -1;
    }
    திரும்பு length;
}

கட்டமைப்பு rsdp_descriptor *RsdpAddr;

வெறுமை
copy_acpi_rsdp(வெறுமை *pos)
{
    எனில் (RsdpAddr)
        திரும்பு;
    முழு length = get_acpi_rsdp_length(pos, -1);
    எனில் (length < 0)
        திரும்பு;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

வெறுமை *find_acpi_rsdp(வெறுமை)
{
    unsigned long start = குறியீடு(zonefseg_start);
    unsigned long end = குறியீடு(zonefseg_end);
    unsigned long pos;

    க்கு (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        எனில் (get_acpi_rsdp_length((வெறுமை *)pos, end - pos) >= 0)
            திரும்பு (வெறுமை *)pos;

    திரும்பு வெற்று;
}

வெறுமை *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    எனில் (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        திரும்பு வெற்று;
    கட்டமைப்பு rsdt_descriptor_rev1 *rsdt = (வெறுமை*)RsdpAddr->rsdt_physical_address;
    கட்டமைப்பு xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? வெற்று : (வெறுமை*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    எனில் (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        வெறுமை *end = (வெறுமை*)xsdt + xsdt->length;
        முழு i;
        க்கு (i=0; (வெறுமை*)&xsdt->table_offset_entry[i] < end; i++) {
            எனில் (xsdt->table_offset_entry[i] >= 0x100000000)
                தொடர்; /* above 4G */
            கட்டமைப்பு acpi_table_header *tbl = (வெறுமை*)(u32)xsdt->table_offset_entry[i];
            எனில் (!tbl || tbl->signature != signature)
                தொடர்;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            திரும்பு tbl;
        }
    }

    எனில் (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        வெறுமை *end = (வெறுமை*)rsdt + rsdt->length;
        முழு i;
        க்கு (i=0; (வெறுமை*)&rsdt->table_offset_entry[i] < end; i++) {
            கட்டமைப்பு acpi_table_header *tbl = (வெறுமை*)rsdt->table_offset_entry[i];
            எனில் (!tbl || tbl->signature != signature)
                தொடர்;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            திரும்பு tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    திரும்பு வெற்று;
}

u32
find_resume_vector(வெறுமை)
{
    கட்டமைப்பு fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    எனில் (!fadt)
        திரும்பு 0;
    கட்டமைப்பு facs_descriptor_rev1 *facs = (வெறுமை*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    எனில் (! facs || facs->signature != FACS_SIGNATURE)
        திரும்பு 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    திரும்பு facs->firmware_waking_vector;
}

நிலையான கட்டமைப்பு acpi_20_generic_address acpi_reset_reg;
நிலையான u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

வெறுமை
acpi_reboot(வெறுமை)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    எனில் (acpi_reset_reg.register_bit_width != 8)
        திரும்பு;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    தேர்வு (acpi_reset_reg.address_space_id) {
    நிலை 0: // System Memory
        writeb((வெறுமை *)(u32)addr, acpi_reset_val);
        நிறுத்து;
    நிலை 1: // System I/O
        outb(acpi_reset_val, addr);
        நிறுத்து;
    நிலை 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        நிறுத்து;
    }
}

நிலையான வெறுமை
acpi_set_reset_reg(கட்டமைப்பு acpi_20_generic_address *reg, u8 val)
{
    எனில் (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        திரும்பு;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

வெறுமை
find_acpi_features(வெறுமை)
{
    கட்டமைப்பு fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    எனில் (!fadt)
        திரும்பு;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    எனில் (pm_tmr)
        pmtimer_setup(pm_tmr);
    எனில் (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    எனில் (fadt->length >= 129) {
        வெறுமை *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
வெறுமை *
smbios_next(வெறுமை *start, u32 length, வெறுமை *prev)
{
    எனில் (!start)
        திரும்பு வெற்று;
    வெறுமை *end = start + length;

    எனில் (!prev) {
        prev = start;
    } இல்லையெனில் {
        கட்டமைப்பு smbios_structure_header *hdr = prev;
        எனில் (prev + அளவு(*hdr) > end)
            திரும்பு வெற்று;
        prev += hdr->length + 2;
        போது (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    கட்டமைப்பு smbios_structure_header *hdr = prev;
    எனில் (prev >= end || prev + அளவு(*hdr) >= end || prev + hdr->length >= end)
        திரும்பு வெற்று;
    திரும்பு prev;
}

வெறுமை *
smbios_21_next(கட்டமைப்பு smbios_21_entry_point *smbios, வெறுமை *prev)
{
    எனில் (!smbios)
        திரும்பு வெற்று;
    திரும்பு smbios_next((வெறுமை*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

நிலையான கட்டமைப்பு smbios_21_entry_point *SMBios21Addr;

வெறுமை
copy_smbios_21(வெறுமை *pos)
{
    எனில் (SMBios21Addr)
        திரும்பு;
    கட்டமைப்பு smbios_21_entry_point *p = pos;
    எனில் (p->signature != SMBIOS_21_SIGNATURE)
        திரும்பு;
    எனில் (checksum(pos, 0x10) != 0)
        திரும்பு;
    எனில் (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        திரும்பு;
    எனில் (checksum(pos+0x10, p->length-0x10) != 0)
        திரும்பு;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

நிலையான கட்டமைப்பு smbios_30_entry_point *SMBios30Addr;

நிலையான முழு
valid_smbios_30_signature(கட்டமைப்பு smbios_30_entry_point *p)
{
    திரும்பு !memcmp(p->signature, "_SM3_", 5);
}

வெறுமை
copy_smbios_30(வெறுமை *pos)
{
    எனில் (SMBios30Addr)
        திரும்பு;
    கட்டமைப்பு smbios_30_entry_point *p = pos;
    எனில் (!valid_smbios_30_signature(p))
        திரும்பு;
    எனில் (checksum(pos, p->length) != 0)
        திரும்பு;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

வெறுமை *smbios_get_tables(u32 *length)
{
    எனில் (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        எனில் (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            திரும்பு (வெறுமை *)addr32;
        }
    }
    எனில் (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        திரும்பு (வெறுமை *)SMBios21Addr->structure_table_address;
    }
    திரும்பு வெற்று;
}

நிலையான முழு
smbios_major_version(வெறுமை)
{
    எனில் (SMBios30Addr)
        திரும்பு SMBios30Addr->smbios_major_version;
    இல்லையெனில் எனில் (SMBios21Addr)
        திரும்பு SMBios21Addr->smbios_major_version;
    இல்லையெனில்
        திரும்பு 0;
}

நிலையான முழு
smbios_minor_version(வெறுமை)
{
    எனில் (SMBios30Addr)
        திரும்பு SMBios30Addr->smbios_minor_version;
    இல்லையெனில் எனில் (SMBios21Addr)
        திரும்பு SMBios21Addr->smbios_minor_version;
    இல்லையெனில்
        திரும்பு 0;
}

வெறுமை
display_uuid(வெறுமை)
{
    u32 smbios_len = 0;
    வெறுமை *smbios_tables = smbios_get_tables(&smbios_len);
    கட்டமைப்பு smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, வெற்று);
    முழு minlen = offsetof(கட்டமைப்பு smbios_type_1, uuid) + அளவு(tbl->uuid);
    க்கு (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        எனில் (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[அளவு(tbl->uuid)] = { 0 };
            எனில் (memcmp(uuid, empty_uuid, அளவு(empty_uuid)) == 0)
                திரும்பு;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            எனில் (smbios_major_version() > 2 ||
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
            } இல்லையெனில் {
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

            திரும்பு;
        }
}

#define set_str_field_or_skip(type, field, மதிப்பு)                       \
    செய் {                                                                \
        முழு size = (மதிப்பு != வெற்று) ? strlen(மதிப்பு) + 1 : 0;             \
        எனில் (size > 1) {                                                 \
            memcpy(end, மதிப்பு, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } இல்லையெனில் {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } போது (0)

நிலையான வெறுமை *
smbios_new_type_0(வெறுமை *start,
                  மாறிலி எழுத்து *vendor, மாறிலி எழுத்து *version, மாறிலி எழுத்து *date)
{
    கட்டமைப்பு smbios_type_0 *p = (கட்டமைப்பு smbios_type_0 *)start;
    எழுத்து *end = (எழுத்து *)start + அளவு(கட்டமைப்பு smbios_type_0);
    முழு str_index = 0;

    p->header.type = 0;
    p->header.length = அளவு(கட்டமைப்பு smbios_type_0);
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
    எனில் (!str_index) {
        *end = 0;
        end++;
    }

    திரும்பு end;
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
நிலையான முழு
smbios_build_tables(கட்டமைப்பு romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *எண்_of_structures)
{
    கட்டமைப்பு smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    எனில் (f_tables->size != *length)
        திரும்பு 0;

    qtables = malloc_tmphigh(f_tables->size);
    எனில் (!qtables) {
        warn_noalloc();
        திரும்பு 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    க்கு (t0 = smbios_next(qtables, qtables_len, வெற்று); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        எனில் (t0->header.type == 0) {
            need_t0 = 0;
            நிறுத்து;
        }
    }

    எனில் (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = அளவு(கட்டமைப்பு smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        எனில் (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } இல்லையெனில் {
            *length += t0_len;
            எனில் (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            எனில் (எண்_of_structures)
                (*எண்_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    எனில் (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_உயர்வு(*length);
    இல்லையெனில்
        tables = malloc_fseg(*length);
    எனில் (!tables) {
        warn_noalloc();
        விடுவி(qtables);
        திரும்பு 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    எனில் (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    விடுவி(qtables);
    திரும்பு 1;
}

நிலையான முழு
smbios_21_setup_entry_point(கட்டமைப்பு romfile_s *f_tables,
                            கட்டமைப்பு smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    எனில் (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->எண்_of_structures))
        திரும்பு 0;

    எனில் ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        திரும்பு 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((வெறுமை *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    திரும்பு 1;
}

நிலையான முழு
smbios_30_setup_entry_point(கட்டமைப்பு romfile_s *f_tables,
                            கட்டமைப்பு smbios_30_entry_point *ep)
{
    எனில் (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             வெற்று, வெற்று))
        திரும்பு 0;

    ep->checksum -= checksum(ep, அளவு(*ep));
    copy_smbios_30(ep);
    திரும்பு 1;
}

நிலையான முழு
smbios_romfile_setup(வெறுமை)
{
    கட்டமைப்பு romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    கட்டமைப்பு romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    ஒன்றிப்பு {
        கட்டமைப்பு smbios_21_entry_point ep21;
        கட்டமைப்பு smbios_30_entry_point ep30;
    } ep;

    எனில் (!f_anchor || !f_tables || f_anchor->size > அளவு(ep))
        திரும்பு 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    எனில் (f_anchor->size == அளவு(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        திரும்பு smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } இல்லையெனில் எனில் (f_anchor->size == அளவு(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        திரும்பு smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } இல்லையெனில் {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        திரும்பு 0;
    }
}

வெறுமை
smbios_setup(வெறுமை)
{
    எனில் (smbios_romfile_setup())
        திரும்பு;
    smbios_legacy_setup();
}

வெறுமை
copy_table(வெறுமை *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
