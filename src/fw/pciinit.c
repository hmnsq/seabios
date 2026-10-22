// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "byteorder.h" // le64_to_cpu
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#অন্তর্ভাব "dev-piix.h" // PIIX_*
#অন্তর্ভাব "e820map.h" // e820_add
#অন্তর্ভাব "hw/ata.h" // PORT_ATA1_CMD_BASE
#অন্তর্ভাব "hw/pci.h" // pci_config_readl
#অন্তর্ভাব "hw/pcidevice.h" // pci_probe_devices
#অন্তর্ভাব "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#অন্তর্ভাব "hw/pci_regs.h" // PCI_COMMAND
#অন্তর্ভাব "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#অন্তর্ভাব "list.h" // struct hlist_node
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "paravirt.h" // RamSize
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // pci_setup
#অন্তর্ভাব "x86.h" // outb

#define PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#define PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#define PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#define PCI_ROM_SLOT 6
#define PCI_অঙ্ক_REGIONS 7
#define PCI_BRIDGE_অঙ্ক_REGIONS 2

গণনা pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_গুনতি,
};

স্থির ধ্রুবক অক্ষর *region_type_name[] = {
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
স্থির u64 pci_io_নিচের_end = 0xa000;
স্থির u64 pci_mem64_শীর্ষ  = 0;
স্থির u32 pci_pad_mem64  = -1;

কাঠামো pci_region_entry {
    কাঠামো pci_device *dev;
    পূর্ণসংখ্যা bar;
    u64 আকার_2;
    u64 align;
    পূর্ণসংখ্যা is64;
    গণনা pci_region_type type;
    কাঠামো hlist_গ্রন্থি গ্রন্থি;
};

কাঠামো pci_region {
    /* pci region assignments */
    u64 base;
    কাঠামো hlist_শীর্ষ list;
};

কাঠামো pci_bus {
    কাঠামো pci_region r[PCI_REGION_TYPE_গুনতি];
    কাঠামো pci_device *bus_dev;
};

স্থির u32 pci_bar(কাঠামো pci_device *pci, পূর্ণসংখ্যা region_অঙ্ক)
{
    যদি (region_অঙ্ক != PCI_ROM_SLOT) {
        ফেরত PCI_BASE_ADDRESS_0 + region_অঙ্ক * 4;
    }

#define PCI_HEADER_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
    ফেরত type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

স্থির শূন্য
pci_set_io_region_addr(কাঠামো pci_device *pci, পূর্ণসংখ্যা bar, u64 addr, পূর্ণসংখ্যা is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    যদি (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
ধ্রুবক u8 pci_irqs[4] = {
    10, 10, 11, 11
};

স্থির পূর্ণসংখ্যা dummy_pci_slot_get_irq(কাঠামো pci_device *pci, পূর্ণসংখ্যা pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    ফেরত 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

স্থির পূর্ণসংখ্যা (*pci_slot_get_irq)(কাঠামো pci_device *pci, পূর্ণসংখ্যা pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
স্থির পূর্ণসংখ্যা piix_pci_slot_get_irq(কাঠামো pci_device *pci, পূর্ণসংখ্যা pin)
{
    পূর্ণসংখ্যা slot_addend = 0;

    যখন (pci->parent != ফাঁকা) {
        slot_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_to_dev(pci->bdf) - 1;
    ফেরত pci_irqs[(pin - 1 + slot_addend) & 3];
}

স্থির পূর্ণসংখ্যা mch_pci_slot_get_irq(কাঠামো pci_device *pci, পূর্ণসংখ্যা pin)
{
    পূর্ণসংখ্যা pin_addend = 0;
    যখন (pci->parent != ফাঁকা) {
        pin_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_to_dev(pci->bdf);
    যদি (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        ফেরত pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    ফেরত pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
স্থির শূন্য piix_isa_bridge_setup(কাঠামো pci_device *pci, শূন্য *arg)
{
    পূর্ণসংখ্যা i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    জন্য (i = 0; i < 4; i++) {
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

স্থির শূন্য mch_isa_lpc_setup(u16 bdf)
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

স্থির পূর্ণসংখ্যা ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
স্থির শূন্য mch_isa_bridge_setup(কাঠামো pci_device *dev, শূন্য *arg)
{
    u16 bdf = dev->bdf;
    পূর্ণসংখ্যা i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    জন্য (i = 0; i < 4; i++) {
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

স্থির শূন্য storage_ide_setup(কাঠামো pci_device *pci, শূন্য *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_set_io_region_addr(pci, 0, PORT_ATA1_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 1, PORT_ATA1_CTRL_BASE, 0);
    pci_set_io_region_addr(pci, 2, PORT_ATA2_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 3, PORT_ATA2_CTRL_BASE, 0);
}

/* PIIX3/PIIX4 IDE */
স্থির শূন্য piix_ide_setup(কাঠামো pci_device *pci, শূন্য *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

স্থির শূন্য pic_ibm_setup(কাঠামো pci_device *pci, শূন্য *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_set_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

স্থির শূন্য apple_macio_setup(কাঠামো pci_device *pci, শূন্য *arg)
{
    /* macio bridge */
    pci_set_io_region_addr(pci, 0, 0x80800000, 0);
}

স্থির শূন্য piix4_pm_config_setup(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_INTERRUPT_সারি, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_base | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_base + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

স্থির পূর্ণসংখ্যা PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
স্থির শূন্য piix4_pm_setup(কাঠামো pci_device *pci, শূন্য *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_setup(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

স্থির শূন্য ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_BASE,
                      (acpi_pm_base + 0x100) | PCI_BASE_ADDRESS_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

স্থির পূর্ণসংখ্যা ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
স্থির শূন্য ich9_smbus_setup(কাঠামো pci_device *dev, শূন্য *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

স্থির শূন্য intel_igd_setup(কাঠামো pci_device *dev, শূন্য *arg)
{
    কাঠামো romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_আকার = le64_to_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    যদি (opregion && opregion->আকার_2) {
        শূন্য *addr = memalign_উপরের(PAGE_আকার, opregion->আকার_2);
        যদি (!addr) {
            warn_noalloc();
            ফেরত;
        }

        যদি (opregion->copy(opregion, addr, opregion->আকার_2) < 0) {
            মুক্ত(addr);
            ফেরত;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->আকার_2 >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    যদি (bdsm_আকার && (dev->bdf == pci_to_bdf(0, 2, 0))) {
        শূন্য *addr = memalign_tmphigh(1024 * 1024, bdsm_আকার);
        যদি (!addr) {
            warn_noalloc();
            ফেরত;
        }

        e820_add((u32)addr, bdsm_আকার, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_আকার >> 20, dev);
    }
}

স্থির ধ্রুবক কাঠামো pci_device_id pci_device_tbl[] = {
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

স্থির পূর্ণসংখ্যা MCHMmcfgBDF = -1;
স্থির শূন্য mch_mmconfig_setup(u16 bdf);

শূন্য pci_resume(শূন্য)
{
    যদি (!CONFIG_QEMU) {
        ফেরত;
    }

    যদি (PiixPmBDF >= 0) {
        piix4_pm_config_setup(PiixPmBDF);
    }

    যদি (ICH9LpcBDF >= 0) {
        mch_isa_lpc_setup(ICH9LpcBDF);
    }

    যদি (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    যদি(MCHMmcfgBDF >= 0) {
        mch_mmconfig_setup(MCHMmcfgBDF);
    }
}

স্থির শূন্য pci_bios_init_device(কাঠামো pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    পূর্ণসংখ্যা pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
    যদি (pin != 0)
        pci_config_writeb(bdf, PCI_INTERRUPT_সারি, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, ফাঁকা);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    যদি (pci->header_type & PCI_HEADER_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

স্থির শূন্য pci_bios_init_devices(শূন্য)
{
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

স্থির শূন্য pci_enable_default_vga(শূন্য)
{
    কাঠামো pci_device *pci;

    foreachpci(pci) {
        যদি (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            ফেরত;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    যদি (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        ফেরত;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

    যখন (pci->parent) {
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

স্থির শূন্য i440fx_mem_addr_setup(কাঠামো pci_device *dev, শূন্য *arg)
{
    যদি (Ram_আকার <= 0x80000000)
        pcimem_start = 0x80000000;
    অন্যথা যদি (Ram_আকার <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

স্থির শূন্য mch_mmconfig_setup(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 upper = addr >> 32;
    u32 lower = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, upper);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, lower);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

স্থির শূন্য mch_mem_addr_setup(কাঠামো pci_device *dev, শূন্য *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 আকার_2 = Q35_HOST_BRIDGE_PCIEXBAR_আকার;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_setup(dev->bdf);
    e820_add(addr, আকার_2, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_start = addr + আকার_2;

    pci_slot_get_irq = mch_pci_slot_get_irq;

    /* setup io address space */
    যদি (acpi_pm_base < 0x1000)
        pci_io_নিচের_end = 0x10000;
    অন্যথা
        pci_io_নিচের_end = acpi_pm_base;
}

স্থির ধ্রুবক কাঠামো pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_setup),
    PCI_DEVICE_END
};

স্থির শূন্য pci_bios_init_platform(শূন্য)
{
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, ফাঁকা);
    }
}

স্থির u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    যদি (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        ফেরত 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    যদি (device_id != PCI_DEVICE_ID_REDHAT_মূল_PORT &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        ফেরত 0;
    }
    u8 cap = 0;

    করো {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } যখন (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_OFFSET) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    যদি (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        যদি (cap_len < RES_RESERVE_CAP_আকার) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            ফেরত 0;
        }
    } অন্যথা {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    ফেরত cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

স্থির শূন্য
pci_bios_init_bus_rec(পূর্ণসংখ্যা bus, u8 *pci_bus)
{
    পূর্ণসংখ্যা bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        যদি (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        যদি (class != PCI_CLASS_BRIDGE_PCI) {
            চালু;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        যদি (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } অন্যথা {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        যদি (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } অন্যথা {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        যদি (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            যদি (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                যদি (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    যদি ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    যদি (secbus + res_bus > *pci_bus) {
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
        } অন্যথা {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

স্থির শূন্য
pci_bios_init_bus(শূন্য)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    যদি (extraroots) {
        যখন (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

স্থির শূন্য
pci_bios_get_bar(কাঠামো pci_device *pci, পূর্ণসংখ্যা bar,
                 পূর্ণসংখ্যা *ptype, u64 *psize, পূর্ণসংখ্যা *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    পূর্ণসংখ্যা is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 mask;

    যদি (bar == PCI_ROM_SLOT) {
        mask = PCI_ROM_ADDRESS_MASK;
        pci_config_writel(bdf, ofs, mask);
    } অন্যথা {
        যদি (old & PCI_BASE_ADDRESS_SPACE_IO) {
            mask = PCI_BASE_ADDRESS_IO_MASK;
            type = PCI_REGION_TYPE_IO;
        } অন্যথা {
            mask = PCI_BASE_ADDRESS_MEM_MASK;
            যদি (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK)
                    == PCI_BASE_ADDRESS_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    যদি (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 উপরের = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)উপরের << 32);
        mask |= ((u64)0xffffffff << 32);
        *psize = (~(val & mask)) + 1;
    } অন্যথা {
        *psize = ((~(val & mask)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

স্থির পূর্ণসংখ্যা pci_bios_bridge_region_is64(কাঠামো pci_region *r,
                                 কাঠামো pci_device *pci, পূর্ণসংখ্যা type)
{
    যদি (type != PCI_REGION_TYPE_PREFMEM)
        ফেরত 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
    যদি (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0x0);
    }
    যদি ((pmem & PCI_PREF_RANGE_TYPE_MASK) != PCI_PREF_RANGE_TYPE_64)
       ফেরত 0;
    কাঠামো pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, গ্রন্থি) {
        যদি (!entry->is64)
            ফেরত 0;
    }
    ফেরত 1;
}

স্থির u64 pci_region_align(কাঠামো pci_region *r)
{
    কাঠামো pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, গ্রন্থি) {
        // The first entry in the sorted list has the largest alignment
        ফেরত entry->align;
    }
    ফেরত 1;
}

স্থির u64 pci_region_যোগ(কাঠামো pci_region *r)
{
    u64 যোগ = 0;
    কাঠামো pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, গ্রন্থি) {
        যোগ += entry->আকার_2;
    }
    ফেরত যোগ;
}

স্থির শূন্য pci_region_migrate_64bit_entries(কাঠামো pci_region *from,
                                             কাঠামো pci_region *to)
{
    কাঠামো hlist_গ্রন্থি *n, **last = &to->list.first;
    কাঠামো pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &from->list, গ্রন্থি) {
        যদি (!entry->is64)
            চালু;
        যদি (entry->dev->class == PCI_CLASS_SERIAL_USB)
            চালু;
        যদি (entry->dev->class == PCI_CLASS_STORAGE_NVME)
            চালু;
        // Move from source list to destination list.
        hlist_del(&entry->গ্রন্থি);
        hlist_add(&entry->গ্রন্থি, last);
        last = &entry->গ্রন্থি.পরবর্তী;
    }
}

স্থির কাঠামো pci_region_entry *
pci_region_create_entry(কাঠামো pci_bus *bus, কাঠামো pci_device *dev,
                        পূর্ণসংখ্যা bar, u64 আকার_2, u64 align, পূর্ণসংখ্যা type, পূর্ণসংখ্যা is64)
{
    কাঠামো pci_region_entry *entry = malloc_tmp(আকার(*entry));
    যদি (!entry) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(entry, 0, আকার(*entry));
    entry->dev = dev;
    entry->bar = bar;
    entry->আকার_2 = আকার_2;
    entry->align = align;
    entry->is64 = is64;
    entry->type = type;
    // Insert into list in sorted order.
    কাঠামো hlist_গ্রন্থি **pprev;
    কাঠামো pci_region_entry *pos;
    hlist_for_each_entry_pprev(pos, pprev, &bus->r[type].list, গ্রন্থি) {
        যদি (pos->align < align || (pos->align == align && pos->আকার_2 < আকার_2))
            বিরতি;
    }
    hlist_add(&entry->গ্রন্থি, pprev);
    ফেরত entry;
}

প্রকারলেখ গণনা hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

স্থির hotplug_type_t pci_bus_hotplug_support(কাঠামো pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    যদি (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 port_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        যদি (port_type == PCI_EXP_TYPE_PCI_BRIDGE)
            যাও check_shpc;

        u8 downstream_port = (port_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (port_type == PCI_EXP_TYPE_মূল_PORT);
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

        ফেরত downstream_port && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    ফেরত !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
স্থির পূর্ণসংখ্যা pci_bridge_has_region(কাঠামো pci_device *pci,
                                 গণনা pci_region_type region_type)
{
    u8 base;

    নির্বাচন (region_type) {
        ক্ষেত্র PCI_REGION_TYPE_IO:
            base = PCI_IO_BASE;
            বিরতি;
        ক্ষেত্র PCI_REGION_TYPE_PREFMEM:
            base = PCI_PREF_MEMORY_BASE;
            বিরতি;
        পূর্বনির্ধারিত:
            /* Regular memory support is mandatory */
            ফেরত 1;
    }

    pci_config_writeb(pci->bdf, base, 0xFF);

    ফেরত pci_config_readb(pci->bdf, base) != 0;
}

স্থির পূর্ণসংখ্যা pci_bios_check_devices(কাঠামো pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        কাঠামো pci_bus *bus = &busses[pci_bdf_to_bus(pci->bdf)];
        যদি (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        পূর্ণসংখ্যা i;
        জন্য (i = 0; i < PCI_অঙ্ক_REGIONS; i++) {
            যদি ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_অঙ্ক_REGIONS && i < PCI_ROM_SLOT))
                চালু;
            পূর্ণসংখ্যা type, is64;
            u64 আকার_2;
            pci_bios_get_bar(pci, i, &type, &আকার_2, &is64);
            যদি (আকার_2 == 0)
                চালু;

            যদি (type != PCI_REGION_TYPE_IO && আকার_2 < PCI_DEVICE_MEM_MIN)
                আকার_2 = PCI_DEVICE_MEM_MIN;
            কাঠামো pci_region_entry *entry = pci_region_create_entry(
                bus, pci, i, আকার_2, আকার_2, type, is64);
            যদি (!entry)
                ফেরত -1;

            যদি (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    পূর্ণসংখ্যা secondary_bus;
    জন্য (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        কাঠামো pci_bus *s = &busses[secondary_bus];
        যদি (!s->bus_dev)
            চালু;
        কাঠামো pci_bus *parent = &busses[pci_bdf_to_bus(s->bus_dev->bdf)];
        যদি (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        পূর্ণসংখ্যা type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        জন্য (type = 0; type < PCI_REGION_TYPE_গুনতি; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            যদি (!pci_bridge_has_region(s->bus_dev, type))
                চালু;
            u64 আকার_2 = 0;
            যদি (qemu_cap) {
                u32 tmp_আকার;
                u64 tmp_আকার_64;
                নির্বাচন(type) {
                ক্ষেত্র PCI_REGION_TYPE_IO:
                    tmp_আকার_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    যদি (tmp_আকার_64 != (u64)-1) {
                        আকার_2 = tmp_আকার_64;
                    }
                    বিরতি;
                ক্ষেত্র PCI_REGION_TYPE_MEM:
                    tmp_আকার = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    যদি (tmp_আকার != (u32)-1) {
                        আকার_2 = tmp_আকার;
                    }
                    বিরতি;
                ক্ষেত্র PCI_REGION_TYPE_PREFMEM:
                    tmp_আকার = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_আকার_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    যদি (tmp_আকার != (u32)-1 && tmp_আকার_64 == (u64)-1) {
                        আকার_2 = tmp_আকার;
                    } অন্যথা যদি (tmp_আকার == (u32)-1 && tmp_আকার_64 != (u64)-1) {
                        আকার_2 = tmp_আকার_64;
                    } অন্যথা যদি (tmp_আকার != (u32)-1 && tmp_আকার_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    বিরতি;
                পূর্বনির্ধারিত:
                    বিরতি;
                }
            }
            যদি (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 যোগ = pci_region_যোগ(&s->r[type]);
            পূর্ণসংখ্যা is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            পূর্ণসংখ্যা resource_optional = 0;
            যদি (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 শীর্ষ_align = pci_mem64_শীর্ষ >> 11;
            যদি (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (শীর্ষ_align > align))
                align = শীর্ষ_align;
            যদি (align > যোগ && hotplug_support && !resource_optional)
                যোগ = align; /* reserve min size for hot-plug */
            যদি (আকার_2 > যোগ) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        আকার_2, region_type_name[type]);
                যদি (type != PCI_REGION_TYPE_IO) {
                    আকার_2 = ALIGN(আকার_2, align);
                }
            } অন্যথা {
                আকার_2 = ALIGN(যোগ, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            কাঠামো pci_region_entry *entry = pci_region_create_entry(
                parent, s->bus_dev, -1, আকার_2, align, type, is64);
            যদি (!entry)
                ফেরত -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      entry->dev->secondary_bus, আকার_2,
                      region_type_name[entry->type]);
        }
    }
    ফেরত 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
স্থির পূর্ণসংখ্যা pci_bios_init_মূল_regions_io(কাঠামো pci_bus *bus)
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
    কাঠামো pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 যোগ = pci_region_যোগ(r_io);
    যদি (যোগ < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->base = 0xc000;
    } অন্যথা যদি (যোগ < pci_io_নিচের_end - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->base = 0x1000;
    } অন্যথা {
        /* not enouth io address space -> error out */
        ফেরত -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->base, r_io->base + যোগ - 1);
    ফেরত 0;
}

স্থির পূর্ণসংখ্যা pci_bios_init_মূল_regions_mem(কাঠামো pci_bus *bus)
{
    কাঠামো pci_region *r_end = &bus->r[PCI_REGION_TYPE_PREFMEM];
    কাঠামো pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    যদি (pci_region_align(r_start) < pci_region_align(r_end)) {
        // Swap regions to improve alignment.
        r_end = r_start;
        r_start = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 যোগ = pci_region_যোগ(r_end);
    u64 align = pci_region_align(r_end);
    r_end->base = ALIGN_DOWN((pcimem_end - যোগ), align);
    যোগ = pci_region_যোগ(r_start);
    align = pci_region_align(r_start);
    r_start->base = ALIGN_DOWN((r_end->base - যোগ), align);

    যদি ((r_start->base < pcimem_start) ||
         (r_start->base > pcimem_end))
        // Memory range requested is larger than available.
        ফেরত -1;
    ফেরত 0;
}

#define PCI_IO_SHIFT            8
#define PCI_MEMORY_SHIFT        16
#define PCI_PREF_MEMORY_SHIFT   16

স্থির শূন্য
pci_region_map_one_entry(কাঠামো pci_region_entry *entry, u64 addr)
{
    যদি (entry->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                entry->dev,
                entry->bar, addr, entry->আকার_2, region_type_name[entry->type]);

        pci_set_io_region_addr(entry->dev, entry->bar, addr, entry->is64);
        ফেরত;
    }

    u16 bdf = entry->dev->bdf;
    u64 limit = addr + entry->আকার_2 - 1;
    যদি (entry->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_BASE, addr >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_BASE_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_LIMIT, limit >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_LIMIT_UPPER16, 0);
    }
    যদি (entry->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_MEMORY_BASE, addr >> PCI_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_MEMORY_LIMIT, limit >> PCI_MEMORY_SHIFT);
    }
    যদি (entry->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_MEMORY_BASE, addr >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_PREF_MEMORY_LIMIT, limit >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writel(bdf, PCI_PREF_BASE_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_LIMIT_UPPER32, limit >> 32);
    }
}

স্থির শূন্য pci_region_map_entries(কাঠামো pci_bus *busses, কাঠামো pci_region *r)
{
    কাঠামো hlist_গ্রন্থি *n;
    কাঠামো pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &r->list, গ্রন্থি) {
        u64 addr = r->base;
        r->base += entry->আকার_2;
        যদি (entry->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[entry->dev->secondary_bus].r[entry->type].base = addr;
        pci_region_map_one_entry(entry, addr);
        hlist_del(&entry->গ্রন্থি);
        মুক্ত(entry);
    }
}

স্থির শূন্য pci_bios_map_devices(কাঠামো pci_bus *busses)
{
    যদি (pci_bios_init_মূল_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_end);
    যদি (pci_pad_mem64 || pci_bios_init_মূল_regions_mem(busses)) {
        কাঠামো pci_region r64_mem, r64_pref;
        r64_mem.list.first = ফাঁকা;
        r64_pref.list.first = ফাঁকা;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        যদি (pci_bios_init_মূল_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 যোগ_mem = pci_region_যোগ(&r64_mem);
        u64 যোগ_pref = pci_region_যোগ(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.base = le64_to_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        যদি (r64_mem.base < 0x100000000LL + Ram_আকার_Over4_G)
            r64_mem.base = 0x100000000LL + Ram_আকার_Over4_G;
        যদি (pci_mem64_শীর্ষ) {
            u64 আকার_2 = (ALIGN(যোগ_mem, (1LL<<30)) +
                        ALIGN(যোগ_pref, (1LL<<30)));
            যদি (pci_pad_mem64)
                আকার_2 = ALIGN(আকার_2, pci_mem64_শীর্ষ >> 3);
            যদি (r64_mem.base < pci_mem64_শীর্ষ - আকার_2) {
                r64_mem.base = pci_mem64_শীর্ষ - আকার_2;
            }
            যদি (e820_is_used(r64_mem.base, আকার_2))
                r64_mem.base -= আকার_2;
        }
        r64_mem.base = ALIGN(r64_mem.base, align_mem);
        r64_mem.base = ALIGN(r64_mem.base, (1LL<<30));    // 1G hugepage
        r64_pref.base = r64_mem.base + যোগ_mem;
        r64_pref.base = ALIGN(r64_pref.base, align_pref);
        r64_pref.base = ALIGN(r64_pref.base, (1LL<<30));  // 1G hugepage
        pcimem64_start = r64_mem.base;
        pcimem64_end = r64_pref.base + যোগ_pref;
        pcimem64_end = ALIGN(pcimem64_end, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_start, pcimem64_end);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } অন্যথা {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    পূর্ণসংখ্যা bus;
    জন্য (bus = 0; bus<=MaxPCIBus; bus++) {
        পূর্ণসংখ্যা type;
        জন্য (type = 0; type < PCI_REGION_TYPE_গুনতি; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

শূন্য
pci_setup(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    dprintf(3, "pci setup\n");

    যদি (CPUPhysBits) {
        pci_mem64_শীর্ষ = 1LL << CPUPhysBits;
        যদি (CPUPhysBits > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            যদি (Ram_আকার_Over4_G < (1LL << 40)) {
                pci_mem64_শীর্ষ = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    যদি (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_আকার_Over4_G ? 1 : 0;

    যদি (!CPULongMode)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    যদি (pci_probe_host() != 0) {
        ফেরত;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = Ram_আকার;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    কাঠামো pci_bus *busses = malloc_tmp(আকার(*busses) * (MaxPCIBus + 1));
    যদি (!busses) {
        warn_noalloc();
        ফেরত;
    }
    memset(busses, 0, আকার(*busses) * (MaxPCIBus + 1));
    যদি (pci_bios_check_devices(busses))
        ফেরত;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    মুক্ত(busses);

    pci_enable_default_vga();
}
