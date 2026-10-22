#که_نه_تعریف_شوی __PIRTABLE_H
#تعریف_کړه __PIRTABLE_H

#ضمن "types.h" // u32

بنية link_info {
    u8 link;
    u16 bitmap;
} PACKED;

بنية pir_خونه {
    u8 bus;
    u8 dev;
    بنية link_info links[4];
    u8 خونه_nr;
    u8 reserved;
} PACKED;

بنية pir_سرلیک {
    u32 signature;
    u16 نسخه_2;
    u16 اندازه;
    u8 لارښود_bus;
    u8 لارښود_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_معلومات;
    u8 reserved[11];
    u8 checksum;
    بنية pir_خونه slots[0];
} PACKED;

#تعریف_کړه PIR_SIGNATURE 0x52495024 // $PIR

#د_که_پای // pirtable.h
