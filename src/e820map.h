#ifndef __E820MAP_H
#define __E820MAP_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

ସଂରଚନା e820entry {
    u64 start;
    u64 size;
    u32 type;
};

ଶୂନ୍ୟ e820_add(u64 start, u64 size, u32 type);
ଶୂନ୍ୟ e820_remove(u64 start, u64 size);
ଶୂନ୍ୟ e820_prepboot(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା e820_is_used(u64 start, u64 size);

// e820 map storage
ବାହ୍ୟ ସଂରଚନା e820entry e820_list[];
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା e820_ଗଣନା;

#endif // e820map.h
