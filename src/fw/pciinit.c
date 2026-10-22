// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "byteorder.h" // le64_to_cpu
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#अन्तर्भाव "dev-piix.h" // PIIX_*
#अन्तर्भाव "e820map.h" // e820_add
#अन्तर्भाव "hw/ata.h" // PORT_ATA1_CMD_BASE
#अन्तर्भाव "hw/pci.h" // pci_config_readl
#अन्तर्भाव "hw/pcidevice.h" // pci_probe_devices
#अन्तर्भाव "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#अन्तर्भाव "hw/pci_regs.h" // PCI_COMMAND
#अन्तर्भाव "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#अन्तर्भाव "list.h" // struct hlist_node
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // RamSize
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // pci_setup
#अन्तर्भाव "x86.h" // outb

#परिभाषय PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#परिभाषय PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#परिभाषय PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#परिभाषय PCI_ROM_SLOT 6
#परिभाषय PCI_अङ्कः_REGIONS 7
#परिभाषय PCI_BRIDGE_अङ्कः_REGIONS 2

गणना pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_गणनम्,
};

स्थिर ध्रुव वर्ण *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_start   = BUILD_PCIMEM_START;
u64 pcimem_अन्तः     = BUILD_PCIMEM_अन्तः;
u64 pcimem64_start = BUILD_PCIMEM64_START;
u64 pcimem64_अन्तः   = BUILD_PCIMEM64_अन्तः;

// Resource allocation limits
स्थिर u64 pci_io_नीचः_अन्तः = 0xa000;
स्थिर u64 pci_mem64_शिखरम्  = 0;
स्थिर u32 pci_pad_mem64  = -1;

संरचना pci_region_प्रविष्टिः {
    संरचना pci_device *dev;
    पूर्णाङ्क bar;
    u64 आकार;
    u64 align;
    पूर्णाङ्क is64;
    गणना pci_region_type type;
    संरचना hlist_ग्रन्थिः ग्रन्थिः;
};

संरचना pci_region {
    /* pci region assignments */
    u64 आधारः;
    संरचना hlist_शीर्ष list;
};

संरचना pci_bus {
    संरचना pci_region r[PCI_REGION_TYPE_गणनम्];
    संरचना pci_device *bus_dev;
};

स्थिर u32 pci_bar(संरचना pci_device *pci, पूर्णाङ्क region_अङ्कः)
{
    यदि (region_अङ्कः != PCI_ROM_SLOT) {
        प्रत्यागम PCI_आधारः_स्थानाङ्कः_0 + region_अङ्कः * 4;
    }

#परिभाषय PCI_शीर्षकम्_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->शीर्षकम्_type & ~PCI_शीर्षकम्_TYPE_MULTI_FUNCTION;
    प्रत्यागम type == PCI_शीर्षकम्_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_स्थानाङ्कः;
}

