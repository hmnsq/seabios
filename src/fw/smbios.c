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
smbios_21_مدخل_نقطه_راهاندازی(u16 max_structure_اندازه,
                         u16 structure_جدول_درازی,
                         فراغ *structure_جدول_آدرس,
                         u16 عدد_از_structures)
{
    فراغ *finaltable;
    إذا (structure_جدول_درازی <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_جدول_درازی);
    وإلا
        finaltable = malloc_بالا(structure_جدول_درازی);
    إذا (!finaltable) {
        warn_noalloc();
        أرجع;
    }
    کپی_حافظه(finaltable, structure_جدول_آدرس, structure_جدول_درازی);

    بنية smbios_21_مدخل_نقطه ep;
    memset(&ep, 0, حجم(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.درازی = 0x1f;
    ep.smbios_major_نسخه = 2;
    ep.smbios_minor_نسخه = 4;
    ep.max_structure_اندازه = max_structure_اندازه;
    کپی_حافظه(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_جدول_درازی = structure_جدول_درازی;
    ep.structure_جدول_آدرس = (u32)finaltable;
    ep.عدد_از_structures = عدد_از_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((فراغ*)&ep + 0x10, ep.درازی - 0x10);

    copy_smbios_21(&ep);
}

ساكن صحيح
get_فیلد(صحيح type, صحيح جابه_جایی_2, فراغ *dest)
{
    حرف name[128];
    snprintf(name, حجم(name), "smbios/field%d-%d", type, جابه_جایی_2);
    بنية romfile_s *file = romfile_پیدا_کن(name);
    إذا (!file)
        أرجع 0;
    file->copy(file, dest, file->اندازه);
    أرجع file->اندازه;
}

ساكن صحيح
get_external(صحيح type, حرف **p, بیعلامت *nr_structs,
             بیعلامت *max_struct_اندازه, حرف *انتها)
{
    ساكن u64 استفاده_شده_bitmap[4] = { 0 };
    حرف *شروع = *p;

    /* Check if we've already reported these tables */
    إذا (استفاده_شده_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        أرجع 1;

    /* Don't introduce spurious end markers */
    إذا (type == 127)
        أرجع 0;

    حرف پیشوند[128];
    snprintf(پیشوند, حجم(پیشوند), "smbios/table%d-", type);
    بنية romfile_s *file = عدم;
    لكل (;;) {
        file = romfile_findprefix(پیشوند, file);
        إذا (!file)
            اكسر;

        إذا (انتها - *p < file->اندازه) {
            warn_noalloc();
            اكسر;
        }

        بنية smbios_structure_سرآیند *سرآیند = (فراغ*)*p;
        file->copy(file, سرآیند, file->اندازه);
        *p += file->اندازه;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        إذا (سرآیند->درازی >= file->اندازه) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        إذا (*p - (حرف*)سرآیند > *max_struct_اندازه)
            *max_struct_اندازه = *p - (حرف*)سرآیند;
    }

    إذا (شروع == *p)
        أرجع 0;

    /* Mark that we've reported on this type */
    استفاده_شده_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    أرجع 1;
}

#تعریف_کن بارگیری_str_فیلد_with_default(type, فیلد, def)                   \
    افعل {                                                                \
        اندازه = get_فیلد(type, offsetof(بنية smbios_type_##type,      \
                                        فیلد), انتها);                   \
        إذا (اندازه == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->فیلد = 0;                                               \
        } وإلا إذا (اندازه > 1) {                                          \
            انتها += اندازه;                                                \
            p->فیلد = ++str_شاخص;                                     \
        } وإلا {                                                        \
            کپی_حافظه(انتها, def, حجم(def));                              \
            انتها += حجم(def);                                         \
            p->فیلد = ++str_شاخص;                                     \
        }                                                               \
    } طالما (0)

#تعریف_کن بارگیری_str_فیلد_or_skip(type, فیلد)                             \
    افعل {                                                                \
        اندازه = get_فیلد(type, offsetof(بنية smbios_type_##type,      \
                                        فیلد), انتها);                   \
        إذا (اندازه > 1) {                                                 \
            انتها += اندازه;                                                \
            p->فیلد = ++str_شاخص;                                     \
        } وإلا {                                                        \
            p->فیلد = 0;                                               \
        }                                                               \
    } طالما (0)

#تعریف_کن مجموعه_فیلد_with_default(type, فیلد, def)                        \
    افعل {                                                                \
        إذا (!get_فیلد(type, offsetof(بنية smbios_type_##type,        \
                                      فیلد), &p->فیلد)) {             \
            p->فیلد = def;                                             \
        }                                                               \
    } طالما (0)

/* Type 0 -- BIOS Information */
#تعریف_کن RELEASE_DATE_STR "01/01/2011"
ساكن فراغ *
smbios_init_type_0(فراغ *شروع)
{
    بنية smbios_type_0 *p = (بنية smbios_type_0 *)شروع;
    حرف *انتها = (حرف *)شروع + حجم(بنية smbios_type_0);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;

    p->سرآیند.type = 0;
    p->سرآیند.درازی = حجم(بنية smbios_type_0);
    p->سرآیند.handle = 0;

    بارگیری_str_فیلد_with_default(0, vendor_str, BUILD_APPNAME);
    بارگیری_str_فیلد_with_default(0, bios_نسخه_str, BUILD_APPNAME);

    p->bios_starting_آدرس_segment = 0xe800;

    بارگیری_str_فیلد_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_اندازه = 0; /* FIXME */

    إذا (!get_فیلد(0, offsetof(بنية smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    إذا (!get_فیلد(0, offsetof(بنية smbios_type_0,
                               bios_characteristics_extension_بایتها),
                   &p->bios_characteristics_extension_بایتها)) {
        p->bios_characteristics_extension_بایتها[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_بایتها[1] = 4;
    }

    مجموعه_فیلد_with_default(0, system_bios_major_release, 1);
    مجموعه_فیلد_with_default(0, system_bios_minor_release, 0);
    مجموعه_فیلد_with_default(0, embedded_controller_major_release, 0xff);
    مجموعه_فیلد_with_default(0, embedded_controller_minor_release, 0xff);

    *انتها = 0;
    انتها++;
    إذا (!str_شاخص) {
        *انتها = 0;
        انتها++;
    }

    أرجع انتها;
}

/* Type 1 -- System Information */
ساكن فراغ *
smbios_init_type_1(فراغ *شروع)
{
    بنية smbios_type_1 *p = (بنية smbios_type_1 *)شروع;
    حرف *انتها = (حرف *)شروع + حجم(بنية smbios_type_1);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;

    p->سرآیند.type = 1;
    p->سرآیند.درازی = حجم(بنية smbios_type_1);
    p->سرآیند.handle = 0x100;

    بارگیری_str_فیلد_with_default(1, manufacturer_str, BUILD_APPNAME);
    بارگیری_str_فیلد_with_default(1, حاصلضرب_name_str, BUILD_APPNAME);
    بارگیری_str_فیلد_or_skip(1, نسخه_str);
    بارگیری_str_فیلد_or_skip(1, serial_عدد_str);

    إذا (!get_فیلد(1, offsetof(بنية smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    مجموعه_فیلد_with_default(1, wake_به_بالا_type, 0x06); /* power switch */

    بارگیری_str_فیلد_or_skip(1, sku_عدد_str);
    بارگیری_str_فیلد_or_skip(1, family_str);

    *انتها = 0;
    انتها++;
    إذا (!str_شاخص) {
        *انتها = 0;
        انتها++;
    }

    أرجع انتها;
}

/* Type 3 -- System Enclosure */
ساكن فراغ *
smbios_init_type_3(فراغ *شروع)
{
    بنية smbios_type_3 *p = (بنية smbios_type_3 *)شروع;
    حرف *انتها = (حرف *)شروع + حجم(بنية smbios_type_3);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;

    p->سرآیند.type = 3;
    p->سرآیند.درازی = حجم(بنية smbios_type_3);
    p->سرآیند.handle = 0x300;

    بارگیری_str_فیلد_with_default(3, manufacturer_str, BUILD_APPNAME);
    مجموعه_فیلد_with_default(3, type, 0x01); /* other */

    بارگیری_str_فیلد_or_skip(3, نسخه_str);
    بارگیری_str_فیلد_or_skip(3, serial_عدد_str);
    بارگیری_str_فیلد_or_skip(3, asset_برچسب_عدد_str);

    مجموعه_فیلد_with_default(3, boot_به_بالا_state, 0x03); /*امن*/
    مجموعه_فیلد_with_default(3, power_supply_state, 0x03); /*امن*/
    مجموعه_فیلد_with_default(3, thermal_state, 0x03); /*امن*/
    مجموعه_فیلد_with_default(3, security_وضعیت, 0x02); /* unknown */

    مجموعه_فیلد_with_default(3, oem_defined, 0);
    مجموعه_فیلد_with_default(3, ارتفاع, 0);
    مجموعه_فیلد_with_default(3, عدد_از_power_cords, 0);
    مجموعه_فیلد_with_default(3, contained_element_شمار, 0);

    *انتها = 0;
    انتها++;
    إذا (!str_شاخص) {
        *انتها = 0;
        انتها++;
    }

    أرجع انتها;
}

/* Type 4 -- Processor Information */
ساكن فراغ *
smbios_init_type_4(فراغ *شروع, بیعلامت صحيح cpu_عدد)
{
    بنية smbios_type_4 *p = (بنية smbios_type_4 *)شروع;
    حرف *انتها = (حرف *)شروع + حجم(بنية smbios_type_4);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;
    حرف name[1024];

    p->سرآیند.type = 4;
    p->سرآیند.درازی = حجم(بنية smbios_type_4);
    p->سرآیند.handle = 0x400 + cpu_عدد;

    اندازه = get_فیلد(4, offsetof(بنية smbios_type_4, socket_designation_str),
                     name);
    إذا (اندازه)
        snprintf(name + اندازه - 1, حجم(name) - اندازه, "%2x", cpu_عدد);
    وإلا
        snprintf(name, حجم(name), "CPU%2x", cpu_عدد);

    کپی_حافظه(انتها, name, طول_رشته(name) + 1);
    انتها += طول_رشته(name) + 1;
    p->socket_designation_str = ++str_شاخص;

    مجموعه_فیلد_with_default(4, processor_type, 0x03); /* CPU */
    مجموعه_فیلد_with_default(4, processor_family, 0x01); /* other */

    بارگیری_str_فیلد_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    إذا (!get_فیلد(4, offsetof(بنية smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    بارگیری_str_فیلد_or_skip(4, processor_نسخه_str);
    مجموعه_فیلد_with_default(4, voltage, 0);
    مجموعه_فیلد_with_default(4, external_clock, 0);

    مجموعه_فیلد_with_default(4, max_سرعت, 2000);
    مجموعه_فیلد_with_default(4, current_سرعت, 2000);

    مجموعه_فیلد_with_default(4, وضعیت, 0x41); /* socket populated, CPU enabled */
    مجموعه_فیلد_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *انتها = 0;
    انتها++;
    إذا (!str_شاخص) {
        *انتها = 0;
        انتها++;
    }

    أرجع انتها;
}

/* Type 16 -- Physical Memory Array */
ساكن فراغ *
smbios_init_type_16(فراغ *شروع, u32 حافظه_اندازه_mb, صحيح nr_mem_devs)
{
    بنية smbios_type_16 *p = (بنية smbios_type_16*)شروع;

    p->سرآیند.type = 16;
    p->سرآیند.درازی = حجم(بنية smbios_type_16);
    p->سرآیند.handle = 0x1000;

    مجموعه_فیلد_with_default(16, location, 0x01); /* other */
    مجموعه_فیلد_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    مجموعه_فیلد_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->بیشینه_ظرفیت = حافظه_اندازه_mb < 2 << 20 ?
                          حافظه_اندازه_mb << 10 : 0x80000000;
    p->حافظه_error_information_handle = 0xfffe; /* none provided */
    p->عدد_از_حافظه_devices = nr_mem_devs;

    شروع += حجم(بنية smbios_type_16);
    *((u16 *)شروع) = 0;

    أرجع شروع + 2;
}

/* Type 17 -- Memory Device */
ساكن فراغ *
smbios_init_type_17(فراغ *شروع, u32 اندازه_mb, صحيح instance)
{
    بنية smbios_type_17 *p = (بنية smbios_type_17 *)شروع;
    حرف *انتها = (حرف *)شروع + حجم(بنية smbios_type_17);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;
    حرف name[1024];

    p->سرآیند.type = 17;
    p->سرآیند.درازی = حجم(بنية smbios_type_17);
    p->سرآیند.handle = 0x1100 + instance;

    p->physical_حافظه_array_handle = 0x1000;
    مجموعه_فیلد_with_default(17, مجموع_عرض, 64);
    مجموعه_فیلد_with_default(17, داده_عرض, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->اندازه = اندازه_mb;
    مجموعه_فیلد_with_default(17, form_ضریب, 0x09); /* DIMM */
    p->device_مجموعه = 0;

    اندازه = get_فیلد(17, offsetof(بنية smbios_type_17, device_locator_str),
                     name);
    إذا (اندازه)
        snprintf(name + اندازه - 1, حجم(name) - اندازه, "%d", instance);
    وإلا
        snprintf(name, حجم(name), "DIMM %d", instance);

    کپی_حافظه(انتها, name, طول_رشته(name) + 1);
    انتها += طول_رشته(name) + 1;
    p->device_locator_str = ++str_شاخص;

    بارگیری_str_فیلد_or_skip(17, bank_locator_str);
    مجموعه_فیلد_with_default(17, حافظه_type, 0x07); /* RAM */
    مجموعه_فیلد_with_default(17, type_detail, 0);

    *انتها = 0;
    انتها++;
    إذا (!str_شاخص) {
        *انتها = 0;
        انتها++;
    }

    أرجع انتها;
}

/* Type 19 -- Memory Array Mapped Address */
ساكن فراغ *
smbios_init_type_19(فراغ *شروع, u32 شروع_mb, u32 اندازه_mb, صحيح instance)
{
    بنية smbios_type_19 *p = (بنية smbios_type_19 *)شروع;

    p->سرآیند.type = 19;
    p->سرآیند.درازی = حجم(بنية smbios_type_19);
    p->سرآیند.handle = 0x1300 + instance;

    p->starting_آدرس = شروع_mb << 10;
    p->ending_آدرس = p->starting_آدرس + (اندازه_mb << 10) - 1;
    p->حافظه_array_handle = 0x1000;
    p->افراز_عرض = 1;

    شروع += حجم(بنية smbios_type_19);
    *((u16 *)شروع) = 0;

    أرجع شروع + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ساكن فراغ *
smbios_init_type_20(فراغ *شروع, u32 شروع_mb, u32 اندازه_mb, صحيح instance,
                    صحيح dev_handle, صحيح array_handle)
{
    بنية smbios_type_20 *p = (بنية smbios_type_20 *)شروع;

    p->سرآیند.type = 20;
    p->سرآیند.درازی = حجم(بنية smbios_type_20);
    p->سرآیند.handle = 0x1400 + instance;

    p->starting_آدرس = شروع_mb << 10;
    p->ending_آدرس = p->starting_آدرس + (اندازه_mb << 10) - 1;
    p->حافظه_device_handle = 0x1100 + dev_handle;
    p->حافظه_array_mapped_آدرس_handle = 0x1300 + array_handle;
    p->افراز_ردیف_position = 1;
    p->interleave_position = 0;
    p->interleaved_داده_عمق = 0;

    شروع += حجم(بنية smbios_type_20);

    *((u16 *)شروع) = 0;
    أرجع شروع+2;
}

/* Type 32 -- System Boot Information */
ساكن فراغ *
smbios_init_type_32(فراغ *شروع)
{
    بنية smbios_type_32 *p = (بنية smbios_type_32 *)شروع;

    p->سرآیند.type = 32;
    p->سرآیند.درازی = حجم(بنية smbios_type_32);
    p->سرآیند.handle = 0x2000;
    memset(p->reserved, 0, 6);
    مجموعه_فیلد_with_default(32, boot_وضعیت, 0); /* no errors detected */

    شروع += حجم(بنية smbios_type_32);
    *((u16 *)شروع) = 0;

    أرجع شروع+2;
}

/* Type 127 -- End of Table */
ساكن فراغ *
smbios_init_type_127(فراغ *شروع)
{
    بنية smbios_type_127 *p = (بنية smbios_type_127 *)شروع;

    p->سرآیند.type = 127;
    p->سرآیند.درازی = حجم(بنية smbios_type_127);
    p->سرآیند.handle = 0x7f00;

    شروع += حجم(بنية smbios_type_127);
    *((u16 *)شروع) = 0;

    أرجع شروع + 2;
}

#تعریف_کن TEMPSMBIOSSIZE (32 * 1024)

فراغ
smbios_legacy_راهاندازی(فراغ)
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

    u32 nr_structs = 0, max_struct_اندازه = 0;
    حرف *q, *p = شروع;
    حرف *انتها = شروع + TEMPSMBIOSSIZE - حجم(بنية smbios_type_127);

#تعریف_کن جمع_کن_struct(type, آرگومانها...)                                       \
    افعل {                                                                \
        إذا (!get_external(type, &p, &nr_structs, &max_struct_اندازه, انتها)) { \
            q = smbios_init_type_##type(آرگومانها);                          \
            nr_structs++;                                               \
            إذا ((q - p) > max_struct_اندازه)                              \
                max_struct_اندازه = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } طالما (0)

    جمع_کن_struct(0, p);
    جمع_کن_struct(1, p);
    جمع_کن_struct(3, p);

    صحيح cpu_رقم;
    لكل (cpu_رقم = 1; cpu_رقم <= Max_شمار_CPUs; cpu_رقم++)
        جمع_کن_struct(4, p, cpu_رقم);

    صحيح ram_mb = (Ram_اندازه + Ram_اندازه_Over4_G) >> 20;
    صحيح nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    جمع_کن_struct(16, p, ram_mb, nr_mem_devs);

    صحيح i, j;
    لكل (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        جمع_کن_struct(17, p, dev_mb, i);
    }

    جمع_کن_struct(19, p, 0, Ram_اندازه >> 20, 0);
    إذا (Ram_اندازه_Over4_G)
        جمع_کن_struct(19, p, 4096, Ram_اندازه_Over4_G >> 20, 1);

    جمع_کن_struct(20, p, 0, Ram_اندازه >> 20, 0, 0, 0);
    إذا (Ram_اندازه_Over4_G) {
        u32 شروع_mb = 4096;
        لكل (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            إذا (i == 0)
                dev_mb -= Ram_اندازه >> 20;

            جمع_کن_struct(20, p, شروع_mb, dev_mb, j, i, 1);
            شروع_mb += dev_mb;
        }
    }

    جمع_کن_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    لكل (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_اندازه, انتها);
    جمع_کن_struct(127, p);

#لغو_تعریف جمع_کن_struct

    smbios_21_مدخل_نقطه_راهاندازی(max_struct_اندازه, p - شروع, شروع, nr_structs);
    حرر(شروع);
}
