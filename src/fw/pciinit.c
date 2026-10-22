// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "byteorder.h" // le64_to_cpu
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#అన్తర్భావ "dev-piix.h" // PIIX_*
#అన్తర్భావ "e820map.h" // e820_add
#అన్తర్భావ "hw/ata.h" // PORT_ATA1_CMD_BASE
#అన్తర్భావ "hw/pci.h" // pci_config_readl
#అన్తర్భావ "hw/pcidevice.h" // pci_probe_devices
#అన్తర్భావ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#అన్తర్భావ "hw/pci_regs.h" // PCI_COMMAND
#అన్తర్భావ "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#అన్తర్భావ "list.h" // struct hlist_node
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // RamSize
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // pci_setup
#అన్తర్భావ "x86.h" // outb

#నిర్వచించు PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#నిర్వచించు PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#నిర్వచించు PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#నిర్వచించు PCI_ROM_స్థానం 6
#నిర్వచించు PCI_అంకె_REGIONS 7
#నిర్వచించు PCI_BRIDGE_అంకె_REGIONS 2

గణన pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_లెక్క,
};

స్థిర స్థిరరాశి అక్షరం *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_ప్రారంభం   = BUILD_PCIMEM_ప్రారంభం;
u64 pcimem_చివర     = BUILD_PCIMEM_చివర;
u64 pcimem64_ప్రారంభం = BUILD_PCIMEM64_ప్రారంభం;
u64 pcimem64_చివర   = BUILD_PCIMEM64_చివర;

// Resource allocation limits
స్థిర u64 pci_io_దిగువ_చివర = 0xa000;
స్థిర u64 pci_mem64_శిఖరం  = 0;
స్థిర u32 pci_pad_mem64  = -1;

నిర్మాణం pci_region_నమోదు {
    నిర్మాణం pci_device *dev;
    పూర్ణాంకం bar;
    u64 కొలత;
    u64 align;
    పూర్ణాంకం is64;
    గణన pci_region_type type;
    నిర్మాణం hlist_కణుపు కణుపు;
};

నిర్మాణం pci_region {
    /* pci region assignments */
    u64 పునాది;
    నిర్మాణం hlist_తల list;
};

నిర్మాణం pci_bus {
    నిర్మాణం pci_region r[PCI_REGION_TYPE_లెక్క];
    నిర్మాణం pci_device *bus_dev;
};

స్థిర u32 pci_bar(నిర్మాణం pci_device *pci, పూర్ణాంకం region_అంకె)
{
    ఒకవేళ (region_అంకె != PCI_ROM_స్థానం) {
        తిరిగి PCI_పునాది_చిరునామా_0 + region_అంకె * 4;
    }

#నిర్వచించు PCI_శీర్షిక_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->శీర్షిక_type & ~PCI_శీర్షిక_TYPE_MULTI_FUNCTION;
    తిరిగి type == PCI_శీర్షిక_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_చిరునామా;
}

