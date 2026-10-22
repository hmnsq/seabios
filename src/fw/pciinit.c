// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "byteorder.h" // le64_to_cpu
#شامل "config.h" // CONFIG_*
#شامل "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#شامل "dev-piix.h" // PIIX_*
#شامل "e820map.h" // e820_add
#شامل "hw/ata.h" // PORT_ATA1_CMD_BASE
#شامل "hw/pci.h" // pci_config_readl
#شامل "hw/pcidevice.h" // pci_probe_devices
#شامل "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#شامل "hw/pci_regs.h" // PCI_COMMAND
#شامل "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#شامل "list.h" // struct hlist_node
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "paravirt.h" // RamSize
#شامل "romfile.h" // romfile_loadint
#شامل "string.h" // memset
#شامل "util.h" // pci_setup
#شامل "x86.h" // outb

#تعریف_کرو PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#تعریف_کرو PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#تعریف_کرو PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#تعریف_کرو PCI_ROM_خانہ 6
#تعریف_کرو PCI_عدد_REGIONS 7
#تعریف_کرو PCI_BRIDGE_عدد_REGIONS 2

شمار pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_گنتی,
};

ساکن ثابت حرف *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_آغاز   = BUILD_PCIMEM_آغاز;
u64 pcimem_انتہا     = BUILD_PCIMEM_انتہا;
u64 pcimem64_آغاز = BUILD_PCIMEM64_آغاز;
u64 pcimem64_انتہا   = BUILD_PCIMEM64_انتہا;

// Resource allocation limits
ساکن u64 pci_io_کم_انتہا = 0xa000;
ساکن u64 pci_mem64_چوٹی  = 0;
ساکن u32 pci_pad_mem64  = -1;

ساخت pci_region_اندراج {
    ساخت pci_device *dev;
    صحیح bar;
    u64 سائز;
    u64 align;
    صحیح is64;
    شمار pci_region_type type;
    ساخت hlist_گرہ گرہ;
};

ساخت pci_region {
    /* pci region assignments */
    u64 بنیاد;
    ساخت hlist_سر list;
};

ساخت pci_bus {
    ساخت pci_region r[PCI_REGION_TYPE_گنتی];
    ساخت pci_device *bus_dev;
};

ساکن u32 pci_bar(ساخت pci_device *pci, صحیح region_عدد)
{
    اگر (region_عدد != PCI_ROM_خانہ) {
        واپس PCI_بنیاد_پتہ_0 + region_عدد * 4;
    }

#تعریف_کرو PCI_سرخی_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->سرخی_type & ~PCI_سرخی_TYPE_MULTI_FUNCTION;
    واپس type == PCI_سرخی_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_پتہ;
}

