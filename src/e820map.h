#ifndef __E820MAP_H
#define __E820MAP_H

#ಅನ್ತರ್ಭಾವ "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

ಸಂರಚನಾ e820entry {
    u64 start;
    u64 size;
    u32 type;
};

ಶೂನ್ಯ e820_add(u64 start, u64 size, u32 type);
ಶೂನ್ಯ e820_remove(u64 start, u64 size);
ಶೂನ್ಯ e820_prepboot(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ e820_is_used(u64 start, u64 size);

// e820 map storage
ಬಾಹ್ಯ ಸಂರಚನಾ e820entry e820_list[];
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ e820_ಎಣಿಕೆ;

#endif // e820map.h
