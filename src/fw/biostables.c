// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "byteorder.h" // le32_to_cpu
#समावेश "config.h" // CONFIG_*
#समावेश "hw/pci.h" // pci_config_writeb
#समावेश "malloc.h" // malloc_fseg
#समावेश "memmap.h" //प्रतीक
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_find
#समावेश "std/acpi.h" // struct rsdp_descriptor
#समावेश "std/mptable.h" // MPTABLE_SIGNATURE
#समावेश "std/pirtable.h" // struct pir_header
#समावेश "std/smbios.h" // struct smbios_21_entry_point
#समावेश "string.h" // memcpy
#समावेश "util.h" // copy_table
#समावेश "x86.h" // outb

काष्ठा pir_header *PirAddr VARFSEG;

अटल व्योम *
copy_fseg_table(स्थिर अक्षर *name, व्योम *pos, u32 आकार)
{
    व्योम *newpos = malloc_fseg(आकार);
    अगर (!newpos) {
        warn_noalloc();
        वापस शून्य;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    स_नकल(newpos, pos, आकार);
    वापस newpos;
}

व्योम
copy_pir(व्योम *pos)
{
    काष्ठा pir_header *p = pos;
    अगर (p->signature != PIR_SIGNATURE)
        वापस;
    अगर (PirAddr)
        वापस;
    अगर (p->आकार < माप(*p))
        वापस;
    अगर (checksum(pos, p->आकार) != 0)
        वापस;
    PirAddr = copy_fseg_table("PIR", pos, p->आकार);
}

व्योम
copy_mptable(व्योम *pos)
{
    काष्ठा mptable_floating_s *p = pos;
    अगर (p->signature != MPTABLE_SIGNATURE)
        वापस;
    अगर (!p->physaddr)
        वापस;
    अगर (checksum(pos, माप(*p)) != 0)
        वापस;
    u32 लंबाई = p->लंबाई * 16;
    u16 mpclength = ((काष्ठा mptable_config_s *)p->physaddr)->लंबाई;
    अगर (लंबाई + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , लंबाई + mpclength);
        वापस;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    काष्ठा mptable_floating_s *newpos = malloc_fseg(लंबाई + mpclength);
    अगर (!newpos) {
        warn_noalloc();
        वापस;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    स_नकल(newpos, pos, लंबाई);
    newpos->physaddr = (u32)newpos + लंबाई;
    newpos->checksum -= checksum(newpos, माप(*newpos));
    स_नकल((व्योम*)newpos + लंबाई, (व्योम*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

अटल पूर्णांक
get_acpi_rsdp_लंबाई(व्योम *pos, अचिन्हित आकार)
{
    काष्ठा rsdp_descriptor *p = pos;
    अगर (p->signature != RSDP_SIGNATURE)
        वापस -1;
    u32 लंबाई = 20;
    अगर (लंबाई > आकार)
        वापस -1;
    अगर (checksum(pos, लंबाई) != 0)
        वापस -1;
    अगर (p->revision > 1) {
        लंबाई = p->लंबाई;
        अगर (लंबाई > आकार)
            वापस -1;
        अगर (checksum(pos, लंबाई) != 0)
            वापस -1;
    }
    वापस लंबाई;
}

काष्ठा rsdp_descriptor *RsdpAddr;

व्योम
copy_acpi_rsdp(व्योम *pos)
{
    अगर (RsdpAddr)
        वापस;
    पूर्णांक लंबाई = get_acpi_rsdp_लंबाई(pos, -1);
    अगर (लंबाई < 0)
        वापस;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, लंबाई);
}

व्योम *find_acpi_rsdp(व्योम)
{
    अचिन्हित दीर्घ start = प्रतीक(zonefseg_start);
    अचिन्हित दीर्घ end = प्रतीक(zonefseg_end);
    अचिन्हित दीर्घ pos;

    क्रम (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        अगर (get_acpi_rsdp_लंबाई((व्योम *)pos, end - pos) >= 0)
            वापस (व्योम *)pos;

    वापस शून्य;
}

व्योम *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    अगर (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        वापस शून्य;
    काष्ठा rsdt_descriptor_rev1 *rsdt = (व्योम*)RsdpAddr->rsdt_physical_address;
    काष्ठा xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? शून्य : (व्योम*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    अगर (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        व्योम *end = (व्योम*)xsdt + xsdt->लंबाई;
        पूर्णांक i;
        क्रम (i=0; (व्योम*)&xsdt->table_offset_entry[i] < end; i++) {
            अगर (xsdt->table_offset_entry[i] >= 0x100000000)
                जारी; /* above 4G */
            काष्ठा acpi_table_header *tbl = (व्योम*)(u32)xsdt->table_offset_entry[i];
            अगर (!tbl || tbl->signature != signature)
                जारी;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            वापस tbl;
        }
    }

    अगर (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        व्योम *end = (व्योम*)rsdt + rsdt->लंबाई;
        पूर्णांक i;
        क्रम (i=0; (व्योम*)&rsdt->table_offset_entry[i] < end; i++) {
            काष्ठा acpi_table_header *tbl = (व्योम*)rsdt->table_offset_entry[i];
            अगर (!tbl || tbl->signature != signature)
                जारी;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            वापस tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    वापस शून्य;
}

u32
find_resume_vector(व्योम)
{
    काष्ठा fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    अगर (!fadt)
        वापस 0;
    काष्ठा facs_descriptor_rev1 *facs = (व्योम*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    अगर (! facs || facs->signature != FACS_SIGNATURE)
        वापस 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    वापस facs->firmware_waking_vector;
}

अटल काष्ठा acpi_20_generic_address acpi_reset_reg;
अटल u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#घोषणा acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

व्योम
acpi_reboot(व्योम)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    अगर (acpi_reset_reg.register_bit_width != 8)
        वापस;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    चयन (acpi_reset_reg.address_space_id) {
    हाल 0: // System Memory
        writeb((व्योम *)(u32)addr, acpi_reset_val);
        अवरोध;
    हाल 1: // System I/O
        outb(acpi_reset_val, addr);
        अवरोध;
    हाल 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        अवरोध;
    }
}

अटल व्योम
acpi_set_reset_reg(काष्ठा acpi_20_generic_address *reg, u8 val)
{
    अगर (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        वापस;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

व्योम
find_acpi_features(व्योम)
{
    काष्ठा fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    अगर (!fadt)
        वापस;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    अगर (pm_tmr)
        pmtimer_setup(pm_tmr);
    अगर (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    अगर (fadt->लंबाई >= 129) {
        व्योम *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
व्योम *
smbios_अग्र(व्योम *start, u32 लंबाई, व्योम *पूर्व)
{
    अगर (!start)
        वापस शून्य;
    व्योम *end = start + लंबाई;

    अगर (!पूर्व) {
        पूर्व = start;
    } अन्यथा {
        काष्ठा smbios_structure_header *hdr = पूर्व;
        अगर (पूर्व + माप(*hdr) > end)
            वापस शून्य;
        पूर्व += hdr->लंबाई + 2;
        जबतक (पूर्व < end && (*(u8*)(पूर्व-1) != '\0' || *(u8*)(पूर्व-2) != '\0'))
            पूर्व++;
    }
    काष्ठा smbios_structure_header *hdr = पूर्व;
    अगर (पूर्व >= end || पूर्व + माप(*hdr) >= end || पूर्व + hdr->लंबाई >= end)
        वापस शून्य;
    वापस पूर्व;
}

व्योम *
smbios_21_अग्र(काष्ठा smbios_21_entry_point *smbios, व्योम *पूर्व)
{
    अगर (!smbios)
        वापस शून्य;
    वापस smbios_अग्र((व्योम*)smbios->structure_table_address,
                       smbios->structure_table_लंबाई, पूर्व);
}

अटल काष्ठा smbios_21_entry_point *SMBios21Addr;

व्योम
copy_smbios_21(व्योम *pos)
{
    अगर (SMBios21Addr)
        वापस;
    काष्ठा smbios_21_entry_point *p = pos;
    अगर (p->signature != SMBIOS_21_SIGNATURE)
        वापस;
    अगर (checksum(pos, 0x10) != 0)
        वापस;
    अगर (स_भेद(p->intermediate_anchor_string, "_DMI_", 5))
        वापस;
    अगर (checksum(pos+0x10, p->लंबाई-0x10) != 0)
        वापस;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->लंबाई);
}

अटल काष्ठा smbios_30_entry_point *SMBios30Addr;

अटल पूर्णांक
valid_smbios_30_signature(काष्ठा smbios_30_entry_point *p)
{
    वापस !स_भेद(p->signature, "_SM3_", 5);
}

व्योम
copy_smbios_30(व्योम *pos)
{
    अगर (SMBios30Addr)
        वापस;
    काष्ठा smbios_30_entry_point *p = pos;
    अगर (!valid_smbios_30_signature(p))
        वापस;
    अगर (checksum(pos, p->लंबाई) != 0)
        वापस;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->लंबाई);
}

व्योम *smbios_get_tables(u32 *लंबाई)
{
    अगर (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        अगर (addr32 == SMBios30Addr->structure_table_address) {
            *लंबाई = SMBios30Addr->structure_table_max_आकार;
            वापस (व्योम *)addr32;
        }
    }
    अगर (SMBios21Addr) {
        *लंबाई = SMBios21Addr->structure_table_लंबाई;
        वापस (व्योम *)SMBios21Addr->structure_table_address;
    }
    वापस शून्य;
}

अटल पूर्णांक
smbios_major_version(व्योम)
{
    अगर (SMBios30Addr)
        वापस SMBios30Addr->smbios_major_version;
    अन्यथा अगर (SMBios21Addr)
        वापस SMBios21Addr->smbios_major_version;
    अन्यथा
        वापस 0;
}

अटल पूर्णांक
smbios_minor_version(व्योम)
{
    अगर (SMBios30Addr)
        वापस SMBios30Addr->smbios_minor_version;
    अन्यथा अगर (SMBios21Addr)
        वापस SMBios21Addr->smbios_minor_version;
    अन्यथा
        वापस 0;
}

व्योम
display_uuid(व्योम)
{
    u32 smbios_len = 0;
    व्योम *smbios_tables = smbios_get_tables(&smbios_len);
    काष्ठा smbios_type_1 *tbl = smbios_अग्र(smbios_tables, smbios_len, शून्य);
    पूर्णांक minlen = दुरत्व(काष्ठा smbios_type_1, uuid) + माप(tbl->uuid);
    क्रम (; tbl; tbl = smbios_अग्र(smbios_tables, smbios_len, tbl))
        अगर (tbl->header.type == 1 && tbl->header.लंबाई >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[माप(tbl->uuid)] = { 0 };
            अगर (स_भेद(uuid, empty_uuid, माप(empty_uuid)) == 0)
                वापस;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            अगर (smbios_major_version() > 2 ||
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

            वापस;
        }
}

#घोषणा set_str_field_or_skip(type, field, मूल्य)                       \
    करो {                                                                \
        पूर्णांक आकार = (मूल्य != शून्य) ? म_माप(मूल्य) + 1 : 0;             \
        अगर (आकार > 1) {                                                 \
            स_नकल(end, मूल्य, आकार);                                   \
            end += आकार;                                                \
            p->field = ++str_सूचकांक;                                     \
        } अन्यथा {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } जबतक (0)

अटल व्योम *
smbios_new_type_0(व्योम *start,
                  स्थिर अक्षर *vendor, स्थिर अक्षर *version, स्थिर अक्षर *date)
{
    काष्ठा smbios_type_0 *p = (काष्ठा smbios_type_0 *)start;
    अक्षर *end = (अक्षर *)start + माप(काष्ठा smbios_type_0);
    पूर्णांक str_सूचकांक = 0;

    p->header.type = 0;
    p->header.लंबाई = माप(काष्ठा smbios_type_0);
    p->header.handle = 0;

    set_str_field_or_skip(0, vendor_str, vendor);
    set_str_field_or_skip(0, bios_version_str, version);
    p->bios_starting_address_segment = 0xe800;
    set_str_field_or_skip(0, bios_release_date_str, date);

    p->bios_rom_आकार = 0; /* FIXME */

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
    अगर (!str_सूचकांक) {
        *end = 0;
        end++;
    }

    वापस end;
}

#घोषणा BIOS_NAME "SeaBIOS"
#घोषणा BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
अटल पूर्णांक
smbios_build_tables(काष्ठा romfile_s *f_tables,
                    u64 *address, u32 *लंबाई,
                    u16 *max_structure_आकार,
                    u16 *संख्या_of_structures)
{
    काष्ठा smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    अगर (f_tables->आकार != *लंबाई)
        वापस 0;

    qtables = malloc_tmphigh(f_tables->आकार);
    अगर (!qtables) {
        warn_noalloc();
        वापस 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->आकार);
    qtables_len = f_tables->आकार;

    /* did we get a type 0 structure ? */
    क्रम (t0 = smbios_अग्र(qtables, qtables_len, शून्य); t0;
         t0 = smbios_अग्र(qtables, qtables_len, t0)) {
        अगर (t0->header.type == 0) {
            need_t0 = 0;
            अवरोध;
        }
    }

    अगर (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = माप(काष्ठा smbios_type_0) + म_माप(BIOS_NAME) +
                     म_माप(VERSION) + म_माप(BIOS_DATE) + 4;
        अगर (t0_len > (0xffff - *लंबाई)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *लंबाई, t0_len);
            need_t0 = 0;
        } अन्यथा {
            *लंबाई += t0_len;
            अगर (max_structure_आकार && t0_len > *max_structure_आकार)
                *max_structure_आकार = t0_len;
            अगर (संख्या_of_structures)
                (*संख्या_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    अगर (*लंबाई > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_ऊपरी(*लंबाई);
    अन्यथा
        tables = malloc_fseg(*लंबाई);
    अगर (!tables) {
        warn_noalloc();
        मुक्त(qtables);
        वापस 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    अगर (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    स_नकल(tables, qtables, qtables_len);
    मुक्त(qtables);
    वापस 1;
}

अटल पूर्णांक
smbios_21_setup_entry_point(काष्ठा romfile_s *f_tables,
                            काष्ठा smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 लंबाई = ep->structure_table_लंबाई;

    अगर (!smbios_build_tables(f_tables,
                             &address,
                             &लंबाई,
                             &ep->max_structure_आकार,
                             &ep->संख्या_of_structures))
        वापस 0;

    अगर ((u32)address != address || (u16)लंबाई != लंबाई) {
        warn_internalerror();
        वापस 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_लंबाई = लंबाई;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((व्योम *)ep + 0x10, ep->लंबाई - 0x10);

    copy_smbios_21(ep);
    वापस 1;
}

अटल पूर्णांक
smbios_30_setup_entry_point(काष्ठा romfile_s *f_tables,
                            काष्ठा smbios_30_entry_point *ep)
{
    अगर (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_आकार,
                             शून्य, शून्य))
        वापस 0;

    ep->checksum -= checksum(ep, माप(*ep));
    copy_smbios_30(ep);
    वापस 1;
}

अटल पूर्णांक
smbios_romfile_setup(व्योम)
{
    काष्ठा romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    काष्ठा romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    जोड़ {
        काष्ठा smbios_21_entry_point ep21;
        काष्ठा smbios_30_entry_point ep30;
    } ep;

    अगर (!f_anchor || !f_tables || f_anchor->आकार > माप(ep))
        वापस 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->आकार);

    अगर (f_anchor->आकार == माप(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        वापस smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } अन्यथा अगर (f_anchor->आकार == माप(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        वापस smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } अन्यथा {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        वापस 0;
    }
}

व्योम
smbios_setup(व्योम)
{
    अगर (smbios_romfile_setup())
        वापस;
    smbios_legacy_setup();
}

व्योम
copy_table(व्योम *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
