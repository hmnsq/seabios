// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "byteorder.h" // le64_to_cpu
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#ಅನ್ತರ್ಭಾವ "dev-piix.h" // PIIX_*
#ಅನ್ತರ್ಭಾವ "e820map.h" // e820_add
#ಅನ್ತರ್ಭಾವ "hw/ata.h" // PORT_ATA1_CMD_BASE
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_readl
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_probe_devices
#ಅನ್ತರ್ಭಾವ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_COMMAND
#ಅನ್ತರ್ಭಾವ "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#ಅನ್ತರ್ಭಾವ "list.h" // struct hlist_node
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // RamSize
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // pci_setup
#ಅನ್ತರ್ಭಾವ "x86.h" // outb

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ROM_ಸ್ಥಾನ 6
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಅಂಕೆ_REGIONS 7
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_BRIDGE_ಅಂಕೆ_REGIONS 2

ಎಣಿಕೆ pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_ಎಣಿಕೆ,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_ಆರಂಭ   = BUILD_PCIMEM_ಆರಂಭ;
u64 pcimem_ಕೊನೆ     = BUILD_PCIMEM_ಕೊನೆ;
u64 pcimem64_ಆರಂಭ = BUILD_PCIMEM64_ಆರಂಭ;
u64 pcimem64_ಕೊನೆ   = BUILD_PCIMEM64_ಕೊನೆ;

// Resource allocation limits
ಸ್ಥಿರ u64 pci_io_ಕೆಳಗಿನ_ಕೊನೆ = 0xa000;
ಸ್ಥಿರ u64 pci_mem64_ತುದಿ  = 0;
ಸ್ಥಿರ u32 pci_pad_mem64  = -1;

ರಚನೆ pci_region_ನಮೂದು {
    ರಚನೆ pci_device *dev;
    ಪೂರ್ಣಾಂಕ bar;
    u64 ಗಾತ್ರ_2;
    u64 align;
    ಪೂರ್ಣಾಂಕ is64;
    ಎಣಿಕೆ pci_region_type type;
    ರಚನೆ hlist_ಗಂಟು ಗಂಟು;
};

ರಚನೆ pci_region {
    /* pci region assignments */
    u64 ತಳ;
    ರಚನೆ hlist_ತಲೆ list;
};

ರಚನೆ pci_bus {
    ರಚನೆ pci_region r[PCI_REGION_TYPE_ಎಣಿಕೆ];
    ರಚನೆ pci_device *bus_dev;
};

ಸ್ಥಿರ u32 pci_bar(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ region_ಅಂಕೆ)
{
    ಆದರೆ (region_ಅಂಕೆ != PCI_ROM_ಸ್ಥಾನ) {
        ಹಿಂತಿರುಗಿ PCI_ತಳ_ವಿಳಾಸ_0 + region_ಅಂಕೆ * 4;
    }

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಶೀರ್ಷಿಕೆ_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->ಶೀರ್ಷಿಕೆ_type & ~PCI_ಶೀರ್ಷಿಕೆ_TYPE_MULTI_FUNCTION;
    ಹಿಂತಿರುಗಿ type == PCI_ಶೀರ್ಷಿಕೆ_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ವಿಳಾಸ;
}

