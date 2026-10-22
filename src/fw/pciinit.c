// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // le64_to_cpu
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#ਅਨ੍ਤਰ੍ਭਾਵ "dev-piix.h" // PIIX_*
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // e820_add
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ata.h" // PORT_ATA1_CMD_BASE
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_readl
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // pci_probe_devices
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_COMMAND
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h" // struct hlist_node
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // RamSize
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // pci_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outb

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ROM_ਥਾਂ 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਅੰਕ_REGIONS 7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_BRIDGE_ਅੰਕ_REGIONS 2

ਗਿਣਤੀ pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_ਗਿਣਤੀ,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_ਸ਼ੁਰੂ   = BUILD_PCIMEM_ਸ਼ੁਰੂ;
u64 pcimem_ਸਿਰਾ     = BUILD_PCIMEM_ਸਿਰਾ;
u64 pcimem64_ਸ਼ੁਰੂ = BUILD_PCIMEM64_ਸ਼ੁਰੂ;
u64 pcimem64_ਸਿਰਾ   = BUILD_PCIMEM64_ਸਿਰਾ;

// Resource allocation limits
ਸਥਿਰ u64 pci_io_ਨੀਵਾਂ_ਸਿਰਾ = 0xa000;
ਸਥਿਰ u64 pci_mem64_ਸਿਖਰ  = 0;
ਸਥਿਰ u32 pci_pad_mem64  = -1;

ਬਣਤਰ pci_region_ਇੰਦਰਾਜ {
    ਬਣਤਰ pci_device *dev;
    ਪੂਰਨ bar;
    u64 ਮਾਪ;
    u64 align;
    ਪੂਰਨ is64;
    ਗਿਣਤੀ pci_region_type type;
    ਬਣਤਰ hlist_ਗੰਢ ਗੰਢ;
};

ਬਣਤਰ pci_region {
    /* pci region assignments */
    u64 ਅਧਾਰ;
    ਬਣਤਰ hlist_ਸਿਰ list;
};

ਬਣਤਰ pci_bus {
    ਬਣਤਰ pci_region r[PCI_REGION_TYPE_ਗਿਣਤੀ];
    ਬਣਤਰ pci_device *bus_dev;
};

ਸਥਿਰ u32 pci_bar(ਬਣਤਰ pci_device *pci, ਪੂਰਨ region_ਅੰਕ)
{
    ਜੇ (region_ਅੰਕ != PCI_ROM_ਥਾਂ) {
        ਵਾਪਸ PCI_ਅਧਾਰ_ਪਤਾ_0 + region_ਅੰਕ * 4;
    }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਸਿਰਲੇਖ_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->ਸਿਰਲੇਖ_type & ~PCI_ਸਿਰਲੇਖ_TYPE_MULTI_FUNCTION;
    ਵਾਪਸ type == PCI_ਸਿਰਲੇਖ_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ਪਤਾ;
}

