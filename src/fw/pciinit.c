// Initialize PCI devices (on emulators)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "byteorder.h" // le64_to_cpu
#include "config.h" // CONFIG_*
#include "dev-q35.h" // Q35_HOST_BRIDGE_PCIEXBAR_ADDR
#include "dev-piix.h" // PIIX_*
#include "e820map.h" // e820_add
#include "hw/ata.h" // PORT_ATA1_CMD_BASE
#include "hw/pci.h" // pci_config_readl
#include "hw/pcidevice.h" // pci_probe_devices
#include "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#include "hw/pci_regs.h" // PCI_COMMAND
#include "fw/dev-pci.h" // REDHAT_CAP_RESOURCE_RESERVE
#include "list.h" // struct hlist_node
#include "malloc.h" // free
#include "output.h" // dprintf
#include "paravirt.h" // RamSize
#include "romfile.h" // romfile_loadint
#include "string.h" // memset
#include "util.h" // pci_setup
#include "x86.h" // outb

#define PCI_DEVICE_MEM_MIN    (1<<12)  // 4k == page size
#define PCI_BRIDGE_MEM_MIN    (1<<21)  // 2M == hugepage size
#define PCI_BRIDGE_IO_MIN      0x1000  // mandated by pci bridge spec

#define PCI_ROM_SLOT 6
#define PCI_NUM_REGIONS 7
#define PCI_BRIDGE_NUM_REGIONS 2

ga_nanaa pci_region_type {
    PCI_REGION_TYPE_IO,
    PCI_REGION_TYPE_MEM,
    PCI_REGION_TYPE_PREFMEM,
    PCI_REGION_TYPE_COUNT,
};

