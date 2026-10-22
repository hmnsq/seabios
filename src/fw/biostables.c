// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "byteorder.h" // le32_to_cpu
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_config_writeb
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_fseg
#ଅନ୍ତର୍ଭାଵ "memmap.h" //ପ୍ରତୀକ
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_find
#ଅନ୍ତର୍ଭାଵ "std/acpi.h" // struct rsdp_descriptor
#ଅନ୍ତର୍ଭାଵ "std/mptable.h" // MPTABLE_SIGNATURE
#ଅନ୍ତର୍ଭାଵ "std/pirtable.h" // struct pir_header
#ଅନ୍ତର୍ଭାଵ "std/smbios.h" // struct smbios_21_entry_point
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "util.h" // copy_table
#ଅନ୍ତର୍ଭାଵ "x86.h" // outb

ସଂରଚନା pir_header *PirAddr VARFSEG;

ସ୍ଥିର ଶୂନ୍ୟ *
copy_fseg_table(ଅଚଳ ଅକ୍ଷର *name, ଶୂନ୍ୟ *pos, u32 size)
{
    ଶୂନ୍ୟ *newpos = malloc_fseg(size);
    ଯଦି (!newpos) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    ଫେରନ୍ତୁ newpos;
}

ଶୂନ୍ୟ
copy_pir(ଶୂନ୍ୟ *pos)
{
    ସଂରଚନା pir_header *p = pos;
    ଯଦି (p->signature != PIR_SIGNATURE)
        ଫେରନ୍ତୁ;
    ଯଦି (PirAddr)
        ଫେରନ୍ତୁ;
    ଯଦି (p->size < ଆକାର(*p))
        ଫେରନ୍ତୁ;
    ଯଦି (checksum(pos, p->size) != 0)
        ଫେରନ୍ତୁ;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

ଶୂନ୍ୟ
copy_mptable(ଶୂନ୍ୟ *pos)
{
    ସଂରଚନା mptable_floating_s *p = pos;
    ଯଦି (p->signature != MPTABLE_SIGNATURE)
        ଫେରନ୍ତୁ;
    ଯଦି (!p->physaddr)
        ଫେରନ୍ତୁ;
    ଯଦି (checksum(pos, ଆକାର(*p)) != 0)
        ଫେରନ୍ତୁ;
    u32 length = p->length * 16;
    u16 mpclength = ((ସଂରଚନା mptable_config_s *)p->physaddr)->length;
    ଯଦି (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        ଫେରନ୍ତୁ;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    ସଂରଚନା mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    ଯଦି (!newpos) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, ଆକାର(*newpos));
    memcpy((ଶୂନ୍ୟ*)newpos + length, (ଶୂନ୍ୟ*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_acpi_rsdp_length(ଶୂନ୍ୟ *pos, unsigned size)
{
    ସଂରଚନା rsdp_descriptor *p = pos;
    ଯଦି (p->signature != RSDP_SIGNATURE)
        ଫେରନ୍ତୁ -1;
    u32 length = 20;
    ଯଦି (length > size)
        ଫେରନ୍ତୁ -1;
    ଯଦି (checksum(pos, length) != 0)
        ଫେରନ୍ତୁ -1;
    ଯଦି (p->revision > 1) {
        length = p->length;
        ଯଦି (length > size)
            ଫେରନ୍ତୁ -1;
        ଯଦି (checksum(pos, length) != 0)
            ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ length;
}

ସଂରଚନା rsdp_descriptor *RsdpAddr;

ଶୂନ୍ୟ
copy_acpi_rsdp(ଶୂନ୍ୟ *pos)
{
    ଯଦି (RsdpAddr)
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା length = get_acpi_rsdp_length(pos, -1);
    ଯଦି (length < 0)
        ଫେରନ୍ତୁ;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

ଶୂନ୍ୟ *find_acpi_rsdp(ଶୂନ୍ୟ)
{
    unsigned long start = ପ୍ରତୀକ(zonefseg_start);
    unsigned long end = ପ୍ରତୀକ(zonefseg_end);
    unsigned long pos;

    ପାଇଁ (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        ଯଦି (get_acpi_rsdp_length((ଶୂନ୍ୟ *)pos, end - pos) >= 0)
            ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)pos;

    ଫେରନ୍ତୁ ଖାଲି;
}

ଶୂନ୍ୟ *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    ଯଦି (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        ଫେରନ୍ତୁ ଖାଲି;
    ସଂରଚନା rsdt_descriptor_rev1 *rsdt = (ଶୂନ୍ୟ*)RsdpAddr->rsdt_physical_address;
    ସଂରଚନା xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? ଖାଲି : (ଶୂନ୍ୟ*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    ଯଦି (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        ଶୂନ୍ୟ *end = (ଶୂନ୍ୟ*)xsdt + xsdt->length;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i=0; (ଶୂନ୍ୟ*)&xsdt->table_offset_entry[i] < end; i++) {
            ଯଦି (xsdt->table_offset_entry[i] >= 0x100000000)
                ଜାରି; /* above 4G */
            ସଂରଚନା acpi_table_header *tbl = (ଶୂନ୍ୟ*)(u32)xsdt->table_offset_entry[i];
            ଯଦି (!tbl || tbl->signature != signature)
                ଜାରି;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            ଫେରନ୍ତୁ tbl;
        }
    }

    ଯଦି (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        ଶୂନ୍ୟ *end = (ଶୂନ୍ୟ*)rsdt + rsdt->length;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i=0; (ଶୂନ୍ୟ*)&rsdt->table_offset_entry[i] < end; i++) {
            ସଂରଚନା acpi_table_header *tbl = (ଶୂନ୍ୟ*)rsdt->table_offset_entry[i];
            ଯଦି (!tbl || tbl->signature != signature)
                ଜାରି;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            ଫେରନ୍ତୁ tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    ଫେରନ୍ତୁ ଖାଲି;
}

u32
find_resume_vector(ଶୂନ୍ୟ)
{
    ସଂରଚନା fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ଯଦି (!fadt)
        ଫେରନ୍ତୁ 0;
    ସଂରଚନା facs_descriptor_rev1 *facs = (ଶୂନ୍ୟ*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    ଯଦି (! facs || facs->signature != FACS_SIGNATURE)
        ଫେରନ୍ତୁ 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    ଫେରନ୍ତୁ facs->firmware_waking_vector;
}

ସ୍ଥିର ସଂରଚନା acpi_20_generic_address acpi_reset_reg;
ସ୍ଥିର u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

ଶୂନ୍ୟ
acpi_reboot(ଶୂନ୍ୟ)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    ଯଦି (acpi_reset_reg.register_bit_width != 8)
        ଫେରନ୍ତୁ;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    ବାଛ (acpi_reset_reg.address_space_id) {
    ସ୍ଥିତି 0: // System Memory
        writeb((ଶୂନ୍ୟ *)(u32)addr, acpi_reset_val);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 1: // System I/O
        outb(acpi_reset_val, addr);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        ଭାଙ୍ଗ;
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
acpi_set_reset_reg(ସଂରଚନା acpi_20_generic_address *reg, u8 val)
{
    ଯଦି (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        ଫେରନ୍ତୁ;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

ଶୂନ୍ୟ
find_acpi_features(ଶୂନ୍ୟ)
{
    ସଂରଚନା fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ଯଦି (!fadt)
        ଫେରନ୍ତୁ;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    ଯଦି (pm_tmr)
        pmtimer_setup(pm_tmr);
    ଯଦି (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    ଯଦି (fadt->length >= 129) {
        ଶୂନ୍ୟ *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
ଶୂନ୍ୟ *
smbios_next(ଶୂନ୍ୟ *start, u32 length, ଶୂନ୍ୟ *prev)
{
    ଯଦି (!start)
        ଫେରନ୍ତୁ ଖାଲି;
    ଶୂନ୍ୟ *end = start + length;

    ଯଦି (!prev) {
        prev = start;
    } ଅନ୍ୟଥା {
        ସଂରଚନା smbios_structure_header *hdr = prev;
        ଯଦି (prev + ଆକାର(*hdr) > end)
            ଫେରନ୍ତୁ ଖାଲି;
        prev += hdr->length + 2;
        ଯେତେବେଳେ (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    ସଂରଚନା smbios_structure_header *hdr = prev;
    ଯଦି (prev >= end || prev + ଆକାର(*hdr) >= end || prev + hdr->length >= end)
        ଫେରନ୍ତୁ ଖାଲି;
    ଫେରନ୍ତୁ prev;
}

ଶୂନ୍ୟ *
smbios_21_next(ସଂରଚନା smbios_21_entry_point *smbios, ଶୂନ୍ୟ *prev)
{
    ଯଦି (!smbios)
        ଫେରନ୍ତୁ ଖାଲି;
    ଫେରନ୍ତୁ smbios_next((ଶୂନ୍ୟ*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

ସ୍ଥିର ସଂରଚନା smbios_21_entry_point *SMBios21Addr;

ଶୂନ୍ୟ
copy_smbios_21(ଶୂନ୍ୟ *pos)
{
    ଯଦି (SMBios21Addr)
        ଫେରନ୍ତୁ;
    ସଂରଚନା smbios_21_entry_point *p = pos;
    ଯଦି (p->signature != SMBIOS_21_SIGNATURE)
        ଫେରନ୍ତୁ;
    ଯଦି (checksum(pos, 0x10) != 0)
        ଫେରନ୍ତୁ;
    ଯଦି (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        ଫେରନ୍ତୁ;
    ଯଦି (checksum(pos+0x10, p->length-0x10) != 0)
        ଫେରନ୍ତୁ;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

ସ୍ଥିର ସଂରଚନା smbios_30_entry_point *SMBios30Addr;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
valid_smbios_30_signature(ସଂରଚନା smbios_30_entry_point *p)
{
    ଫେରନ୍ତୁ !memcmp(p->signature, "_SM3_", 5);
}

ଶୂନ୍ୟ
copy_smbios_30(ଶୂନ୍ୟ *pos)
{
    ଯଦି (SMBios30Addr)
        ଫେରନ୍ତୁ;
    ସଂରଚନା smbios_30_entry_point *p = pos;
    ଯଦି (!valid_smbios_30_signature(p))
        ଫେରନ୍ତୁ;
    ଯଦି (checksum(pos, p->length) != 0)
        ଫେରନ୍ତୁ;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

ଶୂନ୍ୟ *smbios_get_tables(u32 *length)
{
    ଯଦି (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        ଯଦି (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)addr32;
        }
    }
    ଯଦି (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)SMBios21Addr->structure_table_address;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
smbios_major_version(ଶୂନ୍ୟ)
{
    ଯଦି (SMBios30Addr)
        ଫେରନ୍ତୁ SMBios30Addr->smbios_major_version;
    ଅନ୍ୟଥା ଯଦି (SMBios21Addr)
        ଫେରନ୍ତୁ SMBios21Addr->smbios_major_version;
    ଅନ୍ୟଥା
        ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
smbios_minor_version(ଶୂନ୍ୟ)
{
    ଯଦି (SMBios30Addr)
        ଫେରନ୍ତୁ SMBios30Addr->smbios_minor_version;
    ଅନ୍ୟଥା ଯଦି (SMBios21Addr)
        ଫେରନ୍ତୁ SMBios21Addr->smbios_minor_version;
    ଅନ୍ୟଥା
        ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ
display_uuid(ଶୂନ୍ୟ)
{
    u32 smbios_len = 0;
    ଶୂନ୍ୟ *smbios_tables = smbios_get_tables(&smbios_len);
    ସଂରଚନା smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, ଖାଲି);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା minlen = offsetof(ସଂରଚନା smbios_type_1, uuid) + ଆକାର(tbl->uuid);
    ପାଇଁ (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        ଯଦି (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[ଆକାର(tbl->uuid)] = { 0 };
            ଯଦି (memcmp(uuid, empty_uuid, ଆକାର(empty_uuid)) == 0)
                ଫେରନ୍ତୁ;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            ଯଦି (smbios_major_version() > 2 ||
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
            } ଅନ୍ୟଥା {
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

            ଫେରନ୍ତୁ;
        }
}

#define set_str_field_or_skip(type, field, ମୂଲ୍ୟ)                       \
    କର {                                                                \
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = (ମୂଲ୍ୟ != ଖାଲି) ? strlen(ମୂଲ୍ୟ) + 1 : 0;             \
        ଯଦି (size > 1) {                                                 \
            memcpy(end, ମୂଲ୍ୟ, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } ଅନ୍ୟଥା {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } ଯେତେବେଳେ (0)

ସ୍ଥିର ଶୂନ୍ୟ *
smbios_new_type_0(ଶୂନ୍ୟ *start,
                  ଅଚଳ ଅକ୍ଷର *vendor, ଅଚଳ ଅକ୍ଷର *version, ଅଚଳ ଅକ୍ଷର *date)
{
    ସଂରଚନା smbios_type_0 *p = (ସଂରଚନା smbios_type_0 *)start;
    ଅକ୍ଷର *end = (ଅକ୍ଷର *)start + ଆକାର(ସଂରଚନା smbios_type_0);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_index = 0;

    p->header.type = 0;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_0);
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
    ଯଦି (!str_index) {
        *end = 0;
        end++;
    }

    ଫେରନ୍ତୁ end;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
smbios_build_tables(ସଂରଚନା romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *ସଂଖ୍ୟା_of_structures)
{
    ସଂରଚନା smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    ଯଦି (f_tables->size != *length)
        ଫେରନ୍ତୁ 0;

    qtables = malloc_tmphigh(f_tables->size);
    ଯଦି (!qtables) {
        warn_noalloc();
        ଫେରନ୍ତୁ 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    ପାଇଁ (t0 = smbios_next(qtables, qtables_len, ଖାଲି); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        ଯଦି (t0->header.type == 0) {
            need_t0 = 0;
            ଭାଙ୍ଗ;
        }
    }

    ଯଦି (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = ଆକାର(ସଂରଚନା smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        ଯଦି (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } ଅନ୍ୟଥା {
            *length += t0_len;
            ଯଦି (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            ଯଦି (ସଂଖ୍ୟା_of_structures)
                (*ସଂଖ୍ୟା_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    ଯଦି (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_ଉପର(*length);
    ଅନ୍ୟଥା
        tables = malloc_fseg(*length);
    ଯଦି (!tables) {
        warn_noalloc();
        ମୁକ୍ତ(qtables);
        ଫେରନ୍ତୁ 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    ଯଦି (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    ମୁକ୍ତ(qtables);
    ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
smbios_21_setup_entry_point(ସଂରଚନା romfile_s *f_tables,
                            ସଂରଚନା smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    ଯଦି (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->ସଂଖ୍ୟା_of_structures))
        ଫେରନ୍ତୁ 0;

    ଯଦି ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        ଫେରନ୍ତୁ 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((ଶୂନ୍ୟ *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
smbios_30_setup_entry_point(ସଂରଚନା romfile_s *f_tables,
                            ସଂରଚନା smbios_30_entry_point *ep)
{
    ଯଦି (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             ଖାଲି, ଖାଲି))
        ଫେରନ୍ତୁ 0;

    ep->checksum -= checksum(ep, ଆକାର(*ep));
    copy_smbios_30(ep);
    ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
smbios_romfile_setup(ଶୂନ୍ୟ)
{
    ସଂରଚନା romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    ସଂରଚନା romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    ସଂଘ {
        ସଂରଚନା smbios_21_entry_point ep21;
        ସଂରଚନା smbios_30_entry_point ep30;
    } ep;

    ଯଦି (!f_anchor || !f_tables || f_anchor->size > ଆକାର(ep))
        ଫେରନ୍ତୁ 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    ଯଦି (f_anchor->size == ଆକାର(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        ଫେରନ୍ତୁ smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } ଅନ୍ୟଥା ଯଦି (f_anchor->size == ଆକାର(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        ଫେରନ୍ତୁ smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } ଅନ୍ୟଥା {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        ଫେରନ୍ତୁ 0;
    }
}

ଶୂନ୍ୟ
smbios_setup(ଶୂନ୍ୟ)
{
    ଯଦି (smbios_romfile_setup())
        ଫେରନ୍ତୁ;
    smbios_legacy_setup();
}

ଶୂନ୍ୟ
copy_table(ଶୂନ୍ୟ *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
