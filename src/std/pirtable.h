#اگر_غیر_تعریف_شدہ __PIRTABLE_H
#تعریف_کرو __PIRTABLE_H

#شامل "types.h" // u32

ساخت link_info {
    u8 link;
    u16 bitmap;
} PACKED;

ساخت pir_خانہ {
    u8 bus;
    u8 dev;
    ساخت link_info links[4];
    u8 خانہ_nr;
    u8 reserved;
} PACKED;

ساخت pir_سرخی {
    u32 signature;
    u16 نسخہ_2;
    u16 سائز;
    u8 رہنما_آلہ_bus;
    u8 رہنما_آلہ_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_ڈیٹا;
    u8 reserved[11];
    u8 checksum;
    ساخت pir_خانہ slots[0];
} PACKED;

#تعریف_کرو PIR_SIGNATURE 0x52495024 // $PIR

#اگر_ختم // pirtable.h
