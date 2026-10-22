#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __CONFIG_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __CONFIG_H

#ਅਨ੍ਤਰ੍ਭਾਵ "autoconf.h"

// Configuration definitions.

//#define BUILD_APPNAME  "QEMU"
//#define BUILD_CPUNAME8 "QEMUCPU "
//#define BUILD_APPNAME6 "QEMU  "
//#define BUILD_APPNAME4 "QEMU"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_APPNAME  "Bochs"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_CPUNAME8 "BOCHSCPU"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_APPNAME6 "BOCHS "
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_APPNAME4 "BXPC"

// Maximum number of map entries in the e820 map
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_MAX_E820 128
// Space to reserve in high-memory for tables
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_MIN_HIGHTABLE (256*1024)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_MAX_HIGHTABLE (16*1024*1024)
// Largest supported externaly facing drive id
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_MAX_EXTDRIVE 16
// Number of bytes the smbios may be and still live in the f-segment
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_MAX_SMBIOS_FSEG     600
// Maximum number of bytes the mptable may be and still be copied to f-segment
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_MAX_MPTABLE_FSEG    600

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_MODEL_ID      0xFC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_SUBMODEL_ID   0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_BIOS_REVISION 0x01

// Various memory addresses used by the code.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_ਢੇਰ_ADDR          0x7000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_S3_RESUME_ਢੇਰ_ADDR 0x1000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_AP_BOOT_ADDR        0x10000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_EBDA_MINIMUM        0x90000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_LOWRAM_ਸਿਰਾ          0xa0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_ROM_ਸ਼ੁਰੂ           0xc0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_BIOS_ADDR           0xf0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_BIOS_ਮਾਪ           0x10000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_EXTRA_ਢੇਰ_ਮਾਪ    0x800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_SMM_INIT_ADDR       0x30000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_SMM_ADDR            0xa0000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_PCIMEM_ਸ਼ੁਰੂ        0xe0000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_PCIMEM_ਸਿਰਾ          0xfec00000    /* IOAPIC is mapped at */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_PCIMEM64_ਸ਼ੁਰੂ      0x8000000000ULL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_PCIMEM64_ਸਿਰਾ        0x10000000000ULL

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_IOAPIC_ADDR         0xfec00000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_IOAPIC_ID           0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_HPET_ਪਤਾ        0xfed00000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_APIC_ADDR           0xfee00000

// PCI IRQS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_PCI_IRQS            ((1<<5) | (1<<9) | (1<<10) | (1<<11))

// Important real-mode segments
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG_IVT      0x0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG_BDA      0x0040
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG_BIOS     0xf000

// Segment definitions in protected mode (see rombios32_gdt in misc.c)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG32_MODE32_CS    (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG32_MODE32_DS    (2 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG32_MODE16_CS    (3 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG32_MODE16_DS    (4 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG32_MODE16BIG_CS (5 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG32_MODE16BIG_DS (6 << 3)

// Debugging levels.  If non-zero and CONFIG_DEBUG_LEVEL is greater
// than the specified value, then the corresponding irq handler will
// report every enter event.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_02 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_05 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_08 20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_09 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_0e 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_11 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_12 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_13 10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_14 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_15 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_16 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_17 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_18 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_19 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_1a 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_40 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_70 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_74 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_75 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_76 10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_hwpic1 5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_ISR_hwpic2 5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_smi 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_smp 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_pnp 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_pmm 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_pcibios 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_HDL_apm 9

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_unimplemented 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_invalid 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_thread 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_tcg 20

#ਜੇ_ਅੰਤ // config.h