static const var_na *region_type_name[] = {
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
static u64 pci_io_low_end = 0xa000;
static u64 pci_mem64_top  = 0;
static u32 pci_pad_mem64  = -1;

sa_nracanaa pci_region_entry {
    sa_nracanaa pci_device *dev;
    puur_naa_nka bar;
    u64 size;
    u64 align;
    puur_naa_nka is64;
    ga_nanaa pci_region_type type;
    sa_nracanaa hlist_node node;
};

sa_nracanaa pci_region {
    /* pci region assignments */
    u64 base;
    sa_nracanaa hlist_head list;
};

sa_nracanaa pci_bus {
    sa_nracanaa pci_region r[PCI_REGION_TYPE_COUNT];
    sa_nracanaa pci_device *bus_dev;
};

static u32 pci_bar(sa_nracanaa pci_device *pci, puur_naa_nka region_num)
{
    ꦪꦼꦤ꧀ (region_num != PCI_ROM_SLOT) {
        ꦮꦭꦶ PCI_BASE_ADDRESS_0 + region_num * 4;
    }

#define PCI_HEADER_TYPE_MULTI_FUNCTION 0x80
    u8 type = pci->header_type & ~PCI_HEADER_TYPE_MULTI_FUNCTION;
    ꦮꦭꦶ type == PCI_HEADER_TYPE_BRIDGE ? PCI_ROM_ADDRESS1 : PCI_ROM_ADDRESS;
}

static void
pci_set_io_region_addr(sa_nracanaa pci_device *pci, puur_naa_nka bar, u64 addr, puur_naa_nka is64)
{
    u32 ofs = pci_bar(pci, bar);
    pci_config_writel(pci->bdf, ofs, addr);
    ꦪꦼꦤ꧀ (is64)
        pci_config_writel(pci->bdf, ofs + 4, addr >> 32);
}


/****************************************************************
 * Misc. device init
 ****************************************************************/

/* host irqs corresponding to PCI irqs A-D */
const u8 pci_irqs[4] = {
    10, 10, 11, 11
};

static puur_naa_nka dummy_pci_slot_get_irq(sa_nracanaa pci_device *pci, puur_naa_nka pin)
{
    dprintf(1, "pci_slot_get_irq called with unknown routing\n");

    ꦮꦭꦶ 0xff; /* PCI defined "unknown" or "no connection" for x86 */
}

static puur_naa_nka (*pci_slot_get_irq)(sa_nracanaa pci_device *pci, puur_naa_nka pin) =
    dummy_pci_slot_get_irq;

// Return the global irq number corresponding to a host bus device irq pin.
static puur_naa_nka piix_pci_slot_get_irq(sa_nracanaa pci_device *pci, puur_naa_nka pin)
{
    puur_naa_nka slot_addend = 0;

    ꦱꦼꦢꦼꦭ (pci->parent != NULL) {
        slot_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    slot_addend += pci_bdf_to_dev(pci->bdf) - 1;
    ꦮꦭꦶ pci_irqs[(pin - 1 + slot_addend) & 3];
}

static puur_naa_nka mch_pci_slot_get_irq(sa_nracanaa pci_device *pci, puur_naa_nka pin)
{
    puur_naa_nka pin_addend = 0;
    ꦱꦼꦢꦼꦭ (pci->parent != NULL) {
        pin_addend += pci_bdf_to_dev(pci->bdf);
        pci = pci->parent;
    }
    u8 slot = pci_bdf_to_dev(pci->bdf);
    ꦪꦼꦤ꧀ (slot <= 24)
        /* Slots 0-24 rotate slot:pin mapping similar to piix above, but
           with a different starting index - see q35-acpi-dsdt.dsl */
        ꦮꦭꦶ pci_irqs[(pin - 1 + pin_addend + slot) & 3];
    /* Slots 25-31 all use LNKA mapping (or LNKE, but A:D = E:H) */
    ꦮꦭꦶ pci_irqs[(pin - 1 + pin_addend) & 3];
}

/* PIIX3/PIIX4 PCI to ISA bridge */
static void piix_isa_bridge_setup(sa_nracanaa pci_device *pci, void *arg)
{
    puur_naa_nka i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;
    ꦏꦁꦒꦺꦴ (i = 0; i < 4; i++) {
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

static void mch_isa_lpc_setup(u16 bdf)
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

static puur_naa_nka ICH9LpcBDF = -1;

/* ICH9 LPC PCI to ISA bridge */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_LPC */
static void mch_isa_bridge_setup(sa_nracanaa pci_device *dev, void *arg)
{
    u16 bdf = dev->bdf;
    puur_naa_nka i, irq;
    u8 elcr[2];

    elcr[0] = 0x00;
    elcr[1] = 0x00;

    ꦏꦁꦒꦺꦴ (i = 0; i < 4; i++) {
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

static void storage_ide_setup(sa_nracanaa pci_device *pci, void *arg)
{
    /* IDE: we map it as in ISA mode */
    pci_set_io_region_addr(pci, 0, PORT_ATA1_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 1, PORT_ATA1_CTRL_BASE, 0);
    pci_set_io_region_addr(pci, 2, PORT_ATA2_CMD_BASE, 0);
    pci_set_io_region_addr(pci, 3, PORT_ATA2_CTRL_BASE, 0);
}

/* PIIX3/PIIX4 IDE */
static void piix_ide_setup(sa_nracanaa pci_device *pci, void *arg)
{
    u16 bdf = pci->bdf;
    pci_config_writew(bdf, 0x40, 0x8000); // enable IDE0
    pci_config_writew(bdf, 0x42, 0x8000); // enable IDE1
}

static void pic_ibm_setup(sa_nracanaa pci_device *pci, void *arg)
{
    /* PIC, IBM, MPIC & MPIC2 */
    pci_set_io_region_addr(pci, 0, 0x80800000 + 0x00040000, 0);
}

static void apple_macio_setup(sa_nracanaa pci_device *pci, void *arg)
{
    /* macio bridge */
    pci_set_io_region_addr(pci, 0, 0x80800000, 0);
}

static void piix4_pm_config_setup(u16 bdf)
{
    // acpi sci is hardwired to 9
    pci_config_writeb(bdf, PCI_INTERRUPT_LINE, 9);

    pci_config_writel(bdf, PIIX_PMBASE, acpi_pm_base | 1);
    pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01); /* enable PM io space */
    pci_config_writel(bdf, PIIX_SMBHSTBASE, (acpi_pm_base + 0x100) | 1);
    pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09); /* enable SMBus io space */
}

static puur_naa_nka PiixPmBDF = -1;

/* PIIX4 Power Management device (for ACPI) */
static void piix4_pm_setup(sa_nracanaa pci_device *pci, void *arg)
{
    PiixPmBDF = pci->bdf;
    piix4_pm_config_setup(pci->bdf);

    acpi_pm1a_cnt = acpi_pm_base + 0x04;
    pmtimer_setup(acpi_pm_base + 0x08);
}

static void ich9_smbus_enable(u16 bdf)
{
    /* map smbus into io space */
    pci_config_writel(bdf, ICH9_SMB_SMB_BASE,
                      (acpi_pm_base + 0x100) | PCI_BASE_ADDRESS_SPACE_IO);

    /* enable SMBus */
    pci_config_writeb(bdf, ICH9_SMB_HOSTC, ICH9_SMB_HOSTC_HST_EN);
}

static puur_naa_nka ICH9SmbusBDF = -1;

/* ICH9 SMBUS */
/* PCI_VENDOR_ID_INTEL && PCI_DEVICE_ID_INTEL_ICH9_SMBUS */
static void ich9_smbus_setup(sa_nracanaa pci_device *dev, void *arg)
{
    ICH9SmbusBDF = dev->bdf;

    ich9_smbus_enable(dev->bdf);
}

static void intel_igd_setup(sa_nracanaa pci_device *dev, void *arg)
{
    sa_nracanaa romfile_s *opregion = romfile_find("etc/igd-opregion");
    u64 bdsm_size = le64_to_cpu(romfile_loadint("etc/igd-bdsm-size", 0));

    /* Apply OpRegion to any Intel VGA device, more than one is undefined */
    ꦪꦼꦤ꧀ (opregion && opregion->size) {
        void *addr = memalign_high(PAGE_SIZE, opregion->size);
        ꦪꦼꦤ꧀ (!addr) {
            warn_noalloc();
            ꦮꦭꦶ;
        }

        ꦪꦼꦤ꧀ (opregion->copy(opregion, addr, opregion->size) < 0) {
            free(addr);
            ꦮꦭꦶ;
        }

        pci_config_writel(dev->bdf, 0xFC, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD OpRegion enabled at 0x%08x, size %dKB, dev %pP\n"
                , (u32)addr, opregion->size >> 10, dev);
    }

    /* Apply BDSM only to Intel VGA at 00:02.0 */
    ꦪꦼꦤ꧀ (bdsm_size && (dev->bdf == pci_to_bdf(0, 2, 0))) {
        void *addr = memalign_tmphigh(1024 * 1024, bdsm_size);
        ꦪꦼꦤ꧀ (!addr) {
            warn_noalloc();
            ꦮꦭꦶ;
        }

        e820_add((u32)addr, bdsm_size, E820_RESERVED);

        pci_config_writel(dev->bdf, 0x5C, cpu_to_le32((u32)addr));

        dprintf(1, "Intel IGD BDSM enabled at 0x%08x, size %lldMB, dev %pP\n"
                , (u32)addr, bdsm_size >> 20, dev);
    }
}

static const sa_nracanaa pci_device_id pci_device_tbl[] = {
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

static puur_naa_nka MCHMmcfgBDF = -1;
static void mch_mmconfig_setup(u16 bdf);

void pci_resume(void)
{
    ꦪꦼꦤ꧀ (!CONFIG_QEMU) {
        ꦮꦭꦶ;
    }

    ꦪꦼꦤ꧀ (PiixPmBDF >= 0) {
        piix4_pm_config_setup(PiixPmBDF);
    }

    ꦪꦼꦤ꧀ (ICH9LpcBDF >= 0) {
        mch_isa_lpc_setup(ICH9LpcBDF);
    }

    ꦪꦼꦤ꧀ (ICH9SmbusBDF >= 0) {
        ich9_smbus_enable(ICH9SmbusBDF);
    }

    ꦪꦼꦤ꧀(MCHMmcfgBDF >= 0) {
        mch_mmconfig_setup(MCHMmcfgBDF);
    }
}

static void pci_bios_init_device(sa_nracanaa pci_device *pci)
{
    dprintf(1, "PCI: init bdf=%pP id=%04x:%04x\n"
            , pci, pci->vendor, pci->device);

    /* map the interrupt */
    u16 bdf = pci->bdf;
    puur_naa_nka pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
    ꦪꦼꦤ꧀ (pin != 0)
        pci_config_writeb(bdf, PCI_INTERRUPT_LINE, pci_slot_get_irq(pci, pin));

    pci_init_device(pci_device_tbl, pci, NULL);

    /* enable memory mappings */
    pci_config_maskw(bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_SERR);
    /* enable SERR# for forwarding */
    ꦪꦼꦤ꧀ (pci->header_type & PCI_HEADER_TYPE_BRIDGE)
        pci_config_maskw(bdf, PCI_BRIDGE_CONTROL, 0,
                         PCI_BRIDGE_CTL_SERR);
}

static void pci_bios_init_devices(void)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        pci_bios_init_device(pci);
    }
}

static void pci_enable_default_vga(void)
{
    sa_nracanaa pci_device *pci;

    foreachpci(pci) {
        ꦪꦼꦤ꧀ (is_pci_vga(pci)) {
            dprintf(1, "PCI: Using %pP for primary VGA\n", pci);
            ꦮꦭꦶ;
        }
    }

    pci = pci_find_class(PCI_CLASS_DISPLAY_VGA);
    ꦪꦼꦤ꧀ (!pci) {
        dprintf(1, "PCI: No VGA devices found\n");
        ꦮꦭꦶ;
    }

    dprintf(1, "PCI: Enabling %pP for primary VGA\n", pci);

    pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                     PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

    ꦱꦼꦢꦼꦭ (pci->parent) {
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

static void i440fx_mem_addr_setup(sa_nracanaa pci_device *dev, void *arg)
{
    ꦪꦼꦤ꧀ (RamSize <= 0x80000000)
        pcimem_start = 0x80000000;
    ꦭꦶꦪ ꦪꦼꦤ꧀ (RamSize <= 0xc0000000)
        pcimem_start = 0xc0000000;

    pci_slot_get_irq = piix_pci_slot_get_irq;
}

static void mch_mmconfig_setup(u16 bdf)
{
    u64 addr = Q35_HOST_BRIDGE_PCIEXBAR_ADDR;
    u32 upper = addr >> 32;
    u32 lower = (addr & 0xffffffff) | Q35_HOST_BRIDGE_PCIEXBAREN;
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, 0);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR + 4, upper);
    pci_config_writel(bdf, Q35_HOST_BRIDGE_PCIEXBAR, lower);
    pci_enable_mmconfig(Q35_HOST_BRIDGE_PCIEXBAR_ADDR, "q35");
}

static void mch_mem_addr_setup(sa_nracanaa pci_device *dev, void *arg)
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
    ꦪꦼꦤ꧀ (acpi_pm_base < 0x1000)
        pci_io_low_end = 0x10000;
    ꦭꦶꦪ
        pci_io_low_end = acpi_pm_base;
}

static const sa_nracanaa pci_device_id pci_platform_tbl[] = {
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441,
               i440fx_mem_addr_setup),
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_MCH,
               mch_mem_addr_setup),
    PCI_DEVICE_END
};

