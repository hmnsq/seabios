#اگر_غیر_تعریف_شدہ __CONFIG_H
#تعریف_کرو __CONFIG_H

#شامل "autoconf.h"

// Configuration definitions.

//#define BUILD_APPNAME  "QEMU"
//#define BUILD_CPUNAME8 "QEMUCPU "
//#define BUILD_APPNAME6 "QEMU  "
//#define BUILD_APPNAME4 "QEMU"
#تعریف_کرو BUILD_APPNAME  "Bochs"
#تعریف_کرو BUILD_CPUNAME8 "BOCHSCPU"
#تعریف_کرو BUILD_APPNAME6 "BOCHS "
#تعریف_کرو BUILD_APPNAME4 "BXPC"

// Maximum number of map entries in the e820 map
#تعریف_کرو BUILD_MAX_E820 128
// Space to reserve in high-memory for tables
#تعریف_کرو BUILD_MIN_HIGHTABLE (256*1024)
#تعریف_کرو BUILD_MAX_HIGHTABLE (16*1024*1024)
// Largest supported externaly facing drive id
#تعریف_کرو BUILD_MAX_EXTDRIVE 16
// Number of bytes the smbios may be and still live in the f-segment
#تعریف_کرو BUILD_MAX_SMBIOS_FSEG     600
// Maximum number of bytes the mptable may be and still be copied to f-segment
#تعریف_کرو BUILD_MAX_MPTABLE_FSEG    600

#تعریف_کرو BUILD_MODEL_ID      0xFC
#تعریف_کرو BUILD_SUBMODEL_ID   0x00
#تعریف_کرو BUILD_BIOS_REVISION 0x01

// Various memory addresses used by the code.
#تعریف_کرو BUILD_ڈھیر_ADDR          0x7000
#تعریف_کرو BUILD_S3_RESUME_ڈھیر_ADDR 0x1000
#تعریف_کرو BUILD_AP_BOOT_ADDR        0x10000
#تعریف_کرو BUILD_EBDA_MINIMUM        0x90000
#تعریف_کرو BUILD_LOWRAM_انتہا          0xa0000
#تعریف_کرو BUILD_ROM_آغاز           0xc0000
#تعریف_کرو BUILD_BIOS_ADDR           0xf0000
#تعریف_کرو BUILD_BIOS_سائز           0x10000
#تعریف_کرو BUILD_EXTRA_ڈھیر_سائز    0x800
#تعریف_کرو BUILD_SMM_INIT_ADDR       0x30000
#تعریف_کرو BUILD_SMM_ADDR            0xa0000

#تعریف_کرو BUILD_PCIMEM_آغاز        0xe0000000
#تعریف_کرو BUILD_PCIMEM_انتہا          0xfec00000    /* IOAPIC is mapped at */
#تعریف_کرو BUILD_PCIMEM64_آغاز      0x8000000000ULL
#تعریف_کرو BUILD_PCIMEM64_انتہا        0x10000000000ULL

#تعریف_کرو BUILD_IOAPIC_ADDR         0xfec00000
#تعریف_کرو BUILD_IOAPIC_ID           0
#تعریف_کرو BUILD_HPET_پتہ        0xfed00000
#تعریف_کرو BUILD_APIC_ADDR           0xfee00000

// PCI IRQS
#تعریف_کرو BUILD_PCI_IRQS            ((1<<5) | (1<<9) | (1<<10) | (1<<11))

// Important real-mode segments
#تعریف_کرو SEG_IVT      0x0000
#تعریف_کرو SEG_BDA      0x0040
#تعریف_کرو SEG_BIOS     0xf000

// Segment definitions in protected mode (see rombios32_gdt in misc.c)
#تعریف_کرو SEG32_MODE32_CS    (1 << 3)
#تعریف_کرو SEG32_MODE32_DS    (2 << 3)
#تعریف_کرو SEG32_MODE16_CS    (3 << 3)
#تعریف_کرو SEG32_MODE16_DS    (4 << 3)
#تعریف_کرو SEG32_MODE16BIG_CS (5 << 3)
#تعریف_کرو SEG32_MODE16BIG_DS (6 << 3)

// Debugging levels.  If non-zero and CONFIG_DEBUG_LEVEL is greater
// than the specified value, then the corresponding irq handler will
// report every enter event.
#تعریف_کرو DEBUG_ISR_02 1
#تعریف_کرو DEBUG_HDL_05 1
#تعریف_کرو DEBUG_ISR_08 20
#تعریف_کرو DEBUG_ISR_09 9
#تعریف_کرو DEBUG_ISR_0e 9
#تعریف_کرو DEBUG_HDL_11 2
#تعریف_کرو DEBUG_HDL_12 2
#تعریف_کرو DEBUG_HDL_13 10
#تعریف_کرو DEBUG_HDL_14 2
#تعریف_کرو DEBUG_HDL_15 9
#تعریف_کرو DEBUG_HDL_16 9
#تعریف_کرو DEBUG_HDL_17 2
#تعریف_کرو DEBUG_HDL_18 1
#تعریف_کرو DEBUG_HDL_19 1
#تعریف_کرو DEBUG_HDL_1a 9
#تعریف_کرو DEBUG_HDL_40 1
#تعریف_کرو DEBUG_ISR_70 9
#تعریف_کرو DEBUG_ISR_74 9
#تعریف_کرو DEBUG_ISR_75 1
#تعریف_کرو DEBUG_ISR_76 10
#تعریف_کرو DEBUG_ISR_hwpic1 5
#تعریف_کرو DEBUG_ISR_hwpic2 5
#تعریف_کرو DEBUG_HDL_smi 9
#تعریف_کرو DEBUG_HDL_smp 1
#تعریف_کرو DEBUG_HDL_pnp 1
#تعریف_کرو DEBUG_HDL_pmm 1
#تعریف_کرو DEBUG_HDL_pcibios 9
#تعریف_کرو DEBUG_HDL_apm 9

#تعریف_کرو DEBUG_unimplemented 2
#تعریف_کرو DEBUG_invalid 3
#تعریف_کرو DEBUG_thread 2
#تعریف_کرو DEBUG_tcg 20

#اگر_ختم // config.h
