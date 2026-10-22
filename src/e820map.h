#ifndef __E820MAP_H
#define __E820MAP_H

#अन्तर्भाव "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

संरचना e820entry {
    u64 start;
    u64 size;
    u32 type;
};

शून्य e820_add(u64 start, u64 size, u32 type);
शून्य e820_remove(u64 start, u64 size);
शून्य e820_prepboot(शून्य);
पूर्णाङ्क e820_is_used(u64 start, u64 size);

// e820 map storage
बाह्य संरचना e820entry e820_list[];
बाह्य पूर्णाङ्क e820_गिनती;

#endif // e820map.h
