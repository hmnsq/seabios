#ifndef __E820MAP_H
#define __E820MAP_H

#شامل "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

ساختار e820entry {
    u64 start;
    u64 size;
    u32 type;
};

تهی e820_add(u64 start, u64 size, u32 type);
تهی e820_remove(u64 start, u64 size);
تهی e820_prepboot(تهی);
صحیح e820_is_used(u64 start, u64 size);

// e820 map storage
بیرونی ساختار e820entry e820_list[];
بیرونی صحیح e820_شمار;

#endif // e820map.h
