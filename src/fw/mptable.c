// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_bdf_to_bus
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "std/mptable.h" // MPTABLE_SIGNATURE
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // MaxCountCPUs
#ଅନ୍ତର୍ଭାଵ "x86.h" // cpuid

ଶୂନ୍ୟ
mptable_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_MPTABLE)
        ଫେରନ୍ତୁ;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    ସଂରଚନା mptable_config_s *config = malloc_tmp(32*1024);
    ଯଦି (!config) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(config, 0, ଆକାର(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    memcpy(config->oemid, BUILD_CPUNAME8, ଆକାର(config->oemid));
    memcpy(config->productid, "0.1         ", ଆକାର(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    ଯଦି (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pkgcpus = 1;
    ଯଦି (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    ସଂରଚନା mpt_cpu *cpus = (ଶୂନ୍ୟ*)&config[1], *cpu = cpus;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < Max_ଗଣନା_CPUs; i+=pkgcpus) {
        memset(cpu, 0, ଆକାର(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା entrycount = cpu - cpus;

    // PCI bus
    ସଂରଚନା mpt_bus *buses = (ଶୂନ୍ୟ*)cpu, *bus = buses;
    ଯଦି (!hlist_empty(&PCIDevices)) {
        memset(bus, 0, ଆକାର(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        memcpy(bus->bustype, "PCI   ", ଆକାର(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isabusid = bus - buses;
    memset(bus, 0, ଆକାର(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    memcpy(bus->bustype, "ISA   ", ଆକାର(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    ସଂରଚନା mpt_ioapic *ioapic = (ଶୂନ୍ୟ*)bus;
    memset(ioapic, 0, ଆକାର(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    ସଂରଚନା mpt_intsrc *intsrcs = (ଶୂନ୍ୟ*)&ioapic[1], *intsrc = intsrcs;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା dev = -1;
    unsigned short pinmask = 0;

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        ଯଦି (pci_bdf_to_bus(bdf) != 0)
            ଭାଙ୍ଗ;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା irq = pci_config_readb(bdf, PCI_INTERRUPT_ଧାଡ଼ି);
        ଯଦି (pin == 0)
            ଜାରି;
        ଯଦି (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        ଯଦି (pinmask & (1 << pin)) /* pin was seen already */
            ଜାରି;
        pinmask |= (1 << pin);
        memset(intsrc, 0, ଆକାର(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା irq0_override = romfile_loadint("etc/irq0-override", 0);
    ପାଇଁ (i = 0; i < 16; i++) {
        memset(intsrc, 0, ଆକାର(*intsrc));
        ଯଦି (BUILD_PCI_IRQS & (1 << i))
            ଜାରି;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        ଯଦି (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            ଯଦି (i == 0)
                intsrc->dstirq = 2;
            ଅନ୍ୟଥା ଯଦି (i == 2)
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା length = (ଶୂନ୍ୟ*)intsrc - (ଶୂନ୍ୟ*)config;
    config->entrycount = entrycount;
    config->length = length;
    config->checksum -= checksum(config, length);

    // floating pointer structure
    ସଂରଚନା mptable_floating_s floating;
    memset(&floating, 0, ଆକାର(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.length = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, ଆକାର(floating));
    copy_mptable(&floating);
    ମୁକ୍ତ(config);
}
