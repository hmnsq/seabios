#ifndef __E820MAP_H
#define __E820MAP_H

#അന്തര്ഭാവ "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

ഘടന e820entry {
    u64 start;
    u64 size;
    u32 type;
};

ശൂന്യം e820_add(u64 start, u64 size, u32 type);
ശൂന്യം e820_remove(u64 start, u64 size);
ശൂന്യം e820_prepboot(ശൂന്യം);
പൂർണ്ണസംഖ്യ e820_is_used(u64 start, u64 size);

// e820 map storage
ബാഹ്യ ഘടന e820entry e820_list[];
ബാഹ്യ പൂർണ്ണസംഖ്യ e820_എണ്ണം;

#endif // e820map.h
