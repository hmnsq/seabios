// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "byteorder.h" // le32_to_cpu
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_writeb
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_fseg
#ಅನ್ತರ್ಭಾವ "memmap.h" //ಚಿಹ್ನೆ
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_find
#ಅನ್ತರ್ಭಾವ "std/acpi.h" // struct rsdp_descriptor
#ಅನ್ತರ್ಭಾವ "std/mptable.h" // MPTABLE_SIGNATURE
#ಅನ್ತರ್ಭಾವ "std/pirtable.h" // struct pir_header
#ಅನ್ತರ್ಭಾವ "std/smbios.h" // struct smbios_21_entry_point
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "util.h" // copy_table
#ಅನ್ತರ್ಭಾವ "x86.h" // outb

ಸಂರಚನಾ pir_header *PirAddr VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ *
copy_fseg_table(ಧ್ರುವ ವರ್ಣ *name, ಶೂನ್ಯ *pos, u32 size)
{
    ಶೂನ್ಯ *newpos = malloc_fseg(size);
    ಯದಿ (!newpos) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    ಪ್ರತ್ಯಾಗಮ newpos;
}

ಶೂನ್ಯ
copy_pir(ಶೂನ್ಯ *pos)
{
    ಸಂರಚನಾ pir_header *p = pos;
    ಯದಿ (p->signature != PIR_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (PirAddr)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (p->size < ಪರಿಮಾಣ(*p))
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (checksum(pos, p->size) != 0)
        ಪ್ರತ್ಯಾಗಮ;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

ಶೂನ್ಯ
copy_mptable(ಶೂನ್ಯ *pos)
{
    ಸಂರಚನಾ mptable_floating_s *p = pos;
    ಯದಿ (p->signature != MPTABLE_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (!p->physaddr)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (checksum(pos, ಪರಿಮಾಣ(*p)) != 0)
        ಪ್ರತ್ಯಾಗಮ;
    u32 length = p->length * 16;
    u16 mpclength = ((ಸಂರಚನಾ mptable_config_s *)p->physaddr)->length;
    ಯದಿ (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        ಪ್ರತ್ಯಾಗಮ;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    ಸಂರಚನಾ mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    ಯದಿ (!newpos) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, ಪರಿಮಾಣ(*newpos));
    memcpy((ಶೂನ್ಯ*)newpos + length, (ಶೂನ್ಯ*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
get_acpi_rsdp_length(ಶೂನ್ಯ *pos, unsigned size)
{
    ಸಂರಚನಾ rsdp_descriptor *p = pos;
    ಯದಿ (p->signature != RSDP_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ -1;
    u32 length = 20;
    ಯದಿ (length > size)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (checksum(pos, length) != 0)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (p->revision > 1) {
        length = p->length;
        ಯದಿ (length > size)
            ಪ್ರತ್ಯಾಗಮ -1;
        ಯದಿ (checksum(pos, length) != 0)
            ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ length;
}

ಸಂರಚನಾ rsdp_descriptor *RsdpAddr;

ಶೂನ್ಯ
copy_acpi_rsdp(ಶೂನ್ಯ *pos)
{
    ಯದಿ (RsdpAddr)
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ length = get_acpi_rsdp_length(pos, -1);
    ಯದಿ (length < 0)
        ಪ್ರತ್ಯಾಗಮ;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

ಶೂನ್ಯ *find_acpi_rsdp(ಶೂನ್ಯ)
{
    unsigned long start = ಚಿಹ್ನೆ(zonefseg_start);
    unsigned long end = ಚಿಹ್ನೆ(zonefseg_end);
    unsigned long pos;

    ಪ್ರತಿ (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        ಯದಿ (get_acpi_rsdp_length((ಶೂನ್ಯ *)pos, end - pos) >= 0)
            ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)pos;

    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಶೂನ್ಯ *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    ಯದಿ (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಸಂರಚನಾ rsdt_descriptor_rev1 *rsdt = (ಶೂನ್ಯ*)RsdpAddr->rsdt_physical_address;
    ಸಂರಚನಾ xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? ರಿಕ್ತ : (ಶೂನ್ಯ*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    ಯದಿ (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        ಶೂನ್ಯ *end = (ಶೂನ್ಯ*)xsdt + xsdt->length;
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i=0; (ಶೂನ್ಯ*)&xsdt->table_offset_entry[i] < end; i++) {
            ಯದಿ (xsdt->table_offset_entry[i] >= 0x100000000)
                ಅನುವರ್ತ; /* above 4G */
            ಸಂರಚನಾ acpi_table_header *tbl = (ಶೂನ್ಯ*)(u32)xsdt->table_offset_entry[i];
            ಯದಿ (!tbl || tbl->signature != signature)
                ಅನುವರ್ತ;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            ಪ್ರತ್ಯಾಗಮ tbl;
        }
    }

    ಯದಿ (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        ಶೂನ್ಯ *end = (ಶೂನ್ಯ*)rsdt + rsdt->length;
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i=0; (ಶೂನ್ಯ*)&rsdt->table_offset_entry[i] < end; i++) {
            ಸಂರಚನಾ acpi_table_header *tbl = (ಶೂನ್ಯ*)rsdt->table_offset_entry[i];
            ಯದಿ (!tbl || tbl->signature != signature)
                ಅನುವರ್ತ;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            ಪ್ರತ್ಯಾಗಮ tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

u32
find_resume_vector(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ಯದಿ (!fadt)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಸಂರಚನಾ facs_descriptor_rev1 *facs = (ಶೂನ್ಯ*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    ಯದಿ (! facs || facs->signature != FACS_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    ಪ್ರತ್ಯಾಗಮ facs->firmware_waking_vector;
}

ಸ್ಥಿರ ಸಂರಚನಾ acpi_20_generic_address acpi_reset_reg;
ಸ್ಥಿರ u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

ಶೂನ್ಯ
acpi_reboot(ಶೂನ್ಯ)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    ಯದಿ (acpi_reset_reg.register_bit_width != 8)
        ಪ್ರತ್ಯಾಗಮ;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    ವಿಕಲ್ಪ (acpi_reset_reg.address_space_id) {
    ಪಕ್ಷ 0: // System Memory
        writeb((ಶೂನ್ಯ *)(u32)addr, acpi_reset_val);
        ಭಙ್ಗ;
    ಪಕ್ಷ 1: // System I/O
        outb(acpi_reset_val, addr);
        ಭಙ್ಗ;
    ಪಕ್ಷ 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        ಭಙ್ಗ;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
acpi_set_reset_reg(ಸಂರಚನಾ acpi_20_generic_address *reg, u8 val)
{
    ಯದಿ (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        ಪ್ರತ್ಯಾಗಮ;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

ಶೂನ್ಯ
find_acpi_features(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ಯದಿ (!fadt)
        ಪ್ರತ್ಯಾಗಮ;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    ಯದಿ (pm_tmr)
        pmtimer_setup(pm_tmr);
    ಯದಿ (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    ಯದಿ (fadt->length >= 129) {
        ಶೂನ್ಯ *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
ಶೂನ್ಯ *
smbios_next(ಶೂನ್ಯ *start, u32 length, ಶೂನ್ಯ *prev)
{
    ಯದಿ (!start)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಶೂನ್ಯ *end = start + length;

    ಯದಿ (!prev) {
        prev = start;
    } ಅನ್ಯಥಾ {
        ಸಂರಚನಾ smbios_structure_header *hdr = prev;
        ಯದಿ (prev + ಪರಿಮಾಣ(*hdr) > end)
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        prev += hdr->length + 2;
        ಯಾವತ್ (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    ಸಂರಚನಾ smbios_structure_header *hdr = prev;
    ಯದಿ (prev >= end || prev + ಪರಿಮಾಣ(*hdr) >= end || prev + hdr->length >= end)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ prev;
}

ಶೂನ್ಯ *
smbios_21_next(ಸಂರಚನಾ smbios_21_entry_point *smbios, ಶೂನ್ಯ *prev)
{
    ಯದಿ (!smbios)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ smbios_next((ಶೂನ್ಯ*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

ಸ್ಥಿರ ಸಂರಚನಾ smbios_21_entry_point *SMBios21Addr;

ಶೂನ್ಯ
copy_smbios_21(ಶೂನ್ಯ *pos)
{
    ಯದಿ (SMBios21Addr)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ smbios_21_entry_point *p = pos;
    ಯದಿ (p->signature != SMBIOS_21_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (checksum(pos, 0x10) != 0)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (checksum(pos+0x10, p->length-0x10) != 0)
        ಪ್ರತ್ಯಾಗಮ;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

ಸ್ಥಿರ ಸಂರಚನಾ smbios_30_entry_point *SMBios30Addr;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
valid_smbios_30_signature(ಸಂರಚನಾ smbios_30_entry_point *p)
{
    ಪ್ರತ್ಯಾಗಮ !memcmp(p->signature, "_SM3_", 5);
}

ಶೂನ್ಯ
copy_smbios_30(ಶೂನ್ಯ *pos)
{
    ಯದಿ (SMBios30Addr)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ smbios_30_entry_point *p = pos;
    ಯದಿ (!valid_smbios_30_signature(p))
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (checksum(pos, p->length) != 0)
        ಪ್ರತ್ಯಾಗಮ;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

ಶೂನ್ಯ *smbios_get_tables(u32 *length)
{
    ಯದಿ (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        ಯದಿ (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)addr32;
        }
    }
    ಯದಿ (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)SMBios21Addr->structure_table_address;
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
smbios_major_version(ಶೂನ್ಯ)
{
    ಯದಿ (SMBios30Addr)
        ಪ್ರತ್ಯಾಗಮ SMBios30Addr->smbios_major_version;
    ಅನ್ಯಥಾ ಯದಿ (SMBios21Addr)
        ಪ್ರತ್ಯಾಗಮ SMBios21Addr->smbios_major_version;
    ಅನ್ಯಥಾ
        ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
smbios_minor_version(ಶೂನ್ಯ)
{
    ಯದಿ (SMBios30Addr)
        ಪ್ರತ್ಯಾಗಮ SMBios30Addr->smbios_minor_version;
    ಅನ್ಯಥಾ ಯದಿ (SMBios21Addr)
        ಪ್ರತ್ಯಾಗಮ SMBios21Addr->smbios_minor_version;
    ಅನ್ಯಥಾ
        ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ
display_uuid(ಶೂನ್ಯ)
{
    u32 smbios_len = 0;
    ಶೂನ್ಯ *smbios_tables = smbios_get_tables(&smbios_len);
    ಸಂರಚನಾ smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, ರಿಕ್ತ);
    ಪೂರ್ಣಾಙ್ಕ minlen = offsetof(ಸಂರಚನಾ smbios_type_1, uuid) + ಪರಿಮಾಣ(tbl->uuid);
    ಪ್ರತಿ (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        ಯದಿ (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[ಪರಿಮಾಣ(tbl->uuid)] = { 0 };
            ಯದಿ (memcmp(uuid, empty_uuid, ಪರಿಮಾಣ(empty_uuid)) == 0)
                ಪ್ರತ್ಯಾಗಮ;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            ಯದಿ (smbios_major_version() > 2 ||
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
            } ಅನ್ಯಥಾ {
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

            ಪ್ರತ್ಯಾಗಮ;
        }
}

#define set_str_field_or_skip(type, field, ಮೌಲ್ಯ)                       \
    ಕುರು {                                                                \
        ಪೂರ್ಣಾಙ್ಕ size = (ಮೌಲ್ಯ != ರಿಕ್ತ) ? strlen(ಮೌಲ್ಯ) + 1 : 0;             \
        ಯದಿ (size > 1) {                                                 \
            memcpy(end, ಮೌಲ್ಯ, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } ಅನ್ಯಥಾ {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } ಯಾವತ್ (0)

ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_new_type_0(ಶೂನ್ಯ *start,
                  ಧ್ರುವ ವರ್ಣ *vendor, ಧ್ರುವ ವರ್ಣ *version, ಧ್ರುವ ವರ್ಣ *date)
{
    ಸಂರಚನಾ smbios_type_0 *p = (ಸಂರಚನಾ smbios_type_0 *)start;
    ವರ್ಣ *end = (ವರ್ಣ *)start + ಪರಿಮಾಣ(ಸಂರಚನಾ smbios_type_0);
    ಪೂರ್ಣಾಙ್ಕ str_index = 0;

    p->header.type = 0;
    p->header.length = ಪರಿಮಾಣ(ಸಂರಚನಾ smbios_type_0);
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
    ಯದಿ (!str_index) {
        *end = 0;
        end++;
    }

    ಪ್ರತ್ಯಾಗಮ end;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
smbios_build_tables(ಸಂರಚನಾ romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *ಸಂಖ್ಯೆ_of_structures)
{
    ಸಂರಚನಾ smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    ಯದಿ (f_tables->size != *length)
        ಪ್ರತ್ಯಾಗಮ 0;

    qtables = malloc_tmphigh(f_tables->size);
    ಯದಿ (!qtables) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    ಪ್ರತಿ (t0 = smbios_next(qtables, qtables_len, ರಿಕ್ತ); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        ಯದಿ (t0->header.type == 0) {
            need_t0 = 0;
            ಭಙ್ಗ;
        }
    }

    ಯದಿ (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = ಪರಿಮಾಣ(ಸಂರಚನಾ smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        ಯದಿ (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } ಅನ್ಯಥಾ {
            *length += t0_len;
            ಯದಿ (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            ಯದಿ (ಸಂಖ್ಯೆ_of_structures)
                (*ಸಂಖ್ಯೆ_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    ಯದಿ (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_ಮೇಲಿನ(*length);
    ಅನ್ಯಥಾ
        tables = malloc_fseg(*length);
    ಯದಿ (!tables) {
        warn_noalloc();
        ಮುಕ್ತ(qtables);
        ಪ್ರತ್ಯಾಗಮ 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    ಯದಿ (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    ಮುಕ್ತ(qtables);
    ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
smbios_21_setup_entry_point(ಸಂರಚನಾ romfile_s *f_tables,
                            ಸಂರಚನಾ smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    ಯದಿ (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->ಸಂಖ್ಯೆ_of_structures))
        ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((ಶೂನ್ಯ *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
smbios_30_setup_entry_point(ಸಂರಚನಾ romfile_s *f_tables,
                            ಸಂರಚನಾ smbios_30_entry_point *ep)
{
    ಯದಿ (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             ರಿಕ್ತ, ರಿಕ್ತ))
        ಪ್ರತ್ಯಾಗಮ 0;

    ep->checksum -= checksum(ep, ಪರಿಮಾಣ(*ep));
    copy_smbios_30(ep);
    ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
smbios_romfile_setup(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    ಸಂರಚನಾ romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    ಸಙ್ಘ {
        ಸಂರಚನಾ smbios_21_entry_point ep21;
        ಸಂರಚನಾ smbios_30_entry_point ep30;
    } ep;

    ಯದಿ (!f_anchor || !f_tables || f_anchor->size > ಪರಿಮಾಣ(ep))
        ಪ್ರತ್ಯಾಗಮ 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    ಯದಿ (f_anchor->size == ಪರಿಮಾಣ(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        ಪ್ರತ್ಯಾಗಮ smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } ಅನ್ಯಥಾ ಯದಿ (f_anchor->size == ಪರಿಮಾಣ(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        ಪ್ರತ್ಯಾಗಮ smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } ಅನ್ಯಥಾ {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }
}

ಶೂನ್ಯ
smbios_setup(ಶೂನ್ಯ)
{
    ಯದಿ (smbios_romfile_setup())
        ಪ್ರತ್ಯಾಗಮ;
    smbios_legacy_setup();
}

ಶೂನ್ಯ
copy_table(ಶೂನ್ಯ *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