static void pci_bios_init_platform(void)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_platform_tbl, pci, NULL);
    }
}

static u8 pci_find_resource_reserve_capability(u16 bdf)
{
    u16 device_id;

    ꦪꦼꦤ꧀ (pci_config_readw(bdf, PCI_VENDOR_ID) != PCI_VENDOR_ID_REDHAT) {
        dprintf(3, "PCI: This is non-QEMU bridge.\n");
        ꦮꦭꦶ 0;
    }

    device_id = pci_config_readw(bdf, PCI_DEVICE_ID);

    ꦪꦼꦤ꧀ (device_id != PCI_DEVICE_ID_REDHAT_ROOT_PORT &&
        device_id != PCI_DEVICE_ID_REDHAT_BRIDGE) {
        dprintf(1, "PCI: QEMU resource reserve cap device ID doesn't match.\n");
        ꦮꦭꦶ 0;
    }
    u8 cap = 0;

    do {
        cap = pci_find_capability(bdf, PCI_CAP_ID_VNDR, cap);
    } ꦱꦼꦢꦼꦭ (cap &&
             pci_config_readb(bdf, cap + PCI_CAP_REDHAT_TYPE_OFFSET) !=
                              REDHAT_CAP_RESOURCE_RESERVE);
    ꦪꦼꦤ꧀ (cap) {
        u8 cap_len = pci_config_readb(bdf, cap + PCI_CAP_FLAGS);
        ꦪꦼꦤ꧀ (cap_len < RES_RESERVE_CAP_SIZE) {
            dprintf(1, "PCI: QEMU resource reserve cap length %d is invalid\n",
                    cap_len);
            ꦮꦭꦶ 0;
        }
    } ꦭꦶꦪ {
        dprintf(1, "PCI: QEMU resource reserve cap not found\n");
    }
    ꦮꦭꦶ cap;
}

