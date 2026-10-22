#ifndef __E820MAP_H
#define __E820MAP_H

#અન્તર્ભાવ "types.h" // u64

#define E820_RAM            1
#define E820_RESERVED       2
#define E820_ACPI           3
#define E820_NVS            4
#define E820_UNUSABLE       5
#define E820_SOFT_RESERVED  0xEFFFFFFF

સંરચના e820entry {
    u64 start;
    u64 size;
    u32 type;
};

શૂન્ય e820_add(u64 start, u64 size, u32 type);
શૂન્ય e820_remove(u64 start, u64 size);
શૂન્ય e820_prepboot(શૂન્ય);
પૂર્ણાંક e820_is_used(u64 start, u64 size);

// e820 map storage
બાહ્ય સંરચના e820entry e820_list[];
બાહ્ય પૂર્ણાંક e820_ગણતરી;

#endif // e820map.h
