#अगर_अघोषित __E820MAP_H
#घोषणा __E820MAP_H

#समावेश "types.h" // u64

#घोषणा E820_RAM            1
#घोषणा E820_RESERVED       2
#घोषणा E820_ACPI           3
#घोषणा E820_NVS            4
#घोषणा E820_UNUSABLE       5
#घोषणा E820_SOFT_RESERVED  0xEFFFFFFF

काष्ठा e820entry {
    u64 start;
    u64 आकार;
    u32 type;
};

व्योम e820_add(u64 start, u64 आकार, u32 type);
व्योम e820_remove(u64 start, u64 आकार);
व्योम e820_prepboot(व्योम);
पूर्णांक e820_is_used(u64 start, u64 आकार);

// e820 map storage
बाह्य काष्ठा e820entry e820_list[];
बाह्य पूर्णांक e820_गिनती;

#पूर्ण_अगर // e820map.h
