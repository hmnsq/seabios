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

#تعریف_کن PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#تعریف_کن PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#تعریف_کن PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#تعریف_کن PCI_ROM_جایگاه 6
#تعریف_کن PCI_رقم_REGIONS 7
#تعریف_کن PCI_BRIDGE_رقم_REGIONS 2

تعداد pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_شمار,
};

ساكن ثابت حرف *region_type_name[] = {
    [ PCI_REGION_TYPE_IO ]      = "io",
    [ PCI_REGION_TYPE_MEM ]     = "mem",
    [ PCI_REGION_TYPE_PREFMEM ] = "prefmem",
};

// Memory ranges exported to legacy ACPI type table generation
u64 pcimem_شروع   = BUILD_PCIMEM_شروع;
u64 pcimem_انتها     = BUILD_PCIMEM_انتها;
u64 pcimem64_شروع = BUILD_PCIMEM64_شروع;
u64 pcimem64_انتها   = BUILD_PCIMEM64_انتها;

// Resource allocation limits
ساكن u64 pci_io_کم_انتها = 0xa000;
ساكن u64 pci_mem64_بالاترین  = 0;
ساكن u32 pci_pad_mem64  = -1;

بنية pci_region_مدخل {
    بنية pci_device *dev;
    صحيح bar;
    u64 اندازه;
    u64 align;
    صحيح is64;
    تعداد pci_region_type type;
    بنية hlist_گره گره;
};

بنية pci_region {
    /* pci region assignments */
    u64 پایه;
    بنية hlist_سر list;
};

بنية pci_bus {
    بنية pci_region r[PCI_REGION_TYPE_شمار];
    بنية pci_device *bus_dev;
};

ساكن u32 pci_bar(بنية pci_device *pci, صحيح region_رقم)
{
    إذا (region_رقم != PCI_ROM_جایگاه) {
        أرجع PCI_پایه_آدرس_0 + region_رقم * 4;
    }

#تعریف_کن PCI_سرآیند_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->سرآیند_type & ~PCI_سرآیند_TYPE_MULTI_FUNCTION;
    أرجع type == PCI_سرآیند_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_آدرس;
}

ساكن فراغ
pci_مجموعه_io_region_addr(بنية pci_device *pci, صحيح bar, u64 addr, صحيح is64)
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

