// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // RamSize
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_findprefix
#ਅਨ੍ਤਰ੍ਭਾਵ "std/smbios.h" // struct smbios_21_entry_point
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // MaxCountCPUs
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // cpuid

ਸਥਿਰ ਖਾਲੀ
smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ_ਤਿਆਰੀ(u16 max_structure_ਮਾਪ,
                         u16 structure_ਸਾਰਣੀ_ਲੰਮਾਈ,
                         ਖਾਲੀ *structure_ਸਾਰਣੀ_ਪਤਾ,
                         u16 ਸੰਖਿਆ_ਦਾ_structures)
{
    ਖਾਲੀ *finaltable;
    ਜੇ (structure_ਸਾਰਣੀ_ਲੰਮਾਈ <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_ਸਾਰਣੀ_ਲੰਮਾਈ);
    ਨਹੀਂਤਾਂ
        finaltable = malloc_ਉੱਚਾ(structure_ਸਾਰਣੀ_ਲੰਮਾਈ);
    ਜੇ (!finaltable) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(finaltable, structure_ਸਾਰਣੀ_ਪਤਾ, structure_ਸਾਰਣੀ_ਲੰਮਾਈ);

    ਬਣਤਰ smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ ep;
    memset(&ep, 0, ਆਕਾਰ(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.ਲੰਮਾਈ = 0x1f;
    ep.smbios_major_ਸੰਸਕਰਣ = 2;
    ep.smbios_minor_ਸੰਸਕਰਣ = 4;
    ep.max_structure_ਮਾਪ = max_structure_ਮਾਪ;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_ਸਾਰਣੀ_ਲੰਮਾਈ = structure_ਸਾਰਣੀ_ਲੰਮਾਈ;
    ep.structure_ਸਾਰਣੀ_ਪਤਾ = (u32)finaltable;
    ep.ਸੰਖਿਆ_ਦਾ_structures = ਸੰਖਿਆ_ਦਾ_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((ਖਾਲੀ*)&ep + 0x10, ep.ਲੰਮਾਈ - 0x10);

    copy_smbios_21(&ep);
}

ਸਥਿਰ ਪੂਰਨ
get_ਖੇਤਰ(ਪੂਰਨ type, ਪੂਰਨ ਵਿੱਥ_2, ਖਾਲੀ *dest)
{
    ਅੱਖਰ name[128];
    snprintf(name, ਆਕਾਰ(name), "smbios/field%d-%d", type, ਵਿੱਥ_2);
    ਬਣਤਰ romfile_s *file = romfile_ਲੱਭੋ(name);
    ਜੇ (!file)
        ਵਾਪਸ 0;
    file->copy(file, dest, file->ਮਾਪ);
    ਵਾਪਸ file->ਮਾਪ;
}

ਸਥਿਰ ਪੂਰਨ
get_external(ਪੂਰਨ type, ਅੱਖਰ **p, ਬਿਨਾਨਿਸ਼ਾਨੀ *nr_structs,
             ਬਿਨਾਨਿਸ਼ਾਨੀ *max_struct_ਮਾਪ, ਅੱਖਰ *ਸਿਰਾ)
{
    ਸਥਿਰ u64 ਵਰਤਿਆ_bitmap[4] = { 0 };
    ਅੱਖਰ *ਸ਼ੁਰੂ = *p;

    /* Check if we've already reported these tables */
    ਜੇ (ਵਰਤਿਆ_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        ਵਾਪਸ 1;

    /* Don't introduce spurious end markers */
    ਜੇ (type == 127)
        ਵਾਪਸ 0;

    ਅੱਖਰ ਅਗੇਤਰ[128];
    snprintf(ਅਗੇਤਰ, ਆਕਾਰ(ਅਗੇਤਰ), "smbios/table%d-", type);
    ਬਣਤਰ romfile_s *file = ਨਿਰਾ;
    ਲਈ (;;) {
        file = romfile_findprefix(ਅਗੇਤਰ, file);
        ਜੇ (!file)
            ਰੋਕੋ;

        ਜੇ (ਸਿਰਾ - *p < file->ਮਾਪ) {
            warn_noalloc();
            ਰੋਕੋ;
        }

        ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ *ਸਿਰਲੇਖ = (ਖਾਲੀ*)*p;
        file->copy(file, ਸਿਰਲੇਖ, file->ਮਾਪ);
        *p += file->ਮਾਪ;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        ਜੇ (ਸਿਰਲੇਖ->ਲੰਮਾਈ >= file->ਮਾਪ) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        ਜੇ (*p - (ਅੱਖਰ*)ਸਿਰਲੇਖ > *max_struct_ਮਾਪ)
            *max_struct_ਮਾਪ = *p - (ਅੱਖਰ*)ਸਿਰਲੇਖ;
    }

    ਜੇ (ਸ਼ੁਰੂ == *p)
        ਵਾਪਸ 0;

    /* Mark that we've reported on this type */
    ਵਰਤਿਆ_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    ਵਾਪਸ 1;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਦੋ_str_ਖੇਤਰ_with_default(type, ਖੇਤਰ, def)                   \
    ਕਰੋ {                                                                \
        ਮਾਪ = get_ਖੇਤਰ(type, offsetof(ਬਣਤਰ smbios_type_##type,      \
                                        ਖੇਤਰ), ਸਿਰਾ);                   \
        ਜੇ (ਮਾਪ == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->ਖੇਤਰ = 0;                                               \
        } ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ > 1) {                                          \
            ਸਿਰਾ += ਮਾਪ;                                                \
            p->ਖੇਤਰ = ++str_ਸੂਚਕਾਂਕ;                                     \
        } ਨਹੀਂਤਾਂ {                                                        \
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਸਿਰਾ, def, ਆਕਾਰ(def));                              \
            ਸਿਰਾ += ਆਕਾਰ(def);                                         \
            p->ਖੇਤਰ = ++str_ਸੂਚਕਾਂਕ;                                     \
        }                                                               \
    } ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਦੋ_str_ਖੇਤਰ_or_skip(type, ਖੇਤਰ)                             \
    ਕਰੋ {                                                                \
        ਮਾਪ = get_ਖੇਤਰ(type, offsetof(ਬਣਤਰ smbios_type_##type,      \
                                        ਖੇਤਰ), ਸਿਰਾ);                   \
        ਜੇ (ਮਾਪ > 1) {                                                 \
            ਸਿਰਾ += ਮਾਪ;                                                \
            p->ਖੇਤਰ = ++str_ਸੂਚਕਾਂਕ;                                     \
        } ਨਹੀਂਤਾਂ {                                                        \
            p->ਖੇਤਰ = 0;                                               \
        }                                                               \
    } ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਖੇਤਰ_with_default(type, ਖੇਤਰ, def)                        \
    ਕਰੋ {                                                                \
        ਜੇ (!get_ਖੇਤਰ(type, offsetof(ਬਣਤਰ smbios_type_##type,        \
                                      ਖੇਤਰ), &p->ਖੇਤਰ)) {             \
            p->ਖੇਤਰ = def;                                             \
        }                                                               \
    } ਜਦੋਂ (0)

/* Type 0 -- BIOS Information */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASE_DATE_STR "01/01/2011"
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_0(ਖਾਲੀ *ਸ਼ੁਰੂ)
{
    ਬਣਤਰ smbios_type_0 *p = (ਬਣਤਰ smbios_type_0 *)ਸ਼ੁਰੂ;
    ਅੱਖਰ *ਸਿਰਾ = (ਅੱਖਰ *)ਸ਼ੁਰੂ + ਆਕਾਰ(ਬਣਤਰ smbios_type_0);
    ਮਾਪ_t ਮਾਪ;
    ਪੂਰਨ str_ਸੂਚਕਾਂਕ = 0;

    p->ਸਿਰਲੇਖ.type = 0;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_0);
    p->ਸਿਰਲੇਖ.handle = 0;

    ਲੱਦੋ_str_ਖੇਤਰ_with_default(0, vendor_str, BUILD_APPNAME);
    ਲੱਦੋ_str_ਖੇਤਰ_with_default(0, bios_ਸੰਸਕਰਣ_str, BUILD_APPNAME);

    p->bios_starting_ਪਤਾ_segment = 0xe800;

    ਲੱਦੋ_str_ਖੇਤਰ_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_ਮਾਪ = 0; /* FIXME */

    ਜੇ (!get_ਖੇਤਰ(0, offsetof(ਬਣਤਰ smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    ਜੇ (!get_ਖੇਤਰ(0, offsetof(ਬਣਤਰ smbios_type_0,
                               bios_characteristics_extension_ਬਾਈਟਾਂ),
                   &p->bios_characteristics_extension_ਬਾਈਟਾਂ)) {
        p->bios_characteristics_extension_ਬਾਈਟਾਂ[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_ਬਾਈਟਾਂ[1] = 4;
    }

    ਸੈੱਟ_ਖੇਤਰ_with_default(0, system_bios_major_release, 1);
    ਸੈੱਟ_ਖੇਤਰ_with_default(0, system_bios_minor_release, 0);
    ਸੈੱਟ_ਖੇਤਰ_with_default(0, embedded_controller_major_release, 0xff);
    ਸੈੱਟ_ਖੇਤਰ_with_default(0, embedded_controller_minor_release, 0xff);

    *ਸਿਰਾ = 0;
    ਸਿਰਾ++;
    ਜੇ (!str_ਸੂਚਕਾਂਕ) {
        *ਸਿਰਾ = 0;
        ਸਿਰਾ++;
    }

    ਵਾਪਸ ਸਿਰਾ;
}

/* Type 1 -- System Information */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_1(ਖਾਲੀ *ਸ਼ੁਰੂ)
{
    ਬਣਤਰ smbios_type_1 *p = (ਬਣਤਰ smbios_type_1 *)ਸ਼ੁਰੂ;
    ਅੱਖਰ *ਸਿਰਾ = (ਅੱਖਰ *)ਸ਼ੁਰੂ + ਆਕਾਰ(ਬਣਤਰ smbios_type_1);
    ਮਾਪ_t ਮਾਪ;
    ਪੂਰਨ str_ਸੂਚਕਾਂਕ = 0;

    p->ਸਿਰਲੇਖ.type = 1;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_1);
    p->ਸਿਰਲੇਖ.handle = 0x100;

    ਲੱਦੋ_str_ਖੇਤਰ_with_default(1, manufacturer_str, BUILD_APPNAME);
    ਲੱਦੋ_str_ਖੇਤਰ_with_default(1, ਗੁਣਨਫਲ_name_str, BUILD_APPNAME);
    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(1, ਸੰਸਕਰਣ_str);
    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(1, serial_ਸੰਖਿਆ_str);

    ਜੇ (!get_ਖੇਤਰ(1, offsetof(ਬਣਤਰ smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    ਸੈੱਟ_ਖੇਤਰ_with_default(1, wake_ਉੱਪਰ_type, 0x06); /* power switch */

    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(1, sku_ਸੰਖਿਆ_str);
    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(1, family_str);

    *ਸਿਰਾ = 0;
    ਸਿਰਾ++;
    ਜੇ (!str_ਸੂਚਕਾਂਕ) {
        *ਸਿਰਾ = 0;
        ਸਿਰਾ++;
    }

    ਵਾਪਸ ਸਿਰਾ;
}

/* Type 3 -- System Enclosure */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_3(ਖਾਲੀ *ਸ਼ੁਰੂ)
{
    ਬਣਤਰ smbios_type_3 *p = (ਬਣਤਰ smbios_type_3 *)ਸ਼ੁਰੂ;
    ਅੱਖਰ *ਸਿਰਾ = (ਅੱਖਰ *)ਸ਼ੁਰੂ + ਆਕਾਰ(ਬਣਤਰ smbios_type_3);
    ਮਾਪ_t ਮਾਪ;
    ਪੂਰਨ str_ਸੂਚਕਾਂਕ = 0;

    p->ਸਿਰਲੇਖ.type = 3;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_3);
    p->ਸਿਰਲੇਖ.handle = 0x300;

    ਲੱਦੋ_str_ਖੇਤਰ_with_default(3, manufacturer_str, BUILD_APPNAME);
    ਸੈੱਟ_ਖੇਤਰ_with_default(3, type, 0x01); /* other */

    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(3, ਸੰਸਕਰਣ_str);
    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(3, serial_ਸੰਖਿਆ_str);
    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(3, asset_ਟੈਗ_ਸੰਖਿਆ_str);

    ਸੈੱਟ_ਖੇਤਰ_with_default(3, boot_ਉੱਪਰ_state, 0x03); /*ਸੁਰੱਖਿਅਤ*/
    ਸੈੱਟ_ਖੇਤਰ_with_default(3, power_supply_state, 0x03); /*ਸੁਰੱਖਿਅਤ*/
    ਸੈੱਟ_ਖੇਤਰ_with_default(3, thermal_state, 0x03); /*ਸੁਰੱਖਿਅਤ*/
    ਸੈੱਟ_ਖੇਤਰ_with_default(3, security_ਸਥਿਤੀ, 0x02); /* unknown */

    ਸੈੱਟ_ਖੇਤਰ_with_default(3, oem_defined, 0);
    ਸੈੱਟ_ਖੇਤਰ_with_default(3, ਉਚਾਈ, 0);
    ਸੈੱਟ_ਖੇਤਰ_with_default(3, ਸੰਖਿਆ_ਦਾ_power_cords, 0);
    ਸੈੱਟ_ਖੇਤਰ_with_default(3, contained_element_ਗਿਣਤੀ, 0);

    *ਸਿਰਾ = 0;
    ਸਿਰਾ++;
    ਜੇ (!str_ਸੂਚਕਾਂਕ) {
        *ਸਿਰਾ = 0;
        ਸਿਰਾ++;
    }

    ਵਾਪਸ ਸਿਰਾ;
}

/* Type 4 -- Processor Information */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_4(ਖਾਲੀ *ਸ਼ੁਰੂ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu_ਸੰਖਿਆ)
{
    ਬਣਤਰ smbios_type_4 *p = (ਬਣਤਰ smbios_type_4 *)ਸ਼ੁਰੂ;
    ਅੱਖਰ *ਸਿਰਾ = (ਅੱਖਰ *)ਸ਼ੁਰੂ + ਆਕਾਰ(ਬਣਤਰ smbios_type_4);
    ਮਾਪ_t ਮਾਪ;
    ਪੂਰਨ str_ਸੂਚਕਾਂਕ = 0;
    ਅੱਖਰ name[1024];

    p->ਸਿਰਲੇਖ.type = 4;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_4);
    p->ਸਿਰਲੇਖ.handle = 0x400 + cpu_ਸੰਖਿਆ;

    ਮਾਪ = get_ਖੇਤਰ(4, offsetof(ਬਣਤਰ smbios_type_4, socket_designation_str),
                     name);
    ਜੇ (ਮਾਪ)
        snprintf(name + ਮਾਪ - 1, ਆਕਾਰ(name) - ਮਾਪ, "%2x", cpu_ਸੰਖਿਆ);
    ਨਹੀਂਤਾਂ
        snprintf(name, ਆਕਾਰ(name), "CPU%2x", cpu_ਸੰਖਿਆ);

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਸਿਰਾ, name, ਲੜੀ_ਲੰਬਾਈ(name) + 1);
    ਸਿਰਾ += ਲੜੀ_ਲੰਬਾਈ(name) + 1;
    p->socket_designation_str = ++str_ਸੂਚਕਾਂਕ;

    ਸੈੱਟ_ਖੇਤਰ_with_default(4, processor_type, 0x03); /* CPU */
    ਸੈੱਟ_ਖੇਤਰ_with_default(4, processor_family, 0x01); /* other */

    ਲੱਦੋ_str_ਖੇਤਰ_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    ਜੇ (!get_ਖੇਤਰ(4, offsetof(ਬਣਤਰ smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(4, processor_ਸੰਸਕਰਣ_str);
    ਸੈੱਟ_ਖੇਤਰ_with_default(4, voltage, 0);
    ਸੈੱਟ_ਖੇਤਰ_with_default(4, external_clock, 0);

    ਸੈੱਟ_ਖੇਤਰ_with_default(4, max_ਰਫਤਾਰ, 2000);
    ਸੈੱਟ_ਖੇਤਰ_with_default(4, current_ਰਫਤਾਰ, 2000);

    ਸੈੱਟ_ਖੇਤਰ_with_default(4, ਸਥਿਤੀ, 0x41); /* socket populated, CPU enabled */
    ਸੈੱਟ_ਖੇਤਰ_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *ਸਿਰਾ = 0;
    ਸਿਰਾ++;
    ਜੇ (!str_ਸੂਚਕਾਂਕ) {
        *ਸਿਰਾ = 0;
        ਸਿਰਾ++;
    }

    ਵਾਪਸ ਸਿਰਾ;
}

/* Type 16 -- Physical Memory Array */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_16(ਖਾਲੀ *ਸ਼ੁਰੂ, u32 ਸਮ੍ਰਿਤੀ_ਮਾਪ_mb, ਪੂਰਨ nr_mem_devs)
{
    ਬਣਤਰ smbios_type_16 *p = (ਬਣਤਰ smbios_type_16*)ਸ਼ੁਰੂ;

    p->ਸਿਰਲੇਖ.type = 16;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_16);
    p->ਸਿਰਲੇਖ.handle = 0x1000;

    ਸੈੱਟ_ਖੇਤਰ_with_default(16, location, 0x01); /* other */
    ਸੈੱਟ_ਖੇਤਰ_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    ਸੈੱਟ_ਖੇਤਰ_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->ਵੱਧ_ਤੋਂ_ਵੱਧ_ਸਮਰੱਥਾ = ਸਮ੍ਰਿਤੀ_ਮਾਪ_mb < 2 << 20 ?
                          ਸਮ੍ਰਿਤੀ_ਮਾਪ_mb << 10 : 0x80000000;
    p->ਸਮ੍ਰਿਤੀ_error_information_handle = 0xfffe; /* none provided */
    p->ਸੰਖਿਆ_ਦਾ_ਸਮ੍ਰਿਤੀ_devices = nr_mem_devs;

    ਸ਼ੁਰੂ += ਆਕਾਰ(ਬਣਤਰ smbios_type_16);
    *((u16 *)ਸ਼ੁਰੂ) = 0;

    ਵਾਪਸ ਸ਼ੁਰੂ + 2;
}

/* Type 17 -- Memory Device */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_17(ਖਾਲੀ *ਸ਼ੁਰੂ, u32 ਮਾਪ_mb, ਪੂਰਨ instance)
{
    ਬਣਤਰ smbios_type_17 *p = (ਬਣਤਰ smbios_type_17 *)ਸ਼ੁਰੂ;
    ਅੱਖਰ *ਸਿਰਾ = (ਅੱਖਰ *)ਸ਼ੁਰੂ + ਆਕਾਰ(ਬਣਤਰ smbios_type_17);
    ਮਾਪ_t ਮਾਪ;
    ਪੂਰਨ str_ਸੂਚਕਾਂਕ = 0;
    ਅੱਖਰ name[1024];

    p->ਸਿਰਲੇਖ.type = 17;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_17);
    p->ਸਿਰਲੇਖ.handle = 0x1100 + instance;

    p->physical_ਸਮ੍ਰਿਤੀ_array_handle = 0x1000;
    ਸੈੱਟ_ਖੇਤਰ_with_default(17, ਕੁੱਲ_ਚੌੜਾਈ, 64);
    ਸੈੱਟ_ਖੇਤਰ_with_default(17, ਅੰਕੜੇ_ਚੌੜਾਈ, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->ਮਾਪ = ਮਾਪ_mb;
    ਸੈੱਟ_ਖੇਤਰ_with_default(17, form_ਗੁਣਕ, 0x09); /* DIMM */
    p->device_ਸੈੱਟ = 0;

    ਮਾਪ = get_ਖੇਤਰ(17, offsetof(ਬਣਤਰ smbios_type_17, device_locator_str),
                     name);
    ਜੇ (ਮਾਪ)
        snprintf(name + ਮਾਪ - 1, ਆਕਾਰ(name) - ਮਾਪ, "%d", instance);
    ਨਹੀਂਤਾਂ
        snprintf(name, ਆਕਾਰ(name), "DIMM %d", instance);

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਸਿਰਾ, name, ਲੜੀ_ਲੰਬਾਈ(name) + 1);
    ਸਿਰਾ += ਲੜੀ_ਲੰਬਾਈ(name) + 1;
    p->device_locator_str = ++str_ਸੂਚਕਾਂਕ;

    ਲੱਦੋ_str_ਖੇਤਰ_or_skip(17, bank_locator_str);
    ਸੈੱਟ_ਖੇਤਰ_with_default(17, ਸਮ੍ਰਿਤੀ_type, 0x07); /* RAM */
    ਸੈੱਟ_ਖੇਤਰ_with_default(17, type_detail, 0);

    *ਸਿਰਾ = 0;
    ਸਿਰਾ++;
    ਜੇ (!str_ਸੂਚਕਾਂਕ) {
        *ਸਿਰਾ = 0;
        ਸਿਰਾ++;
    }

    ਵਾਪਸ ਸਿਰਾ;
}

/* Type 19 -- Memory Array Mapped Address */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_19(ਖਾਲੀ *ਸ਼ੁਰੂ, u32 ਸ਼ੁਰੂ_mb, u32 ਮਾਪ_mb, ਪੂਰਨ instance)
{
    ਬਣਤਰ smbios_type_19 *p = (ਬਣਤਰ smbios_type_19 *)ਸ਼ੁਰੂ;

    p->ਸਿਰਲੇਖ.type = 19;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_19);
    p->ਸਿਰਲੇਖ.handle = 0x1300 + instance;

    p->starting_ਪਤਾ = ਸ਼ੁਰੂ_mb << 10;
    p->ending_ਪਤਾ = p->starting_ਪਤਾ + (ਮਾਪ_mb << 10) - 1;
    p->ਸਮ੍ਰਿਤੀ_array_handle = 0x1000;
    p->ਹਿੱਸੇ_ਕਰੋ_ਚੌੜਾਈ = 1;

    ਸ਼ੁਰੂ += ਆਕਾਰ(ਬਣਤਰ smbios_type_19);
    *((u16 *)ਸ਼ੁਰੂ) = 0;

    ਵਾਪਸ ਸ਼ੁਰੂ + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_20(ਖਾਲੀ *ਸ਼ੁਰੂ, u32 ਸ਼ੁਰੂ_mb, u32 ਮਾਪ_mb, ਪੂਰਨ instance,
                    ਪੂਰਨ dev_handle, ਪੂਰਨ array_handle)
{
    ਬਣਤਰ smbios_type_20 *p = (ਬਣਤਰ smbios_type_20 *)ਸ਼ੁਰੂ;

    p->ਸਿਰਲੇਖ.type = 20;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_20);
    p->ਸਿਰਲੇਖ.handle = 0x1400 + instance;

    p->starting_ਪਤਾ = ਸ਼ੁਰੂ_mb << 10;
    p->ending_ਪਤਾ = p->starting_ਪਤਾ + (ਮਾਪ_mb << 10) - 1;
    p->ਸਮ੍ਰਿਤੀ_device_handle = 0x1100 + dev_handle;
    p->ਸਮ੍ਰਿਤੀ_array_mapped_ਪਤਾ_handle = 0x1300 + array_handle;
    p->ਹਿੱਸੇ_ਕਰੋ_ਪੰਕਤੀ_position = 1;
    p->interleave_position = 0;
    p->interleaved_ਅੰਕੜੇ_ਡੂੰਘਾਈ = 0;

    ਸ਼ੁਰੂ += ਆਕਾਰ(ਬਣਤਰ smbios_type_20);

    *((u16 *)ਸ਼ੁਰੂ) = 0;
    ਵਾਪਸ ਸ਼ੁਰੂ+2;
}

/* Type 32 -- System Boot Information */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_32(ਖਾਲੀ *ਸ਼ੁਰੂ)
{
    ਬਣਤਰ smbios_type_32 *p = (ਬਣਤਰ smbios_type_32 *)ਸ਼ੁਰੂ;

    p->ਸਿਰਲੇਖ.type = 32;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_32);
    p->ਸਿਰਲੇਖ.handle = 0x2000;
    memset(p->reserved, 0, 6);
    ਸੈੱਟ_ਖੇਤਰ_with_default(32, boot_ਸਥਿਤੀ, 0); /* no errors detected */

    ਸ਼ੁਰੂ += ਆਕਾਰ(ਬਣਤਰ smbios_type_32);
    *((u16 *)ਸ਼ੁਰੂ) = 0;

    ਵਾਪਸ ਸ਼ੁਰੂ+2;
}

/* Type 127 -- End of Table */
ਸਥਿਰ ਖਾਲੀ *
smbios_init_type_127(ਖਾਲੀ *ਸ਼ੁਰੂ)
{
    ਬਣਤਰ smbios_type_127 *p = (ਬਣਤਰ smbios_type_127 *)ਸ਼ੁਰੂ;

    p->ਸਿਰਲੇਖ.type = 127;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_127);
    p->ਸਿਰਲੇਖ.handle = 0x7f00;

    ਸ਼ੁਰੂ += ਆਕਾਰ(ਬਣਤਰ smbios_type_127);
    *((u16 *)ਸ਼ੁਰੂ) = 0;

    ਵਾਪਸ ਸ਼ੁਰੂ + 2;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TEMPSMBIOSSIZE (32 * 1024)

ਖਾਲੀ
smbios_legacy_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_SMBIOS)
        ਵਾਪਸ;

    dprintf(3, "init SMBIOS tables\n");

    ਅੱਖਰ *ਸ਼ੁਰੂ = malloc_tmphigh(TEMPSMBIOSSIZE);
    ਜੇ (! ਸ਼ੁਰੂ) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(ਸ਼ੁਰੂ, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_ਮਾਪ = 0;
    ਅੱਖਰ *q, *p = ਸ਼ੁਰੂ;
    ਅੱਖਰ *ਸਿਰਾ = ਸ਼ੁਰੂ + TEMPSMBIOSSIZE - ਆਕਾਰ(ਬਣਤਰ smbios_type_127);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਜੋੜੋ_struct(type, ਦਲੀਲਾਂ...)                                       \
    ਕਰੋ {                                                                \
        ਜੇ (!get_external(type, &p, &nr_structs, &max_struct_ਮਾਪ, ਸਿਰਾ)) { \
            q = smbios_init_type_##type(ਦਲੀਲਾਂ);                          \
            nr_structs++;                                               \
            ਜੇ ((q - p) > max_struct_ਮਾਪ)                              \
                max_struct_ਮਾਪ = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } ਜਦੋਂ (0)

    ਜੋੜੋ_struct(0, p);
    ਜੋੜੋ_struct(1, p);
    ਜੋੜੋ_struct(3, p);

    ਪੂਰਨ cpu_ਅੰਕ;
    ਲਈ (cpu_ਅੰਕ = 1; cpu_ਅੰਕ <= Max_ਗਿਣਤੀ_CPUs; cpu_ਅੰਕ++)
        ਜੋੜੋ_struct(4, p, cpu_ਅੰਕ);

    ਪੂਰਨ ram_mb = (Ram_ਮਾਪ + Ram_ਮਾਪ_Over4_G) >> 20;
    ਪੂਰਨ nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    ਜੋੜੋ_struct(16, p, ram_mb, nr_mem_devs);

    ਪੂਰਨ i, j;
    ਲਈ (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        ਜੋੜੋ_struct(17, p, dev_mb, i);
    }

    ਜੋੜੋ_struct(19, p, 0, Ram_ਮਾਪ >> 20, 0);
    ਜੇ (Ram_ਮਾਪ_Over4_G)
        ਜੋੜੋ_struct(19, p, 4096, Ram_ਮਾਪ_Over4_G >> 20, 1);

    ਜੋੜੋ_struct(20, p, 0, Ram_ਮਾਪ >> 20, 0, 0, 0);
    ਜੇ (Ram_ਮਾਪ_Over4_G) {
        u32 ਸ਼ੁਰੂ_mb = 4096;
        ਲਈ (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            ਜੇ (i == 0)
                dev_mb -= Ram_ਮਾਪ >> 20;

            ਜੋੜੋ_struct(20, p, ਸ਼ੁਰੂ_mb, dev_mb, j, i, 1);
            ਸ਼ੁਰੂ_mb += dev_mb;
        }
    }

    ਜੋੜੋ_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    ਲਈ (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_ਮਾਪ, ਸਿਰਾ);
    ਜੋੜੋ_struct(127, p);

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਜੋੜੋ_struct

    smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ_ਤਿਆਰੀ(max_struct_ਮਾਪ, p - ਸ਼ੁਰੂ, ਸ਼ੁਰੂ, nr_structs);
    ਮੁਕਤ(ਸ਼ੁਰੂ);
}
