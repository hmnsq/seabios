#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __CONFIG_H
#ವ್ಯಾಖ್ಯಾನಿಸು __CONFIG_H

#ಅನ್ತರ್ಭಾವ "autoconf.h"

// Configuration definitions.

//#define BUILD_APPNAME  "QEMU"
//#define BUILD_CPUNAME8 "QEMUCPU "
//#define BUILD_APPNAME6 "QEMU  "
//#define BUILD_APPNAME4 "QEMU"
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_APPNAME  "Bochs"
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_CPUNAME8 "BOCHSCPU"
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_APPNAME6 "BOCHS "
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_APPNAME4 "BXPC"

// Maximum number of map entries in the e820 map
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_MAX_E820 128
// Space to reserve in high-memory for tables
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_MIN_HIGHTABLE (256*1024)
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_MAX_HIGHTABLE (16*1024*1024)
// Largest supported externaly facing drive id
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_MAX_EXTDRIVE 16
// Number of bytes the smbios may be and still live in the f-segment
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_MAX_SMBIOS_FSEG     600
// Maximum number of bytes the mptable may be and still be copied to f-segment
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_MAX_MPTABLE_FSEG    600

#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_MODEL_ID      0xFC
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_SUBMODEL_ID   0x00
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_BIOS_REVISION 0x01

// Various memory addresses used by the code.
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_ರಾಶಿ_ADDR          0x7000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_S3_RESUME_ರಾಶಿ_ADDR 0x1000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_AP_BOOT_ADDR        0x10000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_EBDA_MINIMUM        0x90000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_LOWRAM_ಕೊನೆ          0xa0000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_ROM_ಆರಂಭ           0xc0000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_BIOS_ADDR           0xf0000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_BIOS_ಗಾತ್ರ           0x10000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_EXTRA_ರಾಶಿ_ಗಾತ್ರ    0x800
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_SMM_INIT_ADDR       0x30000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_SMM_ADDR            0xa0000

#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_PCIMEM_ಆರಂಭ        0xe0000000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_PCIMEM_ಕೊನೆ          0xfec00000    /* IOAPIC is mapped at */
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_PCIMEM64_ಆರಂಭ      0x8000000000ULL
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_PCIMEM64_ಕೊನೆ        0x10000000000ULL

#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_IOAPIC_ADDR         0xfec00000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_IOAPIC_ID           0
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_HPET_ವಿಳಾಸ        0xfed00000
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_APIC_ADDR           0xfee00000

// PCI IRQS
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_PCI_IRQS            ((1<<5) | (1<<9) | (1<<10) | (1<<11))

// Important real-mode segments
#ವ್ಯಾಖ್ಯಾನಿಸು SEG_IVT      0x0000
#ವ್ಯಾಖ್ಯಾನಿಸು SEG_BDA      0x0040
#ವ್ಯಾಖ್ಯಾನಿಸು SEG_BIOS     0xf000

// Segment definitions in protected mode (see rombios32_gdt in misc.c)
#ವ್ಯಾಖ್ಯಾನಿಸು SEG32_MODE32_CS    (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು SEG32_MODE32_DS    (2 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು SEG32_MODE16_CS    (3 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು SEG32_MODE16_DS    (4 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು SEG32_MODE16BIG_CS (5 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು SEG32_MODE16BIG_DS (6 << 3)

// Debugging levels.  If non-zero and CONFIG_DEBUG_LEVEL is greater
// than the specified value, then the corresponding irq handler will
// report every enter event.
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_02 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_05 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_08 20
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_09 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_0e 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_11 2
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_12 2
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_13 10
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_14 2
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_15 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_16 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_17 2
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_18 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_19 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_1a 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_40 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_70 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_74 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_75 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_76 10
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_hwpic1 5
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_ISR_hwpic2 5
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_smi 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_smp 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_pnp 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_pmm 1
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_pcibios 9
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_HDL_apm 9

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_unimplemented 2
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_invalid 3
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_thread 2
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_tcg 20

#ಆದರೆ_ಅಂತ್ಯ // config.h
