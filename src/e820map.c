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
ಸಂರಚನಾ e820entry e820_list[BUILD_MAX_E820] VARFSEG;
ಪೂರ್ಣಾಙ್ಕ e820_ಎಣಿಕೆ VARFSEG;

// Remove an entry from the e820_list.
ಸ್ಥಿರ ಶೂನ್ಯ
remove_e820(ಪೂರ್ಣಾಙ್ಕ i)
{
    e820_ಎಣಿಕೆ--;
    memmove(&e820_list[i], &e820_list[i+1]
            , ಪರಿಮಾಣ(e820_list[0]) * (e820_ಎಣಿಕೆ - i));
}

// Insert an entry in the e820_list at the given position.
ಸ್ಥಿರ ಶೂನ್ಯ
ಸೇರಿಸು_e820(ಪೂರ್ಣಾಙ್ಕ i, u64 start, u64 size, u32 type)
{
    ಯದಿ (e820_ಎಣಿಕೆ >= BUILD_MAX_E820) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , ಪರಿಮಾಣ(e820_list[0]) * (e820_ಎಣಿಕೆ - i));
    e820_ಎಣಿಕೆ++;
    ಸಂರಚನಾ e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
e820_type_name(u32 type)
{
    ವಿಕಲ್ಪ (type) {
    ಪಕ್ಷ E820_RAM:      ಪ್ರತ್ಯಾಗಮ "RAM";
    ಪಕ್ಷ E820_RESERVED: ಪ್ರತ್ಯಾಗಮ "RESERVED";
    ಪಕ್ಷ E820_ACPI:     ಪ್ರತ್ಯಾಗಮ "ACPI";
    ಪಕ್ಷ E820_NVS:      ಪ್ರತ್ಯಾಗಮ "NVS";
    ಪಕ್ಷ E820_UNUSABLE: ಪ್ರತ್ಯಾಗಮ "UNUSABLE";
    ಸ್ವಭಾವ:            ಪ್ರತ್ಯಾಗಮ "UNKNOWN";
    }
}

// Show the current e820_list.
ಸ್ಥಿರ ಶೂನ್ಯ
dump_map(ಶೂನ್ಯ)
{
    dprintf(1, "e820 map has %d items:\n", e820_ಎಣಿಕೆ);
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<e820_ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
ಶೂನ್ಯ
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    ಯದಿ (! size)
        // Huh?  Nothing to do.
        ಪ್ರತ್ಯಾಗಮ;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<e820_ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        ಯದಿ (start > e_end)
            ಅನುವರ್ತ;
        // Found position - check if an existing item needs to be split.
        ಯದಿ (start > e->start) {
            ಯದಿ (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } ಅನ್ಯಥಾ {
                // Split existing item.
                e->size = start - e->start;
                i++;
                ಯದಿ (e_end > end)
                    ಸೇರಿಸು_e820(i, end, e_end - end, e->type);
            }
        }
        ಭಙ್ಗ;
    }
    // Remove/adjust existing items that are overlapping.
    ಯಾವತ್ (i<e820_ಎಣಿಕೆ) {
        ಸಂರಚನಾ e820entry *e = &e820_list[i];
        ಯದಿ (end < e->start)
            // No overlap - done.
            ಭಙ್ಗ;
        u64 e_end = e->start + e->size;
        ಯದಿ (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            ಅನುವರ್ತ;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        ಯದಿ (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        ಭಙ್ಗ;
    }
    // Insert new item.
    ಯದಿ (type != E820_HOLE)
        ಸೇರಿಸು_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
ಶೂನ್ಯ
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
ಶೂನ್ಯ
e820_prepboot(ಶೂನ್ಯ)
{
    dump_map();
}

ಪೂರ್ಣಾಙ್ಕ
e820_is_used(u64 start, u64 size)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<e820_ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ e820entry *e = &e820_list[i];
        ಯದಿ (start + size <= e->start)
            ಅನುವರ್ತ;
        ಯದಿ (start >= e->start + e->size)
            ಅನುವರ್ತ;
        ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}
