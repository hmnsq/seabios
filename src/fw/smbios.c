// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // RamSize
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_findprefix
#ಅನ್ತರ್ಭಾವ "std/smbios.h" // struct smbios_21_entry_point
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // MaxCountCPUs
#ಅನ್ತರ್ಭಾವ "x86.h" // cpuid

ಸ್ಥಿರ ಶೂನ್ಯ
smbios_21_ನಮೂದು_ಬಿಂದು_ಸಿದ್ಧತೆ(u16 max_structure_ಗಾತ್ರ,
                         u16 structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ,
                         ಶೂನ್ಯ *structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ,
                         u16 ಸಂಖ್ಯೆ_ದ_structures)
{
    ಶೂನ್ಯ *finaltable;
    ಆದರೆ (structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ);
    ಇಲ್ಲದಿದ್ದರೆ
        finaltable = malloc_ಮೇಲಿನ(structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ);
    ಆದರೆ (!finaltable) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಮರಣೆ_ನಕಲು(finaltable, structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ, structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ);

    ರಚನೆ smbios_21_ನಮೂದು_ಬಿಂದು ep;
    memset(&ep, 0, ಗಾತ್ರ(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.ಉದ್ದಳತೆ = 0x1f;
    ep.smbios_major_ಆವೃತ್ತಿ = 2;
    ep.smbios_minor_ಆವೃತ್ತಿ = 4;
    ep.max_structure_ಗಾತ್ರ = max_structure_ಗಾತ್ರ;
    ಸ್ಮರಣೆ_ನಕಲು(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ = structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ;
    ep.structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ = (u32)finaltable;
    ep.ಸಂಖ್ಯೆ_ದ_structures = ಸಂಖ್ಯೆ_ದ_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((ಶೂನ್ಯ*)&ep + 0x10, ep.ಉದ್ದಳತೆ - 0x10);

    copy_smbios_21(&ep);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_ಕ್ಷೇತ್ರ(ಪೂರ್ಣಾಂಕ type, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2, ಶೂನ್ಯ *dest)
{
    ಅಕ್ಷರ name[128];
    snprintf(name, ಗಾತ್ರ(name), "smbios/field%d-%d", type, ಸ್ಥಾನಾಂತರ_2);
    ರಚನೆ romfile_s *file = romfile_ಕಂಡುಹಿಡಿ(name);
    ಆದರೆ (!file)
        ಹಿಂತಿರುಗಿ 0;
    file->copy(file, dest, file->ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ file->ಗಾತ್ರ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_external(ಪೂರ್ಣಾಂಕ type, ಅಕ್ಷರ **p, ಚಿಹ್ನೆರಹಿತ *nr_structs,
             ಚಿಹ್ನೆರಹಿತ *max_struct_ಗಾತ್ರ, ಅಕ್ಷರ *ಕೊನೆ)
{
    ಸ್ಥಿರ u64 ಬಳಸಿದ_bitmap[4] = { 0 };
    ಅಕ್ಷರ *ಆರಂಭ = *p;

    /* Check if we've already reported these tables */
    ಆದರೆ (ಬಳಸಿದ_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        ಹಿಂತಿರುಗಿ 1;

    /* Don't introduce spurious end markers */
    ಆದರೆ (type == 127)
        ಹಿಂತಿರುಗಿ 0;

    ಅಕ್ಷರ ಪೂರ್ವಪ್ರತ್ಯಯ[128];
    snprintf(ಪೂರ್ವಪ್ರತ್ಯಯ, ಗಾತ್ರ(ಪೂರ್ವಪ್ರತ್ಯಯ), "smbios/table%d-", type);
    ರಚನೆ romfile_s *file = ಖಾಲಿ;
    ಗಾಗಿ (;;) {
        file = romfile_findprefix(ಪೂರ್ವಪ್ರತ್ಯಯ, file);
        ಆದರೆ (!file)
            ನಿಲ್ಲಿಸು;

        ಆದರೆ (ಕೊನೆ - *p < file->ಗಾತ್ರ_2) {
            warn_noalloc();
            ನಿಲ್ಲಿಸು;
        }

        ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ *ಶೀರ್ಷಿಕೆ = (ಶೂನ್ಯ*)*p;
        file->copy(file, ಶೀರ್ಷಿಕೆ, file->ಗಾತ್ರ_2);
        *p += file->ಗಾತ್ರ_2;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        ಆದರೆ (ಶೀರ್ಷಿಕೆ->ಉದ್ದಳತೆ >= file->ಗಾತ್ರ_2) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        ಆದರೆ (*p - (ಅಕ್ಷರ*)ಶೀರ್ಷಿಕೆ > *max_struct_ಗಾತ್ರ)
            *max_struct_ಗಾತ್ರ = *p - (ಅಕ್ಷರ*)ಶೀರ್ಷಿಕೆ;
    }

    ಆದರೆ (ಆರಂಭ == *p)
        ಹಿಂತಿರುಗಿ 0;

    /* Mark that we've reported on this type */
    ಬಳಸಿದ_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    ಹಿಂತಿರುಗಿ 1;
}

#ವ್ಯಾಖ್ಯಾನಿಸು ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(type, ಕ್ಷೇತ್ರ, def)                   \
    ಮಾಡು {                                                                \
        ಗಾತ್ರ_2 = get_ಕ್ಷೇತ್ರ(type, offsetof(ರಚನೆ smbios_type_##type,      \
                                        ಕ್ಷೇತ್ರ), ಕೊನೆ);                   \
        ಆದರೆ (ಗಾತ್ರ_2 == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->ಕ್ಷೇತ್ರ = 0;                                               \
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 > 1) {                                          \
            ಕೊನೆ += ಗಾತ್ರ_2;                                                \
            p->ಕ್ಷೇತ್ರ = ++str_ಸೂಚ್ಯಂಕ;                                     \
        } ಇಲ್ಲದಿದ್ದರೆ {                                                        \
            ಸ್ಮರಣೆ_ನಕಲು(ಕೊನೆ, def, ಗಾತ್ರ(def));                              \
            ಕೊನೆ += ಗಾತ್ರ(def);                                         \
            p->ಕ್ಷೇತ್ರ = ++str_ಸೂಚ್ಯಂಕ;                                     \
        }                                                               \
    } ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(type, ಕ್ಷೇತ್ರ)                             \
    ಮಾಡು {                                                                \
        ಗಾತ್ರ_2 = get_ಕ್ಷೇತ್ರ(type, offsetof(ರಚನೆ smbios_type_##type,      \
                                        ಕ್ಷೇತ್ರ), ಕೊನೆ);                   \
        ಆದರೆ (ಗಾತ್ರ_2 > 1) {                                                 \
            ಕೊನೆ += ಗಾತ್ರ_2;                                                \
            p->ಕ್ಷೇತ್ರ = ++str_ಸೂಚ್ಯಂಕ;                                     \
        } ಇಲ್ಲದಿದ್ದರೆ {                                                        \
            p->ಕ್ಷೇತ್ರ = 0;                                               \
        }                                                               \
    } ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_ಕ್ಷೇತ್ರ_with_default(type, ಕ್ಷೇತ್ರ, def)                        \
    ಮಾಡು {                                                                \
        ಆದರೆ (!get_ಕ್ಷೇತ್ರ(type, offsetof(ರಚನೆ smbios_type_##type,        \
                                      ಕ್ಷೇತ್ರ), &p->ಕ್ಷೇತ್ರ)) {             \
            p->ಕ್ಷೇತ್ರ = def;                                             \
        }                                                               \
    } ಆಗ (0)

/* Type 0 -- BIOS Information */
#ವ್ಯಾಖ್ಯಾನಿಸು RELEASE_DATE_STR "01/01/2011"
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_0(ಶೂನ್ಯ *ಆರಂಭ)
{
    ರಚನೆ smbios_type_0 *p = (ರಚನೆ smbios_type_0 *)ಆರಂಭ;
    ಅಕ್ಷರ *ಕೊನೆ = (ಅಕ್ಷರ *)ಆರಂಭ + ಗಾತ್ರ(ರಚನೆ smbios_type_0);
    ಗಾತ್ರ_t ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ str_ಸೂಚ್ಯಂಕ = 0;

    p->ಶೀರ್ಷಿಕೆ.type = 0;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_0);
    p->ಶೀರ್ಷಿಕೆ.handle = 0;

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(0, vendor_str, BUILD_APPNAME);
    ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(0, bios_ಆವೃತ್ತಿ_str, BUILD_APPNAME);

    p->bios_starting_ವಿಳಾಸ_segment = 0xe800;

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_ಗಾತ್ರ = 0; /* FIXME */

    ಆದರೆ (!get_ಕ್ಷೇತ್ರ(0, offsetof(ರಚನೆ smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    ಆದರೆ (!get_ಕ್ಷೇತ್ರ(0, offsetof(ರಚನೆ smbios_type_0,
                               bios_characteristics_extension_ಬೈಟುಗಳು),
                   &p->bios_characteristics_extension_ಬೈಟುಗಳು)) {
        p->bios_characteristics_extension_ಬೈಟುಗಳು[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_ಬೈಟುಗಳು[1] = 4;
    }

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(0, system_bios_major_release, 1);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(0, system_bios_minor_release, 0);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(0, embedded_controller_major_release, 0xff);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(0, embedded_controller_minor_release, 0xff);

    *ಕೊನೆ = 0;
    ಕೊನೆ++;
    ಆದರೆ (!str_ಸೂಚ್ಯಂಕ) {
        *ಕೊನೆ = 0;
        ಕೊನೆ++;
    }

    ಹಿಂತಿರುಗಿ ಕೊನೆ;
}

/* Type 1 -- System Information */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_1(ಶೂನ್ಯ *ಆರಂಭ)
{
    ರಚನೆ smbios_type_1 *p = (ರಚನೆ smbios_type_1 *)ಆರಂಭ;
    ಅಕ್ಷರ *ಕೊನೆ = (ಅಕ್ಷರ *)ಆರಂಭ + ಗಾತ್ರ(ರಚನೆ smbios_type_1);
    ಗಾತ್ರ_t ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ str_ಸೂಚ್ಯಂಕ = 0;

    p->ಶೀರ್ಷಿಕೆ.type = 1;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_1);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x100;

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(1, manufacturer_str, BUILD_APPNAME);
    ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(1, ಗುಣಲಬ್ಧ_name_str, BUILD_APPNAME);
    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(1, ಆವೃತ್ತಿ_str);
    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(1, serial_ಸಂಖ್ಯೆ_str);

    ಆದರೆ (!get_ಕ್ಷೇತ್ರ(1, offsetof(ರಚನೆ smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(1, wake_ಮೇಲೆ_type, 0x06); /* power switch */

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(1, sku_ಸಂಖ್ಯೆ_str);
    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(1, family_str);

    *ಕೊನೆ = 0;
    ಕೊನೆ++;
    ಆದರೆ (!str_ಸೂಚ್ಯಂಕ) {
        *ಕೊನೆ = 0;
        ಕೊನೆ++;
    }

    ಹಿಂತಿರುಗಿ ಕೊನೆ;
}

/* Type 3 -- System Enclosure */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_3(ಶೂನ್ಯ *ಆರಂಭ)
{
    ರಚನೆ smbios_type_3 *p = (ರಚನೆ smbios_type_3 *)ಆರಂಭ;
    ಅಕ್ಷರ *ಕೊನೆ = (ಅಕ್ಷರ *)ಆರಂಭ + ಗಾತ್ರ(ರಚನೆ smbios_type_3);
    ಗಾತ್ರ_t ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ str_ಸೂಚ್ಯಂಕ = 0;

    p->ಶೀರ್ಷಿಕೆ.type = 3;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_3);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x300;

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(3, manufacturer_str, BUILD_APPNAME);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, type, 0x01); /* other */

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(3, ಆವೃತ್ತಿ_str);
    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(3, serial_ಸಂಖ್ಯೆ_str);
    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(3, asset_ಟ್ಯಾಗ್_ಸಂಖ್ಯೆ_str);

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, boot_ಮೇಲೆ_state, 0x03); /*ಸುರಕ್ಷಿತ*/
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, power_supply_state, 0x03); /*ಸುರಕ್ಷಿತ*/
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, thermal_state, 0x03); /*ಸುರಕ್ಷಿತ*/
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, security_ಸ್ಥಿತಿ, 0x02); /* unknown */

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, oem_defined, 0);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, ಎತ್ತರ, 0);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, ಸಂಖ್ಯೆ_ದ_power_cords, 0);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(3, contained_element_ಎಣಿಕೆ, 0);

    *ಕೊನೆ = 0;
    ಕೊನೆ++;
    ಆದರೆ (!str_ಸೂಚ್ಯಂಕ) {
        *ಕೊನೆ = 0;
        ಕೊನೆ++;
    }

    ಹಿಂತಿರುಗಿ ಕೊನೆ;
}

/* Type 4 -- Processor Information */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_4(ಶೂನ್ಯ *ಆರಂಭ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu_ಸಂಖ್ಯೆ)
{
    ರಚನೆ smbios_type_4 *p = (ರಚನೆ smbios_type_4 *)ಆರಂಭ;
    ಅಕ್ಷರ *ಕೊನೆ = (ಅಕ್ಷರ *)ಆರಂಭ + ಗಾತ್ರ(ರಚನೆ smbios_type_4);
    ಗಾತ್ರ_t ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ str_ಸೂಚ್ಯಂಕ = 0;
    ಅಕ್ಷರ name[1024];

    p->ಶೀರ್ಷಿಕೆ.type = 4;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_4);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x400 + cpu_ಸಂಖ್ಯೆ;

    ಗಾತ್ರ_2 = get_ಕ್ಷೇತ್ರ(4, offsetof(ರಚನೆ smbios_type_4, socket_designation_str),
                     name);
    ಆದರೆ (ಗಾತ್ರ_2)
        snprintf(name + ಗಾತ್ರ_2 - 1, ಗಾತ್ರ(name) - ಗಾತ್ರ_2, "%2x", cpu_ಸಂಖ್ಯೆ);
    ಇಲ್ಲದಿದ್ದರೆ
        snprintf(name, ಗಾತ್ರ(name), "CPU%2x", cpu_ಸಂಖ್ಯೆ);

    ಸ್ಮರಣೆ_ನಕಲು(ಕೊನೆ, name, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1);
    ಕೊನೆ += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1;
    p->socket_designation_str = ++str_ಸೂಚ್ಯಂಕ;

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, processor_type, 0x03); /* CPU */
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, processor_family, 0x01); /* other */

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    ಆದರೆ (!get_ಕ್ಷೇತ್ರ(4, offsetof(ರಚನೆ smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(4, processor_ಆವೃತ್ತಿ_str);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, voltage, 0);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, external_clock, 0);

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, max_ವೇಗ, 2000);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, current_ವೇಗ, 2000);

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, ಸ್ಥಿತಿ_2, 0x41); /* socket populated, CPU enabled */
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *ಕೊನೆ = 0;
    ಕೊನೆ++;
    ಆದರೆ (!str_ಸೂಚ್ಯಂಕ) {
        *ಕೊನೆ = 0;
        ಕೊನೆ++;
    }

    ಹಿಂತಿರುಗಿ ಕೊನೆ;
}

