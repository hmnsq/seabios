#اگر_غیر_تعریف_شدہ __E820MAP_H
#تعریف_کرو __E820MAP_H

#شامل "types.h" // u64

#تعریف_کرو E820_RAM            1
#تعریف_کرو E820_RESERVED       2
#تعریف_کرو E820_ACPI           3
#تعریف_کرو E820_NVS            4
#تعریف_کرو E820_UNUSABLE       5
#تعریف_کرو E820_SOFT_RESERVED  0xEFFFFFFF

ساخت e820entry {
    u64 آغاز;
    u64 سائز;
    u32 type;
};

خالی e820_جوڑو(u64 آغاز, u64 سائز, u32 type);
خالی e820_remove(u64 آغاز, u64 سائز);
خالی e820_prepboot(خالی);
صحیح e820_is_مستعمل(u64 آغاز, u64 سائز);

// e820 map storage
خارجی ساخت e820entry e820_list[];
خارجی صحیح e820_گنتی;

#اگر_ختم // e820map.h
