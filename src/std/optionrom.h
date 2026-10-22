#اگر_تعریف_نشده __OPTIONROMS_H
#تعریف_کن __OPTIONROMS_H

#ضمن "types.h" // u32

#تعریف_کن OPTION_ROM_SIGNATURE 0xaa55

بنية rom_سرآیند {
    u16 signature;
    u8 اندازه;
    u8 init_وکتور[4];
    u8 reserved[17];
    u16 pcioffset;
    u16 pnpoffset;
} PACKED;

#تعریف_کن PCI_ROM_SIGNATURE 0x52494350 // "PCIR"

بنية pci_داده {
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

بنية pnp_داده {
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

#تعریف_کن OPTION_ROM_ALIGN 2048
#تعریف_کن OPTION_ROM_INITVECTOR offsetof(بنية rom_سرآیند, init_وکتور[0])
#تعریف_کن PCIROM_CODETYPE_X86 0

#پایان_اگر
