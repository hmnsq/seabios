#جيڪڏهن_وصف_نه_ٿيل __PIRTABLE_H
#وصف_ڪريو __PIRTABLE_H

#ضمن "types.h" // u32

بنية link_info {
    u8 link;
    u16 bitmap;
} PACKED;

بنية pir_جڳهه {
    u8 bus;
    u8 dev;
    بنية link_info links[4];
    u8 جڳهه_nr;
    u8 reserved;
} PACKED;

بنية pir_سرو {
    u32 signature;
    u16 نسخو_2;
    u16 سائيز;
    u8 رائوٽر_bus;
    u8 رائوٽر_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_معلومات;
    u8 reserved[11];
    u8 checksum;
    بنية pir_جڳهه slots[0];
} PACKED;

#وصف_ڪريو PIR_SIGNATURE 0x52495024 // $PIR

#جيڪڏهن_پورو // pirtable.h