ਸਥਿਰ ਖਾਲੀ
pci_ਸੈੱਟ_io_region_addr(ਬਣਤਰ pci_device *pci, ਪੂਰਨ bar, u64 addr, ਪੂਰਨ is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    ਜੇ (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
ਅਚਲ u8 pci_irqs[4] = {
    10, 10, 11, 11
};

ਸਥਿਰ ਪੂਰਨ dummy_pci_ਥਾਂ_get_irq(ਬਣਤਰ pci_device *pci, ਪੂਰਨ pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    ਵਾਪਸ 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

ਸਥਿਰ ਪੂਰਨ (*pci_ਥਾਂ_get_irq)(ਬਣਤਰ pci_device *pci, ਪੂਰਨ pin) =
    dummy_pci_ਥਾਂ_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
ਸਥਿਰ ਪੂਰਨ piix_pci_ਥਾਂ_get_irq(ਬਣਤਰ pci_device *pci, ਪੂਰਨ pin)
{
    ਪੂਰਨ ਥਾਂ_addend = 0;

    ਜਦੋਂ (pci->ਮਾਪਾ != ਨਿਰਾ) {
        ਥਾਂ_addend += pci_bdf_ਮੰਜ਼ਿਲ_dev(pci->bdf);
        pci = pci->ਮਾਪਾ;
    }
    ਥਾਂ_addend += pci_bdf_ਮੰਜ਼ਿਲ_dev(pci->bdf) - 1;
    ਵਾਪਸ pci_irqs[(pin - 1 + ਥਾਂ_addend) & 3];
}

ਸਥਿਰ ਪੂਰਨ mch_pci_ਥਾਂ_get_irq(ਬਣਤਰ pci_device *pci, ਪੂਰਨ pin)
{
    ਪੂਰਨ pin_addend = 0;
    ਜਦੋਂ (pci->ਮਾਪਾ != ਨਿਰਾ) {
        pin_addend += pci_bdf_ਮੰਜ਼ਿਲ_dev(pci->bdf);
        pci = pci->ਮਾਪਾ;
    }
    u8 ਥਾਂ = pci_bdf_ਮੰਜ਼ਿਲ_dev(pci->bdf);
    ਜੇ (ਥਾਂ <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        ਵਾਪਸ pci_irqs[(pin - 1 + pin_addend + ਥਾਂ) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    ਵਾਪਸ pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
ਸਥਿਰ ਖਾਲੀ piix_isa_bridge_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    ਪੂਰਨ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    ਲਈ (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_ਪੋਰਟ_ELCR1);
    outb(elcr[1], PIIX_ਪੋਰਟ_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}

ਸਥਿਰ ਖਾਲੀ mch_isa_lpc_ਤਿਆਰੀ(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_ਅਧਾਰ | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

ਸਥਿਰ ਪੂਰਨ ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ਸਥਿਰ ਖਾਲੀ mch_isa_bridge_ਤਿਆਰੀ(ਬਣਤਰ pci_device *dev, ਖਾਲੀ *arg)
{
    u16 bdf = dev->bdf;
    ਪੂਰਨ i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    ਲਈ (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));

        /* activate irq remapping in LPC */

        /* PIRQ[A-D] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT + i, irq);
        /* PIRQ[E-H] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT + i, irq);
    }
    outb(elcr[0], ICH9_LPC_ਪੋਰਟ_ELCR1);
    outb(elcr[1], ICH9_LPC_ਪੋਰਟ_ELCR2);
    dprintf(1, "Q35 LPC init: elcr=%02x %02x\n", elcr[0], elcr[1]);

    ICH9LpcBDF = bdf;

    mch_isa_lpc_ਤਿਆਰੀ(bdf);

    e820_ਜੋੜੋ(ICH9_LPC_RCBA_ADDR, 16*1024, E820_RESERVED);

    acpi_pm1a_cnt = acpi_pm_ਅਧਾਰ + 0x04;
    pmtimer_ਤਿਆਰੀ(acpi_pm_ਅਧਾਰ + 0x08);
}

ਸਥਿਰ ਖਾਲੀ storage_ide_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_ਸੈੱਟ_io_region_addr(pci, 0, ਪੋਰਟ_ATA1_CMD_ਅਧਾਰ, 0);
    pci_ਸੈੱਟ_io_region_addr(pci, 1, ਪੋਰਟ_ATA1_CTRL_ਅਧਾਰ, 0);
    pci_ਸੈੱਟ_io_region_addr(pci, 2, ਪੋਰਟ_ATA2_CMD_ਅਧਾਰ, 0);
    pci_ਸੈੱਟ_io_region_addr(pci, 3, ਪੋਰਟ_ATA2_CTRL_ਅਧਾਰ, 0);
}

/* PIIX3/PIIX4 IDE */
ਸਥਿਰ ਖਾਲੀ piix_ide_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

ਸਥਿਰ ਖਾਲੀ pic_ibm_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_ਸੈੱਟ_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

ਸਥਿਰ ਖਾਲੀ apple_macio_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    /* macio bridge */
    pci_ਸੈੱਟ_io_region_addr(pci, 0, 0x80800000, 0);
}

ਸਥਿਰ ਖਾਲੀ piix4_pm_config_ਤਿਆਰੀ(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_ਰੁਕਾਵਟ_ਸਤਰ, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_ਅਧਾਰ | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_ਅਧਾਰ + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

ਸਥਿਰ ਪੂਰਨ PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
ਸਥਿਰ ਖਾਲੀ piix4_pm_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_ਤਿਆਰੀ(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_ਅਧਾਰ + 0x04;
    pmtimer_ਤਿਆਰੀ(acpi_pm_ਅਧਾਰ + 0x08);
}

ਸਥਿਰ ਖਾਲੀ ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_ਅਧਾਰ,
                      (acpi_pm_ਅਧਾਰ + 0x100) | PCI_ਅਧਾਰ_ਪਤਾ_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

ਸਥਿਰ ਪੂਰਨ ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
ਸਥਿਰ ਖਾਲੀ ich9_smbus_ਤਿਆਰੀ(ਬਣਤਰ pci_device *dev, ਖਾਲੀ *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

ਸਥਿਰ ਖਾਲੀ intel_igd_ਤਿਆਰੀ(ਬਣਤਰ pci_device *dev, ਖਾਲੀ *arg)
{
    ਬਣਤਰ romfile_s *opregion = romfile_ਲੱਭੋ("etc/igd-opregion");
    u64 bdsm_ਮਾਪ = le64_ਮੰਜ਼ਿਲ_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    ਜੇ (opregion && opregion->ਮਾਪ) {
        ਖਾਲੀ *addr = memalign_ਉੱਚਾ(ਪੰਨਾ_ਮਾਪ, opregion->ਮਾਪ);
        ਜੇ (!addr) {
            warn_noalloc();
            ਵਾਪਸ;
        }

        ਜੇ (opregion->copy(opregion, addr, opregion->ਮਾਪ) < 0) {
            ਮੁਕਤ(addr);
            ਵਾਪਸ;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_ਮੰਜ਼ਿਲ_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->ਮਾਪ >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    ਜੇ (bdsm_ਮਾਪ && (dev->bdf == pci_ਮੰਜ਼ਿਲ_bdf(0, 2, 0))) {
        ਖਾਲੀ *addr = memalign_tmphigh(1024 * 1024, bdsm_ਮਾਪ);
        ਜੇ (!addr) {
            warn_noalloc();
            ਵਾਪਸ;
        }

        e820_ਜੋੜੋ((u32)addr, bdsm_ਮਾਪ, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_ਮੰਜ਼ਿਲ_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_ਮਾਪ >> 20, dev);
    }
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ pci_device_id pci_device_tbl[] = {
    /* PIIX3/PIIX4 PCI to ISA bridge */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0,
               piix_isa_bridge_ਤਿਆਰੀ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0,
               piix_isa_bridge_ਤਿਆਰੀ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC,
               mch_isa_bridge_ਤਿਆਰੀ),

    /* STORAGE IDE */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1,
                     PCI_CLASS_STORAGE_IDE, piix_ide_ਤਿਆਰੀ),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB,
                     PCI_CLASS_STORAGE_IDE, piix_ide_ਤਿਆਰੀ),
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
                     storage_ide_ਤਿਆਰੀ),

    /* PIC, IBM, MPIC & MPIC2 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0x0046, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_ਤਿਆਰੀ),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0xFFFF, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_ਤਿਆਰੀ),

    /* PIIX4 Power Management device (for ACPI) */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3,
               piix4_pm_ਤਿਆਰੀ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_SMBUS,
               ich9_smbus_ਤਿਆਰੀ),

    /* 0xff00 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0017, 0xff00, apple_macio_ਤਿਆਰੀ),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0022, 0xff00, apple_macio_ਤਿਆਰੀ),

    /* Intel IGD OpRegion setup */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
                     intel_igd_ਤਿਆਰੀ),

    PCI_DEVICE_ਸਿਰਾ,
};

ਸਥਿਰ ਪੂਰਨ MCHMmcfgBDF = -1;
ਸਥਿਰ ਖਾਲੀ mch_mmconfig_ਤਿਆਰੀ(u16 bdf);

ਖਾਲੀ pci_resume(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU) {
        ਵਾਪਸ;
    }

    ਜੇ (PiixPmBDF >= 0) {
        piix4_pm_config_ਤਿਆਰੀ(PiixPmBDF);
    }

    ਜੇ (ICH9LpcBDF >= 0) {
        mch_isa_lpc_ਤਿਆਰੀ(ICH9LpcBDF);
    }

    ਜੇ (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    ਜੇ(MCHMmcfgBDF >= 0) {
        mch_mmconfig_ਤਿਆਰੀ(MCHMmcfgBDF);
    }
}

ਸਥਿਰ ਖਾਲੀ pci_bios_init_device(ਬਣਤਰ pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    ਪੂਰਨ pin = pci_config_readb(bdf, PCI_ਰੁਕਾਵਟ_PIN);
    ਜੇ (pin != 0)
        pci_config_writeb(bdf, PCI_ਰੁਕਾਵਟ_ਸਤਰ, pci_ਥਾਂ_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, ਨਿਰਾ);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_ਸਮ੍ਰਿਤੀ | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    ਜੇ (pci->ਸਿਰਲੇਖ_type & PCI_ਸਿਰਲੇਖ_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

ਸਥਿਰ ਖਾਲੀ pci_bios_init_devices(ਖਾਲੀ)
{
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

ਸਥਿਰ ਖਾਲੀ pci_enable_default_vga(ਖਾਲੀ)
{
    ਬਣਤਰ pci_device *pci;

    foreachpci(pci) {
        ਜੇ (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            ਵਾਪਸ;
        }
    }

    pci = pci_ਲੱਭੋ_class(PCI_CLASS_DISPLAY_VGA);
    ਜੇ (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        ਵਾਪਸ;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_ਸਮ੍ਰਿਤੀ);

    ਜਦੋਂ (pci->ਮਾਪਾ) {
        pci = pci->ਮਾਪਾ;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_ਸਮ੍ਰਿਤੀ);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ i440fx_mem_addr_ਤਿਆਰੀ(ਬਣਤਰ pci_device *dev, ਖਾਲੀ *arg)
{
    ਜੇ (Ram_ਮਾਪ <= 0x80000000)
        pcimem_ਸ਼ੁਰੂ = 0x80000000;
    ਨਹੀਂਤਾਂ ਜੇ (Ram_ਮਾਪ <= 0xc0000000)
        pcimem_ਸ਼ੁਰੂ = 0xc0000000;

    pci_ਥਾਂ_get_irq = piix_pci_ਥਾਂ_get_irq;
}

ਸਥਿਰ ਖਾਲੀ mch_mmconfig_ਤਿਆਰੀ(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 ਉੱਤਲਾ = addr >> 32;
    u32 ਹੇਠਲਾ = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, ਉੱਤਲਾ);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, ਹੇਠਲਾ);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

ਸਥਿਰ ਖਾਲੀ mch_mem_addr_ਤਿਆਰੀ(ਬਣਤਰ pci_device *dev, ਖਾਲੀ *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 ਮਾਪ = Q35_HOST_BRIDGE_PCIEXBAR_ਮਾਪ;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_ਤਿਆਰੀ(dev->bdf);
    e820_ਜੋੜੋ(addr, ਮਾਪ, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_ਸ਼ੁਰੂ = addr + ਮਾਪ;

    pci_ਥਾਂ_get_irq = mch_pci_ਥਾਂ_get_irq;

    /* setup io address space */
    ਜੇ (acpi_pm_ਅਧਾਰ < 0x1000)
        pci_io_ਨੀਵਾਂ_ਸਿਰਾ = 0x10000;
    ਨਹੀਂਤਾਂ
        pci_io_ਨੀਵਾਂ_ਸਿਰਾ = acpi_pm_ਅਧਾਰ;
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_ਤਿਆਰੀ),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_ਤਿਆਰੀ),
    PCI_DEVICE_ਸਿਰਾ
};

ਸਥਿਰ ਖਾਲੀ pci_bios_init_platform(ਖਾਲੀ)
{
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, ਨਿਰਾ);
    }
}

ਸਥਿਰ u8 pci_ਲੱਭੋ_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    ਜੇ (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        ਵਾਪਸ 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    ਜੇ (device_id != PCI_DEVICE_ID_REDHAT_ਜੜ੍ਹ_ਪੋਰਟ &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        ਵਾਪਸ 0;
    }
    u8 cap = 0;

    ਕਰੋ {
        cap = pci_ਲੱਭੋ_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } ਜਦੋਂ (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_ਵਿੱਥ) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    ਜੇ (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        ਜੇ (cap_len < RES_RESERVE_CAP_ਮਾਪ) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            ਵਾਪਸ 0;
        }
    } ਨਹੀਂਤਾਂ {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    ਵਾਪਸ cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ
pci_bios_init_bus_rec(ਪੂਰਨ bus, u8 *pci_bus)
{
    ਪੂਰਨ bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ਜੇ (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ਜੇ (class != PCI_CLASS_BRIDGE_PCI) {
            ਜਾਰੀ;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        ਜੇ (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } ਨਹੀਂਤਾਂ {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        ਜੇ (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } ਨਹੀਂਤਾਂ {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        ਜੇ (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_ਲੱਭੋ_resource_reserve_capability(bdf);

            ਜੇ (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                ਜੇ (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    ਜੇ ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    ਜੇ (secbus + res_bus > *pci_bus) {
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
        } ਨਹੀਂਤਾਂ {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

ਸਥਿਰ ਖਾਲੀ
pci_bios_init_bus(ਖਾਲੀ)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    ਜੇ (extraroots) {
        ਜਦੋਂ (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ
pci_bios_get_bar(ਬਣਤਰ pci_device *pci, ਪੂਰਨ bar,
                 ਪੂਰਨ *ptype, u64 *psize, ਪੂਰਨ *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    ਪੂਰਨ is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 ਨਕਾਬ;

    ਜੇ (bar == PCI_ROM_ਥਾਂ) {
        ਨਕਾਬ = PCI_ROM_ਪਤਾ_ਨਕਾਬ;
        pci_config_writel(bdf, ofs, ਨਕਾਬ);
    } ਨਹੀਂਤਾਂ {
        ਜੇ (old & PCI_ਅਧਾਰ_ਪਤਾ_SPACE_IO) {
            ਨਕਾਬ = PCI_ਅਧਾਰ_ਪਤਾ_IO_ਨਕਾਬ;
            type = PCI_REGION_TYPE_IO;
        } ਨਹੀਂਤਾਂ {
            ਨਕਾਬ = PCI_ਅਧਾਰ_ਪਤਾ_MEM_ਨਕਾਬ;
            ਜੇ (old & PCI_ਅਧਾਰ_ਪਤਾ_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_ਨਕਾਬ)
                    == PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    ਜੇ (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 ਉੱਚਾ = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)ਉੱਚਾ << 32);
        ਨਕਾਬ |= ((u64)0xffffffff << 32);
        *psize = (~(val & ਨਕਾਬ)) + 1;
    } ਨਹੀਂਤਾਂ {
        *psize = ((~(val & ਨਕਾਬ)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

ਸਥਿਰ ਪੂਰਨ pci_bios_bridge_region_is64(ਬਣਤਰ pci_region *r,
                                 ਬਣਤਰ pci_device *pci, ਪੂਰਨ type)
{
    ਜੇ (type != PCI_REGION_TYPE_PREFMEM)
        ਵਾਪਸ 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ);
    ਜੇ (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ);
        pci_config_writel(pci->bdf, PCI_PREF_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ, 0x0);
    }
    ਜੇ ((pmem & PCI_PREF_RANGE_TYPE_ਨਕਾਬ) != PCI_PREF_RANGE_TYPE_64)
       ਵਾਪਸ 0;
    ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
    hlist_for_each_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, &r->list, ਗੰਢ) {
        ਜੇ (!ਇੰਦਰਾਜ->is64)
            ਵਾਪਸ 0;
    }
    ਵਾਪਸ 1;
}

ਸਥਿਰ u64 pci_region_align(ਬਣਤਰ pci_region *r)
{
    ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
    hlist_for_each_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, &r->list, ਗੰਢ) {
        // The first entry in the sorted list has the largest alignment
        ਵਾਪਸ ਇੰਦਰਾਜ->align;
    }
    ਵਾਪਸ 1;
}

ਸਥਿਰ u64 pci_region_ਜੋੜ(ਬਣਤਰ pci_region *r)
{
    u64 ਜੋੜ = 0;
    ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
    hlist_for_each_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, &r->list, ਗੰਢ) {
        ਜੋੜ += ਇੰਦਰਾਜ->ਮਾਪ;
    }
    ਵਾਪਸ ਜੋੜ;
}

ਸਥਿਰ ਖਾਲੀ pci_region_migrate_64bit_entries(ਬਣਤਰ pci_region *from,
                                             ਬਣਤਰ pci_region *ਮੰਜ਼ਿਲ)
{
    ਬਣਤਰ hlist_ਗੰਢ *n, **ਆਖਰੀ = &ਮੰਜ਼ਿਲ->list.ਪਹਿਲਾ;
    ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
    hlist_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(ਇੰਦਰਾਜ, n, &from->list, ਗੰਢ) {
        ਜੇ (!ਇੰਦਰਾਜ->is64)
            ਜਾਰੀ;
        ਜੇ (ਇੰਦਰਾਜ->dev->class == PCI_CLASS_SERIAL_USB)
            ਜਾਰੀ;
        ਜੇ (ਇੰਦਰਾਜ->dev->class == PCI_CLASS_STORAGE_NVME)
            ਜਾਰੀ;
        // Move from source list to destination list.
        hlist_del(&ਇੰਦਰਾਜ->ਗੰਢ);
        hlist_ਜੋੜੋ(&ਇੰਦਰਾਜ->ਗੰਢ, ਆਖਰੀ);
        ਆਖਰੀ = &ਇੰਦਰਾਜ->ਗੰਢ.ਅਗਲਾ;
    }
}

ਸਥਿਰ ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *
pci_region_create_ਇੰਦਰਾਜ(ਬਣਤਰ pci_bus *bus, ਬਣਤਰ pci_device *dev,
                        ਪੂਰਨ bar, u64 ਮਾਪ, u64 align, ਪੂਰਨ type, ਪੂਰਨ is64)
{
    ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = malloc_tmp(ਆਕਾਰ(*ਇੰਦਰਾਜ));
    ਜੇ (!ਇੰਦਰਾਜ) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(ਇੰਦਰਾਜ, 0, ਆਕਾਰ(*ਇੰਦਰਾਜ));
    ਇੰਦਰਾਜ->dev = dev;
    ਇੰਦਰਾਜ->bar = bar;
    ਇੰਦਰਾਜ->ਮਾਪ = ਮਾਪ;
    ਇੰਦਰਾਜ->align = align;
    ਇੰਦਰਾਜ->is64 = is64;
    ਇੰਦਰਾਜ->type = type;
    // Insert into list in sorted order.
    ਬਣਤਰ hlist_ਗੰਢ **pprev;
    ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *pos;
    hlist_for_each_ਇੰਦਰਾਜ_pprev(pos, pprev, &bus->r[type].list, ਗੰਢ) {
        ਜੇ (pos->align < align || (pos->align == align && pos->ਮਾਪ < ਮਾਪ))
            ਰੋਕੋ;
    }
    hlist_ਜੋੜੋ(&ਇੰਦਰਾਜ->ਗੰਢ, pprev);
    ਵਾਪਸ ਇੰਦਰਾਜ;
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

ਸਥਿਰ hotplug_type_t pci_bus_hotplug_support(ਬਣਤਰ pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    ਜੇ (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 ਪੋਰਟ_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        ਜੇ (ਪੋਰਟ_type == PCI_EXP_TYPE_PCI_BRIDGE)
            ਜਾਓ check_shpc;

        u8 downstream_ਪੋਰਟ = (ਪੋਰਟ_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (ਪੋਰਟ_type == PCI_EXP_TYPE_ਜੜ੍ਹ_ਪੋਰਟ);
        /*
         * PCI Express SPEC, 7.8.2:
         *   Slot Implemented – When Set, this bit indicates that the Link
         *   HwInit associated with this Port is connected to a slot (as
         *   compared to being connected to a system-integrated device or
         *   being disabled).
         *   This bit is valid for Downstream Ports. This bit is undefined
         *   for Upstream Ports.
         */
        u16 ਥਾਂ_implemented = pcie_flags & PCI_EXP_FLAGS_ਥਾਂ;

        ਵਾਪਸ downstream_ਪੋਰਟ && ਥਾਂ_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_ਲੱਭੋ_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    ਵਾਪਸ !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
ਸਥਿਰ ਪੂਰਨ pci_bridge_has_region(ਬਣਤਰ pci_device *pci,
                                 ਗਿਣਤੀ pci_region_type region_type)
{
    u8 ਅਧਾਰ;

    ਚੋਣ (region_type) {
        ਹਾਲਤ PCI_REGION_TYPE_IO:
            ਅਧਾਰ = PCI_IO_ਅਧਾਰ;
            ਰੋਕੋ;
        ਹਾਲਤ PCI_REGION_TYPE_PREFMEM:
            ਅਧਾਰ = PCI_PREF_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ;
            ਰੋਕੋ;
        ਮੂਲ:
            /* Regular memory support is mandatory */
            ਵਾਪਸ 1;
    }

    pci_config_writeb(pci->bdf, ਅਧਾਰ, 0xFF);

    ਵਾਪਸ pci_config_readb(pci->bdf, ਅਧਾਰ) != 0;
}

ਸਥਿਰ ਪੂਰਨ pci_bios_check_devices(ਬਣਤਰ pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        ਬਣਤਰ pci_bus *bus = &busses[pci_bdf_ਮੰਜ਼ਿਲ_bus(pci->bdf)];
        ਜੇ (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        ਪੂਰਨ i;
        ਲਈ (i = 0; i < PCI_ਅੰਕ_REGIONS; i++) {
            ਜੇ ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_ਅੰਕ_REGIONS && i < PCI_ROM_ਥਾਂ))
                ਜਾਰੀ;
            ਪੂਰਨ type, is64;
            u64 ਮਾਪ;
            pci_bios_get_bar(pci, i, &type, &ਮਾਪ, &is64);
            ਜੇ (ਮਾਪ == 0)
                ਜਾਰੀ;

            ਜੇ (type != PCI_REGION_TYPE_IO && ਮਾਪ < PCI_DEVICE_MEM_MIN)
                ਮਾਪ = PCI_DEVICE_MEM_MIN;
            ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = pci_region_create_ਇੰਦਰਾਜ(
                bus, pci, i, ਮਾਪ, ਮਾਪ, type, is64);
            ਜੇ (!ਇੰਦਰਾਜ)
                ਵਾਪਸ -1;

            ਜੇ (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    ਪੂਰਨ secondary_bus;
    ਲਈ (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        ਬਣਤਰ pci_bus *s = &busses[secondary_bus];
        ਜੇ (!s->bus_dev)
            ਜਾਰੀ;
        ਬਣਤਰ pci_bus *ਮਾਪਾ = &busses[pci_bdf_ਮੰਜ਼ਿਲ_bus(s->bus_dev->bdf)];
        ਜੇ (!ਮਾਪਾ->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            ਮਾਪਾ = &busses[0];
        ਪੂਰਨ type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_ਲੱਭੋ_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_ਲੱਭੋ_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        ਲਈ (type = 0; type < PCI_REGION_TYPE_ਗਿਣਤੀ; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            ਜੇ (!pci_bridge_has_region(s->bus_dev, type))
                ਜਾਰੀ;
            u64 ਮਾਪ = 0;
            ਜੇ (qemu_cap) {
                u32 tmp_ਮਾਪ;
                u64 tmp_ਮਾਪ_64;
                ਚੋਣ(type) {
                ਹਾਲਤ PCI_REGION_TYPE_IO:
                    tmp_ਮਾਪ_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    ਜੇ (tmp_ਮਾਪ_64 != (u64)-1) {
                        ਮਾਪ = tmp_ਮਾਪ_64;
                    }
                    ਰੋਕੋ;
                ਹਾਲਤ PCI_REGION_TYPE_MEM:
                    tmp_ਮਾਪ = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    ਜੇ (tmp_ਮਾਪ != (u32)-1) {
                        ਮਾਪ = tmp_ਮਾਪ;
                    }
                    ਰੋਕੋ;
                ਹਾਲਤ PCI_REGION_TYPE_PREFMEM:
                    tmp_ਮਾਪ = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_ਮਾਪ_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    ਜੇ (tmp_ਮਾਪ != (u32)-1 && tmp_ਮਾਪ_64 == (u64)-1) {
                        ਮਾਪ = tmp_ਮਾਪ;
                    } ਨਹੀਂਤਾਂ ਜੇ (tmp_ਮਾਪ == (u32)-1 && tmp_ਮਾਪ_64 != (u64)-1) {
                        ਮਾਪ = tmp_ਮਾਪ_64;
                    } ਨਹੀਂਤਾਂ ਜੇ (tmp_ਮਾਪ != (u32)-1 && tmp_ਮਾਪ_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    ਰੋਕੋ;
                ਮੂਲ:
                    ਰੋਕੋ;
                }
            }
            ਜੇ (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 ਜੋੜ = pci_region_ਜੋੜ(&s->r[type]);
            ਪੂਰਨ is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            ਪੂਰਨ resource_optional = 0;
            ਜੇ (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 ਸਿਖਰ_align = pci_mem64_ਸਿਖਰ >> 11;
            ਜੇ (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (ਸਿਖਰ_align > align))
                align = ਸਿਖਰ_align;
            ਜੇ (align > ਜੋੜ && hotplug_support && !resource_optional)
                ਜੋੜ = align; /* reserve min size for hot-plug */
            ਜੇ (ਮਾਪ > ਜੋੜ) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        ਮਾਪ, region_type_name[type]);
                ਜੇ (type != PCI_REGION_TYPE_IO) {
                    ਮਾਪ = ALIGN(ਮਾਪ, align);
                }
            } ਨਹੀਂਤਾਂ {
                ਮਾਪ = ALIGN(ਜੋੜ, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = pci_region_create_ਇੰਦਰਾਜ(
                ਮਾਪਾ, s->bus_dev, -1, ਮਾਪ, align, type, is64);
            ਜੇ (!ਇੰਦਰਾਜ)
                ਵਾਪਸ -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      ਇੰਦਰਾਜ->dev->secondary_bus, ਮਾਪ,
                      region_type_name[ਇੰਦਰਾਜ->type]);
        }
    }
    ਵਾਪਸ 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
ਸਥਿਰ ਪੂਰਨ pci_bios_init_ਜੜ੍ਹ_regions_io(ਬਣਤਰ pci_bus *bus)
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
    ਬਣਤਰ pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 ਜੋੜ = pci_region_ਜੋੜ(r_io);
    ਜੇ (ਜੋੜ < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->ਅਧਾਰ = 0xc000;
    } ਨਹੀਂਤਾਂ ਜੇ (ਜੋੜ < pci_io_ਨੀਵਾਂ_ਸਿਰਾ - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->ਅਧਾਰ = 0x1000;
    } ਨਹੀਂਤਾਂ {
        /* not enouth io address space -> error out */
        ਵਾਪਸ -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->ਅਧਾਰ, r_io->ਅਧਾਰ + ਜੋੜ - 1);
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ pci_bios_init_ਜੜ੍ਹ_regions_mem(ਬਣਤਰ pci_bus *bus)
{
    ਬਣਤਰ pci_region *r_ਸਿਰਾ = &bus->r[PCI_REGION_TYPE_PREFMEM];
    ਬਣਤਰ pci_region *r_ਸ਼ੁਰੂ = &bus->r[PCI_REGION_TYPE_MEM];

    ਜੇ (pci_region_align(r_ਸ਼ੁਰੂ) < pci_region_align(r_ਸਿਰਾ)) {
        // Swap regions to improve alignment.
        r_ਸਿਰਾ = r_ਸ਼ੁਰੂ;
        r_ਸ਼ੁਰੂ = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 ਜੋੜ = pci_region_ਜੋੜ(r_ਸਿਰਾ);
    u64 align = pci_region_align(r_ਸਿਰਾ);
    r_ਸਿਰਾ->ਅਧਾਰ = ALIGN_ਥੱਲੇ((pcimem_ਸਿਰਾ - ਜੋੜ), align);
    ਜੋੜ = pci_region_ਜੋੜ(r_ਸ਼ੁਰੂ);
    align = pci_region_align(r_ਸ਼ੁਰੂ);
    r_ਸ਼ੁਰੂ->ਅਧਾਰ = ALIGN_ਥੱਲੇ((r_ਸਿਰਾ->ਅਧਾਰ - ਜੋੜ), align);

    ਜੇ ((r_ਸ਼ੁਰੂ->ਅਧਾਰ < pcimem_ਸ਼ੁਰੂ) ||
         (r_ਸ਼ੁਰੂ->ਅਧਾਰ > pcimem_ਸਿਰਾ))
        // Memory range requested is larger than available.
        ਵਾਪਸ -1;
    ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_IO_ਖਿਸਕਾਓ            8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਸਮ੍ਰਿਤੀ_ਖਿਸਕਾਓ        16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PREF_ਸਮ੍ਰਿਤੀ_ਖਿਸਕਾਓ   16

ਸਥਿਰ ਖਾਲੀ
pci_region_map_one_ਇੰਦਰਾਜ(ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ, u64 addr)
{
    ਜੇ (ਇੰਦਰਾਜ->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                ਇੰਦਰਾਜ->dev,
                ਇੰਦਰਾਜ->bar, addr, ਇੰਦਰਾਜ->ਮਾਪ, region_type_name[ਇੰਦਰਾਜ->type]);

        pci_ਸੈੱਟ_io_region_addr(ਇੰਦਰਾਜ->dev, ਇੰਦਰਾਜ->bar, addr, ਇੰਦਰਾਜ->is64);
        ਵਾਪਸ;
    }

    u16 bdf = ਇੰਦਰਾਜ->dev->bdf;
    u64 ਹੱਦਬੰਦੀ = addr + ਇੰਦਰਾਜ->ਮਾਪ - 1;
    ਜੇ (ਇੰਦਰਾਜ->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_ਅਧਾਰ, addr >> PCI_IO_ਖਿਸਕਾਓ);
        pci_config_writew(bdf, PCI_IO_ਅਧਾਰ_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_ਹੱਦਬੰਦੀ, ਹੱਦਬੰਦੀ >> PCI_IO_ਖਿਸਕਾਓ);
        pci_config_writew(bdf, PCI_IO_ਹੱਦਬੰਦੀ_UPPER16, 0);
    }
    ਜੇ (ਇੰਦਰਾਜ->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ, addr >> PCI_ਸਮ੍ਰਿਤੀ_ਖਿਸਕਾਓ);
        pci_config_writew(bdf, PCI_ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ, ਹੱਦਬੰਦੀ >> PCI_ਸਮ੍ਰਿਤੀ_ਖਿਸਕਾਓ);
    }
    ਜੇ (ਇੰਦਰਾਜ->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ, addr >> PCI_PREF_ਸਮ੍ਰਿਤੀ_ਖਿਸਕਾਓ);
        pci_config_writew(bdf, PCI_PREF_ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ, ਹੱਦਬੰਦੀ >> PCI_PREF_ਸਮ੍ਰਿਤੀ_ਖਿਸਕਾਓ);
        pci_config_writel(bdf, PCI_PREF_ਅਧਾਰ_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_ਹੱਦਬੰਦੀ_UPPER32, ਹੱਦਬੰਦੀ >> 32);
    }
}

ਸਥਿਰ ਖਾਲੀ pci_region_map_entries(ਬਣਤਰ pci_bus *busses, ਬਣਤਰ pci_region *r)
{
    ਬਣਤਰ hlist_ਗੰਢ *n;
    ਬਣਤਰ pci_region_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
    hlist_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(ਇੰਦਰਾਜ, n, &r->list, ਗੰਢ) {
        u64 addr = r->ਅਧਾਰ;
        r->ਅਧਾਰ += ਇੰਦਰਾਜ->ਮਾਪ;
        ਜੇ (ਇੰਦਰਾਜ->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[ਇੰਦਰਾਜ->dev->secondary_bus].r[ਇੰਦਰਾਜ->type].ਅਧਾਰ = addr;
        pci_region_map_one_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, addr);
        hlist_del(&ਇੰਦਰਾਜ->ਗੰਢ);
        ਮੁਕਤ(ਇੰਦਰਾਜ);
    }
}

ਸਥਿਰ ਖਾਲੀ pci_bios_map_devices(ਬਣਤਰ pci_bus *busses)
{
    ਜੇ (pci_bios_init_ਜੜ੍ਹ_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_ਸ਼ੁਰੂ, pcimem_ਸਿਰਾ);
    ਜੇ (pci_pad_mem64 || pci_bios_init_ਜੜ੍ਹ_regions_mem(busses)) {
        ਬਣਤਰ pci_region r64_mem, r64_pref;
        r64_mem.list.ਪਹਿਲਾ = ਨਿਰਾ;
        r64_pref.list.ਪਹਿਲਾ = ਨਿਰਾ;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        ਜੇ (pci_bios_init_ਜੜ੍ਹ_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 ਜੋੜ_mem = pci_region_ਜੋੜ(&r64_mem);
        u64 ਜੋੜ_pref = pci_region_ਜੋੜ(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.ਅਧਾਰ = le64_ਮੰਜ਼ਿਲ_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        ਜੇ (r64_mem.ਅਧਾਰ < 0x100000000LL + Ram_ਮਾਪ_Over4_G)
            r64_mem.ਅਧਾਰ = 0x100000000LL + Ram_ਮਾਪ_Over4_G;
        ਜੇ (pci_mem64_ਸਿਖਰ) {
            u64 ਮਾਪ = (ALIGN(ਜੋੜ_mem, (1LL<<30)) +
                        ALIGN(ਜੋੜ_pref, (1LL<<30)));
            ਜੇ (pci_pad_mem64)
                ਮਾਪ = ALIGN(ਮਾਪ, pci_mem64_ਸਿਖਰ >> 3);
            ਜੇ (r64_mem.ਅਧਾਰ < pci_mem64_ਸਿਖਰ - ਮਾਪ) {
                r64_mem.ਅਧਾਰ = pci_mem64_ਸਿਖਰ - ਮਾਪ;
            }
            ਜੇ (e820_is_ਵਰਤਿਆ(r64_mem.ਅਧਾਰ, ਮਾਪ))
                r64_mem.ਅਧਾਰ -= ਮਾਪ;
        }
        r64_mem.ਅਧਾਰ = ALIGN(r64_mem.ਅਧਾਰ, align_mem);
        r64_mem.ਅਧਾਰ = ALIGN(r64_mem.ਅਧਾਰ, (1LL<<30));    // 1G hugepage
        r64_pref.ਅਧਾਰ = r64_mem.ਅਧਾਰ + ਜੋੜ_mem;
        r64_pref.ਅਧਾਰ = ALIGN(r64_pref.ਅਧਾਰ, align_pref);
        r64_pref.ਅਧਾਰ = ALIGN(r64_pref.ਅਧਾਰ, (1LL<<30));  // 1G hugepage
        pcimem64_ਸ਼ੁਰੂ = r64_mem.ਅਧਾਰ;
        pcimem64_ਸਿਰਾ = r64_pref.ਅਧਾਰ + ਜੋੜ_pref;
        pcimem64_ਸਿਰਾ = ALIGN(pcimem64_ਸਿਰਾ, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_ਸ਼ੁਰੂ, pcimem64_ਸਿਰਾ);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } ਨਹੀਂਤਾਂ {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_ਸ਼ੁਰੂ = 0;
    }
    // Map regions on each device.
    ਪੂਰਨ bus;
    ਲਈ (bus = 0; bus<=MaxPCIBus; bus++) {
        ਪੂਰਨ type;
        ਲਈ (type = 0; type < PCI_REGION_TYPE_ਗਿਣਤੀ; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

ਖਾਲੀ
pci_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    dprintf(3, "pci setup\n");

    ਜੇ (CPUPhys_ਬਿੱਟ) {
        pci_mem64_ਸਿਖਰ = 1LL << CPUPhys_ਬਿੱਟ;
        ਜੇ (CPUPhys_ਬਿੱਟ > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            ਜੇ (Ram_ਮਾਪ_Over4_G < (1LL << 40)) {
                pci_mem64_ਸਿਖਰ = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    ਜੇ (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_ਮਾਪ_Over4_G ? 1 : 0;

    ਜੇ (!CPULong_ਬਹੁਲਕ)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    ਜੇ (pci_probe_host() != 0) {
        ਵਾਪਸ;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_ਸ਼ੁਰੂ = Ram_ਮਾਪ;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    ਬਣਤਰ pci_bus *busses = malloc_tmp(ਆਕਾਰ(*busses) * (MaxPCIBus + 1));
    ਜੇ (!busses) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(busses, 0, ਆਕਾਰ(*busses) * (MaxPCIBus + 1));
    ਜੇ (pci_bios_check_devices(busses))
        ਵਾਪਸ;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    ਮੁਕਤ(busses);

    pci_enable_default_vga();
}
