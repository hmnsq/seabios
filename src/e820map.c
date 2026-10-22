// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // BUILD_MAX_E820
#അന്തര്ഭാവ "e820map.h" // struct e820entry
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
ഘടന e820entry e820_list[BUILD_MAX_E820] VARFSEG;
പൂർണ്ണസംഖ്യ e820_എണ്ണം VARFSEG;

// Remove an entry from the e820_list.
സ്ഥിതം ശൂന്യം
remove_e820(പൂർണ്ണസംഖ്യ i)
{
    e820_എണ്ണം--;
    memmove(&e820_list[i], &e820_list[i+1]
            , വലുപ്പം(e820_list[0]) * (e820_എണ്ണം - i));
}

// Insert an entry in the e820_list at the given position.
സ്ഥിതം ശൂന്യം
തിരുകുക_e820(പൂർണ്ണസംഖ്യ i, u64 start, u64 size, u32 type)
{
    എങ്കിൽ (e820_എണ്ണം >= BUILD_MAX_E820) {
        warn_noalloc();
        തിരികെ;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , വലുപ്പം(e820_list[0]) * (e820_എണ്ണം - i));
    e820_എണ്ണം++;
    ഘടന e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
e820_type_name(u32 type)
{
    തിരഞ്ഞെടുപ്പ് (type) {
    അവസ്ഥ E820_RAM:      തിരികെ "RAM";
    അവസ്ഥ E820_RESERVED: തിരികെ "RESERVED";
    അവസ്ഥ E820_ACPI:     തിരികെ "ACPI";
    അവസ്ഥ E820_NVS:      തിരികെ "NVS";
    അവസ്ഥ E820_UNUSABLE: തിരികെ "UNUSABLE";
    സ്ഥിരസ്ഥിതി:            തിരികെ "UNKNOWN";
    }
}

// Show the current e820_list.
സ്ഥിതം ശൂന്യം
dump_map(ശൂന്യം)
{
    dprintf(1, "e820 map has %d items:\n", e820_എണ്ണം);
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<e820_എണ്ണം; i++) {
        ഘടന e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
ശൂന്യം
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    എങ്കിൽ (! size)
        // Huh?  Nothing to do.
        തിരികെ;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<e820_എണ്ണം; i++) {
        ഘടന e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        എങ്കിൽ (start > e_end)
            തുടരുക;
        // Found position - check if an existing item needs to be split.
        എങ്കിൽ (start > e->start) {
            എങ്കിൽ (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } അല്ലെങ്കിൽ {
                // Split existing item.
                e->size = start - e->start;
                i++;
                എങ്കിൽ (e_end > end)
                    തിരുകുക_e820(i, end, e_end - end, e->type);
            }
        }
        നിർത്തുക;
    }
    // Remove/adjust existing items that are overlapping.
    ആകുമ്പോൾ (i<e820_എണ്ണം) {
        ഘടന e820entry *e = &e820_list[i];
        എങ്കിൽ (end < e->start)
            // No overlap - done.
            നിർത്തുക;
        u64 e_end = e->start + e->size;
        എങ്കിൽ (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            തുടരുക;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        എങ്കിൽ (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        നിർത്തുക;
    }
    // Insert new item.
    എങ്കിൽ (type != E820_HOLE)
        തിരുകുക_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
ശൂന്യം
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
ശൂന്യം
e820_prepboot(ശൂന്യം)
{
    dump_map();
}

പൂർണ്ണസംഖ്യ
e820_is_used(u64 start, u64 size)
{
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<e820_എണ്ണം; i++) {
        ഘടന e820entry *e = &e820_list[i];
        എങ്കിൽ (start + size <= e->start)
            തുടരുക;
        എങ്കിൽ (start >= e->start + e->size)
            തുടരുക;
        തിരികെ 1;
    }
    തിരികെ 0;
}
