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

#define PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#define PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#define PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#define PCI_ROM_SLOT 6
#define PCI_ಅಂಕೆ_REGIONS 7
#define PCI_BRIDGE_ಅಂಕೆ_REGIONS 2

ಗಣನಾ pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_ಎಣಿಕೆ,
};

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_start   = BUILD_PCIMEM_START;
u64 pcimem_end     = BUILD_PCIMEM_END;
u64 pcimem64_start = BUILD_PCIMEM64_START;
u64 pcimem64_end   = BUILD_PCIMEM64_END;

// Resource allocation limits
ಸ್ಥಿರ u64 pci_io_ಕೆಳಗಿನ_end = 0xa000;
ಸ್ಥಿರ u64 pci_mem64_ತುದಿ  = 0;
ಸ್ಥಿರ u32 pci_pad_mem64  = -1;

ಸಂರಚನಾ pci_region_entry {
    ಸಂರಚನಾ pci_device *dev;
    ಪೂರ್ಣಾಙ್ಕ bar;
    u64 size;
    u64 align;
    ಪೂರ್ಣಾಙ್ಕ is64;
    ಗಣನಾ pci_region_type type;
    ಸಂರಚನಾ hlist_ಗಂಟು ಗಂಟು;
};

ಸಂರಚನಾ pci_region {
    /* pci region assignments */
    u64 base;
    ಸಂರಚನಾ hlist_head list;
};

ಸಂರಚನಾ pci_bus {
    ಸಂರಚನಾ pci_region r[PCI_REGION_TYPE_ಎಣಿಕೆ];
    ಸಂರಚನಾ pci_device *bus_dev;
};