स्थिर शून्य
pci_समुच्चयः_io_region_addr(संरचना pci_device *pci, पूर्णाङ्क bar, u64 addr, पूर्णाङ्क is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    यदि (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
ध्रुव u8 pci_irqs[4] = {
    10, 10, 11, 11
};

स्थिर पूर्णाङ्क dummy_pci_slot_get_irq(संरचना pci_device *pci, पूर्णाङ्क pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    प्रत्यागम 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

स्थिर पूर्णाङ्क (*pci_slot_get_irq)(संरचना pci_device *pci, पूर्णाङ्क pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
स्थिर पूर्णाङ्क piix_pci_slot_get_irq(संरचना pci_device *pci, पूर्णाङ्क pin)
{
    पूर्णाङ्क slot_addend = 0;

    यावत् (pci->parent != रिक्त) {
        slot_addend += pci_bdf_गन्तव्यम्_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_गन्तव्यम्_dev(pci->bdf) - 1;
    प्रत्यागम pci_irqs[(pin - 1 + slot_addend) & 3];
}

स्थिर पूर्णाङ्क mch_pci_slot_get_irq(संरचना pci_device *pci, पूर्णाङ्क pin)
{
    पूर्णाङ्क pin_addend = 0;
    यावत् (pci->parent != रिक्त) {
        pin_addend += pci_bdf_गन्तव्यम्_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_गन्तव्यम्_dev(pci->bdf);
    यदि (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        प्रत्यागम pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    प्रत्यागम pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
स्थिर शून्य piix_isa_bridge_सज्जता(संरचना pci_device *pci, शून्य *arg)
{
    पूर्णाङ्क i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    प्रति (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_पत्तनम्_ELCR1);
    outb(elcr[1], PIIX_पत्तनम्_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}

स्थिर शून्य mch_isa_lpc_सज्जता(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_आधारः | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

स्थिर पूर्णाङ्क ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
स्थिर शून्य mch_isa_bridge_सज्जता(संरचना pci_device *dev, शून्य *arg)
{
    u16 bdf = dev->bdf;
    पूर्णाङ्क i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    प्रति (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));

        /* activate irq remapping in LPC */

        /* PIRQ[A-D] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT + i, irq);
        /* PIRQ[E-H] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT + i, irq);
    }
    outb(elcr[0], ICH9_LPC_पत्तनम्_ELCR1);
    outb(elcr[1], ICH9_LPC_पत्तनम्_ELCR2);
    dprintf(1, "Q35 LPC init: elcr=%02x %02x\n", elcr[0], elcr[1]);

    ICH9LpcBDF = bdf;

    mch_isa_lpc_सज्जता(bdf);

    e820_सङ्कलय(ICH9_LPC_RCBA_ADDR, 16*1024, E820_RESERVED);

    acpi_pm1a_cnt = acpi_pm_आधारः + 0x04;
    pmtimer_सज्जता(acpi_pm_आधारः + 0x08);
}

स्थिर शून्य storage_ide_सज्जता(संरचना pci_device *pci, शून्य *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_समुच्चयः_io_region_addr(pci, 0, पत्तनम्_ATA1_CMD_आधारः, 0);
    pci_समुच्चयः_io_region_addr(pci, 1, पत्तनम्_ATA1_CTRL_आधारः, 0);
    pci_समुच्चयः_io_region_addr(pci, 2, पत्तनम्_ATA2_CMD_आधारः, 0);
    pci_समुच्चयः_io_region_addr(pci, 3, पत्तनम्_ATA2_CTRL_आधारः, 0);
}

/* PIIX3/PIIX4 IDE */
स्थिर शून्य piix_ide_सज्जता(संरचना pci_device *pci, शून्य *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

स्थिर शून्य pic_ibm_सज्जता(संरचना pci_device *pci, शून्य *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_समुच्चयः_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

स्थिर शून्य apple_macio_सज्जता(संरचना pci_device *pci, शून्य *arg)
{
    /* macio bridge */
    pci_समुच्चयः_io_region_addr(pci, 0, 0x80800000, 0);
}

स्थिर शून्य piix4_pm_config_सज्जता(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_व्यवधानम्_पङ्क्तिः, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_आधारः | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_आधारः + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

स्थिर पूर्णाङ्क PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
स्थिर शून्य piix4_pm_सज्जता(संरचना pci_device *pci, शून्य *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_सज्जता(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_आधारः + 0x04;
    pmtimer_सज्जता(acpi_pm_आधारः + 0x08);
}

स्थिर शून्य ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_आधारः,
                      (acpi_pm_आधारः + 0x100) | PCI_आधारः_स्थानाङ्कः_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

स्थिर पूर्णाङ्क ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
स्थिर शून्य ich9_smbus_सज्जता(संरचना pci_device *dev, शून्य *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

स्थिर शून्य intel_igd_सज्जता(संरचना pci_device *dev, शून्य *arg)
{
    संरचना romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_आकार = le64_गन्तव्यम्_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    यदि (opregion && opregion->आकार) {
        शून्य *addr = memalign_उच्चः(PAGE_आकार, opregion->आकार);
        यदि (!addr) {
            warn_noalloc();
            प्रत्यागम;
        }

        यदि (opregion->copy(opregion, addr, opregion->आकार) < 0) {
            मुक्त(addr);
            प्रत्यागम;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_गन्तव्यम्_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->आकार >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    यदि (bdsm_आकार && (dev->bdf == pci_गन्तव्यम्_bdf(0, 2, 0))) {
        शून्य *addr = memalign_tmphigh(1024 * 1024, bdsm_आकार);
        यदि (!addr) {
            warn_noalloc();
            प्रत्यागम;
        }

        e820_सङ्कलय((u32)addr, bdsm_आकार, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_गन्तव्यम्_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_आकार >> 20, dev);
    }
}

स्थिर ध्रुव संरचना pci_device_id pci_device_tbl[] = {
    /* PIIX3/PIIX4 PCI to ISA bridge */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0,
               piix_isa_bridge_सज्जता),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0,
               piix_isa_bridge_सज्जता),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC,
               mch_isa_bridge_सज्जता),

    /* STORAGE IDE */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1,
                     PCI_CLASS_STORAGE_IDE, piix_ide_सज्जता),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB,
                     PCI_CLASS_STORAGE_IDE, piix_ide_सज्जता),
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
                     storage_ide_सज्जता),

    /* PIC, IBM, MPIC & MPIC2 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0x0046, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_सज्जता),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0xFFFF, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_सज्जता),

    /* PIIX4 Power Management device (for ACPI) */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3,
               piix4_pm_सज्जता),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_SMBUS,
               ich9_smbus_सज्जता),

    /* 0xff00 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0017, 0xff00, apple_macio_सज्जता),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0022, 0xff00, apple_macio_सज्जता),

    /* Intel IGD OpRegion setup */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
                     intel_igd_सज्जता),

    PCI_DEVICE_अन्तः,
};

स्थिर पूर्णाङ्क MCHMmcfgBDF = -1;
स्थिर शून्य mch_mmconfig_सज्जता(u16 bdf);

शून्य pci_resume(शून्य)
{
    यदि (!CONFIG_QEMU) {
        प्रत्यागम;
    }

    यदि (PiixPmBDF >= 0) {
        piix4_pm_config_सज्जता(PiixPmBDF);
    }

    यदि (ICH9LpcBDF >= 0) {
        mch_isa_lpc_सज्जता(ICH9LpcBDF);
    }

    यदि (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    यदि(MCHMmcfgBDF >= 0) {
        mch_mmconfig_सज्जता(MCHMmcfgBDF);
    }
}

स्थिर शून्य pci_bios_init_device(संरचना pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    पूर्णाङ्क pin = pci_config_readb(bdf, PCI_व्यवधानम्_PIN);
    यदि (pin != 0)
        pci_config_writeb(bdf, PCI_व्यवधानम्_पङ्क्तिः, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, रिक्त);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_स्मृतिः | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    यदि (pci->शीर्षकम्_type & PCI_शीर्षकम्_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

स्थिर शून्य pci_bios_init_devices(शून्य)
{
    संरचना pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

स्थिर शून्य pci_enable_default_vga(शून्य)
{
    संरचना pci_device *pci;

    foreachpci(pci) {
        यदि (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            प्रत्यागम;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    यदि (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        प्रत्यागम;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_स्मृतिः);

    यावत् (pci->parent) {
        pci = pci->parent;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_स्मृतिः);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

स्थिर शून्य i440fx_mem_addr_सज्जता(संरचना pci_device *dev, शून्य *arg)
{
    यदि (Ram_आकार <= 0x80000000)
        pcimem_start = 0x80000000;
    अन्यथा यदि (Ram_आकार <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

स्थिर शून्य mch_mmconfig_सज्जता(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 ऊर्ध्वस्थम् = addr >> 32;
    u32 अधस्तनम् = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, ऊर्ध्वस्थम्);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, अधस्तनम्);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

स्थिर शून्य mch_mem_addr_सज्जता(संरचना pci_device *dev, शून्य *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 आकार = Q35_HOST_BRIDGE_PCIEXBAR_आकार;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_सज्जता(dev->bdf);
    e820_सङ्कलय(addr, आकार, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_start = addr + आकार;

    pci_slot_get_irq = mch_pci_slot_get_irq;

    /* setup io address space */
    यदि (acpi_pm_आधारः < 0x1000)
        pci_io_नीचः_अन्तः = 0x10000;
    अन्यथा
        pci_io_नीचः_अन्तः = acpi_pm_आधारः;
}

स्थिर ध्रुव संरचना pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_सज्जता),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_सज्जता),
    PCI_DEVICE_अन्तः
};

स्थिर शून्य pci_bios_init_platform(शून्य)
{
    संरचना pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, रिक्त);
    }
}

स्थिर u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    यदि (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        प्रत्यागम 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    यदि (device_id != PCI_DEVICE_ID_REDHAT_मूलम्_पत्तनम् &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        प्रत्यागम 0;
    }
    u8 cap = 0;

    कुरु {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } यावत् (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_विस्थापनम्) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    यदि (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        यदि (cap_len < RES_RESERVE_CAP_आकार) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            प्रत्यागम 0;
        }
    } अन्यथा {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    प्रत्यागम cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

स्थिर शून्य
pci_bios_init_bus_rec(पूर्णाङ्क bus, u8 *pci_bus)
{
    पूर्णाङ्क bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        यदि (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        यदि (class != PCI_CLASS_BRIDGE_PCI) {
            अनुवर्त;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        यदि (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } अन्यथा {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        यदि (*pci_bus != secbus) {
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

        यदि (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            यदि (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                यदि (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    यदि ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    यदि (secbus + res_bus > *pci_bus) {
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

स्थिर शून्य
pci_bios_init_bus(शून्य)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    यदि (extraroots) {
        यावत् (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

स्थिर शून्य
pci_bios_get_bar(संरचना pci_device *pci, पूर्णाङ्क bar,
                 पूर्णाङ्क *ptype, u64 *psize, पूर्णाङ्क *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    पूर्णाङ्क is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 आवरणम्;

    यदि (bar == PCI_ROM_SLOT) {
        आवरणम् = PCI_ROM_स्थानाङ्कः_आवरणम्;
        pci_config_writel(bdf, ofs, आवरणम्);
    } अन्यथा {
        यदि (old & PCI_आधारः_स्थानाङ्कः_SPACE_IO) {
            आवरणम् = PCI_आधारः_स्थानाङ्कः_IO_आवरणम्;
            type = PCI_REGION_TYPE_IO;
        } अन्यथा {
            आवरणम् = PCI_आधारः_स्थानाङ्कः_MEM_आवरणम्;
            यदि (old & PCI_आधारः_स्थानाङ्कः_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_आधारः_स्थानाङ्कः_MEM_TYPE_आवरणम्)
                    == PCI_आधारः_स्थानाङ्कः_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    यदि (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 उच्चः = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)उच्चः << 32);
        आवरणम् |= ((u64)0xffffffff << 32);
        *psize = (~(val & आवरणम्)) + 1;
    } अन्यथा {
        *psize = ((~(val & आवरणम्)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

स्थिर पूर्णाङ्क pci_bios_bridge_region_is64(संरचना pci_region *r,
                                 संरचना pci_device *pci, पूर्णाङ्क type)
{
    यदि (type != PCI_REGION_TYPE_PREFMEM)
        प्रत्यागम 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_स्मृतिः_आधारः);
    यदि (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_स्मृतिः_आधारः, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_स्मृतिः_आधारः);
        pci_config_writel(pci->bdf, PCI_PREF_स्मृतिः_आधारः, 0x0);
    }
    यदि ((pmem & PCI_PREF_RANGE_TYPE_आवरणम्) != PCI_PREF_RANGE_TYPE_64)
       प्रत्यागम 0;
    संरचना pci_region_प्रविष्टिः *प्रविष्टिः;
    hlist_for_each_प्रविष्टिः(प्रविष्टिः, &r->list, ग्रन्थिः) {
        यदि (!प्रविष्टिः->is64)
            प्रत्यागम 0;
    }
    प्रत्यागम 1;
}

स्थिर u64 pci_region_align(संरचना pci_region *r)
{
    संरचना pci_region_प्रविष्टिः *प्रविष्टिः;
    hlist_for_each_प्रविष्टिः(प्रविष्टिः, &r->list, ग्रन्थिः) {
        // The first entry in the sorted list has the largest alignment
        प्रत्यागम प्रविष्टिः->align;
    }
    प्रत्यागम 1;
}

स्थिर u64 pci_region_योग(संरचना pci_region *r)
{
    u64 योग = 0;
    संरचना pci_region_प्रविष्टिः *प्रविष्टिः;
    hlist_for_each_प्रविष्टिः(प्रविष्टिः, &r->list, ग्रन्थिः) {
        योग += प्रविष्टिः->आकार;
    }
    प्रत्यागम योग;
}

स्थिर शून्य pci_region_migrate_64bit_entries(संरचना pci_region *from,
                                             संरचना pci_region *गन्तव्यम्)
{
    संरचना hlist_ग्रन्थिः *n, **चरमः = &गन्तव्यम्->list.first;
    संरचना pci_region_प्रविष्टिः *प्रविष्टिः;
    hlist_for_each_प्रविष्टिः_safe(प्रविष्टिः, n, &from->list, ग्रन्थिः) {
        यदि (!प्रविष्टिः->is64)
            अनुवर्त;
        यदि (प्रविष्टिः->dev->class == PCI_CLASS_SERIAL_USB)
            अनुवर्त;
        यदि (प्रविष्टिः->dev->class == PCI_CLASS_STORAGE_NVME)
            अनुवर्त;
        // Move from source list to destination list.
        hlist_del(&प्रविष्टिः->ग्रन्थिः);
        hlist_सङ्कलय(&प्रविष्टिः->ग्रन्थिः, चरमः);
        चरमः = &प्रविष्टिः->ग्रन्थिः.अग्र;
    }
}

स्थिर संरचना pci_region_प्रविष्टिः *
pci_region_create_प्रविष्टिः(संरचना pci_bus *bus, संरचना pci_device *dev,
                        पूर्णाङ्क bar, u64 आकार, u64 align, पूर्णाङ्क type, पूर्णाङ्क is64)
{
    संरचना pci_region_प्रविष्टिः *प्रविष्टिः = malloc_tmp(परिमाण(*प्रविष्टिः));
    यदि (!प्रविष्टिः) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स्मृतिं_पूरय(प्रविष्टिः, 0, परिमाण(*प्रविष्टिः));
    प्रविष्टिः->dev = dev;
    प्रविष्टिः->bar = bar;
    प्रविष्टिः->आकार = आकार;
    प्रविष्टिः->align = align;
    प्रविष्टिः->is64 = is64;
    प्रविष्टिः->type = type;
    // Insert into list in sorted order.
    संरचना hlist_ग्रन्थिः **pprev;
    संरचना pci_region_प्रविष्टिः *pos;
    hlist_for_each_प्रविष्टिः_pprev(pos, pprev, &bus->r[type].list, ग्रन्थिः) {
        यदि (pos->align < align || (pos->align == align && pos->आकार < आकार))
            भङ्ग;
    }
    hlist_सङ्कलय(&प्रविष्टिः->ग्रन्थिः, pprev);
    प्रत्यागम प्रविष्टिः;
}

प्रकारनिर्देश गणना hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

स्थिर hotplug_type_t pci_bus_hotplug_support(संरचना pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    यदि (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 पत्तनम्_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        यदि (पत्तनम्_type == PCI_EXP_TYPE_PCI_BRIDGE)
            गच्छ check_shpc;

        u8 downstream_पत्तनम् = (पत्तनम्_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (पत्तनम्_type == PCI_EXP_TYPE_मूलम्_पत्तनम्);
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

        प्रत्यागम downstream_पत्तनम् && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    प्रत्यागम !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
स्थिर पूर्णाङ्क pci_bridge_has_region(संरचना pci_device *pci,
                                 गणना pci_region_type region_type)
{
    u8 आधारः;

    विकल्प (region_type) {
        पक्ष PCI_REGION_TYPE_IO:
            आधारः = PCI_IO_आधारः;
            भङ्ग;
        पक्ष PCI_REGION_TYPE_PREFMEM:
            आधारः = PCI_PREF_स्मृतिः_आधारः;
            भङ्ग;
        स्वभाव:
            /* Regular memory support is mandatory */
            प्रत्यागम 1;
    }

    pci_config_writeb(pci->bdf, आधारः, 0xFF);

    प्रत्यागम pci_config_readb(pci->bdf, आधारः) != 0;
}

स्थिर पूर्णाङ्क pci_bios_check_devices(संरचना pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        संरचना pci_bus *bus = &busses[pci_bdf_गन्तव्यम्_bus(pci->bdf)];
        यदि (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        पूर्णाङ्क i;
        प्रति (i = 0; i < PCI_अङ्कः_REGIONS; i++) {
            यदि ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_अङ्कः_REGIONS && i < PCI_ROM_SLOT))
                अनुवर्त;
            पूर्णाङ्क type, is64;
            u64 आकार;
            pci_bios_get_bar(pci, i, &type, &आकार, &is64);
            यदि (आकार == 0)
                अनुवर्त;

            यदि (type != PCI_REGION_TYPE_IO && आकार < PCI_DEVICE_MEM_MIN)
                आकार = PCI_DEVICE_MEM_MIN;
            संरचना pci_region_प्रविष्टिः *प्रविष्टिः = pci_region_create_प्रविष्टिः(
                bus, pci, i, आकार, आकार, type, is64);
            यदि (!प्रविष्टिः)
                प्रत्यागम -1;

            यदि (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    पूर्णाङ्क secondary_bus;
    प्रति (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        संरचना pci_bus *s = &busses[secondary_bus];
        यदि (!s->bus_dev)
            अनुवर्त;
        संरचना pci_bus *parent = &busses[pci_bdf_गन्तव्यम्_bus(s->bus_dev->bdf)];
        यदि (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        पूर्णाङ्क type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        प्रति (type = 0; type < PCI_REGION_TYPE_गणनम्; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            यदि (!pci_bridge_has_region(s->bus_dev, type))
                अनुवर्त;
            u64 आकार = 0;
            यदि (qemu_cap) {
                u32 tmp_आकार;
                u64 tmp_आकार_64;
                विकल्प(type) {
                पक्ष PCI_REGION_TYPE_IO:
                    tmp_आकार_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    यदि (tmp_आकार_64 != (u64)-1) {
                        आकार = tmp_आकार_64;
                    }
                    भङ्ग;
                पक्ष PCI_REGION_TYPE_MEM:
                    tmp_आकार = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    यदि (tmp_आकार != (u32)-1) {
                        आकार = tmp_आकार;
                    }
                    भङ्ग;
                पक्ष PCI_REGION_TYPE_PREFMEM:
                    tmp_आकार = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_आकार_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    यदि (tmp_आकार != (u32)-1 && tmp_आकार_64 == (u64)-1) {
                        आकार = tmp_आकार;
                    } अन्यथा यदि (tmp_आकार == (u32)-1 && tmp_आकार_64 != (u64)-1) {
                        आकार = tmp_आकार_64;
                    } अन्यथा यदि (tmp_आकार != (u32)-1 && tmp_आकार_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    भङ्ग;
                स्वभाव:
                    भङ्ग;
                }
            }
            यदि (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 योग = pci_region_योग(&s->r[type]);
            पूर्णाङ्क is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            पूर्णाङ्क resource_optional = 0;
            यदि (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 शिखरम्_align = pci_mem64_शिखरम् >> 11;
            यदि (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (शिखरम्_align > align))
                align = शिखरम्_align;
            यदि (align > योग && hotplug_support && !resource_optional)
                योग = align; /* reserve min size for hot-plug */
            यदि (आकार > योग) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        आकार, region_type_name[type]);
                यदि (type != PCI_REGION_TYPE_IO) {
                    आकार = ALIGN(आकार, align);
                }
            } अन्यथा {
                आकार = ALIGN(योग, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            संरचना pci_region_प्रविष्टिः *प्रविष्टिः = pci_region_create_प्रविष्टिः(
                parent, s->bus_dev, -1, आकार, align, type, is64);
            यदि (!प्रविष्टिः)
                प्रत्यागम -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      प्रविष्टिः->dev->secondary_bus, आकार,
                      region_type_name[प्रविष्टिः->type]);
        }
    }
    प्रत्यागम 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
स्थिर पूर्णाङ्क pci_bios_init_मूलम्_regions_io(संरचना pci_bus *bus)
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
    संरचना pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 योग = pci_region_योग(r_io);
    यदि (योग < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->आधारः = 0xc000;
    } अन्यथा यदि (योग < pci_io_नीचः_अन्तः - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->आधारः = 0x1000;
    } अन्यथा {
        /* not enouth io address space -> error out */
        प्रत्यागम -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->आधारः, r_io->आधारः + योग - 1);
    प्रत्यागम 0;
}

स्थिर पूर्णाङ्क pci_bios_init_मूलम्_regions_mem(संरचना pci_bus *bus)
{
    संरचना pci_region *r_अन्तः = &bus->r[PCI_REGION_TYPE_PREFMEM];
    संरचना pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    यदि (pci_region_align(r_start) < pci_region_align(r_अन्तः)) {
        // Swap regions to improve alignment.
        r_अन्तः = r_start;
        r_start = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 योग = pci_region_योग(r_अन्तः);
    u64 align = pci_region_align(r_अन्तः);
    r_अन्तः->आधारः = ALIGN_अधस्तात्((pcimem_अन्तः - योग), align);
    योग = pci_region_योग(r_start);
    align = pci_region_align(r_start);
    r_start->आधारः = ALIGN_अधस्तात्((r_अन्तः->आधारः - योग), align);

    यदि ((r_start->आधारः < pcimem_start) ||
         (r_start->आधारः > pcimem_अन्तः))
        // Memory range requested is larger than available.
        प्रत्यागम -1;
    प्रत्यागम 0;
}

#परिभाषय PCI_IO_सरणम्            8
#परिभाषय PCI_स्मृतिः_सरणम्        16
#परिभाषय PCI_PREF_स्मृतिः_सरणम्   16

स्थिर शून्य
pci_region_map_one_प्रविष्टिः(संरचना pci_region_प्रविष्टिः *प्रविष्टिः, u64 addr)
{
    यदि (प्रविष्टिः->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                प्रविष्टिः->dev,
                प्रविष्टिः->bar, addr, प्रविष्टिः->आकार, region_type_name[प्रविष्टिः->type]);

        pci_समुच्चयः_io_region_addr(प्रविष्टिः->dev, प्रविष्टिः->bar, addr, प्रविष्टिः->is64);
        प्रत्यागम;
    }

    u16 bdf = प्रविष्टिः->dev->bdf;
    u64 सीमा = addr + प्रविष्टिः->आकार - 1;
    यदि (प्रविष्टिः->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_आधारः, addr >> PCI_IO_सरणम्);
        pci_config_writew(bdf, PCI_IO_आधारः_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_सीमा, सीमा >> PCI_IO_सरणम्);
        pci_config_writew(bdf, PCI_IO_सीमा_UPPER16, 0);
    }
    यदि (प्रविष्टिः->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_स्मृतिः_आधारः, addr >> PCI_स्मृतिः_सरणम्);
        pci_config_writew(bdf, PCI_स्मृतिः_सीमा, सीमा >> PCI_स्मृतिः_सरणम्);
    }
    यदि (प्रविष्टिः->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_स्मृतिः_आधारः, addr >> PCI_PREF_स्मृतिः_सरणम्);
        pci_config_writew(bdf, PCI_PREF_स्मृतिः_सीमा, सीमा >> PCI_PREF_स्मृतिः_सरणम्);
        pci_config_writel(bdf, PCI_PREF_आधारः_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_सीमा_UPPER32, सीमा >> 32);
    }
}

स्थिर शून्य pci_region_map_entries(संरचना pci_bus *busses, संरचना pci_region *r)
{
    संरचना hlist_ग्रन्थिः *n;
    संरचना pci_region_प्रविष्टिः *प्रविष्टिः;
    hlist_for_each_प्रविष्टिः_safe(प्रविष्टिः, n, &r->list, ग्रन्थिः) {
        u64 addr = r->आधारः;
        r->आधारः += प्रविष्टिः->आकार;
        यदि (प्रविष्टिः->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[प्रविष्टिः->dev->secondary_bus].r[प्रविष्टिः->type].आधारः = addr;
        pci_region_map_one_प्रविष्टिः(प्रविष्टिः, addr);
        hlist_del(&प्रविष्टिः->ग्रन्थिः);
        मुक्त(प्रविष्टिः);
    }
}

स्थिर शून्य pci_bios_map_devices(संरचना pci_bus *busses)
{
    यदि (pci_bios_init_मूलम्_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_अन्तः);
    यदि (pci_pad_mem64 || pci_bios_init_मूलम्_regions_mem(busses)) {
        संरचना pci_region r64_mem, r64_pref;
        r64_mem.list.first = रिक्त;
        r64_pref.list.first = रिक्त;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        यदि (pci_bios_init_मूलम्_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 योग_mem = pci_region_योग(&r64_mem);
        u64 योग_pref = pci_region_योग(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.आधारः = le64_गन्तव्यम्_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        यदि (r64_mem.आधारः < 0x100000000LL + Ram_आकार_Over4_G)
            r64_mem.आधारः = 0x100000000LL + Ram_आकार_Over4_G;
        यदि (pci_mem64_शिखरम्) {
            u64 आकार = (ALIGN(योग_mem, (1LL<<30)) +
                        ALIGN(योग_pref, (1LL<<30)));
            यदि (pci_pad_mem64)
                आकार = ALIGN(आकार, pci_mem64_शिखरम् >> 3);
            यदि (r64_mem.आधारः < pci_mem64_शिखरम् - आकार) {
                r64_mem.आधारः = pci_mem64_शिखरम् - आकार;
            }
            यदि (e820_is_प्रयुक्तम्(r64_mem.आधारः, आकार))
                r64_mem.आधारः -= आकार;
        }
        r64_mem.आधारः = ALIGN(r64_mem.आधारः, align_mem);
        r64_mem.आधारः = ALIGN(r64_mem.आधारः, (1LL<<30));    // 1G hugepage
        r64_pref.आधारः = r64_mem.आधारः + योग_mem;
        r64_pref.आधारः = ALIGN(r64_pref.आधारः, align_pref);
        r64_pref.आधारः = ALIGN(r64_pref.आधारः, (1LL<<30));  // 1G hugepage
        pcimem64_start = r64_mem.आधारः;
        pcimem64_अन्तः = r64_pref.आधारः + योग_pref;
        pcimem64_अन्तः = ALIGN(pcimem64_अन्तः, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_start, pcimem64_अन्तः);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } अन्यथा {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    पूर्णाङ्क bus;
    प्रति (bus = 0; bus<=MaxPCIBus; bus++) {
        पूर्णाङ्क type;
        प्रति (type = 0; type < PCI_REGION_TYPE_गणनम्; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

शून्य
pci_सज्जता(शून्य)
{
    यदि (!CONFIG_QEMU)
        प्रत्यागम;

    dprintf(3, "pci setup\n");

    यदि (CPUPhys_द्वयङ्काः) {
        pci_mem64_शिखरम् = 1LL << CPUPhys_द्वयङ्काः;
        यदि (CPUPhys_द्वयङ्काः > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            यदि (Ram_आकार_Over4_G < (1LL << 40)) {
                pci_mem64_शिखरम् = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    यदि (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_आकार_Over4_G ? 1 : 0;

    यदि (!CPULong_बहुलकम्)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    यदि (pci_probe_host() != 0) {
        प्रत्यागम;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = Ram_आकार;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    संरचना pci_bus *busses = malloc_tmp(परिमाण(*busses) * (MaxPCIBus + 1));
    यदि (!busses) {
        warn_noalloc();
        प्रत्यागम;
    }
    स्मृतिं_पूरय(busses, 0, परिमाण(*busses) * (MaxPCIBus + 1));
    यदि (pci_bios_check_devices(busses))
        प्रत्यागम;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    मुक्त(busses);

    pci_enable_default_vga();
}
