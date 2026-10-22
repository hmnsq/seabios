#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __OPTIONROMS_H
#ವ್ಯಾಖ್ಯಾನಿಸು __OPTIONROMS_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#ವ್ಯಾಖ್ಯಾನಿಸು OPTION_ROM_SIGNATURE 0xaa55

ರಚನೆ rom_ಶೀರ್ಷಿಕೆ {
    u16 signature;
    u8 ಗಾತ್ರ_2;
    u8 init_ಸದಿಶ[4];
    u8 reserved[17];
    u16 pcioffset;
    u16 pnpoffset;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ROM_SIGNATURE 0x52494350 // "PCIR"

ರಚನೆ pci_ದತ್ತಾಂಶ {
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

ರಚನೆ pnp_ದತ್ತಾಂಶ {
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

#ವ್ಯಾಖ್ಯಾನಿಸು OPTION_ROM_ALIGN 2048
#ವ್ಯಾಖ್ಯಾನಿಸು OPTION_ROM_INITVECTOR offsetof(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ, init_ಸದಿಶ[0])
#ವ್ಯಾಖ್ಯಾನಿಸು PCIROM_CODETYPE_X86 0

#ಆದರೆ_ಅಂತ್ಯ
