// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "byteorder.h" // le32_to_cpu
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "hw/pci.h" // pci_config_writeb
#अन्तर्भाव "malloc.h" // malloc_fseg
#अन्तर्भाव "memmap.h" //चिह्न
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_find
#अन्तर्भाव "std/acpi.h" // struct rsdp_descriptor
#अन्तर्भाव "std/mptable.h" // MPTABLE_SIGNATURE
#अन्तर्भाव "std/pirtable.h" // struct pir_header
#अन्तर्भाव "std/smbios.h" // struct smbios_21_entry_point
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "util.h" // copy_table
#अन्तर्भाव "x86.h" // outb

संरचना pir_header *PirAddr VARFSEG;

स्थिर शून्य *
copy_fseg_table(अचर वर्ण *name, शून्य *pos, u32 size)
{
    शून्य *newpos = malloc_fseg(size);
    यदि (!newpos) {
        warn_noalloc();
        फर्काऊ रिक्त;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    स_नकल(newpos, pos, size);
    फर्काऊ newpos;
}

शून्य
copy_pir(शून्य *pos)
{
    संरचना pir_header *p = pos;
    यदि (p->signature != PIR_SIGNATURE)
        फर्काऊ;
    यदि (PirAddr)
        फर्काऊ;
    यदि (p->size < परिमाण(*p))
        फर्काऊ;
    यदि (checksum(pos, p->size) != 0)
        फर्काऊ;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

शून्य
copy_mptable(शून्य *pos)
{
    संरचना mptable_floating_s *p = pos;
    यदि (p->signature != MPTABLE_SIGNATURE)
        फर्काऊ;
    यदि (!p->physaddr)
        फर्काऊ;
    यदि (checksum(pos, परिमाण(*p)) != 0)
        फर्काऊ;
    u32 length = p->length * 16;
    u16 mpclength = ((संरचना mptable_config_s *)p->physaddr)->length;
    यदि (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        फर्काऊ;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    संरचना mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    यदि (!newpos) {
        warn_noalloc();
        फर्काऊ;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    स_नकल(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, परिमाण(*newpos));
    स_नकल((शून्य*)newpos + length, (शून्य*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

स्थिर पूर्णाङ्क
get_acpi_rsdp_length(शून्य *pos, अचिन्हित size)
{
    संरचना rsdp_descriptor *p = pos;
    यदि (p->signature != RSDP_SIGNATURE)
        फर्काऊ -1;
    u32 length = 20;
    यदि (length > size)
        फर्काऊ -1;
    यदि (checksum(pos, length) != 0)
        फर्काऊ -1;
    यदि (p->revision > 1) {
        length = p->length;
        यदि (length > size)
            फर्काऊ -1;
        यदि (checksum(pos, length) != 0)
            फर्काऊ -1;
    }
    फर्काऊ length;
}

संरचना rsdp_descriptor *RsdpAddr;

शून्य
copy_acpi_rsdp(शून्य *pos)
{
    यदि (RsdpAddr)
        फर्काऊ;
    पूर्णाङ्क length = get_acpi_rsdp_length(pos, -1);
    यदि (length < 0)
        फर्काऊ;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

शून्य *find_acpi_rsdp(शून्य)
{
    अचिन्हित दीर्घ start = चिह्न(zonefseg_start);
    अचिन्हित दीर्घ end = चिह्न(zonefseg_end);
    अचिन्हित दीर्घ pos;

    लागि (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        यदि (get_acpi_rsdp_length((शून्य *)pos, end - pos) >= 0)
            फर्काऊ (शून्य *)pos;

    फर्काऊ रिक्त;
}

शून्य *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    यदि (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        फर्काऊ रिक्त;
    संरचना rsdt_descriptor_rev1 *rsdt = (शून्य*)RsdpAddr->rsdt_physical_address;
    संरचना xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? रिक्त : (शून्य*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    यदि (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        शून्य *end = (शून्य*)xsdt + xsdt->length;
        पूर्णाङ्क i;
        लागि (i=0; (शून्य*)&xsdt->table_offset_entry[i] < end; i++) {
            यदि (xsdt->table_offset_entry[i] >= 0x100000000)
                जारी; /* above 4G */
            संरचना acpi_table_header *tbl = (शून्य*)(u32)xsdt->table_offset_entry[i];
            यदि (!tbl || tbl->signature != signature)
                जारी;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            फर्काऊ tbl;
        }
    }

    यदि (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        शून्य *end = (शून्य*)rsdt + rsdt->length;
        पूर्णाङ्क i;
        लागि (i=0; (शून्य*)&rsdt->table_offset_entry[i] < end; i++) {
            संरचना acpi_table_header *tbl = (शून्य*)rsdt->table_offset_entry[i];
            यदि (!tbl || tbl->signature != signature)
                जारी;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            फर्काऊ tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    फर्काऊ रिक्त;
}

u32
find_resume_vector(शून्य)
{
    संरचना fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    यदि (!fadt)
        फर्काऊ 0;
    संरचना facs_descriptor_rev1 *facs = (शून्य*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    यदि (! facs || facs->signature != FACS_SIGNATURE)
        फर्काऊ 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    फर्काऊ facs->firmware_waking_vector;
}

स्थिर संरचना acpi_20_generic_address acpi_reset_reg;
स्थिर u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

शून्य
acpi_reboot(शून्य)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    यदि (acpi_reset_reg.register_bit_width != 8)
        फर्काऊ;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    विकल्प (acpi_reset_reg.address_space_id) {
    पक्ष 0: // System Memory
        writeb((शून्य *)(u32)addr, acpi_reset_val);
        रोक;
    पक्ष 1: // System I/O
        outb(acpi_reset_val, addr);
        रोक;
    पक्ष 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        रोक;
    }
}

स्थिर शून्य
acpi_set_reset_reg(संरचना acpi_20_generic_address *reg, u8 val)
{
    यदि (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        फर्काऊ;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

शून्य
find_acpi_features(शून्य)
{
    संरचना fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    यदि (!fadt)
        फर्काऊ;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    यदि (pm_tmr)
        pmtimer_setup(pm_tmr);
    यदि (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    यदि (fadt->length >= 129) {
        शून्य *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
शून्य *
smbios_next(शून्य *start, u32 length, शून्य *prev)
{
    यदि (!start)
        फर्काऊ रिक्त;
    शून्य *end = start + length;

    यदि (!prev) {
        prev = start;
    } अन्यथा {
        संरचना smbios_structure_header *hdr = prev;
        यदि (prev + परिमाण(*hdr) > end)
            फर्काऊ रिक्त;
        prev += hdr->length + 2;
        जबसम्म (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    संरचना smbios_structure_header *hdr = prev;
    यदि (prev >= end || prev + परिमाण(*hdr) >= end || prev + hdr->length >= end)
        फर्काऊ रिक्त;
    फर्काऊ prev;
}

शून्य *
smbios_21_next(संरचना smbios_21_entry_point *smbios, शून्य *prev)
{
    यदि (!smbios)
        फर्काऊ रिक्त;
    फर्काऊ smbios_next((शून्य*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

स्थिर संरचना smbios_21_entry_point *SMBios21Addr;

शून्य
copy_smbios_21(शून्य *pos)
{
    यदि (SMBios21Addr)
        फर्काऊ;
    संरचना smbios_21_entry_point *p = pos;
    यदि (p->signature != SMBIOS_21_SIGNATURE)
        फर्काऊ;
    यदि (checksum(pos, 0x10) != 0)
        फर्काऊ;
    यदि (स_भेद(p->intermediate_anchor_string, "_DMI_", 5))
        फर्काऊ;
    यदि (checksum(pos+0x10, p->length-0x10) != 0)
        फर्काऊ;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

स्थिर संरचना smbios_30_entry_point *SMBios30Addr;

स्थिर पूर्णाङ्क
valid_smbios_30_signature(संरचना smbios_30_entry_point *p)
{
    फर्काऊ !स_भेद(p->signature, "_SM3_", 5);
}

शून्य
copy_smbios_30(शून्य *pos)
{
    यदि (SMBios30Addr)
        फर्काऊ;
    संरचना smbios_30_entry_point *p = pos;
    यदि (!valid_smbios_30_signature(p))
        फर्काऊ;
    यदि (checksum(pos, p->length) != 0)
        फर्काऊ;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

शून्य *smbios_get_tables(u32 *length)
{
    यदि (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        यदि (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            फर्काऊ (शून्य *)addr32;
        }
    }
    यदि (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        फर्काऊ (शून्य *)SMBios21Addr->structure_table_address;
    }
    फर्काऊ रिक्त;
}

स्थिर पूर्णाङ्क
smbios_major_version(शून्य)
{
    यदि (SMBios30Addr)
        फर्काऊ SMBios30Addr->smbios_major_version;
    अन्यथा यदि (SMBios21Addr)
        फर्काऊ SMBios21Addr->smbios_major_version;
    अन्यथा
        फर्काऊ 0;
}

स्थिर पूर्णाङ्क
smbios_minor_version(शून्य)
{
    यदि (SMBios30Addr)
        फर्काऊ SMBios30Addr->smbios_minor_version;
    अन्यथा यदि (SMBios21Addr)
        फर्काऊ SMBios21Addr->smbios_minor_version;
    अन्यथा
        फर्काऊ 0;
}

शून्य
display_uuid(शून्य)
{
    u32 smbios_len = 0;
    शून्य *smbios_tables = smbios_get_tables(&smbios_len);
    संरचना smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, रिक्त);
    पूर्णाङ्क minlen = दुरत्व(संरचना smbios_type_1, uuid) + परिमाण(tbl->uuid);
    लागि (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        यदि (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[परिमाण(tbl->uuid)] = { 0 };
            यदि (स_भेद(uuid, empty_uuid, परिमाण(empty_uuid)) == 0)
                फर्काऊ;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            यदि (smbios_major_version() > 2 ||
                (smbios_major_version() == 2 &&
                 smbios_minor_version() >= 6)) {
                म_लिखो("Machine UUID"
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
                म_लिखो("Machine UUID"
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

            फर्काऊ;
        }
}

#define set_str_field_or_skip(type, field, मान)                       \
    गर {                                                                \
        पूर्णाङ्क size = (मान != रिक्त) ? म_माप(मान) + 1 : 0;             \
        यदि (size > 1) {                                                 \
            स_नकल(end, मान, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } अन्यथा {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } जबसम्म (0)

स्थिर शून्य *
smbios_new_type_0(शून्य *start,
                  अचर वर्ण *vendor, अचर वर्ण *version, अचर वर्ण *date)
{
    संरचना smbios_type_0 *p = (संरचना smbios_type_0 *)start;
    वर्ण *end = (वर्ण *)start + परिमाण(संरचना smbios_type_0);
    पूर्णाङ्क str_index = 0;

    p->header.type = 0;
    p->header.length = परिमाण(संरचना smbios_type_0);
    p->header.handle = 0;

    set_str_field_or_skip(0, vendor_str, vendor);
    set_str_field_or_skip(0, bios_version_str, version);
    p->bios_starting_address_segment = 0xe800;
    set_str_field_or_skip(0, bios_release_date_str, date);

    p->bios_rom_size = 0; /* FIXME */

    /* BIOS characteristics not supported */
    स_रखो(p->bios_characteristics, 0, 8);
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
    यदि (!str_index) {
        *end = 0;
        end++;
    }

    फर्काऊ end;
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
स्थिर पूर्णाङ्क
smbios_build_tables(संरचना romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *सङ्ख्या_of_structures)
{
    संरचना smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    यदि (f_tables->size != *length)
        फर्काऊ 0;

    qtables = malloc_tmphigh(f_tables->size);
    यदि (!qtables) {
        warn_noalloc();
        फर्काऊ 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    लागि (t0 = smbios_next(qtables, qtables_len, रिक्त); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        यदि (t0->header.type == 0) {
            need_t0 = 0;
            रोक;
        }
    }

    यदि (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = परिमाण(संरचना smbios_type_0) + म_माप(BIOS_NAME) +
                     म_माप(VERSION) + म_माप(BIOS_DATE) + 4;
        यदि (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } अन्यथा {
            *length += t0_len;
            यदि (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            यदि (सङ्ख्या_of_structures)
                (*सङ्ख्या_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    यदि (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_माथिल्लो(*length);
    अन्यथा
        tables = malloc_fseg(*length);
    यदि (!tables) {
        warn_noalloc();
        मुक्त(qtables);
        फर्काऊ 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    यदि (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    स_नकल(tables, qtables, qtables_len);
    मुक्त(qtables);
    फर्काऊ 1;
}

स्थिर पूर्णाङ्क
smbios_21_setup_entry_point(संरचना romfile_s *f_tables,
                            संरचना smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    यदि (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->सङ्ख्या_of_structures))
        फर्काऊ 0;

    यदि ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        फर्काऊ 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((शून्य *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    फर्काऊ 1;
}

स्थिर पूर्णाङ्क
smbios_30_setup_entry_point(संरचना romfile_s *f_tables,
                            संरचना smbios_30_entry_point *ep)
{
    यदि (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             रिक्त, रिक्त))
        फर्काऊ 0;

    ep->checksum -= checksum(ep, परिमाण(*ep));
    copy_smbios_30(ep);
    फर्काऊ 1;
}

स्थिर पूर्णाङ्क
smbios_romfile_setup(शून्य)
{
    संरचना romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    संरचना romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    सङ्घ {
        संरचना smbios_21_entry_point ep21;
        संरचना smbios_30_entry_point ep30;
    } ep;

    यदि (!f_anchor || !f_tables || f_anchor->size > परिमाण(ep))
        फर्काऊ 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    यदि (f_anchor->size == परिमाण(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        फर्काऊ smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } अन्यथा यदि (f_anchor->size == परिमाण(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        फर्काऊ smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } अन्यथा {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        फर्काऊ 0;
    }
}

शून्य
smbios_setup(शून्य)
{
    यदि (smbios_romfile_setup())
        फर्काऊ;
    smbios_legacy_setup();
}

शून्य
copy_table(शून्य *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