/* Type 16 -- Physical Memory Array */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_16(ಶೂನ್ಯ *ಆರಂಭ, u32 ಸ್ಮರಣೆ_ಗಾತ್ರ_mb, ಪೂರ್ಣಾಂಕ nr_mem_devs)
{
    ರಚನೆ smbios_type_16 *p = (ರಚನೆ smbios_type_16*)ಆರಂಭ;

    p->ಶೀರ್ಷಿಕೆ.type = 16;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_16);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x1000;

    ಗಣ_ಕ್ಷೇತ್ರ_with_default(16, location, 0x01); /* other */
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->ಗರಿಷ್ಠ_ಸಾಮರ್ಥ್ಯ = ಸ್ಮರಣೆ_ಗಾತ್ರ_mb < 2 << 20 ?
                          ಸ್ಮರಣೆ_ಗಾತ್ರ_mb << 10 : 0x80000000;
    p->ಸ್ಮರಣೆ_error_information_handle = 0xfffe; /* none provided */
    p->ಸಂಖ್ಯೆ_ದ_ಸ್ಮರಣೆ_devices = nr_mem_devs;

    ಆರಂಭ += ಗಾತ್ರ(ರಚನೆ smbios_type_16);
    *((u16 *)ಆರಂಭ) = 0;

    ಹಿಂತಿರುಗಿ ಆರಂಭ + 2;
}

