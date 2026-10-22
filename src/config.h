#यदि_अपरिभाषितम् __CONFIG_H
#परिभाषय __CONFIG_H

#अन्तर्भाव "autoconf.h"

// Configuration definitions.

//#define BUILD_APPNAME  "QEMU"
//#define BUILD_CPUNAME8 "QEMUCPU "
//#define BUILD_APPNAME6 "QEMU  "
//#define BUILD_APPNAME4 "QEMU"
#परिभाषय BUILD_APPNAME  "Bochs"
#परिभाषय BUILD_CPUNAME8 "BOCHSCPU"
#परिभाषय BUILD_APPNAME6 "BOCHS "
#परिभाषय BUILD_APPNAME4 "BXPC"

// Maximum number of map entries in the e820 map
#परिभाषय BUILD_MAX_E820 128
// Space to reserve in high-memory for tables
#परिभाषय BUILD_MIN_HIGHTABLE (256*1024)
#परिभाषय BUILD_MAX_HIGHTABLE (16*1024*1024)
// Largest supported externaly facing drive id
#परिभाषय BUILD_MAX_EXTDRIVE 16
// Number of bytes the smbios may be and still live in the f-segment
#परिभाषय BUILD_MAX_SMBIOS_FSEG     600
// Maximum number of bytes the mptable may be and still be copied to f-segment
#परिभाषय BUILD_MAX_MPTABLE_FSEG    600

#परिभाषय BUILD_MODEL_ID      0xFC
#परिभाषय BUILD_SUBMODEL_ID   0x00
#परिभाषय BUILD_BIOS_REVISION 0x01

// Various memory addresses used by the code.
#परिभाषय BUILD_राशिः_ADDR          0x7000
#परिभाषय BUILD_S3_RESUME_राशिः_ADDR 0x1000
#परिभाषय BUILD_AP_BOOT_ADDR        0x10000
#परिभाषय BUILD_EBDA_MINIMUM        0x90000
#परिभाषय BUILD_LOWRAM_अन्तः          0xa0000
#परिभाषय BUILD_ROM_START           0xc0000
#परिभाषय BUILD_BIOS_ADDR           0xf0000
#परिभाषय BUILD_BIOS_आकार           0x10000
#परिभाषय BUILD_EXTRA_राशिः_आकार    0x800
#परिभाषय BUILD_SMM_INIT_ADDR       0x30000
#परिभाषय BUILD_SMM_ADDR            0xa0000

#परिभाषय BUILD_PCIMEM_START        0xe0000000
#परिभाषय BUILD_PCIMEM_अन्तः          0xfec00000    /* IOAPIC is mapped at */
#परिभाषय BUILD_PCIMEM64_START      0x8000000000ULL
#परिभाषय BUILD_PCIMEM64_अन्तः        0x10000000000ULL

#परिभाषय BUILD_IOAPIC_ADDR         0xfec00000
#परिभाषय BUILD_IOAPIC_ID           0
#परिभाषय BUILD_HPET_स्थानाङ्कः        0xfed00000
#परिभाषय BUILD_APIC_ADDR           0xfee00000

// PCI IRQS
#परिभाषय BUILD_PCI_IRQS            ((1<<5) | (1<<9) | (1<<10) | (1<<11))

// Important real-mode segments
#परिभाषय SEG_IVT      0x0000
#परिभाषय SEG_BDA      0x0040
#परिभाषय SEG_BIOS     0xf000

// Segment definitions in protected mode (see rombios32_gdt in misc.c)
#परिभाषय SEG32_MODE32_CS    (1 << 3)
#परिभाषय SEG32_MODE32_DS    (2 << 3)
#परिभाषय SEG32_MODE16_CS    (3 << 3)
#परिभाषय SEG32_MODE16_DS    (4 << 3)
#परिभाषय SEG32_MODE16BIG_CS (5 << 3)
#परिभाषय SEG32_MODE16BIG_DS (6 << 3)

// Debugging levels.  If non-zero and CONFIG_DEBUG_LEVEL is greater
// than the specified value, then the corresponding irq handler will
// report every enter event.
#परिभाषय DEBUG_ISR_02 1
#परिभाषय DEBUG_HDL_05 1
#परिभाषय DEBUG_ISR_08 20
#परिभाषय DEBUG_ISR_09 9
#परिभाषय DEBUG_ISR_0e 9
#परिभाषय DEBUG_HDL_11 2
#परिभाषय DEBUG_HDL_12 2
#परिभाषय DEBUG_HDL_13 10
#परिभाषय DEBUG_HDL_14 2
#परिभाषय DEBUG_HDL_15 9
#परिभाषय DEBUG_HDL_16 9
#परिभाषय DEBUG_HDL_17 2
#परिभाषय DEBUG_HDL_18 1
#परिभाषय DEBUG_HDL_19 1
#परिभाषय DEBUG_HDL_1a 9
#परिभाषय DEBUG_HDL_40 1
#परिभाषय DEBUG_ISR_70 9
#परिभाषय DEBUG_ISR_74 9
#परिभाषय DEBUG_ISR_75 1
#परिभाषय DEBUG_ISR_76 10
#परिभाषय DEBUG_ISR_hwpic1 5
#परिभाषय DEBUG_ISR_hwpic2 5
#परिभाषय DEBUG_HDL_smi 9
#परिभाषय DEBUG_HDL_smp 1
#परिभाषय DEBUG_HDL_pnp 1
#परिभाषय DEBUG_HDL_pmm 1
#परिभाषय DEBUG_HDL_pcibios 9
#परिभाषय DEBUG_HDL_apm 9

#परिभाषय DEBUG_unimplemented 2
#परिभाषय DEBUG_invalid 3
#परिभाषय DEBUG_thread 2
#परिभाषय DEBUG_tcg 20

#यदि_अन्तः // config.h
