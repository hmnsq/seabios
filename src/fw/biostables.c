// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "byteorder.h" // le32_to_cpu
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "hw/pci.h" // pci_config_writeb
#અન્તર્ભાવ "malloc.h" // malloc_fseg
#અન્તર્ભાવ "memmap.h" //પ્રતીક
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_find
#અન્તર્ભાવ "std/acpi.h" // struct rsdp_descriptor
#અન્તર્ભાવ "std/mptable.h" // MPTABLE_SIGNATURE
#અન્તર્ભાવ "std/pirtable.h" // struct pir_header
#અન્તર્ભાવ "std/smbios.h" // struct smbios_21_entry_point
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "util.h" // copy_table
#અન્તર્ભાવ "x86.h" // outb

સંરચના pir_header *PirAddr VARFSEG;

સ્થિર શૂન્ય *
copy_fseg_table(અચલ અક્ષર *name, શૂન્ય *pos, u32 size)
{
    શૂન્ય *newpos = malloc_fseg(size);
    જો (!newpos) {
        warn_noalloc();
        પાછા ખાલી;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    પાછા newpos;
}

શૂન્ય
copy_pir(શૂન્ય *pos)
{
    સંરચના pir_header *p = pos;
    જો (p->signature != PIR_SIGNATURE)
        પાછા;
    જો (PirAddr)
        પાછા;
    જો (p->size < કદ(*p))
        પાછા;
    જો (checksum(pos, p->size) != 0)
        પાછા;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

શૂન્ય
copy_mptable(શૂન્ય *pos)
{
    સંરચના mptable_floating_s *p = pos;
    જો (p->signature != MPTABLE_SIGNATURE)
        પાછા;
    જો (!p->physaddr)
        પાછા;
    જો (checksum(pos, કદ(*p)) != 0)
        પાછા;
    u32 length = p->length * 16;
    u16 mpclength = ((સંરચના mptable_config_s *)p->physaddr)->length;
    જો (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        પાછા;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    સંરચના mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    જો (!newpos) {
        warn_noalloc();
        પાછા;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, કદ(*newpos));
    memcpy((શૂન્ય*)newpos + length, (શૂન્ય*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

સ્થિર પૂર્ણાંક
get_acpi_rsdp_length(શૂન્ય *pos, unsigned size)
{
    સંરચના rsdp_descriptor *p = pos;
    જો (p->signature != RSDP_SIGNATURE)
        પાછા -1;
    u32 length = 20;
    જો (length > size)
        પાછા -1;
    જો (checksum(pos, length) != 0)
        પાછા -1;
    જો (p->revision > 1) {
        length = p->length;
        જો (length > size)
            પાછા -1;
        જો (checksum(pos, length) != 0)
            પાછા -1;
    }
    પાછા length;
}

સંરચના rsdp_descriptor *RsdpAddr;

શૂન્ય
copy_acpi_rsdp(શૂન્ય *pos)
{
    જો (RsdpAddr)
        પાછા;
    પૂર્ણાંક length = get_acpi_rsdp_length(pos, -1);
    જો (length < 0)
        પાછા;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

શૂન્ય *find_acpi_rsdp(શૂન્ય)
{
    unsigned long start = પ્રતીક(zonefseg_start);
    unsigned long end = પ્રતીક(zonefseg_end);
    unsigned long pos;

    માટે (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        જો (get_acpi_rsdp_length((શૂન્ય *)pos, end - pos) >= 0)
            પાછા (શૂન્ય *)pos;

    પાછા ખાલી;
}

શૂન્ય *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    જો (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        પાછા ખાલી;
    સંરચના rsdt_descriptor_rev1 *rsdt = (શૂન્ય*)RsdpAddr->rsdt_physical_address;
    સંરચના xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? ખાલી : (શૂન્ય*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    જો (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        શૂન્ય *end = (શૂન્ય*)xsdt + xsdt->length;
        પૂર્ણાંક i;
        માટે (i=0; (શૂન્ય*)&xsdt->table_offset_entry[i] < end; i++) {
            જો (xsdt->table_offset_entry[i] >= 0x100000000)
                ચાલુ; /* above 4G */
            સંરચના acpi_table_header *tbl = (શૂન્ય*)(u32)xsdt->table_offset_entry[i];
            જો (!tbl || tbl->signature != signature)
                ચાલુ;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            પાછા tbl;
        }
    }

    જો (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        શૂન્ય *end = (શૂન્ય*)rsdt + rsdt->length;
        પૂર્ણાંક i;
        માટે (i=0; (શૂન્ય*)&rsdt->table_offset_entry[i] < end; i++) {
            સંરચના acpi_table_header *tbl = (શૂન્ય*)rsdt->table_offset_entry[i];
            જો (!tbl || tbl->signature != signature)
                ચાલુ;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            પાછા tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    પાછા ખાલી;
}

u32
find_resume_vector(શૂન્ય)
{
    સંરચના fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    જો (!fadt)
        પાછા 0;
    સંરચના facs_descriptor_rev1 *facs = (શૂન્ય*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    જો (! facs || facs->signature != FACS_SIGNATURE)
        પાછા 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    પાછા facs->firmware_waking_vector;
}

સ્થિર સંરચના acpi_20_generic_address acpi_reset_reg;
સ્થિર u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

શૂન્ય
acpi_reboot(શૂન્ય)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    જો (acpi_reset_reg.register_bit_width != 8)
        પાછા;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    પસંદગી (acpi_reset_reg.address_space_id) {
    સ્થિતિ 0: // System Memory
        writeb((શૂન્ય *)(u32)addr, acpi_reset_val);
        તોડો;
    સ્થિતિ 1: // System I/O
        outb(acpi_reset_val, addr);
        તોડો;
    સ્થિતિ 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        તોડો;
    }
}

સ્થિર શૂન્ય
acpi_set_reset_reg(સંરચના acpi_20_generic_address *reg, u8 val)
{
    જો (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        પાછા;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

શૂન્ય
find_acpi_features(શૂન્ય)
{
    સંરચના fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    જો (!fadt)
        પાછા;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    જો (pm_tmr)
        pmtimer_setup(pm_tmr);
    જો (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    જો (fadt->length >= 129) {
        શૂન્ય *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
શૂન્ય *
smbios_next(શૂન્ય *start, u32 length, શૂન્ય *prev)
{
    જો (!start)
        પાછા ખાલી;
    શૂન્ય *end = start + length;

    જો (!prev) {
        prev = start;
    } નહીંતર {
        સંરચના smbios_structure_header *hdr = prev;
        જો (prev + કદ(*hdr) > end)
            પાછા ખાલી;
        prev += hdr->length + 2;
        જ્યારે (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    સંરચના smbios_structure_header *hdr = prev;
    જો (prev >= end || prev + કદ(*hdr) >= end || prev + hdr->length >= end)
        પાછા ખાલી;
    પાછા prev;
}

શૂન્ય *
smbios_21_next(સંરચના smbios_21_entry_point *smbios, શૂન્ય *prev)
{
    જો (!smbios)
        પાછા ખાલી;
    પાછા smbios_next((શૂન્ય*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

સ્થિર સંરચના smbios_21_entry_point *SMBios21Addr;

શૂન્ય
copy_smbios_21(શૂન્ય *pos)
{
    જો (SMBios21Addr)
        પાછા;
    સંરચના smbios_21_entry_point *p = pos;
    જો (p->signature != SMBIOS_21_SIGNATURE)
        પાછા;
    જો (checksum(pos, 0x10) != 0)
        પાછા;
    જો (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        પાછા;
    જો (checksum(pos+0x10, p->length-0x10) != 0)
        પાછા;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

સ્થિર સંરચના smbios_30_entry_point *SMBios30Addr;

સ્થિર પૂર્ણાંક
valid_smbios_30_signature(સંરચના smbios_30_entry_point *p)
{
    પાછા !memcmp(p->signature, "_SM3_", 5);
}

શૂન્ય
copy_smbios_30(શૂન્ય *pos)
{
    જો (SMBios30Addr)
        પાછા;
    સંરચના smbios_30_entry_point *p = pos;
    જો (!valid_smbios_30_signature(p))
        પાછા;
    જો (checksum(pos, p->length) != 0)
        પાછા;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

શૂન્ય *smbios_get_tables(u32 *length)
{
    જો (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        જો (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            પાછા (શૂન્ય *)addr32;
        }
    }
    જો (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        પાછા (શૂન્ય *)SMBios21Addr->structure_table_address;
    }
    પાછા ખાલી;
}

સ્થિર પૂર્ણાંક
smbios_major_version(શૂન્ય)
{
    જો (SMBios30Addr)
        પાછા SMBios30Addr->smbios_major_version;
    નહીંતર જો (SMBios21Addr)
        પાછા SMBios21Addr->smbios_major_version;
    નહીંતર
        પાછા 0;
}

સ્થિર પૂર્ણાંક
smbios_minor_version(શૂન્ય)
{
    જો (SMBios30Addr)
        પાછા SMBios30Addr->smbios_minor_version;
    નહીંતર જો (SMBios21Addr)
        પાછા SMBios21Addr->smbios_minor_version;
    નહીંતર
        પાછા 0;
}

શૂન્ય
display_uuid(શૂન્ય)
{
    u32 smbios_len = 0;
    શૂન્ય *smbios_tables = smbios_get_tables(&smbios_len);
    સંરચના smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, ખાલી);
    પૂર્ણાંક minlen = offsetof(સંરચના smbios_type_1, uuid) + કદ(tbl->uuid);
    માટે (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        જો (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[કદ(tbl->uuid)] = { 0 };
            જો (memcmp(uuid, empty_uuid, કદ(empty_uuid)) == 0)
                પાછા;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            જો (smbios_major_version() > 2 ||
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
            } નહીંતર {
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

            પાછા;
        }
}

#define set_str_field_or_skip(type, field, કિંમત)                       \
    કરો {                                                                \
        પૂર્ણાંક size = (કિંમત != ખાલી) ? strlen(કિંમત) + 1 : 0;             \
        જો (size > 1) {                                                 \
            memcpy(end, કિંમત, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } નહીંતર {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } જ્યારે (0)

સ્થિર શૂન્ય *
smbios_new_type_0(શૂન્ય *start,
                  અચલ અક્ષર *vendor, અચલ અક્ષર *version, અચલ અક્ષર *date)
{
    સંરચના smbios_type_0 *p = (સંરચના smbios_type_0 *)start;
    અક્ષર *end = (અક્ષર *)start + કદ(સંરચના smbios_type_0);
    પૂર્ણાંક str_index = 0;

    p->header.type = 0;
    p->header.length = કદ(સંરચના smbios_type_0);
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
    જો (!str_index) {
        *end = 0;
        end++;
    }

    પાછા end;
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
સ્થિર પૂર્ણાંક
smbios_build_tables(સંરચના romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *સંખ્યા_of_structures)
{
    સંરચના smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    જો (f_tables->size != *length)
        પાછા 0;

    qtables = malloc_tmphigh(f_tables->size);
    જો (!qtables) {
        warn_noalloc();
        પાછા 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    માટે (t0 = smbios_next(qtables, qtables_len, ખાલી); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        જો (t0->header.type == 0) {
            need_t0 = 0;
            તોડો;
        }
    }

    જો (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = કદ(સંરચના smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        જો (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } નહીંતર {
            *length += t0_len;
            જો (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            જો (સંખ્યા_of_structures)
                (*સંખ્યા_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    જો (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_ઉપલું(*length);
    નહીંતર
        tables = malloc_fseg(*length);
    જો (!tables) {
        warn_noalloc();
        મુક્ત(qtables);
        પાછા 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    જો (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    મુક્ત(qtables);
    પાછા 1;
}

સ્થિર પૂર્ણાંક
smbios_21_setup_entry_point(સંરચના romfile_s *f_tables,
                            સંરચના smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    જો (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->સંખ્યા_of_structures))
        પાછા 0;

    જો ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        પાછા 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((શૂન્ય *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    પાછા 1;
}

સ્થિર પૂર્ણાંક
smbios_30_setup_entry_point(સંરચના romfile_s *f_tables,
                            સંરચના smbios_30_entry_point *ep)
{
    જો (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             ખાલી, ખાલી))
        પાછા 0;

    ep->checksum -= checksum(ep, કદ(*ep));
    copy_smbios_30(ep);
    પાછા 1;
}

સ્થિર પૂર્ણાંક
smbios_romfile_setup(શૂન્ય)
{
    સંરચના romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    સંરચના romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    સંઘ {
        સંરચના smbios_21_entry_point ep21;
        સંરચના smbios_30_entry_point ep30;
    } ep;

    જો (!f_anchor || !f_tables || f_anchor->size > કદ(ep))
        પાછા 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    જો (f_anchor->size == કદ(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        પાછા smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } નહીંતર જો (f_anchor->size == કદ(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        પાછા smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } નહીંતર {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        પાછા 0;
    }
}

શૂન્ય
smbios_setup(શૂન્ય)
{
    જો (smbios_romfile_setup())
        પાછા;
    smbios_legacy_setup();
}

શૂન્ય
copy_table(શૂન્ય *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