ساکن خالی
pci_سیٹ_io_region_addr(ساخت pci_device *pci, صحیح bar, u64 addr, صحیح is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    اگر (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
ثابت u8 pci_irqs[4] = {
    10, 10, 11, 11
};

ساکن صحیح dummy_pci_خانہ_get_irq(ساخت pci_device *pci, صحیح pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    واپس 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

ساکن صحیح (*pci_خانہ_get_irq)(ساخت pci_device *pci, صحیح pin) =
    dummy_pci_خانہ_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
ساکن صحیح piix_pci_خانہ_get_irq(ساخت pci_device *pci, صحیح pin)
{
    صحیح خانہ_addend = 0;

    جب (pci->والد != تہی) {
        خانہ_addend += pci_bdf_مقصد_dev(pci->bdf);
        pci = pci->والد;
    }
    خانہ_addend += pci_bdf_مقصد_dev(pci->bdf) - 1;
    واپس pci_irqs[(pin - 1 + خانہ_addend) & 3];
}

ساکن صحیح mch_pci_خانہ_get_irq(ساخت pci_device *pci, صحیح pin)
{
    صحیح pin_addend = 0;
    جب (pci->والد != تہی) {
        pin_addend += pci_bdf_مقصد_dev(pci->bdf);
        pci = pci->والد;
    }
    u8 خانہ = pci_bdf_مقصد_dev(pci->bdf);
    اگر (خانہ <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        واپس pci_irqs[(pin - 1 + pin_addend + خانہ) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    واپس pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
ساکن خالی piix_isa_bridge_تیاری(ساخت pci_device *pci, خالی *arg)
{
    صحیح i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    برائے (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));
        /* activate irq remapping in PIIX */
        pci_config_writeb(pci->bdf, 0x60 + i, irq);
    }
    outb(elcr[0], PIIX_پورٹ_ELCR1);
    outb(elcr[1], PIIX_پورٹ_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}

ساکن خالی mch_isa_lpc_تیاری(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_بنیاد | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

ساکن صحیح ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ساکن خالی mch_isa_bridge_تیاری(ساخت pci_device *dev, خالی *arg)
{
    u16 bdf = dev->bdf;
    صحیح i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    برائے (i = 0; i < 4; i++) {
        irq = pci_irqs[i];
        /* set to trigger level */
        elcr[irq >> 3] |= (1 << (irq & 7));

        /* activate irq remapping in LPC */

        /* PIRQ[A-D] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT + i, irq);
        /* PIRQ[E-H] routing */
        pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT + i, irq);
    }
    outb(elcr[0], ICH9_LPC_پورٹ_ELCR1);
    outb(elcr[1], ICH9_LPC_پورٹ_ELCR2);
    dprintf(1, "Q35 LPC init: elcr=%02x %02x\n", elcr[0], elcr[1]);

    ICH9LpcBDF = bdf;

    mch_isa_lpc_تیاری(bdf);

    e820_جوڑو(ICH9_LPC_RCBA_ADDR, 16*1024, E820_RESERVED);

    acpi_pm1a_cnt = acpi_pm_بنیاد + 0x04;
    pmtimer_تیاری(acpi_pm_بنیاد + 0x08);
}

ساکن خالی storage_ide_تیاری(ساخت pci_device *pci, خالی *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_سیٹ_io_region_addr(pci, 0, پورٹ_ATA1_CMD_بنیاد, 0);
    pci_سیٹ_io_region_addr(pci, 1, پورٹ_ATA1_CTRL_بنیاد, 0);
    pci_سیٹ_io_region_addr(pci, 2, پورٹ_ATA2_CMD_بنیاد, 0);
    pci_سیٹ_io_region_addr(pci, 3, پورٹ_ATA2_CTRL_بنیاد, 0);
}

/* PIIX3/PIIX4 IDE */
ساکن خالی piix_ide_تیاری(ساخت pci_device *pci, خالی *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

ساکن خالی pic_ibm_تیاری(ساخت pci_device *pci, خالی *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_سیٹ_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

ساکن خالی apple_macio_تیاری(ساخت pci_device *pci, خالی *arg)
{
    /* macio bridge */
    pci_سیٹ_io_region_addr(pci, 0, 0x80800000, 0);
}

ساکن خالی piix4_pm_config_تیاری(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_مداخلت_سطر, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_بنیاد | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_بنیاد + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

ساکن صحیح PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
ساکن خالی piix4_pm_تیاری(ساخت pci_device *pci, خالی *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_تیاری(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_بنیاد + 0x04;
    pmtimer_تیاری(acpi_pm_بنیاد + 0x08);
}

ساکن خالی ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_بنیاد,
                      (acpi_pm_بنیاد + 0x100) | PCI_بنیاد_پتہ_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

ساکن صحیح ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
ساکن خالی ich9_smbus_تیاری(ساخت pci_device *dev, خالی *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

ساکن خالی intel_igd_تیاری(ساخت pci_device *dev, خالی *arg)
{
    ساخت romfile_s *opregion = romfile_ڈھونڈو("etc/igd-opregion");
    u64 bdsm_سائز = le64_مقصد_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    اگر (opregion && opregion->سائز) {
        خالی *addr = memalign_بلند(صفحہ_سائز, opregion->سائز);
        اگر (!addr) {
            warn_noalloc();
            واپس;
        }

        اگر (opregion->copy(opregion, addr, opregion->سائز) < 0) {
            آزاد(addr);
            واپس;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_مقصد_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->سائز >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    اگر (bdsm_سائز && (dev->bdf == pci_مقصد_bdf(0, 2, 0))) {
        خالی *addr = memalign_tmphigh(1024 * 1024, bdsm_سائز);
        اگر (!addr) {
            warn_noalloc();
            واپس;
        }

        e820_جوڑو((u32)addr, bdsm_سائز, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_مقصد_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_سائز >> 20, dev);
    }
}

ساکن ثابت ساخت pci_device_id pci_device_tbl[] = {
    /* PIIX3/PIIX4 PCI to ISA bridge */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0,
               piix_isa_bridge_تیاری),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0,
               piix_isa_bridge_تیاری),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC,
               mch_isa_bridge_تیاری),

    /* STORAGE IDE */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1,
                     PCI_CLASS_STORAGE_IDE, piix_ide_تیاری),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB,
                     PCI_CLASS_STORAGE_IDE, piix_ide_تیاری),
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
                     storage_ide_تیاری),

    /* PIC, IBM, MPIC & MPIC2 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0x0046, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_تیاری),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0xFFFF, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_تیاری),

    /* PIIX4 Power Management device (for ACPI) */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3,
               piix4_pm_تیاری),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_SMBUS,
               ich9_smbus_تیاری),

    /* 0xff00 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0017, 0xff00, apple_macio_تیاری),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0022, 0xff00, apple_macio_تیاری),

    /* Intel IGD OpRegion setup */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
                     intel_igd_تیاری),

    PCI_DEVICE_انتہا,
};

ساکن صحیح MCHMmcfgBDF = -1;
ساکن خالی mch_mmconfig_تیاری(u16 bdf);

خالی pci_resume(خالی)
{
    اگر (!CONFIG_QEMU) {
        واپس;
    }

    اگر (PiixPmBDF >= 0) {
        piix4_pm_config_تیاری(PiixPmBDF);
    }

    اگر (ICH9LpcBDF >= 0) {
        mch_isa_lpc_تیاری(ICH9LpcBDF);
    }

    اگر (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    اگر(MCHMmcfgBDF >= 0) {
        mch_mmconfig_تیاری(MCHMmcfgBDF);
    }
}

ساکن خالی pci_bios_init_device(ساخت pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    صحیح pin = pci_config_readb(bdf, PCI_مداخلت_PIN);
    اگر (pin != 0)
        pci_config_writeb(bdf, PCI_مداخلت_سطر, pci_خانہ_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, تہی);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_حافظہ | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    اگر (pci->سرخی_type & PCI_سرخی_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

ساکن خالی pci_bios_init_devices(خالی)
{
    ساخت pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

ساکن خالی pci_enable_default_vga(خالی)
{
    ساخت pci_device *pci;

    foreachpci(pci) {
        اگر (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            واپس;
        }
    }

    pci = pci_ڈھونڈو_class(PCI_CLASS_DISPLAY_VGA);
    اگر (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        واپس;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_حافظہ);

    جب (pci->والد) {
        pci = pci->والد;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_حافظہ);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

ساکن خالی i440fx_mem_addr_تیاری(ساخت pci_device *dev, خالی *arg)
{
    اگر (Ram_سائز <= 0x80000000)
        pcimem_آغاز = 0x80000000;
    ورنہ اگر (Ram_سائز <= 0xc0000000)
        pcimem_آغاز = 0xc0000000;

    pci_خانہ_get_irq = piix_pci_خانہ_get_irq;
}

ساکن خالی mch_mmconfig_تیاری(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 بالائی = addr >> 32;
    u32 زیریں = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, بالائی);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, زیریں);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

ساکن خالی mch_mem_addr_تیاری(ساخت pci_device *dev, خالی *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 سائز = Q35_HOST_BRIDGE_PCIEXBAR_سائز;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_تیاری(dev->bdf);
    e820_جوڑو(addr, سائز, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_آغاز = addr + سائز;

    pci_خانہ_get_irq = mch_pci_خانہ_get_irq;

    /* setup io address space */
    اگر (acpi_pm_بنیاد < 0x1000)
        pci_io_کم_انتہا = 0x10000;
    ورنہ
        pci_io_کم_انتہا = acpi_pm_بنیاد;
}

ساکن ثابت ساخت pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_تیاری),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_تیاری),
    PCI_DEVICE_انتہا
};

ساکن خالی pci_bios_init_platform(خالی)
{
    ساخت pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, تہی);
    }
}

ساکن u8 pci_ڈھونڈو_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    اگر (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        واپس 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    اگر (device_id != PCI_DEVICE_ID_REDHAT_جڑ_پورٹ &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        واپس 0;
    }
    u8 cap = 0;

    کرو {
        cap = pci_ڈھونڈو_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } جب (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_انحراف) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    اگر (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        اگر (cap_len < RES_RESERVE_CAP_سائز) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            واپس 0;
        }
    } ورنہ {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    واپس cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

ساکن خالی
pci_bios_init_bus_rec(صحیح bus, u8 *pci_bus)
{
    صحیح bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        اگر (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        اگر (class != PCI_CLASS_BRIDGE_PCI) {
            جاری;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        اگر (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } ورنہ {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        اگر (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } ورنہ {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        اگر (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_ڈھونڈو_resource_reserve_capability(bdf);

            اگر (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                اگر (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    اگر ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    اگر (secbus + res_bus > *pci_bus) {
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
        } ورنہ {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

ساکن خالی
pci_bios_init_bus(خالی)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    اگر (extraroots) {
        جب (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

ساکن خالی
pci_bios_get_bar(ساخت pci_device *pci, صحیح bar,
                 صحیح *ptype, u64 *psize, صحیح *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    صحیح is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 نقاب;

    اگر (bar == PCI_ROM_خانہ) {
        نقاب = PCI_ROM_پتہ_نقاب;
        pci_config_writel(bdf, ofs, نقاب);
    } ورنہ {
        اگر (old & PCI_بنیاد_پتہ_SPACE_IO) {
            نقاب = PCI_بنیاد_پتہ_IO_نقاب;
            type = PCI_REGION_TYPE_IO;
        } ورنہ {
            نقاب = PCI_بنیاد_پتہ_MEM_نقاب;
            اگر (old & PCI_بنیاد_پتہ_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_بنیاد_پتہ_MEM_TYPE_نقاب)
                    == PCI_بنیاد_پتہ_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    اگر (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 بلند = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)بلند << 32);
        نقاب |= ((u64)0xffffffff << 32);
        *psize = (~(val & نقاب)) + 1;
    } ورنہ {
        *psize = ((~(val & نقاب)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

ساکن صحیح pci_bios_bridge_region_is64(ساخت pci_region *r,
                                 ساخت pci_device *pci, صحیح type)
{
    اگر (type != PCI_REGION_TYPE_PREFMEM)
        واپس 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_حافظہ_بنیاد);
    اگر (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_حافظہ_بنیاد, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_حافظہ_بنیاد);
        pci_config_writel(pci->bdf, PCI_PREF_حافظہ_بنیاد, 0x0);
    }
    اگر ((pmem & PCI_PREF_RANGE_TYPE_نقاب) != PCI_PREF_RANGE_TYPE_64)
       واپس 0;
    ساخت pci_region_اندراج *اندراج_2;
    hlist_for_each_اندراج(اندراج_2, &r->list, گرہ) {
        اگر (!اندراج_2->is64)
            واپس 0;
    }
    واپس 1;
}

ساکن u64 pci_region_align(ساخت pci_region *r)
{
    ساخت pci_region_اندراج *اندراج_2;
    hlist_for_each_اندراج(اندراج_2, &r->list, گرہ) {
        // The first entry in the sorted list has the largest alignment
        واپس اندراج_2->align;
    }
    واپس 1;
}

ساکن u64 pci_region_مجموع(ساخت pci_region *r)
{
    u64 مجموع = 0;
    ساخت pci_region_اندراج *اندراج_2;
    hlist_for_each_اندراج(اندراج_2, &r->list, گرہ) {
        مجموع += اندراج_2->سائز;
    }
    واپس مجموع;
}

ساکن خالی pci_region_migrate_64bit_entries(ساخت pci_region *from,
                                             ساخت pci_region *مقصد)
{
    ساخت hlist_گرہ *n, **آخری = &مقصد->list.پہلا;
    ساخت pci_region_اندراج *اندراج_2;
    hlist_for_each_اندراج_محفوظ(اندراج_2, n, &from->list, گرہ) {
        اگر (!اندراج_2->is64)
            جاری;
        اگر (اندراج_2->dev->class == PCI_CLASS_SERIAL_USB)
            جاری;
        اگر (اندراج_2->dev->class == PCI_CLASS_STORAGE_NVME)
            جاری;
        // Move from source list to destination list.
        hlist_del(&اندراج_2->گرہ);
        hlist_جوڑو(&اندراج_2->گرہ, آخری);
        آخری = &اندراج_2->گرہ.اگلا;
    }
}

ساکن ساخت pci_region_اندراج *
pci_region_create_اندراج(ساخت pci_bus *bus, ساخت pci_device *dev,
                        صحیح bar, u64 سائز, u64 align, صحیح type, صحیح is64)
{
    ساخت pci_region_اندراج *اندراج_2 = malloc_tmp(حجم(*اندراج_2));
    اگر (!اندراج_2) {
        warn_noalloc();
        واپس تہی;
    }
    memset(اندراج_2, 0, حجم(*اندراج_2));
    اندراج_2->dev = dev;
    اندراج_2->bar = bar;
    اندراج_2->سائز = سائز;
    اندراج_2->align = align;
    اندراج_2->is64 = is64;
    اندراج_2->type = type;
    // Insert into list in sorted order.
    ساخت hlist_گرہ **pprev;
    ساخت pci_region_اندراج *pos;
    hlist_for_each_اندراج_pprev(pos, pprev, &bus->r[type].list, گرہ) {
        اگر (pos->align < align || (pos->align == align && pos->سائز < سائز))
            توڑو;
    }
    hlist_جوڑو(&اندراج_2->گرہ, pprev);
    واپس اندراج_2;
}

قسمتعریف شمار hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

ساکن hotplug_type_t pci_bus_hotplug_support(ساخت pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    اگر (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 پورٹ_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        اگر (پورٹ_type == PCI_EXP_TYPE_PCI_BRIDGE)
            جاؤ check_shpc;

        u8 downstream_پورٹ = (پورٹ_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (پورٹ_type == PCI_EXP_TYPE_جڑ_پورٹ);
        /*
         * PCI Express SPEC, 7.8.2:
         *   Slot Implemented – When Set, this bit indicates that the Link
         *   HwInit associated with this Port is connected to a slot (as
         *   compared to being connected to a system-integrated device or
         *   being disabled).
         *   This bit is valid for Downstream Ports. This bit is undefined
         *   for Upstream Ports.
         */
        u16 خانہ_implemented = pcie_flags & PCI_EXP_FLAGS_خانہ;

        واپس downstream_پورٹ && خانہ_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_ڈھونڈو_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    واپس !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
ساکن صحیح pci_bridge_has_region(ساخت pci_device *pci,
                                 شمار pci_region_type region_type)
{
    u8 بنیاد;

    انتخاب (region_type) {
        حالت PCI_REGION_TYPE_IO:
            بنیاد = PCI_IO_بنیاد;
            توڑو;
        حالت PCI_REGION_TYPE_PREFMEM:
            بنیاد = PCI_PREF_حافظہ_بنیاد;
            توڑو;
        طےشدہ:
            /* Regular memory support is mandatory */
            واپس 1;
    }

    pci_config_writeb(pci->bdf, بنیاد, 0xFF);

    واپس pci_config_readb(pci->bdf, بنیاد) != 0;
}

ساکن صحیح pci_bios_check_devices(ساخت pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        ساخت pci_bus *bus = &busses[pci_bdf_مقصد_bus(pci->bdf)];
        اگر (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        صحیح i;
        برائے (i = 0; i < PCI_عدد_REGIONS; i++) {
            اگر ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_عدد_REGIONS && i < PCI_ROM_خانہ))
                جاری;
            صحیح type, is64;
            u64 سائز;
            pci_bios_get_bar(pci, i, &type, &سائز, &is64);
            اگر (سائز == 0)
                جاری;

            اگر (type != PCI_REGION_TYPE_IO && سائز < PCI_DEVICE_MEM_MIN)
                سائز = PCI_DEVICE_MEM_MIN;
            ساخت pci_region_اندراج *اندراج_2 = pci_region_create_اندراج(
                bus, pci, i, سائز, سائز, type, is64);
            اگر (!اندراج_2)
                واپس -1;

            اگر (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    صحیح secondary_bus;
    برائے (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        ساخت pci_bus *s = &busses[secondary_bus];
        اگر (!s->bus_dev)
            جاری;
        ساخت pci_bus *والد = &busses[pci_bdf_مقصد_bus(s->bus_dev->bdf)];
        اگر (!والد->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            والد = &busses[0];
        صحیح type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_ڈھونڈو_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_ڈھونڈو_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        برائے (type = 0; type < PCI_REGION_TYPE_گنتی; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            اگر (!pci_bridge_has_region(s->bus_dev, type))
                جاری;
            u64 سائز = 0;
            اگر (qemu_cap) {
                u32 tmp_سائز;
                u64 tmp_سائز_64;
                انتخاب(type) {
                حالت PCI_REGION_TYPE_IO:
                    tmp_سائز_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    اگر (tmp_سائز_64 != (u64)-1) {
                        سائز = tmp_سائز_64;
                    }
                    توڑو;
                حالت PCI_REGION_TYPE_MEM:
                    tmp_سائز = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    اگر (tmp_سائز != (u32)-1) {
                        سائز = tmp_سائز;
                    }
                    توڑو;
                حالت PCI_REGION_TYPE_PREFMEM:
                    tmp_سائز = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_سائز_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    اگر (tmp_سائز != (u32)-1 && tmp_سائز_64 == (u64)-1) {
                        سائز = tmp_سائز;
                    } ورنہ اگر (tmp_سائز == (u32)-1 && tmp_سائز_64 != (u64)-1) {
                        سائز = tmp_سائز_64;
                    } ورنہ اگر (tmp_سائز != (u32)-1 && tmp_سائز_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    توڑو;
                طےشدہ:
                    توڑو;
                }
            }
            اگر (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 مجموع = pci_region_مجموع(&s->r[type]);
            صحیح is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            صحیح resource_optional = 0;
            اگر (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 چوٹی_align = pci_mem64_چوٹی >> 11;
            اگر (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (چوٹی_align > align))
                align = چوٹی_align;
            اگر (align > مجموع && hotplug_support && !resource_optional)
                مجموع = align; /* reserve min size for hot-plug */
            اگر (سائز > مجموع) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        سائز, region_type_name[type]);
                اگر (type != PCI_REGION_TYPE_IO) {
                    سائز = ALIGN(سائز, align);
                }
            } ورنہ {
                سائز = ALIGN(مجموع, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            ساخت pci_region_اندراج *اندراج_2 = pci_region_create_اندراج(
                والد, s->bus_dev, -1, سائز, align, type, is64);
            اگر (!اندراج_2)
                واپس -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      اندراج_2->dev->secondary_bus, سائز,
                      region_type_name[اندراج_2->type]);
        }
    }
    واپس 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
ساکن صحیح pci_bios_init_جڑ_regions_io(ساخت pci_bus *bus)
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
    ساخت pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 مجموع = pci_region_مجموع(r_io);
    اگر (مجموع < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->بنیاد = 0xc000;
    } ورنہ اگر (مجموع < pci_io_کم_انتہا - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->بنیاد = 0x1000;
    } ورنہ {
        /* not enouth io address space -> error out */
        واپس -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->بنیاد, r_io->بنیاد + مجموع - 1);
    واپس 0;
}

ساکن صحیح pci_bios_init_جڑ_regions_mem(ساخت pci_bus *bus)
{
    ساخت pci_region *r_انتہا = &bus->r[PCI_REGION_TYPE_PREFMEM];
    ساخت pci_region *r_آغاز = &bus->r[PCI_REGION_TYPE_MEM];

    اگر (pci_region_align(r_آغاز) < pci_region_align(r_انتہا)) {
        // Swap regions to improve alignment.
        r_انتہا = r_آغاز;
        r_آغاز = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 مجموع = pci_region_مجموع(r_انتہا);
    u64 align = pci_region_align(r_انتہا);
    r_انتہا->بنیاد = ALIGN_نیچے_کو((pcimem_انتہا - مجموع), align);
    مجموع = pci_region_مجموع(r_آغاز);
    align = pci_region_align(r_آغاز);
    r_آغاز->بنیاد = ALIGN_نیچے_کو((r_انتہا->بنیاد - مجموع), align);

    اگر ((r_آغاز->بنیاد < pcimem_آغاز) ||
         (r_آغاز->بنیاد > pcimem_انتہا))
        // Memory range requested is larger than available.
        واپس -1;
    واپس 0;
}

#تعریف_کرو PCI_IO_سرکاؤ            8
#تعریف_کرو PCI_حافظہ_سرکاؤ        16
#تعریف_کرو PCI_PREF_حافظہ_سرکاؤ   16

ساکن خالی
pci_region_map_one_اندراج(ساخت pci_region_اندراج *اندراج_2, u64 addr)
{
    اگر (اندراج_2->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                اندراج_2->dev,
                اندراج_2->bar, addr, اندراج_2->سائز, region_type_name[اندراج_2->type]);

        pci_سیٹ_io_region_addr(اندراج_2->dev, اندراج_2->bar, addr, اندراج_2->is64);
        واپس;
    }

    u16 bdf = اندراج_2->dev->bdf;
    u64 حد_بندی = addr + اندراج_2->سائز - 1;
    اگر (اندراج_2->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_بنیاد, addr >> PCI_IO_سرکاؤ);
        pci_config_writew(bdf, PCI_IO_بنیاد_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_حد_بندی, حد_بندی >> PCI_IO_سرکاؤ);
        pci_config_writew(bdf, PCI_IO_حد_بندی_UPPER16, 0);
    }
    اگر (اندراج_2->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_حافظہ_بنیاد, addr >> PCI_حافظہ_سرکاؤ);
        pci_config_writew(bdf, PCI_حافظہ_حد_بندی, حد_بندی >> PCI_حافظہ_سرکاؤ);
    }
    اگر (اندراج_2->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_حافظہ_بنیاد, addr >> PCI_PREF_حافظہ_سرکاؤ);
        pci_config_writew(bdf, PCI_PREF_حافظہ_حد_بندی, حد_بندی >> PCI_PREF_حافظہ_سرکاؤ);
        pci_config_writel(bdf, PCI_PREF_بنیاد_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_حد_بندی_UPPER32, حد_بندی >> 32);
    }
}

ساکن خالی pci_region_map_entries(ساخت pci_bus *busses, ساخت pci_region *r)
{
    ساخت hlist_گرہ *n;
    ساخت pci_region_اندراج *اندراج_2;
    hlist_for_each_اندراج_محفوظ(اندراج_2, n, &r->list, گرہ) {
        u64 addr = r->بنیاد;
        r->بنیاد += اندراج_2->سائز;
        اگر (اندراج_2->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[اندراج_2->dev->secondary_bus].r[اندراج_2->type].بنیاد = addr;
        pci_region_map_one_اندراج(اندراج_2, addr);
        hlist_del(&اندراج_2->گرہ);
        آزاد(اندراج_2);
    }
}

ساکن خالی pci_bios_map_devices(ساخت pci_bus *busses)
{
    اگر (pci_bios_init_جڑ_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_آغاز, pcimem_انتہا);
    اگر (pci_pad_mem64 || pci_bios_init_جڑ_regions_mem(busses)) {
        ساخت pci_region r64_mem, r64_pref;
        r64_mem.list.پہلا = تہی;
        r64_pref.list.پہلا = تہی;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        اگر (pci_bios_init_جڑ_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 مجموع_mem = pci_region_مجموع(&r64_mem);
        u64 مجموع_pref = pci_region_مجموع(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.بنیاد = le64_مقصد_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        اگر (r64_mem.بنیاد < 0x100000000LL + Ram_سائز_Over4_G)
            r64_mem.بنیاد = 0x100000000LL + Ram_سائز_Over4_G;
        اگر (pci_mem64_چوٹی) {
            u64 سائز = (ALIGN(مجموع_mem, (1LL<<30)) +
                        ALIGN(مجموع_pref, (1LL<<30)));
            اگر (pci_pad_mem64)
                سائز = ALIGN(سائز, pci_mem64_چوٹی >> 3);
            اگر (r64_mem.بنیاد < pci_mem64_چوٹی - سائز) {
                r64_mem.بنیاد = pci_mem64_چوٹی - سائز;
            }
            اگر (e820_is_مستعمل(r64_mem.بنیاد, سائز))
                r64_mem.بنیاد -= سائز;
        }
        r64_mem.بنیاد = ALIGN(r64_mem.بنیاد, align_mem);
        r64_mem.بنیاد = ALIGN(r64_mem.بنیاد, (1LL<<30));    // 1G hugepage
        r64_pref.بنیاد = r64_mem.بنیاد + مجموع_mem;
        r64_pref.بنیاد = ALIGN(r64_pref.بنیاد, align_pref);
        r64_pref.بنیاد = ALIGN(r64_pref.بنیاد, (1LL<<30));  // 1G hugepage
        pcimem64_آغاز = r64_mem.بنیاد;
        pcimem64_انتہا = r64_pref.بنیاد + مجموع_pref;
        pcimem64_انتہا = ALIGN(pcimem64_انتہا, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_آغاز, pcimem64_انتہا);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } ورنہ {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_آغاز = 0;
    }
    // Map regions on each device.
    صحیح bus;
    برائے (bus = 0; bus<=MaxPCIBus; bus++) {
        صحیح type;
        برائے (type = 0; type < PCI_REGION_TYPE_گنتی; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

خالی
pci_تیاری(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    dprintf(3, "pci setup\n");

    اگر (CPUPhys_بٹ) {
        pci_mem64_چوٹی = 1LL << CPUPhys_بٹ;
        اگر (CPUPhys_بٹ > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            اگر (Ram_سائز_Over4_G < (1LL << 40)) {
                pci_mem64_چوٹی = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    اگر (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_سائز_Over4_G ? 1 : 0;

    اگر (!CPULong_منوال)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    اگر (pci_probe_host() != 0) {
        واپس;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_آغاز = Ram_سائز;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    ساخت pci_bus *busses = malloc_tmp(حجم(*busses) * (MaxPCIBus + 1));
    اگر (!busses) {
        warn_noalloc();
        واپس;
    }
    memset(busses, 0, حجم(*busses) * (MaxPCIBus + 1));
    اگر (pci_bios_check_devices(busses))
        واپس;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    آزاد(busses);

    pci_enable_default_vga();
}
