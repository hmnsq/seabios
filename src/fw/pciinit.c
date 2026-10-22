// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "byteorder.h" // le64_to_cpu
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#അന്തര്ഭാവ "dev-piix.h" // PIIX_*
#അന്തര്ഭാവ "e820map.h" // e820_add
#അന്തര്ഭാവ "hw/ata.h" // PORT_ATA1_CMD_BASE
#അന്തര്ഭാവ "hw/pci.h" // pci_config_readl
#അന്തര്ഭാവ "hw/pcidevice.h" // pci_probe_devices
#അന്തര്ഭാവ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#അന്തര്ഭാവ "hw/pci_regs.h" // PCI_COMMAND
#അന്തര്ഭാവ "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#അന്തര്ഭാവ "list.h" // struct hlist_node
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "paravirt.h" // RamSize
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // pci_setup
#അന്തര്ഭാവ "x86.h" // outb

#define PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#define PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#define PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#define PCI_ROM_SLOT 6
#define PCI_അക്കം_REGIONS 7
#define PCI_BRIDGE_അക്കം_REGIONS 2

എണ്ണൽ pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_എണ്ണം,
};

സ്ഥിതം സ്ഥിരം അക്ഷരം *region_type_name[] = {
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
സ്ഥിതം u64 pci_io_താഴെയുള്ള_end = 0xa000;
സ്ഥിതം u64 pci_mem64_മുകൾ  = 0;
സ്ഥിതം u32 pci_pad_mem64  = -1;

ഘടന pci_region_entry {
    ഘടന pci_device *dev;
    പൂർണ്ണസംഖ്യ bar;
    u64 size;
    u64 align;
    പൂർണ്ണസംഖ്യ is64;
    എണ്ണൽ pci_region_type type;
    ഘടന hlist_കണ്ണി കണ്ണി;
};

ഘടന pci_region {
    /* pci region assignments */
    u64 base;
    ഘടന hlist_head list;
};

ഘടന pci_bus {
    ഘടന pci_region r[PCI_REGION_TYPE_എണ്ണം];
    ഘടന pci_device *bus_dev;
};

സ്ഥിതം u32 pci_bar(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ region_അക്കം)
{
    എങ്കിൽ (region_അക്കം != PCI_ROM_SLOT) {
        തിരികെ PCI_BASE_ADDRESS_0 + region_അക്കം * 4;
    }

#define PCI_HEADER_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
    തിരികെ type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

സ്ഥിതം ശൂന്യം
pci_set_io_region_addr(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ bar, u64 addr, പൂർണ്ണസംഖ്യ is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    എങ്കിൽ (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
സ്ഥിരം u8 pci_irqs[4] = {
    10, 10, 11, 11
};

സ്ഥിതം പൂർണ്ണസംഖ്യ dummy_pci_slot_get_irq(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    തിരികെ 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ (*pci_slot_get_irq)(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
സ്ഥിതം പൂർണ്ണസംഖ്യ piix_pci_slot_get_irq(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ pin)
{
    പൂർണ്ണസംഖ്യ slot_addend = 0;

    ആകുമ്പോൾ (pci->parent != ശൂന്യ) {
        slot_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_to_dev(pci->bdf) - 1;
    തിരികെ pci_irqs[(pin - 1 + slot_addend) & 3];
}

സ്ഥിതം പൂർണ്ണസംഖ്യ mch_pci_slot_get_irq(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ pin)
{
    പൂർണ്ണസംഖ്യ pin_addend = 0;
    ആകുമ്പോൾ (pci->parent != ശൂന്യ) {
        pin_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_to_dev(pci->bdf);
    എങ്കിൽ (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        തിരികെ pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    തിരികെ pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
സ്ഥിതം ശൂന്യം piix_isa_bridge_setup(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    പൂർണ്ണസംഖ്യ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    വേണ്ടി (i = 0; i < 4; i++) {
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

സ്ഥിതം ശൂന്യം mch_isa_lpc_setup(u16 bdf)
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

സ്ഥിതം പൂർണ്ണസംഖ്യ ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
സ്ഥിതം ശൂന്യം mch_isa_bridge_setup(ഘടന pci_device *dev, ശൂന്യം *arg)
{
    u16 bdf = dev->bdf;
    പൂർണ്ണസംഖ്യ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    വേണ്ടി (i = 0; i < 4; i++) {
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

സ്ഥിതം ശൂന്യം storage_ide_setup(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_set_io_region_addr(pci, 0, PORT_ATA1_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 1, PORT_ATA1_CTRL_BASE, 0);
    pci_set_io_region_addr(pci, 2, PORT_ATA2_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 3, PORT_ATA2_CTRL_BASE, 0);
}

/* PIIX3/PIIX4 IDE */
സ്ഥിതം ശൂന്യം piix_ide_setup(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

സ്ഥിതം ശൂന്യം pic_ibm_setup(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_set_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

സ്ഥിതം ശൂന്യം apple_macio_setup(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    /* macio bridge */
    pci_set_io_region_addr(pci, 0, 0x80800000, 0);
}

സ്ഥിതം ശൂന്യം piix4_pm_config_setup(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_INTERRUPT_വരി, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_base | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_base + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
സ്ഥിതം ശൂന്യം piix4_pm_setup(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_setup(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

സ്ഥിതം ശൂന്യം ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_BASE,
                      (acpi_pm_base + 0x100) | PCI_BASE_ADDRESS_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
സ്ഥിതം ശൂന്യം ich9_smbus_setup(ഘടന pci_device *dev, ശൂന്യം *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

സ്ഥിതം ശൂന്യം intel_igd_setup(ഘടന pci_device *dev, ശൂന്യം *arg)
{
    ഘടന romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_size = le64_to_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    എങ്കിൽ (opregion && opregion->size) {
        ശൂന്യം *addr = memalign_മുകളിലെ(PAGE_SIZE, opregion->size);
        എങ്കിൽ (!addr) {
            warn_noalloc();
            തിരികെ;
        }

        എങ്കിൽ (opregion->copy(opregion, addr, opregion->size) < 0) {
            സ്വതന്ത്രം(addr);
            തിരികെ;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->size >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    എങ്കിൽ (bdsm_size && (dev->bdf == pci_to_bdf(0, 2, 0))) {
        ശൂന്യം *addr = memalign_tmphigh(1024 * 1024, bdsm_size);
        എങ്കിൽ (!addr) {
            warn_noalloc();
            തിരികെ;
        }

        e820_add((u32)addr, bdsm_size, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_size >> 20, dev);
    }
}

സ്ഥിതം സ്ഥിരം ഘടന pci_device_id pci_device_tbl[] = {
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

സ്ഥിതം പൂർണ്ണസംഖ്യ MCHMmcfgBDF = -1;
സ്ഥിതം ശൂന്യം mch_mmconfig_setup(u16 bdf);

ശൂന്യം pci_resume(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU) {
        തിരികെ;
    }

    എങ്കിൽ (PiixPmBDF >= 0) {
        piix4_pm_config_setup(PiixPmBDF);
    }

    എങ്കിൽ (ICH9LpcBDF >= 0) {
        mch_isa_lpc_setup(ICH9LpcBDF);
    }

    എങ്കിൽ (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    എങ്കിൽ(MCHMmcfgBDF >= 0) {
        mch_mmconfig_setup(MCHMmcfgBDF);
    }
}

സ്ഥിതം ശൂന്യം pci_bios_init_device(ഘടന pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    പൂർണ്ണസംഖ്യ pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
    എങ്കിൽ (pin != 0)
        pci_config_writeb(bdf, PCI_INTERRUPT_വരി, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, ശൂന്യ);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    എങ്കിൽ (pci->header_type & PCI_HEADER_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

സ്ഥിതം ശൂന്യം pci_bios_init_devices(ശൂന്യം)
{
    ഘടന pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

സ്ഥിതം ശൂന്യം pci_enable_default_vga(ശൂന്യം)
{
    ഘടന pci_device *pci;

    foreachpci(pci) {
        എങ്കിൽ (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            തിരികെ;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    എങ്കിൽ (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        തിരികെ;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

    ആകുമ്പോൾ (pci->parent) {
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

സ്ഥിതം ശൂന്യം i440fx_mem_addr_setup(ഘടന pci_device *dev, ശൂന്യം *arg)
{
    എങ്കിൽ (RamSize <= 0x80000000)
        pcimem_start = 0x80000000;
    അല്ലെങ്കിൽ എങ്കിൽ (RamSize <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

സ്ഥിതം ശൂന്യം mch_mmconfig_setup(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 upper = addr >> 32;
    u32 lower = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, upper);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, lower);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

സ്ഥിതം ശൂന്യം mch_mem_addr_setup(ഘടന pci_device *dev, ശൂന്യം *arg)
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
    എങ്കിൽ (acpi_pm_base < 0x1000)
        pci_io_താഴെയുള്ള_end = 0x10000;
    അല്ലെങ്കിൽ
        pci_io_താഴെയുള്ള_end = acpi_pm_base;
}

സ്ഥിതം സ്ഥിരം ഘടന pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_setup),
    PCI_DEVICE_END
};

സ്ഥിതം ശൂന്യം pci_bios_init_platform(ശൂന്യം)
{
    ഘടന pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, ശൂന്യ);
    }
}

സ്ഥിതം u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    എങ്കിൽ (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        തിരികെ 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    എങ്കിൽ (device_id != PCI_DEVICE_ID_REDHAT_വേര്_PORT &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        തിരികെ 0;
    }
    u8 cap = 0;

    ചെയ്യുക {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } ആകുമ്പോൾ (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_OFFSET) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    എങ്കിൽ (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        എങ്കിൽ (cap_len < RES_RESERVE_CAP_SIZE) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            തിരികെ 0;
        }
    } അല്ലെങ്കിൽ {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    തിരികെ cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

സ്ഥിതം ശൂന്യം
pci_bios_init_bus_rec(പൂർണ്ണസംഖ്യ bus, u8 *pci_bus)
{
    പൂർണ്ണസംഖ്യ bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        എങ്കിൽ (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        എങ്കിൽ (class != PCI_CLASS_BRIDGE_PCI) {
            തുടരുക;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        എങ്കിൽ (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } അല്ലെങ്കിൽ {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        എങ്കിൽ (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } അല്ലെങ്കിൽ {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        എങ്കിൽ (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            എങ്കിൽ (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                എങ്കിൽ (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    എങ്കിൽ ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    എങ്കിൽ (secbus + res_bus > *pci_bus) {
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
        } അല്ലെങ്കിൽ {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

സ്ഥിതം ശൂന്യം
pci_bios_init_bus(ശൂന്യം)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    എങ്കിൽ (extraroots) {
        ആകുമ്പോൾ (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

സ്ഥിതം ശൂന്യം
pci_bios_get_bar(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ bar,
                 പൂർണ്ണസംഖ്യ *ptype, u64 *psize, പൂർണ്ണസംഖ്യ *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    പൂർണ്ണസംഖ്യ is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 mask;

    എങ്കിൽ (bar == PCI_ROM_SLOT) {
        mask = PCI_ROM_ADDRESS_MASK;
        pci_config_writel(bdf, ofs, mask);
    } അല്ലെങ്കിൽ {
        എങ്കിൽ (old & PCI_BASE_ADDRESS_SPACE_IO) {
            mask = PCI_BASE_ADDRESS_IO_MASK;
            type = PCI_REGION_TYPE_IO;
        } അല്ലെങ്കിൽ {
            mask = PCI_BASE_ADDRESS_MEM_MASK;
            എങ്കിൽ (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK)
                    == PCI_BASE_ADDRESS_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    എങ്കിൽ (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 മുകളിലെ = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)മുകളിലെ << 32);
        mask |= ((u64)0xffffffff << 32);
        *psize = (~(val & mask)) + 1;
    } അല്ലെങ്കിൽ {
        *psize = ((~(val & mask)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ pci_bios_bridge_region_is64(ഘടന pci_region *r,
                                 ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ type)
{
    എങ്കിൽ (type != PCI_REGION_TYPE_PREFMEM)
        തിരികെ 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
    എങ്കിൽ (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0x0);
    }
    എങ്കിൽ ((pmem & PCI_PREF_RANGE_TYPE_MASK) != PCI_PREF_RANGE_TYPE_64)
       തിരികെ 0;
    ഘടന pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, കണ്ണി) {
        എങ്കിൽ (!entry->is64)
            തിരികെ 0;
    }
    തിരികെ 1;
}

സ്ഥിതം u64 pci_region_align(ഘടന pci_region *r)
{
    ഘടന pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, കണ്ണി) {
        // The first entry in the sorted list has the largest alignment
        തിരികെ entry->align;
    }
    തിരികെ 1;
}

സ്ഥിതം u64 pci_region_sum(ഘടന pci_region *r)
{
    u64 sum = 0;
    ഘടന pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, കണ്ണി) {
        sum += entry->size;
    }
    തിരികെ sum;
}

സ്ഥിതം ശൂന്യം pci_region_migrate_64bit_entries(ഘടന pci_region *from,
                                             ഘടന pci_region *to)
{
    ഘടന hlist_കണ്ണി *n, **last = &to->list.first;
    ഘടന pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &from->list, കണ്ണി) {
        എങ്കിൽ (!entry->is64)
            തുടരുക;
        എങ്കിൽ (entry->dev->class == PCI_CLASS_SERIAL_USB)
            തുടരുക;
        എങ്കിൽ (entry->dev->class == PCI_CLASS_STORAGE_NVME)
            തുടരുക;
        // Move from source list to destination list.
        hlist_del(&entry->കണ്ണി);
        hlist_add(&entry->കണ്ണി, last);
        last = &entry->കണ്ണി.next;
    }
}

സ്ഥിതം ഘടന pci_region_entry *
pci_region_create_entry(ഘടന pci_bus *bus, ഘടന pci_device *dev,
                        പൂർണ്ണസംഖ്യ bar, u64 size, u64 align, പൂർണ്ണസംഖ്യ type, പൂർണ്ണസംഖ്യ is64)
{
    ഘടന pci_region_entry *entry = malloc_tmp(വലുപ്പം(*entry));
    എങ്കിൽ (!entry) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(entry, 0, വലുപ്പം(*entry));
    entry->dev = dev;
    entry->bar = bar;
    entry->size = size;
    entry->align = align;
    entry->is64 = is64;
    entry->type = type;
    // Insert into list in sorted order.
    ഘടന hlist_കണ്ണി **pprev;
    ഘടന pci_region_entry *pos;
    hlist_for_each_entry_pprev(pos, pprev, &bus->r[type].list, കണ്ണി) {
        എങ്കിൽ (pos->align < align || (pos->align == align && pos->size < size))
            നിർത്തുക;
    }
    hlist_add(&entry->കണ്ണി, pprev);
    തിരികെ entry;
}

തരനിർവചനം എണ്ണൽ hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

സ്ഥിതം hotplug_type_t pci_bus_hotplug_support(ഘടന pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    എങ്കിൽ (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 port_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        എങ്കിൽ (port_type == PCI_EXP_TYPE_PCI_BRIDGE)
            പോകുക check_shpc;

        u8 downstream_port = (port_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (port_type == PCI_EXP_TYPE_വേര്_PORT);
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

        തിരികെ downstream_port && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    തിരികെ !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ pci_bridge_has_region(ഘടന pci_device *pci,
                                 എണ്ണൽ pci_region_type region_type)
{
    u8 base;

    തിരഞ്ഞെടുപ്പ് (region_type) {
        അവസ്ഥ PCI_REGION_TYPE_IO:
            base = PCI_IO_BASE;
            നിർത്തുക;
        അവസ്ഥ PCI_REGION_TYPE_PREFMEM:
            base = PCI_PREF_MEMORY_BASE;
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            /* Regular memory support is mandatory */
            തിരികെ 1;
    }

    pci_config_writeb(pci->bdf, base, 0xFF);

    തിരികെ pci_config_readb(pci->bdf, base) != 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ pci_bios_check_devices(ഘടന pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        ഘടന pci_bus *bus = &busses[pci_bdf_to_bus(pci->bdf)];
        എങ്കിൽ (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i = 0; i < PCI_അക്കം_REGIONS; i++) {
            എങ്കിൽ ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_അക്കം_REGIONS && i < PCI_ROM_SLOT))
                തുടരുക;
            പൂർണ്ണസംഖ്യ type, is64;
            u64 size;
            pci_bios_get_bar(pci, i, &type, &size, &is64);
            എങ്കിൽ (size == 0)
                തുടരുക;

            എങ്കിൽ (type != PCI_REGION_TYPE_IO && size < PCI_DEVICE_MEM_MIN)
                size = PCI_DEVICE_MEM_MIN;
            ഘടന pci_region_entry *entry = pci_region_create_entry(
                bus, pci, i, size, size, type, is64);
            എങ്കിൽ (!entry)
                തിരികെ -1;

            എങ്കിൽ (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    പൂർണ്ണസംഖ്യ secondary_bus;
    വേണ്ടി (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        ഘടന pci_bus *s = &busses[secondary_bus];
        എങ്കിൽ (!s->bus_dev)
            തുടരുക;
        ഘടന pci_bus *parent = &busses[pci_bdf_to_bus(s->bus_dev->bdf)];
        എങ്കിൽ (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        പൂർണ്ണസംഖ്യ type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        വേണ്ടി (type = 0; type < PCI_REGION_TYPE_എണ്ണം; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            എങ്കിൽ (!pci_bridge_has_region(s->bus_dev, type))
                തുടരുക;
            u64 size = 0;
            എങ്കിൽ (qemu_cap) {
                u32 tmp_size;
                u64 tmp_size_64;
                തിരഞ്ഞെടുപ്പ്(type) {
                അവസ്ഥ PCI_REGION_TYPE_IO:
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    എങ്കിൽ (tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    }
                    നിർത്തുക;
                അവസ്ഥ PCI_REGION_TYPE_MEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    എങ്കിൽ (tmp_size != (u32)-1) {
                        size = tmp_size;
                    }
                    നിർത്തുക;
                അവസ്ഥ PCI_REGION_TYPE_PREFMEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    എങ്കിൽ (tmp_size != (u32)-1 && tmp_size_64 == (u64)-1) {
                        size = tmp_size;
                    } അല്ലെങ്കിൽ എങ്കിൽ (tmp_size == (u32)-1 && tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    } അല്ലെങ്കിൽ എങ്കിൽ (tmp_size != (u32)-1 && tmp_size_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    നിർത്തുക;
                സ്ഥിരസ്ഥിതി:
                    നിർത്തുക;
                }
            }
            എങ്കിൽ (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 sum = pci_region_sum(&s->r[type]);
            പൂർണ്ണസംഖ്യ is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            പൂർണ്ണസംഖ്യ resource_optional = 0;
            എങ്കിൽ (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 മുകൾ_align = pci_mem64_മുകൾ >> 11;
            എങ്കിൽ (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (മുകൾ_align > align))
                align = മുകൾ_align;
            എങ്കിൽ (align > sum && hotplug_support && !resource_optional)
                sum = align; /* reserve min size for hot-plug */
            എങ്കിൽ (size > sum) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        size, region_type_name[type]);
                എങ്കിൽ (type != PCI_REGION_TYPE_IO) {
                    size = ALIGN(size, align);
                }
            } അല്ലെങ്കിൽ {
                size = ALIGN(sum, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            ഘടന pci_region_entry *entry = pci_region_create_entry(
                parent, s->bus_dev, -1, size, align, type, is64);
            എങ്കിൽ (!entry)
                തിരികെ -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      entry->dev->secondary_bus, size,
                      region_type_name[entry->type]);
        }
    }
    തിരികെ 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
സ്ഥിതം പൂർണ്ണസംഖ്യ pci_bios_init_വേര്_regions_io(ഘടന pci_bus *bus)
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
    ഘടന pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 sum = pci_region_sum(r_io);
    എങ്കിൽ (sum < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->base = 0xc000;
    } അല്ലെങ്കിൽ എങ്കിൽ (sum < pci_io_താഴെയുള്ള_end - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->base = 0x1000;
    } അല്ലെങ്കിൽ {
        /* not enouth io address space -> error out */
        തിരികെ -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->base, r_io->base + sum - 1);
    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ pci_bios_init_വേര്_regions_mem(ഘടന pci_bus *bus)
{
    ഘടന pci_region *r_end = &bus->r[PCI_REGION_TYPE_PREFMEM];
    ഘടന pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    എങ്കിൽ (pci_region_align(r_start) < pci_region_align(r_end)) {
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

    എങ്കിൽ ((r_start->base < pcimem_start) ||
         (r_start->base > pcimem_end))
        // Memory range requested is larger than available.
        തിരികെ -1;
    തിരികെ 0;
}

#define PCI_IO_SHIFT            8
#define PCI_MEMORY_SHIFT        16
#define PCI_PREF_MEMORY_SHIFT   16

സ്ഥിതം ശൂന്യം
pci_region_map_one_entry(ഘടന pci_region_entry *entry, u64 addr)
{
    എങ്കിൽ (entry->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                entry->dev,
                entry->bar, addr, entry->size, region_type_name[entry->type]);

        pci_set_io_region_addr(entry->dev, entry->bar, addr, entry->is64);
        തിരികെ;
    }

    u16 bdf = entry->dev->bdf;
    u64 limit = addr + entry->size - 1;
    എങ്കിൽ (entry->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_BASE, addr >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_BASE_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_LIMIT, limit >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_LIMIT_UPPER16, 0);
    }
    എങ്കിൽ (entry->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_MEMORY_BASE, addr >> PCI_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_MEMORY_LIMIT, limit >> PCI_MEMORY_SHIFT);
    }
    എങ്കിൽ (entry->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_MEMORY_BASE, addr >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_PREF_MEMORY_LIMIT, limit >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writel(bdf, PCI_PREF_BASE_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_LIMIT_UPPER32, limit >> 32);
    }
}

സ്ഥിതം ശൂന്യം pci_region_map_entries(ഘടന pci_bus *busses, ഘടന pci_region *r)
{
    ഘടന hlist_കണ്ണി *n;
    ഘടന pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &r->list, കണ്ണി) {
        u64 addr = r->base;
        r->base += entry->size;
        എങ്കിൽ (entry->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[entry->dev->secondary_bus].r[entry->type].base = addr;
        pci_region_map_one_entry(entry, addr);
        hlist_del(&entry->കണ്ണി);
        സ്വതന്ത്രം(entry);
    }
}

സ്ഥിതം ശൂന്യം pci_bios_map_devices(ഘടന pci_bus *busses)
{
    എങ്കിൽ (pci_bios_init_വേര്_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_end);
    എങ്കിൽ (pci_pad_mem64 || pci_bios_init_വേര്_regions_mem(busses)) {
        ഘടന pci_region r64_mem, r64_pref;
        r64_mem.list.first = ശൂന്യ;
        r64_pref.list.first = ശൂന്യ;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        എങ്കിൽ (pci_bios_init_വേര്_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 sum_mem = pci_region_sum(&r64_mem);
        u64 sum_pref = pci_region_sum(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.base = le64_to_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        എങ്കിൽ (r64_mem.base < 0x100000000LL + RamSizeOver4G)
            r64_mem.base = 0x100000000LL + RamSizeOver4G;
        എങ്കിൽ (pci_mem64_മുകൾ) {
            u64 size = (ALIGN(sum_mem, (1LL<<30)) +
                        ALIGN(sum_pref, (1LL<<30)));
            എങ്കിൽ (pci_pad_mem64)
                size = ALIGN(size, pci_mem64_മുകൾ >> 3);
            എങ്കിൽ (r64_mem.base < pci_mem64_മുകൾ - size) {
                r64_mem.base = pci_mem64_മുകൾ - size;
            }
            എങ്കിൽ (e820_is_used(r64_mem.base, size))
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
    } അല്ലെങ്കിൽ {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    പൂർണ്ണസംഖ്യ bus;
    വേണ്ടി (bus = 0; bus<=MaxPCIBus; bus++) {
        പൂർണ്ണസംഖ്യ type;
        വേണ്ടി (type = 0; type < PCI_REGION_TYPE_എണ്ണം; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

ശൂന്യം
pci_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU)
        തിരികെ;

    dprintf(3, "pci setup\n");

    എങ്കിൽ (CPUPhysBits) {
        pci_mem64_മുകൾ = 1LL << CPUPhysBits;
        എങ്കിൽ (CPUPhysBits > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            എങ്കിൽ (RamSizeOver4G < (1LL << 40)) {
                pci_mem64_മുകൾ = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    എങ്കിൽ (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = RamSizeOver4G ? 1 : 0;

    എങ്കിൽ (!CPULongMode)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    എങ്കിൽ (pci_probe_host() != 0) {
        തിരികെ;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = RamSize;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    ഘടന pci_bus *busses = malloc_tmp(വലുപ്പം(*busses) * (MaxPCIBus + 1));
    എങ്കിൽ (!busses) {
        warn_noalloc();
        തിരികെ;
    }
    memset(busses, 0, വലുപ്പം(*busses) * (MaxPCIBus + 1));
    എങ്കിൽ (pci_bios_check_devices(busses))
        തിരികെ;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    സ്വതന്ത്രം(busses);

    pci_enable_default_vga();
}
