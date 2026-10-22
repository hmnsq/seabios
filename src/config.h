#నిర్వచితం_కాకపోతే __CONFIG_H
#నిర్వచించు __CONFIG_H

#అన్తర్భావ "autoconf.h"

// Configuration definitions.

//#define BUILD_APPNAME  "QEMU"
//#define BUILD_CPUNAME8 "QEMUCPU "
//#define BUILD_APPNAME6 "QEMU  "
//#define BUILD_APPNAME4 "QEMU"
#నిర్వచించు BUILD_APPNAME  "Bochs"
#నిర్వచించు BUILD_CPUNAME8 "BOCHSCPU"
#నిర్వచించు BUILD_APPNAME6 "BOCHS "
#నిర్వచించు BUILD_APPNAME4 "BXPC"

// Maximum number of map entries in the e820 map
#నిర్వచించు BUILD_MAX_E820 128
// Space to reserve in high-memory for tables
#నిర్వచించు BUILD_MIN_HIGHTABLE (256*1024)
#నిర్వచించు BUILD_MAX_HIGHTABLE (16*1024*1024)
// Largest supported externaly facing drive id
#నిర్వచించు BUILD_MAX_EXTDRIVE 16
// Number of bytes the smbios may be and still live in the f-segment
#నిర్వచించు BUILD_MAX_SMBIOS_FSEG     600
// Maximum number of bytes the mptable may be and still be copied to f-segment
#నిర్వచించు BUILD_MAX_MPTABLE_FSEG    600

#నిర్వచించు BUILD_MODEL_ID      0xFC
#నిర్వచించు BUILD_SUBMODEL_ID   0x00
#నిర్వచించు BUILD_BIOS_REVISION 0x01

// Various memory addresses used by the code.
#నిర్వచించు BUILD_దొంతర_ADDR          0x7000
#నిర్వచించు BUILD_S3_RESUME_దొంతర_ADDR 0x1000
#నిర్వచించు BUILD_AP_BOOT_ADDR        0x10000
#నిర్వచించు BUILD_EBDA_MINIMUM        0x90000
#నిర్వచించు BUILD_LOWRAM_చివర          0xa0000
#నిర్వచించు BUILD_ROM_ప్రారంభం           0xc0000
#నిర్వచించు BUILD_BIOS_ADDR           0xf0000
#నిర్వచించు BUILD_BIOS_కొలత           0x10000
#నిర్వచించు BUILD_EXTRA_దొంతర_కొలత    0x800
#నిర్వచించు BUILD_SMM_INIT_ADDR       0x30000
#నిర్వచించు BUILD_SMM_ADDR            0xa0000

#నిర్వచించు BUILD_PCIMEM_ప్రారంభం        0xe0000000
#నిర్వచించు BUILD_PCIMEM_చివర          0xfec00000    /* IOAPIC is mapped at */
#నిర్వచించు BUILD_PCIMEM64_ప్రారంభం      0x8000000000ULL
#నిర్వచించు BUILD_PCIMEM64_చివర        0x10000000000ULL

#నిర్వచించు BUILD_IOAPIC_ADDR         0xfec00000
#నిర్వచించు BUILD_IOAPIC_ID           0
#నిర్వచించు BUILD_HPET_చిరునామా        0xfed00000
#నిర్వచించు BUILD_APIC_ADDR           0xfee00000

// PCI IRQS
#నిర్వచించు BUILD_PCI_IRQS            ((1<<5) | (1<<9) | (1<<10) | (1<<11))

// Important real-mode segments
#నిర్వచించు SEG_IVT      0x0000
#నిర్వచించు SEG_BDA      0x0040
#నిర్వచించు SEG_BIOS     0xf000

// Segment definitions in protected mode (see rombios32_gdt in misc.c)
#నిర్వచించు SEG32_MODE32_CS    (1 << 3)
#నిర్వచించు SEG32_MODE32_DS    (2 << 3)
#నిర్వచించు SEG32_MODE16_CS    (3 << 3)
#నిర్వచించు SEG32_MODE16_DS    (4 << 3)
#నిర్వచించు SEG32_MODE16BIG_CS (5 << 3)
#నిర్వచించు SEG32_MODE16BIG_DS (6 << 3)

// Debugging levels.  If non-zero and CONFIG_DEBUG_LEVEL is greater
// than the specified value, then the corresponding irq handler will
// report every enter event.
#నిర్వచించు DEBUG_ISR_02 1
#నిర్వచించు DEBUG_HDL_05 1
#నిర్వచించు DEBUG_ISR_08 20
#నిర్వచించు DEBUG_ISR_09 9
#నిర్వచించు DEBUG_ISR_0e 9
#నిర్వచించు DEBUG_HDL_11 2
#నిర్వచించు DEBUG_HDL_12 2
#నిర్వచించు DEBUG_HDL_13 10
#నిర్వచించు DEBUG_HDL_14 2
#నిర్వచించు DEBUG_HDL_15 9
#నిర్వచించు DEBUG_HDL_16 9
#నిర్వచించు DEBUG_HDL_17 2
#నిర్వచించు DEBUG_HDL_18 1
#నిర్వచించు DEBUG_HDL_19 1
#నిర్వచించు DEBUG_HDL_1a 9
#నిర్వచించు DEBUG_HDL_40 1
#నిర్వచించు DEBUG_ISR_70 9
#నిర్వచించు DEBUG_ISR_74 9
#నిర్వచించు DEBUG_ISR_75 1
#నిర్వచించు DEBUG_ISR_76 10
#నిర్వచించు DEBUG_ISR_hwpic1 5
#నిర్వచించు DEBUG_ISR_hwpic2 5
#నిర్వచించు DEBUG_HDL_smi 9
#నిర్వచించు DEBUG_HDL_smp 1
#నిర్వచించు DEBUG_HDL_pnp 1
#నిర్వచించు DEBUG_HDL_pmm 1
#నిర్వచించు DEBUG_HDL_pcibios 9
#నిర్వచించు DEBUG_HDL_apm 9

#నిర్వచించు DEBUG_unimplemented 2
#నిర్వచించు DEBUG_invalid 3
#నిర్వచించు DEBUG_thread 2
#నిర్వచించు DEBUG_tcg 20

#అయితే_ముగింపు // config.h
