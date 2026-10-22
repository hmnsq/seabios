#यदि_अपरिभाषितम् __OPTIONROMS_H
#परिभाषय __OPTIONROMS_H

#अन्तर्भाव "types.h" // u32

#परिभाषय OPTION_ROM_SIGNATURE 0xaa55

संरचना rom_शीर्षकम् {
    u16 signature;
    u8 आकार;
    u8 init_सदिशम्[4];
    u8 reserved[17];
    u16 pcioffset;
    u16 pnpoffset;
} PACKED;

#परिभाषय PCI_ROM_SIGNATURE 0x52494350 // "PCIR"

संरचना pci_दत्त {
    u32 signature;
    u16 vendor;
    u16 device;
    u16 vitaldata;
    u16 dlen;
    u8 drevision;
    u8 class_lo;
    u16 class_hi;
    u16 ilen;
    u16 irevision;
    u8 type;
    u8 indicator;
    u16 reserved;
} PACKED;

संरचना pnp_दत्त {
    u32 signature;
    u8 revision;
    u8 len;
    u16 nextoffset;
    u8 reserved_08;
    u8 checksum;
    u32 devid;
    u16 manufacturer;
    u16 productname;
    u8 type_lo;
    u16 type_hi;
    u8 dev_flags;
    u16 bcv;
    u16 dv;
    u16 bev;
    u16 reserved_1c;
    u16 staticresource;
} PACKED;

#परिभाषय OPTION_ROM_ALIGN 2048
#परिभाषय OPTION_ROM_INITVECTOR offsetof(संरचना rom_शीर्षकम्, init_सदिशम्[0])
#परिभाषय PCIROM_CODETYPE_X86 0

#यदि_अन्तः
