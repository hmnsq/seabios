#ifndef __PIRTABLE_H
#define __PIRTABLE_H

#ضمن "types.h" // u32

بنية link_info {
    u8 link;
    u16 bitmap;
} PACKED;

بنية pir_slot {
    u8 bus;
    u8 dev;
    بنية link_info links[4];
    u8 slot_nr;
    u8 reserved;
} PACKED;

بنية pir_header {
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
    بنية pir_slot slots[0];
} PACKED;

#define PIR_SIGNATURE 0x52495024 // $PIR

#endif // pirtable.h
