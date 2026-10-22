#अगर_अघोषित __PIRTABLE_H
#घोषणा __PIRTABLE_H

#समावेश "types.h" // u32

काष्ठा link_info {
    u8 link;
    u16 bitmap;
} PACKED;

काष्ठा pir_slot {
    u8 bus;
    u8 dev;
    काष्ठा link_info links[4];
    u8 slot_nr;
    u8 reserved;
} PACKED;

काष्ठा pir_header {
    u32 signature;
    u16 version;
    u16 आकार;
    u8 router_bus;
    u8 router_devfunc;
    u16 exclusive_irqs;
    u32 compatible_devid;
    u32 miniport_तथ्य;
    u8 reserved[11];
    u8 checksum;
    काष्ठा pir_slot slots[0];
} PACKED;

#घोषणा PIR_SIGNATURE 0x52495024 // $PIR

#पूर्ण_अगर // pirtable.h
