// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "paravirt.h" // RamSize
#شامل "romfile.h" // romfile_findprefix
#شامل "std/smbios.h" // struct smbios_21_entry_point
#شامل "string.h" // memset
#شامل "util.h" // MaxCountCPUs
#شامل "x86.h" // cpuid

ساکن خالی
smbios_21_اندراج_نقطہ_تیاری(u16 max_structure_سائز,
                         u16 structure_جدول_لمبائی,
                         خالی *structure_جدول_پتہ,
                         u16 نمبر_کا_structures)
{
    خالی *finaltable;
    اگر (structure_جدول_لمبائی <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_جدول_لمبائی);
    ورنہ
        finaltable = malloc_بلند(structure_جدول_لمبائی);
    اگر (!finaltable) {
        warn_noalloc();
        واپس;
    }
    حافظہ_نقل(finaltable, structure_جدول_پتہ, structure_جدول_لمبائی);

    ساخت smbios_21_اندراج_نقطہ ep;
    memset(&ep, 0, حجم(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.لمبائی = 0x1f;
    ep.smbios_major_نسخہ = 2;
    ep.smbios_minor_نسخہ = 4;
    ep.max_structure_سائز = max_structure_سائز;
    حافظہ_نقل(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_جدول_لمبائی = structure_جدول_لمبائی;
    ep.structure_جدول_پتہ = (u32)finaltable;
    ep.نمبر_کا_structures = نمبر_کا_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((خالی*)&ep + 0x10, ep.لمبائی - 0x10);

    copy_smbios_21(&ep);
}

ساکن صحیح
get_میدان(صحیح type, صحیح انحراف_2, خالی *dest)
{
    حرف name[128];
    snprintf(name, حجم(name), "smbios/field%d-%d", type, انحراف_2);
    ساخت romfile_s *file = romfile_ڈھونڈو(name);
    اگر (!file)
        واپس 0;
    file->copy(file, dest, file->سائز);
    واپس file->سائز;
}

ساکن صحیح
get_external(صحیح type, حرف **p, غیرعلامتی *nr_structs,
             غیرعلامتی *max_struct_سائز, حرف *انتہا)
{
    ساکن u64 مستعمل_bitmap[4] = { 0 };
    حرف *آغاز = *p;

    /* Check if we've already reported these tables */
    اگر (مستعمل_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        واپس 1;

    /* Don't introduce spurious end markers */
    اگر (type == 127)
        واپس 0;

    حرف سابقہ[128];
    snprintf(سابقہ, حجم(سابقہ), "smbios/table%d-", type);
    ساخت romfile_s *file = تہی;
    برائے (;;) {
        file = romfile_findprefix(سابقہ, file);
        اگر (!file)
            توڑو;

        اگر (انتہا - *p < file->سائز) {
            warn_noalloc();
            توڑو;
        }

        ساخت smbios_structure_سرخی *سرخی = (خالی*)*p;
        file->copy(file, سرخی, file->سائز);
        *p += file->سائز;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        اگر (سرخی->لمبائی >= file->سائز) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        اگر (*p - (حرف*)سرخی > *max_struct_سائز)
            *max_struct_سائز = *p - (حرف*)سرخی;
    }

    اگر (آغاز == *p)
        واپس 0;

    /* Mark that we've reported on this type */
    مستعمل_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    واپس 1;
}

#تعریف_کرو بھرو_str_میدان_with_default(type, میدان, def)                   \
    کرو {                                                                \
        سائز = get_میدان(type, offsetof(ساخت smbios_type_##type,      \
                                        میدان), انتہا);                   \
        اگر (سائز == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->میدان = 0;                                               \
        } ورنہ اگر (سائز > 1) {                                          \
            انتہا += سائز;                                                \
            p->میدان = ++str_اشاریہ;                                     \
        } ورنہ {                                                        \
            حافظہ_نقل(انتہا, def, حجم(def));                              \
            انتہا += حجم(def);                                         \
            p->میدان = ++str_اشاریہ;                                     \
        }                                                               \
    } جب (0)

#تعریف_کرو بھرو_str_میدان_or_skip(type, میدان)                             \
    کرو {                                                                \
        سائز = get_میدان(type, offsetof(ساخت smbios_type_##type,      \
                                        میدان), انتہا);                   \
        اگر (سائز > 1) {                                                 \
            انتہا += سائز;                                                \
            p->میدان = ++str_اشاریہ;                                     \
        } ورنہ {                                                        \
            p->میدان = 0;                                               \
        }                                                               \
    } جب (0)

#تعریف_کرو سیٹ_میدان_with_default(type, میدان, def)                        \
    کرو {                                                                \
        اگر (!get_میدان(type, offsetof(ساخت smbios_type_##type,        \
                                      میدان), &p->میدان)) {             \
            p->میدان = def;                                             \
        }                                                               \
    } جب (0)

/* Type 0 -- BIOS Information */
#تعریف_کرو RELEASE_DATE_STR "01/01/2011"
ساکن خالی *
smbios_init_type_0(خالی *آغاز)
{
    ساخت smbios_type_0 *p = (ساخت smbios_type_0 *)آغاز;
    حرف *انتہا = (حرف *)آغاز + حجم(ساخت smbios_type_0);
    سائز_t سائز;
    صحیح str_اشاریہ = 0;

    p->سرخی.type = 0;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_0);
    p->سرخی.handle = 0;

    بھرو_str_میدان_with_default(0, vendor_str, BUILD_APPNAME);
    بھرو_str_میدان_with_default(0, bios_نسخہ_str, BUILD_APPNAME);

    p->bios_starting_پتہ_segment = 0xe800;

    بھرو_str_میدان_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_سائز = 0; /* FIXME */

    اگر (!get_میدان(0, offsetof(ساخت smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    اگر (!get_میدان(0, offsetof(ساخت smbios_type_0,
                               bios_characteristics_extension_بائٹیں),
                   &p->bios_characteristics_extension_بائٹیں)) {
        p->bios_characteristics_extension_بائٹیں[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_بائٹیں[1] = 4;
    }

    سیٹ_میدان_with_default(0, system_bios_major_release, 1);
    سیٹ_میدان_with_default(0, system_bios_minor_release, 0);
    سیٹ_میدان_with_default(0, embedded_controller_major_release, 0xff);
    سیٹ_میدان_with_default(0, embedded_controller_minor_release, 0xff);

    *انتہا = 0;
    انتہا++;
    اگر (!str_اشاریہ) {
        *انتہا = 0;
        انتہا++;
    }

    واپس انتہا;
}

/* Type 1 -- System Information */
ساکن خالی *
smbios_init_type_1(خالی *آغاز)
{
    ساخت smbios_type_1 *p = (ساخت smbios_type_1 *)آغاز;
    حرف *انتہا = (حرف *)آغاز + حجم(ساخت smbios_type_1);
    سائز_t سائز;
    صحیح str_اشاریہ = 0;

    p->سرخی.type = 1;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_1);
    p->سرخی.handle = 0x100;

    بھرو_str_میدان_with_default(1, manufacturer_str, BUILD_APPNAME);
    بھرو_str_میدان_with_default(1, حاصل_ضرب_name_str, BUILD_APPNAME);
    بھرو_str_میدان_or_skip(1, نسخہ_str);
    بھرو_str_میدان_or_skip(1, serial_نمبر_str);

    اگر (!get_میدان(1, offsetof(ساخت smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    سیٹ_میدان_with_default(1, wake_اوپر_type, 0x06); /* power switch */

    بھرو_str_میدان_or_skip(1, sku_نمبر_str);
    بھرو_str_میدان_or_skip(1, family_str);

    *انتہا = 0;
    انتہا++;
    اگر (!str_اشاریہ) {
        *انتہا = 0;
        انتہا++;
    }

    واپس انتہا;
}

/* Type 3 -- System Enclosure */
ساکن خالی *
smbios_init_type_3(خالی *آغاز)
{
    ساخت smbios_type_3 *p = (ساخت smbios_type_3 *)آغاز;
    حرف *انتہا = (حرف *)آغاز + حجم(ساخت smbios_type_3);
    سائز_t سائز;
    صحیح str_اشاریہ = 0;

    p->سرخی.type = 3;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_3);
    p->سرخی.handle = 0x300;

    بھرو_str_میدان_with_default(3, manufacturer_str, BUILD_APPNAME);
    سیٹ_میدان_with_default(3, type, 0x01); /* other */

    بھرو_str_میدان_or_skip(3, نسخہ_str);
    بھرو_str_میدان_or_skip(3, serial_نمبر_str);
    بھرو_str_میدان_or_skip(3, asset_لیبل_نمبر_str);

    سیٹ_میدان_with_default(3, boot_اوپر_state, 0x03); /*محفوظ*/
    سیٹ_میدان_with_default(3, power_supply_state, 0x03); /*محفوظ*/
    سیٹ_میدان_with_default(3, thermal_state, 0x03); /*محفوظ*/
    سیٹ_میدان_with_default(3, security_حالت, 0x02); /* unknown */

    سیٹ_میدان_with_default(3, oem_defined, 0);
    سیٹ_میدان_with_default(3, اونچائی, 0);
    سیٹ_میدان_with_default(3, نمبر_کا_power_cords, 0);
    سیٹ_میدان_with_default(3, contained_element_گنتی, 0);

    *انتہا = 0;
    انتہا++;
    اگر (!str_اشاریہ) {
        *انتہا = 0;
        انتہا++;
    }

    واپس انتہا;
}

/* Type 4 -- Processor Information */
ساکن خالی *
smbios_init_type_4(خالی *آغاز, غیرعلامتی صحیح cpu_نمبر)
{
    ساخت smbios_type_4 *p = (ساخت smbios_type_4 *)آغاز;
    حرف *انتہا = (حرف *)آغاز + حجم(ساخت smbios_type_4);
    سائز_t سائز;
    صحیح str_اشاریہ = 0;
    حرف name[1024];

    p->سرخی.type = 4;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_4);
    p->سرخی.handle = 0x400 + cpu_نمبر;

    سائز = get_میدان(4, offsetof(ساخت smbios_type_4, socket_designation_str),
                     name);
    اگر (سائز)
        snprintf(name + سائز - 1, حجم(name) - سائز, "%2x", cpu_نمبر);
    ورنہ
        snprintf(name, حجم(name), "CPU%2x", cpu_نمبر);

    حافظہ_نقل(انتہا, name, لڑی_لمبائی(name) + 1);
    انتہا += لڑی_لمبائی(name) + 1;
    p->socket_designation_str = ++str_اشاریہ;

    سیٹ_میدان_with_default(4, processor_type, 0x03); /* CPU */
    سیٹ_میدان_with_default(4, processor_family, 0x01); /* other */

    بھرو_str_میدان_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    اگر (!get_میدان(4, offsetof(ساخت smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    بھرو_str_میدان_or_skip(4, processor_نسخہ_str);
    سیٹ_میدان_with_default(4, voltage, 0);
    سیٹ_میدان_with_default(4, external_clock, 0);

    سیٹ_میدان_with_default(4, max_رفتار, 2000);
    سیٹ_میدان_with_default(4, current_رفتار, 2000);

    سیٹ_میدان_with_default(4, حالت_2, 0x41); /* socket populated, CPU enabled */
    سیٹ_میدان_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *انتہا = 0;
    انتہا++;
    اگر (!str_اشاریہ) {
        *انتہا = 0;
        انتہا++;
    }

    واپس انتہا;
}

/* Type 16 -- Physical Memory Array */
ساکن خالی *
smbios_init_type_16(خالی *آغاز, u32 حافظہ_سائز_mb, صحیح nr_mem_devs)
{
    ساخت smbios_type_16 *p = (ساخت smbios_type_16*)آغاز;

    p->سرخی.type = 16;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_16);
    p->سرخی.handle = 0x1000;

    سیٹ_میدان_with_default(16, location, 0x01); /* other */
    سیٹ_میدان_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    سیٹ_میدان_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->زیادہ_سے_زیادہ_استعداد = حافظہ_سائز_mb < 2 << 20 ?
                          حافظہ_سائز_mb << 10 : 0x80000000;
    p->حافظہ_error_information_handle = 0xfffe; /* none provided */
    p->نمبر_کا_حافظہ_devices = nr_mem_devs;

    آغاز += حجم(ساخت smbios_type_16);
    *((u16 *)آغاز) = 0;

    واپس آغاز + 2;
}

/* Type 17 -- Memory Device */
ساکن خالی *
smbios_init_type_17(خالی *آغاز, u32 سائز_mb, صحیح instance)
{
    ساخت smbios_type_17 *p = (ساخت smbios_type_17 *)آغاز;
    حرف *انتہا = (حرف *)آغاز + حجم(ساخت smbios_type_17);
    سائز_t سائز;
    صحیح str_اشاریہ = 0;
    حرف name[1024];

    p->سرخی.type = 17;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_17);
    p->سرخی.handle = 0x1100 + instance;

    p->physical_حافظہ_array_handle = 0x1000;
    سیٹ_میدان_with_default(17, کل_چوڑائی, 64);
    سیٹ_میدان_with_default(17, ڈیٹا_چوڑائی, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->سائز = سائز_mb;
    سیٹ_میدان_with_default(17, form_ضربی_عدد, 0x09); /* DIMM */
    p->device_سیٹ = 0;

    سائز = get_میدان(17, offsetof(ساخت smbios_type_17, device_locator_str),
                     name);
    اگر (سائز)
        snprintf(name + سائز - 1, حجم(name) - سائز, "%d", instance);
    ورنہ
        snprintf(name, حجم(name), "DIMM %d", instance);

    حافظہ_نقل(انتہا, name, لڑی_لمبائی(name) + 1);
    انتہا += لڑی_لمبائی(name) + 1;
    p->device_locator_str = ++str_اشاریہ;

    بھرو_str_میدان_or_skip(17, bank_locator_str);
    سیٹ_میدان_with_default(17, حافظہ_type, 0x07); /* RAM */
    سیٹ_میدان_with_default(17, type_detail, 0);

    *انتہا = 0;
    انتہا++;
    اگر (!str_اشاریہ) {
        *انتہا = 0;
        انتہا++;
    }

    واپس انتہا;
}

/* Type 19 -- Memory Array Mapped Address */
ساکن خالی *
smbios_init_type_19(خالی *آغاز, u32 آغاز_mb, u32 سائز_mb, صحیح instance)
{
    ساخت smbios_type_19 *p = (ساخت smbios_type_19 *)آغاز;

    p->سرخی.type = 19;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_19);
    p->سرخی.handle = 0x1300 + instance;

    p->starting_پتہ = آغاز_mb << 10;
    p->ending_پتہ = p->starting_پتہ + (سائز_mb << 10) - 1;
    p->حافظہ_array_handle = 0x1000;
    p->تقسیم_چوڑائی = 1;

    آغاز += حجم(ساخت smbios_type_19);
    *((u16 *)آغاز) = 0;

    واپس آغاز + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ساکن خالی *
smbios_init_type_20(خالی *آغاز, u32 آغاز_mb, u32 سائز_mb, صحیح instance,
                    صحیح dev_handle, صحیح array_handle)
{
    ساخت smbios_type_20 *p = (ساخت smbios_type_20 *)آغاز;

    p->سرخی.type = 20;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_20);
    p->سرخی.handle = 0x1400 + instance;

    p->starting_پتہ = آغاز_mb << 10;
    p->ending_پتہ = p->starting_پتہ + (سائز_mb << 10) - 1;
    p->حافظہ_device_handle = 0x1100 + dev_handle;
    p->حافظہ_array_mapped_پتہ_handle = 0x1300 + array_handle;
    p->تقسیم_صف_position = 1;
    p->interleave_position = 0;
    p->interleaved_ڈیٹا_گہرائی = 0;

    آغاز += حجم(ساخت smbios_type_20);

    *((u16 *)آغاز) = 0;
    واپس آغاز+2;
}

/* Type 32 -- System Boot Information */
ساکن خالی *
smbios_init_type_32(خالی *آغاز)
{
    ساخت smbios_type_32 *p = (ساخت smbios_type_32 *)آغاز;

    p->سرخی.type = 32;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_32);
    p->سرخی.handle = 0x2000;
    memset(p->reserved, 0, 6);
    سیٹ_میدان_with_default(32, boot_حالت, 0); /* no errors detected */

    آغاز += حجم(ساخت smbios_type_32);
    *((u16 *)آغاز) = 0;

    واپس آغاز+2;
}

/* Type 127 -- End of Table */
ساکن خالی *
smbios_init_type_127(خالی *آغاز)
{
    ساخت smbios_type_127 *p = (ساخت smbios_type_127 *)آغاز;

    p->سرخی.type = 127;
    p->سرخی.لمبائی = حجم(ساخت smbios_type_127);
    p->سرخی.handle = 0x7f00;

    آغاز += حجم(ساخت smbios_type_127);
    *((u16 *)آغاز) = 0;

    واپس آغاز + 2;
}

#تعریف_کرو TEMPSMBIOSSIZE (32 * 1024)

خالی
smbios_legacy_تیاری(خالی)
{
    اگر (! CONFIG_SMBIOS)
        واپس;

    dprintf(3, "init SMBIOS tables\n");

    حرف *آغاز = malloc_tmphigh(TEMPSMBIOSSIZE);
    اگر (! آغاز) {
        warn_noalloc();
        واپس;
    }
    memset(آغاز, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_سائز = 0;
    حرف *q, *p = آغاز;
    حرف *انتہا = آغاز + TEMPSMBIOSSIZE - حجم(ساخت smbios_type_127);

#تعریف_کرو جوڑو_struct(type, دلائل...)                                       \
    کرو {                                                                \
        اگر (!get_external(type, &p, &nr_structs, &max_struct_سائز, انتہا)) { \
            q = smbios_init_type_##type(دلائل);                          \
            nr_structs++;                                               \
            اگر ((q - p) > max_struct_سائز)                              \
                max_struct_سائز = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } جب (0)

    جوڑو_struct(0, p);
    جوڑو_struct(1, p);
    جوڑو_struct(3, p);

    صحیح cpu_عدد;
    برائے (cpu_عدد = 1; cpu_عدد <= Max_گنتی_CPUs; cpu_عدد++)
        جوڑو_struct(4, p, cpu_عدد);

    صحیح ram_mb = (Ram_سائز + Ram_سائز_Over4_G) >> 20;
    صحیح nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    جوڑو_struct(16, p, ram_mb, nr_mem_devs);

    صحیح i, j;
    برائے (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        جوڑو_struct(17, p, dev_mb, i);
    }

    جوڑو_struct(19, p, 0, Ram_سائز >> 20, 0);
    اگر (Ram_سائز_Over4_G)
        جوڑو_struct(19, p, 4096, Ram_سائز_Over4_G >> 20, 1);

    جوڑو_struct(20, p, 0, Ram_سائز >> 20, 0, 0, 0);
    اگر (Ram_سائز_Over4_G) {
        u32 آغاز_mb = 4096;
        برائے (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            اگر (i == 0)
                dev_mb -= Ram_سائز >> 20;

            جوڑو_struct(20, p, آغاز_mb, dev_mb, j, i, 1);
            آغاز_mb += dev_mb;
        }
    }

    جوڑو_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    برائے (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_سائز, انتہا);
    جوڑو_struct(127, p);

#تعریف_منسوخ جوڑو_struct

    smbios_21_اندراج_نقطہ_تیاری(max_struct_سائز, p - آغاز, آغاز, nr_structs);
    آزاد(آغاز);
}
