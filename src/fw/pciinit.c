// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "byteorder.h" // le64_to_cpu
#ضمن "config.h" // CONFIG_*
#ضمن "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#ضمن "dev-piix.h" // PIIX_*
#ضمن "e820map.h" // e820_add
#ضمن "hw/ata.h" // PORT_ATA1_CMD_BASE
#ضمن "hw/pci.h" // pci_config_readl
#ضمن "hw/pcidevice.h" // pci_probe_devices
#ضمن "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ضمن "hw/pci_regs.h" // PCI_COMMAND
#ضمن "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#ضمن "list.h" // struct hlist_node
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // RamSize
#ضمن "romfile.h" // romfile_loadint
#ضمن "string.h" // memset
#ضمن "util.h" // pci_setup
#ضمن "x86.h" // outb

#تعریف_کرو PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#تعریف_کرو PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#تعریف_کرو PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#تعریف_کرو PCI_ROM_تھاں 6
#تعریف_کرو PCI_عدد_REGIONS 7
#تعریف_کرو PCI_BRIDGE_عدد_REGIONS 2

تعداد pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_گنتی,
};

ساكن ثابت حرف *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_شروع   = BUILD_PCIMEM_شروع;
u64 pcimem_سرا     = BUILD_PCIMEM_سرا;
u64 pcimem64_شروع = BUILD_PCIMEM64_شروع;
u64 pcimem64_سرا   = BUILD_PCIMEM64_سرا;

// Resource allocation limits
ساكن u64 pci_io_کم_سرا = 0xa000;
ساكن u64 pci_mem64_چوٹی  = 0;
ساكن u32 pci_pad_mem64  = -1;

بنية pci_region_اندراج {
    بنية pci_device *dev;
    صحيح bar;
    u64 ناپ;
    u64 align;
    صحيح is64;
    تعداد pci_region_type type;
    بنية hlist_گرہ گرہ;
};

بنية pci_region {
    /* pci region assignments */
    u64 ادھار;
    بنية hlist_سر list;
};

بنية pci_bus {
    بنية pci_region r[PCI_REGION_TYPE_گنتی];
    بنية pci_device *bus_dev;
};

ساكن u32 pci_bar(بنية pci_device *pci, صحيح region_عدد)
{
    إذا (region_عدد != PCI_ROM_تھاں) {
        أرجع PCI_ادھار_پتہ_0 + region_عدد * 4;
    }

#تعریف_کرو PCI_سرخی_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->سرخی_type & ~PCI_سرخی_TYPE_MULTI_FUNCTION;
    أرجع type == PCI_سرخی_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_پتہ;
}

