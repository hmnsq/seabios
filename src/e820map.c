// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // BUILD_MAX_E820
#অন্তর্ভাৱ "e820map.h" // struct e820entry
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
সংরচনা e820entry e820_list[BUILD_MAX_E820] VARFSEG;
পূর্ণাঙ্ক e820_গুনতি VARFSEG;

// Remove an entry from the e820_list.
স্থির শূন্য
remove_e820(পূর্ণাঙ্ক i)
{
    e820_গুনতি--;
    memmove(&e820_list[i], &e820_list[i+1]
            , পরিমাণ(e820_list[0]) * (e820_গুনতি - i));
}

// Insert an entry in the e820_list at the given position.
স্থির শূন্য
ঢোকাও_e820(পূর্ণাঙ্ক i, u64 start, u64 size, u32 type)
{
    যদি (e820_গুনতি >= BUILD_MAX_E820) {
        warn_noalloc();
        ঘূৰাই;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , পরিমাণ(e820_list[0]) * (e820_গুনতি - i));
    e820_গুনতি++;
    সংরচনা e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

স্থির ধ্রুৱ ৱর্ণ *
e820_type_name(u32 type)
{
    ৱিকল্প (type) {
    পক্ষ E820_RAM:      ঘূৰাই "RAM";
    পক্ষ E820_RESERVED: ঘূৰাই "RESERVED";
    পক্ষ E820_ACPI:     ঘূৰাই "ACPI";
    পক্ষ E820_NVS:      ঘূৰাই "NVS";
    পক্ষ E820_UNUSABLE: ঘূৰাই "UNUSABLE";
    স্ৱভাৱ:            ঘূৰাই "UNKNOWN";
    }
}

// Show the current e820_list.
স্থির শূন্য
dump_map(শূন্য)
{
    dprintf(1, "e820 map has %d items:\n", e820_গুনতি);
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<e820_গুনতি; i++) {
        সংরচনা e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
শূন্য
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    যদি (! size)
        // Huh?  Nothing to do.
        ঘূৰাই;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<e820_গুনতি; i++) {
        সংরচনা e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        যদি (start > e_end)
            অনুৱর্ত;
        // Found position - check if an existing item needs to be split.
        যদি (start > e->start) {
            যদি (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } অন্যথা {
                // Split existing item.
                e->size = start - e->start;
                i++;
                যদি (e_end > end)
                    ঢোকাও_e820(i, end, e_end - end, e->type);
            }
        }
        ভঙ্গ;
    }
    // Remove/adjust existing items that are overlapping.
    যেতিয়ালৈ (i<e820_গুনতি) {
        সংরচনা e820entry *e = &e820_list[i];
        যদি (end < e->start)
            // No overlap - done.
            ভঙ্গ;
        u64 e_end = e->start + e->size;
        যদি (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            অনুৱর্ত;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        যদি (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        ভঙ্গ;
    }
    // Insert new item.
    যদি (type != E820_HOLE)
        ঢোকাও_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
শূন্য
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
শূন্য
e820_prepboot(শূন্য)
{
    dump_map();
}

পূর্ণাঙ্ক
e820_is_used(u64 start, u64 size)
{
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<e820_গুনতি; i++) {
        সংরচনা e820entry *e = &e820_list[i];
        যদি (start + size <= e->start)
            অনুৱর্ত;
        যদি (start >= e->start + e->size)
            অনুৱর্ত;
        ঘূৰাই 1;
    }
    ঘূৰাই 0;
}