/* Type 17 -- Memory Device */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_17(ಶೂನ್ಯ *ಆರಂಭ, u32 ಗಾತ್ರ_mb, ಪೂರ್ಣಾಂಕ instance)
{
    ರಚನೆ smbios_type_17 *p = (ರಚನೆ smbios_type_17 *)ಆರಂಭ;
    ಅಕ್ಷರ *ಕೊನೆ = (ಅಕ್ಷರ *)ಆರಂಭ + ಗಾತ್ರ(ರಚನೆ smbios_type_17);
    ಗಾತ್ರ_t ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ str_ಸೂಚ್ಯಂಕ = 0;
    ಅಕ್ಷರ name[1024];

    p->ಶೀರ್ಷಿಕೆ.type = 17;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_17);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x1100 + instance;

    p->physical_ಸ್ಮರಣೆ_array_handle = 0x1000;
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(17, ಒಟ್ಟು_ಅಗಲ, 64);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(17, ದತ್ತಾಂಶ_ಅಗಲ, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->ಗಾತ್ರ_2 = ಗಾತ್ರ_mb;
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(17, form_ಗುಣಕ, 0x09); /* DIMM */
    p->device_ಗಣ = 0;

    ಗಾತ್ರ_2 = get_ಕ್ಷೇತ್ರ(17, offsetof(ರಚನೆ smbios_type_17, device_locator_str),
                     name);
    ಆದರೆ (ಗಾತ್ರ_2)
        snprintf(name + ಗಾತ್ರ_2 - 1, ಗಾತ್ರ(name) - ಗಾತ್ರ_2, "%d", instance);
    ಇಲ್ಲದಿದ್ದರೆ
        snprintf(name, ಗಾತ್ರ(name), "DIMM %d", instance);

    ಸ್ಮರಣೆ_ನಕಲು(ಕೊನೆ, name, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1);
    ಕೊನೆ += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1;
    p->device_locator_str = ++str_ಸೂಚ್ಯಂಕ;

    ತುಂಬು_str_ಕ್ಷೇತ್ರ_or_skip(17, bank_locator_str);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(17, ಸ್ಮರಣೆ_type, 0x07); /* RAM */
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(17, type_detail, 0);

    *ಕೊನೆ = 0;
    ಕೊನೆ++;
    ಆದರೆ (!str_ಸೂಚ್ಯಂಕ) {
        *ಕೊನೆ = 0;
        ಕೊನೆ++;
    }

    ಹಿಂತಿರುಗಿ ಕೊನೆ;
}

