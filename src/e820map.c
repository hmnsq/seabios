// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // BUILD_MAX_E820
#অন্তর্ভাব "e820map.h" // struct e820entry
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
কাঠামো e820entry e820_list[BUILD_MAX_E820] VARFSEG;
পূর্ণসংখ্যা e820_গুনতি VARFSEG;

// Remove an entry from the e820_list.
স্থির শূন্য
remove_e820(পূর্ণসংখ্যা i)
{
    e820_গুনতি--;
    memmove(&e820_list[i], &e820_list[i+1]
            , আকার(e820_list[0]) * (e820_গুনতি - i));
}

// Insert an entry in the e820_list at the given position.
স্থির শূন্য
ঢোকাও_e820(পূর্ণসংখ্যা i, u64 start, u64 আকার_2, u32 type)
{
    যদি (e820_গুনতি >= BUILD_MAX_E820) {
        warn_noalloc();
        ফেরত;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , আকার(e820_list[0]) * (e820_গুনতি - i));
    e820_গুনতি++;
    কাঠামো e820entry *e = &e820_list[i];
    e->start = start;
    e->আকার_2 = আকার_2;
    e->type = type;
}

স্থির ধ্রুবক অক্ষর *
e820_type_name(u32 type)
{
    নির্বাচন (type) {
    ক্ষেত্র E820_RAM:      ফেরত "RAM";
    ক্ষেত্র E820_RESERVED: ফেরত "RESERVED";
    ক্ষেত্র E820_ACPI:     ফেরত "ACPI";
    ক্ষেত্র E820_NVS:      ফেরত "NVS";
    ক্ষেত্র E820_UNUSABLE: ফেরত "UNUSABLE";
    পূর্বনির্ধারিত:            ফেরত "UNKNOWN";
    }
}

// Show the current e820_list.
স্থির শূন্য
dump_map(শূন্য)
{
    dprintf(1, "e820 map has %d items:\n", e820_গুনতি);
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<e820_গুনতি; i++) {
        কাঠামো e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->আকার_2;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
শূন্য
e820_add(u64 start, u64 আকার_2, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, আকার_2, type);

    যদি (! আকার_2)
        // Huh?  Nothing to do.
        ফেরত;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + আকার_2;
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<e820_গুনতি; i++) {
        কাঠামো e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->আকার_2;
        যদি (start > e_end)
            চালু;
        // Found position - check if an existing item needs to be split.
        যদি (start > e->start) {
            যদি (type == e->type) {
                // Same type - merge them.
                আকার_2 += start - e->start;
                start = e->start;
            } অন্যথা {
                // Split existing item.
                e->আকার_2 = start - e->start;
                i++;
                যদি (e_end > end)
                    ঢোকাও_e820(i, end, e_end - end, e->type);
            }
        }
        বিরতি;
    }
    // Remove/adjust existing items that are overlapping.
    যখন (i<e820_গুনতি) {
        কাঠামো e820entry *e = &e820_list[i];
        যদি (end < e->start)
            // No overlap - done.
            বিরতি;
        u64 e_end = e->start + e->আকার_2;
        যদি (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            চালু;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->আকার_2 = e_end - end;
        যদি (type == e->type) {
            // Same type - merge them.
            আকার_2 += e->আকার_2;
            remove_e820(i);
        }
        বিরতি;
    }
    // Insert new item.
    যদি (type != E820_HOLE)
        ঢোকাও_e820(i, start, আকার_2, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
শূন্য
e820_remove(u64 start, u64 আকার_2)
{
    e820_add(start, আকার_2, E820_HOLE);
}

// Report on final memory locations.
শূন্য
e820_prepboot(শূন্য)
{
    dump_map();
}

পূর্ণসংখ্যা
e820_is_used(u64 start, u64 আকার_2)
{
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<e820_গুনতি; i++) {
        কাঠামো e820entry *e = &e820_list[i];
        যদি (start + আকার_2 <= e->start)
            চালু;
        যদি (start >= e->start + e->আকার_2)
            চালু;
        ফেরত 1;
    }
    ফেরত 0;
}
