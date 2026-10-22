// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // BUILD_MAX_E820
#ଅନ୍ତର୍ଭାଵ "e820map.h" // struct e820entry
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
ସଂରଚନା e820entry e820_list[BUILD_MAX_E820] VARFSEG;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା e820_ଗଣନା VARFSEG;

// Remove an entry from the e820_list.
ସ୍ଥିର ଶୂନ୍ୟ
remove_e820(ପୂର୍ଣ୍ଣସଂଖ୍ୟା i)
{
    e820_ଗଣନା--;
    memmove(&e820_list[i], &e820_list[i+1]
            , ଆକାର(e820_list[0]) * (e820_ଗଣନା - i));
}

// Insert an entry in the e820_list at the given position.
ସ୍ଥିର ଶୂନ୍ୟ
ଭର୍ତ୍ତି_e820(ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, u64 start, u64 size, u32 type)
{
    ଯଦି (e820_ଗଣନା >= BUILD_MAX_E820) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , ଆକାର(e820_list[0]) * (e820_ଗଣନା - i));
    e820_ଗଣନା++;
    ସଂରଚନା e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
e820_type_name(u32 type)
{
    ବାଛ (type) {
    ସ୍ଥିତି E820_RAM:      ଫେରନ୍ତୁ "RAM";
    ସ୍ଥିତି E820_RESERVED: ଫେରନ୍ତୁ "RESERVED";
    ସ୍ଥିତି E820_ACPI:     ଫେରନ୍ତୁ "ACPI";
    ସ୍ଥିତି E820_NVS:      ଫେରନ୍ତୁ "NVS";
    ସ୍ଥିତି E820_UNUSABLE: ଫେରନ୍ତୁ "UNUSABLE";
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:            ଫେରନ୍ତୁ "UNKNOWN";
    }
}

// Show the current e820_list.
ସ୍ଥିର ଶୂନ୍ୟ
dump_map(ଶୂନ୍ୟ)
{
    dprintf(1, "e820 map has %d items:\n", e820_ଗଣନା);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<e820_ଗଣନା; i++) {
        ସଂରଚନା e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
ଶୂନ୍ୟ
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    ଯଦି (! size)
        // Huh?  Nothing to do.
        ଫେରନ୍ତୁ;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<e820_ଗଣନା; i++) {
        ସଂରଚନା e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        ଯଦି (start > e_end)
            ଜାରି;
        // Found position - check if an existing item needs to be split.
        ଯଦି (start > e->start) {
            ଯଦି (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } ଅନ୍ୟଥା {
                // Split existing item.
                e->size = start - e->start;
                i++;
                ଯଦି (e_end > end)
                    ଭର୍ତ୍ତି_e820(i, end, e_end - end, e->type);
            }
        }
        ଭାଙ୍ଗ;
    }
    // Remove/adjust existing items that are overlapping.
    ଯେତେବେଳେ (i<e820_ଗଣନା) {
        ସଂରଚନା e820entry *e = &e820_list[i];
        ଯଦି (end < e->start)
            // No overlap - done.
            ଭାଙ୍ଗ;
        u64 e_end = e->start + e->size;
        ଯଦି (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            ଜାରି;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        ଯଦି (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        ଭାଙ୍ଗ;
    }
    // Insert new item.
    ଯଦି (type != E820_HOLE)
        ଭର୍ତ୍ତି_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
ଶୂନ୍ୟ
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
ଶୂନ୍ୟ
e820_prepboot(ଶୂନ୍ୟ)
{
    dump_map();
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
e820_is_used(u64 start, u64 size)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<e820_ଗଣନା; i++) {
        ସଂରଚନା e820entry *e = &e820_list[i];
        ଯଦି (start + size <= e->start)
            ଜାରି;
        ଯଦି (start >= e->start + e->size)
            ଜାରି;
        ଫେରନ୍ତୁ 1;
    }
    ଫେରନ୍ତୁ 0;
}
