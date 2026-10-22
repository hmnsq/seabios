// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // BUILD_MAX_E820
#ಅನ್ತರ್ಭಾವ "e820map.h" // struct e820entry
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
ರಚನೆ e820entry e820_list[BUILD_MAX_E820] VARFSEG;
ಪೂರ್ಣಾಂಕ e820_ಎಣಿಕೆ VARFSEG;

// Remove an entry from the e820_list.
ಸ್ಥಿರ ಶೂನ್ಯ
remove_e820(ಪೂರ್ಣಾಂಕ i)
{
    e820_ಎಣಿಕೆ--;
    memmove(&e820_list[i], &e820_list[i+1]
            , ಗಾತ್ರ(e820_list[0]) * (e820_ಎಣಿಕೆ - i));
}

// Insert an entry in the e820_list at the given position.
ಸ್ಥಿರ ಶೂನ್ಯ
ಸೇರಿಸು_e820(ಪೂರ್ಣಾಂಕ i, u64 ಆರಂಭ, u64 ಗಾತ್ರ_2, u32 type)
{
    ಆದರೆ (e820_ಎಣಿಕೆ >= BUILD_MAX_E820) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , ಗಾತ್ರ(e820_list[0]) * (e820_ಎಣಿಕೆ - i));
    e820_ಎಣಿಕೆ++;
    ರಚನೆ e820entry *e = &e820_list[i];
    e->ಆರಂಭ = ಆರಂಭ;
    e->ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
    e->type = type;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
e820_type_name(u32 type)
{
    ಆಯ್ಕೆ (type) {
    ಸ್ಥಿತಿ E820_RAM:      ಹಿಂತಿರುಗಿ "RAM";
    ಸ್ಥಿತಿ E820_RESERVED: ಹಿಂತಿರುಗಿ "RESERVED";
    ಸ್ಥಿತಿ E820_ACPI:     ಹಿಂತಿರುಗಿ "ACPI";
    ಸ್ಥಿತಿ E820_NVS:      ಹಿಂತಿರುಗಿ "NVS";
    ಸ್ಥಿತಿ E820_UNUSABLE: ಹಿಂತಿರುಗಿ "UNUSABLE";
    ಪೂರ್ವನಿಯೋಜಿತ:            ಹಿಂತಿರುಗಿ "UNKNOWN";
    }
}

// Show the current e820_list.
ಸ್ಥಿರ ಶೂನ್ಯ
dump_map(ಶೂನ್ಯ)
{
    dprintf(1, "e820 map has %d items:\n", e820_ಎಣಿಕೆ);
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<e820_ಎಣಿಕೆ; i++) {
        ರಚನೆ e820entry *e = &e820_list[i];
        u64 e_ಕೊನೆ = e->ಆರಂಭ + e->ಗಾತ್ರ_2;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->ಆರಂಭ, e_ಕೊನೆ, e->type, e820_type_name(e->type));
    }
}

#ವ್ಯಾಖ್ಯಾನಿಸು E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
ಶೂನ್ಯ
e820_ಕೂಡಿಸು(u64 ಆರಂಭ, u64 ಗಾತ್ರ_2, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", ಆರಂಭ, ಗಾತ್ರ_2, type);

    ಆದರೆ (! ಗಾತ್ರ_2)
        // Huh?  Nothing to do.
        ಹಿಂತಿರುಗಿ;

    // Find position of new item (splitting existing item if needed).
    u64 ಕೊನೆ = ಆರಂಭ + ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<e820_ಎಣಿಕೆ; i++) {
        ರಚನೆ e820entry *e = &e820_list[i];
        u64 e_ಕೊನೆ = e->ಆರಂಭ + e->ಗಾತ್ರ_2;
        ಆದರೆ (ಆರಂಭ > e_ಕೊನೆ)
            ಮುಂದುವರಿ;
        // Found position - check if an existing item needs to be split.
        ಆದರೆ (ಆರಂಭ > e->ಆರಂಭ) {
            ಆದರೆ (type == e->type) {
                // Same type - merge them.
                ಗಾತ್ರ_2 += ಆರಂಭ - e->ಆರಂಭ;
                ಆರಂಭ = e->ಆರಂಭ;
            } ಇಲ್ಲದಿದ್ದರೆ {
                // Split existing item.
                e->ಗಾತ್ರ_2 = ಆರಂಭ - e->ಆರಂಭ;
                i++;
                ಆದರೆ (e_ಕೊನೆ > ಕೊನೆ)
                    ಸೇರಿಸು_e820(i, ಕೊನೆ, e_ಕೊನೆ - ಕೊನೆ, e->type);
            }
        }
        ನಿಲ್ಲಿಸು;
    }
    // Remove/adjust existing items that are overlapping.
    ಆಗ (i<e820_ಎಣಿಕೆ) {
        ರಚನೆ e820entry *e = &e820_list[i];
        ಆದರೆ (ಕೊನೆ < e->ಆರಂಭ)
            // No overlap - done.
            ನಿಲ್ಲಿಸು;
        u64 e_ಕೊನೆ = e->ಆರಂಭ + e->ಗಾತ್ರ_2;
        ಆದರೆ (ಕೊನೆ >= e_ಕೊನೆ) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            ಮುಂದುವರಿ;
        }
        // Not completely overlapped - adjust its start.
        e->ಆರಂಭ = ಕೊನೆ;
        e->ಗಾತ್ರ_2 = e_ಕೊನೆ - ಕೊನೆ;
        ಆದರೆ (type == e->type) {
            // Same type - merge them.
            ಗಾತ್ರ_2 += e->ಗಾತ್ರ_2;
            remove_e820(i);
        }
        ನಿಲ್ಲಿಸು;
    }
    // Insert new item.
    ಆದರೆ (type != E820_HOLE)
        ಸೇರಿಸು_e820(i, ಆರಂಭ, ಗಾತ್ರ_2, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
ಶೂನ್ಯ
e820_remove(u64 ಆರಂಭ, u64 ಗಾತ್ರ_2)
{
    e820_ಕೂಡಿಸು(ಆರಂಭ, ಗಾತ್ರ_2, E820_HOLE);
}

// Report on final memory locations.
ಶೂನ್ಯ
e820_prepboot(ಶೂನ್ಯ)
{
    dump_map();
}

ಪೂರ್ಣಾಂಕ
e820_is_ಬಳಸಿದ(u64 ಆರಂಭ, u64 ಗಾತ್ರ_2)
{
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<e820_ಎಣಿಕೆ; i++) {
        ರಚನೆ e820entry *e = &e820_list[i];
        ಆದರೆ (ಆರಂಭ + ಗಾತ್ರ_2 <= e->ಆರಂಭ)
            ಮುಂದುವರಿ;
        ಆದರೆ (ಆರಂಭ >= e->ಆರಂಭ + e->ಗಾತ್ರ_2)
            ಮುಂದುವರಿ;
        ಹಿಂತಿರುಗಿ 1;
    }
    ಹಿಂತಿರುಗಿ 0;
}
