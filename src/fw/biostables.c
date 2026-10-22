// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "byteorder.h" // le32_to_cpu
#شامل "config.h" // CONFIG_*
#شامل "hw/pci.h" // pci_config_writeb
#شامل "malloc.h" // malloc_fseg
#شامل "memmap.h" //علامت
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_find
#شامل "std/acpi.h" // struct rsdp_descriptor
#شامل "std/mptable.h" // MPTABLE_SIGNATURE
#شامل "std/pirtable.h" // struct pir_header
#شامل "std/smbios.h" // struct smbios_21_entry_point
#شامل "string.h" // memcpy
#شامل "util.h" // copy_table
#شامل "x86.h" // outb

ساخت pir_سرخی *PirAddr VARFSEG;

ساکن خالی *
copy_fseg_جدول(ثابت حرف *name, خالی *pos, u32 سائز)
{
    خالی *newpos = malloc_fseg(سائز);
    اگر (!newpos) {
        warn_noalloc();
        واپس تہی;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    حافظہ_نقل(newpos, pos, سائز);
    واپس newpos;
}

خالی
copy_pir(خالی *pos)
{
    ساخت pir_سرخی *p = pos;
    اگر (p->signature != PIR_SIGNATURE)
        واپس;
    اگر (PirAddr)
        واپس;
    اگر (p->سائز < حجم(*p))
        واپس;
    اگر (checksum(pos, p->سائز) != 0)
        واپس;
    PirAddr = copy_fseg_جدول("PIR", pos, p->سائز);
}

خالی
copy_mptable(خالی *pos)
{
    ساخت mptable_floating_s *p = pos;
    اگر (p->signature != MPTABLE_SIGNATURE)
        واپس;
    اگر (!p->physaddr)
        واپس;
    اگر (checksum(pos, حجم(*p)) != 0)
        واپس;
    u32 لمبائی = p->لمبائی * 16;
    u16 mpclength = ((ساخت mptable_config_s *)p->physaddr)->لمبائی;
    اگر (لمبائی + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , لمبائی + mpclength);
        واپس;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    ساخت mptable_floating_s *newpos = malloc_fseg(لمبائی + mpclength);
    اگر (!newpos) {
        warn_noalloc();
        واپس;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    حافظہ_نقل(newpos, pos, لمبائی);
    newpos->physaddr = (u32)newpos + لمبائی;
    newpos->checksum -= checksum(newpos, حجم(*newpos));
    حافظہ_نقل((خالی*)newpos + لمبائی, (خالی*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ساکن صحیح
get_acpi_rsdp_لمبائی(خالی *pos, غیرعلامتی سائز)
{
    ساخت rsdp_descriptor *p = pos;
    اگر (p->signature != RSDP_SIGNATURE)
        واپس -1;
    u32 لمبائی = 20;
    اگر (لمبائی > سائز)
        واپس -1;
    اگر (checksum(pos, لمبائی) != 0)
        واپس -1;
    اگر (p->revision > 1) {
        لمبائی = p->لمبائی;
        اگر (لمبائی > سائز)
            واپس -1;
        اگر (checksum(pos, لمبائی) != 0)
            واپس -1;
    }
    واپس لمبائی;
}

ساخت rsdp_descriptor *RsdpAddr;

خالی
copy_acpi_rsdp(خالی *pos)
{
    اگر (RsdpAddr)
        واپس;
    صحیح لمبائی = get_acpi_rsdp_لمبائی(pos, -1);
    اگر (لمبائی < 0)
        واپس;
    RsdpAddr = copy_fseg_جدول("ACPI RSDP", pos, لمبائی);
}

خالی *ڈھونڈو_acpi_rsdp(خالی)
{
    غیرعلامتی طویل آغاز = علامت(zonefseg_آغاز);
    غیرعلامتی طویل انتہا = علامت(zonefseg_انتہا);
    غیرعلامتی طویل pos;

    برائے (pos = ALIGN(آغاز, 0x10); pos <= ALIGN_نیچے_کو(انتہا, 0x10); pos += 0x10)
        اگر (get_acpi_rsdp_لمبائی((خالی *)pos, انتہا - pos) >= 0)
            واپس (خالی *)pos;

    واپس تہی;
}

خالی *
ڈھونڈو_acpi_جدول(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    اگر (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        واپس تہی;
    ساخت rsdt_descriptor_rev1 *rsdt = (خالی*)RsdpAddr->rsdt_physical_پتہ;
    ساخت xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_پتہ >= 0x100000000
        ? تہی : (خالی*)(u32)(RsdpAddr->xsdt_physical_پتہ);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    اگر (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        خالی *انتہا = (خالی*)xsdt + xsdt->لمبائی;
        صحیح i;
        برائے (i=0; (خالی*)&xsdt->جدول_انحراف_اندراج[i] < انتہا; i++) {
            اگر (xsdt->جدول_انحراف_اندراج[i] >= 0x100000000)
                جاری; /* above 4G */
            ساخت acpi_جدول_سرخی *tbl = (خالی*)(u32)xsdt->جدول_انحراف_اندراج[i];
            اگر (!tbl || tbl->signature != signature)
                جاری;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            واپس tbl;
        }
    }

    اگر (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        خالی *انتہا = (خالی*)rsdt + rsdt->لمبائی;
        صحیح i;
        برائے (i=0; (خالی*)&rsdt->جدول_انحراف_اندراج[i] < انتہا; i++) {
            ساخت acpi_جدول_سرخی *tbl = (خالی*)rsdt->جدول_انحراف_اندراج[i];
            اگر (!tbl || tbl->signature != signature)
                جاری;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            واپس tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    واپس تہی;
}

u32
ڈھونڈو_resume_سمتیہ(خالی)
{
    ساخت fadt_descriptor_rev1 *fadt = ڈھونڈو_acpi_جدول(FACP_SIGNATURE);
    اگر (!fadt)
        واپس 0;
    ساخت facs_descriptor_rev1 *facs = (خالی*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    اگر (! facs || facs->signature != FACS_SIGNATURE)
        واپس 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_سمتیہ);
    واپس facs->firmware_waking_سمتیہ;
}

ساکن ساخت acpi_20_generic_پتہ acpi_reset_reg;
ساکن u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_بنیاد = 0xb000;

#تعریف_کرو acpi_ga_مقصد_bdf(addr) pci_مقصد_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

خالی
acpi_reboot(خالی)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    اگر (acpi_reset_reg.رجسٹر_ثنائی_ہندسہ_چوڑائی != 8)
        واپس;

    u64 addr = le64_مقصد_cpu(acpi_reset_reg.پتہ);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.پتہ_space_id, addr, acpi_reset_val);

    انتخاب (acpi_reset_reg.پتہ_space_id) {
    حالت 0: // System Memory
        writeb((خالی *)(u32)addr, acpi_reset_val);
        توڑو;
    حالت 1: // System I/O
        outb(acpi_reset_val, addr);
        توڑو;
    حالت 2: // PCI config space
        pci_config_writeb(acpi_ga_مقصد_bdf(addr), addr & 0xffff, acpi_reset_val);
        توڑو;
    }
}

ساکن خالی
acpi_سیٹ_reset_reg(ساخت acpi_20_generic_پتہ *reg, u8 val)
{
    اگر (!reg || reg->پتہ_space_id > 2 ||
        reg->رجسٹر_ثنائی_ہندسہ_چوڑائی != 8 || reg->رجسٹر_ثنائی_ہندسہ_انحراف)
        واپس;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

خالی
ڈھونڈو_acpi_features(خالی)
{
    ساخت fadt_descriptor_rev1 *fadt = ڈھونڈو_acpi_جدول(FACP_SIGNATURE);
    اگر (!fadt)
        واپس;
    u32 pm_tmr = le32_مقصد_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_مقصد_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    اگر (pm_tmr)
        pmtimer_تیاری(pm_tmr);
    اگر (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    اگر (fadt->لمبائی >= 129) {
        خالی *p = fadt;
        acpi_سیٹ_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
خالی *
smbios_اگلا(خالی *آغاز, u32 لمبائی, خالی *پچھلا)
{
    اگر (!آغاز)
        واپس تہی;
    خالی *انتہا = آغاز + لمبائی;

    اگر (!پچھلا) {
        پچھلا = آغاز;
    } ورنہ {
        ساخت smbios_structure_سرخی *hdr = پچھلا;
        اگر (پچھلا + حجم(*hdr) > انتہا)
            واپس تہی;
        پچھلا += hdr->لمبائی + 2;
        جب (پچھلا < انتہا && (*(u8*)(پچھلا-1) != '\0' || *(u8*)(پچھلا-2) != '\0'))
            پچھلا++;
    }
    ساخت smbios_structure_سرخی *hdr = پچھلا;
    اگر (پچھلا >= انتہا || پچھلا + حجم(*hdr) >= انتہا || پچھلا + hdr->لمبائی >= انتہا)
        واپس تہی;
    واپس پچھلا;
}

خالی *
smbios_21_اگلا(ساخت smbios_21_اندراج_نقطہ *smbios, خالی *پچھلا)
{
    اگر (!smbios)
        واپس تہی;
    واپس smbios_اگلا((خالی*)smbios->structure_جدول_پتہ,
                       smbios->structure_جدول_لمبائی, پچھلا);
}

ساکن ساخت smbios_21_اندراج_نقطہ *SMBios21Addr;

خالی
copy_smbios_21(خالی *pos)
{
    اگر (SMBios21Addr)
        واپس;
    ساخت smbios_21_اندراج_نقطہ *p = pos;
    اگر (p->signature != SMBIOS_21_SIGNATURE)
        واپس;
    اگر (checksum(pos, 0x10) != 0)
        واپس;
    اگر (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        واپس;
    اگر (checksum(pos+0x10, p->لمبائی-0x10) != 0)
        واپس;
    SMBios21Addr = copy_fseg_جدول("SMBIOS", pos, p->لمبائی);
}

ساکن ساخت smbios_30_اندراج_نقطہ *SMBios30Addr;

ساکن صحیح
valid_smbios_30_signature(ساخت smbios_30_اندراج_نقطہ *p)
{
    واپس !memcmp(p->signature, "_SM3_", 5);
}

خالی
copy_smbios_30(خالی *pos)
{
    اگر (SMBios30Addr)
        واپس;
    ساخت smbios_30_اندراج_نقطہ *p = pos;
    اگر (!valid_smbios_30_signature(p))
        واپس;
    اگر (checksum(pos, p->لمبائی) != 0)
        واپس;
    SMBios30Addr = copy_fseg_جدول("SMBIOS 3.0", pos, p->لمبائی);
}

خالی *smbios_get_tables(u32 *لمبائی)
{
    اگر (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_جدول_پتہ;
        اگر (addr32 == SMBios30Addr->structure_جدول_پتہ) {
            *لمبائی = SMBios30Addr->structure_جدول_max_سائز;
            واپس (خالی *)addr32;
        }
    }
    اگر (SMBios21Addr) {
        *لمبائی = SMBios21Addr->structure_جدول_لمبائی;
        واپس (خالی *)SMBios21Addr->structure_جدول_پتہ;
    }
    واپس تہی;
}

ساکن صحیح
smbios_major_نسخہ(خالی)
{
    اگر (SMBios30Addr)
        واپس SMBios30Addr->smbios_major_نسخہ;
    ورنہ اگر (SMBios21Addr)
        واپس SMBios21Addr->smbios_major_نسخہ;
    ورنہ
        واپس 0;
}

ساکن صحیح
smbios_minor_نسخہ(خالی)
{
    اگر (SMBios30Addr)
        واپس SMBios30Addr->smbios_minor_نسخہ;
    ورنہ اگر (SMBios21Addr)
        واپس SMBios21Addr->smbios_minor_نسخہ;
    ورنہ
        واپس 0;
}

خالی
display_uuid(خالی)
{
    u32 smbios_len = 0;
    خالی *smbios_tables = smbios_get_tables(&smbios_len);
    ساخت smbios_type_1 *tbl = smbios_اگلا(smbios_tables, smbios_len, تہی);
    صحیح minlen = offsetof(ساخت smbios_type_1, uuid) + حجم(tbl->uuid);
    برائے (; tbl; tbl = smbios_اگلا(smbios_tables, smbios_len, tbl))
        اگر (tbl->سرخی.type == 1 && tbl->سرخی.لمبائی >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 خالی_خانے_uuid[حجم(tbl->uuid)] = { 0 };
            اگر (memcmp(uuid, خالی_خانے_uuid, حجم(خالی_خانے_uuid)) == 0)
                واپس;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            اگر (smbios_major_نسخہ() > 2 ||
                (smbios_major_نسخہ() == 2 &&
                 smbios_minor_نسخہ() >= 6)) {
                لکھو("Machine UUID"
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
            } ورنہ {
                لکھو("Machine UUID"
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

            واپس;
        }
}

#تعریف_کرو سیٹ_str_میدان_or_skip(type, میدان, قدر)                       \
    کرو {                                                                \
        صحیح سائز = (قدر != تہی) ? لڑی_لمبائی(قدر) + 1 : 0;             \
        اگر (سائز > 1) {                                                 \
            حافظہ_نقل(انتہا, قدر, سائز);                                   \
            انتہا += سائز;                                                \
            p->میدان = ++str_اشاریہ;                                     \
        } ورنہ {                                                        \
            p->میدان = 0;                                               \
        }                                                               \
    } جب (0)

ساکن خالی *
smbios_new_type_0(خالی *آغاز,
                  ثابت حرف *vendor, ثابت حرف *نسخہ_2, ثابت حرف *date)
{
    ساخت smbios_type_0 *p = (ساخت smbios_type_0 *)آغاز;
    حرف *انتہا = (حرف *)آغاز + حجم(ساخت smbios_type_0);
    صحیح str_اشاریہ = 0;

    p->سرخی.type = 0;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_0);
    p->سرخی.handle = 0;

    سیٹ_str_میدان_or_skip(0, vendor_str, vendor);
    سیٹ_str_میدان_or_skip(0, bios_نسخہ_str, نسخہ_2);
    p->bios_starting_پتہ_segment = 0xe800;
    سیٹ_str_میدان_or_skip(0, bios_release_date_str, date);

    p->bios_rom_سائز = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_بائٹیں[0] = 0;
    p->bios_characteristics_extension_بائٹیں[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *انتہا = 0;
    انتہا++;
    اگر (!str_اشاریہ) {
        *انتہا = 0;
        انتہا++;
    }

    واپس انتہا;
}

#تعریف_کرو BIOS_NAME "SeaBIOS"
#تعریف_کرو BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
ساکن صحیح
smbios_build_tables(ساخت romfile_s *f_tables,
                    u64 *پتہ, u32 *لمبائی,
                    u16 *max_structure_سائز,
                    u16 *نمبر_کا_structures)
{
    ساخت smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    اگر (f_tables->سائز != *لمبائی)
        واپس 0;

    qtables = malloc_tmphigh(f_tables->سائز);
    اگر (!qtables) {
        warn_noalloc();
        واپس 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->سائز);
    qtables_len = f_tables->سائز;

    /* did we get a type 0 structure ? */
    برائے (t0 = smbios_اگلا(qtables, qtables_len, تہی); t0;
         t0 = smbios_اگلا(qtables, qtables_len, t0)) {
        اگر (t0->سرخی.type == 0) {
            need_t0 = 0;
            توڑو;
        }
    }

    اگر (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = حجم(ساخت smbios_type_0) + لڑی_لمبائی(BIOS_NAME) +
                     لڑی_لمبائی(نسخہ) + لڑی_لمبائی(BIOS_DATE) + 4;
        اگر (t0_len > (0xffff - *لمبائی)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *لمبائی, t0_len);
            need_t0 = 0;
        } ورنہ {
            *لمبائی += t0_len;
            اگر (max_structure_سائز && t0_len > *max_structure_سائز)
                *max_structure_سائز = t0_len;
            اگر (نمبر_کا_structures)
                (*نمبر_کا_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    اگر (*لمبائی > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_بلند(*لمبائی);
    ورنہ
        tables = malloc_fseg(*لمبائی);
    اگر (!tables) {
        warn_noalloc();
        آزاد(qtables);
        واپس 0;
    }
    *پتہ = (u32)tables;

    /* populate final blob */
    اگر (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, نسخہ, BIOS_DATE);
    حافظہ_نقل(tables, qtables, qtables_len);
    آزاد(qtables);
    واپس 1;
}

ساکن صحیح
smbios_21_تیاری_اندراج_نقطہ(ساخت romfile_s *f_tables,
                            ساخت smbios_21_اندراج_نقطہ *ep)
{
    u64 پتہ = ep->structure_جدول_پتہ;
    u32 لمبائی = ep->structure_جدول_لمبائی;

    اگر (!smbios_build_tables(f_tables,
                             &پتہ,
                             &لمبائی,
                             &ep->max_structure_سائز,
                             &ep->نمبر_کا_structures))
        واپس 0;

    اگر ((u32)پتہ != پتہ || (u16)لمبائی != لمبائی) {
        warn_internalerror();
        واپس 0;
    }

    /* finalize entry point */
    ep->structure_جدول_پتہ = پتہ;
    ep->structure_جدول_لمبائی = لمبائی;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((خالی *)ep + 0x10, ep->لمبائی - 0x10);

    copy_smbios_21(ep);
    واپس 1;
}

ساکن صحیح
smbios_30_تیاری_اندراج_نقطہ(ساخت romfile_s *f_tables,
                            ساخت smbios_30_اندراج_نقطہ *ep)
{
    اگر (!smbios_build_tables(f_tables,
                             &ep->structure_جدول_پتہ,
                             &ep->structure_جدول_max_سائز,
                             تہی, تہی))
        واپس 0;

    ep->checksum -= checksum(ep, حجم(*ep));
    copy_smbios_30(ep);
    واپس 1;
}

ساکن صحیح
smbios_romfile_تیاری(خالی)
{
    ساخت romfile_s *f_anchor = romfile_ڈھونڈو("etc/smbios/smbios-anchor");
    ساخت romfile_s *f_tables = romfile_ڈھونڈو("etc/smbios/smbios-tables");
    اتحاد {
        ساخت smbios_21_اندراج_نقطہ ep21;
        ساخت smbios_30_اندراج_نقطہ ep30;
    } ep;

    اگر (!f_anchor || !f_tables || f_anchor->سائز > حجم(ep))
        واپس 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->سائز);

    اگر (f_anchor->سائز == حجم(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        واپس smbios_21_تیاری_اندراج_نقطہ(f_tables, &ep.ep21);
    } ورنہ اگر (f_anchor->سائز == حجم(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        واپس smbios_30_تیاری_اندراج_نقطہ(f_tables, &ep.ep30);
    } ورنہ {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        واپس 0;
    }
}

خالی
smbios_تیاری(خالی)
{
    اگر (smbios_romfile_تیاری())
        واپس;
    smbios_legacy_تیاری();
}

خالی
copy_جدول(خالی *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