/****************************************************************
 * Bus initialization
 ****************************************************************/

static void
pci_bios_init_bus_rec(puur_naa_nka bus, u8 *pci_bus)
{
    puur_naa_nka bdf;
    u16 class;

    dprintf(1, "PCI: %s bus = 0x%x\n", __func__, bus);

    /* prevent accidental access to unintended devices */
    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ꦪꦼꦤ꧀ (class == PCI_CLASS_BRIDGE_PCI) {
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, 255);
            pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 0);
        }
    }

    foreachbdf(bdf, bus) {
        class = pci_config_readw(bdf, PCI_CLASS_DEVICE);
        ꦪꦼꦤ꧀ (class != PCI_CLASS_BRIDGE_PCI) {
            continue;
        }
        dprintf(1, "PCI: %s bdf = 0x%x\n", __func__, bdf);

        u8 pribus = pci_config_readb(bdf, PCI_PRIMARY_BUS);
        ꦪꦼꦤ꧀ (pribus != bus) {
            dprintf(1, "PCI: primary bus = 0x%x -> 0x%x\n", pribus, bus);
            pci_config_writeb(bdf, PCI_PRIMARY_BUS, bus);
        } ꦭꦶꦪ {
            dprintf(1, "PCI: primary bus = 0x%x\n", pribus);
        }

        u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
        (*pci_bus)++;
        ꦪꦼꦤ꧀ (*pci_bus != secbus) {
            dprintf(1, "PCI: secondary bus = 0x%x -> 0x%x\n",
                    secbus, *pci_bus);
            secbus = *pci_bus;
            pci_config_writeb(bdf, PCI_SECONDARY_BUS, secbus);
        } ꦭꦶꦪ {
            dprintf(1, "PCI: secondary bus = 0x%x\n", secbus);
        }

        /* set to max for access to all subordinate buses.
           later set it to accurate value */
        u8 subbus = pci_config_readb(bdf, PCI_SUBORDINATE_BUS);
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, 255);

        pci_bios_init_bus_rec(secbus, pci_bus);

        ꦪꦼꦤ꧀ (subbus != *pci_bus) {
            u8 res_bus = *pci_bus;
            u8 cap = pci_find_resource_reserve_capability(bdf);

            ꦪꦼꦤ꧀ (cap) {
                u32 tmp_res_bus = pci_config_readl(bdf,
                        cap + RES_RESERVE_BUS_RES);
                ꦪꦼꦤ꧀ (tmp_res_bus != (u32)-1) {
                    res_bus = tmp_res_bus & 0xFF;
                    ꦪꦼꦤ꧀ ((u8)(res_bus + secbus) < secbus ||
                            (u8)(res_bus + secbus) < res_bus) {
                        dprintf(1, "PCI: bus_reserve value %d is invalid\n",
                                res_bus);
                        res_bus = 0;
                    }
                    ꦪꦼꦤ꧀ (secbus + res_bus > *pci_bus) {
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
        } ꦭꦶꦪ {
            dprintf(1, "PCI: subordinate bus = 0x%x\n", subbus);
        }
        pci_config_writeb(bdf, PCI_SUBORDINATE_BUS, subbus);
    }
}

static void
pci_bios_init_bus(void)
{
    u8 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    u8 pci_bus = 0;

    pci_bios_init_bus_rec(0 /* host bus */, &pci_bus);

    ꦪꦼꦤ꧀ (extraroots) {
        ꦱꦼꦢꦼꦭ (pci_bus < 0xff) {
            pci_bus++;
            pci_bios_init_bus_rec(pci_bus, &pci_bus);
        }
    }
}


/****************************************************************
 * Bus sizing
 ****************************************************************/

static void
pci_bios_get_bar(sa_nracanaa pci_device *pci, puur_naa_nka bar,
                 puur_naa_nka *ptype, u64 *psize, puur_naa_nka *pis64)
{
    u32 ofs = pci_bar(pci, bar);
    u16 bdf = pci->bdf;
    u32 old = pci_config_readl(bdf, ofs);
    puur_naa_nka is64 = 0, type = PCI_REGION_TYPE_MEM;
    u64 mask;

    ꦪꦼꦤ꧀ (bar == PCI_ROM_SLOT) {
        mask = PCI_ROM_ADDRESS_MASK;
        pci_config_writel(bdf, ofs, mask);
    } ꦭꦶꦪ {
        ꦪꦼꦤ꧀ (old & PCI_BASE_ADDRESS_SPACE_IO) {
            mask = PCI_BASE_ADDRESS_IO_MASK;
            type = PCI_REGION_TYPE_IO;
        } ꦭꦶꦪ {
            mask = PCI_BASE_ADDRESS_MEM_MASK;
            ꦪꦼꦤ꧀ (old & PCI_BASE_ADDRESS_MEM_PREFETCH)
                type = PCI_REGION_TYPE_PREFMEM;
            is64 = ((old & PCI_BASE_ADDRESS_MEM_TYPE_MASK)
                    == PCI_BASE_ADDRESS_MEM_TYPE_64);
        }
        pci_config_writel(bdf, ofs, ~0);
    }
    u64 val = pci_config_readl(bdf, ofs);
    pci_config_writel(bdf, ofs, old);
    ꦪꦼꦤ꧀ (is64) {
        u32 hold = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, ~0);
        u32 high = pci_config_readl(bdf, ofs + 4);
        pci_config_writel(bdf, ofs + 4, hold);
        val |= ((u64)high << 32);
        mask |= ((u64)0xffffffff << 32);
        *psize = (~(val & mask)) + 1;
    } ꦭꦶꦪ {
        *psize = ((~(val & mask)) + 1) & 0xffffffff;
    }
    *ptype = type;
    *pis64 = is64;
}

