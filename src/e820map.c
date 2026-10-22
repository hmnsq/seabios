// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // BUILD_MAX_E820
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // struct e820entry
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
ਬਣਤਰ e820entry e820_list[BUILD_MAX_E820] VARFSEG;
ਪੂਰਨ e820_ਗਿਣਤੀ VARFSEG;

// Remove an entry from the e820_list.
ਸਥਿਰ ਖਾਲੀ
remove_e820(ਪੂਰਨ i)
{
    e820_ਗਿਣਤੀ--;
    memmove(&e820_list[i], &e820_list[i+1]
            , ਆਕਾਰ(e820_list[0]) * (e820_ਗਿਣਤੀ - i));
}

// Insert an entry in the e820_list at the given position.
ਸਥਿਰ ਖਾਲੀ
ਪਾਓ_e820(ਪੂਰਨ i, u64 ਸ਼ੁਰੂ, u64 ਮਾਪ, u32 type)
{
    ਜੇ (e820_ਗਿਣਤੀ >= BUILD_MAX_E820) {
        warn_noalloc();
        ਵਾਪਸ;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , ਆਕਾਰ(e820_list[0]) * (e820_ਗਿਣਤੀ - i));
    e820_ਗਿਣਤੀ++;
    ਬਣਤਰ e820entry *e = &e820_list[i];
    e->ਸ਼ੁਰੂ = ਸ਼ੁਰੂ;
    e->ਮਾਪ = ਮਾਪ;
    e->type = type;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
e820_type_name(u32 type)
{
    ਚੋਣ (type) {
    ਹਾਲਤ E820_RAM:      ਵਾਪਸ "RAM";
    ਹਾਲਤ E820_RESERVED: ਵਾਪਸ "RESERVED";
    ਹਾਲਤ E820_ACPI:     ਵਾਪਸ "ACPI";
    ਹਾਲਤ E820_NVS:      ਵਾਪਸ "NVS";
    ਹਾਲਤ E820_UNUSABLE: ਵਾਪਸ "UNUSABLE";
    ਮੂਲ:            ਵਾਪਸ "UNKNOWN";
    }
}

// Show the current e820_list.
ਸਥਿਰ ਖਾਲੀ
dump_map(ਖਾਲੀ)
{
    dprintf(1, "e820 map has %d items:\n", e820_ਗਿਣਤੀ);
    ਪੂਰਨ i;
    ਲਈ (i=0; i<e820_ਗਿਣਤੀ; i++) {
        ਬਣਤਰ e820entry *e = &e820_list[i];
        u64 e_ਸਿਰਾ = e->ਸ਼ੁਰੂ + e->ਮਾਪ;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->ਸ਼ੁਰੂ, e_ਸਿਰਾ, e->type, e820_type_name(e->type));
    }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
ਖਾਲੀ
e820_ਜੋੜੋ(u64 ਸ਼ੁਰੂ, u64 ਮਾਪ, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", ਸ਼ੁਰੂ, ਮਾਪ, type);

    ਜੇ (! ਮਾਪ)
        // Huh?  Nothing to do.
        ਵਾਪਸ;

    // Find position of new item (splitting existing item if needed).
    u64 ਸਿਰਾ = ਸ਼ੁਰੂ + ਮਾਪ;
    ਪੂਰਨ i;
    ਲਈ (i=0; i<e820_ਗਿਣਤੀ; i++) {
        ਬਣਤਰ e820entry *e = &e820_list[i];
        u64 e_ਸਿਰਾ = e->ਸ਼ੁਰੂ + e->ਮਾਪ;
        ਜੇ (ਸ਼ੁਰੂ > e_ਸਿਰਾ)
            ਜਾਰੀ;
        // Found position - check if an existing item needs to be split.
        ਜੇ (ਸ਼ੁਰੂ > e->ਸ਼ੁਰੂ) {
            ਜੇ (type == e->type) {
                // Same type - merge them.
                ਮਾਪ += ਸ਼ੁਰੂ - e->ਸ਼ੁਰੂ;
                ਸ਼ੁਰੂ = e->ਸ਼ੁਰੂ;
            } ਨਹੀਂਤਾਂ {
                // Split existing item.
                e->ਮਾਪ = ਸ਼ੁਰੂ - e->ਸ਼ੁਰੂ;
                i++;
                ਜੇ (e_ਸਿਰਾ > ਸਿਰਾ)
                    ਪਾਓ_e820(i, ਸਿਰਾ, e_ਸਿਰਾ - ਸਿਰਾ, e->type);
            }
        }
        ਰੋਕੋ;
    }
    // Remove/adjust existing items that are overlapping.
    ਜਦੋਂ (i<e820_ਗਿਣਤੀ) {
        ਬਣਤਰ e820entry *e = &e820_list[i];
        ਜੇ (ਸਿਰਾ < e->ਸ਼ੁਰੂ)
            // No overlap - done.
            ਰੋਕੋ;
        u64 e_ਸਿਰਾ = e->ਸ਼ੁਰੂ + e->ਮਾਪ;
        ਜੇ (ਸਿਰਾ >= e_ਸਿਰਾ) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            ਜਾਰੀ;
        }
        // Not completely overlapped - adjust its start.
        e->ਸ਼ੁਰੂ = ਸਿਰਾ;
        e->ਮਾਪ = e_ਸਿਰਾ - ਸਿਰਾ;
        ਜੇ (type == e->type) {
            // Same type - merge them.
            ਮਾਪ += e->ਮਾਪ;
            remove_e820(i);
        }
        ਰੋਕੋ;
    }
    // Insert new item.
    ਜੇ (type != E820_HOLE)
        ਪਾਓ_e820(i, ਸ਼ੁਰੂ, ਮਾਪ, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
ਖਾਲੀ
e820_remove(u64 ਸ਼ੁਰੂ, u64 ਮਾਪ)
{
    e820_ਜੋੜੋ(ਸ਼ੁਰੂ, ਮਾਪ, E820_HOLE);
}

// Report on final memory locations.
ਖਾਲੀ
e820_prepboot(ਖਾਲੀ)
{
    dump_map();
}

ਪੂਰਨ
e820_is_ਵਰਤਿਆ(u64 ਸ਼ੁਰੂ, u64 ਮਾਪ)
{
    ਪੂਰਨ i;
    ਲਈ (i=0; i<e820_ਗਿਣਤੀ; i++) {
        ਬਣਤਰ e820entry *e = &e820_list[i];
        ਜੇ (ਸ਼ੁਰੂ + ਮਾਪ <= e->ਸ਼ੁਰੂ)
            ਜਾਰੀ;
        ਜੇ (ਸ਼ੁਰੂ >= e->ਸ਼ੁਰੂ + e->ਮਾਪ)
            ਜਾਰੀ;
        ਵਾਪਸ 1;
    }
    ਵਾਪਸ 0;
}
