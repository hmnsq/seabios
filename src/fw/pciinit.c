// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "byteorder.h" // le64_to_cpu
#समावेश "config.h" // CONFIG_*
#समावेश "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#समावेश "dev-piix.h" // PIIX_*
#समावेश "e820map.h" // e820_add
#समावेश "hw/ata.h" // PORT_ATA1_CMD_BASE
#समावेश "hw/pci.h" // pci_config_readl
#समावेश "hw/pcidevice.h" // pci_probe_devices
#समावेश "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#समावेश "hw/pci_regs.h" // PCI_COMMAND
#समावेश "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#समावेश "list.h" // struct hlist_node
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // RamSize
#समावेश "romfile.h" // romfile_loadint
#समावेश "string.h" // memset
#समावेश "util.h" // pci_setup
#समावेश "x86.h" // outb

#घोषणा PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#घोषणा PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#घोषणा PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#घोषणा PCI_ROM_SLOT 6
#घोषणा PCI_अंक_REGIONS 7
#घोषणा PCI_BRIDGE_अंक_REGIONS 2

क्रमागत pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_गिनती,
};

अटल स्थिर अक्षर *region_type_name[] = {
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
अटल u64 pci_io_निचला_end = 0xa000;
अटल u64 pci_mem64_शीर्ष  = 0;
अटल u32 pci_pad_mem64  = -1;

काष्ठा pci_region_entry {
    काष्ठा pci_device *dev;
    पूर्णांक bar;
    u64 आकार;
    u64 align;
    पूर्णांक is64;
    क्रमागत pci_region_type type;
    काष्ठा hlist_गाँठ गाँठ;
};

काष्ठा pci_region {
    /* pci region assignments */
    u64 base;
    काष्ठा hlist_शीर्ष list;
};

काष्ठा pci_bus {
    काष्ठा pci_region r[PCI_REGION_TYPE_गिनती];
    काष्ठा pci_device *bus_dev;
};

अटल u32 pci_bar(काष्ठा pci_device *pci, पूर्णांक region_अंक)
{
    अगर (region_अंक != PCI_ROM_SLOT) {
        वापस PCI_BASE_ADDRESS_0 + region_अंक * 4;
    }

#घोषणा PCI_HEADER_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
    वापस type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

अटल व्योम
pci_set_io_region_addr(काष्ठा pci_device *pci, पूर्णांक bar, u64 addr, पूर्णांक is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    अगर (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
स्थिर u8 pci_irqs[4] = {
    10, 10, 11, 11
};

अटल पूर्णांक dummy_pci_slot_get_irq(काष्ठा pci_device *pci, पूर्णांक pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    वापस 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

अटल पूर्णांक (*pci_slot_get_irq)(काष्ठा pci_device *pci, पूर्णांक pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
अटल पूर्णांक piix_pci_slot_get_irq(काष्ठा pci_device *pci, पूर्णांक pin)
{
    पूर्णांक slot_addend = 0;

    जबतक (pci->parent != शून्य) {
        slot_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_to_dev(pci->bdf) - 1;
    वापस pci_irqs[(pin - 1 + slot_addend) & 3];
}

अटल पूर्णांक mch_pci_slot_get_irq(काष्ठा pci_device *pci, पूर्णांक pin)
{
    पूर्णांक pin_addend = 0;
    जबतक (pci->parent != शून्य) {
        pin_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_to_dev(pci->bdf);
    अगर (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        वापस pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    वापस pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
अटल व्योम piix_isa_bridge_setup(काष्ठा pci_device *pci, व्योम *arg)
{
    पूर्णांक i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    क्रम (i = 0; i < 4; i++) {
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

अटल व्योम mch_isa_lpc_setup(u16 bdf)
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

अटल पूर्णांक ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
अटल व्योम mch_isa_bridge_setup(काष्ठा pci_device *dev, व्योम *arg)
{
    u16 bdf = dev->bdf;
    पूर्णांक i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    क्रम (i = 0; i < 4; i++) {
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

अटल व्योम storage_ide_setup(काष्ठा pci_device *pci, व्योम *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_set_io_region_addr(pci, 0, PORT_ATA1_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 1, PORT_ATA1_CTRL_BASE, 0);
    pci_set_io_region_addr(pci, 2, PORT_ATA2_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 3, PORT_ATA2_CTRL_BASE, 0);
}

/* PIIX3/PIIX4 IDE */
अटल व्योम piix_ide_setup(काष्ठा pci_device *pci, व्योम *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

अटल व्योम pic_ibm_setup(काष्ठा pci_device *pci, व्योम *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_set_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

अटल व्योम apple_macio_setup(काष्ठा pci_device *pci, व्योम *arg)
{
    /* macio bridge */
    pci_set_io_region_addr(pci, 0, 0x80800000, 0);
}

अटल व्योम piix4_pm_config_setup(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_INTERRUPT_पंक्ति, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_base | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_base + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

अटल पूर्णांक PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
अटल व्योम piix4_pm_setup(काष्ठा pci_device *pci, व्योम *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_setup(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

अटल व्योम ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_BASE,
                      (acpi_pm_base + 0x100) | PCI_BASE_ADDRESS_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

अटल पूर्णांक ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
अटल व्योम ich9_smbus_setup(काष्ठा pci_device *dev, व्योम *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

अटल व्योम intel_igd_setup(काष्ठा pci_device *dev, व्योम *arg)
{
    काष्ठा romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_आकार = le64_to_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    अगर (opregion && opregion->आकार) {
        व्योम *addr = memalign_ऊपरी(PAGE_आकार, opregion->आकार);
        अगर (!addr) {
            warn_noalloc();
            वापस;
        }

        अगर (opregion->copy(opregion, addr, opregion->आकार) < 0) {
            मुक्त(addr);
            वापस;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->आकार >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    अगर (bdsm_आकार && (dev->bdf == pci_to_bdf(0, 2, 0))) {
        व्योम *addr = memalign_tmphigh(1024 * 1024, bdsm_आकार);
        अगर (!addr) {
            warn_noalloc();
            वापस;
        }

        e820_add((u32)addr, bdsm_आकार, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_आकार >> 20, dev);
    }
}

अटल स्थिर काष्ठा pci_device_id pci_device_tbl[] = {
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

अटल पूर्णांक MCHMmcfgBDF = -1;
अटल व्योम mch_mmconfig_setup(u16 bdf);

व्योम pci_resume(व्योम)
{
    अगर (!CONFIG_QEMU) {
        वापस;
    }

    अगर (PiixPmBDF >= 0) {
        piix4_pm_config_setup(PiixPmBDF);
    }

    अगर (ICH9LpcBDF >= 0) {
        mch_isa_lpc_setup(ICH9LpcBDF);
    }

    अगर (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    अगर(MCHMmcfgBDF >= 0) {
        mch_mmconfig_setup(MCHMmcfgBDF);
    }
}

अटल व्योम pci_bios_init_device(काष्ठा pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    पूर्णांक pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
    अगर (pin != 0)
        pci_config_writeb(bdf, PCI_INTERRUPT_पंक्ति, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, शून्य);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    अगर (pci->header_type & PCI_HEADER_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

अटल व्योम pci_bios_init_devices(व्योम)
{
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

अटल व्योम pci_enable_default_vga(व्योम)
{
    काष्ठा pci_device *pci;

    foreachpci(pci) {
        अगर (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            वापस;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    अगर (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        वापस;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

    जबतक (pci->parent) {
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

अटल व्योम i440fx_mem_addr_setup(काष्ठा pci_device *dev, व्योम *arg)
{
    अगर (Ram_आकार <= 0x80000000)
        pcimem_start = 0x80000000;
    अन्यथा अगर (Ram_आकार <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

अटल व्योम mch_mmconfig_setup(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 upper = addr >> 32;
    u32 lower = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, upper);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, lower);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

अटल व्योम mch_mem_addr_setup(काष्ठा pci_device *dev, व्योम *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 आकार = Q35_HOST_BRIDGE_PCIEXBAR_आकार;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_setup(dev->bdf);
    e820_add(addr, आकार, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_start = addr + आकार;

    pci_slot_get_irq = mch_pci_slot_get_irq;

    /* setup io address space */
    अगर (acpi_pm_base < 0x1000)
        pci_io_निचला_end = 0x10000;
    अन्यथा
        pci_io_निचला_end = acpi_pm_base;
}

अटल स्थिर काष्ठा pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_setup),
    PCI_DEVICE_END
};

अटल व्योम pci_bios_init_platform(व्योम)
{
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, शून्य);
    }
}

अटल u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    अगर (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        वापस 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    अगर (device_id != PCI_DEVICE_ID_REDHAT_मूल_PORT &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        वापस 0;
    }
    u8 cap = 0;

    करो {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } जबतक (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_OFFSET) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    अगर (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        अगर (cap_len < RES_RESERVE_CAP_आकार) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            वापस 0;
        }
    } अन्यथा {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    वापस cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

अटल व्योम
pci_bios_init_bus_rec(पूर्णांक bus, u8 *pci_bus)
{
    पूर्णांक bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        अगर (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        अगर (class != PCI_CLASS_BRIDGE_PCI) {
            जारी;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        अगर (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } अन्यथा {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        अगर (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } अन्यथा {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        अगर (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            अगर (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                अगर (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    अगर ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    अगर (secbus + res_bus > *pci_bus) {
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
        } अन्यथा {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

अटल व्योम
pci_bios_init_bus(व्योम)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    अगर (extraroots) {
        जबतक (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

अटल व्योम
pci_bios_get_bar(काष्ठा pci_device *pci, पूर्णांक bar,
                 पूर्णांक *ptype, u64 *psize, पूर्णांक *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    पूर्णांक is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 mask;

    अगर (bar == PCI_ROM_SLOT) {
        mask = PCI_ROM_ADDRESS_MASK;
        pci_config_writel(bdf, ofs, mask);
    } अन्यथा {
        अगर (old & PCI_BASE_ADDRESS_SPACE_IO) {
            mask = PCI_BASE_ADDRESS_IO_MASK;
            type = PCI_REGION_TYPE_IO;
        } अन्यथा {
            mask = PCI_BASE_ADDRESS_MEM_MASK;
            अगर (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK)
                    == PCI_BASE_ADDRESS_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    अगर (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 ऊपरी = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)ऊपरी << 32);
        mask |= ((u64)0xffffffff << 32);
        *psize = (~(val & mask)) + 1;
    } अन्यथा {
        *psize = ((~(val & mask)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

अटल पूर्णांक pci_bios_bridge_region_is64(काष्ठा pci_region *r,
                                 काष्ठा pci_device *pci, पूर्णांक type)
{
    अगर (type != PCI_REGION_TYPE_PREFMEM)
        वापस 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
    अगर (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0x0);
    }
    अगर ((pmem & PCI_PREF_RANGE_TYPE_MASK) != PCI_PREF_RANGE_TYPE_64)
       वापस 0;
    काष्ठा pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, गाँठ) {
        अगर (!entry->is64)
            वापस 0;
    }
    वापस 1;
}

अटल u64 pci_region_align(काष्ठा pci_region *r)
{
    काष्ठा pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, गाँठ) {
        // The first entry in the sorted list has the largest alignment
        वापस entry->align;
    }
    वापस 1;
}

अटल u64 pci_region_योग(काष्ठा pci_region *r)
{
    u64 योग = 0;
    काष्ठा pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, गाँठ) {
        योग += entry->आकार;
    }
    वापस योग;
}

अटल व्योम pci_region_migrate_64bit_entries(काष्ठा pci_region *from,
                                             काष्ठा pci_region *to)
{
    काष्ठा hlist_गाँठ *n, **last = &to->list.first;
    काष्ठा pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &from->list, गाँठ) {
        अगर (!entry->is64)
            जारी;
        अगर (entry->dev->class == PCI_CLASS_SERIAL_USB)
            जारी;
        अगर (entry->dev->class == PCI_CLASS_STORAGE_NVME)
            जारी;
        // Move from source list to destination list.
        hlist_del(&entry->गाँठ);
        hlist_add(&entry->गाँठ, last);
        last = &entry->गाँठ.अग्र;
    }
}

अटल काष्ठा pci_region_entry *
pci_region_create_entry(काष्ठा pci_bus *bus, काष्ठा pci_device *dev,
                        पूर्णांक bar, u64 आकार, u64 align, पूर्णांक type, पूर्णांक is64)
{
    काष्ठा pci_region_entry *entry = malloc_tmp(माप(*entry));
    अगर (!entry) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(entry, 0, माप(*entry));
    entry->dev = dev;
    entry->bar = bar;
    entry->आकार = आकार;
    entry->align = align;
    entry->is64 = is64;
    entry->type = type;
    // Insert into list in sorted order.
    काष्ठा hlist_गाँठ **pprev;
    काष्ठा pci_region_entry *pos;
    hlist_for_each_entry_pprev(pos, pprev, &bus->r[type].list, गाँठ) {
        अगर (pos->align < align || (pos->align == align && pos->आकार < आकार))
            अवरोध;
    }
    hlist_add(&entry->गाँठ, pprev);
    वापस entry;
}

प्रकार क्रमागत hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

अटल hotplug_type_t pci_bus_hotplug_support(काष्ठा pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    अगर (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 port_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        अगर (port_type == PCI_EXP_TYPE_PCI_BRIDGE)
            जाओ check_shpc;

        u8 downstream_port = (port_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (port_type == PCI_EXP_TYPE_मूल_PORT);
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

        वापस downstream_port && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    वापस !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
अटल पूर्णांक pci_bridge_has_region(काष्ठा pci_device *pci,
                                 क्रमागत pci_region_type region_type)
{
    u8 base;

    चयन (region_type) {
        हाल PCI_REGION_TYPE_IO:
            base = PCI_IO_BASE;
            अवरोध;
        हाल PCI_REGION_TYPE_PREFMEM:
            base = PCI_PREF_MEMORY_BASE;
            अवरोध;
        शेष:
            /* Regular memory support is mandatory */
            वापस 1;
    }

    pci_config_writeb(pci->bdf, base, 0xFF);

    वापस pci_config_readb(pci->bdf, base) != 0;
}

अटल पूर्णांक pci_bios_check_devices(काष्ठा pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        काष्ठा pci_bus *bus = &busses[pci_bdf_to_bus(pci->bdf)];
        अगर (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        पूर्णांक i;
        क्रम (i = 0; i < PCI_अंक_REGIONS; i++) {
            अगर ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_अंक_REGIONS && i < PCI_ROM_SLOT))
                जारी;
            पूर्णांक type, is64;
            u64 आकार;
            pci_bios_get_bar(pci, i, &type, &आकार, &is64);
            अगर (आकार == 0)
                जारी;

            अगर (type != PCI_REGION_TYPE_IO && आकार < PCI_DEVICE_MEM_MIN)
                आकार = PCI_DEVICE_MEM_MIN;
            काष्ठा pci_region_entry *entry = pci_region_create_entry(
                bus, pci, i, आकार, आकार, type, is64);
            अगर (!entry)
                वापस -1;

            अगर (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    पूर्णांक secondary_bus;
    क्रम (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        काष्ठा pci_bus *s = &busses[secondary_bus];
        अगर (!s->bus_dev)
            जारी;
        काष्ठा pci_bus *parent = &busses[pci_bdf_to_bus(s->bus_dev->bdf)];
        अगर (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        पूर्णांक type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        क्रम (type = 0; type < PCI_REGION_TYPE_गिनती; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            अगर (!pci_bridge_has_region(s->bus_dev, type))
                जारी;
            u64 आकार = 0;
            अगर (qemu_cap) {
                u32 tmp_आकार;
                u64 tmp_आकार_64;
                चयन(type) {
                हाल PCI_REGION_TYPE_IO:
                    tmp_आकार_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    अगर (tmp_आकार_64 != (u64)-1) {
                        आकार = tmp_आकार_64;
                    }
                    अवरोध;
                हाल PCI_REGION_TYPE_MEM:
                    tmp_आकार = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    अगर (tmp_आकार != (u32)-1) {
                        आकार = tmp_आकार;
                    }
                    अवरोध;
                हाल PCI_REGION_TYPE_PREFMEM:
                    tmp_आकार = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_आकार_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    अगर (tmp_आकार != (u32)-1 && tmp_आकार_64 == (u64)-1) {
                        आकार = tmp_आकार;
                    } अन्यथा अगर (tmp_आकार == (u32)-1 && tmp_आकार_64 != (u64)-1) {
                        आकार = tmp_आकार_64;
                    } अन्यथा अगर (tmp_आकार != (u32)-1 && tmp_आकार_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    अवरोध;
                शेष:
                    अवरोध;
                }
            }
            अगर (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 योग = pci_region_योग(&s->r[type]);
            पूर्णांक is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            पूर्णांक resource_optional = 0;
            अगर (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 शीर्ष_align = pci_mem64_शीर्ष >> 11;
            अगर (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (शीर्ष_align > align))
                align = शीर्ष_align;
            अगर (align > योग && hotplug_support && !resource_optional)
                योग = align; /* reserve min size for hot-plug */
            अगर (आकार > योग) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        आकार, region_type_name[type]);
                अगर (type != PCI_REGION_TYPE_IO) {
                    आकार = ALIGN(आकार, align);
                }
            } अन्यथा {
                आकार = ALIGN(योग, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            काष्ठा pci_region_entry *entry = pci_region_create_entry(
                parent, s->bus_dev, -1, आकार, align, type, is64);
            अगर (!entry)
                वापस -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      entry->dev->secondary_bus, आकार,
                      region_type_name[entry->type]);
        }
    }
    वापस 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
अटल पूर्णांक pci_bios_init_मूल_regions_io(काष्ठा pci_bus *bus)
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
    काष्ठा pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 योग = pci_region_योग(r_io);
    अगर (योग < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->base = 0xc000;
    } अन्यथा अगर (योग < pci_io_निचला_end - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->base = 0x1000;
    } अन्यथा {
        /* not enouth io address space -> error out */
        वापस -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->base, r_io->base + योग - 1);
    वापस 0;
}

अटल पूर्णांक pci_bios_init_मूल_regions_mem(काष्ठा pci_bus *bus)
{
    काष्ठा pci_region *r_end = &bus->r[PCI_REGION_TYPE_PREFMEM];
    काष्ठा pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    अगर (pci_region_align(r_start) < pci_region_align(r_end)) {
        // Swap regions to improve alignment.
        r_end = r_start;
        r_start = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 योग = pci_region_योग(r_end);
    u64 align = pci_region_align(r_end);
    r_end->base = ALIGN_DOWN((pcimem_end - योग), align);
    योग = pci_region_योग(r_start);
    align = pci_region_align(r_start);
    r_start->base = ALIGN_DOWN((r_end->base - योग), align);

    अगर ((r_start->base < pcimem_start) ||
         (r_start->base > pcimem_end))
        // Memory range requested is larger than available.
        वापस -1;
    वापस 0;
}

#घोषणा PCI_IO_SHIFT            8
#घोषणा PCI_MEMORY_SHIFT        16
#घोषणा PCI_PREF_MEMORY_SHIFT   16

अटल व्योम
pci_region_map_one_entry(काष्ठा pci_region_entry *entry, u64 addr)
{
    अगर (entry->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                entry->dev,
                entry->bar, addr, entry->आकार, region_type_name[entry->type]);

        pci_set_io_region_addr(entry->dev, entry->bar, addr, entry->is64);
        वापस;
    }

    u16 bdf = entry->dev->bdf;
    u64 limit = addr + entry->आकार - 1;
    अगर (entry->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_BASE, addr >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_BASE_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_LIMIT, limit >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_LIMIT_UPPER16, 0);
    }
    अगर (entry->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_MEMORY_BASE, addr >> PCI_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_MEMORY_LIMIT, limit >> PCI_MEMORY_SHIFT);
    }
    अगर (entry->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_MEMORY_BASE, addr >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_PREF_MEMORY_LIMIT, limit >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writel(bdf, PCI_PREF_BASE_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_LIMIT_UPPER32, limit >> 32);
    }
}

अटल व्योम pci_region_map_entries(काष्ठा pci_bus *busses, काष्ठा pci_region *r)
{
    काष्ठा hlist_गाँठ *n;
    काष्ठा pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &r->list, गाँठ) {
        u64 addr = r->base;
        r->base += entry->आकार;
        अगर (entry->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[entry->dev->secondary_bus].r[entry->type].base = addr;
        pci_region_map_one_entry(entry, addr);
        hlist_del(&entry->गाँठ);
        मुक्त(entry);
    }
}

अटल व्योम pci_bios_map_devices(काष्ठा pci_bus *busses)
{
    अगर (pci_bios_init_मूल_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_end);
    अगर (pci_pad_mem64 || pci_bios_init_मूल_regions_mem(busses)) {
        काष्ठा pci_region r64_mem, r64_pref;
        r64_mem.list.first = शून्य;
        r64_pref.list.first = शून्य;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        अगर (pci_bios_init_मूल_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 योग_mem = pci_region_योग(&r64_mem);
        u64 योग_pref = pci_region_योग(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.base = le64_to_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        अगर (r64_mem.base < 0x100000000LL + Ram_आकार_Over4_G)
            r64_mem.base = 0x100000000LL + Ram_आकार_Over4_G;
        अगर (pci_mem64_शीर्ष) {
            u64 आकार = (ALIGN(योग_mem, (1LL<<30)) +
                        ALIGN(योग_pref, (1LL<<30)));
            अगर (pci_pad_mem64)
                आकार = ALIGN(आकार, pci_mem64_शीर्ष >> 3);
            अगर (r64_mem.base < pci_mem64_शीर्ष - आकार) {
                r64_mem.base = pci_mem64_शीर्ष - आकार;
            }
            अगर (e820_is_used(r64_mem.base, आकार))
                r64_mem.base -= आकार;
        }
        r64_mem.base = ALIGN(r64_mem.base, align_mem);
        r64_mem.base = ALIGN(r64_mem.base, (1LL<<30));    // 1G hugepage
        r64_pref.base = r64_mem.base + योग_mem;
        r64_pref.base = ALIGN(r64_pref.base, align_pref);
        r64_pref.base = ALIGN(r64_pref.base, (1LL<<30));  // 1G hugepage
        pcimem64_start = r64_mem.base;
        pcimem64_end = r64_pref.base + योग_pref;
        pcimem64_end = ALIGN(pcimem64_end, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_start, pcimem64_end);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } अन्यथा {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    पूर्णांक bus;
    क्रम (bus = 0; bus<=MaxPCIBus; bus++) {
        पूर्णांक type;
        क्रम (type = 0; type < PCI_REGION_TYPE_गिनती; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

व्योम
pci_setup(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    dprintf(3, "pci setup\n");

    अगर (CPUPhysBits) {
        pci_mem64_शीर्ष = 1LL << CPUPhysBits;
        अगर (CPUPhysBits > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            अगर (Ram_आकार_Over4_G < (1LL << 40)) {
                pci_mem64_शीर्ष = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    अगर (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_आकार_Over4_G ? 1 : 0;

    अगर (!CPULongMode)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    अगर (pci_probe_host() != 0) {
        वापस;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = Ram_आकार;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    काष्ठा pci_bus *busses = malloc_tmp(माप(*busses) * (MaxPCIBus + 1));
    अगर (!busses) {
        warn_noalloc();
        वापस;
    }
    स_रखो(busses, 0, माप(*busses) * (MaxPCIBus + 1));
    अगर (pci_bios_check_devices(busses))
        वापस;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    मुक्त(busses);

    pci_enable_default_vga();
}
