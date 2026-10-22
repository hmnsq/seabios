// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // BUILD_MAX_E820
#అన్తర్భావ "e820map.h" // struct e820entry
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
నిర్మాణం e820entry e820_list[BUILD_MAX_E820] VARFSEG;
పూర్ణాంకం e820_లెక్క VARFSEG;

// Remove an entry from the e820_list.
స్థిర శూన్య
remove_e820(పూర్ణాంకం i)
{
    e820_లెక్క--;
    memmove(&e820_list[i], &e820_list[i+1]
            , పరిమాణం(e820_list[0]) * (e820_లెక్క - i));
}

// Insert an entry in the e820_list at the given position.
స్థిర శూన్య
చొప్పించు_e820(పూర్ణాంకం i, u64 ప్రారంభం, u64 కొలత, u32 type)
{
    ఒకవేళ (e820_లెక్క >= BUILD_MAX_E820) {
        warn_noalloc();
        తిరిగి;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , పరిమాణం(e820_list[0]) * (e820_లెక్క - i));
    e820_లెక్క++;
    నిర్మాణం e820entry *e = &e820_list[i];
    e->ప్రారంభం = ప్రారంభం;
    e->కొలత = కొలత;
    e->type = type;
}

స్థిర స్థిరరాశి అక్షరం *
e820_type_name(u32 type)
{
    ఎంపిక (type) {
    స్థితి E820_RAM:      తిరిగి "RAM";
    స్థితి E820_RESERVED: తిరిగి "RESERVED";
    స్థితి E820_ACPI:     తిరిగి "ACPI";
    స్థితి E820_NVS:      తిరిగి "NVS";
    స్థితి E820_UNUSABLE: తిరిగి "UNUSABLE";
    అప్రమేయ:            తిరిగి "UNKNOWN";
    }
}

// Show the current e820_list.
స్థిర శూన్య
dump_map(శూన్య)
{
    dprintf(1, "e820 map has %d items:\n", e820_లెక్క);
    పూర్ణాంకం i;
    కొరకు (i=0; i<e820_లెక్క; i++) {
        నిర్మాణం e820entry *e = &e820_list[i];
        u64 e_చివర = e->ప్రారంభం + e->కొలత;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->ప్రారంభం, e_చివర, e->type, e820_type_name(e->type));
    }
}

#నిర్వచించు E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
శూన్య
e820_కూడు(u64 ప్రారంభం, u64 కొలత, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", ప్రారంభం, కొలత, type);

    ఒకవేళ (! కొలత)
        // Huh?  Nothing to do.
        తిరిగి;

    // Find position of new item (splitting existing item if needed).
    u64 చివర = ప్రారంభం + కొలత;
    పూర్ణాంకం i;
    కొరకు (i=0; i<e820_లెక్క; i++) {
        నిర్మాణం e820entry *e = &e820_list[i];
        u64 e_చివర = e->ప్రారంభం + e->కొలత;
        ఒకవేళ (ప్రారంభం > e_చివర)
            కొనసాగు;
        // Found position - check if an existing item needs to be split.
        ఒకవేళ (ప్రారంభం > e->ప్రారంభం) {
            ఒకవేళ (type == e->type) {
                // Same type - merge them.
                కొలత += ప్రారంభం - e->ప్రారంభం;
                ప్రారంభం = e->ప్రారంభం;
            } లేకపోతే {
                // Split existing item.
                e->కొలత = ప్రారంభం - e->ప్రారంభం;
                i++;
                ఒకవేళ (e_చివర > చివర)
                    చొప్పించు_e820(i, చివర, e_చివర - చివర, e->type);
            }
        }
        ఆపు;
    }
    // Remove/adjust existing items that are overlapping.
    అయితే (i<e820_లెక్క) {
        నిర్మాణం e820entry *e = &e820_list[i];
        ఒకవేళ (చివర < e->ప్రారంభం)
            // No overlap - done.
            ఆపు;
        u64 e_చివర = e->ప్రారంభం + e->కొలత;
        ఒకవేళ (చివర >= e_చివర) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            కొనసాగు;
        }
        // Not completely overlapped - adjust its start.
        e->ప్రారంభం = చివర;
        e->కొలత = e_చివర - చివర;
        ఒకవేళ (type == e->type) {
            // Same type - merge them.
            కొలత += e->కొలత;
            remove_e820(i);
        }
        ఆపు;
    }
    // Insert new item.
    ఒకవేళ (type != E820_HOLE)
        చొప్పించు_e820(i, ప్రారంభం, కొలత, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
శూన్య
e820_remove(u64 ప్రారంభం, u64 కొలత)
{
    e820_కూడు(ప్రారంభం, కొలత, E820_HOLE);
}

// Report on final memory locations.
శూన్య
e820_prepboot(శూన్య)
{
    dump_map();
}

పూర్ణాంకం
e820_is_వాడిన(u64 ప్రారంభం, u64 కొలత)
{
    పూర్ణాంకం i;
    కొరకు (i=0; i<e820_లెక్క; i++) {
        నిర్మాణం e820entry *e = &e820_list[i];
        ఒకవేళ (ప్రారంభం + కొలత <= e->ప్రారంభం)
            కొనసాగు;
        ఒకవేళ (ప్రారంభం >= e->ప్రారంభం + e->కొలత)
            కొనసాగు;
        తిరిగి 1;
    }
    తిరిగి 0;
}
