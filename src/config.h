#جيڪڏهن_وصف_نه_ٿيل __CONFIG_H
#وصف_ڪريو __CONFIG_H

#ضمن "autoconf.h"

// Configuration definitions.

//#define BUILD_APPNAME  "QEMU"
//#define BUILD_CPUNAME8 "QEMUCPU "
//#define BUILD_APPNAME6 "QEMU  "
//#define BUILD_APPNAME4 "QEMU"
#وصف_ڪريو BUILD_APPNAME  "Bochs"
#وصف_ڪريو BUILD_CPUNAME8 "BOCHSCPU"
#وصف_ڪريو BUILD_APPNAME6 "BOCHS "
#وصف_ڪريو BUILD_APPNAME4 "BXPC"

// Maximum number of map entries in the e820 map
#وصف_ڪريو BUILD_MAX_E820 128
// Space to reserve in high-memory for tables
#وصف_ڪريو BUILD_MIN_HIGHTABLE (256*1024)
#وصف_ڪريو BUILD_MAX_HIGHTABLE (16*1024*1024)
// Largest supported externaly facing drive id
#وصف_ڪريو BUILD_MAX_EXTDRIVE 16
// Number of bytes the smbios may be and still live in the f-segment
#وصف_ڪريو BUILD_MAX_SMBIOS_FSEG     600
// Maximum number of bytes the mptable may be and still be copied to f-segment
#وصف_ڪريو BUILD_MAX_MPTABLE_FSEG    600

#وصف_ڪريو BUILD_MODEL_ID      0xFC
#وصف_ڪريو BUILD_SUBMODEL_ID   0x00
#وصف_ڪريو BUILD_BIOS_REVISION 0x01

// Various memory addresses used by the code.
#وصف_ڪريو BUILD_ڍير_ADDR          0x7000
#وصف_ڪريو BUILD_S3_RESUME_ڍير_ADDR 0x1000
#وصف_ڪريو BUILD_AP_BOOT_ADDR        0x10000
#وصف_ڪريو BUILD_EBDA_MINIMUM        0x90000
#وصف_ڪريو BUILD_LOWRAM_پڇاڙي          0xa0000
#وصف_ڪريو BUILD_ROM_شروعات           0xc0000
#وصف_ڪريو BUILD_BIOS_ADDR           0xf0000
#وصف_ڪريو BUILD_BIOS_سائيز           0x10000
#وصف_ڪريو BUILD_EXTRA_ڍير_سائيز    0x800
#وصف_ڪريو BUILD_SMM_INIT_ADDR       0x30000
#وصف_ڪريو BUILD_SMM_ADDR            0xa0000

#وصف_ڪريو BUILD_PCIMEM_شروعات        0xe0000000
#وصف_ڪريو BUILD_PCIMEM_پڇاڙي          0xfec00000    /* IOAPIC is mapped at */
#وصف_ڪريو BUILD_PCIMEM64_شروعات      0x8000000000ULL
#وصف_ڪريو BUILD_PCIMEM64_پڇاڙي        0x10000000000ULL

#وصف_ڪريو BUILD_IOAPIC_ADDR         0xfec00000
#وصف_ڪريو BUILD_IOAPIC_ID           0
#وصف_ڪريو BUILD_HPET_پتو        0xfed00000
#وصف_ڪريو BUILD_APIC_ADDR           0xfee00000

// PCI IRQS
#وصف_ڪريو BUILD_PCI_IRQS            ((1<<5) | (1<<9) | (1<<10) | (1<<11))

// Important real-mode segments
#وصف_ڪريو SEG_IVT      0x0000
#وصف_ڪريو SEG_BDA      0x0040
#وصف_ڪريو SEG_BIOS     0xf000

// Segment definitions in protected mode (see rombios32_gdt in misc.c)
#وصف_ڪريو SEG32_MODE32_CS    (1 << 3)
#وصف_ڪريو SEG32_MODE32_DS    (2 << 3)
#وصف_ڪريو SEG32_MODE16_CS    (3 << 3)
#وصف_ڪريو SEG32_MODE16_DS    (4 << 3)
#وصف_ڪريو SEG32_MODE16BIG_CS (5 << 3)
#وصف_ڪريو SEG32_MODE16BIG_DS (6 << 3)

// Debugging levels.  If non-zero and CONFIG_DEBUG_LEVEL is greater
// than the specified value, then the corresponding irq handler will
// report every enter event.
#وصف_ڪريو DEBUG_ISR_02 1
#وصف_ڪريو DEBUG_HDL_05 1
#وصف_ڪريو DEBUG_ISR_08 20
#وصف_ڪريو DEBUG_ISR_09 9
#وصف_ڪريو DEBUG_ISR_0e 9
#وصف_ڪريو DEBUG_HDL_11 2
#وصف_ڪريو DEBUG_HDL_12 2
#وصف_ڪريو DEBUG_HDL_13 10
#وصف_ڪريو DEBUG_HDL_14 2
#وصف_ڪريو DEBUG_HDL_15 9
#وصف_ڪريو DEBUG_HDL_16 9
#وصف_ڪريو DEBUG_HDL_17 2
#وصف_ڪريو DEBUG_HDL_18 1
#وصف_ڪريو DEBUG_HDL_19 1
#وصف_ڪريو DEBUG_HDL_1a 9
#وصف_ڪريو DEBUG_HDL_40 1
#وصف_ڪريو DEBUG_ISR_70 9
#وصف_ڪريو DEBUG_ISR_74 9
#وصف_ڪريو DEBUG_ISR_75 1
#وصف_ڪريو DEBUG_ISR_76 10
#وصف_ڪريو DEBUG_ISR_hwpic1 5
#وصف_ڪريو DEBUG_ISR_hwpic2 5
#وصف_ڪريو DEBUG_HDL_smi 9
#وصف_ڪريو DEBUG_HDL_smp 1
#وصف_ڪريو DEBUG_HDL_pnp 1
#وصف_ڪريو DEBUG_HDL_pmm 1
#وصف_ڪريو DEBUG_HDL_pcibios 9
#وصف_ڪريو DEBUG_HDL_apm 9

#وصف_ڪريو DEBUG_unimplemented 2
#وصف_ڪريو DEBUG_invalid 3
#وصف_ڪريو DEBUG_thread 2
#وصف_ڪريو DEBUG_tcg 20

#جيڪڏهن_پورو // config.h
