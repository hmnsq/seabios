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
صحيح e820_شمېر VARFSEG;

// Remove an entry from the e820_list.
ساكن فراغ
remove_e820(صحيح i)
{
    e820_شمېر--;
    memmove(&e820_list[i], &e820_list[i+1]
            , حجم(e820_list[0]) * (e820_شمېر - i));
}

// Insert an entry in the e820_list at the given position.
ساكن فراغ
ورننباسه_e820(صحيح i, u64 پیل, u64 اندازه, u32 type)
{
    إذا (e820_شمېر >= BUILD_MAX_E820) {
        warn_noalloc();
        أرجع;
    }

    memmove(&e820_list[i+1], &e820_list[i]
            , حجم(e820_list[0]) * (e820_شمېر - i));
    e820_شمېر++;
    بنية e820entry *e = &e820_list[i];
    e->پیل = پیل;
    e->اندازه = اندازه;
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
    dprintf(1, "e820 map has %d items:\n", e820_شمېر);
    صحيح i;
    لكل (i=0; i<e820_شمېر; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_پای_ټکی = e->پیل + e->اندازه;
        dprintf(1, "  %d: %016llx - %016llx = %d %s\n", i
                , e->پیل, e_پای_ټکی, e->type, e820_type_name(e->type));
    }
}

#تعریف_کړه E820_HOLE         ((u32)-1) // Used internally to remove entries

// Add a new entry to the list.  This scans for overlaps and keeps the
// list sorted.
فراغ
e820_جمع_کړه(u64 پیل, u64 اندازه, u32 type)
{
    dprintf(8, "Add to e820 map: %08llx %08llx %d\n", پیل, اندازه, type);

    إذا (! اندازه)
        // Huh?  Nothing to do.
        أرجع;

    // Find position of new item (splitting existing item if needed).
    u64 پای_ټکی = پیل + اندازه;
    صحيح i;
    لكل (i=0; i<e820_شمېر; i++) {
        بنية e820entry *e = &e820_list[i];
        u64 e_پای_ټکی = e->پیل + e->اندازه;
        إذا (پیل > e_پای_ټکی)
            استمر;
        // Found position - check if an existing item needs to be split.
        إذا (پیل > e->پیل) {
            إذا (type == e->type) {
                // Same type - merge them.
                اندازه += پیل - e->پیل;
                پیل = e->پیل;
            } وإلا {
                // Split existing item.
                e->اندازه = پیل - e->پیل;
                i++;
                إذا (e_پای_ټکی > پای_ټکی)
                    ورننباسه_e820(i, پای_ټکی, e_پای_ټکی - پای_ټکی, e->type);
            }
        }
        اكسر;
    }
    // Remove/adjust existing items that are overlapping.
    طالما (i<e820_شمېر) {
        بنية e820entry *e = &e820_list[i];
        إذا (پای_ټکی < e->پیل)
            // No overlap - done.
            اكسر;
        u64 e_پای_ټکی = e->پیل + e->اندازه;
        إذا (پای_ټکی >= e_پای_ټکی) {
            // Existing item completely overlapped - remove it.
            remove_e820(i);
            استمر;
        }
        // Not completely overlapped - adjust its start.
        e->پیل = پای_ټکی;
        e->اندازه = e_پای_ټکی - پای_ټکی;
        إذا (type == e->type) {
            // Same type - merge them.
            اندازه += e->اندازه;
            remove_e820(i);
        }
        اكسر;
    }
    // Insert new item.
    إذا (type != E820_HOLE)
        ورننباسه_e820(i, پیل, اندازه, type);
    //dump_map();
}

// Remove any definitions in a memory range (make a memory hole).
فراغ
e820_remove(u64 پیل, u64 اندازه)
{
    e820_جمع_کړه(پیل, اندازه, E820_HOLE);
}

// Report on final memory locations.
فراغ
e820_prepboot(فراغ)
{
    dump_map();
}

صحيح
e820_is_کارول_شوی(u64 پیل, u64 اندازه)
{
    صحيح i;
    لكل (i=0; i<e820_شمېر; i++) {
        بنية e820entry *e = &e820_list[i];
        إذا (پیل + اندازه <= e->پیل)
            استمر;
        إذا (پیل >= e->پیل + e->اندازه)
            استمر;
        أرجع 1;
    }
    أرجع 0;
}