ಸ್ಥಿರ ಶೂನ್ಯ
pci_ಗಣ_io_region_addr(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ bar, u64 addr, ಪೂರ್ಣಾಂಕ is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    ಆದರೆ (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
ಸ್ಥಿರಾಂಕ u8 pci_irqs[4] = {
    10, 10, 11, 11
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dummy_pci_ಸ್ಥಾನ_get_irq(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    ಹಿಂತಿರುಗಿ 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ (*pci_ಸ್ಥಾನ_get_irq)(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ pin) =
    dummy_pci_ಸ್ಥಾನ_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ piix_pci_ಸ್ಥಾನ_get_irq(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ pin)
{
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನ_addend = 0;

    ಆಗ (pci->ಪೋಷಕ != ಖಾಲಿ) {
        ಸ್ಥಾನ_addend += pci_bdf_ಗಮ್ಯ_dev(pci->bdf);
        pci = pci->ಪೋಷಕ;
    }
    ಸ್ಥಾನ_addend += pci_bdf_ಗಮ್ಯ_dev(pci->bdf) - 1;
    ಹಿಂತಿರುಗಿ pci_irqs[(pin - 1 + ಸ್ಥಾನ_addend) & 3];
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mch_pci_ಸ್ಥಾನ_get_irq(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ pin)
{
    ಪೂರ್ಣಾಂಕ pin_addend = 0;
    ಆಗ (pci->ಪೋಷಕ != ಖಾಲಿ) {
        pin_addend += pci_bdf_ಗಮ್ಯ_dev(pci->bdf);
        pci = pci->ಪೋಷಕ;
    }
    u8 ಸ್ಥಾನ = pci_bdf_ಗಮ್ಯ_dev(pci->bdf);
    ಆದರೆ (ಸ್ಥಾನ <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        ಹಿಂತಿರುಗಿ pci_irqs[(pin - 1 + pin_addend + ಸ್ಥಾನ) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    ಹಿಂತಿರುಗಿ pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
ಸ್ಥಿರ ಶೂನ್ಯ piix_isa_bridge_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    ಪೂರ್ಣಾಂಕ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    ಗಾಗಿ (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_ಪೋರ್ಟ್_ELCR1);
    outb(elcr[1], PIIX_ಪೋರ್ಟ್_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}

ಸ್ಥಿರ ಶೂನ್ಯ mch_isa_lpc_ಸಿದ್ಧತೆ(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_ತಳ | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ಸ್ಥಿರ ಶೂನ್ಯ mch_isa_bridge_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *dev, ಶೂನ್ಯ *arg)
{
    u16 bdf = dev->bdf;
    ಪೂರ್ಣಾಂಕ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    ಗಾಗಿ (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));

        /* activate irq remapping in LPC */

        /* PIRQ[A-D] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT + i, irq);
        /* PIRQ[E-H] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT + i, irq);
    }
    outb(elcr[0], ICH9_LPC_ಪೋರ್ಟ್_ELCR1);
    outb(elcr[1], ICH9_LPC_ಪೋರ್ಟ್_ELCR2);
    dprintf(1, "Q35 LPC init: elcr=%02x %02x\n", elcr[0], elcr[1]);

    ICH9LpcBDF = bdf;

    mch_isa_lpc_ಸಿದ್ಧತೆ(bdf);

    e820_ಕೂಡಿಸು(ICH9_LPC_RCBA_ADDR, 16*1024, E820_RESERVED);

    acpi_pm1a_cnt = acpi_pm_ತಳ + 0x04;
    pmtimer_ಸಿದ್ಧತೆ(acpi_pm_ತಳ + 0x08);
}

ಸ್ಥಿರ ಶೂನ್ಯ storage_ide_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_ಗಣ_io_region_addr(pci, 0, ಪೋರ್ಟ್_ATA1_CMD_ತಳ, 0);
    pci_ಗಣ_io_region_addr(pci, 1, ಪೋರ್ಟ್_ATA1_CTRL_ತಳ, 0);
    pci_ಗಣ_io_region_addr(pci, 2, ಪೋರ್ಟ್_ATA2_CMD_ತಳ, 0);
    pci_ಗಣ_io_region_addr(pci, 3, ಪೋರ್ಟ್_ATA2_CTRL_ತಳ, 0);
}

/* PIIX3/PIIX4 IDE */
ಸ್ಥಿರ ಶೂನ್ಯ piix_ide_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

ಸ್ಥಿರ ಶೂನ್ಯ pic_ibm_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_ಗಣ_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ apple_macio_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    /* macio bridge */
    pci_ಗಣ_io_region_addr(pci, 0, 0x80800000, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ piix4_pm_config_ಸಿದ್ಧತೆ(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_ಅಡಚಣೆ_ಸಾಲು, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_ತಳ | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_ತಳ + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
ಸ್ಥಿರ ಶೂನ್ಯ piix4_pm_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_ಸಿದ್ಧತೆ(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_ತಳ + 0x04;
    pmtimer_ಸಿದ್ಧತೆ(acpi_pm_ತಳ + 0x08);
}

ಸ್ಥಿರ ಶೂನ್ಯ ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_ತಳ,
                      (acpi_pm_ತಳ + 0x100) | PCI_ತಳ_ವಿಳಾಸ_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
ಸ್ಥಿರ ಶೂನ್ಯ ich9_smbus_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *dev, ಶೂನ್ಯ *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

ಸ್ಥಿರ ಶೂನ್ಯ intel_igd_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *dev, ಶೂನ್ಯ *arg)
{
    ರಚನೆ romfile_s *opregion = romfile_ಕಂಡುಹಿಡಿ("etc/igd-opregion");
    u64 bdsm_ಗಾತ್ರ = le64_ಗಮ್ಯ_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    ಆದರೆ (opregion && opregion->ಗಾತ್ರ_2) {
        ಶೂನ್ಯ *addr = memalign_ಮೇಲಿನ(ಪುಟ_ಗಾತ್ರ, opregion->ಗಾತ್ರ_2);
        ಆದರೆ (!addr) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ;
        }

        ಆದರೆ (opregion->copy(opregion, addr, opregion->ಗಾತ್ರ_2) < 0) {
            ಬಿಡುಗಡೆ(addr);
            ಹಿಂತಿರುಗಿ;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_ಗಮ್ಯ_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->ಗಾತ್ರ_2 >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    ಆದರೆ (bdsm_ಗಾತ್ರ && (dev->bdf == pci_ಗಮ್ಯ_bdf(0, 2, 0))) {
        ಶೂನ್ಯ *addr = memalign_tmphigh(1024 * 1024, bdsm_ಗಾತ್ರ);
        ಆದರೆ (!addr) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ;
        }

        e820_ಕೂಡಿಸು((u32)addr, bdsm_ಗಾತ್ರ, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_ಗಮ್ಯ_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_ಗಾತ್ರ >> 20, dev);
    }
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ pci_device_id pci_device_tbl[] = {
    /* PIIX3/PIIX4 PCI to ISA bridge */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0,
               piix_isa_bridge_ಸಿದ್ಧತೆ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0,
               piix_isa_bridge_ಸಿದ್ಧತೆ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC,
               mch_isa_bridge_ಸಿದ್ಧತೆ),

    /* STORAGE IDE */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1,
                     PCI_CLASS_STORAGE_IDE, piix_ide_ಸಿದ್ಧತೆ),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB,
                     PCI_CLASS_STORAGE_IDE, piix_ide_ಸಿದ್ಧತೆ),
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
                     storage_ide_ಸಿದ್ಧತೆ),

    /* PIC, IBM, MPIC & MPIC2 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0x0046, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_ಸಿದ್ಧತೆ),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0xFFFF, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_ಸಿದ್ಧತೆ),

    /* PIIX4 Power Management device (for ACPI) */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3,
               piix4_pm_ಸಿದ್ಧತೆ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_SMBUS,
               ich9_smbus_ಸಿದ್ಧತೆ),

    /* 0xff00 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0017, 0xff00, apple_macio_ಸಿದ್ಧತೆ),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0022, 0xff00, apple_macio_ಸಿದ್ಧತೆ),

    /* Intel IGD OpRegion setup */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
                     intel_igd_ಸಿದ್ಧತೆ),

    PCI_DEVICE_ಕೊನೆ,
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ MCHMmcfgBDF = -1;
ಸ್ಥಿರ ಶೂನ್ಯ mch_mmconfig_ಸಿದ್ಧತೆ(u16 bdf);

ಶೂನ್ಯ pci_resume(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU) {
        ಹಿಂತಿರುಗಿ;
    }

    ಆದರೆ (PiixPmBDF >= 0) {
        piix4_pm_config_ಸಿದ್ಧತೆ(PiixPmBDF);
    }

    ಆದರೆ (ICH9LpcBDF >= 0) {
        mch_isa_lpc_ಸಿದ್ಧತೆ(ICH9LpcBDF);
    }

    ಆದರೆ (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    ಆದರೆ(MCHMmcfgBDF >= 0) {
        mch_mmconfig_ಸಿದ್ಧತೆ(MCHMmcfgBDF);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_init_device(ರಚನೆ pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    ಪೂರ್ಣಾಂಕ pin = pci_config_readb(bdf, PCI_ಅಡಚಣೆ_PIN);
    ಆದರೆ (pin != 0)
        pci_config_writeb(bdf, PCI_ಅಡಚಣೆ_ಸಾಲು, pci_ಸ್ಥಾನ_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, ಖಾಲಿ);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_ಸ್ಮರಣೆ | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    ಆದರೆ (pci->ಶೀರ್ಷಿಕೆ_type & PCI_ಶೀರ್ಷಿಕೆ_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_init_devices(ಶೂನ್ಯ)
{
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_enable_default_vga(ಶೂನ್ಯ)
{
    ರಚನೆ pci_device *pci;

    foreachpci(pci) {
        ಆದರೆ (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            ಹಿಂತಿರುಗಿ;
        }
    }

    pci = pci_ಕಂಡುಹಿಡಿ_class(PCI_CLASS_DISPLAY_VGA);
    ಆದರೆ (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_ಸ್ಮರಣೆ);

    ಆಗ (pci->ಪೋಷಕ) {
        pci = pci->ಪೋಷಕ;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_ಸ್ಮರಣೆ);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ i440fx_mem_addr_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *dev, ಶೂನ್ಯ *arg)
{
    ಆದರೆ (Ram_ಗಾತ್ರ <= 0x80000000)
        pcimem_ಆರಂಭ = 0x80000000;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Ram_ಗಾತ್ರ <= 0xc0000000)
        pcimem_ಆರಂಭ = 0xc0000000;

    pci_ಸ್ಥಾನ_get_irq = piix_pci_ಸ್ಥಾನ_get_irq;
}

ಸ್ಥಿರ ಶೂನ್ಯ mch_mmconfig_ಸಿದ್ಧತೆ(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 ಊರ್ಧ್ವ = addr >> 32;
    u32 ಅಧೋ = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, ಊರ್ಧ್ವ);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, ಅಧೋ);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

ಸ್ಥಿರ ಶೂನ್ಯ mch_mem_addr_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *dev, ಶೂನ್ಯ *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 ಗಾತ್ರ_2 = Q35_HOST_BRIDGE_PCIEXBAR_ಗಾತ್ರ;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_ಸಿದ್ಧತೆ(dev->bdf);
    e820_ಕೂಡಿಸು(addr, ಗಾತ್ರ_2, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_ಆರಂಭ = addr + ಗಾತ್ರ_2;

    pci_ಸ್ಥಾನ_get_irq = mch_pci_ಸ್ಥಾನ_get_irq;

    /* setup io address space */
    ಆದರೆ (acpi_pm_ತಳ < 0x1000)
        pci_io_ಕೆಳಗಿನ_ಕೊನೆ = 0x10000;
    ಇಲ್ಲದಿದ್ದರೆ
        pci_io_ಕೆಳಗಿನ_ಕೊನೆ = acpi_pm_ತಳ;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_ಸಿದ್ಧತೆ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_ಸಿದ್ಧತೆ),
    PCI_DEVICE_ಕೊನೆ
};

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_init_platform(ಶೂನ್ಯ)
{
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, ಖಾಲಿ);
    }
}

ಸ್ಥಿರ u8 pci_ಕಂಡುಹಿಡಿ_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    ಆದರೆ (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    ಆದರೆ (device_id != PCI_DEVICE_ID_REDHAT_ಬೇರು_ಪೋರ್ಟ್ &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        ಹಿಂತಿರುಗಿ 0;
    }
    u8 cap = 0;

    ಮಾಡು {
        cap = pci_ಕಂಡುಹಿಡಿ_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } ಆಗ (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_ಸ್ಥಾನಾಂತರ) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    ಆದರೆ (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        ಆದರೆ (cap_len < RES_RESERVE_CAP_ಗಾತ್ರ) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            ಹಿಂತಿರುಗಿ 0;
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    ಹಿಂತಿರುಗಿ cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
pci_bios_init_bus_rec(ಪೂರ್ಣಾಂಕ bus, u8 *pci_bus)
{
    ಪೂರ್ಣಾಂಕ bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ಆದರೆ (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ಆದರೆ (class != PCI_CLASS_BRIDGE_PCI) {
            ಮುಂದುವರಿ;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        ಆದರೆ (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } ಇಲ್ಲದಿದ್ದರೆ {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        ಆದರೆ (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } ಇಲ್ಲದಿದ್ದರೆ {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        ಆದರೆ (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_ಕಂಡುಹಿಡಿ_resource_reserve_capability(bdf);

            ಆದರೆ (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                ಆದರೆ (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    ಆದರೆ ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    ಆದರೆ (secbus + res_bus > *pci_bus) {
                        dprintf(1, "PCI: QEMU resource reserve cap: bus = %u\n",
                                res_bus);
                        res_bus = secbus + res_bus;
                    }
                }
            }
            dprintf(1, "PCI: subordinate bus = 0x%x -> 0x%x\n",
                    subbus, res_bus);
            subbus = res_bus;
            *pci_bus = res_bus;
        } ಇಲ್ಲದಿದ್ದರೆ {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
pci_bios_init_bus(ಶೂನ್ಯ)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    ಆದರೆ (extraroots) {
        ಆಗ (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
pci_bios_get_bar(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ bar,
                 ಪೂರ್ಣಾಂಕ *ptype, u64 *psize, ಪೂರ್ಣಾಂಕ *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    ಪೂರ್ಣಾಂಕ is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 ಮುಖವಾಡ;

    ಆದರೆ (bar == PCI_ROM_ಸ್ಥಾನ) {
        ಮುಖವಾಡ = PCI_ROM_ವಿಳಾಸ_ಮುಖವಾಡ;
        pci_config_writel(bdf, ofs, ಮುಖವಾಡ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (old & PCI_ತಳ_ವಿಳಾಸ_SPACE_IO) {
            ಮುಖವಾಡ = PCI_ತಳ_ವಿಳಾಸ_IO_ಮುಖವಾಡ;
            type = PCI_REGION_TYPE_IO;
        } ಇಲ್ಲದಿದ್ದರೆ {
            ಮುಖವಾಡ = PCI_ತಳ_ವಿಳಾಸ_MEM_ಮುಖವಾಡ;
            ಆದರೆ (old & PCI_ತಳ_ವಿಳಾಸ_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_ಮುಖವಾಡ)
                    == PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    ಆದರೆ (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 ಮೇಲಿನ = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)ಮೇಲಿನ << 32);
        ಮುಖವಾಡ |= ((u64)0xffffffff << 32);
        *psize = (~(val & ಮುಖವಾಡ)) + 1;
    } ಇಲ್ಲದಿದ್ದರೆ {
        *psize = ((~(val & ಮುಖವಾಡ)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pci_bios_bridge_region_is64(ರಚನೆ pci_region *r,
                                 ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ type)
{
    ಆದರೆ (type != PCI_REGION_TYPE_PREFMEM)
        ಹಿಂತಿರುಗಿ 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_ಸ್ಮರಣೆ_ತಳ);
    ಆದರೆ (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_ಸ್ಮರಣೆ_ತಳ, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_ಸ್ಮರಣೆ_ತಳ);
        pci_config_writel(pci->bdf, PCI_PREF_ಸ್ಮರಣೆ_ತಳ, 0x0);
    }
    ಆದರೆ ((pmem & PCI_PREF_RANGE_TYPE_ಮುಖವಾಡ) != PCI_PREF_RANGE_TYPE_64)
       ಹಿಂತಿರುಗಿ 0;
    ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು;
    hlist_for_each_ನಮೂದು(ನಮೂದು, &r->list, ಗಂಟು) {
        ಆದರೆ (!ನಮೂದು->is64)
            ಹಿಂತಿರುಗಿ 0;
    }
    ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ u64 pci_region_align(ರಚನೆ pci_region *r)
{
    ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು;
    hlist_for_each_ನಮೂದು(ನಮೂದು, &r->list, ಗಂಟು) {
        // The first entry in the sorted list has the largest alignment
        ಹಿಂತಿರುಗಿ ನಮೂದು->align;
    }
    ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ u64 pci_region_ಮೊತ್ತ(ರಚನೆ pci_region *r)
{
    u64 ಮೊತ್ತ = 0;
    ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು;
    hlist_for_each_ನಮೂದು(ನಮೂದು, &r->list, ಗಂಟು) {
        ಮೊತ್ತ += ನಮೂದು->ಗಾತ್ರ_2;
    }
    ಹಿಂತಿರುಗಿ ಮೊತ್ತ;
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_region_migrate_64bit_entries(ರಚನೆ pci_region *from,
                                             ರಚನೆ pci_region *ಗಮ್ಯ)
{
    ರಚನೆ hlist_ಗಂಟು *n, **ಕೊನೆಯ = &ಗಮ್ಯ->list.ಮೊದಲ;
    ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು;
    hlist_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(ನಮೂದು, n, &from->list, ಗಂಟು) {
        ಆದರೆ (!ನಮೂದು->is64)
            ಮುಂದುವರಿ;
        ಆದರೆ (ನಮೂದು->dev->class == PCI_CLASS_SERIAL_USB)
            ಮುಂದುವರಿ;
        ಆದರೆ (ನಮೂದು->dev->class == PCI_CLASS_STORAGE_NVME)
            ಮುಂದುವರಿ;
        // Move from source list to destination list.
        hlist_del(&ನಮೂದು->ಗಂಟು);
        hlist_ಕೂಡಿಸು(&ನಮೂದು->ಗಂಟು, ಕೊನೆಯ);
        ಕೊನೆಯ = &ನಮೂದು->ಗಂಟು.ಮುಂದಿನ;
    }
}

ಸ್ಥಿರ ರಚನೆ pci_region_ನಮೂದು *
pci_region_create_ನಮೂದು(ರಚನೆ pci_bus *bus, ರಚನೆ pci_device *dev,
                        ಪೂರ್ಣಾಂಕ bar, u64 ಗಾತ್ರ_2, u64 align, ಪೂರ್ಣಾಂಕ type, ಪೂರ್ಣಾಂಕ is64)
{
    ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು = malloc_tmp(ಗಾತ್ರ(*ನಮೂದು));
    ಆದರೆ (!ನಮೂದು) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(ನಮೂದು, 0, ಗಾತ್ರ(*ನಮೂದು));
    ನಮೂದು->dev = dev;
    ನಮೂದು->bar = bar;
    ನಮೂದು->ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
    ನಮೂದು->align = align;
    ನಮೂದು->is64 = is64;
    ನಮೂದು->type = type;
    // Insert into list in sorted order.
    ರಚನೆ hlist_ಗಂಟು **pprev;
    ರಚನೆ pci_region_ನಮೂದು *pos;
    hlist_for_each_ನಮೂದು_pprev(pos, pprev, &bus->r[type].list, ಗಂಟು) {
        ಆದರೆ (pos->align < align || (pos->align == align && pos->ಗಾತ್ರ_2 < ಗಾತ್ರ_2))
            ನಿಲ್ಲಿಸು;
    }
    hlist_ಕೂಡಿಸು(&ನಮೂದು->ಗಂಟು, pprev);
    ಹಿಂತಿರುಗಿ ನಮೂದು;
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

ಸ್ಥಿರ hotplug_type_t pci_bus_hotplug_support(ರಚನೆ pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    ಆದರೆ (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 ಪೋರ್ಟ್_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        ಆದರೆ (ಪೋರ್ಟ್_type == PCI_EXP_TYPE_PCI_BRIDGE)
            ಹೋಗು check_shpc;

        u8 downstream_ಪೋರ್ಟ್ = (ಪೋರ್ಟ್_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (ಪೋರ್ಟ್_type == PCI_EXP_TYPE_ಬೇರು_ಪೋರ್ಟ್);
        /*
         * PCI Express SPEC, 7.8.2:
         *   Slot Implemented – When Set, this bit indicates that the Link
         *   HwInit associated with this Port is connected to a slot (as
         *   compared to being connected to a system-integrated device or
         *   being disabled).
         *   This bit is valid for Downstream Ports. This bit is undefined
         *   for Upstream Ports.
         */
        u16 ಸ್ಥಾನ_implemented = pcie_flags & PCI_EXP_FLAGS_ಸ್ಥಾನ;

        ಹಿಂತಿರುಗಿ downstream_ಪೋರ್ಟ್ && ಸ್ಥಾನ_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_ಕಂಡುಹಿಡಿ_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    ಹಿಂತಿರುಗಿ !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pci_bridge_has_region(ರಚನೆ pci_device *pci,
                                 ಎಣಿಕೆ pci_region_type region_type)
{
    u8 ತಳ;

    ಆಯ್ಕೆ (region_type) {
        ಸ್ಥಿತಿ PCI_REGION_TYPE_IO:
            ತಳ = PCI_IO_ತಳ;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ PCI_REGION_TYPE_PREFMEM:
            ತಳ = PCI_PREF_ಸ್ಮರಣೆ_ತಳ;
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            /* Regular memory support is mandatory */
            ಹಿಂತಿರುಗಿ 1;
    }

    pci_config_writeb(pci->bdf, ತಳ, 0xFF);

    ಹಿಂತಿರುಗಿ pci_config_readb(pci->bdf, ತಳ) != 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pci_bios_check_devices(ರಚನೆ pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        ರಚನೆ pci_bus *bus = &busses[pci_bdf_ಗಮ್ಯ_bus(pci->bdf)];
        ಆದರೆ (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i = 0; i < PCI_ಅಂಕೆ_REGIONS; i++) {
            ಆದರೆ ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_ಅಂಕೆ_REGIONS && i < PCI_ROM_ಸ್ಥಾನ))
                ಮುಂದುವರಿ;
            ಪೂರ್ಣಾಂಕ type, is64;
            u64 ಗಾತ್ರ_2;
            pci_bios_get_bar(pci, i, &type, &ಗಾತ್ರ_2, &is64);
            ಆದರೆ (ಗಾತ್ರ_2 == 0)
                ಮುಂದುವರಿ;

            ಆದರೆ (type != PCI_REGION_TYPE_IO && ಗಾತ್ರ_2 < PCI_DEVICE_MEM_MIN)
                ಗಾತ್ರ_2 = PCI_DEVICE_MEM_MIN;
            ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು = pci_region_create_ನಮೂದು(
                bus, pci, i, ಗಾತ್ರ_2, ಗಾತ್ರ_2, type, is64);
            ಆದರೆ (!ನಮೂದು)
                ಹಿಂತಿರುಗಿ -1;

            ಆದರೆ (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    ಪೂರ್ಣಾಂಕ secondary_bus;
    ಗಾಗಿ (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        ರಚನೆ pci_bus *s = &busses[secondary_bus];
        ಆದರೆ (!s->bus_dev)
            ಮುಂದುವರಿ;
        ರಚನೆ pci_bus *ಪೋಷಕ = &busses[pci_bdf_ಗಮ್ಯ_bus(s->bus_dev->bdf)];
        ಆದರೆ (!ಪೋಷಕ->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            ಪೋಷಕ = &busses[0];
        ಪೂರ್ಣಾಂಕ type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_ಕಂಡುಹಿಡಿ_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_ಕಂಡುಹಿಡಿ_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        ಗಾಗಿ (type = 0; type < PCI_REGION_TYPE_ಎಣಿಕೆ; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            ಆದರೆ (!pci_bridge_has_region(s->bus_dev, type))
                ಮುಂದುವರಿ;
            u64 ಗಾತ್ರ_2 = 0;
            ಆದರೆ (qemu_cap) {
                u32 tmp_ಗಾತ್ರ;
                u64 tmp_ಗಾತ್ರ_64;
                ಆಯ್ಕೆ(type) {
                ಸ್ಥಿತಿ PCI_REGION_TYPE_IO:
                    tmp_ಗಾತ್ರ_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    ಆದರೆ (tmp_ಗಾತ್ರ_64 != (u64)-1) {
                        ಗಾತ್ರ_2 = tmp_ಗಾತ್ರ_64;
                    }
                    ನಿಲ್ಲಿಸು;
                ಸ್ಥಿತಿ PCI_REGION_TYPE_MEM:
                    tmp_ಗಾತ್ರ = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    ಆದರೆ (tmp_ಗಾತ್ರ != (u32)-1) {
                        ಗಾತ್ರ_2 = tmp_ಗಾತ್ರ;
                    }
                    ನಿಲ್ಲಿಸು;
                ಸ್ಥಿತಿ PCI_REGION_TYPE_PREFMEM:
                    tmp_ಗಾತ್ರ = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_ಗಾತ್ರ_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    ಆದರೆ (tmp_ಗಾತ್ರ != (u32)-1 && tmp_ಗಾತ್ರ_64 == (u64)-1) {
                        ಗಾತ್ರ_2 = tmp_ಗಾತ್ರ;
                    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (tmp_ಗಾತ್ರ == (u32)-1 && tmp_ಗಾತ್ರ_64 != (u64)-1) {
                        ಗಾತ್ರ_2 = tmp_ಗಾತ್ರ_64;
                    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (tmp_ಗಾತ್ರ != (u32)-1 && tmp_ಗಾತ್ರ_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    ನಿಲ್ಲಿಸು;
                ಪೂರ್ವನಿಯೋಜಿತ:
                    ನಿಲ್ಲಿಸು;
                }
            }
            ಆದರೆ (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 ಮೊತ್ತ = pci_region_ಮೊತ್ತ(&s->r[type]);
            ಪೂರ್ಣಾಂಕ is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            ಪೂರ್ಣಾಂಕ resource_optional = 0;
            ಆದರೆ (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 ತುದಿ_align = pci_mem64_ತುದಿ >> 11;
            ಆದರೆ (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (ತುದಿ_align > align))
                align = ತುದಿ_align;
            ಆದರೆ (align > ಮೊತ್ತ && hotplug_support && !resource_optional)
                ಮೊತ್ತ = align; /* reserve min size for hot-plug */
            ಆದರೆ (ಗಾತ್ರ_2 > ಮೊತ್ತ) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        ಗಾತ್ರ_2, region_type_name[type]);
                ಆದರೆ (type != PCI_REGION_TYPE_IO) {
                    ಗಾತ್ರ_2 = ALIGN(ಗಾತ್ರ_2, align);
                }
            } ಇಲ್ಲದಿದ್ದರೆ {
                ಗಾತ್ರ_2 = ALIGN(ಮೊತ್ತ, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು = pci_region_create_ನಮೂದು(
                ಪೋಷಕ, s->bus_dev, -1, ಗಾತ್ರ_2, align, type, is64);
            ಆದರೆ (!ನಮೂದು)
                ಹಿಂತಿರುಗಿ -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      ನಮೂದು->dev->secondary_bus, ಗಾತ್ರ_2,
                      region_type_name[ನಮೂದು->type]);
        }
    }
    ಹಿಂತಿರುಗಿ 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pci_bios_init_ಬೇರು_regions_io(ರಚನೆ pci_bus *bus)
{
    /*
     * QEMU I/O address space usage:
     *   0000 - 0fff    legacy isa, pci config, pci root bus, ...
     *   1000 - 9fff    free
     *   a000 - afff    hotplug (cpu, pci via acpi, i440fx/piix only)
     *   b000 - bfff    power management (PORT_ACPI_PM_BASE)
     *                  [ qemu 1.4+ implements pci config registers
     *                    properly so guests can place the registers
     *                    where they want, on older versions its fixed ]
     *   c000 - ffff    free, traditionally used for pci io
     */
    ರಚನೆ pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 ಮೊತ್ತ = pci_region_ಮೊತ್ತ(r_io);
    ಆದರೆ (ಮೊತ್ತ < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->ತಳ = 0xc000;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೊತ್ತ < pci_io_ಕೆಳಗಿನ_ಕೊನೆ - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->ತಳ = 0x1000;
    } ಇಲ್ಲದಿದ್ದರೆ {
        /* not enouth io address space -> error out */
        ಹಿಂತಿರುಗಿ -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->ತಳ, r_io->ತಳ + ಮೊತ್ತ - 1);
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pci_bios_init_ಬೇರು_regions_mem(ರಚನೆ pci_bus *bus)
{
    ರಚನೆ pci_region *r_ಕೊನೆ = &bus->r[PCI_REGION_TYPE_PREFMEM];
    ರಚನೆ pci_region *r_ಆರಂಭ = &bus->r[PCI_REGION_TYPE_MEM];

    ಆದರೆ (pci_region_align(r_ಆರಂಭ) < pci_region_align(r_ಕೊನೆ)) {
        // Swap regions to improve alignment.
        r_ಕೊನೆ = r_ಆರಂಭ;
        r_ಆರಂಭ = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 ಮೊತ್ತ = pci_region_ಮೊತ್ತ(r_ಕೊನೆ);
    u64 align = pci_region_align(r_ಕೊನೆ);
    r_ಕೊನೆ->ತಳ = ALIGN_ಕೆಳಕ್ಕೆ((pcimem_ಕೊನೆ - ಮೊತ್ತ), align);
    ಮೊತ್ತ = pci_region_ಮೊತ್ತ(r_ಆರಂಭ);
    align = pci_region_align(r_ಆರಂಭ);
    r_ಆರಂಭ->ತಳ = ALIGN_ಕೆಳಕ್ಕೆ((r_ಕೊನೆ->ತಳ - ಮೊತ್ತ), align);

    ಆದರೆ ((r_ಆರಂಭ->ತಳ < pcimem_ಆರಂಭ) ||
         (r_ಆರಂಭ->ತಳ > pcimem_ಕೊನೆ))
        // Memory range requested is larger than available.
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_IO_ಸರಿಸು            8
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಸ್ಮರಣೆ_ಸರಿಸು        16
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PREF_ಸ್ಮರಣೆ_ಸರಿಸು   16

ಸ್ಥಿರ ಶೂನ್ಯ
pci_region_map_one_ನಮೂದು(ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು, u64 addr)
{
    ಆದರೆ (ನಮೂದು->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                ನಮೂದು->dev,
                ನಮೂದು->bar, addr, ನಮೂದು->ಗಾತ್ರ_2, region_type_name[ನಮೂದು->type]);

        pci_ಗಣ_io_region_addr(ನಮೂದು->dev, ನಮೂದು->bar, addr, ನಮೂದು->is64);
        ಹಿಂತಿರುಗಿ;
    }

    u16 bdf = ನಮೂದು->dev->bdf;
    u64 ಎಲ್ಲೆ = addr + ನಮೂದು->ಗಾತ್ರ_2 - 1;
    ಆದರೆ (ನಮೂದು->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_ತಳ, addr >> PCI_IO_ಸರಿಸು);
        pci_config_writew(bdf, PCI_IO_ತಳ_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_ಎಲ್ಲೆ, ಎಲ್ಲೆ >> PCI_IO_ಸರಿಸು);
        pci_config_writew(bdf, PCI_IO_ಎಲ್ಲೆ_UPPER16, 0);
    }
    ಆದರೆ (ನಮೂದು->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_ಸ್ಮರಣೆ_ತಳ, addr >> PCI_ಸ್ಮರಣೆ_ಸರಿಸು);
        pci_config_writew(bdf, PCI_ಸ್ಮರಣೆ_ಎಲ್ಲೆ, ಎಲ್ಲೆ >> PCI_ಸ್ಮರಣೆ_ಸರಿಸು);
    }
    ಆದರೆ (ನಮೂದು->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_ಸ್ಮರಣೆ_ತಳ, addr >> PCI_PREF_ಸ್ಮರಣೆ_ಸರಿಸು);
        pci_config_writew(bdf, PCI_PREF_ಸ್ಮರಣೆ_ಎಲ್ಲೆ, ಎಲ್ಲೆ >> PCI_PREF_ಸ್ಮರಣೆ_ಸರಿಸು);
        pci_config_writel(bdf, PCI_PREF_ತಳ_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_ಎಲ್ಲೆ_UPPER32, ಎಲ್ಲೆ >> 32);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_region_map_entries(ರಚನೆ pci_bus *busses, ರಚನೆ pci_region *r)
{
    ರಚನೆ hlist_ಗಂಟು *n;
    ರಚನೆ pci_region_ನಮೂದು *ನಮೂದು;
    hlist_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(ನಮೂದು, n, &r->list, ಗಂಟು) {
        u64 addr = r->ತಳ;
        r->ತಳ += ನಮೂದು->ಗಾತ್ರ_2;
        ಆದರೆ (ನಮೂದು->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[ನಮೂದು->dev->secondary_bus].r[ನಮೂದು->type].ತಳ = addr;
        pci_region_map_one_ನಮೂದು(ನಮೂದು, addr);
        hlist_del(&ನಮೂದು->ಗಂಟು);
        ಬಿಡುಗಡೆ(ನಮೂದು);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_map_devices(ರಚನೆ pci_bus *busses)
{
    ಆದರೆ (pci_bios_init_ಬೇರು_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_ಆರಂಭ, pcimem_ಕೊನೆ);
    ಆದರೆ (pci_pad_mem64 || pci_bios_init_ಬೇರು_regions_mem(busses)) {
        ರಚನೆ pci_region r64_mem, r64_pref;
        r64_mem.list.ಮೊದಲ = ಖಾಲಿ;
        r64_pref.list.ಮೊದಲ = ಖಾಲಿ;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        ಆದರೆ (pci_bios_init_ಬೇರು_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 ಮೊತ್ತ_mem = pci_region_ಮೊತ್ತ(&r64_mem);
        u64 ಮೊತ್ತ_pref = pci_region_ಮೊತ್ತ(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.ತಳ = le64_ಗಮ್ಯ_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        ಆದರೆ (r64_mem.ತಳ < 0x100000000LL + Ram_ಗಾತ್ರ_Over4_G)
            r64_mem.ತಳ = 0x100000000LL + Ram_ಗಾತ್ರ_Over4_G;
        ಆದರೆ (pci_mem64_ತುದಿ) {
            u64 ಗಾತ್ರ_2 = (ALIGN(ಮೊತ್ತ_mem, (1LL<<30)) +
                        ALIGN(ಮೊತ್ತ_pref, (1LL<<30)));
            ಆದರೆ (pci_pad_mem64)
                ಗಾತ್ರ_2 = ALIGN(ಗಾತ್ರ_2, pci_mem64_ತುದಿ >> 3);
            ಆದರೆ (r64_mem.ತಳ < pci_mem64_ತುದಿ - ಗಾತ್ರ_2) {
                r64_mem.ತಳ = pci_mem64_ತುದಿ - ಗಾತ್ರ_2;
            }
            ಆದರೆ (e820_is_ಬಳಸಿದ(r64_mem.ತಳ, ಗಾತ್ರ_2))
                r64_mem.ತಳ -= ಗಾತ್ರ_2;
        }
        r64_mem.ತಳ = ALIGN(r64_mem.ತಳ, align_mem);
        r64_mem.ತಳ = ALIGN(r64_mem.ತಳ, (1LL<<30));    // 1G hugepage
        r64_pref.ತಳ = r64_mem.ತಳ + ಮೊತ್ತ_mem;
        r64_pref.ತಳ = ALIGN(r64_pref.ತಳ, align_pref);
        r64_pref.ತಳ = ALIGN(r64_pref.ತಳ, (1LL<<30));  // 1G hugepage
        pcimem64_ಆರಂಭ = r64_mem.ತಳ;
        pcimem64_ಕೊನೆ = r64_pref.ತಳ + ಮೊತ್ತ_pref;
        pcimem64_ಕೊನೆ = ALIGN(pcimem64_ಕೊನೆ, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_ಆರಂಭ, pcimem64_ಕೊನೆ);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } ಇಲ್ಲದಿದ್ದರೆ {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_ಆರಂಭ = 0;
    }
    // Map regions on each device.
    ಪೂರ್ಣಾಂಕ bus;
    ಗಾಗಿ (bus = 0; bus<=MaxPCIBus; bus++) {
        ಪೂರ್ಣಾಂಕ type;
        ಗಾಗಿ (type = 0; type < PCI_REGION_TYPE_ಎಣಿಕೆ; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

ಶೂನ್ಯ
pci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "pci setup\n");

    ಆದರೆ (CPUPhys_ಬಿಟ್ಟುಗಳು) {
        pci_mem64_ತುದಿ = 1LL << CPUPhys_ಬಿಟ್ಟುಗಳು;
        ಆದರೆ (CPUPhys_ಬಿಟ್ಟುಗಳು > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            ಆದರೆ (Ram_ಗಾತ್ರ_Over4_G < (1LL << 40)) {
                pci_mem64_ತುದಿ = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    ಆದರೆ (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_ಗಾತ್ರ_Over4_G ? 1 : 0;

    ಆದರೆ (!CPULong_ಬಹುಲಕ)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    ಆದರೆ (pci_probe_host() != 0) {
        ಹಿಂತಿರುಗಿ;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_ಆರಂಭ = Ram_ಗಾತ್ರ;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    ರಚನೆ pci_bus *busses = malloc_tmp(ಗಾತ್ರ(*busses) * (MaxPCIBus + 1));
    ಆದರೆ (!busses) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(busses, 0, ಗಾತ್ರ(*busses) * (MaxPCIBus + 1));
    ಆದರೆ (pci_bios_check_devices(busses))
        ಹಿಂತಿರುಗಿ;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    ಬಿಡುಗಡೆ(busses);

    pci_enable_default_vga();
}
