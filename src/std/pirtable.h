#यदि_अपरिभाषितम् __PIRTABLE_H
#परिभाषय __PIRTABLE_H

#अन्तर्भाव "types.h" // u32

संरचना link_info {
    u8 link;
    u16 bitmap;
} PACKED;

संरचना pir_slot {
    u8 bus;
    u8 dev;
    संरचना link_info links[4];
    u8 slot_nr;
    u8 reserved;
} PACKED;

संरचना pir_शीर्षकम् {
    u32 signature;
    u16 संस्करणम्_2;
    u16 आकार;
    u8 मार्गदर्शकः_bus;
    u8 मार्गदर्शकः_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_दत्त;
    u8 reserved[11];
    u8 checksum;
    संरचना pir_slot slots[0];
} PACKED;

#परिभाषय PIR_SIGNATURE 0x52495024 // $PIR

#यदि_अन्तः // pirtable.h
