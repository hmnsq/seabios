// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "hw/pci.h" // pci_bdf_to_bus
#অন্তর্ভাব "hw/pcidevice.h" // foreachpci
#অন্তর্ভাব "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "std/mptable.h" // MPTABLE_SIGNATURE
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // MaxCountCPUs
#অন্তর্ভাব "x86.h" // cpuid

শূন্য
mptable_setup(শূন্য)
{
    যদি (! CONFIG_MPTABLE)
        ফেরত;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    কাঠামো mptable_config_s *config = malloc_tmp(32*1024);
    যদি (!config) {
        warn_noalloc();
        ফেরত;
    }
    memset(config, 0, আকার(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    memcpy(config->oemid, BUILD_CPUNAME8, আকার(config->oemid));
    memcpy(config->productid, "0.1         ", আকার(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    যদি (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    পূর্ণসংখ্যা pkgcpus = 1;
    যদি (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    কাঠামো mpt_cpu *cpus = (শূন্য*)&config[1], *cpu = cpus;
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < Max_গুনতি_CPUs; i+=pkgcpus) {
        memset(cpu, 0, আকার(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    পূর্ণসংখ্যা entrycount = cpu - cpus;

    // PCI bus
    কাঠামো mpt_bus *buses = (শূন্য*)cpu, *bus = buses;
    যদি (!hlist_empty(&PCIDevices)) {
        memset(bus, 0, আকার(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        memcpy(bus->bustype, "PCI   ", আকার(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    পূর্ণসংখ্যা isabusid = bus - buses;
    memset(bus, 0, আকার(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    memcpy(bus->bustype, "ISA   ", আকার(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    কাঠামো mpt_ioapic *ioapic = (শূন্য*)bus;
    memset(ioapic, 0, আকার(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    কাঠামো mpt_intsrc *intsrcs = (শূন্য*)&ioapic[1], *intsrc = intsrcs;
    পূর্ণসংখ্যা dev = -1;
    unsigned short pinmask = 0;

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        যদি (pci_bdf_to_bus(bdf) != 0)
            বিরতি;
        পূর্ণসংখ্যা pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        পূর্ণসংখ্যা irq = pci_config_readb(bdf, PCI_INTERRUPT_সারি);
        যদি (pin == 0)
            চালু;
        যদি (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        যদি (pinmask & (1 << pin)) /* pin was seen already */
            চালু;
        pinmask |= (1 << pin);
        memset(intsrc, 0, আকার(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    পূর্ণসংখ্যা irq0_override = romfile_loadint("etc/irq0-override", 0);
    জন্য (i = 0; i < 16; i++) {
        memset(intsrc, 0, আকার(*intsrc));
        যদি (BUILD_PCI_IRQS & (1 << i))
            চালু;
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
    পূর্ণসংখ্যা দৈর্ঘ্য = (শূন্য*)intsrc - (শূন্য*)config;
    config->entrycount = entrycount;
    config->দৈর্ঘ্য = দৈর্ঘ্য;
    config->checksum -= checksum(config, দৈর্ঘ্য);

    // floating pointer structure
    কাঠামো mptable_floating_s floating;
    memset(&floating, 0, আকার(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.দৈর্ঘ্য = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, আকার(floating));
    copy_mptable(&floating);
    মুক্ত(config);
}
