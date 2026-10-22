// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "hw/pci.h" // pci_bdf_to_bus
#अन्तर्भाव "hw/pcidevice.h" // foreachpci
#अन्तर्भाव "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "std/mptable.h" // MPTABLE_SIGNATURE
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // MaxCountCPUs
#अन्तर्भाव "x86.h" // cpuid

शून्य
mptable_setup(शून्य)
{
    सचे (! CONFIG_MPTABLE)
        पटिनिवत्त;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    संरचना mptable_config_s *config = malloc_tmp(32*1024);
    सचे (!config) {
        warn_noalloc();
        पटिनिवत्त;
    }
    memset(config, 0, परिमाण(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    memcpy(config->oemid, BUILD_CPUNAME8, परिमाण(config->oemid));
    memcpy(config->productid, "0.1         ", परिमाण(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    सचे (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    पूर्णाङ्क pkgcpus = 1;
    सचे (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    संरचना mpt_cpu *cpus = (शून्य*)&config[1], *cpu = cpus;
    पूर्णाङ्क i;
    अत्थाय (i = 0; i < Max_गणनम्_CPUs; i+=pkgcpus) {
        memset(cpu, 0, परिमाण(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    पूर्णाङ्क entrycount = cpu - cpus;

    // PCI bus
    संरचना mpt_bus *buses = (शून्य*)cpu, *bus = buses;
    सचे (!hlist_empty(&PCIDevices)) {
        memset(bus, 0, परिमाण(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        memcpy(bus->bustype, "PCI   ", परिमाण(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    पूर्णाङ्क isabusid = bus - buses;
    memset(bus, 0, परिमाण(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    memcpy(bus->bustype, "ISA   ", परिमाण(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    संरचना mpt_ioapic *ioapic = (शून्य*)bus;
    memset(ioapic, 0, परिमाण(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    संरचना mpt_intsrc *intsrcs = (शून्य*)&ioapic[1], *intsrc = intsrcs;
    पूर्णाङ्क dev = -1;
    unsigned short pinmask = 0;

    संरचना pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        सचे (pci_bdf_to_bus(bdf) != 0)
            भङ्ग;
        पूर्णाङ्क pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        पूर्णाङ्क irq = pci_config_readb(bdf, PCI_INTERRUPT_पङ्क्तिः);
        सचे (pin == 0)
            अनुवर्त;
        सचे (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        सचे (pinmask & (1 << pin)) /* pin was seen already */
            अनुवर्त;
        pinmask |= (1 << pin);
        memset(intsrc, 0, परिमाण(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    पूर्णाङ्क irq0_override = romfile_loadint("etc/irq0-override", 0);
    अत्थाय (i = 0; i < 16; i++) {
        memset(intsrc, 0, परिमाण(*intsrc));
        सचे (BUILD_PCI_IRQS & (1 << i))
            अनुवर्त;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        सचे (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            सचे (i == 0)
                intsrc->dstirq = 2;
            अञ्ञथा सचे (i == 2)
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
    पूर्णाङ्क length = (शून्य*)intsrc - (शून्य*)config;
    config->entrycount = entrycount;
    config->length = length;
    config->checksum -= checksum(config, length);

    // floating pointer structure
    संरचना mptable_floating_s floating;
    memset(&floating, 0, परिमाण(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.length = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, परिमाण(floating));
    copy_mptable(&floating);
    मुक्त(config);
}
