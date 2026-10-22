#ifndef __PIRTABLE_H
#define __PIRTABLE_H

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

संरचना pir_header {
    u32 signature;
    u16 version;
    u16 size;
    u8 router_bus;
    u8 router_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_data;
    u8 reserved[11];
    u8 checksum;
    संरचना pir_slot slots[0];
} PACKED;

#define PIR_SIGNATURE 0x52495024 // $PIR

#endif // pirtable.h