ساكن صحيح dummy_pci_جایگاه_get_irq(بنية pci_device *pci, صحيح pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    أرجع 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

ساكن صحيح (*pci_جایگاه_get_irq)(بنية pci_device *pci, صحيح pin) =
    dummy_pci_جایگاه_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
ساكن صحيح piix_pci_جایگاه_get_irq(بنية pci_device *pci, صحيح pin)
{
    صحيح جایگاه_addend = 0;

    طالما (pci->والد != عدم) {
        جایگاه_addend += pci_bdf_مقصد_dev(pci->bdf);
        pci = pci->والد;
    }
    جایگاه_addend += pci_bdf_مقصد_dev(pci->bdf) - 1;
    أرجع pci_irqs[(pin - 1 + جایگاه_addend) & 3];
}

ساكن صحيح mch_pci_جایگاه_get_irq(بنية pci_device *pci, صحيح pin)
{
    صحيح pin_addend = 0;
    طالما (pci->والد != عدم) {
        pin_addend += pci_bdf_مقصد_dev(pci->bdf);
        pci = pci->والد;
    }
    u8 جایگاه = pci_bdf_مقصد_dev(pci->bdf);
    إذا (جایگاه <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        أرجع pci_irqs[(pin - 1 + pin_addend + جایگاه) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    أرجع pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
ساكن فراغ piix_isa_bridge_راهاندازی(بنية pci_device *pci, فراغ *arg)
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
    outb(elcr[0], PIIX_پورت_ELCR1);
    outb(elcr[1], PIIX_پورت_ELCR2);
    dprintf(1, "PIIX3/PIIX4 init: elcr=%02x %02x\n", elcr[0], elcr[1]);
}

ساكن فراغ mch_isa_lpc_راهاندازی(u16 bdf)
{
    /* pm io base */
    pci_config_writel(bdf, ICH9_LPC_PMBASE,
                      acpi_pm_پایه | ICH9_LPC_PMBASE_RTE);

    /* acpi enable, SCI: IRQ9 000b = irq9*/
    pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, ICH9_LPC_ACPI_CTRL_ACPI_EN);

    /* set root complex register block BAR */
    pci_config_writel(bdf, ICH9_LPC_RCBA,
                      ICH9_LPC_RCBA_ADDR | ICH9_LPC_RCBA_EN);
}

ساكن صحيح ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
ساكن فراغ mch_isa_bridge_راهاندازی(بنية pci_device *dev, فراغ *arg)
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
    outb(elcr[0], ICH9_LPC_پورت_ELCR1);
    outb(elcr[1], ICH9_LPC_پورت_ELCR2);
    dprintf(1, "Q35 LPC init: elcr=%02x %02x\n", elcr[0], elcr[1]);

    ICH9LpcBDF = bdf;

    mch_isa_lpc_راهاندازی(bdf);

    e820_جمع_کن(ICH9_LPC_RCBA_ADDR, 16*1024, E820_RESERVED);

    acpi_pm1a_cnt = acpi_pm_پایه + 0x04;
    pmtimer_راهاندازی(acpi_pm_پایه + 0x08);
}

ساكن فراغ storage_ide_راهاندازی(بنية pci_device *pci, فراغ *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_مجموعه_io_region_addr(pci, 0, پورت_ATA1_CMD_پایه, 0);
    pci_مجموعه_io_region_addr(pci, 1, پورت_ATA1_CTRL_پایه, 0);
    pci_مجموعه_io_region_addr(pci, 2, پورت_ATA2_CMD_پایه, 0);
    pci_مجموعه_io_region_addr(pci, 3, پورت_ATA2_CTRL_پایه, 0);
}

/* PIIX3/PIIX4 IDE */
ساكن فراغ piix_ide_راهاندازی(بنية pci_device *pci, فراغ *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

ساكن فراغ pic_ibm_راهاندازی(بنية pci_device *pci, فراغ *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_مجموعه_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

ساكن فراغ apple_macio_راهاندازی(بنية pci_device *pci, فراغ *arg)
{
    /* macio bridge */
    pci_مجموعه_io_region_addr(pci, 0, 0x80800000, 0);
}

ساكن فراغ piix4_pm_config_راهاندازی(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_وقفه_سطر, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_پایه | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_پایه + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

ساكن صحيح PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
ساكن فراغ piix4_pm_راهاندازی(بنية pci_device *pci, فراغ *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_راهاندازی(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_پایه + 0x04;
    pmtimer_راهاندازی(acpi_pm_پایه + 0x08);
}

ساكن فراغ ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_پایه,
                      (acpi_pm_پایه + 0x100) | PCI_پایه_آدرس_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

ساكن صحيح ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
ساكن فراغ ich9_smbus_راهاندازی(بنية pci_device *dev, فراغ *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

ساكن فراغ intel_igd_راهاندازی(بنية pci_device *dev, فراغ *arg)
{
    بنية romfile_s *opregion = romfile_پیدا_کن("etc/igd-opregion");
    u64 bdsm_اندازه = le64_مقصد_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    إذا (opregion && opregion->اندازه) {
        فراغ *addr = memalign_بالا(صفحه_اندازه, opregion->اندازه);
        إذا (!addr) {
            warn_noalloc();
            أرجع;
        }

        إذا (opregion->copy(opregion, addr, opregion->اندازه) < 0) {
            حرر(addr);
            أرجع;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_مقصد_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->اندازه >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    إذا (bdsm_اندازه && (dev->bdf == pci_مقصد_bdf(0, 2, 0))) {
        فراغ *addr = memalign_tmphigh(1024 * 1024, bdsm_اندازه);
        إذا (!addr) {
            warn_noalloc();
            أرجع;
        }

        e820_جمع_کن((u32)addr, bdsm_اندازه, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_مقصد_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_اندازه >> 20, dev);
    }
}

ساكن ثابت بنية pci_device_id pci_device_tbl[] = {
    /* PIIX3/PIIX4 PCI to ISA bridge */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0,
               piix_isa_bridge_راهاندازی),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0,
               piix_isa_bridge_راهاندازی),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_LPC,
               mch_isa_bridge_راهاندازی),

    /* STORAGE IDE */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1,
                     PCI_CLASS_STORAGE_IDE, piix_ide_راهاندازی),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB,
                     PCI_CLASS_STORAGE_IDE, piix_ide_راهاندازی),
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
                     storage_ide_راهاندازی),

    /* PIC, IBM, MPIC & MPIC2 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0x0046, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_راهاندازی),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_IBM, 0xFFFF, PCI_CLASS_SYSTEM_PIC,
                     pic_ibm_راهاندازی),

    /* PIIX4 Power Management device (for ACPI) */
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_3,
               piix4_pm_راهاندازی),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_SMBUS,
               ich9_smbus_راهاندازی),

    /* 0xff00 */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0017, 0xff00, apple_macio_راهاندازی),
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_APPLE, 0x0022, 0xff00, apple_macio_راهاندازی),

    /* Intel IGD OpRegion setup */
    PCI_DEVICE_CLASS(PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
                     intel_igd_راهاندازی),

    PCI_DEVICE_انتها,
};

