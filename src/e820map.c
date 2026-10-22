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
صحيح e820_ڳڻپ VARFSEG;

// Remove an entry from the e820_list.
ساكن فراغ
remove_e820(صحيح i)
{
    e820_ڳڻپ--;
    memmove(&e820_list[i], &e820_list[i+1]
            , حجم(e820_list[0]) * (e820_ڳڻپ - i));
}

// Insert an entry in the e820_list at the given position.
ساكن فراغ
وجهو_e820(صحيح i, u64 شروعات, u64 سائيز, u32 type)
{
    إذا (e820_ڳڻپ >= BUILD_MAX_E820) {
        warn_noalloc();
        أرجع;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , حجم(e820_list[0]) * (e820_ڳڻپ - i));
    e820_ڳڻپ++;
    بنية e820entry *e = &e820_list[i];
    e->شروعات = شروعات;
    e->سائيز = سائيز;
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
    dprintf(1, "e820 map has %d items:\n", e820_ڳڻپ);
    صحيح i;
    لكل (i=0; i<e820_ڳڻپ; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_پڇاڙي = e->شروعات + e->سائيز;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->شروعات, e_پڇاڙي, e->type, e820_type_name(e->type));
    }
}

#وصف_ڪريو E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
فراغ
e820_جوڙيو(u64 شروعات, u64 سائيز, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", شروعات, سائيز, type);

    إذا (! سائيز)
        // Huh?  Nothing to do.
        أرجع;

    // Find position of new item (splitting existing item if needed).
    u64 پڇاڙي = شروعات + سائيز;
    صحيح i;
    لكل (i=0; i<e820_ڳڻپ; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_پڇاڙي = e->شروعات + e->سائيز;
        إذا (شروعات > e_پڇاڙي)
            استمر;
        // Found position - check if an existing item needs to be split.
        إذا (شروعات > e->شروعات) {
            إذا (type == e->type) {
                // Same type - merge them.
                سائيز += شروعات - e->شروعات;
                شروعات = e->شروعات;
            } وإلا {
                // Split existing item.
                e->سائيز = شروعات - e->شروعات;
                i++;
                إذا (e_پڇاڙي > پڇاڙي)
                    وجهو_e820(i, پڇاڙي, e_پڇاڙي - پڇاڙي, e->type);
            }
        }
        اكسر;
    }
    // Remove/adjust existing items that are overlapping.
    طالما (i<e820_ڳڻپ) {
        بنية e820entry *e = &e820_list[i];
        إذا (پڇاڙي < e->شروعات)
            // No overlap - done.
            اكسر;
        u64 e_پڇاڙي = e->شروعات + e->سائيز;
        إذا (پڇاڙي >= e_پڇاڙي) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            استمر;
        }
        // Not completely overlapped - adjust its start.
        e->شروعات = پڇاڙي;
        e->سائيز = e_پڇاڙي - پڇاڙي;
        إذا (type == e->type) {
            // Same type - merge them.
            سائيز += e->سائيز;
            remove_e820(i);
        }
        اكسر;
    }
    // Insert new item.
    إذا (type != E820_HOLE)
        وجهو_e820(i, شروعات, سائيز, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
فراغ
e820_remove(u64 شروعات, u64 سائيز)
{
    e820_جوڙيو(شروعات, سائيز, E820_HOLE);
}

// Report on final memory locations.
فراغ
e820_prepboot(فراغ)
{
    dump_map();
}

صحيح
e820_is_استعمال_ٿيل(u64 شروعات, u64 سائيز)
{
    صحيح i;
    لكل (i=0; i<e820_ڳڻپ; i++) {
        بنية e820entry *e = &e820_list[i];
        إذا (شروعات + سائيز <= e->شروعات)
            استمر;
        إذا (شروعات >= e->شروعات + e->سائيز)
            استمر;
        أرجع 1;
    }
    أرجع 0;
}
