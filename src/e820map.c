// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // BUILD_MAX_E820
#include "e820map.h" // struct e820entry
#include "output.h" // dprintf
#include "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
sa_nracanaa e820entry e820_list[BUILD_MAX_E820] VARFSEG;
puur_naa_nka e820_count VARFSEG;

// Remove an entry from the e820_list.
static void
remove_e820(puur_naa_nka i)
{
    e820_count--;
    memmove(&e820_list[i], &e820_list[i+1]
            , parimaa_na(e820_list[0]) * (e820_count - i));
}

// Insert an entry in the e820_list at the given position.
static void
insert_e820(puur_naa_nka i, u64 start, u64 size, u32 type)
{
    ᬬᬾᬦ᭄ (e820_count >= BUILD_MAX_E820) {
        warn_noalloc();
        ᬯᬮᬶ;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , parimaa_na(e820_list[0]) * (e820_count - i));
    e820_count++;
    sa_nracanaa e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

static const var_na *
e820_type_name(u32 type)
{
    switch (type) {
    pak_sa E820_RAM:      ᬯᬮᬶ "RAM";
    pak_sa E820_RESERVED: ᬯᬮᬶ "RESERVED";
    pak_sa E820_ACPI:     ᬯᬮᬶ "ACPI";
    pak_sa E820_NVS:      ᬯᬮᬶ "NVS";
    pak_sa E820_UNUSABLE: ᬯᬮᬶ "UNUSABLE";
    default:            ᬯᬮᬶ "UNKNOWN";
    }
}

// Show the current e820_list.
static void
dump_map(void)
{
    dprintf(1, "e820 map has %d items:\n", e820_count);
    puur_naa_nka i;
    for (i=0; i<e820_count; i++) {
        sa_nracanaa e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
void
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    ᬬᬾᬦ᭄ (! size)
        // Huh?  Nothing to do.
        ᬯᬮᬶ;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    puur_naa_nka i;
    for (i=0; i<e820_count; i++) {
        sa_nracanaa e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        ᬬᬾᬦ᭄ (start > e_end)
            continue;
        // Found position - check if an existing item needs to be split.
        ᬬᬾᬦ᭄ (start > e->start) {
            ᬬᬾᬦ᭄ (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } ᬮᬶᬬ {
                // Split existing item.
                e->size = start - e->start;
                i++;
                ᬬᬾᬦ᭄ (e_end > end)
                    insert_e820(i, end, e_end - end, e->type);
            }
        }
        bha_nga;
    }
    // Remove/adjust existing items that are overlapping.
    ᬲᭂᬤᭂᬮ (i<e820_count) {
        sa_nracanaa e820entry *e = &e820_list[i];
        ᬬᬾᬦ᭄ (end < e->start)
            // No overlap - done.
            bha_nga;
        u64 e_end = e->start + e->size;
        ᬬᬾᬦ᭄ (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            continue;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        ᬬᬾᬦ᭄ (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        bha_nga;
    }
    // Insert new item.
    ᬬᬾᬦ᭄ (type != E820_HOLE)
        insert_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
void
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
void
e820_prepboot(void)
{
    dump_map();
}

puur_naa_nka
e820_is_used(u64 start, u64 size)
{
    puur_naa_nka i;
    for (i=0; i<e820_count; i++) {
        sa_nracanaa e820entry *e = &e820_list[i];
        ᬬᬾᬦ᭄ (start + size <= e->start)
            continue;
        ᬬᬾᬦ᭄ (start >= e->start + e->size)
            continue;
        ᬯᬮᬶ 1;
    }
    ᬯᬮᬶ 0;
}
