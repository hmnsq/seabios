// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "byteorder.h" // le64_to_cpu
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#અન્તર્ભાવ "dev-piix.h" // PIIX_*
#અન્તર્ભાવ "e820map.h" // e820_add
#અન્તર્ભાવ "hw/ata.h" // PORT_ATA1_CMD_BASE
#અન્તર્ભાવ "hw/pci.h" // pci_config_readl
#અન્તર્ભાવ "hw/pcidevice.h" // pci_probe_devices
#અન્તર્ભાવ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#અન્તર્ભાવ "hw/pci_regs.h" // PCI_COMMAND
#અન્તર્ભાવ "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#અન્તર્ભાવ "list.h" // struct hlist_node
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "paravirt.h" // RamSize
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // pci_setup
#અન્તર્ભાવ "x86.h" // outb

#define PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#define PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#define PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#define PCI_ROM_SLOT 6
#define PCI_અંક_REGIONS 7
#define PCI_BRIDGE_અંક_REGIONS 2

ગણના pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_ગણતરી,
};

સ્થિર અચલ અક્ષર *region_type_name[] = {
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
સ્થિર u64 pci_io_નીચલું_end = 0xa000;
સ્થિર u64 pci_mem64_ટોચ  = 0;
સ્થિર u32 pci_pad_mem64  = -1;

સંરચના pci_region_entry {
    સંરચના pci_device *dev;
    પૂર્ણાંક bar;
    u64 size;
    u64 align;
    પૂર્ણાંક is64;
    ગણના pci_region_type type;
    સંરચના hlist_ગાંઠ ગાંઠ;
};

સંરચના pci_region {
    /* pci region assignments */
    u64 base;
    સંરચના hlist_head list;
};

સંરચના pci_bus {
    સંરચના pci_region r[PCI_REGION_TYPE_ગણતરી];
    સંરચના pci_device *bus_dev;
};

સ્થિર u32 pci_bar(સંરચના pci_device *pci, પૂર્ણાંક region_અંક)
{
    જો (region_અંક != PCI_ROM_SLOT) {
        પાછા PCI_BASE_ADDRESS_0 + region_અંક * 4;
    }

#define PCI_HEADER_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
    પાછા type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

સ્થિર શૂન્ય
pci_set_io_region_addr(સંરચના pci_device *pci, પૂર્ણાંક bar, u64 addr, પૂર્ણાંક is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    જો (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
અચલ u8 pci_irqs[4] = {
    10, 10, 11, 11
};

સ્થિર પૂર્ણાંક dummy_pci_slot_get_irq(સંરચના pci_device *pci, પૂર્ણાંક pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    પાછા 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

સ્થિર પૂર્ણાંક (*pci_slot_get_irq)(સંરચના pci_device *pci, પૂર્ણાંક pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
સ્થિર પૂર્ણાંક piix_pci_slot_get_irq(સંરચના pci_device *pci, પૂર્ણાંક pin)
{
    પૂર્ણાંક slot_addend = 0;

    જ્યારે (pci->parent != ખાલી) {
        slot_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_to_dev(pci->bdf) - 1;
    પાછા pci_irqs[(pin - 1 + slot_addend) & 3];
}

સ્થિર પૂર્ણાંક mch_pci_slot_get_irq(સંરચના pci_device *pci, પૂર્ણાંક pin)
{
    પૂર્ણાંક pin_addend = 0;
    જ્યારે (pci->parent != ખાલી) {
        pin_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_to_dev(pci->bdf);
    જો (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        પાછા pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    પાછા pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
સ્થિર શૂન્ય piix_isa_bridge_setup(સંરચના pci_device *pci, શૂન્ય *arg)
{
    પૂર્ણાંક i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    માટે (i = 0; i < 4; i++) {
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

સ્થિર શૂન્ય mch_isa_lpc_setup(u16 bdf)
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

સ્થિર પૂર્ણાંક ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
સ્થિર શૂન્ય mch_isa_bridge_setup(સંરચના pci_device *dev, શૂન્ય *arg)
{
    u16 bdf = dev->bdf;
    પૂર્ણાંક i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    માટે (i = 0; i < 4; i++) {
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

સ્થિર શૂન્ય storage_ide_setup(સંરચના pci_device *pci, શૂન્ય *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_set_io_region_addr(pci, 0, PORT_ATA1_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 1, PORT_ATA1_CTRL_BASE, 0);
    pci_set_io_region_addr(pci, 2, PORT_ATA2_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 3, PORT_ATA2_CTRL_BASE, 0);
}

/* PIIX3/PIIX4 IDE */
સ્થિર શૂન્ય piix_ide_setup(સંરચના pci_device *pci, શૂન્ય *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

સ્થિર શૂન્ય pic_ibm_setup(સંરચના pci_device *pci, શૂન્ય *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_set_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

સ્થિર શૂન્ય apple_macio_setup(સંરચના pci_device *pci, શૂન્ય *arg)
{
    /* macio bridge */
    pci_set_io_region_addr(pci, 0, 0x80800000, 0);
}

સ્થિર શૂન્ય piix4_pm_config_setup(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_INTERRUPT_લીટી, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_base | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_base + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

સ્થિર પૂર્ણાંક PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
સ્થિર શૂન્ય piix4_pm_setup(સંરચના pci_device *pci, શૂન્ય *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_setup(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

સ્થિર શૂન્ય ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_BASE,
                      (acpi_pm_base + 0x100) | PCI_BASE_ADDRESS_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

સ્થિર પૂર્ણાંક ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
સ્થિર શૂન્ય ich9_smbus_setup(સંરચના pci_device *dev, શૂન્ય *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

સ્થિર શૂન્ય intel_igd_setup(સંરચના pci_device *dev, શૂન્ય *arg)
{
    સંરચના romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_size = le64_to_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    જો (opregion && opregion->size) {
        શૂન્ય *addr = memalign_ઉપલું(PAGE_SIZE, opregion->size);
        જો (!addr) {
            warn_noalloc();
            પાછા;
        }

        જો (opregion->copy(opregion, addr, opregion->size) < 0) {
            મુક્ત(addr);
            પાછા;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->size >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    જો (bdsm_size && (dev->bdf == pci_to_bdf(0, 2, 0))) {
        શૂન્ય *addr = memalign_tmphigh(1024 * 1024, bdsm_size);
        જો (!addr) {
            warn_noalloc();
            પાછા;
        }

        e820_add((u32)addr, bdsm_size, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_size >> 20, dev);
    }
}

સ્થિર અચલ સંરચના pci_device_id pci_device_tbl[] = {
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

સ્થિર પૂર્ણાંક MCHMmcfgBDF = -1;
સ્થિર શૂન્ય mch_mmconfig_setup(u16 bdf);

શૂન્ય pci_resume(શૂન્ય)
{
    જો (!CONFIG_QEMU) {
        પાછા;
    }

    જો (PiixPmBDF >= 0) {
        piix4_pm_config_setup(PiixPmBDF);
    }

    જો (ICH9LpcBDF >= 0) {
        mch_isa_lpc_setup(ICH9LpcBDF);
    }

    જો (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    જો(MCHMmcfgBDF >= 0) {
        mch_mmconfig_setup(MCHMmcfgBDF);
    }
}

સ્થિર શૂન્ય pci_bios_init_device(સંરચના pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    પૂર્ણાંક pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
    જો (pin != 0)
        pci_config_writeb(bdf, PCI_INTERRUPT_લીટી, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, ખાલી);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    જો (pci->header_type & PCI_HEADER_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

સ્થિર શૂન્ય pci_bios_init_devices(શૂન્ય)
{
    સંરચના pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

સ્થિર શૂન્ય pci_enable_default_vga(શૂન્ય)
{
    સંરચના pci_device *pci;

    foreachpci(pci) {
        જો (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            પાછા;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    જો (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        પાછા;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

    જ્યારે (pci->parent) {
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

સ્થિર શૂન્ય i440fx_mem_addr_setup(સંરચના pci_device *dev, શૂન્ય *arg)
{
    જો (RamSize <= 0x80000000)
        pcimem_start = 0x80000000;
    નહીંતર જો (RamSize <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

સ્થિર શૂન્ય mch_mmconfig_setup(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 upper = addr >> 32;
    u32 lower = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, upper);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, lower);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

સ્થિર શૂન્ય mch_mem_addr_setup(સંરચના pci_device *dev, શૂન્ય *arg)
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
    જો (acpi_pm_base < 0x1000)
        pci_io_નીચલું_end = 0x10000;
    નહીંતર
        pci_io_નીચલું_end = acpi_pm_base;
}

સ્થિર અચલ સંરચના pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_setup),
    PCI_DEVICE_END
};

સ્થિર શૂન્ય pci_bios_init_platform(શૂન્ય)
{
    સંરચના pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, ખાલી);
    }
}

સ્થિર u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    જો (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        પાછા 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    જો (device_id != PCI_DEVICE_ID_REDHAT_મૂળ_PORT &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        પાછા 0;
    }
    u8 cap = 0;

    કરો {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } જ્યારે (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_OFFSET) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    જો (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        જો (cap_len < RES_RESERVE_CAP_SIZE) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            પાછા 0;
        }
    } નહીંતર {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    પાછા cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

સ્થિર શૂન્ય
pci_bios_init_bus_rec(પૂર્ણાંક bus, u8 *pci_bus)
{
    પૂર્ણાંક bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        જો (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        જો (class != PCI_CLASS_BRIDGE_PCI) {
            ચાલુ;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        જો (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } નહીંતર {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        જો (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } નહીંતર {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        જો (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            જો (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                જો (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    જો ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    જો (secbus + res_bus > *pci_bus) {
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
        } નહીંતર {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

સ્થિર શૂન્ય
pci_bios_init_bus(શૂન્ય)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    જો (extraroots) {
        જ્યારે (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

સ્થિર શૂન્ય
pci_bios_get_bar(સંરચના pci_device *pci, પૂર્ણાંક bar,
                 પૂર્ણાંક *ptype, u64 *psize, પૂર્ણાંક *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    પૂર્ણાંક is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 mask;

    જો (bar == PCI_ROM_SLOT) {
        mask = PCI_ROM_ADDRESS_MASK;
        pci_config_writel(bdf, ofs, mask);
    } નહીંતર {
        જો (old & PCI_BASE_ADDRESS_SPACE_IO) {
            mask = PCI_BASE_ADDRESS_IO_MASK;
            type = PCI_REGION_TYPE_IO;
        } નહીંતર {
            mask = PCI_BASE_ADDRESS_MEM_MASK;
            જો (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK)
                    == PCI_BASE_ADDRESS_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    જો (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 ઉપલું = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)ઉપલું << 32);
        mask |= ((u64)0xffffffff << 32);
        *psize = (~(val & mask)) + 1;
    } નહીંતર {
        *psize = ((~(val & mask)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

સ્થિર પૂર્ણાંક pci_bios_bridge_region_is64(સંરચના pci_region *r,
                                 સંરચના pci_device *pci, પૂર્ણાંક type)
{
    જો (type != PCI_REGION_TYPE_PREFMEM)
        પાછા 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
    જો (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0x0);
    }
    જો ((pmem & PCI_PREF_RANGE_TYPE_MASK) != PCI_PREF_RANGE_TYPE_64)
       પાછા 0;
    સંરચના pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, ગાંઠ) {
        જો (!entry->is64)
            પાછા 0;
    }
    પાછા 1;
}

સ્થિર u64 pci_region_align(સંરચના pci_region *r)
{
    સંરચના pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, ગાંઠ) {
        // The first entry in the sorted list has the largest alignment
        પાછા entry->align;
    }
    પાછા 1;
}

સ્થિર u64 pci_region_sum(સંરચના pci_region *r)
{
    u64 sum = 0;
    સંરચના pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, ગાંઠ) {
        sum += entry->size;
    }
    પાછા sum;
}

સ્થિર શૂન્ય pci_region_migrate_64bit_entries(સંરચના pci_region *from,
                                             સંરચના pci_region *to)
{
    સંરચના hlist_ગાંઠ *n, **last = &to->list.first;
    સંરચના pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &from->list, ગાંઠ) {
        જો (!entry->is64)
            ચાલુ;
        જો (entry->dev->class == PCI_CLASS_SERIAL_USB)
            ચાલુ;
        જો (entry->dev->class == PCI_CLASS_STORAGE_NVME)
            ચાલુ;
        // Move from source list to destination list.
        hlist_del(&entry->ગાંઠ);
        hlist_add(&entry->ગાંઠ, last);
        last = &entry->ગાંઠ.next;
    }
}

સ્થિર સંરચના pci_region_entry *
pci_region_create_entry(સંરચના pci_bus *bus, સંરચના pci_device *dev,
                        પૂર્ણાંક bar, u64 size, u64 align, પૂર્ણાંક type, પૂર્ણાંક is64)
{
    સંરચના pci_region_entry *entry = malloc_tmp(કદ(*entry));
    જો (!entry) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(entry, 0, કદ(*entry));
    entry->dev = dev;
    entry->bar = bar;
    entry->size = size;
    entry->align = align;
    entry->is64 = is64;
    entry->type = type;
    // Insert into list in sorted order.
    સંરચના hlist_ગાંઠ **pprev;
    સંરચના pci_region_entry *pos;
    hlist_for_each_entry_pprev(pos, pprev, &bus->r[type].list, ગાંઠ) {
        જો (pos->align < align || (pos->align == align && pos->size < size))
            તોડો;
    }
    hlist_add(&entry->ગાંઠ, pprev);
    પાછા entry;
}

પ્રકારવ્યાખ્યા ગણના hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

સ્થિર hotplug_type_t pci_bus_hotplug_support(સંરચના pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    જો (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 port_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        જો (port_type == PCI_EXP_TYPE_PCI_BRIDGE)
            જાઓ check_shpc;

        u8 downstream_port = (port_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (port_type == PCI_EXP_TYPE_મૂળ_PORT);
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

        પાછા downstream_port && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    પાછા !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
સ્થિર પૂર્ણાંક pci_bridge_has_region(સંરચના pci_device *pci,
                                 ગણના pci_region_type region_type)
{
    u8 base;

    પસંદગી (region_type) {
        સ્થિતિ PCI_REGION_TYPE_IO:
            base = PCI_IO_BASE;
            તોડો;
        સ્થિતિ PCI_REGION_TYPE_PREFMEM:
            base = PCI_PREF_MEMORY_BASE;
            તોડો;
        મૂળભૂત:
            /* Regular memory support is mandatory */
            પાછા 1;
    }

    pci_config_writeb(pci->bdf, base, 0xFF);

    પાછા pci_config_readb(pci->bdf, base) != 0;
}

સ્થિર પૂર્ણાંક pci_bios_check_devices(સંરચના pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        સંરચના pci_bus *bus = &busses[pci_bdf_to_bus(pci->bdf)];
        જો (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        પૂર્ણાંક i;
        માટે (i = 0; i < PCI_અંક_REGIONS; i++) {
            જો ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_અંક_REGIONS && i < PCI_ROM_SLOT))
                ચાલુ;
            પૂર્ણાંક type, is64;
            u64 size;
            pci_bios_get_bar(pci, i, &type, &size, &is64);
            જો (size == 0)
                ચાલુ;

            જો (type != PCI_REGION_TYPE_IO && size < PCI_DEVICE_MEM_MIN)
                size = PCI_DEVICE_MEM_MIN;
            સંરચના pci_region_entry *entry = pci_region_create_entry(
                bus, pci, i, size, size, type, is64);
            જો (!entry)
                પાછા -1;

            જો (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    પૂર્ણાંક secondary_bus;
    માટે (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        સંરચના pci_bus *s = &busses[secondary_bus];
        જો (!s->bus_dev)
            ચાલુ;
        સંરચના pci_bus *parent = &busses[pci_bdf_to_bus(s->bus_dev->bdf)];
        જો (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        પૂર્ણાંક type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        માટે (type = 0; type < PCI_REGION_TYPE_ગણતરી; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            જો (!pci_bridge_has_region(s->bus_dev, type))
                ચાલુ;
            u64 size = 0;
            જો (qemu_cap) {
                u32 tmp_size;
                u64 tmp_size_64;
                પસંદગી(type) {
                સ્થિતિ PCI_REGION_TYPE_IO:
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    જો (tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    }
                    તોડો;
                સ્થિતિ PCI_REGION_TYPE_MEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    જો (tmp_size != (u32)-1) {
                        size = tmp_size;
                    }
                    તોડો;
                સ્થિતિ PCI_REGION_TYPE_PREFMEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    જો (tmp_size != (u32)-1 && tmp_size_64 == (u64)-1) {
                        size = tmp_size;
                    } નહીંતર જો (tmp_size == (u32)-1 && tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    } નહીંતર જો (tmp_size != (u32)-1 && tmp_size_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    તોડો;
                મૂળભૂત:
                    તોડો;
                }
            }
            જો (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 sum = pci_region_sum(&s->r[type]);
            પૂર્ણાંક is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            પૂર્ણાંક resource_optional = 0;
            જો (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 ટોચ_align = pci_mem64_ટોચ >> 11;
            જો (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (ટોચ_align > align))
                align = ટોચ_align;
            જો (align > sum && hotplug_support && !resource_optional)
                sum = align; /* reserve min size for hot-plug */
            જો (size > sum) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        size, region_type_name[type]);
                જો (type != PCI_REGION_TYPE_IO) {
                    size = ALIGN(size, align);
                }
            } નહીંતર {
                size = ALIGN(sum, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            સંરચના pci_region_entry *entry = pci_region_create_entry(
                parent, s->bus_dev, -1, size, align, type, is64);
            જો (!entry)
                પાછા -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      entry->dev->secondary_bus, size,
                      region_type_name[entry->type]);
        }
    }
    પાછા 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
સ્થિર પૂર્ણાંક pci_bios_init_મૂળ_regions_io(સંરચના pci_bus *bus)
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
    સંરચના pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 sum = pci_region_sum(r_io);
    જો (sum < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->base = 0xc000;
    } નહીંતર જો (sum < pci_io_નીચલું_end - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->base = 0x1000;
    } નહીંતર {
        /* not enouth io address space -> error out */
        પાછા -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->base, r_io->base + sum - 1);
    પાછા 0;
}

સ્થિર પૂર્ણાંક pci_bios_init_મૂળ_regions_mem(સંરચના pci_bus *bus)
{
    સંરચના pci_region *r_end = &bus->r[PCI_REGION_TYPE_PREFMEM];
    સંરચના pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    જો (pci_region_align(r_start) < pci_region_align(r_end)) {
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

    જો ((r_start->base < pcimem_start) ||
         (r_start->base > pcimem_end))
        // Memory range requested is larger than available.
        પાછા -1;
    પાછા 0;
}

#define PCI_IO_SHIFT            8
#define PCI_MEMORY_SHIFT        16
#define PCI_PREF_MEMORY_SHIFT   16

સ્થિર શૂન્ય
pci_region_map_one_entry(સંરચના pci_region_entry *entry, u64 addr)
{
    જો (entry->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                entry->dev,
                entry->bar, addr, entry->size, region_type_name[entry->type]);

        pci_set_io_region_addr(entry->dev, entry->bar, addr, entry->is64);
        પાછા;
    }

    u16 bdf = entry->dev->bdf;
    u64 limit = addr + entry->size - 1;
    જો (entry->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_BASE, addr >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_BASE_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_LIMIT, limit >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_LIMIT_UPPER16, 0);
    }
    જો (entry->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_MEMORY_BASE, addr >> PCI_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_MEMORY_LIMIT, limit >> PCI_MEMORY_SHIFT);
    }
    જો (entry->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_MEMORY_BASE, addr >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_PREF_MEMORY_LIMIT, limit >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writel(bdf, PCI_PREF_BASE_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_LIMIT_UPPER32, limit >> 32);
    }
}

સ્થિર શૂન્ય pci_region_map_entries(સંરચના pci_bus *busses, સંરચના pci_region *r)
{
    સંરચના hlist_ગાંઠ *n;
    સંરચના pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &r->list, ગાંઠ) {
        u64 addr = r->base;
        r->base += entry->size;
        જો (entry->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[entry->dev->secondary_bus].r[entry->type].base = addr;
        pci_region_map_one_entry(entry, addr);
        hlist_del(&entry->ગાંઠ);
        મુક્ત(entry);
    }
}

સ્થિર શૂન્ય pci_bios_map_devices(સંરચના pci_bus *busses)
{
    જો (pci_bios_init_મૂળ_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_end);
    જો (pci_pad_mem64 || pci_bios_init_મૂળ_regions_mem(busses)) {
        સંરચના pci_region r64_mem, r64_pref;
        r64_mem.list.first = ખાલી;
        r64_pref.list.first = ખાલી;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        જો (pci_bios_init_મૂળ_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 sum_mem = pci_region_sum(&r64_mem);
        u64 sum_pref = pci_region_sum(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.base = le64_to_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        જો (r64_mem.base < 0x100000000LL + RamSizeOver4G)
            r64_mem.base = 0x100000000LL + RamSizeOver4G;
        જો (pci_mem64_ટોચ) {
            u64 size = (ALIGN(sum_mem, (1LL<<30)) +
                        ALIGN(sum_pref, (1LL<<30)));
            જો (pci_pad_mem64)
                size = ALIGN(size, pci_mem64_ટોચ >> 3);
            જો (r64_mem.base < pci_mem64_ટોચ - size) {
                r64_mem.base = pci_mem64_ટોચ - size;
            }
            જો (e820_is_used(r64_mem.base, size))
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
    } નહીંતર {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    પૂર્ણાંક bus;
    માટે (bus = 0; bus<=MaxPCIBus; bus++) {
        પૂર્ણાંક type;
        માટે (type = 0; type < PCI_REGION_TYPE_ગણતરી; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

શૂન્ય
pci_setup(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    dprintf(3, "pci setup\n");

    જો (CPUPhysBits) {
        pci_mem64_ટોચ = 1LL << CPUPhysBits;
        જો (CPUPhysBits > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            જો (RamSizeOver4G < (1LL << 40)) {
                pci_mem64_ટોચ = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    જો (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = RamSizeOver4G ? 1 : 0;

    જો (!CPULongMode)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    જો (pci_probe_host() != 0) {
        પાછા;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = RamSize;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    સંરચના pci_bus *busses = malloc_tmp(કદ(*busses) * (MaxPCIBus + 1));
    જો (!busses) {
        warn_noalloc();
        પાછા;
    }
    memset(busses, 0, કદ(*busses) * (MaxPCIBus + 1));
    જો (pci_bios_check_devices(busses))
        પાછા;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    મુક્ત(busses);

    pci_enable_default_vga();
}
