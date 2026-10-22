#నిర్వచితం_కాకపోతే __OPTIONROMS_H
#నిర్వచించు __OPTIONROMS_H

#అన్తర్భావ "types.h" // u32

#నిర్వచించు OPTION_ROM_SIGNATURE 0xaa55

నిర్మాణం rom_శీర్షిక {
    u16 signature;
    u8 కొలత;
    u8 init_సదిశ[4];
    u8 reserved[17];
    u16 pcioffset;
    u16 pnpoffset;
} PACKED;

#నిర్వచించు PCI_ROM_SIGNATURE 0x52494350 // "PCIR"

నిర్మాణం pci_దత్తాంశం {
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

నిర్మాణం pnp_దత్తాంశం {
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

#నిర్వచించు OPTION_ROM_ALIGN 2048
#నిర్వచించు OPTION_ROM_INITVECTOR offsetof(నిర్మాణం rom_శీర్షిక, init_సదిశ[0])
#నిర్వచించు PCIROM_CODETYPE_X86 0

#అయితే_ముగింపు
