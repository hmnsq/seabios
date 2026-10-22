#ifndef __E820MAP_H
#define __E820MAP_H

#অন্তর্ভাৱ "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

সংরচনা e820entry {
    u64 start;
    u64 size;
    u32 type;
};

শূন্য e820_add(u64 start, u64 size, u32 type);
শূন্য e820_remove(u64 start, u64 size);
শূন্য e820_prepboot(শূন্য);
পূর্ণাঙ্ক e820_is_used(u64 start, u64 size);

// e820 map storage
বাহ্য সংরচনা e820entry e820_list[];
বাহ্য পূর্ণাঙ্ক e820_গুনতি;

#endif // e820map.h
