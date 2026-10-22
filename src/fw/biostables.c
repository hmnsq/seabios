// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "byteorder.h" // le32_to_cpu
#ضمن "config.h" // CONFIG_*
#ضمن "hw/pci.h" // pci_config_writeb
#ضمن "malloc.h" // malloc_fseg
#ضمن "memmap.h" //نماد
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_find
#ضمن "std/acpi.h" // struct rsdp_descriptor
#ضمن "std/mptable.h" // MPTABLE_SIGNATURE
#ضمن "std/pirtable.h" // struct pir_header
#ضمن "std/smbios.h" // struct smbios_21_entry_point
#ضمن "string.h" // memcpy
#ضمن "util.h" // copy_table
#ضمن "x86.h" // outb

بنية pir_سرآیند *PirAddr VARFSEG;

ساكن فراغ *
copy_fseg_جدول(ثابت حرف *name, فراغ *pos, u32 اندازه)
{
    فراغ *newpos = malloc_fseg(اندازه);
    إذا (!newpos) {
        warn_noalloc();
        أرجع عدم;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    کپی_حافظه(newpos, pos, اندازه);
    أرجع newpos;
}

فراغ
copy_pir(فراغ *pos)
{
    بنية pir_سرآیند *p = pos;
    إذا (p->signature != PIR_SIGNATURE)
        أرجع;
    إذا (PirAddr)
        أرجع;
    إذا (p->اندازه < حجم(*p))
        أرجع;
    إذا (checksum(pos, p->اندازه) != 0)
        أرجع;
    PirAddr = copy_fseg_جدول("PIR", pos, p->اندازه);
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
    u32 درازی = p->درازی * 16;
    u16 mpclength = ((بنية mptable_config_s *)p->physaddr)->درازی;
    إذا (درازی + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , درازی + mpclength);
        أرجع;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    بنية mptable_floating_s *newpos = malloc_fseg(درازی + mpclength);
    إذا (!newpos) {
        warn_noalloc();
        أرجع;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    کپی_حافظه(newpos, pos, درازی);
    newpos->physaddr = (u32)newpos + درازی;
    newpos->checksum -= checksum(newpos, حجم(*newpos));
    کپی_حافظه((فراغ*)newpos + درازی, (فراغ*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ساكن صحيح
get_acpi_rsdp_درازی(فراغ *pos, بیعلامت اندازه)
{
    بنية rsdp_descriptor *p = pos;
    إذا (p->signature != RSDP_SIGNATURE)
        أرجع -1;
    u32 درازی = 20;
    إذا (درازی > اندازه)
        أرجع -1;
    إذا (checksum(pos, درازی) != 0)
        أرجع -1;
    إذا (p->revision > 1) {
        درازی = p->درازی;
        إذا (درازی > اندازه)
            أرجع -1;
        إذا (checksum(pos, درازی) != 0)
            أرجع -1;
    }
    أرجع درازی;
}

بنية rsdp_descriptor *RsdpAddr;

فراغ
copy_acpi_rsdp(فراغ *pos)
{
    إذا (RsdpAddr)
        أرجع;
    صحيح درازی = get_acpi_rsdp_درازی(pos, -1);
    إذا (درازی < 0)
        أرجع;
    RsdpAddr = copy_fseg_جدول("ACPI RSDP", pos, درازی);
}

فراغ *پیدا_کن_acpi_rsdp(فراغ)
{
    بیعلامت دراز شروع = نماد(zonefseg_شروع);
    بیعلامت دراز انتها = نماد(zonefseg_انتها);
    بیعلامت دراز pos;

    لكل (pos = ALIGN(شروع, 0x10); pos <= ALIGN_پایین(انتها, 0x10); pos += 0x10)
        إذا (get_acpi_rsdp_درازی((فراغ *)pos, انتها - pos) >= 0)
            أرجع (فراغ *)pos;

    أرجع عدم;
}

فراغ *
پیدا_کن_acpi_جدول(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    إذا (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        أرجع عدم;
    بنية rsdt_descriptor_rev1 *rsdt = (فراغ*)RsdpAddr->rsdt_physical_آدرس;
    بنية xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_آدرس >= 0x100000000
        ? عدم : (فراغ*)(u32)(RsdpAddr->xsdt_physical_آدرس);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    إذا (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        فراغ *انتها = (فراغ*)xsdt + xsdt->درازی;
        صحيح i;
        لكل (i=0; (فراغ*)&xsdt->جدول_جابه_جایی_مدخل[i] < انتها; i++) {
            إذا (xsdt->جدول_جابه_جایی_مدخل[i] >= 0x100000000)
                استمر; /* above 4G */
            بنية acpi_جدول_سرآیند *tbl = (فراغ*)(u32)xsdt->جدول_جابه_جایی_مدخل[i];
            إذا (!tbl || tbl->signature != signature)
                استمر;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            أرجع tbl;
        }
    }

    إذا (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        فراغ *انتها = (فراغ*)rsdt + rsdt->درازی;
        صحيح i;
        لكل (i=0; (فراغ*)&rsdt->جدول_جابه_جایی_مدخل[i] < انتها; i++) {
            بنية acpi_جدول_سرآیند *tbl = (فراغ*)rsdt->جدول_جابه_جایی_مدخل[i];
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
پیدا_کن_resume_وکتور(فراغ)
{
    بنية fadt_descriptor_rev1 *fadt = پیدا_کن_acpi_جدول(FACP_SIGNATURE);
    إذا (!fadt)
        أرجع 0;
    بنية facs_descriptor_rev1 *facs = (فراغ*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    إذا (! facs || facs->signature != FACS_SIGNATURE)
        أرجع 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_وکتور);
    أرجع facs->firmware_waking_وکتور;
}

ساكن بنية acpi_20_generic_آدرس acpi_reset_reg;
ساكن u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_پایه = 0xb000;

#تعریف_کن acpi_ga_مقصد_bdf(addr) pci_مقصد_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

فراغ
acpi_reboot(فراغ)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    إذا (acpi_reset_reg.ثبات_بیت_عرض != 8)
        أرجع;

    u64 addr = le64_مقصد_cpu(acpi_reset_reg.آدرس);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.آدرس_space_id, addr, acpi_reset_val);

    بدل (acpi_reset_reg.آدرس_space_id) {
    حالة 0: // System Memory
        writeb((فراغ *)(u32)addr, acpi_reset_val);
        اكسر;
    حالة 1: // System I/O
        outb(acpi_reset_val, addr);
        اكسر;
    حالة 2: // PCI config space
        pci_config_writeb(acpi_ga_مقصد_bdf(addr), addr & 0xffff, acpi_reset_val);
        اكسر;
    }
}

ساكن فراغ
acpi_مجموعه_reset_reg(بنية acpi_20_generic_آدرس *reg, u8 val)
{
    إذا (!reg || reg->آدرس_space_id > 2 ||
        reg->ثبات_بیت_عرض != 8 || reg->ثبات_بیت_جابه_جایی)
        أرجع;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

فراغ
پیدا_کن_acpi_features(فراغ)
{
    بنية fadt_descriptor_rev1 *fadt = پیدا_کن_acpi_جدول(FACP_SIGNATURE);
    إذا (!fadt)
        أرجع;
    u32 pm_tmr = le32_مقصد_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_مقصد_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    إذا (pm_tmr)
        pmtimer_راهاندازی(pm_tmr);
    إذا (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    إذا (fadt->درازی >= 129) {
        فراغ *p = fadt;
        acpi_مجموعه_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
فراغ *
smbios_بعدی(فراغ *شروع, u32 درازی, فراغ *prev)
{
    إذا (!شروع)
        أرجع عدم;
    فراغ *انتها = شروع + درازی;

    إذا (!prev) {
        prev = شروع;
    } وإلا {
        بنية smbios_structure_سرآیند *hdr = prev;
        إذا (prev + حجم(*hdr) > انتها)
            أرجع عدم;
        prev += hdr->درازی + 2;
        طالما (prev < انتها && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    بنية smbios_structure_سرآیند *hdr = prev;
    إذا (prev >= انتها || prev + حجم(*hdr) >= انتها || prev + hdr->درازی >= انتها)
        أرجع عدم;
    أرجع prev;
}

فراغ *
smbios_21_بعدی(بنية smbios_21_مدخل_نقطه *smbios, فراغ *prev)
{
    إذا (!smbios)
        أرجع عدم;
    أرجع smbios_بعدی((فراغ*)smbios->structure_جدول_آدرس,
                       smbios->structure_جدول_درازی, prev);
}

ساكن بنية smbios_21_مدخل_نقطه *SMBios21Addr;

فراغ
copy_smbios_21(فراغ *pos)
{
    إذا (SMBios21Addr)
        أرجع;
    بنية smbios_21_مدخل_نقطه *p = pos;
    إذا (p->signature != SMBIOS_21_SIGNATURE)
        أرجع;
    إذا (checksum(pos, 0x10) != 0)
        أرجع;
    إذا (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        أرجع;
    إذا (checksum(pos+0x10, p->درازی-0x10) != 0)
        أرجع;
    SMBios21Addr = copy_fseg_جدول("SMBIOS", pos, p->درازی);
}

ساكن بنية smbios_30_مدخل_نقطه *SMBios30Addr;

ساكن صحيح
valid_smbios_30_signature(بنية smbios_30_مدخل_نقطه *p)
{
    أرجع !memcmp(p->signature, "_SM3_", 5);
}

فراغ
copy_smbios_30(فراغ *pos)
{
    إذا (SMBios30Addr)
        أرجع;
    بنية smbios_30_مدخل_نقطه *p = pos;
    إذا (!valid_smbios_30_signature(p))
        أرجع;
    إذا (checksum(pos, p->درازی) != 0)
        أرجع;
    SMBios30Addr = copy_fseg_جدول("SMBIOS 3.0", pos, p->درازی);
}

فراغ *smbios_get_tables(u32 *درازی)
{
    إذا (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_جدول_آدرس;
        إذا (addr32 == SMBios30Addr->structure_جدول_آدرس) {
            *درازی = SMBios30Addr->structure_جدول_max_اندازه;
            أرجع (فراغ *)addr32;
        }
    }
    إذا (SMBios21Addr) {
        *درازی = SMBios21Addr->structure_جدول_درازی;
        أرجع (فراغ *)SMBios21Addr->structure_جدول_آدرس;
    }
    أرجع عدم;
}

ساكن صحيح
smbios_major_نسخه(فراغ)
{
    إذا (SMBios30Addr)
        أرجع SMBios30Addr->smbios_major_نسخه;
    وإلا إذا (SMBios21Addr)
        أرجع SMBios21Addr->smbios_major_نسخه;
    وإلا
        أرجع 0;
}

ساكن صحيح
smbios_minor_نسخه(فراغ)
{
    إذا (SMBios30Addr)
        أرجع SMBios30Addr->smbios_minor_نسخه;
    وإلا إذا (SMBios21Addr)
        أرجع SMBios21Addr->smbios_minor_نسخه;
    وإلا
        أرجع 0;
}

فراغ
display_uuid(فراغ)
{
    u32 smbios_len = 0;
    فراغ *smbios_tables = smbios_get_tables(&smbios_len);
    بنية smbios_type_1 *tbl = smbios_بعدی(smbios_tables, smbios_len, عدم);
    صحيح minlen = offsetof(بنية smbios_type_1, uuid) + حجم(tbl->uuid);
    لكل (; tbl; tbl = smbios_بعدی(smbios_tables, smbios_len, tbl))
        إذا (tbl->سرآیند.type == 1 && tbl->سرآیند.درازی >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 خالی_uuid[حجم(tbl->uuid)] = { 0 };
            إذا (memcmp(uuid, خالی_uuid, حجم(خالی_uuid)) == 0)
                أرجع;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            إذا (smbios_major_نسخه() > 2 ||
                (smbios_major_نسخه() == 2 &&
                 smbios_minor_نسخه() >= 6)) {
                چاپ_کن("Machine UUID"
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
                چاپ_کن("Machine UUID"
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

#تعریف_کن مجموعه_str_فیلد_or_skip(type, فیلد, مقدار)                       \
    افعل {                                                                \
        صحيح اندازه = (مقدار != عدم) ? طول_رشته(مقدار) + 1 : 0;             \
        إذا (اندازه > 1) {                                                 \
            کپی_حافظه(انتها, مقدار, اندازه);                                   \
            انتها += اندازه;                                                \
            p->فیلد = ++str_شاخص;                                     \
        } وإلا {                                                        \
            p->فیلد = 0;                                               \
        }                                                               \
    } طالما (0)

ساكن فراغ *
smbios_new_type_0(فراغ *شروع,
                  ثابت حرف *vendor, ثابت حرف *نسخه_2, ثابت حرف *date)
{
    بنية smbios_type_0 *p = (بنية smbios_type_0 *)شروع;
    حرف *انتها = (حرف *)شروع + حجم(بنية smbios_type_0);
    صحيح str_شاخص = 0;

    p->سرآیند.type = 0;
    p->سرآیند.درازی = حجم(بنية smbios_type_0);
    p->سرآیند.handle = 0;

    مجموعه_str_فیلد_or_skip(0, vendor_str, vendor);
    مجموعه_str_فیلد_or_skip(0, bios_نسخه_str, نسخه_2);
    p->bios_starting_آدرس_segment = 0xe800;
    مجموعه_str_فیلد_or_skip(0, bios_release_date_str, date);

    p->bios_rom_اندازه = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_بایتها[0] = 0;
    p->bios_characteristics_extension_بایتها[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *انتها = 0;
    انتها++;
    إذا (!str_شاخص) {
        *انتها = 0;
        انتها++;
    }

    أرجع انتها;
}

#تعریف_کن BIOS_NAME "SeaBIOS"
#تعریف_کن BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
ساكن صحيح
smbios_build_tables(بنية romfile_s *f_tables,
                    u64 *آدرس, u32 *درازی,
                    u16 *max_structure_اندازه,
                    u16 *عدد_از_structures)
{
    بنية smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    إذا (f_tables->اندازه != *درازی)
        أرجع 0;

    qtables = malloc_tmphigh(f_tables->اندازه);
    إذا (!qtables) {
        warn_noalloc();
        أرجع 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->اندازه);
    qtables_len = f_tables->اندازه;

    /* did we get a type 0 structure ? */
    لكل (t0 = smbios_بعدی(qtables, qtables_len, عدم); t0;
         t0 = smbios_بعدی(qtables, qtables_len, t0)) {
        إذا (t0->سرآیند.type == 0) {
            need_t0 = 0;
            اكسر;
        }
    }

    إذا (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = حجم(بنية smbios_type_0) + طول_رشته(BIOS_NAME) +
                     طول_رشته(نسخه) + طول_رشته(BIOS_DATE) + 4;
        إذا (t0_len > (0xffff - *درازی)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *درازی, t0_len);
            need_t0 = 0;
        } وإلا {
            *درازی += t0_len;
            إذا (max_structure_اندازه && t0_len > *max_structure_اندازه)
                *max_structure_اندازه = t0_len;
            إذا (عدد_از_structures)
                (*عدد_از_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    إذا (*درازی > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_بالا(*درازی);
    وإلا
        tables = malloc_fseg(*درازی);
    إذا (!tables) {
        warn_noalloc();
        حرر(qtables);
        أرجع 0;
    }
    *آدرس = (u32)tables;

    /* populate final blob */
    إذا (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, نسخه, BIOS_DATE);
    کپی_حافظه(tables, qtables, qtables_len);
    حرر(qtables);
    أرجع 1;
}

ساكن صحيح
smbios_21_راهاندازی_مدخل_نقطه(بنية romfile_s *f_tables,
                            بنية smbios_21_مدخل_نقطه *ep)
{
    u64 آدرس = ep->structure_جدول_آدرس;
    u32 درازی = ep->structure_جدول_درازی;

    إذا (!smbios_build_tables(f_tables,
                             &آدرس,
                             &درازی,
                             &ep->max_structure_اندازه,
                             &ep->عدد_از_structures))
        أرجع 0;

    إذا ((u32)آدرس != آدرس || (u16)درازی != درازی) {
        warn_internalerror();
        أرجع 0;
    }

    /* finalize entry point */
    ep->structure_جدول_آدرس = آدرس;
    ep->structure_جدول_درازی = درازی;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((فراغ *)ep + 0x10, ep->درازی - 0x10);

    copy_smbios_21(ep);
    أرجع 1;
}

ساكن صحيح
smbios_30_راهاندازی_مدخل_نقطه(بنية romfile_s *f_tables,
                            بنية smbios_30_مدخل_نقطه *ep)
{
    إذا (!smbios_build_tables(f_tables,
                             &ep->structure_جدول_آدرس,
                             &ep->structure_جدول_max_اندازه,
                             عدم, عدم))
        أرجع 0;

    ep->checksum -= checksum(ep, حجم(*ep));
    copy_smbios_30(ep);
    أرجع 1;
}

ساكن صحيح
smbios_romfile_راهاندازی(فراغ)
{
    بنية romfile_s *f_anchor = romfile_پیدا_کن("etc/smbios/smbios-anchor");
    بنية romfile_s *f_tables = romfile_پیدا_کن("etc/smbios/smbios-tables");
    اتحاد {
        بنية smbios_21_مدخل_نقطه ep21;
        بنية smbios_30_مدخل_نقطه ep30;
    } ep;

    إذا (!f_anchor || !f_tables || f_anchor->اندازه > حجم(ep))
        أرجع 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->اندازه);

    إذا (f_anchor->اندازه == حجم(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        أرجع smbios_21_راهاندازی_مدخل_نقطه(f_tables, &ep.ep21);
    } وإلا إذا (f_anchor->اندازه == حجم(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        أرجع smbios_30_راهاندازی_مدخل_نقطه(f_tables, &ep.ep30);
    } وإلا {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        أرجع 0;
    }
}

فراغ
smbios_راهاندازی(فراغ)
{
    إذا (smbios_romfile_راهاندازی())
        أرجع;
    smbios_legacy_راهاندازی();
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
