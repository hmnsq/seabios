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
صحيح e820_کيراءن VARFSEG;

// Remove an entry from the e820_list.
ساكن فراغ
remove_e820(صحيح i)
{
    e820_کيراءن--;
    memmove(&e820_list[i], &e820_list[i+1]
            , حجم(e820_list[0]) * (e820_کيراءن - i));
}

// Insert an entry in the e820_list at the given position.
ساكن فراغ
سيسيڤ_e820(صحيح i, u64 start, u64 size, u32 type)
{
    إذا (e820_کيراءن >= BUILD_MAX_E820) {
        warn_noalloc();
        أرجع;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , حجم(e820_list[0]) * (e820_کيراءن - i));
    e820_کيراءن++;
    بنية e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

ساكن ثابت حرف *
e820_type_name(u32 type)
{
    بدل (type) {
    حالة E820_RAM:      أرجع "RAM";
    حالة E820_RESERVED: أرجع "RESERVED";
    حالة E820_ACPI:     أرجع "ACPI";
    حالة E820_NVS:      أرجع "NVS";
    حالة E820_UNUSABLE: أرجع "UNUSABLE";
    افتراضي:            أرجع "UNKNOWN";
    }
}

// Show the current e820_list.
ساكن فراغ
dump_map(فراغ)
{
    dprintf(1, "e820 map has %d items:\n", e820_کيراءن);
    صحيح i;
    لكل (i=0; i<e820_کيراءن; i++) {
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

    إذا (! size)
        // Huh?  Nothing to do.
        أرجع;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    صحيح i;
    لكل (i=0; i<e820_کيراءن; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        إذا (start > e_end)
            استمر;
        // Found position - check if an existing item needs to be split.
        إذا (start > e->start) {
            إذا (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } وإلا {
                // Split existing item.
                e->size = start - e->start;
                i++;
                إذا (e_end > end)
                    سيسيڤ_e820(i, end, e_end - end, e->type);
            }
        }
        اكسر;
    }
    // Remove/adjust existing items that are overlapping.
    طالما (i<e820_کيراءن) {
        بنية e820entry *e = &e820_list[i];
        إذا (end < e->start)
            // No overlap - done.
            اكسر;
        u64 e_end = e->start + e->size;
        إذا (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            استمر;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        إذا (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        اكسر;
    }
    // Insert new item.
    إذا (type != E820_HOLE)
        سيسيڤ_e820(i, start, size, type);
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
    لكل (i=0; i<e820_کيراءن; i++) {
        بنية e820entry *e = &e820_list[i];
        إذا (start + size <= e->start)
            استمر;
        إذا (start >= e->start + e->size)
            استمر;
        أرجع 1;
    }
    أرجع 0;
}