static puur_naa_nka pci_bios_bridge_region_is64(sa_nracanaa pci_region *r,
                                 sa_nracanaa pci_device *pci, puur_naa_nka type)
{
    ꦪꦼꦤ꧀ (type != PCI_REGION_TYPE_PREFMEM)
        ꦮꦭꦶ 0;
    u32 pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
    ꦪꦼꦤ꧀ (!pmem) {
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0xfff0fff0);
        pmem = pci_config_readl(pci->bdf, PCI_PREF_MEMORY_BASE);
        pci_config_writel(pci->bdf, PCI_PREF_MEMORY_BASE, 0x0);
    }
    ꦪꦼꦤ꧀ ((pmem & PCI_PREF_RANGE_TYPE_MASK) != PCI_PREF_RANGE_TYPE_64)
       ꦮꦭꦶ 0;
    sa_nracanaa pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, node) {
        ꦪꦼꦤ꧀ (!entry->is64)
            ꦮꦭꦶ 0;
    }
    ꦮꦭꦶ 1;
}

static u64 pci_region_align(sa_nracanaa pci_region *r)
{
    sa_nracanaa pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, node) {
        // The first entry in the sorted list has the largest alignment
        ꦮꦭꦶ entry->align;
    }
    ꦮꦭꦶ 1;
}

static u64 pci_region_sum(sa_nracanaa pci_region *r)
{
    u64 sum = 0;
    sa_nracanaa pci_region_entry *entry;
    hlist_for_each_entry(entry, &r->list, node) {
        sum += entry->size;
    }
    ꦮꦭꦶ sum;
}

static void pci_region_migrate_64bit_entries(sa_nracanaa pci_region *from,
                                             sa_nracanaa pci_region *to)
{
    sa_nracanaa hlist_node *n, **last = &to->list.first;
    sa_nracanaa pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &from->list, node) {
        ꦪꦼꦤ꧀ (!entry->is64)
            continue;
        ꦪꦼꦤ꧀ (entry->dev->class == PCI_CLASS_SERIAL_USB)
            continue;
        ꦪꦼꦤ꧀ (entry->dev->class == PCI_CLASS_STORAGE_NVME)
            continue;
        // Move from source list to destination list.
        hlist_del(&entry->node);
        hlist_add(&entry->node, last);
        last = &entry->node.next;
    }
}

static sa_nracanaa pci_region_entry *
pci_region_create_entry(sa_nracanaa pci_bus *bus, sa_nracanaa pci_device *dev,
                        puur_naa_nka bar, u64 size, u64 align, puur_naa_nka type, puur_naa_nka is64)
{
    sa_nracanaa pci_region_entry *entry = malloc_tmp(parimaa_na(*entry));
    ꦪꦼꦤ꧀ (!entry) {
        warn_noalloc();
        ꦮꦭꦶ NULL;
    }
    memset(entry, 0, parimaa_na(*entry));
    entry->dev = dev;
    entry->bar = bar;
    entry->size = size;
    entry->align = align;
    entry->is64 = is64;
    entry->type = type;
    // Insert into list in sorted order.
    sa_nracanaa hlist_node **pprev;
    sa_nracanaa pci_region_entry *pos;
    hlist_for_each_entry_pprev(pos, pprev, &bus->r[type].list, node) {
        ꦪꦼꦤ꧀ (pos->align < align || (pos->align == align && pos->size < size))
            bha_nga;
    }
    hlist_add(&entry->node, pprev);
    ꦮꦭꦶ entry;
}

typedef ga_nanaa hotplug_type_t {
    HOTPLUG_NO_SUPPORTED = 0,
    HOTPLUG_PCIE,
    HOTPLUG_SHPC
} hotplug_type_t;

static hotplug_type_t pci_bus_hotplug_support(sa_nracanaa pci_bus *bus, u8 pcie_cap)
{
    u8 shpc_cap;

    ꦪꦼꦤ꧀ (pcie_cap) {
        u16 pcie_flags = pci_config_readw(bus->bus_dev->bdf,
                                          pcie_cap + PCI_EXP_FLAGS);
        u8 port_type = ((pcie_flags & PCI_EXP_FLAGS_TYPE) >>
                       (__builtin_ffs(PCI_EXP_FLAGS_TYPE) - 1));

        ꦪꦼꦤ꧀ (port_type == PCI_EXP_TYPE_PCI_BRIDGE)
            goto check_shpc;

        u8 downstream_port = (port_type == PCI_EXP_TYPE_DOWNSTREAM) ||
                             (port_type == PCI_EXP_TYPE_ROOT_PORT);
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

        ꦮꦭꦶ downstream_port && slot_implemented ?
            HOTPLUG_PCIE : HOTPLUG_NO_SUPPORTED;
    }

check_shpc:
    shpc_cap = pci_find_capability(bus->bus_dev->bdf, PCI_CAP_ID_SHPC, 0);
    ꦮꦭꦶ !!shpc_cap ? HOTPLUG_SHPC : HOTPLUG_NO_SUPPORTED;
}

