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
smbios_21_داخلا_نقطو_تياري(u16 max_structure_سائيز,
                         u16 structure_جدول_ڊيگهائي,
                         فراغ *structure_جدول_پتو,
                         u16 عدد_جو_structures)
{
    فراغ *finaltable;
    إذا (structure_جدول_ڊيگهائي <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_جدول_ڊيگهائي);
    وإلا
        finaltable = malloc_مٿاهون(structure_جدول_ڊيگهائي);
    إذا (!finaltable) {
        warn_noalloc();
        أرجع;
    }
    يادگيري_نقل(finaltable, structure_جدول_پتو, structure_جدول_ڊيگهائي);

    بنية smbios_21_داخلا_نقطو ep;
    memset(&ep, 0, حجم(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.ڊيگهائي = 0x1f;
    ep.smbios_major_نسخو = 2;
    ep.smbios_minor_نسخو = 4;
    ep.max_structure_سائيز = max_structure_سائيز;
    يادگيري_نقل(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_جدول_ڊيگهائي = structure_جدول_ڊيگهائي;
    ep.structure_جدول_پتو = (u32)finaltable;
    ep.عدد_جو_structures = عدد_جو_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((فراغ*)&ep + 0x10, ep.ڊيگهائي - 0x10);

    copy_smbios_21(&ep);
}

ساكن صحيح
get_ميدان(صحيح type, صحيح وٿي_2, فراغ *dest)
{
    حرف name[128];
    snprintf(name, حجم(name), "smbios/field%d-%d", type, وٿي_2);
    بنية romfile_s *file = romfile_ڳوليو(name);
    إذا (!file)
        أرجع 0;
    file->copy(file, dest, file->سائيز);
    أرجع file->سائيز;
}

ساكن صحيح
get_external(صحيح type, حرف **p, بنانشان *nr_structs,
             بنانشان *max_struct_سائيز, حرف *پڇاڙي)
{
    ساكن u64 استعمال_ٿيل_bitmap[4] = { 0 };
    حرف *شروعات = *p;

    /* Check if we've already reported these tables */
    إذا (استعمال_ٿيل_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        أرجع 1;

    /* Don't introduce spurious end markers */
    إذا (type == 127)
        أرجع 0;

    حرف اڳياڙي[128];
    snprintf(اڳياڙي, حجم(اڳياڙي), "smbios/table%d-", type);
    بنية romfile_s *file = عدم;
    لكل (;;) {
        file = romfile_findprefix(اڳياڙي, file);
        إذا (!file)
            اكسر;

        إذا (پڇاڙي - *p < file->سائيز) {
            warn_noalloc();
            اكسر;
        }

        بنية smbios_structure_سرو *سرو = (فراغ*)*p;
        file->copy(file, سرو, file->سائيز);
        *p += file->سائيز;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        إذا (سرو->ڊيگهائي >= file->سائيز) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        إذا (*p - (حرف*)سرو > *max_struct_سائيز)
            *max_struct_سائيز = *p - (حرف*)سرو;
    }

    إذا (شروعات == *p)
        أرجع 0;

    /* Mark that we've reported on this type */
    استعمال_ٿيل_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    أرجع 1;
}

#وصف_ڪريو ڀريو_str_ميدان_with_default(type, ميدان, def)                   \
    افعل {                                                                \
        سائيز = get_ميدان(type, offsetof(بنية smbios_type_##type,      \
                                        ميدان), پڇاڙي);                   \
        إذا (سائيز == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->ميدان = 0;                                               \
        } وإلا إذا (سائيز > 1) {                                          \
            پڇاڙي += سائيز;                                                \
            p->ميدان = ++str_اشاريو;                                     \
        } وإلا {                                                        \
            يادگيري_نقل(پڇاڙي, def, حجم(def));                              \
            پڇاڙي += حجم(def);                                         \
            p->ميدان = ++str_اشاريو;                                     \
        }                                                               \
    } طالما (0)

#وصف_ڪريو ڀريو_str_ميدان_or_skip(type, ميدان)                             \
    افعل {                                                                \
        سائيز = get_ميدان(type, offsetof(بنية smbios_type_##type,      \
                                        ميدان), پڇاڙي);                   \
        إذا (سائيز > 1) {                                                 \
            پڇاڙي += سائيز;                                                \
            p->ميدان = ++str_اشاريو;                                     \
        } وإلا {                                                        \
            p->ميدان = 0;                                               \
        }                                                               \
    } طالما (0)

#وصف_ڪريو سيٽ_ميدان_with_default(type, ميدان, def)                        \
    افعل {                                                                \
        إذا (!get_ميدان(type, offsetof(بنية smbios_type_##type,        \
                                      ميدان), &p->ميدان)) {             \
            p->ميدان = def;                                             \
        }                                                               \
    } طالما (0)

/* Type 0 -- BIOS Information */
#وصف_ڪريو RELEASE_DATE_STR "01/01/2011"
ساكن فراغ *
smbios_init_type_0(فراغ *شروعات)
{
    بنية smbios_type_0 *p = (بنية smbios_type_0 *)شروعات;
    حرف *پڇاڙي = (حرف *)شروعات + حجم(بنية smbios_type_0);
    سائيز_t سائيز;
    صحيح str_اشاريو = 0;

    p->سرو.type = 0;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_0);
    p->سرو.handle = 0;

    ڀريو_str_ميدان_with_default(0, vendor_str, BUILD_APPNAME);
    ڀريو_str_ميدان_with_default(0, bios_نسخو_str, BUILD_APPNAME);

    p->bios_starting_پتو_segment = 0xe800;

    ڀريو_str_ميدان_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_سائيز = 0; /* FIXME */

    إذا (!get_ميدان(0, offsetof(بنية smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    إذا (!get_ميدان(0, offsetof(بنية smbios_type_0,
                               bios_characteristics_extension_بائيٽون),
                   &p->bios_characteristics_extension_بائيٽون)) {
        p->bios_characteristics_extension_بائيٽون[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_بائيٽون[1] = 4;
    }

    سيٽ_ميدان_with_default(0, system_bios_major_release, 1);
    سيٽ_ميدان_with_default(0, system_bios_minor_release, 0);
    سيٽ_ميدان_with_default(0, embedded_controller_major_release, 0xff);
    سيٽ_ميدان_with_default(0, embedded_controller_minor_release, 0xff);

    *پڇاڙي = 0;
    پڇاڙي++;
    إذا (!str_اشاريو) {
        *پڇاڙي = 0;
        پڇاڙي++;
    }

    أرجع پڇاڙي;
}

/* Type 1 -- System Information */
ساكن فراغ *
smbios_init_type_1(فراغ *شروعات)
{
    بنية smbios_type_1 *p = (بنية smbios_type_1 *)شروعات;
    حرف *پڇاڙي = (حرف *)شروعات + حجم(بنية smbios_type_1);
    سائيز_t سائيز;
    صحيح str_اشاريو = 0;

    p->سرو.type = 1;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_1);
    p->سرو.handle = 0x100;

    ڀريو_str_ميدان_with_default(1, manufacturer_str, BUILD_APPNAME);
    ڀريو_str_ميدان_with_default(1, حاصل_ضرب_name_str, BUILD_APPNAME);
    ڀريو_str_ميدان_or_skip(1, نسخو_str);
    ڀريو_str_ميدان_or_skip(1, serial_عدد_str);

    إذا (!get_ميدان(1, offsetof(بنية smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    سيٽ_ميدان_with_default(1, wake_مٿي_type, 0x06); /* power switch */

    ڀريو_str_ميدان_or_skip(1, sku_عدد_str);
    ڀريو_str_ميدان_or_skip(1, family_str);

    *پڇاڙي = 0;
    پڇاڙي++;
    إذا (!str_اشاريو) {
        *پڇاڙي = 0;
        پڇاڙي++;
    }

    أرجع پڇاڙي;
}

/* Type 3 -- System Enclosure */
ساكن فراغ *
smbios_init_type_3(فراغ *شروعات)
{
    بنية smbios_type_3 *p = (بنية smbios_type_3 *)شروعات;
    حرف *پڇاڙي = (حرف *)شروعات + حجم(بنية smbios_type_3);
    سائيز_t سائيز;
    صحيح str_اشاريو = 0;

    p->سرو.type = 3;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_3);
    p->سرو.handle = 0x300;

    ڀريو_str_ميدان_with_default(3, manufacturer_str, BUILD_APPNAME);
    سيٽ_ميدان_with_default(3, type, 0x01); /* other */

    ڀريو_str_ميدان_or_skip(3, نسخو_str);
    ڀريو_str_ميدان_or_skip(3, serial_عدد_str);
    ڀريو_str_ميدان_or_skip(3, asset_ٽيگ_عدد_str);

    سيٽ_ميدان_with_default(3, boot_مٿي_state, 0x03); /*محفوظ*/
    سيٽ_ميدان_with_default(3, power_supply_state, 0x03); /*محفوظ*/
    سيٽ_ميدان_with_default(3, thermal_state, 0x03); /*محفوظ*/
    سيٽ_ميدان_with_default(3, security_حالت, 0x02); /* unknown */

    سيٽ_ميدان_with_default(3, oem_defined, 0);
    سيٽ_ميدان_with_default(3, اوچائي, 0);
    سيٽ_ميدان_with_default(3, عدد_جو_power_cords, 0);
    سيٽ_ميدان_with_default(3, contained_element_ڳڻپ, 0);

    *پڇاڙي = 0;
    پڇاڙي++;
    إذا (!str_اشاريو) {
        *پڇاڙي = 0;
        پڇاڙي++;
    }

    أرجع پڇاڙي;
}

/* Type 4 -- Processor Information */
ساكن فراغ *
smbios_init_type_4(فراغ *شروعات, بنانشان صحيح cpu_عدد)
{
    بنية smbios_type_4 *p = (بنية smbios_type_4 *)شروعات;
    حرف *پڇاڙي = (حرف *)شروعات + حجم(بنية smbios_type_4);
    سائيز_t سائيز;
    صحيح str_اشاريو = 0;
    حرف name[1024];

    p->سرو.type = 4;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_4);
    p->سرو.handle = 0x400 + cpu_عدد;

    سائيز = get_ميدان(4, offsetof(بنية smbios_type_4, socket_designation_str),
                     name);
    إذا (سائيز)
        snprintf(name + سائيز - 1, حجم(name) - سائيز, "%2x", cpu_عدد);
    وإلا
        snprintf(name, حجم(name), "CPU%2x", cpu_عدد);

    يادگيري_نقل(پڇاڙي, name, لڙي_ڊيگهه(name) + 1);
    پڇاڙي += لڙي_ڊيگهه(name) + 1;
    p->socket_designation_str = ++str_اشاريو;

    سيٽ_ميدان_with_default(4, processor_type, 0x03); /* CPU */
    سيٽ_ميدان_with_default(4, processor_family, 0x01); /* other */

    ڀريو_str_ميدان_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    إذا (!get_ميدان(4, offsetof(بنية smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    ڀريو_str_ميدان_or_skip(4, processor_نسخو_str);
    سيٽ_ميدان_with_default(4, voltage, 0);
    سيٽ_ميدان_with_default(4, external_clock, 0);

    سيٽ_ميدان_with_default(4, max_رفتار, 2000);
    سيٽ_ميدان_with_default(4, current_رفتار, 2000);

    سيٽ_ميدان_with_default(4, حالت, 0x41); /* socket populated, CPU enabled */
    سيٽ_ميدان_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *پڇاڙي = 0;
    پڇاڙي++;
    إذا (!str_اشاريو) {
        *پڇاڙي = 0;
        پڇاڙي++;
    }

    أرجع پڇاڙي;
}

/* Type 16 -- Physical Memory Array */
ساكن فراغ *
smbios_init_type_16(فراغ *شروعات, u32 يادگيري_سائيز_mb, صحيح nr_mem_devs)
{
    بنية smbios_type_16 *p = (بنية smbios_type_16*)شروعات;

    p->سرو.type = 16;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_16);
    p->سرو.handle = 0x1000;

    سيٽ_ميدان_with_default(16, location, 0x01); /* other */
    سيٽ_ميدان_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    سيٽ_ميدان_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->وڌ_کان_وڌ_صلاحيت = يادگيري_سائيز_mb < 2 << 20 ?
                          يادگيري_سائيز_mb << 10 : 0x80000000;
    p->يادگيري_error_information_handle = 0xfffe; /* none provided */
    p->عدد_جو_يادگيري_devices = nr_mem_devs;

    شروعات += حجم(بنية smbios_type_16);
    *((u16 *)شروعات) = 0;

    أرجع شروعات + 2;
}

/* Type 17 -- Memory Device */
ساكن فراغ *
smbios_init_type_17(فراغ *شروعات, u32 سائيز_mb, صحيح instance)
{
    بنية smbios_type_17 *p = (بنية smbios_type_17 *)شروعات;
    حرف *پڇاڙي = (حرف *)شروعات + حجم(بنية smbios_type_17);
    سائيز_t سائيز;
    صحيح str_اشاريو = 0;
    حرف name[1024];

    p->سرو.type = 17;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_17);
    p->سرو.handle = 0x1100 + instance;

    p->physical_يادگيري_array_handle = 0x1000;
    سيٽ_ميدان_with_default(17, ڪل_ويڪر, 64);
    سيٽ_ميدان_with_default(17, معلومات_ويڪر, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->سائيز = سائيز_mb;
    سيٽ_ميدان_with_default(17, form_ضرب_ڪندڙ, 0x09); /* DIMM */
    p->device_سيٽ = 0;

    سائيز = get_ميدان(17, offsetof(بنية smbios_type_17, device_locator_str),
                     name);
    إذا (سائيز)
        snprintf(name + سائيز - 1, حجم(name) - سائيز, "%d", instance);
    وإلا
        snprintf(name, حجم(name), "DIMM %d", instance);

    يادگيري_نقل(پڇاڙي, name, لڙي_ڊيگهه(name) + 1);
    پڇاڙي += لڙي_ڊيگهه(name) + 1;
    p->device_locator_str = ++str_اشاريو;

    ڀريو_str_ميدان_or_skip(17, bank_locator_str);
    سيٽ_ميدان_with_default(17, يادگيري_type, 0x07); /* RAM */
    سيٽ_ميدان_with_default(17, type_detail, 0);

    *پڇاڙي = 0;
    پڇاڙي++;
    إذا (!str_اشاريو) {
        *پڇاڙي = 0;
        پڇاڙي++;
    }

    أرجع پڇاڙي;
}

/* Type 19 -- Memory Array Mapped Address */
ساكن فراغ *
smbios_init_type_19(فراغ *شروعات, u32 شروعات_mb, u32 سائيز_mb, صحيح instance)
{
    بنية smbios_type_19 *p = (بنية smbios_type_19 *)شروعات;

    p->سرو.type = 19;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_19);
    p->سرو.handle = 0x1300 + instance;

    p->starting_پتو = شروعات_mb << 10;
    p->ending_پتو = p->starting_پتو + (سائيز_mb << 10) - 1;
    p->يادگيري_array_handle = 0x1000;
    p->حصا_ڪريو_ويڪر = 1;

    شروعات += حجم(بنية smbios_type_19);
    *((u16 *)شروعات) = 0;

    أرجع شروعات + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ساكن فراغ *
smbios_init_type_20(فراغ *شروعات, u32 شروعات_mb, u32 سائيز_mb, صحيح instance,
                    صحيح dev_handle, صحيح array_handle)
{
    بنية smbios_type_20 *p = (بنية smbios_type_20 *)شروعات;

    p->سرو.type = 20;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_20);
    p->سرو.handle = 0x1400 + instance;

    p->starting_پتو = شروعات_mb << 10;
    p->ending_پتو = p->starting_پتو + (سائيز_mb << 10) - 1;
    p->يادگيري_device_handle = 0x1100 + dev_handle;
    p->يادگيري_array_mapped_پتو_handle = 0x1300 + array_handle;
    p->حصا_ڪريو_صف_position = 1;
    p->interleave_position = 0;
    p->interleaved_معلومات_اونهائي = 0;

    شروعات += حجم(بنية smbios_type_20);

    *((u16 *)شروعات) = 0;
    أرجع شروعات+2;
}

/* Type 32 -- System Boot Information */
ساكن فراغ *
smbios_init_type_32(فراغ *شروعات)
{
    بنية smbios_type_32 *p = (بنية smbios_type_32 *)شروعات;

    p->سرو.type = 32;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_32);
    p->سرو.handle = 0x2000;
    memset(p->reserved, 0, 6);
    سيٽ_ميدان_with_default(32, boot_حالت, 0); /* no errors detected */

    شروعات += حجم(بنية smbios_type_32);
    *((u16 *)شروعات) = 0;

    أرجع شروعات+2;
}

/* Type 127 -- End of Table */
ساكن فراغ *
smbios_init_type_127(فراغ *شروعات)
{
    بنية smbios_type_127 *p = (بنية smbios_type_127 *)شروعات;

    p->سرو.type = 127;
    p->سرو.ڊيگهائي = حجم(بنية smbios_type_127);
    p->سرو.handle = 0x7f00;

    شروعات += حجم(بنية smbios_type_127);
    *((u16 *)شروعات) = 0;

    أرجع شروعات + 2;
}

#وصف_ڪريو TEMPSMBIOSSIZE (32 * 1024)

فراغ
smbios_legacy_تياري(فراغ)
{
    إذا (! CONFIG_SMBIOS)
        أرجع;

    dprintf(3, "init SMBIOS tables\n");

    حرف *شروعات = malloc_tmphigh(TEMPSMBIOSSIZE);
    إذا (! شروعات) {
        warn_noalloc();
        أرجع;
    }
    memset(شروعات, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_سائيز = 0;
    حرف *q, *p = شروعات;
    حرف *پڇاڙي = شروعات + TEMPSMBIOSSIZE - حجم(بنية smbios_type_127);

#وصف_ڪريو جوڙيو_struct(type, دليلون...)                                       \
    افعل {                                                                \
        إذا (!get_external(type, &p, &nr_structs, &max_struct_سائيز, پڇاڙي)) { \
            q = smbios_init_type_##type(دليلون);                          \
            nr_structs++;                                               \
            إذا ((q - p) > max_struct_سائيز)                              \
                max_struct_سائيز = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } طالما (0)

    جوڙيو_struct(0, p);
    جوڙيو_struct(1, p);
    جوڙيو_struct(3, p);

    صحيح cpu_انگ;
    لكل (cpu_انگ = 1; cpu_انگ <= Max_ڳڻپ_CPUs; cpu_انگ++)
        جوڙيو_struct(4, p, cpu_انگ);

    صحيح ram_mb = (Ram_سائيز + Ram_سائيز_Over4_G) >> 20;
    صحيح nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    جوڙيو_struct(16, p, ram_mb, nr_mem_devs);

    صحيح i, j;
    لكل (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        جوڙيو_struct(17, p, dev_mb, i);
    }

    جوڙيو_struct(19, p, 0, Ram_سائيز >> 20, 0);
    إذا (Ram_سائيز_Over4_G)
        جوڙيو_struct(19, p, 4096, Ram_سائيز_Over4_G >> 20, 1);

    جوڙيو_struct(20, p, 0, Ram_سائيز >> 20, 0, 0, 0);
    إذا (Ram_سائيز_Over4_G) {
        u32 شروعات_mb = 4096;
        لكل (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            إذا (i == 0)
                dev_mb -= Ram_سائيز >> 20;

            جوڙيو_struct(20, p, شروعات_mb, dev_mb, j, i, 1);
            شروعات_mb += dev_mb;
        }
    }

    جوڙيو_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    لكل (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_سائيز, پڇاڙي);
    جوڙيو_struct(127, p);

#وصف_رد جوڙيو_struct

    smbios_21_داخلا_نقطو_تياري(max_struct_سائيز, p - شروعات, شروعات, nr_structs);
    حرر(شروعات);
}