/* Type 19 -- Memory Array Mapped Address */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_19(ಶೂನ್ಯ *ಆರಂಭ, u32 ಆರಂಭ_mb, u32 ಗಾತ್ರ_mb, ಪೂರ್ಣಾಂಕ instance)
{
    ರಚನೆ smbios_type_19 *p = (ರಚನೆ smbios_type_19 *)ಆರಂಭ;

    p->ಶೀರ್ಷಿಕೆ.type = 19;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_19);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x1300 + instance;

    p->starting_ವಿಳಾಸ = ಆರಂಭ_mb << 10;
    p->ending_ವಿಳಾಸ = p->starting_ವಿಳಾಸ + (ಗಾತ್ರ_mb << 10) - 1;
    p->ಸ್ಮರಣೆ_array_handle = 0x1000;
    p->ವಿಭಜನೆ_ಅಗಲ = 1;

    ಆರಂಭ += ಗಾತ್ರ(ರಚನೆ smbios_type_19);
    *((u16 *)ಆರಂಭ) = 0;

    ಹಿಂತಿರುಗಿ ಆರಂಭ + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_20(ಶೂನ್ಯ *ಆರಂಭ, u32 ಆರಂಭ_mb, u32 ಗಾತ್ರ_mb, ಪೂರ್ಣಾಂಕ instance,
                    ಪೂರ್ಣಾಂಕ dev_handle, ಪೂರ್ಣಾಂಕ array_handle)
{
    ರಚನೆ smbios_type_20 *p = (ರಚನೆ smbios_type_20 *)ಆರಂಭ;

    p->ಶೀರ್ಷಿಕೆ.type = 20;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_20);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x1400 + instance;

    p->starting_ವಿಳಾಸ = ಆರಂಭ_mb << 10;
    p->ending_ವಿಳಾಸ = p->starting_ವಿಳಾಸ + (ಗಾತ್ರ_mb << 10) - 1;
    p->ಸ್ಮರಣೆ_device_handle = 0x1100 + dev_handle;
    p->ಸ್ಮರಣೆ_array_mapped_ವಿಳಾಸ_handle = 0x1300 + array_handle;
    p->ವಿಭಜನೆ_ಅಡ್ಡಸಾಲು_position = 1;
    p->interleave_position = 0;
    p->interleaved_ದತ್ತಾಂಶ_ಆಳ = 0;

    ಆರಂಭ += ಗಾತ್ರ(ರಚನೆ smbios_type_20);

    *((u16 *)ಆರಂಭ) = 0;
    ಹಿಂತಿರುಗಿ ಆರಂಭ+2;
}