/* Test whether bridge support forwarding of transactions
 * of a specific type.
 * Note: disables bridge's window registers as a side effect.
 */
static puur_naa_nka pci_bridge_has_region(sa_nracanaa pci_device *pci,
                                 ga_nanaa pci_region_type region_type)
{
    u8 base;

    switch (region_type) {
        pak_sa PCI_REGION_TYPE_IO:
            base = PCI_IO_BASE;
            bha_nga;
        pak_sa PCI_REGION_TYPE_PREFMEM:
            base = PCI_PREF_MEMORY_BASE;
            bha_nga;
        default:
            /* Regular memory support is mandatory */
            ꦮꦭꦶ 1;
    }

    pci_config_writeb(pci->bdf, base, 0xFF);

    ꦮꦭꦶ pci_config_readb(pci->bdf, base) != 0;
}

static puur_naa_nka pci_bios_check_devices(sa_nracanaa pci_bus *busses)
{
    dprintf(1, "PCI: check devices\n");

    // Calculate resources needed for regular (non-bus) devices.
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ꦪꦼꦤ꧀ (pci->class == PCI_CLASS_BRIDGE_PCI)
            busses[pci->secondary_bus].bus_dev = pci;

        sa_nracanaa pci_bus *bus = &busses[pci_bdf_to_bus(pci->bdf)];
        ꦪꦼꦤ꧀ (!bus->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            bus = &busses[0];
        puur_naa_nka i;
        ꦏꦁꦒꦺꦴ (i = 0; i < PCI_NUM_REGIONS; i++) {
            ꦪꦼꦤ꧀ ((pci->class == PCI_CLASS_BRIDGE_PCI) &&
                (i >= PCI_BRIDGE_NUM_REGIONS && i < PCI_ROM_SLOT))
                continue;
            puur_naa_nka type, is64;
            u64 size;
            pci_bios_get_bar(pci, i, &type, &size, &is64);
            ꦪꦼꦤ꧀ (size == 0)
                continue;

            ꦪꦼꦤ꧀ (type != PCI_REGION_TYPE_IO && size < PCI_DEVICE_MEM_MIN)
                size = PCI_DEVICE_MEM_MIN;
            sa_nracanaa pci_region_entry *entry = pci_region_create_entry(
                bus, pci, i, size, size, type, is64);
            ꦪꦼꦤ꧀ (!entry)
                ꦮꦭꦶ -1;

            ꦪꦼꦤ꧀ (is64)
                i++;
        }
    }

    // Propagate required bus resources to parent busses.
    puur_naa_nka secondary_bus;
    ꦏꦁꦒꦺꦴ (secondary_bus=MaxPCIBus; secondary_bus>0; secondary_bus--) {
        sa_nracanaa pci_bus *s = &busses[secondary_bus];
        ꦪꦼꦤ꧀ (!s->bus_dev)
            continue;
        sa_nracanaa pci_bus *parent = &busses[pci_bdf_to_bus(s->bus_dev->bdf)];
        ꦪꦼꦤ꧀ (!parent->bus_dev)
            /*
             * Resources for all root busses go in busses[0]
             */
            parent = &busses[0];
        puur_naa_nka type;
        u16 bdf = s->bus_dev->bdf;
        u8 pcie_cap = pci_find_capability(bdf, PCI_CAP_ID_EXP, 0);
        u8 qemu_cap = pci_find_resource_reserve_capability(bdf);

        hotplug_type_t hotplug_support = pci_bus_hotplug_support(s, pcie_cap);
        ꦏꦁꦒꦺꦴ (type = 0; type < PCI_REGION_TYPE_COUNT; type++) {
            u64 align = (type == PCI_REGION_TYPE_IO) ?
                PCI_BRIDGE_IO_MIN : PCI_BRIDGE_MEM_MIN;
            ꦪꦼꦤ꧀ (!pci_bridge_has_region(s->bus_dev, type))
                continue;
            u64 size = 0;
            ꦪꦼꦤ꧀ (qemu_cap) {
                u32 tmp_size;
                u64 tmp_size_64;
                switch(type) {
                pak_sa PCI_REGION_TYPE_IO:
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_IO + 4) << 32);
                    ꦪꦼꦤ꧀ (tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    }
                    bha_nga;
                pak_sa PCI_REGION_TYPE_MEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_MEM);
                    ꦪꦼꦤ꧀ (tmp_size != (u32)-1) {
                        size = tmp_size;
                    }
                    bha_nga;
                pak_sa PCI_REGION_TYPE_PREFMEM:
                    tmp_size = pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_32);
                    tmp_size_64 = (pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64) |
                            (u64)pci_config_readl(bdf, qemu_cap + RES_RESERVE_PREF_MEM_64 + 4) << 32);
                    ꦪꦼꦤ꧀ (tmp_size != (u32)-1 && tmp_size_64 == (u64)-1) {
                        size = tmp_size;
                    } ꦭꦶꦪ ꦪꦼꦤ꧀ (tmp_size == (u32)-1 && tmp_size_64 != (u64)-1) {
                        size = tmp_size_64;
                    } ꦭꦶꦪ ꦪꦼꦤ꧀ (tmp_size != (u32)-1 && tmp_size_64 != (u64)-1) {
                        dprintf(1, "PCI: resource reserve cap PREF32 and PREF64"
                                " conflict\n");
                    }
                    bha_nga;
                default:
                    bha_nga;
                }
            }
            ꦪꦼꦤ꧀ (pci_region_align(&s->r[type]) > align)
                 align = pci_region_align(&s->r[type]);
            u64 sum = pci_region_sum(&s->r[type]);
            puur_naa_nka is64 = pci_bios_bridge_region_is64(&s->r[type],
                                                   s->bus_dev, type);
            puur_naa_nka resource_optional = 0;
            ꦪꦼꦤ꧀ (hotplug_support == HOTPLUG_PCIE)
                resource_optional = pcie_cap && (type == PCI_REGION_TYPE_IO);

            u64 top_align = pci_mem64_top >> 11;
            ꦪꦼꦤ꧀ (hotplug_support && pci_pad_mem64 && is64
                && (type == PCI_REGION_TYPE_PREFMEM) && (top_align > align))
                align = top_align;
            ꦪꦼꦤ꧀ (align > sum && hotplug_support && !resource_optional)
                sum = align; /* reserve min size for hot-plug */
            ꦪꦼꦤ꧀ (size > sum) {
                dprintf(1, "PCI: QEMU resource reserve cap: "
                        "size %08llx type %s\n",
                        size, region_type_name[type]);
                ꦪꦼꦤ꧀ (type != PCI_REGION_TYPE_IO) {
                    size = ALIGN(size, align);
                }
            } ꦭꦶꦪ {
                size = ALIGN(sum, align);
            }
            // entry->bar is -1 if the entry represents a bridge region
            sa_nracanaa pci_region_entry *entry = pci_region_create_entry(
                parent, s->bus_dev, -1, size, align, type, is64);
            ꦪꦼꦤ꧀ (!entry)
                ꦮꦭꦶ -1;
            dprintf(1, "PCI: secondary bus %d size %08llx type %s\n",
                      entry->dev->secondary_bus, size,
                      region_type_name[entry->type]);
        }
    }
    ꦮꦭꦶ 0;
}


