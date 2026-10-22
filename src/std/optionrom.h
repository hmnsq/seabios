#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __OPTIONROMS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __OPTIONROMS_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPTION_ROM_SIGNATURE 0xaa55

ਬਣਤਰ rom_ਸਿਰਲੇਖ {
    u16 signature;
    u8 ਮਾਪ;
    u8 init_ਸਦਿਸ਼[4];
    u8 reserved[17];
    u16 pcioffset;
    u16 pnpoffset;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ROM_SIGNATURE 0x52494350 // "PCIR"

ਬਣਤਰ pci_ਅੰਕੜੇ {
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

ਬਣਤਰ pnp_ਅੰਕੜੇ {
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPTION_ROM_ALIGN 2048
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPTION_ROM_INITVECTOR offsetof(ਬਣਤਰ rom_ਸਿਰਲੇਖ, init_ਸਦਿਸ਼[0])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCIROM_CODETYPE_X86 0

#ਜੇ_ਅੰਤ
