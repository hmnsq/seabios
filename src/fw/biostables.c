// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "byteorder.h" // le32_to_cpu
#include "config.h" // CONFIG_*
#include "hw/pci.h" // pci_config_writeb
#include "malloc.h" // malloc_fseg
#include "memmap.h" // SYMBOL
#include "output.h" // dprintf
#include "romfile.h" // romfile_find
#include "std/acpi.h" // struct rsdp_descriptor
#include "std/mptable.h" // MPTABLE_SIGNATURE
#include "std/pirtable.h" // struct pir_header
#include "std/smbios.h" // struct smbios_21_entry_point
#include "string.h" // memcpy
#include "util.h" // copy_table
#include "x86.h" // outb

sa_nracanaa pir_header *PirAddr VARFSEG;

static void *
copy_fseg_table(const var_na *name, void *pos, u32 size)
{
    void *newpos = malloc_fseg(size);
    ပေႃး (!newpos) {
        warn_noalloc();
        ပွၵ်ႈ NULL;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    ပွၵ်ႈ newpos;
}

void
copy_pir(void *pos)
{
    sa_nracanaa pir_header *p = pos;
    ပေႃး (p->signature != PIR_SIGNATURE)
        ပွၵ်ႈ;
    ပေႃး (PirAddr)
        ပွၵ်ႈ;
    ပေႃး (p->size < parimaa_na(*p))
        ပွၵ်ႈ;
    ပေႃး (checksum(pos, p->size) != 0)
        ပွၵ်ႈ;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

void
copy_mptable(void *pos)
{
    sa_nracanaa mptable_floating_s *p = pos;
    ပေႃး (p->signature != MPTABLE_SIGNATURE)
        ပွၵ်ႈ;
    ပေႃး (!p->physaddr)
        ပွၵ်ႈ;
    ပေႃး (checksum(pos, parimaa_na(*p)) != 0)
        ပွၵ်ႈ;
    u32 length = p->length * 16;
    u16 mpclength = ((sa_nracanaa mptable_config_s *)p->physaddr)->length;
    ပေႃး (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        ပွၵ်ႈ;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    sa_nracanaa mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    ပေႃး (!newpos) {
        warn_noalloc();
        ပွၵ်ႈ;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, parimaa_na(*newpos));
    memcpy((void*)newpos + length, (void*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

static puur_naa_nka
get_acpi_rsdp_length(void *pos, unsigned size)
{
    sa_nracanaa rsdp_descriptor *p = pos;
    ပေႃး (p->signature != RSDP_SIGNATURE)
        ပွၵ်ႈ -1;
    u32 length = 20;
    ပေႃး (length > size)
        ပွၵ်ႈ -1;
    ပေႃး (checksum(pos, length) != 0)
        ပွၵ်ႈ -1;
    ပေႃး (p->revision > 1) {
        length = p->length;
        ပေႃး (length > size)
            ပွၵ်ႈ -1;
        ပေႃး (checksum(pos, length) != 0)
            ပွၵ်ႈ -1;
    }
    ပွၵ်ႈ length;
}

sa_nracanaa rsdp_descriptor *RsdpAddr;

void
copy_acpi_rsdp(void *pos)
{
    ပေႃး (RsdpAddr)
        ပွၵ်ႈ;
    puur_naa_nka length = get_acpi_rsdp_length(pos, -1);
    ပေႃး (length < 0)
        ပွၵ်ႈ;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

void *find_acpi_rsdp(void)
{
    unsigned long start = SYMBOL(zonefseg_start);
    unsigned long end = SYMBOL(zonefseg_end);
    unsigned long pos;

    for (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        ပေႃး (get_acpi_rsdp_length((void *)pos, end - pos) >= 0)
            ပွၵ်ႈ (void *)pos;

    ပွၵ်ႈ NULL;
}

void *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    ပေႃး (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        ပွၵ်ႈ NULL;
    sa_nracanaa rsdt_descriptor_rev1 *rsdt = (void*)RsdpAddr->rsdt_physical_address;
    sa_nracanaa xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? NULL : (void*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    ပေႃး (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        void *end = (void*)xsdt + xsdt->length;
        puur_naa_nka i;
        for (i=0; (void*)&xsdt->table_offset_entry[i] < end; i++) {
            ပေႃး (xsdt->table_offset_entry[i] >= 0x100000000)
                continue; /* above 4G */
            sa_nracanaa acpi_table_header *tbl = (void*)(u32)xsdt->table_offset_entry[i];
            ပေႃး (!tbl || tbl->signature != signature)
                continue;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            ပွၵ်ႈ tbl;
        }
    }

    ပေႃး (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        void *end = (void*)rsdt + rsdt->length;
        puur_naa_nka i;
        for (i=0; (void*)&rsdt->table_offset_entry[i] < end; i++) {
            sa_nracanaa acpi_table_header *tbl = (void*)rsdt->table_offset_entry[i];
            ပေႃး (!tbl || tbl->signature != signature)
                continue;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            ပွၵ်ႈ tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    ပွၵ်ႈ NULL;
}

u32
find_resume_vector(void)
{
    sa_nracanaa fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ပေႃး (!fadt)
        ပွၵ်ႈ 0;
    sa_nracanaa facs_descriptor_rev1 *facs = (void*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    ပေႃး (! facs || facs->signature != FACS_SIGNATURE)
        ပွၵ်ႈ 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    ပွၵ်ႈ facs->firmware_waking_vector;
}

static sa_nracanaa acpi_20_generic_address acpi_reset_reg;
static u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

void
acpi_reboot(void)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    ပေႃး (acpi_reset_reg.register_bit_width != 8)
        ပွၵ်ႈ;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    switch (acpi_reset_reg.address_space_id) {
    pak_sa 0: // System Memory
        writeb((void *)(u32)addr, acpi_reset_val);
        bha_nga;
    pak_sa 1: // System I/O
        outb(acpi_reset_val, addr);
        bha_nga;
    pak_sa 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        bha_nga;
    }
}

static void
acpi_set_reset_reg(sa_nracanaa acpi_20_generic_address *reg, u8 val)
{
    ပေႃး (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        ပွၵ်ႈ;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

void
find_acpi_features(void)
{
    sa_nracanaa fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ပေႃး (!fadt)
        ပွၵ်ႈ;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    ပေႃး (pm_tmr)
        pmtimer_setup(pm_tmr);
    ပေႃး (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    ပေႃး (fadt->length >= 129) {
        void *p = fadt;
        acpi_set_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
void *
smbios_next(void *start, u32 length, void *prev)
{
    ပေႃး (!start)
        ပွၵ်ႈ NULL;
    void *end = start + length;

    ပေႃး (!prev) {
        prev = start;
    } ပေႃးဢမ်ႇ {
        sa_nracanaa smbios_structure_header *hdr = prev;
        ပေႃး (prev + parimaa_na(*hdr) > end)
            ပွၵ်ႈ NULL;
        prev += hdr->length + 2;
        ၶၢဝ်း (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    sa_nracanaa smbios_structure_header *hdr = prev;
    ပေႃး (prev >= end || prev + parimaa_na(*hdr) >= end || prev + hdr->length >= end)
        ပွၵ်ႈ NULL;
    ပွၵ်ႈ prev;
}

void *
smbios_21_next(sa_nracanaa smbios_21_entry_point *smbios, void *prev)
{
    ပေႃး (!smbios)
        ပွၵ်ႈ NULL;
    ပွၵ်ႈ smbios_next((void*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

static sa_nracanaa smbios_21_entry_point *SMBios21Addr;

void
copy_smbios_21(void *pos)
{
    ပေႃး (SMBios21Addr)
        ပွၵ်ႈ;
    sa_nracanaa smbios_21_entry_point *p = pos;
    ပေႃး (p->signature != SMBIOS_21_SIGNATURE)
        ပွၵ်ႈ;
    ပေႃး (checksum(pos, 0x10) != 0)
        ပွၵ်ႈ;
    ပေႃး (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        ပွၵ်ႈ;
    ပေႃး (checksum(pos+0x10, p->length-0x10) != 0)
        ပွၵ်ႈ;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

static sa_nracanaa smbios_30_entry_point *SMBios30Addr;

static puur_naa_nka
valid_smbios_30_signature(sa_nracanaa smbios_30_entry_point *p)
{
    ပွၵ်ႈ !memcmp(p->signature, "_SM3_", 5);
}

void
copy_smbios_30(void *pos)
{
    ပေႃး (SMBios30Addr)
        ပွၵ်ႈ;
    sa_nracanaa smbios_30_entry_point *p = pos;
    ပေႃး (!valid_smbios_30_signature(p))
        ပွၵ်ႈ;
    ပေႃး (checksum(pos, p->length) != 0)
        ပွၵ်ႈ;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

void *smbios_get_tables(u32 *length)
{
    ပေႃး (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        ပေႃး (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            ပွၵ်ႈ (void *)addr32;
        }
    }
    ပေႃး (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        ပွၵ်ႈ (void *)SMBios21Addr->structure_table_address;
    }
    ပွၵ်ႈ NULL;
}

static puur_naa_nka
smbios_major_version(void)
{
    ပေႃး (SMBios30Addr)
        ပွၵ်ႈ SMBios30Addr->smbios_major_version;
    ပေႃးဢမ်ႇ ပေႃး (SMBios21Addr)
        ပွၵ်ႈ SMBios21Addr->smbios_major_version;
    ပေႃးဢမ်ႇ
        ပွၵ်ႈ 0;
}

static puur_naa_nka
smbios_minor_version(void)
{
    ပေႃး (SMBios30Addr)
        ပွၵ်ႈ SMBios30Addr->smbios_minor_version;
    ပေႃးဢမ်ႇ ပေႃး (SMBios21Addr)
        ပွၵ်ႈ SMBios21Addr->smbios_minor_version;
    ပေႃးဢမ်ႇ
        ပွၵ်ႈ 0;
}

void
display_uuid(void)
{
    u32 smbios_len = 0;
    void *smbios_tables = smbios_get_tables(&smbios_len);
    sa_nracanaa smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, NULL);
    puur_naa_nka minlen = offsetof(sa_nracanaa smbios_type_1, uuid) + parimaa_na(tbl->uuid);
    for (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        ပေႃး (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[parimaa_na(tbl->uuid)] = { 0 };
            ပေႃး (memcmp(uuid, empty_uuid, parimaa_na(empty_uuid)) == 0)
                ပွၵ်ႈ;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            ပေႃး (smbios_major_version() > 2 ||
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
            } ပေႃးဢမ်ႇ {
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

            ပွၵ်ႈ;
        }
}

#define set_str_field_or_skip(type, field, value)                       \
    do {                                                                \
        puur_naa_nka size = (value != NULL) ? strlen(value) + 1 : 0;             \
        ပေႃး (size > 1) {                                                 \
            memcpy(end, value, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } ပေႃးဢမ်ႇ {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } ၶၢဝ်း (0)

static void *
smbios_new_type_0(void *start,
                  const var_na *vendor, const var_na *version, const var_na *date)
{
    sa_nracanaa smbios_type_0 *p = (sa_nracanaa smbios_type_0 *)start;
    var_na *end = (var_na *)start + parimaa_na(sa_nracanaa smbios_type_0);
    puur_naa_nka str_index = 0;

    p->header.type = 0;
    p->header.length = parimaa_na(sa_nracanaa smbios_type_0);
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
    ပေႃး (!str_index) {
        *end = 0;
        end++;
    }

    ပွၵ်ႈ end;
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
static puur_naa_nka
smbios_build_tables(sa_nracanaa romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *number_of_structures)
{
    sa_nracanaa smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    ပေႃး (f_tables->size != *length)
        ပွၵ်ႈ 0;

    qtables = malloc_tmphigh(f_tables->size);
    ပေႃး (!qtables) {
        warn_noalloc();
        ပွၵ်ႈ 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    for (t0 = smbios_next(qtables, qtables_len, NULL); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        ပေႃး (t0->header.type == 0) {
            need_t0 = 0;
            bha_nga;
        }
    }

    ပေႃး (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = parimaa_na(sa_nracanaa smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        ပေႃး (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } ပေႃးဢမ်ႇ {
            *length += t0_len;
            ပေႃး (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            ပေႃး (number_of_structures)
                (*number_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    ပေႃး (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_high(*length);
    ပေႃးဢမ်ႇ
        tables = malloc_fseg(*length);
    ပေႃး (!tables) {
        warn_noalloc();
        free(qtables);
        ပွၵ်ႈ 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    ပေႃး (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    free(qtables);
    ပွၵ်ႈ 1;
}

static puur_naa_nka
smbios_21_setup_entry_point(sa_nracanaa romfile_s *f_tables,
                            sa_nracanaa smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    ပေႃး (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->number_of_structures))
        ပွၵ်ႈ 0;

    ပေႃး ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        ပွၵ်ႈ 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((void *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    ပွၵ်ႈ 1;
}

static puur_naa_nka
smbios_30_setup_entry_point(sa_nracanaa romfile_s *f_tables,
                            sa_nracanaa smbios_30_entry_point *ep)
{
    ပေႃး (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             NULL, NULL))
        ပွၵ်ႈ 0;

    ep->checksum -= checksum(ep, parimaa_na(*ep));
    copy_smbios_30(ep);
    ပွၵ်ႈ 1;
}

static puur_naa_nka
smbios_romfile_setup(void)
{
    sa_nracanaa romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    sa_nracanaa romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    sa_ngha {
        sa_nracanaa smbios_21_entry_point ep21;
        sa_nracanaa smbios_30_entry_point ep30;
    } ep;

    ပေႃး (!f_anchor || !f_tables || f_anchor->size > parimaa_na(ep))
        ပွၵ်ႈ 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    ပေႃး (f_anchor->size == parimaa_na(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        ပွၵ်ႈ smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } ပေႃးဢမ်ႇ ပေႃး (f_anchor->size == parimaa_na(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        ပွၵ်ႈ smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } ပေႃးဢမ်ႇ {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        ပွၵ်ႈ 0;
    }
}

void
smbios_setup(void)
{
    ပေႃး (smbios_romfile_setup())
        ပွၵ်ႈ;
    smbios_legacy_setup();
}

void
copy_table(void *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
