#جے_بنا_تعریف __PIRTABLE_H
#تعریف_کرو __PIRTABLE_H

#ضمن "types.h" // u32

بنية link_info {
    u8 link;
    u16 bitmap;
} PACKED;

بنية pir_تھاں {
    u8 bus;
    u8 dev;
    بنية link_info links[4];
    u8 تھاں_nr;
    u8 reserved;
} PACKED;

بنية pir_سرخی {
    u32 signature;
    u16 نسخہ_2;
    u16 ناپ;
    u8 راؤٹر_bus;
    u8 راؤٹر_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_انکڑے;
    u8 reserved[11];
    u8 checksum;
    بنية pir_تھاں slots[0];
} PACKED;

#تعریف_کرو PIR_SIGNATURE 0x52495024 // $PIR

#جے_انت // pirtable.h
