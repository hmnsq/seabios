#اگر_تعریف_نشده __PIRTABLE_H
#تعریف_کن __PIRTABLE_H

#ضمن "types.h" // u32

بنية link_info {
    u8 link;
    u16 bitmap;
} PACKED;

بنية pir_جایگاه {
    u8 bus;
    u8 dev;
    بنية link_info links[4];
    u8 جایگاه_nr;
    u8 reserved;
} PACKED;

بنية pir_سرآیند {
    u32 signature;
    u16 نسخه_2;
    u16 اندازه;
    u8 مسیریاب_bus;
    u8 مسیریاب_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_داده;
    u8 reserved[11];
    u8 checksum;
    بنية pir_جایگاه slots[0];
} PACKED;

#تعریف_کن PIR_SIGNATURE 0x52495024 // $PIR

#پایان_اگر // pirtable.h
