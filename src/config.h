#که_نه_تعریف_شوی __CONFIG_H
#تعریف_کړه __CONFIG_H

#ضمن "autoconf.h"

// Configuration definitions.

//#define BUILD_APPNAME  "QEMU"
//#define BUILD_CPUNAME8 "QEMUCPU "
//#define BUILD_APPNAME6 "QEMU  "
//#define BUILD_APPNAME4 "QEMU"
#تعریف_کړه BUILD_APPNAME  "Bochs"
#تعریف_کړه BUILD_CPUNAME8 "BOCHSCPU"
#تعریف_کړه BUILD_APPNAME6 "BOCHS "
#تعریف_کړه BUILD_APPNAME4 "BXPC"

// Maximum number of map entries in the e820 map
#تعریف_کړه BUILD_MAX_E820 128
// Space to reserve in high-memory for tables
#تعریف_کړه BUILD_MIN_HIGHTABLE (256*1024)
#تعریف_کړه BUILD_MAX_HIGHTABLE (16*1024*1024)
// Largest supported externaly facing drive id
#تعریف_کړه BUILD_MAX_EXTDRIVE 16
// Number of bytes the smbios may be and still live in the f-segment
#تعریف_کړه BUILD_MAX_SMBIOS_FSEG     600
// Maximum number of bytes the mptable may be and still be copied to f-segment
#تعریف_کړه BUILD_MAX_MPTABLE_FSEG    600

#تعریف_کړه BUILD_MODEL_ID      0xFC
#تعریف_کړه BUILD_SUBMODEL_ID   0x00
#تعریف_کړه BUILD_BIOS_REVISION 0x01

// Various memory addresses used by the code.
#تعریف_کړه BUILD_ډېری_ADDR          0x7000
#تعریف_کړه BUILD_S3_RESUME_ډېری_ADDR 0x1000
#تعریف_کړه BUILD_AP_BOOT_ADDR        0x10000
#تعریف_کړه BUILD_EBDA_MINIMUM        0x90000
#تعریف_کړه BUILD_LOWRAM_پای_ټکی          0xa0000
#تعریف_کړه BUILD_ROM_پیل           0xc0000
#تعریف_کړه BUILD_BIOS_ADDR           0xf0000
#تعریف_کړه BUILD_BIOS_اندازه           0x10000
#تعریف_کړه BUILD_EXTRA_ډېری_اندازه    0x800
#تعریف_کړه BUILD_SMM_INIT_ADDR       0x30000
#تعریف_کړه BUILD_SMM_ADDR            0xa0000

#تعریف_کړه BUILD_PCIMEM_پیل        0xe0000000
#تعریف_کړه BUILD_PCIMEM_پای_ټکی          0xfec00000    /* IOAPIC is mapped at */
#تعریف_کړه BUILD_PCIMEM64_پیل      0x8000000000ULL
#تعریف_کړه BUILD_PCIMEM64_پای_ټکی        0x10000000000ULL

#تعریف_کړه BUILD_IOAPIC_ADDR         0xfec00000
#تعریف_کړه BUILD_IOAPIC_ID           0
#تعریف_کړه BUILD_HPET_پته        0xfed00000
#تعریف_کړه BUILD_APIC_ADDR           0xfee00000

// PCI IRQS
#تعریف_کړه BUILD_PCI_IRQS            ((1<<5) | (1<<9) | (1<<10) | (1<<11))

// Important real-mode segments
#تعریف_کړه SEG_IVT      0x0000
#تعریف_کړه SEG_BDA      0x0040
#تعریف_کړه SEG_BIOS     0xf000

// Segment definitions in protected mode (see rombios32_gdt in misc.c)
#تعریف_کړه SEG32_MODE32_CS    (1 << 3)
#تعریف_کړه SEG32_MODE32_DS    (2 << 3)
#تعریف_کړه SEG32_MODE16_CS    (3 << 3)
#تعریف_کړه SEG32_MODE16_DS    (4 << 3)
#تعریف_کړه SEG32_MODE16BIG_CS (5 << 3)
#تعریف_کړه SEG32_MODE16BIG_DS (6 << 3)

// Debugging levels.  If non-zero and CONFIG_DEBUG_LEVEL is greater
// than the specified value, then the corresponding irq handler will
// report every enter event.
#تعریف_کړه DEBUG_ISR_02 1
#تعریف_کړه DEBUG_HDL_05 1
#تعریف_کړه DEBUG_ISR_08 20
#تعریف_کړه DEBUG_ISR_09 9
#تعریف_کړه DEBUG_ISR_0e 9
#تعریف_کړه DEBUG_HDL_11 2
#تعریف_کړه DEBUG_HDL_12 2
#تعریف_کړه DEBUG_HDL_13 10
#تعریف_کړه DEBUG_HDL_14 2
#تعریف_کړه DEBUG_HDL_15 9
#تعریف_کړه DEBUG_HDL_16 9
#تعریف_کړه DEBUG_HDL_17 2
#تعریف_کړه DEBUG_HDL_18 1
#تعریف_کړه DEBUG_HDL_19 1
#تعریف_کړه DEBUG_HDL_1a 9
#تعریف_کړه DEBUG_HDL_40 1
#تعریف_کړه DEBUG_ISR_70 9
#تعریف_کړه DEBUG_ISR_74 9
#تعریف_کړه DEBUG_ISR_75 1
#تعریف_کړه DEBUG_ISR_76 10
#تعریف_کړه DEBUG_ISR_hwpic1 5
#تعریف_کړه DEBUG_ISR_hwpic2 5
#تعریف_کړه DEBUG_HDL_smi 9
#تعریف_کړه DEBUG_HDL_smp 1
#تعریف_کړه DEBUG_HDL_pnp 1
#تعریف_کړه DEBUG_HDL_pmm 1
#تعریف_کړه DEBUG_HDL_pcibios 9
#تعریف_کړه DEBUG_HDL_apm 9

#تعریف_کړه DEBUG_unimplemented 2
#تعریف_کړه DEBUG_invalid 3
#تعریف_کړه DEBUG_thread 2
#تعریف_کړه DEBUG_tcg 20

#د_که_پای // config.h
