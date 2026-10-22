#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PIRTABLE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PIRTABLE_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

ਬਣਤਰ link_info {
    u8 link;
    u16 bitmap;
} PACKED;

ਬਣਤਰ pir_ਥਾਂ {
    u8 bus;
    u8 dev;
    ਬਣਤਰ link_info links[4];
    u8 ਥਾਂ_nr;
    u8 reserved;
} PACKED;

ਬਣਤਰ pir_ਸਿਰਲੇਖ {
    u32 signature;
    u16 ਸੰਸਕਰਣ_2;
    u16 ਮਾਪ;
    u8 ਰਾਊਟਰ_bus;
    u8 ਰਾਊਟਰ_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_ਅੰਕੜੇ;
    u8 reserved[11];
    u8 checksum;
    ਬਣਤਰ pir_ਥਾਂ slots[0];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIR_SIGNATURE 0x52495024 // $PIR

#ਜੇ_ਅੰਤ // pirtable.h