/* Type 32 -- System Boot Information */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_32(ಶೂನ್ಯ *ಆರಂಭ)
{
    ರಚನೆ smbios_type_32 *p = (ರಚನೆ smbios_type_32 *)ಆರಂಭ;

    p->ಶೀರ್ಷಿಕೆ.type = 32;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_32);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x2000;
    memset(p->reserved, 0, 6);
    ಗಣ_ಕ್ಷೇತ್ರ_with_default(32, boot_ಸ್ಥಿತಿ, 0); /* no errors detected */

    ಆರಂಭ += ಗಾತ್ರ(ರಚನೆ smbios_type_32);
    *((u16 *)ಆರಂಭ) = 0;

    ಹಿಂತಿರುಗಿ ಆರಂಭ+2;
}

/* Type 127 -- End of Table */
ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_init_type_127(ಶೂನ್ಯ *ಆರಂಭ)
{
    ರಚನೆ smbios_type_127 *p = (ರಚನೆ smbios_type_127 *)ಆರಂಭ;

    p->ಶೀರ್ಷಿಕೆ.type = 127;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_127);
    p->ಶೀರ್ಷಿಕೆ.handle = 0x7f00;

    ಆರಂಭ += ಗಾತ್ರ(ರಚನೆ smbios_type_127);
    *((u16 *)ಆರಂಭ) = 0;

    ಹಿಂತಿರುಗಿ ಆರಂಭ + 2;
}

