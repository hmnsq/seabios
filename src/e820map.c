// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // BUILD_MAX_E820
#شامل "e820map.h" // struct e820entry
#شامل "output.h" // dprintf
#شامل "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
ساختار e820entry e820_list[BUILD_MAX_E820] VARFSEG;
صحیح e820_شمار VARFSEG;

// Remove an entry from the e820_list.
ایستا تهی
remove_e820(صحیح i)
{
    e820_شمار--;
    memmove(&e820_list[i], &e820_list[i+1]
            , اندازه(e820_list[0]) * (e820_شمار - i));
}

// Insert an entry in the e820_list at the given position.
ایستا تهی
درج_e820(صحیح i, u64 start, u64 size, u32 type)
{
    اگر (e820_شمار >= BUILD_MAX_E820) {
        warn_noalloc();
        بازگشت;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , اندازه(e820_list[0]) * (e820_شمار - i));
    e820_شمار++;
    ساختار e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

ایستا ثابت نویسه *
e820_type_name(u32 type)
{
    انتخاب (type) {
    حالت E820_RAM:      بازگشت "RAM";
    حالت E820_RESERVED: بازگشت "RESERVED";
    حالت E820_ACPI:     بازگشت "ACPI";
    حالت E820_NVS:      بازگشت "NVS";
    حالت E820_UNUSABLE: بازگشت "UNUSABLE";
    پیشفرض:            بازگشت "UNKNOWN";
    }
}

// Show the current e820_list.
ایستا تهی
dump_map(تهی)
{
    dprintf(1, "e820 map has %d items:\n", e820_شمار);
    صحیح i;
    برای (i=0; i<e820_شمار; i++) {
        ساختار e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
تهی
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    اگر (! size)
        // Huh?  Nothing to do.
        بازگشت;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    صحیح i;
    برای (i=0; i<e820_شمار; i++) {
        ساختار e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        اگر (start > e_end)
            ادامه;
        // Found position - check if an existing item needs to be split.
        اگر (start > e->start) {
            اگر (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } وگرنه {
                // Split existing item.
                e->size = start - e->start;
                i++;
                اگر (e_end > end)
                    درج_e820(i, end, e_end - end, e->type);
            }
        }
        بشکن;
    }
    // Remove/adjust existing items that are overlapping.
    وقتی (i<e820_شمار) {
        ساختار e820entry *e = &e820_list[i];
        اگر (end < e->start)
            // No overlap - done.
            بشکن;
        u64 e_end = e->start + e->size;
        اگر (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            ادامه;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        اگر (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        بشکن;
    }
    // Insert new item.
    اگر (type != E820_HOLE)
        درج_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
تهی
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
تهی
e820_prepboot(تهی)
{
    dump_map();
}

صحیح
e820_is_used(u64 start, u64 size)
{
    صحیح i;
    برای (i=0; i<e820_شمار; i++) {
        ساختار e820entry *e = &e820_list[i];
        اگر (start + size <= e->start)
            ادامه;
        اگر (start >= e->start + e->size)
            ادامه;
        بازگشت 1;
    }
    بازگشت 0;
}
