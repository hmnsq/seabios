// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // BUILD_MAX_E820
#अन्तर्भाव "e820map.h" // struct e820entry
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
संरचना e820entry e820_list[BUILD_MAX_E820] VARFSEG;
पूर्णाङ्क e820_गणनम् VARFSEG;

// Remove an entry from the e820_list.
स्थिर शून्य
remove_e820(पूर्णाङ्क i)
{
    e820_गणनम्--;
    memmove(&e820_list[i], &e820_list[i+1]
            , परिमाण(e820_list[0]) * (e820_गणनम् - i));
}

// Insert an entry in the e820_list at the given position.
स्थिर शून्य
निवेशय_e820(पूर्णाङ्क i, u64 start, u64 आकार, u32 type)
{
    यदि (e820_गणनम् >= BUILD_MAX_E820) {
        warn_noalloc();
        प्रत्यागम;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , परिमाण(e820_list[0]) * (e820_गणनम् - i));
    e820_गणनम्++;
    संरचना e820entry *e = &e820_list[i];
    e->start = start;
    e->आकार = आकार;
    e->type = type;
}

स्थिर ध्रुव वर्ण *
e820_type_name(u32 type)
{
    विकल्प (type) {
    पक्ष E820_RAM:      प्रत्यागम "RAM";
    पक्ष E820_RESERVED: प्रत्यागम "RESERVED";
    पक्ष E820_ACPI:     प्रत्यागम "ACPI";
    पक्ष E820_NVS:      प्रत्यागम "NVS";
    पक्ष E820_UNUSABLE: प्रत्यागम "UNUSABLE";
    स्वभाव:            प्रत्यागम "UNKNOWN";
    }
}

// Show the current e820_list.
स्थिर शून्य
dump_map(शून्य)
{
    dprintf(1, "e820 map has %d items:\n", e820_गणनम्);
    पूर्णाङ्क i;
    प्रति (i=0; i<e820_गणनम्; i++) {
        संरचना e820entry *e = &e820_list[i];
        u64 e_अन्तः = e->start + e->आकार;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_अन्तः, e->type, e820_type_name(e->type));
    }
}

#परिभाषय E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
शून्य
e820_सङ्कलय(u64 start, u64 आकार, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, आकार, type);

    यदि (! आकार)
        // Huh?  Nothing to do.
        प्रत्यागम;

    // Find position of new item (splitting existing item if needed).
    u64 अन्तः = start + आकार;
    पूर्णाङ्क i;
    प्रति (i=0; i<e820_गणनम्; i++) {
        संरचना e820entry *e = &e820_list[i];
        u64 e_अन्तः = e->start + e->आकार;
        यदि (start > e_अन्तः)
            अनुवर्त;
        // Found position - check if an existing item needs to be split.
        यदि (start > e->start) {
            यदि (type == e->type) {
                // Same type - merge them.
                आकार += start - e->start;
                start = e->start;
            } अन्यथा {
                // Split existing item.
                e->आकार = start - e->start;
                i++;
                यदि (e_अन्तः > अन्तः)
                    निवेशय_e820(i, अन्तः, e_अन्तः - अन्तः, e->type);
            }
        }
        भङ्ग;
    }
    // Remove/adjust existing items that are overlapping.
    यावत् (i<e820_गणनम्) {
        संरचना e820entry *e = &e820_list[i];
        यदि (अन्तः < e->start)
            // No overlap - done.
            भङ्ग;
        u64 e_अन्तः = e->start + e->आकार;
        यदि (अन्तः >= e_अन्तः) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            अनुवर्त;
        }
        // Not completely overlapped - adjust its start.
        e->start = अन्तः;
        e->आकार = e_अन्तः - अन्तः;
        यदि (type == e->type) {
            // Same type - merge them.
            आकार += e->आकार;
            remove_e820(i);
        }
        भङ्ग;
    }
    // Insert new item.
    यदि (type != E820_HOLE)
        निवेशय_e820(i, start, आकार, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
शून्य
e820_remove(u64 start, u64 आकार)
{
    e820_सङ्कलय(start, आकार, E820_HOLE);
}

// Report on final memory locations.
शून्य
e820_prepboot(शून्य)
{
    dump_map();
}

पूर्णाङ्क
e820_is_प्रयुक्तम्(u64 start, u64 आकार)
{
    पूर्णाङ्क i;
    प्रति (i=0; i<e820_गणनम्; i++) {
        संरचना e820entry *e = &e820_list[i];
        यदि (start + आकार <= e->start)
            अनुवर्त;
        यदि (start >= e->start + e->आकार)
            अनुवर्त;
        प्रत्यागम 1;
    }
    प्रत्यागम 0;
}