ساكن صحيح MCHMmcfgBDF = -1;
ساكن فراغ mch_mmconfig_راهاندازی(u16 bdf);

فراغ pci_resume(فراغ)
{
    إذا (!CONFIG_QEMU) {
        أرجع;
    }

    إذا (PiixPmBDF >= 0) {
        piix4_pm_config_راهاندازی(PiixPmBDF);
    }

    إذا (ICH9LpcBDF >= 0) {
        mch_isa_lpc_راهاندازی(ICH9LpcBDF);
    }

    إذا (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    إذا(MCHMmcfgBDF >= 0) {
        mch_mmconfig_راهاندازی(MCHMmcfgBDF);
    }
}

ساكن فراغ pci_bios_init_device(بنية pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    صحيح pin = pci_config_readb(bdf, PCI_وقفه_PIN);
    إذا (pin != 0)
        pci_config_writeb(bdf, PCI_وقفه_سطر, pci_جایگاه_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, عدم);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_حافظه | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    إذا (pci->سرآیند_type & PCI_سرآیند_TYPE_BRIDGE)
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

    pci = pci_پیدا_کن_class(PCI_CLASS_DISPLAY_VGA);
    إذا (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        أرجع;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_حافظه);

    طالما (pci->والد) {
        pci = pci->والد;

        dprintf(1, "PCI: Setting VGA enable on bridge %pP\n", pci);

        pci_config_maskw(pci->bdf, PCI_BRIDGE_CONTROL, 0, PCI_BRIDGE_CTL_VGA);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_حافظه);
    }
}

/****************************************************************
 * Platform device initialization
 ****************************************************************/

ساكن فراغ i440fx_mem_addr_راهاندازی(بنية pci_device *dev, فراغ *arg)
{
    إذا (Ram_اندازه <= 0x80000000)
        pcimem_شروع = 0x80000000;
    وإلا إذا (Ram_اندازه <= 0xc0000000)
        pcimem_شروع = 0xc0000000;

    pci_جایگاه_get_irq = piix_pci_جایگاه_get_irq;
}

ساكن فراغ mch_mmconfig_راهاندازی(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 فوقانی = addr >> 32;
    u32 پایینی = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, فوقانی);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, پایینی);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

ساكن فراغ mch_mem_addr_راهاندازی(بنية pci_device *dev, فراغ *arg)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 اندازه = Q35_HOST_BRIDGE_PCIEXBAR_اندازه;

    /* setup mmconfig */
    MCHMmcfgBDF = dev->bdf;
    mch_mmconfig_راهاندازی(dev->bdf);
    e820_جمع_کن(addr, اندازه, E820_RESERVED);

    /* setup pci i/o window (above mmconfig) */
    pcimem_شروع = addr + اندازه;

    pci_جایگاه_get_irq = mch_pci_جایگاه_get_irq;

    /* setup io address space */
    إذا (acpi_pm_پایه < 0x1000)
        pci_io_کم_انتها = 0x10000;
    وإلا
        pci_io_کم_انتها = acpi_pm_پایه;
}

