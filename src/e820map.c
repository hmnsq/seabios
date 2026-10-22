// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // BUILD_MAX_E820
#समावेश "e820map.h" // struct e820entry
#समावेश "output.h" // dprintf
#समावेश "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
काष्ठा e820entry e820_list[BUILD_MAX_E820] VARFSEG;
पूर्णांक e820_गिनती VARFSEG;

// Remove an entry from the e820_list.
अटल व्योम
remove_e820(पूर्णांक i)
{
    e820_गिनती--;
    स_हटाओ(&e820_list[i], &e820_list[i+1]
            , माप(e820_list[0]) * (e820_गिनती - i));
}

// Insert an entry in the e820_list at the given position.
अटल व्योम
जोड़ो_e820(पूर्णांक i, u64 start, u64 आकार, u32 type)
{
    अगर (e820_गिनती >= BUILD_MAX_E820) {
        warn_noalloc();
        वापस;
    }

    स_हटाओ(&e820_list[i+1], &e820_list[i]
            , माप(e820_list[0]) * (e820_गिनती - i));
    e820_गिनती++;
    काष्ठा e820entry *e = &e820_list[i];
    e->start = start;
    e->आकार = आकार;
    e->type = type;
}

अटल स्थिर अक्षर *
e820_type_name(u32 type)
{
    चयन (type) {
    हाल E820_RAM:      वापस "RAM";
    हाल E820_RESERVED: वापस "RESERVED";
    हाल E820_ACPI:     वापस "ACPI";
    हाल E820_NVS:      वापस "NVS";
    हाल E820_UNUSABLE: वापस "UNUSABLE";
    शेष:            वापस "UNKNOWN";
    }
}

// Show the current e820_list.
अटल व्योम
dump_map(व्योम)
{
    dprintf(1, "e820 map has %d items:\n", e820_गिनती);
    पूर्णांक i;
    क्रम (i=0; i<e820_गिनती; i++) {
        काष्ठा e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->आकार;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->start, e_end, e->type, e820_type_name(e->type));
    }
}

#घोषणा E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
व्योम
e820_add(u64 start, u64 आकार, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", start, आकार, type);

    अगर (! आकार)
        // Huh?  Nothing to do.
        वापस;

    // Find position of new item (splitting existing item if needed).
    u64 end = start + आकार;
    पूर्णांक i;
    क्रम (i=0; i<e820_गिनती; i++) {
        काष्ठा e820entry *e = &e820_list[i];
        u64 e_end = e->start + e->आकार;
        अगर (start > e_end)
            जारी;
        // Found position - check if an existing item needs to be split.
        अगर (start > e->start) {
            अगर (type == e->type) {
                // Same type - merge them.
                आकार += start - e->start;
                start = e->start;
            } अन्यथा {
                // Split existing item.
                e->आकार = start - e->start;
                i++;
                अगर (e_end > end)
                    जोड़ो_e820(i, end, e_end - end, e->type);
            }
        }
        अवरोध;
    }
    // Remove/adjust existing items that are overlapping.
    जबतक (i<e820_गिनती) {
        काष्ठा e820entry *e = &e820_list[i];
        अगर (end < e->start)
            // No overlap - done.
            अवरोध;
        u64 e_end = e->start + e->आकार;
        अगर (end >= e_end) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            जारी;
        }
        // Not completely overlapped - adjust its start.
        e->start = end;
        e->आकार = e_end - end;
        अगर (type == e->type) {
            // Same type - merge them.
            आकार += e->आकार;
            remove_e820(i);
        }
        अवरोध;
    }
    // Insert new item.
    अगर (type != E820_HOLE)
        जोड़ो_e820(i, start, आकार, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
व्योम
e820_remove(u64 start, u64 आकार)
{
    e820_add(start, आकार, E820_HOLE);
}

// Report on final memory locations.
व्योम
e820_prepboot(व्योम)
{
    dump_map();
}

पूर्णांक
e820_is_used(u64 start, u64 आकार)
{
    पूर्णांक i;
    क्रम (i=0; i<e820_गिनती; i++) {
        काष्ठा e820entry *e = &e820_list[i];
        अगर (start + आकार <= e->start)
            जारी;
        अगर (start >= e->start + e->आकार)
            जारी;
        वापस 1;
    }
    वापस 0;
}
