// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "hw/pci.h" // pci_bdf_to_bus
#অন্তর্ভাৱ "hw/pcidevice.h" // foreachpci
#অন্তর্ভাৱ "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "romfile.h" // romfile_loadint
#অন্তর্ভাৱ "std/mptable.h" // MPTABLE_SIGNATURE
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // MaxCountCPUs
#অন্তর্ভাৱ "x86.h" // cpuid

শূন্য
mptable_setup(শূন্য)
{
    যদি (! CONFIG_MPTABLE)
        ঘূৰাই;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    সংরচনা mptable_config_s *config = malloc_tmp(32*1024);
    যদি (!config) {
        warn_noalloc();
        ঘূৰাই;
    }
    memset(config, 0, পরিমাণ(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    memcpy(config->oemid, BUILD_CPUNAME8, পরিমাণ(config->oemid));
    memcpy(config->productid, "0.1         ", পরিমাণ(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    যদি (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    পূর্ণাঙ্ক pkgcpus = 1;
    যদি (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    সংরচনা mpt_cpu *cpus = (শূন্য*)&config[1], *cpu = cpus;
    পূর্ণাঙ্ক i;
    বাবে (i = 0; i < Max_গুনতি_CPUs; i+=pkgcpus) {
        memset(cpu, 0, পরিমাণ(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    পূর্ণাঙ্ক entrycount = cpu - cpus;

    // PCI bus
    সংরচনা mpt_bus *buses = (শূন্য*)cpu, *bus = buses;
    যদি (!hlist_empty(&PCIDevices)) {
        memset(bus, 0, পরিমাণ(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        memcpy(bus->bustype, "PCI   ", পরিমাণ(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    পূর্ণাঙ্ক isabusid = bus - buses;
    memset(bus, 0, পরিমাণ(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    memcpy(bus->bustype, "ISA   ", পরিমাণ(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    সংরচনা mpt_ioapic *ioapic = (শূন্য*)bus;
    memset(ioapic, 0, পরিমাণ(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    সংরচনা mpt_intsrc *intsrcs = (শূন্য*)&ioapic[1], *intsrc = intsrcs;
    পূর্ণাঙ্ক dev = -1;
    unsigned short pinmask = 0;

    সংরচনা pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        যদি (pci_bdf_to_bus(bdf) != 0)
            ভঙ্গ;
        পূর্ণাঙ্ক pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        পূর্ণাঙ্ক irq = pci_config_readb(bdf, PCI_INTERRUPT_সাৰি);
        যদি (pin == 0)
            অনুৱর্ত;
        যদি (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        যদি (pinmask & (1 << pin)) /* pin was seen already */
            অনুৱর্ত;
        pinmask |= (1 << pin);
        memset(intsrc, 0, পরিমাণ(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    পূর্ণাঙ্ক irq0_override = romfile_loadint("etc/irq0-override", 0);
    বাবে (i = 0; i < 16; i++) {
        memset(intsrc, 0, পরিমাণ(*intsrc));
        যদি (BUILD_PCI_IRQS & (1 << i))
            অনুৱর্ত;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        যদি (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            যদি (i == 0)
                intsrc->dstirq = 2;
            অন্যথা যদি (i == 2)
                intsrc--;
        }
        intsrc++;
    }

    /* Local interrupt assignment */
    intsrc->type = MPT_TYPE_LOCAL_INT;
    intsrc->irqtype = 3; /* ExtINT */
    intsrc->irqflag = 0; /* PO, EL default */
    intsrc->srcbus = isabusid; /* ISA */
    intsrc->srcbusirq = 0;
    intsrc->dstapic = 0; /* BSP == APIC #0 */
    intsrc->dstirq = 0; /* LINTIN0 */
    intsrc++;

    intsrc->type = MPT_TYPE_LOCAL_INT;
    intsrc->irqtype = 1; /* NMI */
    intsrc->irqflag = 0; /* PO, EL default */
    intsrc->srcbus = isabusid; /* ISA */
    intsrc->srcbusirq = 0;
    intsrc->dstapic = 0xff; /* to all local APICs */
    intsrc->dstirq = 1; /* LINTIN1 */
    intsrc++;
    entrycount += intsrc - intsrcs;

    // Finalize config structure.
    পূর্ণাঙ্ক length = (শূন্য*)intsrc - (শূন্য*)config;
    config->entrycount = entrycount;
    config->length = length;
    config->checksum -= checksum(config, length);

    // floating pointer structure
    সংরচনা mptable_floating_s floating;
    memset(&floating, 0, পরিমাণ(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.length = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, পরিমাণ(floating));
    copy_mptable(&floating);
    মুক্ত(config);
}
