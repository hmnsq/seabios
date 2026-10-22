// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // BUILD_MAX_E820
#ضمن "e820map.h" // struct e820entry
#ضمن "output.h" // dprintf
#ضمن "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
بنية e820entry e820_list[BUILD_MAX_E820] VARFSEG;
صحيح e820_count VARFSEG;

// Remove an entry from the e820_list.
ساكن فراغ
remove_e820(صحيح i)
{
    e820_count--;
    memmove(&e820_list[i], &e820_list[i+1]
            , حجم(e820_list[0]) * (e820_count - i));
}

// Insert an entry in the e820_list at the given position.
ساكن فراغ
insert_e820(صحيح i, u64 start, u64 size, u32 type)
{
    אם (e820_count >= BUILD_MAX_E820) {
        warn_noalloc();
        החזר;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , حجم(e820_list[0]) * (e820_count - i));
    e820_count++;
    بنية e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

ساكن ثابت حرف *
e820_type_name(u32 type)
{
    بدل (type) {
    حالة E820_RAM:      החזר "RAM";
    حالة E820_RESERVED: החזר "RESERVED";
    حالة E820_ACPI:     החזר "ACPI";
    حالة E820_NVS:      החזר "NVS";
    حالة E820_UNUSABLE: החזר "UNUSABLE";
    افتراضي:            החזר "UNKNOWN";
    }
}

// Show the current e820_list.
ساكن فراغ
dump_map(فراغ)
{
    dprintf(1, "e820 map has %d items:\n", e820_count);
    صحيح i;
    עבור (i=0; i<e820_count; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
فراغ
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    אם (! size)
        // Huh?  Nothing to do.
        החזר;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    صحيح i;
    עבור (i=0; i<e820_count; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        אם (start > e_end)
            استمر;
        // Found position - check if an existing item needs to be split.
        אם (start > e->start) {
            אם (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } אחרת {
                // Split existing item.
                e->size = start - e->start;
                i++;
                אם (e_end > end)
                    insert_e820(i, end, e_end - end, e->type);
            }
        }
        اكسر;
    }
    // Remove/adjust existing items that are overlapping.
    כלעוד (i<e820_count) {
        بنية e820entry *e = &e820_list[i];
        אם (end < e->start)
            // No overlap - done.
            اكسر;
        u64 e_end = e->start + e->size;
        אם (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            استمر;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        אם (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        اكسر;
    }
    // Insert new item.
    אם (type != E820_HOLE)
        insert_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
فراغ
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
فراغ
e820_prepboot(فراغ)
{
    dump_map();
}

صحيح
e820_is_used(u64 start, u64 size)
{
    صحيح i;
    עבור (i=0; i<e820_count; i++) {
        بنية e820entry *e = &e820_list[i];
        אם (start + size <= e->start)
            استمر;
        אם (start >= e->start + e->size)
            استمر;
        החזר 1;
    }
    החזר 0;
}