#ವ್ಯಾಖ್ಯಾನಿಸು TEMPSMBIOSSIZE (32 * 1024)

ಶೂನ್ಯ
smbios_legacy_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_SMBIOS)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init SMBIOS tables\n");

    ಅಕ್ಷರ *ಆರಂಭ = malloc_tmphigh(TEMPSMBIOSSIZE);
    ಆದರೆ (! ಆರಂಭ) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(ಆರಂಭ, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_ಗಾತ್ರ = 0;
    ಅಕ್ಷರ *q, *p = ಆರಂಭ;
    ಅಕ್ಷರ *ಕೊನೆ = ಆರಂಭ + TEMPSMBIOSSIZE - ಗಾತ್ರ(ರಚನೆ smbios_type_127);

#ವ್ಯಾಖ್ಯಾನಿಸು ಕೂಡಿಸು_struct(type, ವಾದಗಳು...)                                       \
    ಮಾಡು {                                                                \
        ಆದರೆ (!get_external(type, &p, &nr_structs, &max_struct_ಗಾತ್ರ, ಕೊನೆ)) { \
            q = smbios_init_type_##type(ವಾದಗಳು);                          \
            nr_structs++;                                               \
            ಆದರೆ ((q - p) > max_struct_ಗಾತ್ರ)                              \
                max_struct_ಗಾತ್ರ = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } ಆಗ (0)

    ಕೂಡಿಸು_struct(0, p);
    ಕೂಡಿಸು_struct(1, p);
    ಕೂಡಿಸು_struct(3, p);

    ಪೂರ್ಣಾಂಕ cpu_ಅಂಕೆ;
    ಗಾಗಿ (cpu_ಅಂಕೆ = 1; cpu_ಅಂಕೆ <= Max_ಎಣಿಕೆ_CPUs; cpu_ಅಂಕೆ++)
        ಕೂಡಿಸು_struct(4, p, cpu_ಅಂಕೆ);

    ಪೂರ್ಣಾಂಕ ram_mb = (Ram_ಗಾತ್ರ + Ram_ಗಾತ್ರ_Over4_G) >> 20;
    ಪೂರ್ಣಾಂಕ nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    ಕೂಡಿಸು_struct(16, p, ram_mb, nr_mem_devs);

    ಪೂರ್ಣಾಂಕ i, j;
    ಗಾಗಿ (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        ಕೂಡಿಸು_struct(17, p, dev_mb, i);
    }

    ಕೂಡಿಸು_struct(19, p, 0, Ram_ಗಾತ್ರ >> 20, 0);
    ಆದರೆ (Ram_ಗಾತ್ರ_Over4_G)
        ಕೂಡಿಸು_struct(19, p, 4096, Ram_ಗಾತ್ರ_Over4_G >> 20, 1);

    ಕೂಡಿಸು_struct(20, p, 0, Ram_ಗಾತ್ರ >> 20, 0, 0, 0);
    ಆದರೆ (Ram_ಗಾತ್ರ_Over4_G) {
        u32 ಆರಂಭ_mb = 4096;
        ಗಾಗಿ (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            ಆದರೆ (i == 0)
                dev_mb -= Ram_ಗಾತ್ರ >> 20;

            ಕೂಡಿಸು_struct(20, p, ಆರಂಭ_mb, dev_mb, j, i, 1);
            ಆರಂಭ_mb += dev_mb;
        }
    }

    ಕೂಡಿಸು_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    ಗಾಗಿ (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_ಗಾತ್ರ, ಕೊನೆ);
    ಕೂಡಿಸು_struct(127, p);

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಕೂಡಿಸು_struct

    smbios_21_ನಮೂದು_ಬಿಂದು_ಸಿದ್ಧತೆ(max_struct_ಗಾತ್ರ, p - ಆರಂಭ, ಆರಂಭ, nr_structs);
    ಬಿಡುಗಡೆ(ಆರಂಭ);
}
