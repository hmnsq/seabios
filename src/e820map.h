#جے_بنا_تعریف __E820MAP_H
#تعریف_کرو __E820MAP_H

#ضمن "types.h" // u64

#تعریف_کرو E820_RAM            1
#تعریف_کرو E820_RESERVED       2
#تعریف_کرو E820_ACPI           3
#تعریف_کرو E820_NVS            4
#تعریف_کرو E820_UNUSABLE       5
#تعریف_کرو E820_SOFT_RESERVED  0xEFFFFFFF

بنية e820entry {
    u64 شروع;
    u64 ناپ;
    u32 type;
};

فراغ e820_جوڑو(u64 شروع, u64 ناپ, u32 type);
فراغ e820_remove(u64 شروع, u64 ناپ);
فراغ e820_prepboot(فراغ);
صحيح e820_is_ورتیا(u64 شروع, u64 ناپ);

// e820 map storage
خارجي بنية e820entry e820_list[];
خارجي صحيح e820_گنتی;

#جے_انت // e820map.h
