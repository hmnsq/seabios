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

struct pir_header *PirAddr VARFSEG;

static void *
copy_fseg_table(const char *name, void *pos, u32 size)
{
    void *newpos = malloc_fseg(size);
    nếu (!newpos) {
        warn_noalloc();
        trả_về NULL;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    memcpy(newpos, pos, size);
    trả_về newpos;
}

void
copy_pir(void *pos)
{
    struct pir_header *p = pos;
    nếu (p->signature != PIR_SIGNATURE)
        trả_về;
    nếu (PirAddr)
        trả_về;
    nếu (p->size < sizeof(*p))
        trả_về;
    nếu (checksum(pos, p->size) != 0)
        trả_về;
    PirAddr = copy_fseg_table("PIR", pos, p->size);
}

void
copy_mptable(void *pos)
{
    struct mptable_floating_s *p = pos;
    nếu (p->signature != MPTABLE_SIGNATURE)
        trả_về;
    nếu (!p->physaddr)
        trả_về;
    nếu (checksum(pos, sizeof(*p)) != 0)
        trả_về;
    u32 length = p->length * 16;
    u16 mpclength = ((struct mptable_config_s *)p->physaddr)->length;
    nếu (length + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , length + mpclength);
        trả_về;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    struct mptable_floating_s *newpos = malloc_fseg(length + mpclength);
    nếu (!newpos) {
        warn_noalloc();
        trả_về;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    memcpy(newpos, pos, length);
    newpos->physaddr = (u32)newpos + length;
    newpos->checksum -= checksum(newpos, sizeof(*newpos));
    memcpy((void*)newpos + length, (void*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

static số_nguyên
get_acpi_rsdp_length(void *pos, unsigned size)
{
    struct rsdp_descriptor *p = pos;
    nếu (p->signature != RSDP_SIGNATURE)
        trả_về -1;
    u32 length = 20;
    nếu (length > size)
        trả_về -1;
    nếu (checksum(pos, length) != 0)
        trả_về -1;
    nếu (p->revision > 1) {
        length = p->length;
        nếu (length > size)
            trả_về -1;
        nếu (checksum(pos, length) != 0)
            trả_về -1;
    }
    trả_về length;
}

struct rsdp_descriptor *RsdpAddr;

void
copy_acpi_rsdp(void *pos)
{
    nếu (RsdpAddr)
        trả_về;
    số_nguyên length = get_acpi_rsdp_length(pos, -1);
    nếu (length < 0)
        trả_về;
    RsdpAddr = copy_fseg_table("ACPI RSDP", pos, length);
}

void *find_acpi_rsdp(void)
{
    unsigned long start = SYMBOL(zonefseg_start);
    unsigned long end = SYMBOL(zonefseg_end);
    unsigned long pos;

    cho (pos = ALIGN(start, 0x10); pos <= ALIGN_DOWN(end, 0x10); pos += 0x10)
        nếu (get_acpi_rsdp_length((void *)pos, end - pos) >= 0)
            trả_về (void *)pos;

    trả_về NULL;
}

void *
find_acpi_table(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    nếu (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        trả_về NULL;
    struct rsdt_descriptor_rev1 *rsdt = (void*)RsdpAddr->rsdt_physical_address;
    struct xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_address >= 0x100000000
        ? NULL : (void*)(u32)(RsdpAddr->xsdt_physical_address);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    nếu (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        void *end = (void*)xsdt + xsdt->length;
        số_nguyên i;
        cho (i=0; (void*)&xsdt->table_offset_entry[i] < end; i++) {
            nếu (xsdt->table_offset_entry[i] >= 0x100000000)
                continue; /* above 4G */
            struct acpi_table_header *tbl = (void*)(u32)xsdt->table_offset_entry[i];
            nếu (!tbl || tbl->signature != signature)
                continue;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            trả_về tbl;
        }
    }

    nếu (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        void *end = (void*)rsdt + rsdt->length;
        số_nguyên i;
        cho (i=0; (void*)&rsdt->table_offset_entry[i] < end; i++) {
            struct acpi_table_header *tbl = (void*)rsdt->table_offset_entry[i];
            nếu (!tbl || tbl->signature != signature)
                continue;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            trả_về tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    trả_về NULL;
}

u32
find_resume_vector(void)
{
    struct fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    nếu (!fadt)
        trả_về 0;
    struct facs_descriptor_rev1 *facs = (void*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    nếu (! facs || facs->signature != FACS_SIGNATURE)
        trả_về 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_vector);
    trả_về facs->firmware_waking_vector;
}

static struct acpi_20_generic_address acpi_reset_reg;
static u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_base = 0xb000;

#define acpi_ga_to_bdf(addr) pci_to_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

void
acpi_reboot(void)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    nếu (acpi_reset_reg.register_bit_width != 8)
        trả_về;

    u64 addr = le64_to_cpu(acpi_reset_reg.address);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.address_space_id, addr, acpi_reset_val);

    switch (acpi_reset_reg.address_space_id) {
    case 0: // System Memory
        writeb((void *)(u32)addr, acpi_reset_val);
        break;
    case 1: // System I/O
        outb(acpi_reset_val, addr);
        break;
    case 2: // PCI config space
        pci_config_writeb(acpi_ga_to_bdf(addr), addr & 0xffff, acpi_reset_val);
        break;
    }
}

static void
acpi_set_reset_reg(struct acpi_20_generic_address *reg, u8 val)
{
    nếu (!reg || reg->address_space_id > 2 ||
        reg->register_bit_width != 8 || reg->register_bit_offset)
        trả_về;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

void
find_acpi_features(void)
{
    struct fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    nếu (!fadt)
        trả_về;
    u32 pm_tmr = le32_to_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_to_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    nếu (pm_tmr)
        pmtimer_setup(pm_tmr);
    nếu (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    nếu (fadt->length >= 129) {
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
    nếu (!start)
        trả_về NULL;
    void *end = start + length;

    nếu (!prev) {
        prev = start;
    } không_thì {
        struct smbios_structure_header *hdr = prev;
        nếu (prev + sizeof(*hdr) > end)
            trả_về NULL;
        prev += hdr->length + 2;
        trong_khi (prev < end && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    struct smbios_structure_header *hdr = prev;
    nếu (prev >= end || prev + sizeof(*hdr) >= end || prev + hdr->length >= end)
        trả_về NULL;
    trả_về prev;
}

void *
smbios_21_next(struct smbios_21_entry_point *smbios, void *prev)
{
    nếu (!smbios)
        trả_về NULL;
    trả_về smbios_next((void*)smbios->structure_table_address,
                       smbios->structure_table_length, prev);
}

static struct smbios_21_entry_point *SMBios21Addr;

void
copy_smbios_21(void *pos)
{
    nếu (SMBios21Addr)
        trả_về;
    struct smbios_21_entry_point *p = pos;
    nếu (p->signature != SMBIOS_21_SIGNATURE)
        trả_về;
    nếu (checksum(pos, 0x10) != 0)
        trả_về;
    nếu (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        trả_về;
    nếu (checksum(pos+0x10, p->length-0x10) != 0)
        trả_về;
    SMBios21Addr = copy_fseg_table("SMBIOS", pos, p->length);
}

static struct smbios_30_entry_point *SMBios30Addr;

static số_nguyên
valid_smbios_30_signature(struct smbios_30_entry_point *p)
{
    trả_về !memcmp(p->signature, "_SM3_", 5);
}

void
copy_smbios_30(void *pos)
{
    nếu (SMBios30Addr)
        trả_về;
    struct smbios_30_entry_point *p = pos;
    nếu (!valid_smbios_30_signature(p))
        trả_về;
    nếu (checksum(pos, p->length) != 0)
        trả_về;
    SMBios30Addr = copy_fseg_table("SMBIOS 3.0", pos, p->length);
}

void *smbios_get_tables(u32 *length)
{
    nếu (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_table_address;
        nếu (addr32 == SMBios30Addr->structure_table_address) {
            *length = SMBios30Addr->structure_table_max_size;
            trả_về (void *)addr32;
        }
    }
    nếu (SMBios21Addr) {
        *length = SMBios21Addr->structure_table_length;
        trả_về (void *)SMBios21Addr->structure_table_address;
    }
    trả_về NULL;
}

static số_nguyên
smbios_major_version(void)
{
    nếu (SMBios30Addr)
        trả_về SMBios30Addr->smbios_major_version;
    không_thì nếu (SMBios21Addr)
        trả_về SMBios21Addr->smbios_major_version;
    không_thì
        trả_về 0;
}

static số_nguyên
smbios_minor_version(void)
{
    nếu (SMBios30Addr)
        trả_về SMBios30Addr->smbios_minor_version;
    không_thì nếu (SMBios21Addr)
        trả_về SMBios21Addr->smbios_minor_version;
    không_thì
        trả_về 0;
}

void
display_uuid(void)
{
    u32 smbios_len = 0;
    void *smbios_tables = smbios_get_tables(&smbios_len);
    struct smbios_type_1 *tbl = smbios_next(smbios_tables, smbios_len, NULL);
    số_nguyên minlen = offsetof(struct smbios_type_1, uuid) + sizeof(tbl->uuid);
    cho (; tbl; tbl = smbios_next(smbios_tables, smbios_len, tbl))
        nếu (tbl->header.type == 1 && tbl->header.length >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 empty_uuid[sizeof(tbl->uuid)] = { 0 };
            nếu (memcmp(uuid, empty_uuid, sizeof(empty_uuid)) == 0)
                trả_về;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            nếu (smbios_major_version() > 2 ||
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
            } không_thì {
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

            trả_về;
        }
}

#define set_str_field_or_skip(type, field, value)                       \
    do {                                                                \
        số_nguyên size = (value != NULL) ? strlen(value) + 1 : 0;             \
        nếu (size > 1) {                                                 \
            memcpy(end, value, size);                                   \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } không_thì {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } trong_khi (0)

static void *
smbios_new_type_0(void *start,
                  const char *vendor, const char *version, const char *date)
{
    struct smbios_type_0 *p = (struct smbios_type_0 *)start;
    char *end = (char *)start + sizeof(struct smbios_type_0);
    số_nguyên str_index = 0;

    p->header.type = 0;
    p->header.length = sizeof(struct smbios_type_0);
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
    nếu (!str_index) {
        *end = 0;
        end++;
    }

    trả_về end;
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
static số_nguyên
smbios_build_tables(struct romfile_s *f_tables,
                    u64 *address, u32 *length,
                    u16 *max_structure_size,
                    u16 *number_of_structures)
{
    struct smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    nếu (f_tables->size != *length)
        trả_về 0;

    qtables = malloc_tmphigh(f_tables->size);
    nếu (!qtables) {
        warn_noalloc();
        trả_về 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->size);
    qtables_len = f_tables->size;

    /* did we get a type 0 structure ? */
    cho (t0 = smbios_next(qtables, qtables_len, NULL); t0;
         t0 = smbios_next(qtables, qtables_len, t0)) {
        nếu (t0->header.type == 0) {
            need_t0 = 0;
            break;
        }
    }

    nếu (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = sizeof(struct smbios_type_0) + strlen(BIOS_NAME) +
                     strlen(VERSION) + strlen(BIOS_DATE) + 4;
        nếu (t0_len > (0xffff - *length)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *length, t0_len);
            need_t0 = 0;
        } không_thì {
            *length += t0_len;
            nếu (max_structure_size && t0_len > *max_structure_size)
                *max_structure_size = t0_len;
            nếu (number_of_structures)
                (*number_of_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    nếu (*length > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_high(*length);
    không_thì
        tables = malloc_fseg(*length);
    nếu (!tables) {
        warn_noalloc();
        free(qtables);
        trả_về 0;
    }
    *address = (u32)tables;

    /* populate final blob */
    nếu (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, VERSION, BIOS_DATE);
    memcpy(tables, qtables, qtables_len);
    free(qtables);
    trả_về 1;
}

static số_nguyên
smbios_21_setup_entry_point(struct romfile_s *f_tables,
                            struct smbios_21_entry_point *ep)
{
    u64 address = ep->structure_table_address;
    u32 length = ep->structure_table_length;

    nếu (!smbios_build_tables(f_tables,
                             &address,
                             &length,
                             &ep->max_structure_size,
                             &ep->number_of_structures))
        trả_về 0;

    nếu ((u32)address != address || (u16)length != length) {
        warn_internalerror();
        trả_về 0;
    }

    /* finalize entry point */
    ep->structure_table_address = address;
    ep->structure_table_length = length;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((void *)ep + 0x10, ep->length - 0x10);

    copy_smbios_21(ep);
    trả_về 1;
}

static số_nguyên
smbios_30_setup_entry_point(struct romfile_s *f_tables,
                            struct smbios_30_entry_point *ep)
{
    nếu (!smbios_build_tables(f_tables,
                             &ep->structure_table_address,
                             &ep->structure_table_max_size,
                             NULL, NULL))
        trả_về 0;

    ep->checksum -= checksum(ep, sizeof(*ep));
    copy_smbios_30(ep);
    trả_về 1;
}

static số_nguyên
smbios_romfile_setup(void)
{
    struct romfile_s *f_anchor = romfile_find("etc/smbios/smbios-anchor");
    struct romfile_s *f_tables = romfile_find("etc/smbios/smbios-tables");
    union {
        struct smbios_21_entry_point ep21;
        struct smbios_30_entry_point ep30;
    } ep;

    nếu (!f_anchor || !f_tables || f_anchor->size > sizeof(ep))
        trả_về 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->size);

    nếu (f_anchor->size == sizeof(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        trả_về smbios_21_setup_entry_point(f_tables, &ep.ep21);
    } không_thì nếu (f_anchor->size == sizeof(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        trả_về smbios_30_setup_entry_point(f_tables, &ep.ep30);
    } không_thì {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        trả_về 0;
    }
}

void
smbios_setup(void)
{
    nếu (smbios_romfile_setup())
        trả_về;
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
