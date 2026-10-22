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
smbios_21_ثبت_ټکی_تیاری(u16 max_structure_اندازه,
                         u16 structure_جدول_اوږدتیا,
                         فراغ *structure_جدول_پته,
                         u16 شمېره_د_structures)
{
    فراغ *finaltable;
    إذا (structure_جدول_اوږدتیا <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_جدول_اوږدتیا);
    وإلا
        finaltable = malloc_لوړ(structure_جدول_اوږدتیا);
    إذا (!finaltable) {
        warn_noalloc();
        أرجع;
    }
    حافظه_نقل(finaltable, structure_جدول_پته, structure_جدول_اوږدتیا);

    بنية smbios_21_ثبت_ټکی ep;
    memset(&ep, 0, حجم(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.اوږدتیا = 0x1f;
    ep.smbios_major_نسخه = 2;
    ep.smbios_minor_نسخه = 4;
    ep.max_structure_اندازه = max_structure_اندازه;
    حافظه_نقل(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_جدول_اوږدتیا = structure_جدول_اوږدتیا;
    ep.structure_جدول_پته = (u32)finaltable;
    ep.شمېره_د_structures = شمېره_د_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((فراغ*)&ep + 0x10, ep.اوږدتیا - 0x10);

    copy_smbios_21(&ep);
}

ساكن صحيح
get_ډګر(صحيح type, صحيح دننه_واټن_2, فراغ *dest)
{
    حرف name[128];
    snprintf(name, حجم(name), "smbios/field%d-%d", type, دننه_واټن_2);
    بنية romfile_s *file = romfile_پیدا_کړه(name);
    إذا (!file)
        أرجع 0;
    file->copy(file, dest, file->اندازه);
    أرجع file->اندازه;
}

ساكن صحيح
get_external(صحيح type, حرف **p, بینښه *nr_structs,
             بینښه *max_struct_اندازه, حرف *پای_ټکی)
{
    ساكن u64 کارول_شوی_bitmap[4] = { 0 };
    حرف *پیل = *p;

    /* Check if we've already reported these tables */
    إذا (کارول_شوی_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        أرجع 1;

    /* Don't introduce spurious end markers */
    إذا (type == 127)
        أرجع 0;

    حرف مختاړی[128];
    snprintf(مختاړی, حجم(مختاړی), "smbios/table%d-", type);
    بنية romfile_s *file = عدم;
    لكل (;;) {
        file = romfile_findprefix(مختاړی, file);
        إذا (!file)
            اكسر;

        إذا (پای_ټکی - *p < file->اندازه) {
            warn_noalloc();
            اكسر;
        }

        بنية smbios_structure_سرلیک *سرلیک = (فراغ*)*p;
        file->copy(file, سرلیک, file->اندازه);
        *p += file->اندازه;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        إذا (سرلیک->اوږدتیا >= file->اندازه) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        إذا (*p - (حرف*)سرلیک > *max_struct_اندازه)
            *max_struct_اندازه = *p - (حرف*)سرلیک;
    }

    إذا (پیل == *p)
        أرجع 0;

    /* Mark that we've reported on this type */
    کارول_شوی_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    أرجع 1;
}

#تعریف_کړه بارول_str_ډګر_with_default(type, ډګر, def)                   \
    افعل {                                                                \
        اندازه = get_ډګر(type, offsetof(بنية smbios_type_##type,      \
                                        ډګر), پای_ټکی);                   \
        إذا (اندازه == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->ډګر = 0;                                               \
        } وإلا إذا (اندازه > 1) {                                          \
            پای_ټکی += اندازه;                                                \
            p->ډګر = ++str_شاخص;                                     \
        } وإلا {                                                        \
            حافظه_نقل(پای_ټکی, def, حجم(def));                              \
            پای_ټکی += حجم(def);                                         \
            p->ډګر = ++str_شاخص;                                     \
        }                                                               \
    } طالما (0)

#تعریف_کړه بارول_str_ډګر_or_skip(type, ډګر)                             \
    افعل {                                                                \
        اندازه = get_ډګر(type, offsetof(بنية smbios_type_##type,      \
                                        ډګر), پای_ټکی);                   \
        إذا (اندازه > 1) {                                                 \
            پای_ټکی += اندازه;                                                \
            p->ډګر = ++str_شاخص;                                     \
        } وإلا {                                                        \
            p->ډګر = 0;                                               \
        }                                                               \
    } طالما (0)

#تعریف_کړه سېټ_ډګر_with_default(type, ډګر, def)                        \
    افعل {                                                                \
        إذا (!get_ډګر(type, offsetof(بنية smbios_type_##type,        \
                                      ډګر), &p->ډګر)) {             \
            p->ډګر = def;                                             \
        }                                                               \
    } طالما (0)

/* Type 0 -- BIOS Information */
#تعریف_کړه RELEASE_DATE_STR "01/01/2011"
ساكن فراغ *
smbios_init_type_0(فراغ *پیل)
{
    بنية smbios_type_0 *p = (بنية smbios_type_0 *)پیل;
    حرف *پای_ټکی = (حرف *)پیل + حجم(بنية smbios_type_0);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;

    p->سرلیک.type = 0;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_0);
    p->سرلیک.handle = 0;

    بارول_str_ډګر_with_default(0, vendor_str, BUILD_APPNAME);
    بارول_str_ډګر_with_default(0, bios_نسخه_str, BUILD_APPNAME);

    p->bios_starting_پته_segment = 0xe800;

    بارول_str_ډګر_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_اندازه = 0; /* FIXME */

    إذا (!get_ډګر(0, offsetof(بنية smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    إذا (!get_ډګر(0, offsetof(بنية smbios_type_0,
                               bios_characteristics_extension_بایټونه),
                   &p->bios_characteristics_extension_بایټونه)) {
        p->bios_characteristics_extension_بایټونه[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_بایټونه[1] = 4;
    }

    سېټ_ډګر_with_default(0, system_bios_major_release, 1);
    سېټ_ډګر_with_default(0, system_bios_minor_release, 0);
    سېټ_ډګر_with_default(0, embedded_controller_major_release, 0xff);
    سېټ_ډګر_with_default(0, embedded_controller_minor_release, 0xff);

    *پای_ټکی = 0;
    پای_ټکی++;
    إذا (!str_شاخص) {
        *پای_ټکی = 0;
        پای_ټکی++;
    }

    أرجع پای_ټکی;
}

/* Type 1 -- System Information */
ساكن فراغ *
smbios_init_type_1(فراغ *پیل)
{
    بنية smbios_type_1 *p = (بنية smbios_type_1 *)پیل;
    حرف *پای_ټکی = (حرف *)پیل + حجم(بنية smbios_type_1);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;

    p->سرلیک.type = 1;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_1);
    p->سرلیک.handle = 0x100;

    بارول_str_ډګر_with_default(1, manufacturer_str, BUILD_APPNAME);
    بارول_str_ډګر_with_default(1, حاصل_ضرب_name_str, BUILD_APPNAME);
    بارول_str_ډګر_or_skip(1, نسخه_str);
    بارول_str_ډګر_or_skip(1, serial_شمېره_str);

    إذا (!get_ډګر(1, offsetof(بنية smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    سېټ_ډګر_with_default(1, wake_پورته_type, 0x06); /* power switch */

    بارول_str_ډګر_or_skip(1, sku_شمېره_str);
    بارول_str_ډګر_or_skip(1, family_str);

    *پای_ټکی = 0;
    پای_ټکی++;
    إذا (!str_شاخص) {
        *پای_ټکی = 0;
        پای_ټکی++;
    }

    أرجع پای_ټکی;
}

/* Type 3 -- System Enclosure */
ساكن فراغ *
smbios_init_type_3(فراغ *پیل)
{
    بنية smbios_type_3 *p = (بنية smbios_type_3 *)پیل;
    حرف *پای_ټکی = (حرف *)پیل + حجم(بنية smbios_type_3);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;

    p->سرلیک.type = 3;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_3);
    p->سرلیک.handle = 0x300;

    بارول_str_ډګر_with_default(3, manufacturer_str, BUILD_APPNAME);
    سېټ_ډګر_with_default(3, type, 0x01); /* other */

    بارول_str_ډګر_or_skip(3, نسخه_str);
    بارول_str_ډګر_or_skip(3, serial_شمېره_str);
    بارول_str_ډګر_or_skip(3, asset_نښان_شمېره_str);

    سېټ_ډګر_with_default(3, boot_پورته_state, 0x03); /*خوندي*/
    سېټ_ډګر_with_default(3, power_supply_state, 0x03); /*خوندي*/
    سېټ_ډګر_with_default(3, thermal_state, 0x03); /*خوندي*/
    سېټ_ډګر_with_default(3, security_حالت, 0x02); /* unknown */

    سېټ_ډګر_with_default(3, oem_defined, 0);
    سېټ_ډګر_with_default(3, لوړوالی, 0);
    سېټ_ډګر_with_default(3, شمېره_د_power_cords, 0);
    سېټ_ډګر_with_default(3, contained_element_شمېر, 0);

    *پای_ټکی = 0;
    پای_ټکی++;
    إذا (!str_شاخص) {
        *پای_ټکی = 0;
        پای_ټکی++;
    }

    أرجع پای_ټکی;
}

/* Type 4 -- Processor Information */
ساكن فراغ *
smbios_init_type_4(فراغ *پیل, بینښه صحيح cpu_شمېره)
{
    بنية smbios_type_4 *p = (بنية smbios_type_4 *)پیل;
    حرف *پای_ټکی = (حرف *)پیل + حجم(بنية smbios_type_4);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;
    حرف name[1024];

    p->سرلیک.type = 4;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_4);
    p->سرلیک.handle = 0x400 + cpu_شمېره;

    اندازه = get_ډګر(4, offsetof(بنية smbios_type_4, socket_designation_str),
                     name);
    إذا (اندازه)
        snprintf(name + اندازه - 1, حجم(name) - اندازه, "%2x", cpu_شمېره);
    وإلا
        snprintf(name, حجم(name), "CPU%2x", cpu_شمېره);

    حافظه_نقل(پای_ټکی, name, مزي_اوږدوالی(name) + 1);
    پای_ټکی += مزي_اوږدوالی(name) + 1;
    p->socket_designation_str = ++str_شاخص;

    سېټ_ډګر_with_default(4, processor_type, 0x03); /* CPU */
    سېټ_ډګر_with_default(4, processor_family, 0x01); /* other */

    بارول_str_ډګر_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    إذا (!get_ډګر(4, offsetof(بنية smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    بارول_str_ډګر_or_skip(4, processor_نسخه_str);
    سېټ_ډګر_with_default(4, voltage, 0);
    سېټ_ډګر_with_default(4, external_clock, 0);

    سېټ_ډګر_with_default(4, max_چټکتیا, 2000);
    سېټ_ډګر_with_default(4, current_چټکتیا, 2000);

    سېټ_ډګر_with_default(4, حالت, 0x41); /* socket populated, CPU enabled */
    سېټ_ډګر_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *پای_ټکی = 0;
    پای_ټکی++;
    إذا (!str_شاخص) {
        *پای_ټکی = 0;
        پای_ټکی++;
    }

    أرجع پای_ټکی;
}

/* Type 16 -- Physical Memory Array */
ساكن فراغ *
smbios_init_type_16(فراغ *پیل, u32 حافظه_اندازه_mb, صحيح nr_mem_devs)
{
    بنية smbios_type_16 *p = (بنية smbios_type_16*)پیل;

    p->سرلیک.type = 16;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_16);
    p->سرلیک.handle = 0x1000;

    سېټ_ډګر_with_default(16, location, 0x01); /* other */
    سېټ_ډګر_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    سېټ_ډګر_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->اعظمي_ظرفیت = حافظه_اندازه_mb < 2 << 20 ?
                          حافظه_اندازه_mb << 10 : 0x80000000;
    p->حافظه_error_information_handle = 0xfffe; /* none provided */
    p->شمېره_د_حافظه_devices = nr_mem_devs;

    پیل += حجم(بنية smbios_type_16);
    *((u16 *)پیل) = 0;

    أرجع پیل + 2;
}

/* Type 17 -- Memory Device */
ساكن فراغ *
smbios_init_type_17(فراغ *پیل, u32 اندازه_mb, صحيح instance)
{
    بنية smbios_type_17 *p = (بنية smbios_type_17 *)پیل;
    حرف *پای_ټکی = (حرف *)پیل + حجم(بنية smbios_type_17);
    اندازه_t اندازه;
    صحيح str_شاخص = 0;
    حرف name[1024];

    p->سرلیک.type = 17;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_17);
    p->سرلیک.handle = 0x1100 + instance;

    p->physical_حافظه_array_handle = 0x1000;
    سېټ_ډګر_with_default(17, ټول_پلنوالی, 64);
    سېټ_ډګر_with_default(17, معلومات_پلنوالی, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->اندازه = اندازه_mb;
    سېټ_ډګر_with_default(17, form_ضریب, 0x09); /* DIMM */
    p->device_سېټ = 0;

    اندازه = get_ډګر(17, offsetof(بنية smbios_type_17, device_locator_str),
                     name);
    إذا (اندازه)
        snprintf(name + اندازه - 1, حجم(name) - اندازه, "%d", instance);
    وإلا
        snprintf(name, حجم(name), "DIMM %d", instance);

    حافظه_نقل(پای_ټکی, name, مزي_اوږدوالی(name) + 1);
    پای_ټکی += مزي_اوږدوالی(name) + 1;
    p->device_locator_str = ++str_شاخص;

    بارول_str_ډګر_or_skip(17, bank_locator_str);
    سېټ_ډګر_with_default(17, حافظه_type, 0x07); /* RAM */
    سېټ_ډګر_with_default(17, type_detail, 0);

    *پای_ټکی = 0;
    پای_ټکی++;
    إذا (!str_شاخص) {
        *پای_ټکی = 0;
        پای_ټکی++;
    }

    أرجع پای_ټکی;
}

/* Type 19 -- Memory Array Mapped Address */
ساكن فراغ *
smbios_init_type_19(فراغ *پیل, u32 پیل_mb, u32 اندازه_mb, صحيح instance)
{
    بنية smbios_type_19 *p = (بنية smbios_type_19 *)پیل;

    p->سرلیک.type = 19;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_19);
    p->سرلیک.handle = 0x1300 + instance;

    p->starting_پته = پیل_mb << 10;
    p->ending_پته = p->starting_پته + (اندازه_mb << 10) - 1;
    p->حافظه_array_handle = 0x1000;
    p->برخه_کول_پلنوالی = 1;

    پیل += حجم(بنية smbios_type_19);
    *((u16 *)پیل) = 0;

    أرجع پیل + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ساكن فراغ *
smbios_init_type_20(فراغ *پیل, u32 پیل_mb, u32 اندازه_mb, صحيح instance,
                    صحيح dev_handle, صحيح array_handle)
{
    بنية smbios_type_20 *p = (بنية smbios_type_20 *)پیل;

    p->سرلیک.type = 20;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_20);
    p->سرلیک.handle = 0x1400 + instance;

    p->starting_پته = پیل_mb << 10;
    p->ending_پته = p->starting_پته + (اندازه_mb << 10) - 1;
    p->حافظه_device_handle = 0x1100 + dev_handle;
    p->حافظه_array_mapped_پته_handle = 0x1300 + array_handle;
    p->برخه_کول_لیکه_position = 1;
    p->interleave_position = 0;
    p->interleaved_معلومات_ژوروالی = 0;

    پیل += حجم(بنية smbios_type_20);

    *((u16 *)پیل) = 0;
    أرجع پیل+2;
}

/* Type 32 -- System Boot Information */
ساكن فراغ *
smbios_init_type_32(فراغ *پیل)
{
    بنية smbios_type_32 *p = (بنية smbios_type_32 *)پیل;

    p->سرلیک.type = 32;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_32);
    p->سرلیک.handle = 0x2000;
    memset(p->reserved, 0, 6);
    سېټ_ډګر_with_default(32, boot_حالت, 0); /* no errors detected */

    پیل += حجم(بنية smbios_type_32);
    *((u16 *)پیل) = 0;

    أرجع پیل+2;
}

/* Type 127 -- End of Table */
ساكن فراغ *
smbios_init_type_127(فراغ *پیل)
{
    بنية smbios_type_127 *p = (بنية smbios_type_127 *)پیل;

    p->سرلیک.type = 127;
    p->سرلیک.اوږدتیا = حجم(بنية smbios_type_127);
    p->سرلیک.handle = 0x7f00;

    پیل += حجم(بنية smbios_type_127);
    *((u16 *)پیل) = 0;

    أرجع پیل + 2;
}

#تعریف_کړه TEMPSMBIOSSIZE (32 * 1024)

فراغ
smbios_legacy_تیاری(فراغ)
{
    إذا (! CONFIG_SMBIOS)
        أرجع;

    dprintf(3, "init SMBIOS tables\n");

    حرف *پیل = malloc_tmphigh(TEMPSMBIOSSIZE);
    إذا (! پیل) {
        warn_noalloc();
        أرجع;
    }
    memset(پیل, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_اندازه = 0;
    حرف *q, *p = پیل;
    حرف *پای_ټکی = پیل + TEMPSMBIOSSIZE - حجم(بنية smbios_type_127);

#تعریف_کړه جمع_کړه_struct(type, دلیلونه...)                                       \
    افعل {                                                                \
        إذا (!get_external(type, &p, &nr_structs, &max_struct_اندازه, پای_ټکی)) { \
            q = smbios_init_type_##type(دلیلونه);                          \
            nr_structs++;                                               \
            إذا ((q - p) > max_struct_اندازه)                              \
                max_struct_اندازه = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } طالما (0)

    جمع_کړه_struct(0, p);
    جمع_کړه_struct(1, p);
    جمع_کړه_struct(3, p);

    صحيح cpu_عدد;
    لكل (cpu_عدد = 1; cpu_عدد <= Max_شمېر_CPUs; cpu_عدد++)
        جمع_کړه_struct(4, p, cpu_عدد);

    صحيح ram_mb = (Ram_اندازه + Ram_اندازه_Over4_G) >> 20;
    صحيح nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    جمع_کړه_struct(16, p, ram_mb, nr_mem_devs);

    صحيح i, j;
    لكل (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        جمع_کړه_struct(17, p, dev_mb, i);
    }

    جمع_کړه_struct(19, p, 0, Ram_اندازه >> 20, 0);
    إذا (Ram_اندازه_Over4_G)
        جمع_کړه_struct(19, p, 4096, Ram_اندازه_Over4_G >> 20, 1);

    جمع_کړه_struct(20, p, 0, Ram_اندازه >> 20, 0, 0, 0);
    إذا (Ram_اندازه_Over4_G) {
        u32 پیل_mb = 4096;
        لكل (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            إذا (i == 0)
                dev_mb -= Ram_اندازه >> 20;

            جمع_کړه_struct(20, p, پیل_mb, dev_mb, j, i, 1);
            پیل_mb += dev_mb;
        }
    }

    جمع_کړه_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    لكل (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_اندازه, پای_ټکی);
    جمع_کړه_struct(127, p);

#تعریف_لغوه جمع_کړه_struct

    smbios_21_ثبت_ټکی_تیاری(max_struct_اندازه, p - پیل, پیل, nr_structs);
    حرر(پیل);
}
