// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "byteorder.h" // le32_to_cpu
#ضمن "config.h" // CONFIG_*
#ضمن "hw/pci.h" // pci_config_writeb
#ضمن "malloc.h" // malloc_fseg
#ضمن "memmap.h" //نشان
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_find
#ضمن "std/acpi.h" // struct rsdp_descriptor
#ضمن "std/mptable.h" // MPTABLE_SIGNATURE
#ضمن "std/pirtable.h" // struct pir_header
#ضمن "std/smbios.h" // struct smbios_21_entry_point
#ضمن "string.h" // memcpy
#ضمن "util.h" // copy_table
#ضمن "x86.h" // outb

بنية pir_سرو *PirAddr VARFSEG;

ساكن فراغ *
copy_fseg_جدول(ثابت حرف *name, فراغ *pos, u32 سائيز)
{
    فراغ *newpos = malloc_fseg(سائيز);
    إذا (!newpos) {
        warn_noalloc();
        أرجع عدم;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    يادگيري_نقل(newpos, pos, سائيز);
    أرجع newpos;
}

فراغ
copy_pir(فراغ *pos)
{
    بنية pir_سرو *p = pos;
    إذا (p->signature != PIR_SIGNATURE)
        أرجع;
    إذا (PirAddr)
        أرجع;
    إذا (p->سائيز < حجم(*p))
        أرجع;
    إذا (checksum(pos, p->سائيز) != 0)
        أرجع;
    PirAddr = copy_fseg_جدول("PIR", pos, p->سائيز);
}

فراغ
copy_mptable(فراغ *pos)
{
    بنية mptable_floating_s *p = pos;
    إذا (p->signature != MPTABLE_SIGNATURE)
        أرجع;
    إذا (!p->physaddr)
        أرجع;
    إذا (checksum(pos, حجم(*p)) != 0)
        أرجع;
    u32 ڊيگهائي = p->ڊيگهائي * 16;
    u16 mpclength = ((بنية mptable_config_s *)p->physaddr)->ڊيگهائي;
    إذا (ڊيگهائي + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , ڊيگهائي + mpclength);
        أرجع;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    بنية mptable_floating_s *newpos = malloc_fseg(ڊيگهائي + mpclength);
    إذا (!newpos) {
        warn_noalloc();
        أرجع;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    يادگيري_نقل(newpos, pos, ڊيگهائي);
    newpos->physaddr = (u32)newpos + ڊيگهائي;
    newpos->checksum -= checksum(newpos, حجم(*newpos));
    يادگيري_نقل((فراغ*)newpos + ڊيگهائي, (فراغ*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ساكن صحيح
get_acpi_rsdp_ڊيگهائي(فراغ *pos, بنانشان سائيز)
{
    بنية rsdp_descriptor *p = pos;
    إذا (p->signature != RSDP_SIGNATURE)
        أرجع -1;
    u32 ڊيگهائي = 20;
    إذا (ڊيگهائي > سائيز)
        أرجع -1;
    إذا (checksum(pos, ڊيگهائي) != 0)
        أرجع -1;
    إذا (p->revision > 1) {
        ڊيگهائي = p->ڊيگهائي;
        إذا (ڊيگهائي > سائيز)
            أرجع -1;
        إذا (checksum(pos, ڊيگهائي) != 0)
            أرجع -1;
    }
    أرجع ڊيگهائي;
}

بنية rsdp_descriptor *RsdpAddr;

فراغ
copy_acpi_rsdp(فراغ *pos)
{
    إذا (RsdpAddr)
        أرجع;
    صحيح ڊيگهائي = get_acpi_rsdp_ڊيگهائي(pos, -1);
    إذا (ڊيگهائي < 0)
        أرجع;
    RsdpAddr = copy_fseg_جدول("ACPI RSDP", pos, ڊيگهائي);
}

فراغ *ڳوليو_acpi_rsdp(فراغ)
{
    بنانشان ڊگهو شروعات = نشان(zonefseg_شروعات);
    بنانشان ڊگهو پڇاڙي = نشان(zonefseg_پڇاڙي);
    بنانشان ڊگهو pos;

    لكل (pos = ALIGN(شروعات, 0x10); pos <= ALIGN_هيٺ_ڏانهن(پڇاڙي, 0x10); pos += 0x10)
        إذا (get_acpi_rsdp_ڊيگهائي((فراغ *)pos, پڇاڙي - pos) >= 0)
            أرجع (فراغ *)pos;

    أرجع عدم;
}

فراغ *
ڳوليو_acpi_جدول(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    إذا (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        أرجع عدم;
    بنية rsdt_descriptor_rev1 *rsdt = (فراغ*)RsdpAddr->rsdt_physical_پتو;
    بنية xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_پتو >= 0x100000000
        ? عدم : (فراغ*)(u32)(RsdpAddr->xsdt_physical_پتو);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    إذا (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        فراغ *پڇاڙي = (فراغ*)xsdt + xsdt->ڊيگهائي;
        صحيح i;
        لكل (i=0; (فراغ*)&xsdt->جدول_وٿي_داخلا[i] < پڇاڙي; i++) {
            إذا (xsdt->جدول_وٿي_داخلا[i] >= 0x100000000)
                استمر; /* above 4G */
            بنية acpi_جدول_سرو *tbl = (فراغ*)(u32)xsdt->جدول_وٿي_داخلا[i];
            إذا (!tbl || tbl->signature != signature)
                استمر;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            أرجع tbl;
        }
    }

    إذا (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        فراغ *پڇاڙي = (فراغ*)rsdt + rsdt->ڊيگهائي;
        صحيح i;
        لكل (i=0; (فراغ*)&rsdt->جدول_وٿي_داخلا[i] < پڇاڙي; i++) {
            بنية acpi_جدول_سرو *tbl = (فراغ*)rsdt->جدول_وٿي_داخلا[i];
            إذا (!tbl || tbl->signature != signature)
                استمر;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            أرجع tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    أرجع عدم;
}

u32
ڳوليو_resume_ويڪٽر(فراغ)
{
    بنية fadt_descriptor_rev1 *fadt = ڳوليو_acpi_جدول(FACP_SIGNATURE);
    إذا (!fadt)
        أرجع 0;
    بنية facs_descriptor_rev1 *facs = (فراغ*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    إذا (! facs || facs->signature != FACS_SIGNATURE)
        أرجع 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_ويڪٽر);
    أرجع facs->firmware_waking_ويڪٽر;
}

ساكن بنية acpi_20_generic_پتو acpi_reset_reg;
ساكن u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_بنياد = 0xb000;

#وصف_ڪريو acpi_ga_منزل_bdf(addr) pci_منزل_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

فراغ
acpi_reboot(فراغ)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    إذا (acpi_reset_reg.رجسٽر_ٻٽو_انگ_ويڪر != 8)
        أرجع;

    u64 addr = le64_منزل_cpu(acpi_reset_reg.پتو);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.پتو_space_id, addr, acpi_reset_val);

    بدل (acpi_reset_reg.پتو_space_id) {
    حالة 0: // System Memory
        writeb((فراغ *)(u32)addr, acpi_reset_val);
        اكسر;
    حالة 1: // System I/O
        outb(acpi_reset_val, addr);
        اكسر;
    حالة 2: // PCI config space
        pci_config_writeb(acpi_ga_منزل_bdf(addr), addr & 0xffff, acpi_reset_val);
        اكسر;
    }
}

ساكن فراغ
acpi_سيٽ_reset_reg(بنية acpi_20_generic_پتو *reg, u8 val)
{
    إذا (!reg || reg->پتو_space_id > 2 ||
        reg->رجسٽر_ٻٽو_انگ_ويڪر != 8 || reg->رجسٽر_ٻٽو_انگ_وٿي)
        أرجع;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

فراغ
ڳوليو_acpi_features(فراغ)
{
    بنية fadt_descriptor_rev1 *fadt = ڳوليو_acpi_جدول(FACP_SIGNATURE);
    إذا (!fadt)
        أرجع;
    u32 pm_tmr = le32_منزل_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_منزل_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    إذا (pm_tmr)
        pmtimer_تياري(pm_tmr);
    إذا (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    إذا (fadt->ڊيگهائي >= 129) {
        فراغ *p = fadt;
        acpi_سيٽ_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
فراغ *
smbios_ايندڙ(فراغ *شروعات, u32 ڊيگهائي, فراغ *prev)
{
    إذا (!شروعات)
        أرجع عدم;
    فراغ *پڇاڙي = شروعات + ڊيگهائي;

    إذا (!prev) {
        prev = شروعات;
    } وإلا {
        بنية smbios_structure_سرو *hdr = prev;
        إذا (prev + حجم(*hdr) > پڇاڙي)
            أرجع عدم;
        prev += hdr->ڊيگهائي + 2;
        طالما (prev < پڇاڙي && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    بنية smbios_structure_سرو *hdr = prev;
    إذا (prev >= پڇاڙي || prev + حجم(*hdr) >= پڇاڙي || prev + hdr->ڊيگهائي >= پڇاڙي)
        أرجع عدم;
    أرجع prev;
}

فراغ *
smbios_21_ايندڙ(بنية smbios_21_داخلا_نقطو *smbios, فراغ *prev)
{
    إذا (!smbios)
        أرجع عدم;
    أرجع smbios_ايندڙ((فراغ*)smbios->structure_جدول_پتو,
                       smbios->structure_جدول_ڊيگهائي, prev);
}

ساكن بنية smbios_21_داخلا_نقطو *SMBios21Addr;

فراغ
copy_smbios_21(فراغ *pos)
{
    إذا (SMBios21Addr)
        أرجع;
    بنية smbios_21_داخلا_نقطو *p = pos;
    إذا (p->signature != SMBIOS_21_SIGNATURE)
        أرجع;
    إذا (checksum(pos, 0x10) != 0)
        أرجع;
    إذا (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        أرجع;
    إذا (checksum(pos+0x10, p->ڊيگهائي-0x10) != 0)
        أرجع;
    SMBios21Addr = copy_fseg_جدول("SMBIOS", pos, p->ڊيگهائي);
}

ساكن بنية smbios_30_داخلا_نقطو *SMBios30Addr;

ساكن صحيح
valid_smbios_30_signature(بنية smbios_30_داخلا_نقطو *p)
{
    أرجع !memcmp(p->signature, "_SM3_", 5);
}

فراغ
copy_smbios_30(فراغ *pos)
{
    إذا (SMBios30Addr)
        أرجع;
    بنية smbios_30_داخلا_نقطو *p = pos;
    إذا (!valid_smbios_30_signature(p))
        أرجع;
    إذا (checksum(pos, p->ڊيگهائي) != 0)
        أرجع;
    SMBios30Addr = copy_fseg_جدول("SMBIOS 3.0", pos, p->ڊيگهائي);
}

فراغ *smbios_get_tables(u32 *ڊيگهائي)
{
    إذا (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_جدول_پتو;
        إذا (addr32 == SMBios30Addr->structure_جدول_پتو) {
            *ڊيگهائي = SMBios30Addr->structure_جدول_max_سائيز;
            أرجع (فراغ *)addr32;
        }
    }
    إذا (SMBios21Addr) {
        *ڊيگهائي = SMBios21Addr->structure_جدول_ڊيگهائي;
        أرجع (فراغ *)SMBios21Addr->structure_جدول_پتو;
    }
    أرجع عدم;
}

ساكن صحيح
smbios_major_نسخو(فراغ)
{
    إذا (SMBios30Addr)
        أرجع SMBios30Addr->smbios_major_نسخو;
    وإلا إذا (SMBios21Addr)
        أرجع SMBios21Addr->smbios_major_نسخو;
    وإلا
        أرجع 0;
}

ساكن صحيح
smbios_minor_نسخو(فراغ)
{
    إذا (SMBios30Addr)
        أرجع SMBios30Addr->smbios_minor_نسخو;
    وإلا إذا (SMBios21Addr)
        أرجع SMBios21Addr->smbios_minor_نسخو;
    وإلا
        أرجع 0;
}

فراغ
display_uuid(فراغ)
{
    u32 smbios_len = 0;
    فراغ *smbios_tables = smbios_get_tables(&smbios_len);
    بنية smbios_type_1 *tbl = smbios_ايندڙ(smbios_tables, smbios_len, عدم);
    صحيح minlen = offsetof(بنية smbios_type_1, uuid) + حجم(tbl->uuid);
    لكل (; tbl; tbl = smbios_ايندڙ(smbios_tables, smbios_len, tbl))
        إذا (tbl->سرو.type == 1 && tbl->سرو.ڊيگهائي >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 خالي_جايون_uuid[حجم(tbl->uuid)] = { 0 };
            إذا (memcmp(uuid, خالي_جايون_uuid, حجم(خالي_جايون_uuid)) == 0)
                أرجع;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            إذا (smbios_major_نسخو() > 2 ||
                (smbios_major_نسخو() == 2 &&
                 smbios_minor_نسخو() >= 6)) {
                لکو("Machine UUID"
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
            } وإلا {
                لکو("Machine UUID"
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

            أرجع;
        }
}

#وصف_ڪريو سيٽ_str_ميدان_or_skip(type, ميدان, قدر)                       \
    افعل {                                                                \
        صحيح سائيز = (قدر != عدم) ? لڙي_ڊيگهه(قدر) + 1 : 0;             \
        إذا (سائيز > 1) {                                                 \
            يادگيري_نقل(پڇاڙي, قدر, سائيز);                                   \
            پڇاڙي += سائيز;                                                \
            p->ميدان = ++str_اشاريو;                                     \
        } وإلا {                                                        \
            p->ميدان = 0;                                               \
        }                                                               \
    } طالما (0)

ساكن فراغ *
smbios_new_type_0(فراغ *شروعات,
                  ثابت حرف *vendor, ثابت حرف *نسخو_2, ثابت حرف *date)
{
    بنية smbios_type_0 *p = (بنية smbios_type_0 *)شروعات;
    حرف *پڇاڙي = (حرف *)شروعات + حجم(بنية smbios_type_0);
    صحيح str_اشاريو = 0;

    p->سرو.type = 0;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_0);
    p->سرو.handle = 0;

    سيٽ_str_ميدان_or_skip(0, vendor_str, vendor);
    سيٽ_str_ميدان_or_skip(0, bios_نسخو_str, نسخو_2);
    p->bios_starting_پتو_segment = 0xe800;
    سيٽ_str_ميدان_or_skip(0, bios_release_date_str, date);

    p->bios_rom_سائيز = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_بائيٽون[0] = 0;
    p->bios_characteristics_extension_بائيٽون[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *پڇاڙي = 0;
    پڇاڙي++;
    إذا (!str_اشاريو) {
        *پڇاڙي = 0;
        پڇاڙي++;
    }

    أرجع پڇاڙي;
}

#وصف_ڪريو BIOS_NAME "SeaBIOS"
#وصف_ڪريو BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
ساكن صحيح
smbios_build_tables(بنية romfile_s *f_tables,
                    u64 *پتو, u32 *ڊيگهائي,
                    u16 *max_structure_سائيز,
                    u16 *عدد_جو_structures)
{
    بنية smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    إذا (f_tables->سائيز != *ڊيگهائي)
        أرجع 0;

    qtables = malloc_tmphigh(f_tables->سائيز);
    إذا (!qtables) {
        warn_noalloc();
        أرجع 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->سائيز);
    qtables_len = f_tables->سائيز;

    /* did we get a type 0 structure ? */
    لكل (t0 = smbios_ايندڙ(qtables, qtables_len, عدم); t0;
         t0 = smbios_ايندڙ(qtables, qtables_len, t0)) {
        إذا (t0->سرو.type == 0) {
            need_t0 = 0;
            اكسر;
        }
    }

    إذا (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = حجم(بنية smbios_type_0) + لڙي_ڊيگهه(BIOS_NAME) +
                     لڙي_ڊيگهه(نسخو) + لڙي_ڊيگهه(BIOS_DATE) + 4;
        إذا (t0_len > (0xffff - *ڊيگهائي)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *ڊيگهائي, t0_len);
            need_t0 = 0;
        } وإلا {
            *ڊيگهائي += t0_len;
            إذا (max_structure_سائيز && t0_len > *max_structure_سائيز)
                *max_structure_سائيز = t0_len;
            إذا (عدد_جو_structures)
                (*عدد_جو_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    إذا (*ڊيگهائي > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_مٿاهون(*ڊيگهائي);
    وإلا
        tables = malloc_fseg(*ڊيگهائي);
    إذا (!tables) {
        warn_noalloc();
        حرر(qtables);
        أرجع 0;
    }
    *پتو = (u32)tables;

    /* populate final blob */
    إذا (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, نسخو, BIOS_DATE);
    يادگيري_نقل(tables, qtables, qtables_len);
    حرر(qtables);
    أرجع 1;
}

ساكن صحيح
smbios_21_تياري_داخلا_نقطو(بنية romfile_s *f_tables,
                            بنية smbios_21_داخلا_نقطو *ep)
{
    u64 پتو = ep->structure_جدول_پتو;
    u32 ڊيگهائي = ep->structure_جدول_ڊيگهائي;

    إذا (!smbios_build_tables(f_tables,
                             &پتو,
                             &ڊيگهائي,
                             &ep->max_structure_سائيز,
                             &ep->عدد_جو_structures))
        أرجع 0;

    إذا ((u32)پتو != پتو || (u16)ڊيگهائي != ڊيگهائي) {
        warn_internalerror();
        أرجع 0;
    }

    /* finalize entry point */
    ep->structure_جدول_پتو = پتو;
    ep->structure_جدول_ڊيگهائي = ڊيگهائي;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((فراغ *)ep + 0x10, ep->ڊيگهائي - 0x10);

    copy_smbios_21(ep);
    أرجع 1;
}

ساكن صحيح
smbios_30_تياري_داخلا_نقطو(بنية romfile_s *f_tables,
                            بنية smbios_30_داخلا_نقطو *ep)
{
    إذا (!smbios_build_tables(f_tables,
                             &ep->structure_جدول_پتو,
                             &ep->structure_جدول_max_سائيز,
                             عدم, عدم))
        أرجع 0;

    ep->checksum -= checksum(ep, حجم(*ep));
    copy_smbios_30(ep);
    أرجع 1;
}

ساكن صحيح
smbios_romfile_تياري(فراغ)
{
    بنية romfile_s *f_anchor = romfile_ڳوليو("etc/smbios/smbios-anchor");
    بنية romfile_s *f_tables = romfile_ڳوليو("etc/smbios/smbios-tables");
    اتحاد {
        بنية smbios_21_داخلا_نقطو ep21;
        بنية smbios_30_داخلا_نقطو ep30;
    } ep;

    إذا (!f_anchor || !f_tables || f_anchor->سائيز > حجم(ep))
        أرجع 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->سائيز);

    إذا (f_anchor->سائيز == حجم(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        أرجع smbios_21_تياري_داخلا_نقطو(f_tables, &ep.ep21);
    } وإلا إذا (f_anchor->سائيز == حجم(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        أرجع smbios_30_تياري_داخلا_نقطو(f_tables, &ep.ep30);
    } وإلا {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        أرجع 0;
    }
}

فراغ
smbios_تياري(فراغ)
{
    إذا (smbios_romfile_تياري())
        أرجع;
    smbios_legacy_تياري();
}

فراغ
copy_جدول(فراغ *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
