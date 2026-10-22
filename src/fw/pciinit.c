// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "byteorder.h" // le64_to_cpu
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#அந்தர்பாவ "dev-piix.h" // PIIX_*
#அந்தர்பாவ "e820map.h" // e820_add
#அந்தர்பாவ "hw/ata.h" // PORT_ATA1_CMD_BASE
#அந்தர்பாவ "hw/pci.h" // pci_config_readl
#அந்தர்பாவ "hw/pcidevice.h" // pci_probe_devices
#அந்தர்பாவ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#அந்தர்பாவ "hw/pci_regs.h" // PCI_COMMAND
#அந்தர்பாவ "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#அந்தர்பாவ "list.h" // struct hlist_node
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "paravirt.h" // RamSize
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // pci_setup
#அந்தர்பாவ "x86.h" // outb

#define PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#define PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#define PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#define PCI_ROM_SLOT 6
#define PCI_இலக்கம்_REGIONS 7
#define PCI_BRIDGE_இலக்கம்_REGIONS 2

எண்ணிக்கை pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_மொத்தம்,
};

நிலையான மாறிலி எழுத்து *region_type_name[] = {
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
நிலையான u64 pci_io_கீழ்_end = 0xa000;
நிலையான u64 pci_mem64_உச்சி  = 0;
நிலையான u32 pci_pad_mem64  = -1;

கட்டமைப்பு pci_region_entry {
    கட்டமைப்பு pci_device *dev;
    முழு bar;
    u64 size;
    u64 align;
    முழு is64;
    எண்ணிக்கை pci_region_type type;
    கட்டமைப்பு hlist_முனை முனை;
};

கட்டமைப்பு pci_region {
    /* pci region assignments */
    u64 base;
    கட்டமைப்பு hlist_head list;
};

கட்டமைப்பு pci_bus {
    கட்டமைப்பு pci_region r[PCI_REGION_TYPE_மொத்தம்];
    கட்டமைப்பு pci_device *bus_dev;
};

நிலையான u32 pci_bar(கட்டமைப்பு pci_device *pci, முழு region_இலக்கம்)
{
    எனில் (region_இலக்கம் != PCI_ROM_SLOT) {
        திரும்பு PCI_BASE_ADDRESS_0 + region_இலக்கம் * 4;
    }

#define PCI_HEADER_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
    திரும்பு type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

நிலையான வெறுமை
pci_set_io_region_addr(கட்டமைப்பு pci_device *pci, முழு bar, u64 addr, முழு is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    எனில் (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
மாறிலி u8 pci_irqs[4] = {
    10, 10, 11, 11
};

நிலையான முழு dummy_pci_slot_get_irq(கட்டமைப்பு pci_device *pci, முழு pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    திரும்பு 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

நிலையான முழு (*pci_slot_get_irq)(கட்டமைப்பு pci_device *pci, முழு pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
நிலையான முழு piix_pci_slot_get_irq(கட்டமைப்பு pci_device *pci, முழு pin)
{
    முழு slot_addend = 0;

    போது (pci->parent != வெற்று) {
        slot_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_to_dev(pci->bdf) - 1;
    திரும்பு pci_irqs[(pin - 1 + slot_addend) & 3];
}

நிலையான முழு mch_pci_slot_get_irq(கட்டமைப்பு pci_device *pci, முழு pin)
{
    முழு pin_addend = 0;
    போது (pci->parent != வெற்று) {
        pin_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_to_dev(pci->bdf);
    எனில் (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        திரும்பு pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    திரும்பு pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
நிலையான வெறுமை piix_isa_bridge_setup(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    முழு i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    க்கு (i = 0; i < 4; i++) {
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

நிலையான வெறுமை mch_isa_lpc_setup(u16 bdf)
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

நிலையான முழு ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
நிலையான வெறுமை mch_isa_bridge_setup(கட்டமைப்பு pci_device *dev, வெறுமை *arg)
{
    u16 bdf = dev->bdf;
    முழு i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    க்கு (i = 0; i < 4; i++) {
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

நிலையான வெறுமை storage_ide_setup(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_set_io_region_addr(pci, 0, PORT_ATA1_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 1, PORT_ATA1_CTRL_BASE, 0);
    pci_set_io_region_addr(pci, 2, PORT_ATA2_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 3, PORT_ATA2_CTRL_BASE, 0);
}

/* PIIX3/PIIX4 IDE */
நிலையான வெறுமை piix_ide_setup(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

நிலையான வெறுமை pic_ibm_setup(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_set_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

நிலையான வெறுமை apple_macio_setup(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    /* macio bridge */
    pci_set_io_region_addr(pci, 0, 0x80800000, 0);
}

நிலையான வெறுமை piix4_pm_config_setup(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_INTERRUPT_வரி, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_base | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_base + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

நிலையான முழு PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
நிலையான வெறுமை piix4_pm_setup(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_setup(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

நிலையான வெறுமை ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_BASE,
                      (acpi_pm_base + 0x100) | PCI_BASE_ADDRESS_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

நிலையான முழு ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
நிலையான வெறுமை ich9_smbus_setup(கட்டமைப்பு pci_device *dev, வெறுமை *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

நிலையான வெறுமை intel_igd_setup(கட்டமைப்பு pci_device *dev, வெறுமை *arg)
{
    கட்டமைப்பு romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_size = le64_to_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    எனில் (opregion && opregion->size) {
        வெறுமை *addr = memalign_உயர்வு(PAGE_SIZE, opregion->size);
        எனில் (!addr) {
            warn_noalloc();
            திரும்பு;
        }

        எனில் (opregion->copy(opregion, addr, opregion->size) < 0) {
            விடுவி(addr);
            திரும்பு;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->size >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    எனில் (bdsm_size && (dev->bdf == pci_to_bdf(0, 2, 0))) {
        வெறுமை *addr = memalign_tmphigh(1024 * 1024, bdsm_size);
        எனில் (!addr) {
            warn_noalloc();
            திரும்பு;
        }

        e820_add((u32)addr, bdsm_size, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_size >> 20, dev);
    }
}

நிலையான மாறிலி கட்டமைப்பு pci_device_id pci_device_tbl[] = {
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

நிலையான முழு MCHMmcfgBDF = -1;
நிலையான வெறுமை mch_mmconfig_setup(u16 bdf);

வெறுமை pci_resume(வெறுமை)
{
    எனில் (!CONFIG_QEMU) {
        திரும்பு;
    }

    எனில் (PiixPmBDF >= 0) {
        piix4_pm_config_setup(PiixPmBDF);
    }

    எனில் (ICH9LpcBDF >= 0) {
        mch_isa_lpc_setup(ICH9LpcBDF);
    }

    எனில் (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    எனில்(MCHMmcfgBDF >= 0) {
        mch_mmconfig_setup(MCHMmcfgBDF);
    }
}

நிலையான வெறுமை pci_bios_init_device(கட்டமைப்பு pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    முழு pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
    எனில் (pin != 0)
        pci_config_writeb(bdf, PCI_INTERRUPT_வரி, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, வெற்று);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    எனில் (pci->header_type & PCI_HEADER_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

நிலையான வெறுமை pci_bios_init_devices(வெறுமை)
{
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

நிலையான வெறுமை pci_enable_default_vga(வெறுமை)
{
    கட்டமைப்பு pci_device *pci;

    foreachpci(pci) {
        எனில் (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            திரும்பு;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    எனில் (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        திரும்பு;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

    போது (pci->parent) {
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

நிலையான வெறுமை i440fx_mem_addr_setup(கட்டமைப்பு pci_device *dev, வெறுமை *arg)
{
    எனில் (RamSize <= 0x80000000)
        pcimem_start = 0x80000000;
    இல்லையெனில் எனில் (RamSize <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

நிலையான வெறுமை mch_mmconfig_setup(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 upper = addr >> 32;
    u32 lower = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, upper);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, lower);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

நிலையான வெறுமை mch_mem_addr_setup(கட்டமைப்பு pci_device *dev, வெறுமை *arg)
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
    எனில் (acpi_pm_base < 0x1000)
        pci_io_கீழ்_end = 0x10000;
    இல்லையெனில்
        pci_io_கீழ்_end = acpi_pm_base;
}

நிலையான மாறிலி கட்டமைப்பு pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_setup),
    PCI_DEVICE_END
};

நிலையான வெறுமை pci_bios_init_platform(வெறுமை)
{
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, வெற்று);
    }
}

நிலையான u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    எனில் (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        திரும்பு 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    எனில் (device_id != PCI_DEVICE_ID_REDHAT_வேர்_PORT &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        திரும்பு 0;
    }
    u8 cap = 0;

    செய் {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } போது (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_OFFSET) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    எனில் (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        எனில் (cap_len < RES_RESERVE_CAP_SIZE) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            திரும்பு 0;
        }
    } இல்லையெனில் {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    திரும்பு cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

நிலையான வெறுமை
pci_bios_init_bus_rec(முழு bus, u8 *pci_bus)
{
    முழு bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        எனில் (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        எனில் (class != PCI_CLASS_BRIDGE_PCI) {
            தொடர்;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        எனில் (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } இல்லையெனில் {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        எனில் (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } இல்லையெனில் {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        எனில் (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            எனில் (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                எனில் (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    எனில் ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    எனில் (secbus + res_bus > *pci_bus) {
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
        } இல்லையெனில் {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

நிலையான வெறுமை
pci_bios_init_bus(வெறுமை)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    எனில் (extraroots) {
        போது (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

நிலையான வெறுமை
pci_bios_get_bar(கட்டமைப்பு pci_device *pci, முழு bar,
                 முழு *ptype, u64 *psize, முழு *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    முழு is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 mask;

    எனில் (bar == PCI_ROM_SLOT) {
        mask = PCI_ROM_ADDRESS_MASK;
        pci_config_writel(bdf, ofs, mask);
    } இல்லையெனில் {
        எனில் (old & PCI_BASE_ADDRESS_SPACE_IO) {
            mask = PCI_BASE_ADDRESS_IO_MASK;
            type = PCI_REGION_TYPE_IO;
        } இல்லையெனில் {
            mask = PCI_BASE_ADDRESS_MEM_MASK;
            எனில் (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK)
                    == PCI_BASE_ADDRESS_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    எனில் (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 உயர்வு = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)உயர்வு << 32);
        mask |= ((u64)0xffffffff << 32);
        *psize = (~(val & mask)) + 1;
    } இல்லையெனில் {
        *psize = ((~(val & mask)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

நிலையான முழு pci_bios_bridge_region_is64(கட்டமைப்பு pci_region *r,
                                 கட்டமைப்பு pci_device *pci, முழு type)
{
    எனில் (type != PCI_REGION_TYPE_PREFMEM)
        திரும்பு 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
    எனில் (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0x0);
    }
    எனில் ((pmem & PCI_PREF_RANGE_TYPE_MASK) != PCI_PREF_RANGE_TYPE_64)
       திரும்பு 0;
    கட்டமைப்பு pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, முனை) {
        எனில் (!entry->is64)
            திரும்பு 0;
    }
    திரும்பு 1;
}

நிலையான u64 pci_region_align(கட்டமைப்பு pci_region *r)
{
    கட்டமைப்பு pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, முனை) {
        // The first entry in the sorted list has the largest alignment
        திரும்பு entry->align;
    }
    திரும்பு 1;
}

நிலையான u64 pci_region_sum(கட்டமைப்பு pci_region *r)
{
    u64 sum = 0;
    கட்டமைப்பு pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, முனை) {
        sum += entry->size;
    }
    திரும்பு sum;
}

நிலையான வெறுமை pci_region_migrate_64bit_entries(கட்டமைப்பு pci_region *from,
                                             கட்டமைப்பு pci_region *to)
{
    கட்டமைப்பு hlist_முனை *n, **last = &to->list.first;
    கட்டமைப்பு pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &from->list, முனை) {
        எனில் (!entry->is64)
            தொடர்;
        எனில் (entry->dev->class == PCI_CLASS_SERIAL_USB)
            தொடர்;
        எனில் (entry->dev->class == PCI_CLASS_STORAGE_NVME)
            தொடர்;
        // Move from source list to destination list.
        hlist_del(&entry->முனை);
        hlist_add(&entry->முனை, last);
        last = &entry->முனை.next;
    }
}

நிலையான கட்டமைப்பு pci_region_entry *
pci_region_create_entry(கட்டமைப்பு pci_bus *bus, கட்டமைப்பு pci_device *dev,
                        முழு bar, u64 size, u64 align, முழு type, முழு is64)
{
    கட்டமைப்பு pci_region_entry *entry = malloc_tmp(அளவு(*entry));
    எனில் (!entry) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(entry, 0, அளவு(*entry));
    entry->dev = dev;
    entry->bar = bar;
    entry->size = size;
    entry->align = align;
    entry->is64 = is64;
    entry->type = type;
    // Insert into list in sorted order.
    கட்டமைப்பு hlist_முனை **pprev;
    கட்டமைப்பு pci_region_entry *pos;
    hlist_for_each_entry_pprev(pos, pprev, &bus->r[type].list, முனை) {
        எனில் (pos->align < align || (pos->align == align && pos->size < size))
            நிறுத்து;
    }
    hlist_add(&entry->முனை, pprev);
    திரும்பு entry;
}

வகைவரை எண்ணிக்கை hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

நிலையான hotplug_type_t pci_bus_hotplug_support(கட்டமைப்பு pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    எனில் (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 port_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        எனில் (port_type == PCI_EXP_TYPE_PCI_BRIDGE)
            செல் check_shpc;

        u8 downstream_port = (port_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (port_type == PCI_EXP_TYPE_வேர்_PORT);
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

        திரும்பு downstream_port && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    திரும்பு !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
நிலையான முழு pci_bridge_has_region(கட்டமைப்பு pci_device *pci,
                                 எண்ணிக்கை pci_region_type region_type)
{
    u8 base;

    தேர்வு (region_type) {
        நிலை PCI_REGION_TYPE_IO:
            base = PCI_IO_BASE;
            நிறுத்து;
        நிலை PCI_REGION_TYPE_PREFMEM:
            base = PCI_PREF_MEMORY_BASE;
            நிறுத்து;
        இயல்பு:
            /* Regular memory support is mandatory */
            திரும்பு 1;
    }

    pci_config_writeb(pci->bdf, base, 0xFF);

    திரும்பு pci_config_readb(pci->bdf, base) != 0;
}

நிலையான முழு pci_bios_check_devices(கட்டமைப்பு pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        கட்டமைப்பு pci_bus *bus = &busses[pci_bdf_to_bus(pci->bdf)];
        எனில் (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        முழு i;
        க்கு (i = 0; i < PCI_இலக்கம்_REGIONS; i++) {
            எனில் ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_இலக்கம்_REGIONS && i < PCI_ROM_SLOT))
                தொடர்;
            முழு type, is64;
            u64 size;
            pci_bios_get_bar(pci, i, &type, &size, &is64);
            எனில் (size == 0)
                தொடர்;

            எனில் (type != PCI_REGION_TYPE_IO && size < PCI_DEVICE_MEM_MIN)
                size = PCI_DEVICE_MEM_MIN;
            கட்டமைப்பு pci_region_entry *entry = pci_region_create_entry(
                bus, pci, i, size, size, type, is64);
            எனில் (!entry)
                திரும்பு -1;

            எனில் (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    முழு secondary_bus;
    க்கு (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        கட்டமைப்பு pci_bus *s = &busses[secondary_bus];
        எனில் (!s->bus_dev)
            தொடர்;
        கட்டமைப்பு pci_bus *parent = &busses[pci_bdf_to_bus(s->bus_dev->bdf)];
        எனில் (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        முழு type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        க்கு (type = 0; type < PCI_REGION_TYPE_மொத்தம்; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            எனில் (!pci_bridge_has_region(s->bus_dev, type))
                தொடர்;
            u64 size = 0;
            எனில் (qemu_cap) {
                u32 tmp_size;
                u64 tmp_size_64;
                தேர்வு(type) {
                நிலை PCI_REGION_TYPE_IO:
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    எனில் (tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    }
                    நிறுத்து;
                நிலை PCI_REGION_TYPE_MEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    எனில் (tmp_size != (u32)-1) {
                        size = tmp_size;
                    }
                    நிறுத்து;
                நிலை PCI_REGION_TYPE_PREFMEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    எனில் (tmp_size != (u32)-1 && tmp_size_64 == (u64)-1) {
                        size = tmp_size;
                    } இல்லையெனில் எனில் (tmp_size == (u32)-1 && tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    } இல்லையெனில் எனில் (tmp_size != (u32)-1 && tmp_size_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    நிறுத்து;
                இயல்பு:
                    நிறுத்து;
                }
            }
            எனில் (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 sum = pci_region_sum(&s->r[type]);
            முழு is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            முழு resource_optional = 0;
            எனில் (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 உச்சி_align = pci_mem64_உச்சி >> 11;
            எனில் (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (உச்சி_align > align))
                align = உச்சி_align;
            எனில் (align > sum && hotplug_support && !resource_optional)
                sum = align; /* reserve min size for hot-plug */
            எனில் (size > sum) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        size, region_type_name[type]);
                எனில் (type != PCI_REGION_TYPE_IO) {
                    size = ALIGN(size, align);
                }
            } இல்லையெனில் {
                size = ALIGN(sum, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            கட்டமைப்பு pci_region_entry *entry = pci_region_create_entry(
                parent, s->bus_dev, -1, size, align, type, is64);
            எனில் (!entry)
                திரும்பு -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      entry->dev->secondary_bus, size,
                      region_type_name[entry->type]);
        }
    }
    திரும்பு 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
நிலையான முழு pci_bios_init_வேர்_regions_io(கட்டமைப்பு pci_bus *bus)
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
    கட்டமைப்பு pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 sum = pci_region_sum(r_io);
    எனில் (sum < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->base = 0xc000;
    } இல்லையெனில் எனில் (sum < pci_io_கீழ்_end - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->base = 0x1000;
    } இல்லையெனில் {
        /* not enouth io address space -> error out */
        திரும்பு -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->base, r_io->base + sum - 1);
    திரும்பு 0;
}

நிலையான முழு pci_bios_init_வேர்_regions_mem(கட்டமைப்பு pci_bus *bus)
{
    கட்டமைப்பு pci_region *r_end = &bus->r[PCI_REGION_TYPE_PREFMEM];
    கட்டமைப்பு pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    எனில் (pci_region_align(r_start) < pci_region_align(r_end)) {
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

    எனில் ((r_start->base < pcimem_start) ||
         (r_start->base > pcimem_end))
        // Memory range requested is larger than available.
        திரும்பு -1;
    திரும்பு 0;
}

#define PCI_IO_SHIFT            8
#define PCI_MEMORY_SHIFT        16
#define PCI_PREF_MEMORY_SHIFT   16

நிலையான வெறுமை
pci_region_map_one_entry(கட்டமைப்பு pci_region_entry *entry, u64 addr)
{
    எனில் (entry->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                entry->dev,
                entry->bar, addr, entry->size, region_type_name[entry->type]);

        pci_set_io_region_addr(entry->dev, entry->bar, addr, entry->is64);
        திரும்பு;
    }

    u16 bdf = entry->dev->bdf;
    u64 limit = addr + entry->size - 1;
    எனில் (entry->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_BASE, addr >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_BASE_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_LIMIT, limit >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_LIMIT_UPPER16, 0);
    }
    எனில் (entry->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_MEMORY_BASE, addr >> PCI_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_MEMORY_LIMIT, limit >> PCI_MEMORY_SHIFT);
    }
    எனில் (entry->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_MEMORY_BASE, addr >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_PREF_MEMORY_LIMIT, limit >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writel(bdf, PCI_PREF_BASE_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_LIMIT_UPPER32, limit >> 32);
    }
}

நிலையான வெறுமை pci_region_map_entries(கட்டமைப்பு pci_bus *busses, கட்டமைப்பு pci_region *r)
{
    கட்டமைப்பு hlist_முனை *n;
    கட்டமைப்பு pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &r->list, முனை) {
        u64 addr = r->base;
        r->base += entry->size;
        எனில் (entry->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[entry->dev->secondary_bus].r[entry->type].base = addr;
        pci_region_map_one_entry(entry, addr);
        hlist_del(&entry->முனை);
        விடுவி(entry);
    }
}

நிலையான வெறுமை pci_bios_map_devices(கட்டமைப்பு pci_bus *busses)
{
    எனில் (pci_bios_init_வேர்_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_end);
    எனில் (pci_pad_mem64 || pci_bios_init_வேர்_regions_mem(busses)) {
        கட்டமைப்பு pci_region r64_mem, r64_pref;
        r64_mem.list.first = வெற்று;
        r64_pref.list.first = வெற்று;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        எனில் (pci_bios_init_வேர்_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 sum_mem = pci_region_sum(&r64_mem);
        u64 sum_pref = pci_region_sum(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.base = le64_to_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        எனில் (r64_mem.base < 0x100000000LL + RamSizeOver4G)
            r64_mem.base = 0x100000000LL + RamSizeOver4G;
        எனில் (pci_mem64_உச்சி) {
            u64 size = (ALIGN(sum_mem, (1LL<<30)) +
                        ALIGN(sum_pref, (1LL<<30)));
            எனில் (pci_pad_mem64)
                size = ALIGN(size, pci_mem64_உச்சி >> 3);
            எனில் (r64_mem.base < pci_mem64_உச்சி - size) {
                r64_mem.base = pci_mem64_உச்சி - size;
            }
            எனில் (e820_is_used(r64_mem.base, size))
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
    } இல்லையெனில் {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    முழு bus;
    க்கு (bus = 0; bus<=MaxPCIBus; bus++) {
        முழு type;
        க்கு (type = 0; type < PCI_REGION_TYPE_மொத்தம்; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

வெறுமை
pci_setup(வெறுமை)
{
    எனில் (!CONFIG_QEMU)
        திரும்பு;

    dprintf(3, "pci setup\n");

    எனில் (CPUPhysBits) {
        pci_mem64_உச்சி = 1LL << CPUPhysBits;
        எனில் (CPUPhysBits > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            எனில் (RamSizeOver4G < (1LL << 40)) {
                pci_mem64_உச்சி = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    எனில் (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = RamSizeOver4G ? 1 : 0;

    எனில் (!CPULongMode)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    எனில் (pci_probe_host() != 0) {
        திரும்பு;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = RamSize;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    கட்டமைப்பு pci_bus *busses = malloc_tmp(அளவு(*busses) * (MaxPCIBus + 1));
    எனில் (!busses) {
        warn_noalloc();
        திரும்பு;
    }
    memset(busses, 0, அளவு(*busses) * (MaxPCIBus + 1));
    எனில் (pci_bios_check_devices(busses))
        திரும்பு;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    விடுவி(busses);

    pci_enable_default_vga();
}
