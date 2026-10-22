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
صحيح e820_گنتی VARFSEG;

// Remove an entry from the e820_list.
ساكن فراغ
remove_e820(صحيح i)
{
    e820_گنتی--;
    memmove(&e820_list[i], &e820_list[i+1]
            , حجم(e820_list[0]) * (e820_گنتی - i));
}

// Insert an entry in the e820_list at the given position.
ساكن فراغ
داخل_کرو_e820(صحيح i, u64 شروع, u64 ناپ, u32 type)
{
    إذا (e820_گنتی >= BUILD_MAX_E820) {
        warn_noalloc();
        أرجع;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , حجم(e820_list[0]) * (e820_گنتی - i));
    e820_گنتی++;
    بنية e820entry *e = &e820_list[i];
    e->شروع = شروع;
    e->ناپ = ناپ;
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
    dprintf(1, "e820 map has %d items:\n", e820_گنتی);
    صحيح i;
    لكل (i=0; i<e820_گنتی; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_سرا = e->شروع + e->ناپ;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->شروع, e_سرا, e->type, e820_type_name(e->type));
    }
}

#تعریف_کرو E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
فراغ
e820_جوڑو(u64 شروع, u64 ناپ, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", شروع, ناپ, type);

    إذا (! ناپ)
        // Huh?  Nothing to do.
        أرجع;

    // Find position of new item (splitting existing item if needed).
    u64 سرا = شروع + ناپ;
    صحيح i;
    لكل (i=0; i<e820_گنتی; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_سرا = e->شروع + e->ناپ;
        إذا (شروع > e_سرا)
            استمر;
        // Found position - check if an existing item needs to be split.
        إذا (شروع > e->شروع) {
            إذا (type == e->type) {
                // Same type - merge them.
                ناپ += شروع - e->شروع;
                شروع = e->شروع;
            } وإلا {
                // Split existing item.
                e->ناپ = شروع - e->شروع;
                i++;
                إذا (e_سرا > سرا)
                    داخل_کرو_e820(i, سرا, e_سرا - سرا, e->type);
            }
        }
        اكسر;
    }
    // Remove/adjust existing items that are overlapping.
    طالما (i<e820_گنتی) {
        بنية e820entry *e = &e820_list[i];
        إذا (سرا < e->شروع)
            // No overlap - done.
            اكسر;
        u64 e_سرا = e->شروع + e->ناپ;
        إذا (سرا >= e_سرا) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            استمر;
        }
        // Not completely overlapped - adjust its start.
        e->شروع = سرا;
        e->ناپ = e_سرا - سرا;
        إذا (type == e->type) {
            // Same type - merge them.
            ناپ += e->ناپ;
            remove_e820(i);
        }
        اكسر;
    }
    // Insert new item.
    إذا (type != E820_HOLE)
        داخل_کرو_e820(i, شروع, ناپ, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
فراغ
e820_remove(u64 شروع, u64 ناپ)
{
    e820_جوڑو(شروع, ناپ, E820_HOLE);
}

// Report on final memory locations.
فراغ
e820_prepboot(فراغ)
{
    dump_map();
}

صحيح
e820_is_ورتیا(u64 شروع, u64 ناپ)
{
    صحيح i;
    لكل (i=0; i<e820_گنتی; i++) {
        بنية e820entry *e = &e820_list[i];
        إذا (شروع + ناپ <= e->شروع)
            استمر;
        إذا (شروع >= e->شروع + e->ناپ)
            استمر;
        أرجع 1;
    }
    أرجع 0;
}