ساكن ثابت بنية pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_راهاندازی),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_راهاندازی),
    PCI_DEVICE_انتها
};

ساكن فراغ pci_bios_init_platform(فراغ)
{
    بنية pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, عدم);
    }
}

ساكن u8 pci_پیدا_کن_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    إذا (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        أرجع 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    إذا (device_id != PCI_DEVICE_ID_REDHAT_ریشه_پورت &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        أرجع 0;
    }
    u8 cap = 0;

    افعل {
        cap = pci_پیدا_کن_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } طالما (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_جابه_جایی) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    إذا (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        إذا (cap_len < RES_RESERVE_CAP_اندازه) {
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
            u8 cap = pci_پیدا_کن_resource_reserve_capability(bdf);

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
    u64 ماسک;

    إذا (bar == PCI_ROM_جایگاه) {
        ماسک = PCI_ROM_آدرس_ماسک;
        pci_config_writel(bdf, ofs, ماسک);
    } وإلا {
        إذا (old & PCI_پایه_آدرس_SPACE_IO) {
            ماسک = PCI_پایه_آدرس_IO_ماسک;
            type = PCI_REGION_TYPE_IO;
        } وإلا {
            ماسک = PCI_پایه_آدرس_MEM_ماسک;
            إذا (old & PCI_پایه_آدرس_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_پایه_آدرس_MEM_TYPE_ماسک)
                    == PCI_پایه_آدرس_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    إذا (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 بالا = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)بالا << 32);
        ماسک |= ((u64)0xffffffff << 32);
        *psize = (~(val & ماسک)) + 1;
    } وإلا {
        *psize = ((~(val & ماسک)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

ساكن صحيح pci_bios_bridge_region_is64(بنية pci_region *r,
                                 بنية pci_device *pci, صحيح type)
{
    إذا (type != PCI_REGION_TYPE_PREFMEM)
        أرجع 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_حافظه_پایه);
    إذا (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_حافظه_پایه, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_حافظه_پایه);
        pci_config_writel(pci->bdf, PCI_PREF_حافظه_پایه, 0x0);
    }
    إذا ((pmem & PCI_PREF_RANGE_TYPE_ماسک) != PCI_PREF_RANGE_TYPE_64)
       أرجع 0;
    بنية pci_region_مدخل *مدخل;
    hlist_for_each_مدخل(مدخل, &r->list, گره) {
        إذا (!مدخل->is64)
            أرجع 0;
    }
    أرجع 1;
}

ساكن u64 pci_region_align(بنية pci_region *r)
{
    بنية pci_region_مدخل *مدخل;
    hlist_for_each_مدخل(مدخل, &r->list, گره) {
        // The first entry in the sorted list has the largest alignment
        أرجع مدخل->align;
    }
    أرجع 1;
}

ساكن u64 pci_region_جمع(بنية pci_region *r)
{
    u64 جمع = 0;
    بنية pci_region_مدخل *مدخل;
    hlist_for_each_مدخل(مدخل, &r->list, گره) {
        جمع += مدخل->اندازه;
    }
    أرجع جمع;
}

ساكن فراغ pci_region_migrate_64bit_entries(بنية pci_region *from,
                                             بنية pci_region *مقصد)
{
    بنية hlist_گره *n, **آخرین = &مقصد->list.اولی;
    بنية pci_region_مدخل *مدخل;
    hlist_for_each_مدخل_امن(مدخل, n, &from->list, گره) {
        إذا (!مدخل->is64)
            استمر;
        إذا (مدخل->dev->class == PCI_CLASS_SERIAL_USB)
            استمر;
        إذا (مدخل->dev->class == PCI_CLASS_STORAGE_NVME)
            استمر;
        // Move from source list to destination list.
        hlist_del(&مدخل->گره);
        hlist_جمع_کن(&مدخل->گره, آخرین);
        آخرین = &مدخل->گره.بعدی;
    }
}

ساكن بنية pci_region_مدخل *
pci_region_create_مدخل(بنية pci_bus *bus, بنية pci_device *dev,
                        صحيح bar, u64 اندازه, u64 align, صحيح type, صحيح is64)
{
    بنية pci_region_مدخل *مدخل = malloc_tmp(حجم(*مدخل));
    إذا (!مدخل) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(مدخل, 0, حجم(*مدخل));
    مدخل->dev = dev;
    مدخل->bar = bar;
    مدخل->اندازه = اندازه;
    مدخل->align = align;
    مدخل->is64 = is64;
    مدخل->type = type;
    // Insert into list in sorted order.
    بنية hlist_گره **pprev;
    بنية pci_region_مدخل *pos;
    hlist_for_each_مدخل_pprev(pos, pprev, &bus->r[type].list, گره) {
        إذا (pos->align < align || (pos->align == align && pos->اندازه < اندازه))
            اكسر;
    }
    hlist_جمع_کن(&مدخل->گره, pprev);
    أرجع مدخل;
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
        u8 پورت_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        إذا (پورت_type == PCI_EXP_TYPE_PCI_BRIDGE)
            اذهب check_shpc;

        u8 downstream_پورت = (پورت_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (پورت_type == PCI_EXP_TYPE_ریشه_پورت);
        /*
         * PCI Express SPEC, 7.8.2:
         *   Slot Implemented – When Set, this bit indicates that the Link
         *   HwInit associated with this Port is connected to a slot (as
         *   compared to being connected to a system-integrated device or
         *   being disabled).
         *   This bit is valid for Downstream Ports. This bit is undefined
         *   for Upstream Ports.
         */
        u16 جایگاه_implemented = pcie_flags & PCI_EXP_FLAGS_جایگاه;

        أرجع downstream_پورت && جایگاه_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_پیدا_کن_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    أرجع !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
ساكن صحيح pci_bridge_has_region(بنية pci_device *pci,
                                 تعداد pci_region_type region_type)
{
    u8 پایه;

    بدل (region_type) {
        حالة PCI_REGION_TYPE_IO:
            پایه = PCI_IO_پایه;
            اكسر;
        حالة PCI_REGION_TYPE_PREFMEM:
            پایه = PCI_PREF_حافظه_پایه;
            اكسر;
        افتراضي:
            /* Regular memory support is mandatory */
            أرجع 1;
    }

    pci_config_writeb(pci->bdf, پایه, 0xFF);

    أرجع pci_config_readb(pci->bdf, پایه) != 0;
}

ساكن صحيح pci_bios_check_devices(بنية pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        بنية pci_bus *bus = &busses[pci_bdf_مقصد_bus(pci->bdf)];
        إذا (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        صحيح i;
        لكل (i = 0; i < PCI_رقم_REGIONS; i++) {
            إذا ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_رقم_REGIONS && i < PCI_ROM_جایگاه))
                استمر;
            صحيح type, is64;
            u64 اندازه;
            pci_bios_get_bar(pci, i, &type, &اندازه, &is64);
            إذا (اندازه == 0)
                استمر;

            إذا (type != PCI_REGION_TYPE_IO && اندازه < PCI_DEVICE_MEM_MIN)
                اندازه = PCI_DEVICE_MEM_MIN;
            بنية pci_region_مدخل *مدخل = pci_region_create_مدخل(
                bus, pci, i, اندازه, اندازه, type, is64);
            إذا (!مدخل)
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
        بنية pci_bus *والد = &busses[pci_bdf_مقصد_bus(s->bus_dev->bdf)];
        إذا (!والد->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            والد = &busses[0];
        صحيح type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_پیدا_کن_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_پیدا_کن_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        لكل (type = 0; type < PCI_REGION_TYPE_شمار; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            إذا (!pci_bridge_has_region(s->bus_dev, type))
                استمر;
            u64 اندازه = 0;
            إذا (qemu_cap) {
                u32 tmp_اندازه;
                u64 tmp_اندازه_64;
                بدل(type) {
                حالة PCI_REGION_TYPE_IO:
                    tmp_اندازه_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    إذا (tmp_اندازه_64 != (u64)-1) {
                        اندازه = tmp_اندازه_64;
                    }
                    اكسر;
                حالة PCI_REGION_TYPE_MEM:
                    tmp_اندازه = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    إذا (tmp_اندازه != (u32)-1) {
                        اندازه = tmp_اندازه;
                    }
                    اكسر;
                حالة PCI_REGION_TYPE_PREFMEM:
                    tmp_اندازه = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_اندازه_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    إذا (tmp_اندازه != (u32)-1 && tmp_اندازه_64 == (u64)-1) {
                        اندازه = tmp_اندازه;
                    } وإلا إذا (tmp_اندازه == (u32)-1 && tmp_اندازه_64 != (u64)-1) {
                        اندازه = tmp_اندازه_64;
                    } وإلا إذا (tmp_اندازه != (u32)-1 && tmp_اندازه_64 != (u64)-1) {
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
            u64 جمع = pci_region_جمع(&s->r[type]);
            صحيح is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            صحيح resource_optional = 0;
            إذا (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 بالاترین_align = pci_mem64_بالاترین >> 11;
            إذا (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (بالاترین_align > align))
                align = بالاترین_align;
            إذا (align > جمع && hotplug_support && !resource_optional)
                جمع = align; /* reserve min size for hot-plug */
            إذا (اندازه > جمع) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        اندازه, region_type_name[type]);
                إذا (type != PCI_REGION_TYPE_IO) {
                    اندازه = ALIGN(اندازه, align);
                }
            } وإلا {
                اندازه = ALIGN(جمع, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            بنية pci_region_مدخل *مدخل = pci_region_create_مدخل(
                والد, s->bus_dev, -1, اندازه, align, type, is64);
            إذا (!مدخل)
                أرجع -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      مدخل->dev->secondary_bus, اندازه,
                      region_type_name[مدخل->type]);
        }
    }
    أرجع 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
ساكن صحيح pci_bios_init_ریشه_regions_io(بنية pci_bus *bus)
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
    u64 جمع = pci_region_جمع(r_io);
    إذا (جمع < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->پایه = 0xc000;
    } وإلا إذا (جمع < pci_io_کم_انتها - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->پایه = 0x1000;
    } وإلا {
        /* not enouth io address space -> error out */
        أرجع -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->پایه, r_io->پایه + جمع - 1);
    أرجع 0;
}

ساكن صحيح pci_bios_init_ریشه_regions_mem(بنية pci_bus *bus)
{
    بنية pci_region *r_انتها = &bus->r[PCI_REGION_TYPE_PREFMEM];
    بنية pci_region *r_شروع = &bus->r[PCI_REGION_TYPE_MEM];

    إذا (pci_region_align(r_شروع) < pci_region_align(r_انتها)) {
        // Swap regions to improve alignment.
        r_انتها = r_شروع;
        r_شروع = &bus->r[PCI_REGION_TYPE_PREFMEM];
    }
    u64 جمع = pci_region_جمع(r_انتها);
    u64 align = pci_region_align(r_انتها);
    r_انتها->پایه = ALIGN_پایین((pcimem_انتها - جمع), align);
    جمع = pci_region_جمع(r_شروع);
    align = pci_region_align(r_شروع);
    r_شروع->پایه = ALIGN_پایین((r_انتها->پایه - جمع), align);

    إذا ((r_شروع->پایه < pcimem_شروع) ||
         (r_شروع->پایه > pcimem_انتها))
        // Memory range requested is larger than available.
        أرجع -1;
    أرجع 0;
}

#تعریف_کن PCI_IO_لغزش            8
#تعریف_کن PCI_حافظه_لغزش        16
#تعریف_کن PCI_PREF_حافظه_لغزش   16

ساكن فراغ
pci_region_map_one_مدخل(بنية pci_region_مدخل *مدخل, u64 addr)
{
    إذا (مدخل->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                مدخل->dev,
                مدخل->bar, addr, مدخل->اندازه, region_type_name[مدخل->type]);

        pci_مجموعه_io_region_addr(مدخل->dev, مدخل->bar, addr, مدخل->is64);
        أرجع;
    }

    u16 bdf = مدخل->dev->bdf;
    u64 حد = addr + مدخل->اندازه - 1;
    إذا (مدخل->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_پایه, addr >> PCI_IO_لغزش);
        pci_config_writew(bdf, PCI_IO_پایه_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_حد, حد >> PCI_IO_لغزش);
        pci_config_writew(bdf, PCI_IO_حد_UPPER16, 0);
    }
    إذا (مدخل->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_حافظه_پایه, addr >> PCI_حافظه_لغزش);
        pci_config_writew(bdf, PCI_حافظه_حد, حد >> PCI_حافظه_لغزش);
    }
    إذا (مدخل->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_حافظه_پایه, addr >> PCI_PREF_حافظه_لغزش);
        pci_config_writew(bdf, PCI_PREF_حافظه_حد, حد >> PCI_PREF_حافظه_لغزش);
        pci_config_writel(bdf, PCI_PREF_پایه_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_حد_UPPER32, حد >> 32);
    }
}

ساكن فراغ pci_region_map_entries(بنية pci_bus *busses, بنية pci_region *r)
{
    بنية hlist_گره *n;
    بنية pci_region_مدخل *مدخل;
    hlist_for_each_مدخل_امن(مدخل, n, &r->list, گره) {
        u64 addr = r->پایه;
        r->پایه += مدخل->اندازه;
        إذا (مدخل->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[مدخل->dev->secondary_bus].r[مدخل->type].پایه = addr;
        pci_region_map_one_مدخل(مدخل, addr);
        hlist_del(&مدخل->گره);
        حرر(مدخل);
    }
}

ساكن فراغ pci_bios_map_devices(بنية pci_bus *busses)
{
    إذا (pci_bios_init_ریشه_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_شروع, pcimem_انتها);
    إذا (pci_pad_mem64 || pci_bios_init_ریشه_regions_mem(busses)) {
        بنية pci_region r64_mem, r64_pref;
        r64_mem.list.اولی = عدم;
        r64_pref.list.اولی = عدم;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        إذا (pci_bios_init_ریشه_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 جمع_mem = pci_region_جمع(&r64_mem);
        u64 جمع_pref = pci_region_جمع(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.پایه = le64_مقصد_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        إذا (r64_mem.پایه < 0x100000000LL + Ram_اندازه_Over4_G)
            r64_mem.پایه = 0x100000000LL + Ram_اندازه_Over4_G;
        إذا (pci_mem64_بالاترین) {
            u64 اندازه = (ALIGN(جمع_mem, (1LL<<30)) +
                        ALIGN(جمع_pref, (1LL<<30)));
            إذا (pci_pad_mem64)
                اندازه = ALIGN(اندازه, pci_mem64_بالاترین >> 3);
            إذا (r64_mem.پایه < pci_mem64_بالاترین - اندازه) {
                r64_mem.پایه = pci_mem64_بالاترین - اندازه;
            }
            إذا (e820_is_استفاده_شده(r64_mem.پایه, اندازه))
                r64_mem.پایه -= اندازه;
        }
        r64_mem.پایه = ALIGN(r64_mem.پایه, align_mem);
        r64_mem.پایه = ALIGN(r64_mem.پایه, (1LL<<30));    // 1G hugepage
        r64_pref.پایه = r64_mem.پایه + جمع_mem;
        r64_pref.پایه = ALIGN(r64_pref.پایه, align_pref);
        r64_pref.پایه = ALIGN(r64_pref.پایه, (1LL<<30));  // 1G hugepage
        pcimem64_شروع = r64_mem.پایه;
        pcimem64_انتها = r64_pref.پایه + جمع_pref;
        pcimem64_انتها = ALIGN(pcimem64_انتها, (1LL<<30));    // 1G hugepage
        dprintf(1, "PCI: 64: %016llx - %016llx\n", pcimem64_شروع, pcimem64_انتها);

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
        لكل (type = 0; type < PCI_REGION_TYPE_شمار; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

فراغ
pci_راهاندازی(فراغ)
{
    إذا (!CONFIG_QEMU)
        أرجع;

    dprintf(3, "pci setup\n");

    إذا (CPUPhys_بیتها) {
        pci_mem64_بالاترین = 1LL << CPUPhys_بیتها;
        إذا (CPUPhys_بیتها > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            إذا (Ram_اندازه_Over4_G < (1LL << 40)) {
                pci_mem64_بالاترین = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    إذا (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = Ram_اندازه_Over4_G ? 1 : 0;

    إذا (!CPULong_نما)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    إذا (pci_probe_host() != 0) {
        أرجع;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_شروع = Ram_اندازه;
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
