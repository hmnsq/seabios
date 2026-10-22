// Support for building memory maps suitable for int 15 e820 calls.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // BUILD_MAX_E820
#شامل "e820map.h" // struct e820entry
#شامل "output.h" // dprintf
#شامل "string.h" // memmove


/****************************************************************
 * e820 memory map
 ****************************************************************/

// Info on e820 map location and size.
ساخت e820entry e820_list[BUILD_MAX_E820] VARFSEG;
صحیح e820_گنتی VARFSEG;

// Remove an entry from the e820_list.
ساکن خالی
remove_e820(صحیح i)
{
    e820_گنتی--;
    memmove(&e820_list[i], &e820_list[i+1]
            , حجم(e820_list[0]) * (e820_گنتی - i));
}

// Insert an entry in the e820_list at the given position.
ساکن خالی
داخل_کرو_e820(صحیح i, u64 آغاز, u64 سائز, u32 type)
{
    اگر (e820_گنتی >= BUILD_MAX_E820) {
        warn_noalloc();
        واپس;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , حجم(e820_list[0]) * (e820_گنتی - i));
    e820_گنتی++;
    ساخت e820entry *e = &e820_list[i];
    e->آغاز = آغاز;
    e->سائز = سائز;
    e->type = type;
}

ساکن ثابت حرف *
e820_type_name(u32 type)
{
    انتخاب (type) {
    حالت E820_RAM:      واپس "RAM";
    حالت E820_RESERVED: واپس "RESERVED";
    حالت E820_ACPI:     واپس "ACPI";
    حالت E820_NVS:      واپس "NVS";
    حالت E820_UNUSABLE: واپس "UNUSABLE";
    طےشدہ:            واپس "UNKNOWN";
    }
}

// Show the current e820_list.
ساکن خالی
dump_map(خالی)
{
    dprintf(1, "e820 map has %d items:\n", e820_گنتی);
    صحیح i;
    برائے (i=0; i<e820_گنتی; i++) {
        ساخت e820entry *e = &e820_list[i];
        u64 e_انتہا = e->آغاز + e->سائز;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->آغاز, e_انتہا, e->type, e820_type_name(e->type));
    }
}

#تعریف_کرو E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
خالی
e820_جوڑو(u64 آغاز, u64 سائز, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", آغاز, سائز, type);

    اگر (! سائز)
        // Huh?  Nothing to do.
        واپس;

    // Find position of new item (splitting existing item if needed).
    u64 انتہا = آغاز + سائز;
    صحیح i;
    برائے (i=0; i<e820_گنتی; i++) {
        ساخت e820entry *e = &e820_list[i];
        u64 e_انتہا = e->آغاز + e->سائز;
        اگر (آغاز > e_انتہا)
            جاری;
        // Found position - check if an existing item needs to be split.
        اگر (آغاز > e->آغاز) {
            اگر (type == e->type) {
                // Same type - merge them.
                سائز += آغاز - e->آغاز;
                آغاز = e->آغاز;
            } ورنہ {
                // Split existing item.
                e->سائز = آغاز - e->آغاز;
                i++;
                اگر (e_انتہا > انتہا)
                    داخل_کرو_e820(i, انتہا, e_انتہا - انتہا, e->type);
            }
        }
        توڑو;
    }
    // Remove/adjust existing items that are overlapping.
    جب (i<e820_گنتی) {
        ساخت e820entry *e = &e820_list[i];
        اگر (انتہا < e->آغاز)
            // No overlap - done.
            توڑو;
        u64 e_انتہا = e->آغاز + e->سائز;
        اگر (انتہا >= e_انتہا) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            جاری;
        }
        // Not completely overlapped - adjust its start.
        e->آغاز = انتہا;
        e->سائز = e_انتہا - انتہا;
        اگر (type == e->type) {
            // Same type - merge them.
            سائز += e->سائز;
            remove_e820(i);
        }
        توڑو;
    }
    // Insert new item.
    اگر (type != E820_HOLE)
        داخل_کرو_e820(i, آغاز, سائز, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
خالی
e820_remove(u64 آغاز, u64 سائز)
{
    e820_جوڑو(آغاز, سائز, E820_HOLE);
}

// Report on final memory locations.
خالی
e820_prepboot(خالی)
{
    dump_map();
}

صحیح
e820_is_مستعمل(u64 آغاز, u64 سائز)
{
    صحیح i;
    برائے (i=0; i<e820_گنتی; i++) {
        ساخت e820entry *e = &e820_list[i];
        اگر (آغاز + سائز <= e->آغاز)
            جاری;
        اگر (آغاز >= e->آغاز + e->سائز)
            جاری;
        واپس 1;
    }
    واپس 0;
}