స్థిర శూన్య
pci_సమితి_io_region_addr(నిర్మాణం pci_device *pci, పూర్ణాంకం bar, u64 addr, పూర్ణాంకం is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    ఒకవేళ (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
స్థిరరాశి u8 pci_irqs[4] = {
    10, 10, 11, 11
};

స్థిర పూర్ణాంకం dummy_pci_స్థానం_get_irq(నిర్మాణం pci_device *pci, పూర్ణాంకం pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    తిరిగి 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

స్థిర పూర్ణాంకం (*pci_స్థానం_get_irq)(నిర్మాణం pci_device *pci, పూర్ణాంకం pin) =
    dummy_pci_స్థానం_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
స్థిర పూర్ణాంకం piix_pci_స్థానం_get_irq(నిర్మాణం pci_device *pci, పూర్ణాంకం pin)
{
    పూర్ణాంకం స్థానం_addend = 0;

    అయితే (pci->మాతృ != ఖాళీ) {
        స్థానం_addend += pci_bdf_గమ్యం_dev(pci->bdf);
        pci = pci->మాతృ;
    }
    స్థానం_addend += pci_bdf_గమ్యం_dev(pci->bdf) - 1;
    తిరిగి pci_irqs[(pin - 1 + స్థానం_addend) & 3];
}

స్థిర పూర్ణాంకం mch_pci_స్థానం_get_irq(నిర్మాణం pci_device *pci, పూర్ణాంకం pin)
{
    పూర్ణాంకం pin_addend = 0;
    అయితే (pci->మాతృ != ఖాళీ) {
        pin_addend += pci_bdf_గమ్యం_dev(pci->bdf);
        pci = pci->మాతృ;
    }
    u8 స్థానం = pci_bdf_గమ్యం_dev(pci->bdf);
    ఒకవేళ (స్థానం <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        తిరిగి pci_irqs[(pin - 1 + pin_addend + స్థానం) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    తిరిగి pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
స్థిర శూన్య piix_isa_bridge_అమరిక(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    పూర్ణాంకం i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    కొరకు (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_పోర్టు_ELCR1);
    outb(elcr[1], PIIX_పోర్టు_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}

స్థిర శూన్య mch_isa_lpc_అమరిక(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_పునాది | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

స్థిర పూర్ణాంకం ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
స్థిర శూన్య mch_isa_bridge_అమరిక(నిర్మాణం pci_device *dev, శూన్య *arg)
{
    u16 bdf = dev->bdf;
    పూర్ణాంకం i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    కొరకు (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));

        /* activate irq remapping in LPC */

        /* PIRQ[A-D] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT + i, irq);
        /* PIRQ[E-H] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT + i, irq);
    }
    outb(elcr[0], ICH9_LPC_పోర్టు_ELCR1);
    outb(elcr[1], ICH9_LPC_పోర్టు_ELCR2);
    dprintf(1, "Q35 LPC init: elcr=%02x %02x\n", elcr[0], elcr[1]);

    ICH9LpcBDF = bdf;

    mch_isa_lpc_అమరిక(bdf);

    e820_కూడు(ICH9_LPC_RCBA_ADDR, 16*1024, E820_RESERVED);

    acpi_pm1a_cnt = acpi_pm_పునాది + 0x04;
    pmtimer_అమరిక(acpi_pm_పునాది + 0x08);
}

స్థిర శూన్య storage_ide_అమరిక(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_సమితి_io_region_addr(pci, 0, పోర్టు_ATA1_CMD_పునాది, 0);
    pci_సమితి_io_region_addr(pci, 1, పోర్టు_ATA1_CTRL_పునాది, 0);
    pci_సమితి_io_region_addr(pci, 2, పోర్టు_ATA2_CMD_పునాది, 0);
    pci_సమితి_io_region_addr(pci, 3, పోర్టు_ATA2_CTRL_పునాది, 0);
}

/* PIIX3/PIIX4 IDE */
స్థిర శూన్య piix_ide_అమరిక(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

స్థిర శూన్య pic_ibm_అమరిక(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_సమితి_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

స్థిర శూన్య apple_macio_అమరిక(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    /* macio bridge */
    pci_సమితి_io_region_addr(pci, 0, 0x80800000, 0);
}

స్థిర శూన్య piix4_pm_config_అమరిక(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_అంతరాయం_పంక్తి, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_పునాది | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_పునాది + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

స్థిర పూర్ణాంకం PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
స్థిర శూన్య piix4_pm_అమరిక(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_అమరిక(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_పునాది + 0x04;
    pmtimer_అమరిక(acpi_pm_పునాది + 0x08);
}

స్థిర శూన్య ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_పునాది,
                      (acpi_pm_పునాది + 0x100) | PCI_పునాది_చిరునామా_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

స్థిర పూర్ణాంకం ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
స్థిర శూన్య ich9_smbus_అమరిక(నిర్మాణం pci_device *dev, శూన్య *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

స్థిర శూన్య intel_igd_అమరిక(నిర్మాణం pci_device *dev, శూన్య *arg)
{
    నిర్మాణం romfile_s *opregion = romfile_కనుగొను("etc/igd-opregion");
    u64 bdsm_కొలత = le64_గమ్యం_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    ఒకవేళ (opregion && opregion->కొలత) {
        శూన్య *addr = memalign_ఎగువ(పుట_కొలత, opregion->కొలత);
        ఒకవేళ (!addr) {
            warn_noalloc();
            తిరిగి;
        }

        ఒకవేళ (opregion->copy(opregion, addr, opregion->కొలత) < 0) {
            విడుదల(addr);
            తిరిగి;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_గమ్యం_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->కొలత >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    ఒకవేళ (bdsm_కొలత && (dev->bdf == pci_గమ్యం_bdf(0, 2, 0))) {
        శూన్య *addr = memalign_tmphigh(1024 * 1024, bdsm_కొలత);
        ఒకవేళ (!addr) {
            warn_noalloc();
            తిరిగి;
        }

        e820_కూడు((u32)addr, bdsm_కొలత, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_గమ్యం_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_కొలత >> 20, dev);
    }
}

స్థిర స్థిరరాశి నిర్మాణం pci_device_id pci_device_tbl[] = {
    /* PIIX3/PIIX4 PCI to ISA bridge */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0,
               piix_isa_bridge_అమరిక),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0,
               piix_isa_bridge_అమరిక),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC,
               mch_isa_bridge_అమరిక),

    /* STORAGE IDE */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1,
                     PCI_CLASS_STORAGE_IDE, piix_ide_అమరిక),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB,
                     PCI_CLASS_STORAGE_IDE, piix_ide_అమరిక),
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
                     storage_ide_అమరిక),

    /* PIC, IBM, MPIC & MPIC2 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0x0046, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_అమరిక),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0xFFFF, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_అమరిక),

    /* PIIX4 Power Management device (for ACPI) */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3,
               piix4_pm_అమరిక),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_SMBUS,
               ich9_smbus_అమరిక),

    /* 0xff00 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0017, 0xff00, apple_macio_అమరిక),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0022, 0xff00, apple_macio_అమరిక),

    /* Intel IGD OpRegion setup */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
                     intel_igd_అమరిక),

    PCI_DEVICE_చివర,
};

స్థిర పూర్ణాంకం MCHMmcfgBDF = -1;
స్థిర శూన్య mch_mmconfig_అమరిక(u16 bdf);

శూన్య pci_resume(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU) {
        తిరిగి;
    }

    ఒకవేళ (PiixPmBDF >= 0) {
        piix4_pm_config_అమరిక(PiixPmBDF);
    }

    ఒకవేళ (ICH9LpcBDF >= 0) {
        mch_isa_lpc_అమరిక(ICH9LpcBDF);
    }

    ఒకవేళ (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    ఒకవేళ(MCHMmcfgBDF >= 0) {
        mch_mmconfig_అమరిక(MCHMmcfgBDF);
    }
}

స్థిర శూన్య pci_bios_init_device(నిర్మాణం pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    పూర్ణాంకం pin = pci_config_readb(bdf, PCI_అంతరాయం_PIN);
    ఒకవేళ (pin != 0)
        pci_config_writeb(bdf, PCI_అంతరాయం_పంక్తి, pci_స్థానం_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, ఖాళీ);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_జ్ఞాపకం | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    ఒకవేళ (pci->శీర్షిక_type & PCI_శీర్షిక_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

స్థిర శూన్య pci_bios_init_devices(శూన్య)
{
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

స్థిర శూన్య pci_enable_default_vga(శూన్య)
{
    నిర్మాణం pci_device *pci;

    foreachpci(pci) {
        ఒకవేళ (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            తిరిగి;
        }
    }

    pci = pci_కనుగొను_class(PCI_CLASS_DISPLAY_VGA);
    ఒకవేళ (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        తిరిగి;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_జ్ఞాపకం);

    అయితే (pci->మాతృ) {
        pci = pci->మాతృ;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_జ్ఞాపకం);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

స్థిర శూన్య i440fx_mem_addr_అమరిక(నిర్మాణం pci_device *dev, శూన్య *arg)
{
    ఒకవేళ (Ram_కొలత <= 0x80000000)
        pcimem_ప్రారంభం = 0x80000000;
    లేకపోతే ఒకవేళ (Ram_కొలత <= 0xc0000000)
        pcimem_ప్రారంభం = 0xc0000000;

    pci_స్థానం_get_irq = piix_pci_స్థానం_get_irq;
}

స్థిర శూన్య mch_mmconfig_అమరిక(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 పై = addr >> 32;
    u32 క్రింది = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, పై);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, క్రింది);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

స్థిర శూన్య mch_mem_addr_అమరిక(నిర్మాణం pci_device *dev, శూన్య *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 కొలత = Q35_HOST_BRIDGE_PCIEXBAR_కొలత;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_అమరిక(dev->bdf);
    e820_కూడు(addr, కొలత, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_ప్రారంభం = addr + కొలత;

    pci_స్థానం_get_irq = mch_pci_స్థానం_get_irq;

    /* setup io address space */
    ఒకవేళ (acpi_pm_పునాది < 0x1000)
        pci_io_దిగువ_చివర = 0x10000;
    లేకపోతే
        pci_io_దిగువ_చివర = acpi_pm_పునాది;
}

స్థిర స్థిరరాశి నిర్మాణం pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_అమరిక),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_అమరిక),
    PCI_DEVICE_చివర
};

స్థిర శూన్య pci_bios_init_platform(శూన్య)
{
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, ఖాళీ);
    }
}

స్థిర u8 pci_కనుగొను_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    ఒకవేళ (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        తిరిగి 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    ఒకవేళ (device_id != PCI_DEVICE_ID_REDHAT_మూలం_పోర్టు &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        తిరిగి 0;
    }
    u8 cap = 0;

    చేయి {
        cap = pci_కనుగొను_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } అయితే (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_స్థానభ్రంశం) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    ఒకవేళ (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        ఒకవేళ (cap_len < RES_RESERVE_CAP_కొలత) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            తిరిగి 0;
        }
    } లేకపోతే {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    తిరిగి cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

స్థిర శూన్య
pci_bios_init_bus_rec(పూర్ణాంకం bus, u8 *pci_bus)
{
    పూర్ణాంకం bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ఒకవేళ (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ఒకవేళ (class != PCI_CLASS_BRIDGE_PCI) {
            కొనసాగు;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        ఒకవేళ (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } లేకపోతే {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        ఒకవేళ (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } లేకపోతే {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        ఒకవేళ (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_కనుగొను_resource_reserve_capability(bdf);

            ఒకవేళ (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                ఒకవేళ (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    ఒకవేళ ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    ఒకవేళ (secbus + res_bus > *pci_bus) {
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
        } లేకపోతే {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

స్థిర శూన్య
pci_bios_init_bus(శూన్య)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    ఒకవేళ (extraroots) {
        అయితే (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

స్థిర శూన్య
pci_bios_get_bar(నిర్మాణం pci_device *pci, పూర్ణాంకం bar,
                 పూర్ణాంకం *ptype, u64 *psize, పూర్ణాంకం *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    పూర్ణాంకం is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 ముసుగు;

    ఒకవేళ (bar == PCI_ROM_స్థానం) {
        ముసుగు = PCI_ROM_చిరునామా_ముసుగు;
        pci_config_writel(bdf, ofs, ముసుగు);
    } లేకపోతే {
        ఒకవేళ (old & PCI_పునాది_చిరునామా_SPACE_IO) {
            ముసుగు = PCI_పునాది_చిరునామా_IO_ముసుగు;
            type = PCI_REGION_TYPE_IO;
        } లేకపోతే {
            ముసుగు = PCI_పునాది_చిరునామా_MEM_ముసుగు;
            ఒకవేళ (old & PCI_పునాది_చిరునామా_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_పునాది_చిరునామా_MEM_TYPE_ముసుగు)
                    == PCI_పునాది_చిరునామా_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    ఒకవేళ (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 ఎగువ = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)ఎగువ << 32);
        ముసుగు |= ((u64)0xffffffff << 32);
        *psize = (~(val & ముసుగు)) + 1;
    } లేకపోతే {
        *psize = ((~(val & ముసుగు)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

స్థిర పూర్ణాంకం pci_bios_bridge_region_is64(నిర్మాణం pci_region *r,
                                 నిర్మాణం pci_device *pci, పూర్ణాంకం type)
{
    ఒకవేళ (type != PCI_REGION_TYPE_PREFMEM)
        తిరిగి 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_జ్ఞాపకం_పునాది);
    ఒకవేళ (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_జ్ఞాపకం_పునాది, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_జ్ఞాపకం_పునాది);
        pci_config_writel(pci->bdf, PCI_PREF_జ్ఞాపకం_పునాది, 0x0);
    }
    ఒకవేళ ((pmem & PCI_PREF_RANGE_TYPE_ముసుగు) != PCI_PREF_RANGE_TYPE_64)
       తిరిగి 0;
    నిర్మాణం pci_region_నమోదు *నమోదు_2;
    hlist_for_each_నమోదు(నమోదు_2, &r->list, కణుపు) {
        ఒకవేళ (!నమోదు_2->is64)
            తిరిగి 0;
    }
    తిరిగి 1;
}

స్థిర u64 pci_region_align(నిర్మాణం pci_region *r)
{
    నిర్మాణం pci_region_నమోదు *నమోదు_2;
    hlist_for_each_నమోదు(నమోదు_2, &r->list, కణుపు) {
        // The first entry in the sorted list has the largest alignment
        తిరిగి నమోదు_2->align;
    }
    తిరిగి 1;
}

స్థిర u64 pci_region_కూడిక(నిర్మాణం pci_region *r)
{
    u64 కూడిక = 0;
    నిర్మాణం pci_region_నమోదు *నమోదు_2;
    hlist_for_each_నమోదు(నమోదు_2, &r->list, కణుపు) {
        కూడిక += నమోదు_2->కొలత;
    }
    తిరిగి కూడిక;
}

స్థిర శూన్య pci_region_migrate_64bit_entries(నిర్మాణం pci_region *from,
                                             నిర్మాణం pci_region *గమ్యం)
{
    నిర్మాణం hlist_కణుపు *n, **చివరి = &గమ్యం->list.మొదటి;
    నిర్మాణం pci_region_నమోదు *నమోదు_2;
    hlist_for_each_నమోదు_సురక్షిత(నమోదు_2, n, &from->list, కణుపు) {
        ఒకవేళ (!నమోదు_2->is64)
            కొనసాగు;
        ఒకవేళ (నమోదు_2->dev->class == PCI_CLASS_SERIAL_USB)
            కొనసాగు;
        ఒకవేళ (నమోదు_2->dev->class == PCI_CLASS_STORAGE_NVME)
            కొనసాగు;
        // Move from source list to destination list.
        hlist_del(&నమోదు_2->కణుపు);
        hlist_కూడు(&నమోదు_2->కణుపు, చివరి);
        చివరి = &నమోదు_2->కణుపు.తదుపరి;
    }
}

స్థిర నిర్మాణం pci_region_నమోదు *
pci_region_create_నమోదు(నిర్మాణం pci_bus *bus, నిర్మాణం pci_device *dev,
                        పూర్ణాంకం bar, u64 కొలత, u64 align, పూర్ణాంకం type, పూర్ణాంకం is64)
{
    నిర్మాణం pci_region_నమోదు *నమోదు_2 = malloc_tmp(పరిమాణం(*నమోదు_2));
    ఒకవేళ (!నమోదు_2) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(నమోదు_2, 0, పరిమాణం(*నమోదు_2));
    నమోదు_2->dev = dev;
    నమోదు_2->bar = bar;
    నమోదు_2->కొలత = కొలత;
    నమోదు_2->align = align;
    నమోదు_2->is64 = is64;
    నమోదు_2->type = type;
    // Insert into list in sorted order.
    నిర్మాణం hlist_కణుపు **pprev;
    నిర్మాణం pci_region_నమోదు *pos;
    hlist_for_each_నమోదు_pprev(pos, pprev, &bus->r[type].list, కణుపు) {
        ఒకవేళ (pos->align < align || (pos->align == align && pos->కొలత < కొలత))
            ఆపు;
    }
    hlist_కూడు(&నమోదు_2->కణుపు, pprev);
    తిరిగి నమోదు_2;
}

రకనిర్వచనం గణన hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

స్థిర hotplug_type_t pci_bus_hotplug_support(నిర్మాణం pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    ఒకవేళ (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 పోర్టు_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        ఒకవేళ (పోర్టు_type == PCI_EXP_TYPE_PCI_BRIDGE)
            వెళ్ళు check_shpc;

        u8 downstream_పోర్టు = (పోర్టు_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (పోర్టు_type == PCI_EXP_TYPE_మూలం_పోర్టు);
        /*
         * PCI Express SPEC, 7.8.2:
         *   Slot Implemented – When Set, this bit indicates that the Link
         *   HwInit associated with this Port is connected to a slot (as
         *   compared to being connected to a system-integrated device or
         *   being disabled).
         *   This bit is valid for Downstream Ports. This bit is undefined
         *   for Upstream Ports.
         */
        u16 స్థానం_implemented = pcie_flags & PCI_EXP_FLAGS_స్థానం;

        తిరిగి downstream_పోర్టు && స్థానం_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_కనుగొను_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    తిరిగి !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
స్థిర పూర్ణాంకం pci_bridge_has_region(నిర్మాణం pci_device *pci,
                                 గణన pci_region_type region_type)
{
    u8 పునాది;

    ఎంపిక (region_type) {
        స్థితి PCI_REGION_TYPE_IO:
            పునాది = PCI_IO_పునాది;
            ఆపు;
        స్థితి PCI_REGION_TYPE_PREFMEM:
            పునాది = PCI_PREF_జ్ఞాపకం_పునాది;
            ఆపు;
        అప్రమేయ:
            /* Regular memory support is mandatory */
            తిరిగి 1;
    }

    pci_config_writeb(pci->bdf, పునాది, 0xFF);

    తిరిగి pci_config_readb(pci->bdf, పునాది) != 0;
}

స్థిర పూర్ణాంకం pci_bios_check_devices(నిర్మాణం pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        నిర్మాణం pci_bus *bus = &busses[pci_bdf_గమ్యం_bus(pci->bdf)];
        ఒకవేళ (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        పూర్ణాంకం i;
        కొరకు (i = 0; i < PCI_అంకె_REGIONS; i++) {
            ఒకవేళ ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_అంకె_REGIONS && i < PCI_ROM_స్థానం))
                కొనసాగు;
            పూర్ణాంకం type, is64;
            u64 కొలత;
            pci_bios_get_bar(pci, i, &type, &కొలత, &is64);
            ఒకవేళ (కొలత == 0)
                కొనసాగు;

            ఒకవేళ (type != PCI_REGION_TYPE_IO && కొలత < PCI_DEVICE_MEM_MIN)
                కొలత = PCI_DEVICE_MEM_MIN;
            నిర్మాణం pci_region_నమోదు *నమోదు_2 = pci_region_create_నమోదు(
                bus, pci, i, కొలత, కొలత, type, is64);
            ఒకవేళ (!నమోదు_2)
                తిరిగి -1;

            ఒకవేళ (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    పూర్ణాంకం secondary_bus;
    కొరకు (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        నిర్మాణం pci_bus *s = &busses[secondary_bus];
        ఒకవేళ (!s->bus_dev)
            కొనసాగు;
        నిర్మాణం pci_bus *మాతృ = &busses[pci_bdf_గమ్యం_bus(s->bus_dev->bdf)];
        ఒకవేళ (!మాతృ->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            మాతృ = &busses[0];
        పూర్ణాంకం type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_కనుగొను_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_కనుగొను_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        కొరకు (type = 0; type < PCI_REGION_TYPE_లెక్క; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            ఒకవేళ (!pci_bridge_has_region(s->bus_dev, type))
                కొనసాగు;
            u64 కొలత = 0;
            ఒకవేళ (qemu_cap) {
                u32 tmp_కొలత;
                u64 tmp_కొలత_64;
                ఎంపిక(type) {
                స్థితి PCI_REGION_TYPE_IO:
                    tmp_కొలత_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    ఒకవేళ (tmp_కొలత_64 != (u64)-1) {
                        కొలత = tmp_కొలత_64;
                    }
                    ఆపు;
                స్థితి PCI_REGION_TYPE_MEM:
                    tmp_కొలత = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    ఒకవేళ (tmp_కొలత != (u32)-1) {
                        కొలత = tmp_కొలత;
                    }
                    ఆపు;
                స్థితి PCI_REGION_TYPE_PREFMEM:
                    tmp_కొలత = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_కొలత_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    ఒకవేళ (tmp_కొలత != (u32)-1 && tmp_కొలత_64 == (u64)-1) {
                        కొలత = tmp_కొలత;
                    } లేకపోతే ఒకవేళ (tmp_కొలత == (u32)-1 && tmp_కొలత_64 != (u64)-1) {
                        కొలత = tmp_కొలత_64;
                    } లేకపోతే ఒకవేళ (tmp_కొలత != (u32)-1 && tmp_కొలత_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    ఆపు;
                అప్రమేయ:
                    ఆపు;
                }
            }
            ఒకవేళ (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 కూడిక = pci_region_కూడిక(&s->r[type]);
            పూర్ణాంకం is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            పూర్ణాంకం resource_optional = 0;
            ఒకవేళ (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 శిఖరం_align = pci_mem64_శిఖరం >> 11;
            ఒకవేళ (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (శిఖరం_align > align))
                align = శిఖరం_align;
            ఒకవేళ (align > కూడిక && hotplug_support && !resource_optional)
                కూడిక = align; /* reserve min size for hot-plug */
            ఒకవేళ (కొలత > కూడిక) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        కొలత, region_type_name[type]);
                ఒకవేళ (type != PCI_REGION_TYPE_IO) {
                    కొలత = ALIGN(కొలత, align);
                }
            } లేకపోతే {
                కొలత = ALIGN(కూడిక, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            నిర్మాణం pci_region_నమోదు *నమోదు_2 = pci_region_create_నమోదు(
                మాతృ, s->bus_dev, -1, కొలత, align, type, is64);
            ఒకవేళ (!నమోదు_2)
                తిరిగి -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      నమోదు_2->dev->secondary_bus, కొలత,
                      region_type_name[నమోదు_2->type]);
        }
    }
    తిరిగి 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
స్థిర పూర్ణాంకం pci_bios_init_మూలం_regions_io(నిర్మాణం pci_bus *bus)
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
    నిర్మాణం pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 కూడిక = pci_region_కూడిక(r_io);
    ఒకవేళ (కూడిక < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->పునాది = 0xc000;
    } లేకపోతే ఒకవేళ (కూడిక < pci_io_దిగువ_చివర - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->పునాది = 0x1000;
    } లేకపోతే {
        /* not enouth io address space -> error out */
        తిరిగి -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->పునాది, r_io->పునాది + కూడిక - 1);
    తిరిగి 0;
}

స్థిర పూర్ణాంకం pci_bios_init_మూలం_regions_mem(నిర్మాణం pci_bus *bus)
{
    నిర్మాణం pci_region *r_చివర = &bus->r[PCI_REGION_TYPE_PREFMEM];
    నిర్మాణం pci_region *r_ప్రారంభం = &bus->r[PCI_REGION_TYPE_MEM];

    ఒకవేళ (pci_region_align(r_ప్రారంభం) < pci_region_align(r_చివర)) {
        // Swap regions to improve alignment.
        r_చివర = r_ప్రారంభం;
        r_ప్రారంభం = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 కూడిక = pci_region_కూడిక(r_చివర);
    u64 align = pci_region_align(r_చివర);
    r_చివర->పునాది = ALIGN_కిందికి((pcimem_చివర - కూడిక), align);
    కూడిక = pci_region_కూడిక(r_ప్రారంభం);
    align = pci_region_align(r_ప్రారంభం);
    r_ప్రారంభం->పునాది = ALIGN_కిందికి((r_చివర->పునాది - కూడిక), align);

    ఒకవేళ ((r_ప్రారంభం->పునాది < pcimem_ప్రారంభం) ||
         (r_ప్రారంభం->పునాది > pcimem_చివర))
        // Memory range requested is larger than available.
        తిరిగి -1;
    తిరిగి 0;
}

#నిర్వచించు PCI_IO_జరుపు            8
#నిర్వచించు PCI_జ్ఞాపకం_జరుపు        16
#నిర్వచించు PCI_PREF_జ్ఞాపకం_జరుపు   16

స్థిర శూన్య
pci_region_map_one_నమోదు(నిర్మాణం pci_region_నమోదు *నమోదు_2, u64 addr)
{
    ఒకవేళ (నమోదు_2->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                నమోదు_2->dev,
                నమోదు_2->bar, addr, నమోదు_2->కొలత, region_type_name[నమోదు_2->type]);

        pci_సమితి_io_region_addr(నమోదు_2->dev, నమోదు_2->bar, addr, నమోదు_2->is64);
        తిరిగి;
    }

    u16 bdf = నమోదు_2->dev->bdf;
    u64 హద్దు = addr + నమోదు_2->కొలత - 1;
    ఒకవేళ (నమోదు_2->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_పునాది, addr >> PCI_IO_జరుపు);
        pci_config_writew(bdf, PCI_IO_పునాది_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_హద్దు, హద్దు >> PCI_IO_జరుపు);
        pci_config_writew(bdf, PCI_IO_హద్దు_UPPER16, 0);
    }
    ఒకవేళ (నమోదు_2->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_జ్ఞాపకం_పునాది, addr >> PCI_జ్ఞాపకం_జరుపు);
        pci_config_writew(bdf, PCI_జ్ఞాపకం_హద్దు, హద్దు >> PCI_జ్ఞాపకం_జరుపు);
    }
    ఒకవేళ (నమోదు_2->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_జ్ఞాపకం_పునాది, addr >> PCI_PREF_జ్ఞాపకం_జరుపు);
        pci_config_writew(bdf, PCI_PREF_జ్ఞాపకం_హద్దు, హద్దు >> PCI_PREF_జ్ఞాపకం_జరుపు);
        pci_config_writel(bdf, PCI_PREF_పునాది_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_హద్దు_UPPER32, హద్దు >> 32);
    }
}

స్థిర శూన్య pci_region_map_entries(నిర్మాణం pci_bus *busses, నిర్మాణం pci_region *r)
{
    నిర్మాణం hlist_కణుపు *n;
    నిర్మాణం pci_region_నమోదు *నమోదు_2;
    hlist_for_each_నమోదు_సురక్షిత(నమోదు_2, n, &r->list, కణుపు) {
        u64 addr = r->పునాది;
        r->పునాది += నమోదు_2->కొలత;
        ఒకవేళ (నమోదు_2->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[నమోదు_2->dev->secondary_bus].r[నమోదు_2->type].పునాది = addr;
        pci_region_map_one_నమోదు(నమోదు_2, addr);
        hlist_del(&నమోదు_2->కణుపు);
        విడుదల(నమోదు_2);
    }
}

స్థిర శూన్య pci_bios_map_devices(నిర్మాణం pci_bus *busses)
{
    ఒకవేళ (pci_bios_init_మూలం_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_ప్రారంభం, pcimem_చివర);
    ఒకవేళ (pci_pad_mem64 || pci_bios_init_మూలం_regions_mem(busses)) {
        నిర్మాణం pci_region r64_mem, r64_pref;
        r64_mem.list.మొదటి = ఖాళీ;
        r64_pref.list.మొదటి = ఖాళీ;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        ఒకవేళ (pci_bios_init_మూలం_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 కూడిక_mem = pci_region_కూడిక(&r64_mem);
        u64 కూడిక_pref = pci_region_కూడిక(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.పునాది = le64_గమ్యం_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        ఒకవేళ (r64_mem.పునాది < 0x100000000LL + Ram_కొలత_Over4_G)
            r64_mem.పునాది = 0x100000000LL + Ram_కొలత_Over4_G;
        ఒకవేళ (pci_mem64_శిఖరం) {
            u64 కొలత = (ALIGN(కూడిక_mem, (1LL<<30)) +
                        ALIGN(కూడిక_pref, (1LL<<30)));
            ఒకవేళ (pci_pad_mem64)
                కొలత = ALIGN(కొలత, pci_mem64_శిఖరం >> 3);
            ఒకవేళ (r64_mem.పునాది < pci_mem64_శిఖరం - కొలత) {
                r64_mem.పునాది = pci_mem64_శిఖరం - కొలత;
            }
            ఒకవేళ (e820_is_వాడిన(r64_mem.పునాది, కొలత))
                r64_mem.పునాది -= కొలత;
        }
        r64_mem.పునాది = ALIGN(r64_mem.పునాది, align_mem);
        r64_mem.పునాది = ALIGN(r64_mem.పునాది, (1LL<<30));    // 1G hugepage
        r64_pref.పునాది = r64_mem.పునాది + కూడిక_mem;
        r64_pref.పునాది = ALIGN(r64_pref.పునాది, align_pref);
        r64_pref.పునాది = ALIGN(r64_pref.పునాది, (1LL<<30));  // 1G hugepage
        pcimem64_ప్రారంభం = r64_mem.పునాది;
        pcimem64_చివర = r64_pref.పునాది + కూడిక_pref;
        pcimem64_చివర = ALIGN(pcimem64_చివర, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_ప్రారంభం, pcimem64_చివర);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } లేకపోతే {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_ప్రారంభం = 0;
    }
    // Map regions on each device.
    పూర్ణాంకం bus;
    కొరకు (bus = 0; bus<=MaxPCIBus; bus++) {
        పూర్ణాంకం type;
        కొరకు (type = 0; type < PCI_REGION_TYPE_లెక్క; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

శూన్య
pci_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    dprintf(3, "pci setup\n");

    ఒకవేళ (CPUPhys_బిట్లు) {
        pci_mem64_శిఖరం = 1LL << CPUPhys_బిట్లు;
        ఒకవేళ (CPUPhys_బిట్లు > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            ఒకవేళ (Ram_కొలత_Over4_G < (1LL << 40)) {
                pci_mem64_శిఖరం = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    ఒకవేళ (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_కొలత_Over4_G ? 1 : 0;

    ఒకవేళ (!CPULong_బహుళకం)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    ఒకవేళ (pci_probe_host() != 0) {
        తిరిగి;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_ప్రారంభం = Ram_కొలత;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    నిర్మాణం pci_bus *busses = malloc_tmp(పరిమాణం(*busses) * (MaxPCIBus + 1));
    ఒకవేళ (!busses) {
        warn_noalloc();
        తిరిగి;
    }
    memset(busses, 0, పరిమాణం(*busses) * (MaxPCIBus + 1));
    ఒకవేళ (pci_bios_check_devices(busses))
        తిరిగి;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    విడుదల(busses);

    pci_enable_default_vga();
}