ساكن فراغ
pci_سیٹ_io_region_addr(بنية pci_device *pci, صحيح bar, u64 addr, صحيح is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    إذا (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
ثابت u8 pci_irqs[4] = {
    10, 10, 11, 11
};

ساكن صحيح dummy_pci_تھاں_get_irq(بنية pci_device *pci, صحيح pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    أرجع 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

ساكن صحيح (*pci_تھاں_get_irq)(بنية pci_device *pci, صحيح pin) =
    dummy_pci_تھاں_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
ساكن صحيح piix_pci_تھاں_get_irq(بنية pci_device *pci, صحيح pin)
{
    صحيح تھاں_addend = 0;

    طالما (pci->ماپا != عدم) {
        تھاں_addend += pci_bdf_منزل_dev(pci->bdf);
        pci = pci->ماپا;
    }
    تھاں_addend += pci_bdf_منزل_dev(pci->bdf) - 1;
    أرجع pci_irqs[(pin - 1 + تھاں_addend) & 3];
}

ساكن صحيح mch_pci_تھاں_get_irq(بنية pci_device *pci, صحيح pin)
{
    صحيح pin_addend = 0;
    طالما (pci->ماپا != عدم) {
        pin_addend += pci_bdf_منزل_dev(pci->bdf);
        pci = pci->ماپا;
    }
    u8 تھاں = pci_bdf_منزل_dev(pci->bdf);
    إذا (تھاں <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        أرجع pci_irqs[(pin - 1 + pin_addend + تھاں) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    أرجع pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
ساكن فراغ piix_isa_bridge_تیاری(بنية pci_device *pci, فراغ *arg)
{
    صحيح i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    لكل (i = 0; i < 4; i++) {
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

ساكن فراغ mch_isa_lpc_تیاری(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_ادھار | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

ساكن صحيح ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ساكن فراغ mch_isa_bridge_تیاری(بنية pci_device *dev, فراغ *arg)
{
    u16 bdf = dev->bdf;
    صحيح i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    لكل (i = 0; i < 4; i++) {
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

    acpi_pm1a_cnt = acpi_pm_ادھار + 0x04;
    pmtimer_تیاری(acpi_pm_ادھار + 0x08);
}

ساكن فراغ storage_ide_تیاری(بنية pci_device *pci, فراغ *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_سیٹ_io_region_addr(pci, 0, پورٹ_ATA1_CMD_ادھار, 0);
    pci_سیٹ_io_region_addr(pci, 1, پورٹ_ATA1_CTRL_ادھار, 0);
    pci_سیٹ_io_region_addr(pci, 2, پورٹ_ATA2_CMD_ادھار, 0);
    pci_سیٹ_io_region_addr(pci, 3, پورٹ_ATA2_CTRL_ادھار, 0);
}

/* PIIX3/PIIX4 IDE */
ساكن فراغ piix_ide_تیاری(بنية pci_device *pci, فراغ *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

ساكن فراغ pic_ibm_تیاری(بنية pci_device *pci, فراغ *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_سیٹ_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

ساكن فراغ apple_macio_تیاری(بنية pci_device *pci, فراغ *arg)
{
    /* macio bridge */
    pci_سیٹ_io_region_addr(pci, 0, 0x80800000, 0);
}

ساكن فراغ piix4_pm_config_تیاری(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_رکاوٹ_سطر, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_ادھار | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_ادھار + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

ساكن صحيح PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
ساكن فراغ piix4_pm_تیاری(بنية pci_device *pci, فراغ *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_تیاری(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_ادھار + 0x04;
    pmtimer_تیاری(acpi_pm_ادھار + 0x08);
}

ساكن فراغ ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_ادھار,
                      (acpi_pm_ادھار + 0x100) | PCI_ادھار_پتہ_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

ساكن صحيح ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
ساكن فراغ ich9_smbus_تیاری(بنية pci_device *dev, فراغ *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

ساكن فراغ intel_igd_تیاری(بنية pci_device *dev, فراغ *arg)
{
    بنية romfile_s *opregion = romfile_لبھو("etc/igd-opregion");
    u64 bdsm_ناپ = le64_منزل_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    إذا (opregion && opregion->ناپ) {
        فراغ *addr = memalign_بلند(ورقہ_ناپ, opregion->ناپ);
        إذا (!addr) {
            warn_noalloc();
            أرجع;
        }

        إذا (opregion->copy(opregion, addr, opregion->ناپ) < 0) {
            حرر(addr);
            أرجع;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_منزل_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->ناپ >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    إذا (bdsm_ناپ && (dev->bdf == pci_منزل_bdf(0, 2, 0))) {
        فراغ *addr = memalign_tmphigh(1024 * 1024, bdsm_ناپ);
        إذا (!addr) {
            warn_noalloc();
            أرجع;
        }

        e820_جوڑو((u32)addr, bdsm_ناپ, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_منزل_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_ناپ >> 20, dev);
    }
}

ساكن ثابت بنية pci_device_id pci_device_tbl[] = {
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

    PCI_DEVICE_سرا,
};

ساكن صحيح MCHMmcfgBDF = -1;
ساكن فراغ mch_mmconfig_تیاری(u16 bdf);

فراغ pci_resume(فراغ)
{
    إذا (!CONFIG_QEMU) {
        أرجع;
    }

    إذا (PiixPmBDF >= 0) {
        piix4_pm_config_تیاری(PiixPmBDF);
    }

    إذا (ICH9LpcBDF >= 0) {
        mch_isa_lpc_تیاری(ICH9LpcBDF);
    }

    إذا (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    إذا(MCHMmcfgBDF >= 0) {
        mch_mmconfig_تیاری(MCHMmcfgBDF);
    }
}

ساكن فراغ pci_bios_init_device(بنية pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    صحيح pin = pci_config_readb(bdf, PCI_رکاوٹ_PIN);
    إذا (pin != 0)
        pci_config_writeb(bdf, PCI_رکاوٹ_سطر, pci_تھاں_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, عدم);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_حافظہ | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    إذا (pci->سرخی_type & PCI_سرخی_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

ساكن فراغ pci_bios_init_devices(فراغ)
{
    بنية pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

ساكن فراغ pci_enable_default_vga(فراغ)
{
    بنية pci_device *pci;

    foreachpci(pci) {
        إذا (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            أرجع;
        }
    }

    pci = pci_لبھو_class(PCI_CLASS_DISPLAY_VGA);
    إذا (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        أرجع;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_حافظہ);

    طالما (pci->ماپا) {
        pci = pci->ماپا;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_حافظہ);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

ساكن فراغ i440fx_mem_addr_تیاری(بنية pci_device *dev, فراغ *arg)
{
    إذا (Ram_ناپ <= 0x80000000)
        pcimem_شروع = 0x80000000;
    وإلا إذا (Ram_ناپ <= 0xc0000000)
        pcimem_شروع = 0xc0000000;

    pci_تھاں_get_irq = piix_pci_تھاں_get_irq;
}

ساكن فراغ mch_mmconfig_تیاری(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 اتلا = addr >> 32;
    u32 ہیٹھلا = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, اتلا);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, ہیٹھلا);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

ساكن فراغ mch_mem_addr_تیاری(بنية pci_device *dev, فراغ *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 ناپ = Q35_HOST_BRIDGE_PCIEXBAR_ناپ;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_تیاری(dev->bdf);
    e820_جوڑو(addr, ناپ, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_شروع = addr + ناپ;

    pci_تھاں_get_irq = mch_pci_تھاں_get_irq;

    /* setup io address space */
    إذا (acpi_pm_ادھار < 0x1000)
        pci_io_کم_سرا = 0x10000;
    وإلا
        pci_io_کم_سرا = acpi_pm_ادھار;
}

ساكن ثابت بنية pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_تیاری),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_تیاری),
    PCI_DEVICE_سرا
};

ساكن فراغ pci_bios_init_platform(فراغ)
{
    بنية pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, عدم);
    }
}

ساكن u8 pci_لبھو_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    إذا (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        أرجع 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    إذا (device_id != PCI_DEVICE_ID_REDHAT_جڑ_پورٹ &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        أرجع 0;
    }
    u8 cap = 0;

    افعل {
        cap = pci_لبھو_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } طالما (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_وتھ) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    إذا (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        إذا (cap_len < RES_RESERVE_CAP_ناپ) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            أرجع 0;
        }
    } وإلا {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    أرجع cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

ساكن فراغ
pci_bios_init_bus_rec(صحيح bus, u8 *pci_bus)
{
    صحيح bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        إذا (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        إذا (class != PCI_CLASS_BRIDGE_PCI) {
            استمر;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        إذا (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } وإلا {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        إذا (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } وإلا {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        إذا (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_لبھو_resource_reserve_capability(bdf);

            إذا (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                إذا (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    إذا ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    إذا (secbus + res_bus > *pci_bus) {
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
        } وإلا {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

ساكن فراغ
pci_bios_init_bus(فراغ)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    إذا (extraroots) {
        طالما (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

ساكن فراغ
pci_bios_get_bar(بنية pci_device *pci, صحيح bar,
                 صحيح *ptype, u64 *psize, صحيح *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    صحيح is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 نقاب;

    إذا (bar == PCI_ROM_تھاں) {
        نقاب = PCI_ROM_پتہ_نقاب;
        pci_config_writel(bdf, ofs, نقاب);
    } وإلا {
        إذا (old & PCI_ادھار_پتہ_SPACE_IO) {
            نقاب = PCI_ادھار_پتہ_IO_نقاب;
            type = PCI_REGION_TYPE_IO;
        } وإلا {
            نقاب = PCI_ادھار_پتہ_MEM_نقاب;
            إذا (old & PCI_ادھار_پتہ_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_ادھار_پتہ_MEM_TYPE_نقاب)
                    == PCI_ادھار_پتہ_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    إذا (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 بلند = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)بلند << 32);
        نقاب |= ((u64)0xffffffff << 32);
        *psize = (~(val & نقاب)) + 1;
    } وإلا {
        *psize = ((~(val & نقاب)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

ساكن صحيح pci_bios_bridge_region_is64(بنية pci_region *r,
                                 بنية pci_device *pci, صحيح type)
{
    إذا (type != PCI_REGION_TYPE_PREFMEM)
        أرجع 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_حافظہ_ادھار);
    إذا (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_حافظہ_ادھار, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_حافظہ_ادھار);
        pci_config_writel(pci->bdf, PCI_PREF_حافظہ_ادھار, 0x0);
    }
    إذا ((pmem & PCI_PREF_RANGE_TYPE_نقاب) != PCI_PREF_RANGE_TYPE_64)
       أرجع 0;
    بنية pci_region_اندراج *اندراج;
    hlist_for_each_اندراج(اندراج, &r->list, گرہ) {
        إذا (!اندراج->is64)
            أرجع 0;
    }
    أرجع 1;
}

ساكن u64 pci_region_align(بنية pci_region *r)
{
    بنية pci_region_اندراج *اندراج;
    hlist_for_each_اندراج(اندراج, &r->list, گرہ) {
        // The first entry in the sorted list has the largest alignment
        أرجع اندراج->align;
    }
    أرجع 1;
}

ساكن u64 pci_region_جوڑ(بنية pci_region *r)
{
    u64 جوڑ = 0;
    بنية pci_region_اندراج *اندراج;
    hlist_for_each_اندراج(اندراج, &r->list, گرہ) {
        جوڑ += اندراج->ناپ;
    }
    أرجع جوڑ;
}

ساكن فراغ pci_region_migrate_64bit_entries(بنية pci_region *from,
                                             بنية pci_region *منزل)
{
    بنية hlist_گرہ *n, **آخری = &منزل->list.پہلا;
    بنية pci_region_اندراج *اندراج;
    hlist_for_each_اندراج_محفوظ(اندراج, n, &from->list, گرہ) {
        إذا (!اندراج->is64)
            استمر;
        إذا (اندراج->dev->class == PCI_CLASS_SERIAL_USB)
            استمر;
        إذا (اندراج->dev->class == PCI_CLASS_STORAGE_NVME)
            استمر;
        // Move from source list to destination list.
        hlist_del(&اندراج->گرہ);
        hlist_جوڑو(&اندراج->گرہ, آخری);
        آخری = &اندراج->گرہ.اگلا;
    }
}

ساكن بنية pci_region_اندراج *
pci_region_create_اندراج(بنية pci_bus *bus, بنية pci_device *dev,
                        صحيح bar, u64 ناپ, u64 align, صحيح type, صحيح is64)
{
    بنية pci_region_اندراج *اندراج = malloc_tmp(حجم(*اندراج));
    إذا (!اندراج) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(اندراج, 0, حجم(*اندراج));
    اندراج->dev = dev;
    اندراج->bar = bar;
    اندراج->ناپ = ناپ;
    اندراج->align = align;
    اندراج->is64 = is64;
    اندراج->type = type;
    // Insert into list in sorted order.
    بنية hlist_گرہ **pprev;
    بنية pci_region_اندراج *pos;
    hlist_for_each_اندراج_pprev(pos, pprev, &bus->r[type].list, گرہ) {
        إذا (pos->align < align || (pos->align == align && pos->ناپ < ناپ))
            اكسر;
    }
    hlist_جوڑو(&اندراج->گرہ, pprev);
    أرجع اندراج;
}

تعريفنوع تعداد hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

ساكن hotplug_type_t pci_bus_hotplug_support(بنية pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    إذا (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 پورٹ_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        إذا (پورٹ_type == PCI_EXP_TYPE_PCI_BRIDGE)
            اذهب check_shpc;

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
        u16 تھاں_implemented = pcie_flags & PCI_EXP_FLAGS_تھاں;

        أرجع downstream_پورٹ && تھاں_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_لبھو_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    أرجع !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
ساكن صحيح pci_bridge_has_region(بنية pci_device *pci,
                                 تعداد pci_region_type region_type)
{
    u8 ادھار;

    بدل (region_type) {
        حالة PCI_REGION_TYPE_IO:
            ادھار = PCI_IO_ادھار;
            اكسر;
        حالة PCI_REGION_TYPE_PREFMEM:
            ادھار = PCI_PREF_حافظہ_ادھار;
            اكسر;
        افتراضي:
            /* Regular memory support is mandatory */
            أرجع 1;
    }

    pci_config_writeb(pci->bdf, ادھار, 0xFF);

    أرجع pci_config_readb(pci->bdf, ادھار) != 0;
}

ساكن صحيح pci_bios_check_devices(بنية pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        بنية pci_bus *bus = &busses[pci_bdf_منزل_bus(pci->bdf)];
        إذا (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        صحيح i;
        لكل (i = 0; i < PCI_عدد_REGIONS; i++) {
            إذا ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_عدد_REGIONS && i < PCI_ROM_تھاں))
                استمر;
            صحيح type, is64;
            u64 ناپ;
            pci_bios_get_bar(pci, i, &type, &ناپ, &is64);
            إذا (ناپ == 0)
                استمر;

            إذا (type != PCI_REGION_TYPE_IO && ناپ < PCI_DEVICE_MEM_MIN)
                ناپ = PCI_DEVICE_MEM_MIN;
            بنية pci_region_اندراج *اندراج = pci_region_create_اندراج(
                bus, pci, i, ناپ, ناپ, type, is64);
            إذا (!اندراج)
                أرجع -1;

            إذا (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    صحيح secondary_bus;
    لكل (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        بنية pci_bus *s = &busses[secondary_bus];
        إذا (!s->bus_dev)
            استمر;
        بنية pci_bus *ماپا = &busses[pci_bdf_منزل_bus(s->bus_dev->bdf)];
        إذا (!ماپا->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            ماپا = &busses[0];
        صحيح type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_لبھو_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_لبھو_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        لكل (type = 0; type < PCI_REGION_TYPE_گنتی; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            إذا (!pci_bridge_has_region(s->bus_dev, type))
                استمر;
            u64 ناپ = 0;
            إذا (qemu_cap) {
                u32 tmp_ناپ;
                u64 tmp_ناپ_64;
                بدل(type) {
                حالة PCI_REGION_TYPE_IO:
                    tmp_ناپ_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    إذا (tmp_ناپ_64 != (u64)-1) {
                        ناپ = tmp_ناپ_64;
                    }
                    اكسر;
                حالة PCI_REGION_TYPE_MEM:
                    tmp_ناپ = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    إذا (tmp_ناپ != (u32)-1) {
                        ناپ = tmp_ناپ;
                    }
                    اكسر;
                حالة PCI_REGION_TYPE_PREFMEM:
                    tmp_ناپ = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_ناپ_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    إذا (tmp_ناپ != (u32)-1 && tmp_ناپ_64 == (u64)-1) {
                        ناپ = tmp_ناپ;
                    } وإلا إذا (tmp_ناپ == (u32)-1 && tmp_ناپ_64 != (u64)-1) {
                        ناپ = tmp_ناپ_64;
                    } وإلا إذا (tmp_ناپ != (u32)-1 && tmp_ناپ_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    اكسر;
                افتراضي:
                    اكسر;
                }
            }
            إذا (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 جوڑ = pci_region_جوڑ(&s->r[type]);
            صحيح is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            صحيح resource_optional = 0;
            إذا (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 چوٹی_align = pci_mem64_چوٹی >> 11;
            إذا (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (چوٹی_align > align))
                align = چوٹی_align;
            إذا (align > جوڑ && hotplug_support && !resource_optional)
                جوڑ = align; /* reserve min size for hot-plug */
            إذا (ناپ > جوڑ) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        ناپ, region_type_name[type]);
                إذا (type != PCI_REGION_TYPE_IO) {
                    ناپ = ALIGN(ناپ, align);
                }
            } وإلا {
                ناپ = ALIGN(جوڑ, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            بنية pci_region_اندراج *اندراج = pci_region_create_اندراج(
                ماپا, s->bus_dev, -1, ناپ, align, type, is64);
            إذا (!اندراج)
                أرجع -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      اندراج->dev->secondary_bus, ناپ,
                      region_type_name[اندراج->type]);
        }
    }
    أرجع 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
ساكن صحيح pci_bios_init_جڑ_regions_io(بنية pci_bus *bus)
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
    بنية pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 جوڑ = pci_region_جوڑ(r_io);
    إذا (جوڑ < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->ادھار = 0xc000;
    } وإلا إذا (جوڑ < pci_io_کم_سرا - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->ادھار = 0x1000;
    } وإلا {
        /* not enouth io address space -> error out */
        أرجع -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->ادھار, r_io->ادھار + جوڑ - 1);
    أرجع 0;
}

ساكن صحيح pci_bios_init_جڑ_regions_mem(بنية pci_bus *bus)
{
    بنية pci_region *r_سرا = &bus->r[PCI_REGION_TYPE_PREFMEM];
    بنية pci_region *r_شروع = &bus->r[PCI_REGION_TYPE_MEM];

    إذا (pci_region_align(r_شروع) < pci_region_align(r_سرا)) {
        // Swap regions to improve alignment.
        r_سرا = r_شروع;
        r_شروع = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 جوڑ = pci_region_جوڑ(r_سرا);
    u64 align = pci_region_align(r_سرا);
    r_سرا->ادھار = ALIGN_تھلے((pcimem_سرا - جوڑ), align);
    جوڑ = pci_region_جوڑ(r_شروع);
    align = pci_region_align(r_شروع);
    r_شروع->ادھار = ALIGN_تھلے((r_سرا->ادھار - جوڑ), align);

    إذا ((r_شروع->ادھار < pcimem_شروع) ||
         (r_شروع->ادھار > pcimem_سرا))
        // Memory range requested is larger than available.
        أرجع -1;
    أرجع 0;
}

#تعریف_کرو PCI_IO_کھسکاؤ            8
#تعریف_کرو PCI_حافظہ_کھسکاؤ        16
#تعریف_کرو PCI_PREF_حافظہ_کھسکاؤ   16

ساكن فراغ
pci_region_map_one_اندراج(بنية pci_region_اندراج *اندراج, u64 addr)
{
    إذا (اندراج->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                اندراج->dev,
                اندراج->bar, addr, اندراج->ناپ, region_type_name[اندراج->type]);

        pci_سیٹ_io_region_addr(اندراج->dev, اندراج->bar, addr, اندراج->is64);
        أرجع;
    }

    u16 bdf = اندراج->dev->bdf;
    u64 حد_بندی = addr + اندراج->ناپ - 1;
    إذا (اندراج->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_ادھار, addr >> PCI_IO_کھسکاؤ);
        pci_config_writew(bdf, PCI_IO_ادھار_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_حد_بندی, حد_بندی >> PCI_IO_کھسکاؤ);
        pci_config_writew(bdf, PCI_IO_حد_بندی_UPPER16, 0);
    }
    إذا (اندراج->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_حافظہ_ادھار, addr >> PCI_حافظہ_کھسکاؤ);
        pci_config_writew(bdf, PCI_حافظہ_حد_بندی, حد_بندی >> PCI_حافظہ_کھسکاؤ);
    }
    إذا (اندراج->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_حافظہ_ادھار, addr >> PCI_PREF_حافظہ_کھسکاؤ);
        pci_config_writew(bdf, PCI_PREF_حافظہ_حد_بندی, حد_بندی >> PCI_PREF_حافظہ_کھسکاؤ);
        pci_config_writel(bdf, PCI_PREF_ادھار_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_حد_بندی_UPPER32, حد_بندی >> 32);
    }
}

ساكن فراغ pci_region_map_entries(بنية pci_bus *busses, بنية pci_region *r)
{
    بنية hlist_گرہ *n;
    بنية pci_region_اندراج *اندراج;
    hlist_for_each_اندراج_محفوظ(اندراج, n, &r->list, گرہ) {
        u64 addr = r->ادھار;
        r->ادھار += اندراج->ناپ;
        إذا (اندراج->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[اندراج->dev->secondary_bus].r[اندراج->type].ادھار = addr;
        pci_region_map_one_اندراج(اندراج, addr);
        hlist_del(&اندراج->گرہ);
        حرر(اندراج);
    }
}

ساكن فراغ pci_bios_map_devices(بنية pci_bus *busses)
{
    إذا (pci_bios_init_جڑ_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_شروع, pcimem_سرا);
    إذا (pci_pad_mem64 || pci_bios_init_جڑ_regions_mem(busses)) {
        بنية pci_region r64_mem, r64_pref;
        r64_mem.list.پہلا = عدم;
        r64_pref.list.پہلا = عدم;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        إذا (pci_bios_init_جڑ_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 جوڑ_mem = pci_region_جوڑ(&r64_mem);
        u64 جوڑ_pref = pci_region_جوڑ(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.ادھار = le64_منزل_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        إذا (r64_mem.ادھار < 0x100000000LL + Ram_ناپ_Over4_G)
            r64_mem.ادھار = 0x100000000LL + Ram_ناپ_Over4_G;
        إذا (pci_mem64_چوٹی) {
            u64 ناپ = (ALIGN(جوڑ_mem, (1LL<<30)) +
                        ALIGN(جوڑ_pref, (1LL<<30)));
            إذا (pci_pad_mem64)
                ناپ = ALIGN(ناپ, pci_mem64_چوٹی >> 3);
            إذا (r64_mem.ادھار < pci_mem64_چوٹی - ناپ) {
                r64_mem.ادھار = pci_mem64_چوٹی - ناپ;
            }
            إذا (e820_is_ورتیا(r64_mem.ادھار, ناپ))
                r64_mem.ادھار -= ناپ;
        }
        r64_mem.ادھار = ALIGN(r64_mem.ادھار, align_mem);
        r64_mem.ادھار = ALIGN(r64_mem.ادھار, (1LL<<30));    // 1G hugepage
        r64_pref.ادھار = r64_mem.ادھار + جوڑ_mem;
        r64_pref.ادھار = ALIGN(r64_pref.ادھار, align_pref);
        r64_pref.ادھار = ALIGN(r64_pref.ادھار, (1LL<<30));  // 1G hugepage
        pcimem64_شروع = r64_mem.ادھار;
        pcimem64_سرا = r64_pref.ادھار + جوڑ_pref;
        pcimem64_سرا = ALIGN(pcimem64_سرا, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_شروع, pcimem64_سرا);

        pci_region_map_entries(busses, &r64_mem);
        pci_region_map_entries(busses, &r64_pref);
    } وإلا {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_شروع = 0;
    }
    // Map regions on each device.
    صحيح bus;
    لكل (bus = 0; bus<=MaxPCIBus; bus++) {
        صحيح type;
        لكل (type = 0; type < PCI_REGION_TYPE_گنتی; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

فراغ
pci_تیاری(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    dprintf(3, "pci setup\n");

    إذا (CPUPhys_بٹ) {
        pci_mem64_چوٹی = 1LL << CPUPhys_بٹ;
        إذا (CPUPhys_بٹ > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            إذا (Ram_ناپ_Over4_G < (1LL << 40)) {
                pci_mem64_چوٹی = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    إذا (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_ناپ_Over4_G ? 1 : 0;

    إذا (!CPULong_منوال)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    إذا (pci_probe_host() != 0) {
        أرجع;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_شروع = Ram_ناپ;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    بنية pci_bus *busses = malloc_tmp(حجم(*busses) * (MaxPCIBus + 1));
    إذا (!busses) {
        warn_noalloc();
        أرجع;
    }
    memset(busses, 0, حجم(*busses) * (MaxPCIBus + 1));
    إذا (pci_bios_check_devices(busses))
        أرجع;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    حرر(busses);

    pci_enable_default_vga();
}
