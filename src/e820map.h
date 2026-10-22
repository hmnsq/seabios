#ifndef __E820MAP_H
#define __E820MAP_H

#அந்தர்பாவ "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

கட்டமைப்பு e820entry {
    u64 start;
    u64 size;
    u32 type;
};

வெறுமை e820_add(u64 start, u64 size, u32 type);
வெறுமை e820_remove(u64 start, u64 size);
வெறுமை e820_prepboot(வெறுமை);
முழு e820_is_used(u64 start, u64 size);

// e820 map storage
வெளி கட்டமைப்பு e820entry e820_list[];
வெளி முழு e820_மொத்தம்;

#endif // e820map.h
