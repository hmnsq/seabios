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
पूर्णांक e820_मोजणी VARFSEG;

// Remove an entry from the e820_list.
स्थिर शून्य
remove_e820(पूर्णांक i)
{
    e820_मोजणी--;
    स_हटाओ(&e820_list[i], &e820_list[i+1]
            , आकार(e820_list[0]) * (e820_मोजणी - i));
}

// Insert an entry in the e820_list at the given position.
स्थिर शून्य
घाला_e820(पूर्णांक i, u64 start, u64 size, u32 type)
{
    जर (e820_मोजणी >= BUILD_MAX_E820) {
        warn_noalloc();
        परत;
    }

    स_हटाओ(&e820_list[i+1], &e820_list[i]
            , आकार(e820_list[0]) * (e820_मोजणी - i));
    e820_मोजणी++;
    संरचना e820entry *e = &e820_list[i];
    e->start = start;
    e->size = size;
    e->type = type;
}

स्थिर स्थिरांक अक्षर *
e820_type_name(u32 type)
{
    निवड (type) {
    स्थिती E820_RAM:      परत "RAM";
    स्थिती E820_RESERVED: परत "RESERVED";
    स्थिती E820_ACPI:     परत "ACPI";
    स्थिती E820_NVS:      परत "NVS";
    स्थिती E820_UNUSABLE: परत "UNUSABLE";
    पूर्वनिर्धारित:            परत "UNKNOWN";
    }
}

// Show the current e820_list.
स्थिर शून्य
dump_map(शून्य)
{
    dprintf(1, "e820 map has %d items:\n", e820_मोजणी);
    पूर्णांक i;
    साठी (i=0; i<e820_मोजणी; i++) {
        संरचना e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#define E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
शून्य
e820_add(u64 start, u64 size, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, size, type);

    जर (! size)
        // Huh?  Nothing to do.
        परत;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + size;
    पूर्णांक i;
    साठी (i=0; i<e820_मोजणी; i++) {
        संरचना e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->size;
        जर (start > e_end)
            सुरूठेवा;
        // Found position - check if an existing item needs to be split.
        जर (start > e->start) {
            जर (type == e->type) {
                // Same type - merge them.
                size += start - e->start;
                start = e->start;
            } अन्यथा {
                // Split existing item.
                e->size = start - e->start;
                i++;
                जर (e_end > end)
                    घाला_e820(i, end, e_end - end, e->type);
            }
        }
        थांबा;
    }
    // Remove/adjust existing items that are overlapping.
    पर्यंत (i<e820_मोजणी) {
        संरचना e820entry *e = &e820_list[i];
        जर (end < e->start)
            // No overlap - done.
            थांबा;
        u64 e_end = e->start + e->size;
        जर (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            सुरूठेवा;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->size = e_end - end;
        जर (type == e->type) {
            // Same type - merge them.
            size += e->size;
            remove_e820(i);
        }
        थांबा;
    }
    // Insert new item.
    जर (type != E820_HOLE)
        घाला_e820(i, start, size, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
शून्य
e820_remove(u64 start, u64 size)
{
    e820_add(start, size, E820_HOLE);
}

// Report on final memory locations.
शून्य
e820_prepboot(शून्य)
{
    dump_map();
}

पूर्णांक
e820_is_used(u64 start, u64 size)
{
    पूर्णांक i;
    साठी (i=0; i<e820_मोजणी; i++) {
        संरचना e820entry *e = &e820_list[i];
        जर (start + size <= e->start)
            सुरूठेवा;
        जर (start >= e->start + e->size)
            सुरूठेवा;
        परत 1;
    }
    परत 0;
}
