// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // RamSize
#ضمن "romfile.h" // romfile_findprefix
#ضمن "std/smbios.h" // struct smbios_21_entry_point
#ضمن "string.h" // memset
#ضمن "util.h" // MaxCountCPUs
#ضمن "x86.h" // cpuid

ساكن فراغ
smbios_21_اندراج_نقطہ_تیاری(u16 max_structure_ناپ,
                         u16 structure_جدول_لمائی,
                         فراغ *structure_جدول_پتہ,
                         u16 نمبر_دا_structures)
{
    فراغ *finaltable;
    إذا (structure_جدول_لمائی <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_جدول_لمائی);
    وإلا
        finaltable = malloc_بلند(structure_جدول_لمائی);
    إذا (!finaltable) {
        warn_noalloc();
        أرجع;
    }
    حافظہ_نقل(finaltable, structure_جدول_پتہ, structure_جدول_لمائی);

    بنية smbios_21_اندراج_نقطہ ep;
    memset(&ep, 0, حجم(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.لمائی = 0x1f;
    ep.smbios_major_نسخہ = 2;
    ep.smbios_minor_نسخہ = 4;
    ep.max_structure_ناپ = max_structure_ناپ;
    حافظہ_نقل(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_جدول_لمائی = structure_جدول_لمائی;
    ep.structure_جدول_پتہ = (u32)finaltable;
    ep.نمبر_دا_structures = نمبر_دا_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((فراغ*)&ep + 0x10, ep.لمائی - 0x10);

    copy_smbios_21(&ep);
}

ساكن صحيح
get_میدان(صحيح type, صحيح وتھ_2, فراغ *dest)
{
    حرف name[128];
    snprintf(name, حجم(name), "smbios/field%d-%d", type, وتھ_2);
    بنية romfile_s *file = romfile_لبھو(name);
    إذا (!file)
        أرجع 0;
    file->copy(file, dest, file->ناپ);
    أرجع file->ناپ;
}

ساكن صحيح
get_external(صحيح type, حرف **p, بنانشانی *nr_structs,
             بنانشانی *max_struct_ناپ, حرف *سرا)
{
    ساكن u64 ورتیا_bitmap[4] = { 0 };
    حرف *شروع = *p;

    /* Check if we've already reported these tables */
    إذا (ورتیا_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        أرجع 1;

    /* Don't introduce spurious end markers */
    إذا (type == 127)
        أرجع 0;

    حرف اگیتر[128];
    snprintf(اگیتر, حجم(اگیتر), "smbios/table%d-", type);
    بنية romfile_s *file = عدم;
    لكل (;;) {
        file = romfile_findprefix(اگیتر, file);
        إذا (!file)
            اكسر;

        إذا (سرا - *p < file->ناپ) {
            warn_noalloc();
            اكسر;
        }

        بنية smbios_structure_سرخی *سرخی = (فراغ*)*p;
        file->copy(file, سرخی, file->ناپ);
        *p += file->ناپ;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        إذا (سرخی->لمائی >= file->ناپ) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        إذا (*p - (حرف*)سرخی > *max_struct_ناپ)
            *max_struct_ناپ = *p - (حرف*)سرخی;
    }

    إذا (شروع == *p)
        أرجع 0;

    /* Mark that we've reported on this type */
    ورتیا_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    أرجع 1;
}

#تعریف_کرو لدو_str_میدان_with_default(type, میدان, def)                   \
    افعل {                                                                \
        ناپ = get_میدان(type, offsetof(بنية smbios_type_##type,      \
                                        میدان), سرا);                   \
        إذا (ناپ == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->میدان = 0;                                               \
        } وإلا إذا (ناپ > 1) {                                          \
            سرا += ناپ;                                                \
            p->میدان = ++str_اشاریہ;                                     \
        } وإلا {                                                        \
            حافظہ_نقل(سرا, def, حجم(def));                              \
            سرا += حجم(def);                                         \
            p->میدان = ++str_اشاریہ;                                     \
        }                                                               \
    } طالما (0)

#تعریف_کرو لدو_str_میدان_or_skip(type, میدان)                             \
    افعل {                                                                \
        ناپ = get_میدان(type, offsetof(بنية smbios_type_##type,      \
                                        میدان), سرا);                   \
        إذا (ناپ > 1) {                                                 \
            سرا += ناپ;                                                \
            p->میدان = ++str_اشاریہ;                                     \
        } وإلا {                                                        \
            p->میدان = 0;                                               \
        }                                                               \
    } طالما (0)

#تعریف_کرو سیٹ_میدان_with_default(type, میدان, def)                        \
    افعل {                                                                \
        إذا (!get_میدان(type, offsetof(بنية smbios_type_##type,        \
                                      میدان), &p->میدان)) {             \
            p->میدان = def;                                             \
        }                                                               \
    } طالما (0)

/* Type 0 -- BIOS Information */
#تعریف_کرو RELEASE_DATE_STR "01/01/2011"
ساكن فراغ *
smbios_init_type_0(فراغ *شروع)
{
    بنية smbios_type_0 *p = (بنية smbios_type_0 *)شروع;
    حرف *سرا = (حرف *)شروع + حجم(بنية smbios_type_0);
    ناپ_t ناپ;
    صحيح str_اشاریہ = 0;

    p->سرخی.type = 0;
    p->سرخی.لمائی = حجم(بنية smbios_type_0);
    p->سرخی.handle = 0;

    لدو_str_میدان_with_default(0, vendor_str, BUILD_APPNAME);
    لدو_str_میدان_with_default(0, bios_نسخہ_str, BUILD_APPNAME);

    p->bios_starting_پتہ_segment = 0xe800;

    لدو_str_میدان_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_ناپ = 0; /* FIXME */

    إذا (!get_میدان(0, offsetof(بنية smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    إذا (!get_میدان(0, offsetof(بنية smbios_type_0,
                               bios_characteristics_extension_بائٹاں),
                   &p->bios_characteristics_extension_بائٹاں)) {
        p->bios_characteristics_extension_بائٹاں[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_بائٹاں[1] = 4;
    }

    سیٹ_میدان_with_default(0, system_bios_major_release, 1);
    سیٹ_میدان_with_default(0, system_bios_minor_release, 0);
    سیٹ_میدان_with_default(0, embedded_controller_major_release, 0xff);
    سیٹ_میدان_with_default(0, embedded_controller_minor_release, 0xff);

    *سرا = 0;
    سرا++;
    إذا (!str_اشاریہ) {
        *سرا = 0;
        سرا++;
    }

    أرجع سرا;
}

/* Type 1 -- System Information */
ساكن فراغ *
smbios_init_type_1(فراغ *شروع)
{
    بنية smbios_type_1 *p = (بنية smbios_type_1 *)شروع;
    حرف *سرا = (حرف *)شروع + حجم(بنية smbios_type_1);
    ناپ_t ناپ;
    صحيح str_اشاریہ = 0;

    p->سرخی.type = 1;
    p->سرخی.لمائی = حجم(بنية smbios_type_1);
    p->سرخی.handle = 0x100;

    لدو_str_میدان_with_default(1, manufacturer_str, BUILD_APPNAME);
    لدو_str_میدان_with_default(1, ضرب_حاصل_name_str, BUILD_APPNAME);
    لدو_str_میدان_or_skip(1, نسخہ_str);
    لدو_str_میدان_or_skip(1, serial_نمبر_str);

    إذا (!get_میدان(1, offsetof(بنية smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    سیٹ_میدان_with_default(1, wake_اتے_type, 0x06); /* power switch */

    لدو_str_میدان_or_skip(1, sku_نمبر_str);
    لدو_str_میدان_or_skip(1, family_str);

    *سرا = 0;
    سرا++;
    إذا (!str_اشاریہ) {
        *سرا = 0;
        سرا++;
    }

    أرجع سرا;
}

/* Type 3 -- System Enclosure */
ساكن فراغ *
smbios_init_type_3(فراغ *شروع)
{
    بنية smbios_type_3 *p = (بنية smbios_type_3 *)شروع;
    حرف *سرا = (حرف *)شروع + حجم(بنية smbios_type_3);
    ناپ_t ناپ;
    صحيح str_اشاریہ = 0;

    p->سرخی.type = 3;
    p->سرخی.لمائی = حجم(بنية smbios_type_3);
    p->سرخی.handle = 0x300;

    لدو_str_میدان_with_default(3, manufacturer_str, BUILD_APPNAME);
    سیٹ_میدان_with_default(3, type, 0x01); /* other */

    لدو_str_میدان_or_skip(3, نسخہ_str);
    لدو_str_میدان_or_skip(3, serial_نمبر_str);
    لدو_str_میدان_or_skip(3, asset_ٹیگ_نمبر_str);

    سیٹ_میدان_with_default(3, boot_اتے_state, 0x03); /*محفوظ*/
    سیٹ_میدان_with_default(3, power_supply_state, 0x03); /*محفوظ*/
    سیٹ_میدان_with_default(3, thermal_state, 0x03); /*محفوظ*/
    سیٹ_میدان_with_default(3, security_حالت, 0x02); /* unknown */

    سیٹ_میدان_with_default(3, oem_defined, 0);
    سیٹ_میدان_with_default(3, اچائی, 0);
    سیٹ_میدان_with_default(3, نمبر_دا_power_cords, 0);
    سیٹ_میدان_with_default(3, contained_element_گنتی, 0);

    *سرا = 0;
    سرا++;
    إذا (!str_اشاریہ) {
        *سرا = 0;
        سرا++;
    }

    أرجع سرا;
}

/* Type 4 -- Processor Information */
ساكن فراغ *
smbios_init_type_4(فراغ *شروع, بنانشانی صحيح cpu_نمبر)
{
    بنية smbios_type_4 *p = (بنية smbios_type_4 *)شروع;
    حرف *سرا = (حرف *)شروع + حجم(بنية smbios_type_4);
    ناپ_t ناپ;
    صحيح str_اشاریہ = 0;
    حرف name[1024];

    p->سرخی.type = 4;
    p->سرخی.لمائی = حجم(بنية smbios_type_4);
    p->سرخی.handle = 0x400 + cpu_نمبر;

    ناپ = get_میدان(4, offsetof(بنية smbios_type_4, socket_designation_str),
                     name);
    إذا (ناپ)
        snprintf(name + ناپ - 1, حجم(name) - ناپ, "%2x", cpu_نمبر);
    وإلا
        snprintf(name, حجم(name), "CPU%2x", cpu_نمبر);

    حافظہ_نقل(سرا, name, لڑی_لمبائی(name) + 1);
    سرا += لڑی_لمبائی(name) + 1;
    p->socket_designation_str = ++str_اشاریہ;

    سیٹ_میدان_with_default(4, processor_type, 0x03); /* CPU */
    سیٹ_میدان_with_default(4, processor_family, 0x01); /* other */

    لدو_str_میدان_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    إذا (!get_میدان(4, offsetof(بنية smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    لدو_str_میدان_or_skip(4, processor_نسخہ_str);
    سیٹ_میدان_with_default(4, voltage, 0);
    سیٹ_میدان_with_default(4, external_clock, 0);

    سیٹ_میدان_with_default(4, max_رفتار, 2000);
    سیٹ_میدان_with_default(4, current_رفتار, 2000);

    سیٹ_میدان_with_default(4, حالت, 0x41); /* socket populated, CPU enabled */
    سیٹ_میدان_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *سرا = 0;
    سرا++;
    إذا (!str_اشاریہ) {
        *سرا = 0;
        سرا++;
    }

    أرجع سرا;
}

/* Type 16 -- Physical Memory Array */
ساكن فراغ *
smbios_init_type_16(فراغ *شروع, u32 حافظہ_ناپ_mb, صحيح nr_mem_devs)
{
    بنية smbios_type_16 *p = (بنية smbios_type_16*)شروع;

    p->سرخی.type = 16;
    p->سرخی.لمائی = حجم(بنية smbios_type_16);
    p->سرخی.handle = 0x1000;

    سیٹ_میدان_with_default(16, location, 0x01); /* other */
    سیٹ_میدان_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    سیٹ_میدان_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->ودھ_توں_ودھ_سمرتھا = حافظہ_ناپ_mb < 2 << 20 ?
                          حافظہ_ناپ_mb << 10 : 0x80000000;
    p->حافظہ_error_information_handle = 0xfffe; /* none provided */
    p->نمبر_دا_حافظہ_devices = nr_mem_devs;

    شروع += حجم(بنية smbios_type_16);
    *((u16 *)شروع) = 0;

    أرجع شروع + 2;
}

/* Type 17 -- Memory Device */
ساكن فراغ *
smbios_init_type_17(فراغ *شروع, u32 ناپ_mb, صحيح instance)
{
    بنية smbios_type_17 *p = (بنية smbios_type_17 *)شروع;
    حرف *سرا = (حرف *)شروع + حجم(بنية smbios_type_17);
    ناپ_t ناپ;
    صحيح str_اشاریہ = 0;
    حرف name[1024];

    p->سرخی.type = 17;
    p->سرخی.لمائی = حجم(بنية smbios_type_17);
    p->سرخی.handle = 0x1100 + instance;

    p->physical_حافظہ_array_handle = 0x1000;
    سیٹ_میدان_with_default(17, کل_چوڑائی, 64);
    سیٹ_میدان_with_default(17, انکڑے_چوڑائی, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->ناپ = ناپ_mb;
    سیٹ_میدان_with_default(17, form_ضارب, 0x09); /* DIMM */
    p->device_سیٹ = 0;

    ناپ = get_میدان(17, offsetof(بنية smbios_type_17, device_locator_str),
                     name);
    إذا (ناپ)
        snprintf(name + ناپ - 1, حجم(name) - ناپ, "%d", instance);
    وإلا
        snprintf(name, حجم(name), "DIMM %d", instance);

    حافظہ_نقل(سرا, name, لڑی_لمبائی(name) + 1);
    سرا += لڑی_لمبائی(name) + 1;
    p->device_locator_str = ++str_اشاریہ;

    لدو_str_میدان_or_skip(17, bank_locator_str);
    سیٹ_میدان_with_default(17, حافظہ_type, 0x07); /* RAM */
    سیٹ_میدان_with_default(17, type_detail, 0);

    *سرا = 0;
    سرا++;
    إذا (!str_اشاریہ) {
        *سرا = 0;
        سرا++;
    }

    أرجع سرا;
}

/* Type 19 -- Memory Array Mapped Address */
ساكن فراغ *
smbios_init_type_19(فراغ *شروع, u32 شروع_mb, u32 ناپ_mb, صحيح instance)
{
    بنية smbios_type_19 *p = (بنية smbios_type_19 *)شروع;

    p->سرخی.type = 19;
    p->سرخی.لمائی = حجم(بنية smbios_type_19);
    p->سرخی.handle = 0x1300 + instance;

    p->starting_پتہ = شروع_mb << 10;
    p->ending_پتہ = p->starting_پتہ + (ناپ_mb << 10) - 1;
    p->حافظہ_array_handle = 0x1000;
    p->حصے_کرو_چوڑائی = 1;

    شروع += حجم(بنية smbios_type_19);
    *((u16 *)شروع) = 0;

    أرجع شروع + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ساكن فراغ *
smbios_init_type_20(فراغ *شروع, u32 شروع_mb, u32 ناپ_mb, صحيح instance,
                    صحيح dev_handle, صحيح array_handle)
{
    بنية smbios_type_20 *p = (بنية smbios_type_20 *)شروع;

    p->سرخی.type = 20;
    p->سرخی.لمائی = حجم(بنية smbios_type_20);
    p->سرخی.handle = 0x1400 + instance;

    p->starting_پتہ = شروع_mb << 10;
    p->ending_پتہ = p->starting_پتہ + (ناپ_mb << 10) - 1;
    p->حافظہ_device_handle = 0x1100 + dev_handle;
    p->حافظہ_array_mapped_پتہ_handle = 0x1300 + array_handle;
    p->حصے_کرو_صف_position = 1;
    p->interleave_position = 0;
    p->interleaved_انکڑے_ڈونگھائی = 0;

    شروع += حجم(بنية smbios_type_20);

    *((u16 *)شروع) = 0;
    أرجع شروع+2;
}

/* Type 32 -- System Boot Information */
ساكن فراغ *
smbios_init_type_32(فراغ *شروع)
{
    بنية smbios_type_32 *p = (بنية smbios_type_32 *)شروع;

    p->سرخی.type = 32;
    p->سرخی.لمائی = حجم(بنية smbios_type_32);
    p->سرخی.handle = 0x2000;
    memset(p->reserved, 0, 6);
    سیٹ_میدان_with_default(32, boot_حالت, 0); /* no errors detected */

    شروع += حجم(بنية smbios_type_32);
    *((u16 *)شروع) = 0;

    أرجع شروع+2;
}

/* Type 127 -- End of Table */
ساكن فراغ *
smbios_init_type_127(فراغ *شروع)
{
    بنية smbios_type_127 *p = (بنية smbios_type_127 *)شروع;

    p->سرخی.type = 127;
    p->سرخی.لمائی = حجم(بنية smbios_type_127);
    p->سرخی.handle = 0x7f00;

    شروع += حجم(بنية smbios_type_127);
    *((u16 *)شروع) = 0;

    أرجع شروع + 2;
}

#تعریف_کرو TEMPSMBIOSSIZE (32 * 1024)

فراغ
smbios_legacy_تیاری(فراغ)
{
    إذا (! CONFIG_SMBIOS)
        أرجع;

    dprintf(3, "init SMBIOS tables\n");

    حرف *شروع = malloc_tmphigh(TEMPSMBIOSSIZE);
    إذا (! شروع) {
        warn_noalloc();
        أرجع;
    }
    memset(شروع, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_ناپ = 0;
    حرف *q, *p = شروع;
    حرف *سرا = شروع + TEMPSMBIOSSIZE - حجم(بنية smbios_type_127);

#تعریف_کرو جوڑو_struct(type, دلیلاں...)                                       \
    افعل {                                                                \
        إذا (!get_external(type, &p, &nr_structs, &max_struct_ناپ, سرا)) { \
            q = smbios_init_type_##type(دلیلاں);                          \
            nr_structs++;                                               \
            إذا ((q - p) > max_struct_ناپ)                              \
                max_struct_ناپ = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } طالما (0)

    جوڑو_struct(0, p);
    جوڑو_struct(1, p);
    جوڑو_struct(3, p);

    صحيح cpu_عدد;
    لكل (cpu_عدد = 1; cpu_عدد <= Max_گنتی_CPUs; cpu_عدد++)
        جوڑو_struct(4, p, cpu_عدد);

    صحيح ram_mb = (Ram_ناپ + Ram_ناپ_Over4_G) >> 20;
    صحيح nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    جوڑو_struct(16, p, ram_mb, nr_mem_devs);

    صحيح i, j;
    لكل (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        جوڑو_struct(17, p, dev_mb, i);
    }

    جوڑو_struct(19, p, 0, Ram_ناپ >> 20, 0);
    إذا (Ram_ناپ_Over4_G)
        جوڑو_struct(19, p, 4096, Ram_ناپ_Over4_G >> 20, 1);

    جوڑو_struct(20, p, 0, Ram_ناپ >> 20, 0, 0, 0);
    إذا (Ram_ناپ_Over4_G) {
        u32 شروع_mb = 4096;
        لكل (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            إذا (i == 0)
                dev_mb -= Ram_ناپ >> 20;

            جوڑو_struct(20, p, شروع_mb, dev_mb, j, i, 1);
            شروع_mb += dev_mb;
        }
    }

    جوڑو_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    لكل (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_ناپ, سرا);
    جوڑو_struct(127, p);

#تعریف_رد جوڑو_struct

    smbios_21_اندراج_نقطہ_تیاری(max_struct_ناپ, p - شروع, شروع, nr_structs);
    حرر(شروع);
}