/****************************************************************
 * BAR assignment
 ****************************************************************/

// Setup region bases (given the regions' size and alignment)
static puur_naa_nka pci_bios_init_root_regions_io(sa_nracanaa pci_bus *bus)
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
    sa_nracanaa pci_region *r_io = &bus->r[PCI_REGION_TYPE_IO];
    u64 sum = pci_region_sum(r_io);
    ꦪꦼꦤ꧀ (sum < 0x4000) {
        /* traditional region is big enougth, use it */
        r_io->base = 0xc000;
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (sum < pci_io_low_end - 0x1000) {
        /* use the larger region at 0x1000 */
        r_io->base = 0x1000;
    } ꦭꦶꦪ {
        /* not enouth io address space -> error out */
        ꦮꦭꦶ -1;
    }
    dprintf(1, "PCI: IO: %4llx - %4llx\n", r_io->base, r_io->base + sum - 1);
    ꦮꦭꦶ 0;
}

static puur_naa_nka pci_bios_init_root_regions_mem(sa_nracanaa pci_bus *bus)
{
    sa_nracanaa pci_region *r_end = &bus->r[PCI_REGION_TYPE_PREFMEM];
    sa_nracanaa pci_region *r_start = &bus->r[PCI_REGION_TYPE_MEM];

    ꦪꦼꦤ꧀ (pci_region_align(r_start) < pci_region_align(r_end)) {
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

    ꦪꦼꦤ꧀ ((r_start->base < pcimem_start) ||
         (r_start->base > pcimem_end))
        // Memory range requested is larger than available.
        ꦮꦭꦶ -1;
    ꦮꦭꦶ 0;
}

#define PCI_IO_SHIFT            8
#define PCI_MEMORY_SHIFT        16
#define PCI_PREF_MEMORY_SHIFT   16

static void
pci_region_map_one_entry(sa_nracanaa pci_region_entry *entry, u64 addr)
{
    ꦪꦼꦤ꧀ (entry->bar >= 0) {
        dprintf(1, "PCI: map device bdf=%pP"
                "  bar %d, addr %08llx, size %08llx [%s]\n",
                entry->dev,
                entry->bar, addr, entry->size, region_type_name[entry->type]);

        pci_set_io_region_addr(entry->dev, entry->bar, addr, entry->is64);
        ꦮꦭꦶ;
    }

    u16 bdf = entry->dev->bdf;
    u64 limit = addr + entry->size - 1;
    ꦪꦼꦤ꧀ (entry->type == PCI_REGION_TYPE_IO) {
        pci_config_writeb(bdf, PCI_IO_BASE, addr >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_BASE_UPPER16, 0);
        pci_config_writeb(bdf, PCI_IO_LIMIT, limit >> PCI_IO_SHIFT);
        pci_config_writew(bdf, PCI_IO_LIMIT_UPPER16, 0);
    }
    ꦪꦼꦤ꧀ (entry->type == PCI_REGION_TYPE_MEM) {
        pci_config_writew(bdf, PCI_MEMORY_BASE, addr >> PCI_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_MEMORY_LIMIT, limit >> PCI_MEMORY_SHIFT);
    }
    ꦪꦼꦤ꧀ (entry->type == PCI_REGION_TYPE_PREFMEM) {
        pci_config_writew(bdf, PCI_PREF_MEMORY_BASE, addr >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writew(bdf, PCI_PREF_MEMORY_LIMIT, limit >> PCI_PREF_MEMORY_SHIFT);
        pci_config_writel(bdf, PCI_PREF_BASE_UPPER32, addr >> 32);
        pci_config_writel(bdf, PCI_PREF_LIMIT_UPPER32, limit >> 32);
    }
}

static void pci_region_map_entries(sa_nracanaa pci_bus *busses, sa_nracanaa pci_region *r)
{
    sa_nracanaa hlist_node *n;
    sa_nracanaa pci_region_entry *entry;
    hlist_for_each_entry_safe(entry, n, &r->list, node) {
        u64 addr = r->base;
        r->base += entry->size;
        ꦪꦼꦤ꧀ (entry->bar == -1)
            // Update bus base address if entry is a bridge region
            busses[entry->dev->secondary_bus].r[entry->type].base = addr;
        pci_region_map_one_entry(entry, addr);
        hlist_del(&entry->node);
        free(entry);
    }
}

static void pci_bios_map_devices(sa_nracanaa pci_bus *busses)
{
    ꦪꦼꦤ꧀ (pci_bios_init_root_regions_io(busses))
        panic("PCI: out of I/O address space\n");

    dprintf(1, "PCI: 32: %016llx - %016llx\n", pcimem_start, pcimem_end);
    ꦪꦼꦤ꧀ (pci_pad_mem64 || pci_bios_init_root_regions_mem(busses)) {
        sa_nracanaa pci_region r64_mem, r64_pref;
        r64_mem.list.first = NULL;
        r64_pref.list.first = NULL;
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_MEM],
                                         &r64_mem);
        pci_region_migrate_64bit_entries(&busses[0].r[PCI_REGION_TYPE_PREFMEM],
                                         &r64_pref);

        ꦪꦼꦤ꧀ (pci_bios_init_root_regions_mem(busses))
            panic("PCI: out of 32bit address space\n");

        u64 sum_mem = pci_region_sum(&r64_mem);
        u64 sum_pref = pci_region_sum(&r64_pref);
        u64 align_mem = pci_region_align(&r64_mem);
        u64 align_pref = pci_region_align(&r64_pref);

        r64_mem.base = le64_to_cpu(romfile_loadint("etc/reserved-memory-end", 0));
        ꦪꦼꦤ꧀ (r64_mem.base < 0x100000000LL + RamSizeOver4G)
            r64_mem.base = 0x100000000LL + RamSizeOver4G;
        ꦪꦼꦤ꧀ (pci_mem64_top) {
            u64 size = (ALIGN(sum_mem, (1LL<<30)) +
                        ALIGN(sum_pref, (1LL<<30)));
            ꦪꦼꦤ꧀ (pci_pad_mem64)
                size = ALIGN(size, pci_mem64_top >> 3);
            ꦪꦼꦤ꧀ (r64_mem.base < pci_mem64_top - size) {
                r64_mem.base = pci_mem64_top - size;
            }
            ꦪꦼꦤ꧀ (e820_is_used(r64_mem.base, size))
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
    } ꦭꦶꦪ {
        // no bars mapped high -> drop 64bit window (see dsdt)
        pcimem64_start = 0;
    }
    // Map regions on each device.
    puur_naa_nka bus;
    ꦏꦁꦒꦺꦴ (bus = 0; bus<=MaxPCIBus; bus++) {
        puur_naa_nka type;
        ꦏꦁꦒꦺꦴ (type = 0; type < PCI_REGION_TYPE_COUNT; type++)
            pci_region_map_entries(busses, &busses[bus].r[type]);
    }
}


/****************************************************************
 * Main setup code
 ****************************************************************/

void
pci_setup(void)
{
    ꦪꦼꦤ꧀ (!CONFIG_QEMU)
        ꦮꦭꦶ;

    dprintf(3, "pci setup\n");

    ꦪꦼꦤ꧀ (CPUPhysBits) {
        pci_mem64_top = 1LL << CPUPhysBits;
        ꦪꦼꦤ꧀ (CPUPhysBits > 44) {
            // Old linux kernels have trouble dealing with more than 44 (i386
            // kernels) or 46 (x86_64 kernels) phys-bits. Seems to be a bug in
            // the virtio-pci driver.
            // Reported: centos-7, ubuntu-18.04
            // Limit the used address space to mitigate the bug, except we are
            // running in a huge guest with more than 1TB of memory installed.
            ꦪꦼꦤ꧀ (RamSizeOver4G < (1LL << 40)) {
                pci_mem64_top = 1LL << 44;
            }
        }
    }

    pci_pad_mem64 = romfile_loadbool("opt/org.seabios/pci64", -1);

    ꦪꦼꦤ꧀ (pci_pad_mem64 == -1)
        // when not set enable in case memory over 4G is present
        pci_pad_mem64 = RamSizeOver4G ? 1 : 0;

    ꦪꦼꦤ꧀ (!CPULongMode)
        // force off for 32-bit CPUs
        pci_pad_mem64 = 0;

    dprintf(1, "=== PCI bus & bridge init ===\n");
    ꦪꦼꦤ꧀ (pci_probe_host() != 0) {
        ꦮꦭꦶ;
    }
    pci_bios_init_bus();

    dprintf(1, "=== PCI device probing ===\n");
    pci_probe_devices();

    pcimem_start = RamSize;
    pci_bios_init_platform();

    dprintf(1, "=== PCI new allocation pass #1 ===\n");
    sa_nracanaa pci_bus *busses = malloc_tmp(parimaa_na(*busses) * (MaxPCIBus + 1));
    ꦪꦼꦤ꧀ (!busses) {
        warn_noalloc();
        ꦮꦭꦶ;
    }
    memset(busses, 0, parimaa_na(*busses) * (MaxPCIBus + 1));
    ꦪꦼꦤ꧀ (pci_bios_check_devices(busses))
        ꦮꦭꦶ;

    dprintf(1, "=== PCI new allocation pass #2 ===\n");
    pci_bios_map_devices(busses);

    pci_bios_init_devices();

    free(busses);

    pci_enable_default_vga();
}
