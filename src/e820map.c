// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // BUILD_MAX_E820
#અન્તર્ભાવ "e820map.h" // struct e820entry
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
સંરચના e820entry e820_list[BUILD_MAX_E820] VARFSEG;
પૂર્ણાંક e820_ગણતરી VARFSEG;

// Remove an entry from the e820_list.
સ્થિર શૂન્ય
remove_e820(પૂર્ણાંક i)
{
    e820_ગણતરી--;
    memmove(&e820_list[i], &e820_list[i+1]
            , કદ(e820_list[0]) * (e820_ગણતરી - i));
}

// Insert an entry in the e820_list at the given position.
સ્થિર શૂન્ય
ઉમેરો_e820(પૂર્ણાંક i, u64 start, u64 size, u32 type)
{
    જો (e820_ગણતરી >= BUILD_MAX_E820) {
        warn_noalloc();
        પાછા;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , કદ(e820_list[0]) * (e820_ગણતરી - i));
    e820_ગણતરી++;
    સંરચના e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

સ્થિર અચલ અક્ષર *
e820_type_name(u32 type)
{
    પસંદગી (type) {
    સ્થિતિ E820_RAM:      પાછા "RAM";
    સ્થિતિ E820_RESERVED: પાછા "RESERVED";
    સ્થિતિ E820_ACPI:     પાછા "ACPI";
    સ્થિતિ E820_NVS:      પાછા "NVS";
    સ્થિતિ E820_UNUSABLE: પાછા "UNUSABLE";
    મૂળભૂત:            પાછા "UNKNOWN";
    }
}

// Show the current e820_list.
સ્થિર શૂન્ય
dump_map(શૂન્ય)
{
    dprintf(1, "e820 map has %d items:\n", e820_ગણતરી);
    પૂર્ણાંક i;
    માટે (i=0; i<e820_ગણતરી; i++) {
        સંરચના e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
શૂન્ય
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    જો (! size)
        // Huh?  Nothing to do.
        પાછા;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    પૂર્ણાંક i;
    માટે (i=0; i<e820_ગણતરી; i++) {
        સંરચના e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        જો (start > e_end)
            ચાલુ;
        // Found position - check if an existing item needs to be split.
        જો (start > e->start) {
            જો (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } નહીંતર {
                // Split existing item.
                e->size = start - e->start;
                i++;
                જો (e_end > end)
                    ઉમેરો_e820(i, end, e_end - end, e->type);
            }
        }
        તોડો;
    }
    // Remove/adjust existing items that are overlapping.
    જ્યારે (i<e820_ગણતરી) {
        સંરચના e820entry *e = &e820_list[i];
        જો (end < e->start)
            // No overlap - done.
            તોડો;
        u64 e_end = e->start + e->size;
        જો (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            ચાલુ;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        જો (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        તોડો;
    }
    // Insert new item.
    જો (type != E820_HOLE)
        ઉમેરો_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
શૂન્ય
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
શૂન્ય
e820_prepboot(શૂન્ય)
{
    dump_map();
}

પૂર્ણાંક
e820_is_used(u64 start, u64 size)
{
    પૂર્ણાંક i;
    માટે (i=0; i<e820_ગણતરી; i++) {
        સંરચના e820entry *e = &e820_list[i];
        જો (start + size <= e->start)
            ચાલુ;
        જો (start >= e->start + e->size)
            ચાલુ;
        પાછા 1;
    }
    પાછા 0;
}