ಸ್ಥಿರ u32 pci_bar(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ region_ಅಂಕೆ)
{
    ಯದಿ (region_ಅಂಕೆ != PCI_ROM_SLOT) {
        ಪ್ರತ್ಯಾಗಮ PCI_BASE_ADDRESS_0 + region_ಅಂಕೆ * 4;
    }

#define PCI_HEADER_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
    ಪ್ರತ್ಯಾಗಮ type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

ಸ್ಥಿರ ಶೂನ್ಯ
pci_set_io_region_addr(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ bar, u64 addr, ಪೂರ್ಣಾಙ್ಕ is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    ಯದಿ (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
ಧ್ರುವ u8 pci_irqs[4] = {
    10, 10, 11, 11
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dummy_pci_slot_get_irq(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    ಪ್ರತ್ಯಾಗಮ 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ (*pci_slot_get_irq)(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ piix_pci_slot_get_irq(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ pin)
{
    ಪೂರ್ಣಾಙ್ಕ slot_addend = 0;

    ಯಾವತ್ (pci->parent != ರಿಕ್ತ) {
        slot_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_to_dev(pci->bdf) - 1;
    ಪ್ರತ್ಯಾಗಮ pci_irqs[(pin - 1 + slot_addend) & 3];
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mch_pci_slot_get_irq(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ pin)
{
    ಪೂರ್ಣಾಙ್ಕ pin_addend = 0;
    ಯಾವತ್ (pci->parent != ರಿಕ್ತ) {
        pin_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_to_dev(pci->bdf);
    ಯದಿ (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        ಪ್ರತ್ಯಾಗಮ pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    ಪ್ರತ್ಯಾಗಮ pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
ಸ್ಥಿರ ಶೂನ್ಯ piix_isa_bridge_setup(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    ಪೂರ್ಣಾಙ್ಕ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    ಪ್ರತಿ (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_PORT_ELCR1);
    outb(elcr[1], PIIX_PORT_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}

ಸ್ಥಿರ ಶೂನ್ಯ mch_isa_lpc_setup(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_base | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ಸ್ಥಿರ ಶೂನ್ಯ mch_isa_bridge_setup(ಸಂರಚನಾ pci_device *dev, ಶೂನ್ಯ *arg)
{
    u16 bdf = dev->bdf;
    ಪೂರ್ಣಾಙ್ಕ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    ಪ್ರತಿ (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));

        /* activate irq remapping in LPC */

        /* PIRQ[A-D] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT + i, irq);
        /* PIRQ[E-H] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT + i, irq);
    }
    outb(elcr[0], ICH9_LPC_PORT_ELCR1);
    outb(elcr[1], ICH9_LPC_PORT_ELCR2);
    dprintf(1, "Q35 LPC init: elcr=%02x %02x\n", elcr[0], elcr[1]);

    ICH9LpcBDF = bdf;

    mch_isa_lpc_setup(bdf);

    e820_add(ICH9_LPC_RCBA_ADDR, 16*1024, E820_RESERVED);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

ಸ್ಥಿರ ಶೂನ್ಯ storage_ide_setup(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_set_io_region_addr(pci, 0, PORT_ATA1_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 1, PORT_ATA1_CTRL_BASE, 0);
    pci_set_io_region_addr(pci, 2, PORT_ATA2_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 3, PORT_ATA2_CTRL_BASE, 0);
}

/* PIIX3/PIIX4 IDE */
ಸ್ಥಿರ ಶೂನ್ಯ piix_ide_setup(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

ಸ್ಥಿರ ಶೂನ್ಯ pic_ibm_setup(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_set_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ apple_macio_setup(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    /* macio bridge */
    pci_set_io_region_addr(pci, 0, 0x80800000, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ piix4_pm_config_setup(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_INTERRUPT_ಸಾಲು, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_base | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_base + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
ಸ್ಥಿರ ಶೂನ್ಯ piix4_pm_setup(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_setup(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

ಸ್ಥಿರ ಶೂನ್ಯ ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_BASE,
                      (acpi_pm_base + 0x100) | PCI_BASE_ADDRESS_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
ಸ್ಥಿರ ಶೂನ್ಯ ich9_smbus_setup(ಸಂರಚನಾ pci_device *dev, ಶೂನ್ಯ *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

ಸ್ಥಿರ ಶೂನ್ಯ intel_igd_setup(ಸಂರಚನಾ pci_device *dev, ಶೂನ್ಯ *arg)
{
    ಸಂರಚನಾ romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_size = le64_to_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    ಯದಿ (opregion && opregion->size) {
        ಶೂನ್ಯ *addr = memalign_ಮೇಲಿನ(PAGE_SIZE, opregion->size);
        ಯದಿ (!addr) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ;
        }

        ಯದಿ (opregion->copy(opregion, addr, opregion->size) < 0) {
            ಮುಕ್ತ(addr);
            ಪ್ರತ್ಯಾಗಮ;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->size >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    ಯದಿ (bdsm_size && (dev->bdf == pci_to_bdf(0, 2, 0))) {
        ಶೂನ್ಯ *addr = memalign_tmphigh(1024 * 1024, bdsm_size);
        ಯದಿ (!addr) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ;
        }

        e820_add((u32)addr, bdsm_size, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_size >> 20, dev);
    }
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ pci_device_id pci_device_tbl[] = {
    /* PIIX3/PIIX4 PCI to ISA bridge */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0,
               piix_isa_bridge_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0,
               piix_isa_bridge_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC,
               mch_isa_bridge_setup),

    /* STORAGE IDE */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1,
                     PCI_CLASS_STORAGE_IDE, piix_ide_setup),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB,
                     PCI_CLASS_STORAGE_IDE, piix_ide_setup),
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
                     storage_ide_setup),

    /* PIC, IBM, MPIC & MPIC2 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0x0046, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_setup),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0xFFFF, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_setup),

    /* PIIX4 Power Management device (for ACPI) */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3,
               piix4_pm_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_SMBUS,
               ich9_smbus_setup),

    /* 0xff00 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0017, 0xff00, apple_macio_setup),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0022, 0xff00, apple_macio_setup),

    /* Intel IGD OpRegion setup */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
                     intel_igd_setup),

    PCI_DEVICE_END,
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ MCHMmcfgBDF = -1;
ಸ್ಥಿರ ಶೂನ್ಯ mch_mmconfig_setup(u16 bdf);

ಶೂನ್ಯ pci_resume(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU) {
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (PiixPmBDF >= 0) {
        piix4_pm_config_setup(PiixPmBDF);
    }

    ಯದಿ (ICH9LpcBDF >= 0) {
        mch_isa_lpc_setup(ICH9LpcBDF);
    }

    ಯದಿ (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    ಯದಿ(MCHMmcfgBDF >= 0) {
        mch_mmconfig_setup(MCHMmcfgBDF);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_init_device(ಸಂರಚನಾ pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    ಪೂರ್ಣಾಙ್ಕ pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
    ಯದಿ (pin != 0)
        pci_config_writeb(bdf, PCI_INTERRUPT_ಸಾಲು, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, ರಿಕ್ತ);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    ಯದಿ (pci->header_type & PCI_HEADER_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_init_devices(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_enable_default_vga(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ pci_device *pci;

    foreachpci(pci) {
        ಯದಿ (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            ಪ್ರತ್ಯಾಗಮ;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    ಯದಿ (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        ಪ್ರತ್ಯಾಗಮ;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

    ಯಾವತ್ (pci->parent) {
        pci = pci->parent;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ i440fx_mem_addr_setup(ಸಂರಚನಾ pci_device *dev, ಶೂನ್ಯ *arg)
{
    ಯದಿ (RamSize <= 0x80000000)
        pcimem_start = 0x80000000;
    ಅನ್ಯಥಾ ಯದಿ (RamSize <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

ಸ್ಥಿರ ಶೂನ್ಯ mch_mmconfig_setup(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 upper = addr >> 32;
    u32 lower = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, upper);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, lower);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

ಸ್ಥಿರ ಶೂನ್ಯ mch_mem_addr_setup(ಸಂರಚನಾ pci_device *dev, ಶೂನ್ಯ *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 size = Q35_HOST_BRIDGE_PCIEXBAR_SIZE;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_setup(dev->bdf);
    e820_add(addr, size, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_start = addr + size;

    pci_slot_get_irq = mch_pci_slot_get_irq;

    /* setup io address space */
    ಯದಿ (acpi_pm_base < 0x1000)
        pci_io_ಕೆಳಗಿನ_end = 0x10000;
    ಅನ್ಯಥಾ
        pci_io_ಕೆಳಗಿನ_end = acpi_pm_base;
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_setup),
    PCI_DEVICE_END
};

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_init_platform(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, ರಿಕ್ತ);
    }
}

ಸ್ಥಿರ u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    ಯದಿ (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    ಯದಿ (device_id != PCI_DEVICE_ID_REDHAT_ಬೇರು_PORT &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }
    u8 cap = 0;

    ಕುರು {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } ಯಾವತ್ (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_OFFSET) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    ಯದಿ (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        ಯದಿ (cap_len < RES_RESERVE_CAP_SIZE) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            ಪ್ರತ್ಯಾಗಮ 0;
        }
    } ಅನ್ಯಥಾ {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    ಪ್ರತ್ಯಾಗಮ cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
pci_bios_init_bus_rec(ಪೂರ್ಣಾಙ್ಕ bus, u8 *pci_bus)
{
    ಪೂರ್ಣಾಙ್ಕ bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ಯದಿ (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ಯದಿ (class != PCI_CLASS_BRIDGE_PCI) {
            ಅನುವರ್ತ;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        ಯದಿ (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } ಅನ್ಯಥಾ {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        ಯದಿ (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } ಅನ್ಯಥಾ {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        ಯದಿ (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            ಯದಿ (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                ಯದಿ (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    ಯದಿ ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    ಯದಿ (secbus + res_bus > *pci_bus) {
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
        } ಅನ್ಯಥಾ {
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

    ಯದಿ (extraroots) {
        ಯಾವತ್ (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
pci_bios_get_bar(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ bar,
                 ಪೂರ್ಣಾಙ್ಕ *ptype, u64 *psize, ಪೂರ್ಣಾಙ್ಕ *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    ಪೂರ್ಣಾಙ್ಕ is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 mask;

    ಯದಿ (bar == PCI_ROM_SLOT) {
        mask = PCI_ROM_ADDRESS_MASK;
        pci_config_writel(bdf, ofs, mask);
    } ಅನ್ಯಥಾ {
        ಯದಿ (old & PCI_BASE_ADDRESS_SPACE_IO) {
            mask = PCI_BASE_ADDRESS_IO_MASK;
            type = PCI_REGION_TYPE_IO;
        } ಅನ್ಯಥಾ {
            mask = PCI_BASE_ADDRESS_MEM_MASK;
            ಯದಿ (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK)
                    == PCI_BASE_ADDRESS_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    ಯದಿ (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 ಮೇಲಿನ = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)ಮೇಲಿನ << 32);
        mask |= ((u64)0xffffffff << 32);
        *psize = (~(val & mask)) + 1;
    } ಅನ್ಯಥಾ {
        *psize = ((~(val & mask)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pci_bios_bridge_region_is64(ಸಂರಚನಾ pci_region *r,
                                 ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ type)
{
    ಯದಿ (type != PCI_REGION_TYPE_PREFMEM)
        ಪ್ರತ್ಯಾಗಮ 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
    ಯದಿ (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0x0);
    }
    ಯದಿ ((pmem & PCI_PREF_RANGE_TYPE_MASK) != PCI_PREF_RANGE_TYPE_64)
       ಪ್ರತ್ಯಾಗಮ 0;
    ಸಂರಚನಾ pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, ಗಂಟು) {
        ಯದಿ (!entry->is64)
            ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ u64 pci_region_align(ಸಂರಚನಾ pci_region *r)
{
    ಸಂರಚನಾ pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, ಗಂಟು) {
        // The first entry in the sorted list has the largest alignment
        ಪ್ರತ್ಯಾಗಮ entry->align;
    }
    ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ u64 pci_region_sum(ಸಂರಚನಾ pci_region *r)
{
    u64 sum = 0;
    ಸಂರಚನಾ pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, ಗಂಟು) {
        sum += entry->size;
    }
    ಪ್ರತ್ಯಾಗಮ sum;
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_region_migrate_64bit_entries(ಸಂರಚನಾ pci_region *from,
                                             ಸಂರಚನಾ pci_region *to)
{
    ಸಂರಚನಾ hlist_ಗಂಟು *n, **last = &to->list.first;
    ಸಂರಚನಾ pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &from->list, ಗಂಟು) {
        ಯದಿ (!entry->is64)
            ಅನುವರ್ತ;
        ಯದಿ (entry->dev->class == PCI_CLASS_SERIAL_USB)
            ಅನುವರ್ತ;
        ಯದಿ (entry->dev->class == PCI_CLASS_STORAGE_NVME)
            ಅನುವರ್ತ;
        // Move from source list to destination list.
        hlist_del(&entry->ಗಂಟು);
        hlist_add(&entry->ಗಂಟು, last);
        last = &entry->ಗಂಟು.next;
    }
}

ಸ್ಥಿರ ಸಂರಚನಾ pci_region_entry *
pci_region_create_entry(ಸಂರಚನಾ pci_bus *bus, ಸಂರಚನಾ pci_device *dev,
                        ಪೂರ್ಣಾಙ್ಕ bar, u64 size, u64 align, ಪೂರ್ಣಾಙ್ಕ type, ಪೂರ್ಣಾಙ್ಕ is64)
{
    ಸಂರಚನಾ pci_region_entry *entry = malloc_tmp(ಪರಿಮಾಣ(*entry));
    ಯದಿ (!entry) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(entry, 0, ಪರಿಮಾಣ(*entry));
    entry->dev = dev;
    entry->bar = bar;
    entry->size = size;
    entry->align = align;
    entry->is64 = is64;
    entry->type = type;
    // Insert into list in sorted order.
    ಸಂರಚನಾ hlist_ಗಂಟು **pprev;
    ಸಂರಚನಾ pci_region_entry *pos;
    hlist_for_each_entry_pprev(pos, pprev, &bus->r[type].list, ಗಂಟು) {
        ಯದಿ (pos->align < align || (pos->align == align && pos->size < size))
            ಭಙ್ಗ;
    }
    hlist_add(&entry->ಗಂಟು, pprev);
    ಪ್ರತ್ಯಾಗಮ entry;
}

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

ಸ್ಥಿರ hotplug_type_t pci_bus_hotplug_support(ಸಂರಚನಾ pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    ಯದಿ (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 port_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        ಯದಿ (port_type == PCI_EXP_TYPE_PCI_BRIDGE)
            ಗಚ್ಛ check_shpc;

        u8 downstream_port = (port_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (port_type == PCI_EXP_TYPE_ಬೇರು_PORT);
        /*
         * PCI Express SPEC, 7.8.2:
         *   Slot Implemented – When Set, this bit indicates that the Link
         *   HwInit associated with this Port is connected to a slot (as
         *   compared to being connected to a system-integrated device or
         *   being disabled).
         *   This bit is valid for Downstream Ports. This bit is undefined
         *   for Upstream Ports.
         */
        u16 slot_implemented = pcie_flags & PCI_EXP_FLAGS_SLOT;

        ಪ್ರತ್ಯಾಗಮ downstream_port && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    ಪ್ರತ್ಯಾಗಮ !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pci_bridge_has_region(ಸಂರಚನಾ pci_device *pci,
                                 ಗಣನಾ pci_region_type region_type)
{
    u8 base;

    ವಿಕಲ್ಪ (region_type) {
        ಪಕ್ಷ PCI_REGION_TYPE_IO:
            base = PCI_IO_BASE;
            ಭಙ್ಗ;
        ಪಕ್ಷ PCI_REGION_TYPE_PREFMEM:
            base = PCI_PREF_MEMORY_BASE;
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            /* Regular memory support is mandatory */
            ಪ್ರತ್ಯಾಗಮ 1;
    }

    pci_config_writeb(pci->bdf, base, 0xFF);

    ಪ್ರತ್ಯಾಗಮ pci_config_readb(pci->bdf, base) != 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pci_bios_check_devices(ಸಂರಚನಾ pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        ಸಂರಚನಾ pci_bus *bus = &busses[pci_bdf_to_bus(pci->bdf)];
        ಯದಿ (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i = 0; i < PCI_ಅಂಕೆ_REGIONS; i++) {
            ಯದಿ ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_ಅಂಕೆ_REGIONS && i < PCI_ROM_SLOT))
                ಅನುವರ್ತ;
            ಪೂರ್ಣಾಙ್ಕ type, is64;
            u64 size;
            pci_bios_get_bar(pci, i, &type, &size, &is64);
            ಯದಿ (size == 0)
                ಅನುವರ್ತ;

            ಯದಿ (type != PCI_REGION_TYPE_IO && size < PCI_DEVICE_MEM_MIN)
                size = PCI_DEVICE_MEM_MIN;
            ಸಂರಚನಾ pci_region_entry *entry = pci_region_create_entry(
                bus, pci, i, size, size, type, is64);
            ಯದಿ (!entry)
                ಪ್ರತ್ಯಾಗಮ -1;

            ಯದಿ (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    ಪೂರ್ಣಾಙ್ಕ secondary_bus;
    ಪ್ರತಿ (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        ಸಂರಚನಾ pci_bus *s = &busses[secondary_bus];
        ಯದಿ (!s->bus_dev)
            ಅನುವರ್ತ;
        ಸಂರಚನಾ pci_bus *parent = &busses[pci_bdf_to_bus(s->bus_dev->bdf)];
        ಯದಿ (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        ಪೂರ್ಣಾಙ್ಕ type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        ಪ್ರತಿ (type = 0; type < PCI_REGION_TYPE_ಎಣಿಕೆ; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            ಯದಿ (!pci_bridge_has_region(s->bus_dev, type))
                ಅನುವರ್ತ;
            u64 size = 0;
            ಯದಿ (qemu_cap) {
                u32 tmp_size;
                u64 tmp_size_64;
                ವಿಕಲ್ಪ(type) {
                ಪಕ್ಷ PCI_REGION_TYPE_IO:
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    ಯದಿ (tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    }
                    ಭಙ್ಗ;
                ಪಕ್ಷ PCI_REGION_TYPE_MEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    ಯದಿ (tmp_size != (u32)-1) {
                        size = tmp_size;
                    }
                    ಭಙ್ಗ;
                ಪಕ್ಷ PCI_REGION_TYPE_PREFMEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    ಯದಿ (tmp_size != (u32)-1 && tmp_size_64 == (u64)-1) {
                        size = tmp_size;
                    } ಅನ್ಯಥಾ ಯದಿ (tmp_size == (u32)-1 && tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    } ಅನ್ಯಥಾ ಯದಿ (tmp_size != (u32)-1 && tmp_size_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    ಭಙ್ಗ;
                ಸ್ವಭಾವ:
                    ಭಙ್ಗ;
                }
            }
            ಯದಿ (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 sum = pci_region_sum(&s->r[type]);
            ಪೂರ್ಣಾಙ್ಕ is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            ಪೂರ್ಣಾಙ್ಕ resource_optional = 0;
            ಯದಿ (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 ತುದಿ_align = pci_mem64_ತುದಿ >> 11;
            ಯದಿ (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (ತುದಿ_align > align))
                align = ತುದಿ_align;
            ಯದಿ (align > sum && hotplug_support && !resource_optional)
                sum = align; /* reserve min size for hot-plug */
            ಯದಿ (size > sum) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        size, region_type_name[type]);
                ಯದಿ (type != PCI_REGION_TYPE_IO) {
                    size = ALIGN(size, align);
                }
            } ಅನ್ಯಥಾ {
                size = ALIGN(sum, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            ಸಂರಚನಾ pci_region_entry *entry = pci_region_create_entry(
                parent, s->bus_dev, -1, size, align, type, is64);
            ಯದಿ (!entry)
                ಪ್ರತ್ಯಾಗಮ -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      entry->dev->secondary_bus, size,
                      region_type_name[entry->type]);
        }
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pci_bios_init_ಬೇರು_regions_io(ಸಂರಚನಾ pci_bus *bus)
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
    ಸಂರಚನಾ pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 sum = pci_region_sum(r_io);
    ಯದಿ (sum < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->base = 0xc000;
    } ಅನ್ಯಥಾ ಯದಿ (sum < pci_io_ಕೆಳಗಿನ_end - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->base = 0x1000;
    } ಅನ್ಯಥಾ {
        /* not enouth io address space -> error out */
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->base, r_io->base + sum - 1);
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pci_bios_init_ಬೇರು_regions_mem(ಸಂರಚನಾ pci_bus *bus)
{
    ಸಂರಚನಾ pci_region *r_end = &bus->r[PCI_REGION_TYPE_PREFMEM];
    ಸಂರಚನಾ pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    ಯದಿ (pci_region_align(r_start) < pci_region_align(r_end)) {
        // Swap regions to improve alignment.
        r_end = r_start;
        r_start = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 sum = pci_region_sum(r_end);
    u64 align = pci_region_align(r_end);
    r_end->base = ALIGN_DOWN((pcimem_end - sum), align);
    sum = pci_region_sum(r_start);
    align = pci_region_align(r_start);
    r_start->base = ALIGN_DOWN((r_end->base - sum), align);

    ಯದಿ ((r_start->base < pcimem_start) ||
         (r_start->base > pcimem_end))
        // Memory range requested is larger than available.
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ 0;
}

#define PCI_IO_SHIFT            8
#define PCI_MEMORY_SHIFT        16
#define PCI_PREF_MEMORY_SHIFT   16

ಸ್ಥಿರ ಶೂನ್ಯ
pci_region_map_one_entry(ಸಂರಚನಾ pci_region_entry *entry, u64 addr)
{
    ಯದಿ (entry->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                entry->dev,
                entry->bar, addr, entry->size, region_type_name[entry->type]);

        pci_set_io_region_addr(entry->dev, entry->bar, addr, entry->is64);
        ಪ್ರತ್ಯಾಗಮ;
    }

    u16 bdf = entry->dev->bdf;
    u64 limit = addr + entry->size - 1;
    ಯದಿ (entry->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_BASE, addr >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_BASE_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_LIMIT, limit >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_LIMIT_UPPER16, 0);
    }
    ಯದಿ (entry->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_MEMORY_BASE, addr >> PCI_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_MEMORY_LIMIT, limit >> PCI_MEMORY_SHIFT);
    }
    ಯದಿ (entry->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_MEMORY_BASE, addr >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_PREF_MEMORY_LIMIT, limit >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writel(bdf, PCI_PREF_BASE_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_LIMIT_UPPER32, limit >> 32);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_region_map_entries(ಸಂರಚನಾ pci_bus *busses, ಸಂರಚನಾ pci_region *r)
{
    ಸಂರಚನಾ hlist_ಗಂಟು *n;
    ಸಂರಚನಾ pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &r->list, ಗಂಟು) {
        u64 addr = r->base;
        r->base += entry->size;
        ಯದಿ (entry->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[entry->dev->secondary_bus].r[entry->type].base = addr;
        pci_region_map_one_entry(entry, addr);
        hlist_del(&entry->ಗಂಟು);
        ಮುಕ್ತ(entry);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ pci_bios_map_devices(ಸಂರಚನಾ pci_bus *busses)
{
    ಯದಿ (pci_bios_init_ಬೇರು_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_end);
    ಯದಿ (pci_pad_mem64 || pci_bios_init_ಬೇರು_regions_mem(busses)) {
        ಸಂರಚನಾ pci_region r64_mem, r64_pref;
        r64_mem.list.first = ರಿಕ್ತ;
        r64_pref.list.first = ರಿಕ್ತ;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        ಯದಿ (pci_bios_init_ಬೇರು_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 sum_mem = pci_region_sum(&r64_mem);
        u64 sum_pref = pci_region_sum(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.base = le64_to_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        ಯದಿ (r64_mem.base < 0x100000000LL + RamSizeOver4G)
            r64_mem.base = 0x100000000LL + RamSizeOver4G;
        ಯದಿ (pci_mem64_ತುದಿ) {
            u64 size = (ALIGN(sum_mem, (1LL<<30)) +
                        ALIGN(sum_pref, (1LL<<30)));
            ಯದಿ (pci_pad_mem64)
                size = ALIGN(size, pci_mem64_ತುದಿ >> 3);
            ಯದಿ (r64_mem.base < pci_mem64_ತುದಿ - size) {
                r64_mem.base = pci_mem64_ತುದಿ - size;
            }
            ಯದಿ (e820_is_used(r64_mem.base, size))
                r64_mem.base -= size;
        }
        r64_mem.base = ALIGN(r64_mem.base, align_mem);
        r64_mem.base = ALIGN(r64_mem.base, (1LL<<30));    // 1G hugepage
        r64_pref.base = r64_mem.base + sum_mem;
        r64_pref.base = ALIGN(r64_pref.base, align_pref);
        r64_pref.base = ALIGN(r64_pref.base, (1LL<<30));  // 1G hugepage
        pcimem64_start = r64_mem.base;
        pcimem64_end = r64_pref.base + sum_pref;
        pcimem64_end = ALIGN(pcimem64_end, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_start, pcimem64_end);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } ಅನ್ಯಥಾ {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    ಪೂರ್ಣಾಙ್ಕ bus;
    ಪ್ರತಿ (bus = 0; bus<=MaxPCIBus; bus++) {
        ಪೂರ್ಣಾಙ್ಕ type;
        ಪ್ರತಿ (type = 0; type < PCI_REGION_TYPE_ಎಣಿಕೆ; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

ಶೂನ್ಯ
pci_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "pci setup\n");

    ಯದಿ (CPUPhysBits) {
        pci_mem64_ತುದಿ = 1LL << CPUPhysBits;
        ಯದಿ (CPUPhysBits > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            ಯದಿ (RamSizeOver4G < (1LL << 40)) {
                pci_mem64_ತುದಿ = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    ಯದಿ (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = RamSizeOver4G ? 1 : 0;

    ಯದಿ (!CPULongMode)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    ಯದಿ (pci_probe_host() != 0) {
        ಪ್ರತ್ಯಾಗಮ;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = RamSize;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    ಸಂರಚನಾ pci_bus *busses = malloc_tmp(ಪರಿಮಾಣ(*busses) * (MaxPCIBus + 1));
    ಯದಿ (!busses) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(busses, 0, ಪರಿಮಾಣ(*busses) * (MaxPCIBus + 1));
    ಯದಿ (pci_bios_check_devices(busses))
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    ಮುಕ್ತ(busses);

    pci_enable_default_vga();
}
