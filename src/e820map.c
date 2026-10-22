// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // BUILD_MAX_E820
#அந்தர்பாவ "e820map.h" // struct e820entry
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
கட்டமைப்பு e820entry e820_list[BUILD_MAX_E820] VARFSEG;
முழு e820_மொத்தம் VARFSEG;

// Remove an entry from the e820_list.
நிலையான வெறுமை
remove_e820(முழு i)
{
    e820_மொத்தம்--;
    memmove(&e820_list[i], &e820_list[i+1]
            , அளவு(e820_list[0]) * (e820_மொத்தம் - i));
}

// Insert an entry in the e820_list at the given position.
நிலையான வெறுமை
செருகு_e820(முழு i, u64 start, u64 size, u32 type)
{
    எனில் (e820_மொத்தம் >= BUILD_MAX_E820) {
        warn_noalloc();
        திரும்பு;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , அளவு(e820_list[0]) * (e820_மொத்தம் - i));
    e820_மொத்தம்++;
    கட்டமைப்பு e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

நிலையான மாறிலி எழுத்து *
e820_type_name(u32 type)
{
    தேர்வு (type) {
    நிலை E820_RAM:      திரும்பு "RAM";
    நிலை E820_RESERVED: திரும்பு "RESERVED";
    நிலை E820_ACPI:     திரும்பு "ACPI";
    நிலை E820_NVS:      திரும்பு "NVS";
    நிலை E820_UNUSABLE: திரும்பு "UNUSABLE";
    இயல்பு:            திரும்பு "UNKNOWN";
    }
}

// Show the current e820_list.
நிலையான வெறுமை
dump_map(வெறுமை)
{
    dprintf(1, "e820 map has %d items:\n", e820_மொத்தம்);
    முழு i;
    க்கு (i=0; i<e820_மொத்தம்; i++) {
        கட்டமைப்பு e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
வெறுமை
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    எனில் (! size)
        // Huh?  Nothing to do.
        திரும்பு;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    முழு i;
    க்கு (i=0; i<e820_மொத்தம்; i++) {
        கட்டமைப்பு e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        எனில் (start > e_end)
            தொடர்;
        // Found position - check if an existing item needs to be split.
        எனில் (start > e->start) {
            எனில் (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } இல்லையெனில் {
                // Split existing item.
                e->size = start - e->start;
                i++;
                எனில் (e_end > end)
                    செருகு_e820(i, end, e_end - end, e->type);
            }
        }
        நிறுத்து;
    }
    // Remove/adjust existing items that are overlapping.
    போது (i<e820_மொத்தம்) {
        கட்டமைப்பு e820entry *e = &e820_list[i];
        எனில் (end < e->start)
            // No overlap - done.
            நிறுத்து;
        u64 e_end = e->start + e->size;
        எனில் (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            தொடர்;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        எனில் (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        நிறுத்து;
    }
    // Insert new item.
    எனில் (type != E820_HOLE)
        செருகு_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
வெறுமை
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
வெறுமை
e820_prepboot(வெறுமை)
{
    dump_map();
}

முழு
e820_is_used(u64 start, u64 size)
{
    முழு i;
    க்கு (i=0; i<e820_மொத்தம்; i++) {
        கட்டமைப்பு e820entry *e = &e820_list[i];
        எனில் (start + size <= e->start)
            தொடர்;
        எனில் (start >= e->start + e->size)
            தொடர்;
        திரும்பு 1;
    }
    திரும்பு 0;
}
