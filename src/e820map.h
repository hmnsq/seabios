#यदि_अपरिभाषितम् __E820MAP_H
#परिभाषय __E820MAP_H

#अन्तर्भाव "types.h" // u64

#परिभाषय E820_RAM            1
#परिभाषय E820_RESERVED       2
#परिभाषय E820_ACPI           3
#परिभाषय E820_NVS            4
#परिभाषय E820_UNUSABLE       5
#परिभाषय E820_SOFT_RESERVED  0xEFFFFFFF

संरचना e820entry {
    u64 start;
    u64 आकार;
    u32 type;
};

शून्य e820_सङ्कलय(u64 start, u64 आकार, u32 type);
शून्य e820_remove(u64 start, u64 आकार);
शून्य e820_prepboot(शून्य);
पूर्णाङ्क e820_is_प्रयुक्तम्(u64 start, u64 आकार);

// e820 map storage
बाह्य संरचना e820entry e820_list[];
बाह्य पूर्णाङ्क e820_गणनम्;

#यदि_अन्तः // e820map.h
