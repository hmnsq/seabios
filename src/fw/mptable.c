// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "hw/pci.h" // pci_bdf_to_bus
#समावेश "hw/pcidevice.h" // foreachpci
#समावेश "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_loadint
#समावेश "std/mptable.h" // MPTABLE_SIGNATURE
#समावेश "string.h" // memset
#समावेश "util.h" // MaxCountCPUs
#समावेश "x86.h" // cpuid

व्योम
mptable_setup(व्योम)
{
    अगर (! CONFIG_MPTABLE)
        वापस;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    काष्ठा mptable_config_s *config = malloc_tmp(32*1024);
    अगर (!config) {
        warn_noalloc();
        वापस;
    }
    स_रखो(config, 0, माप(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    स_नकल(config->oemid, BUILD_CPUNAME8, माप(config->oemid));
    स_नकल(config->productid, "0.1         ", माप(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    अगर (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    पूर्णांक pkgcpus = 1;
    अगर (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    काष्ठा mpt_cpu *cpus = (व्योम*)&config[1], *cpu = cpus;
    पूर्णांक i;
    क्रम (i = 0; i < Max_गिनती_CPUs; i+=pkgcpus) {
        स_रखो(cpu, 0, माप(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    पूर्णांक entrycount = cpu - cpus;

    // PCI bus
    काष्ठा mpt_bus *buses = (व्योम*)cpu, *bus = buses;
    अगर (!hlist_empty(&PCIDevices)) {
        स_रखो(bus, 0, माप(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        स_नकल(bus->bustype, "PCI   ", माप(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    पूर्णांक isabusid = bus - buses;
    स_रखो(bus, 0, माप(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    स_नकल(bus->bustype, "ISA   ", माप(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    काष्ठा mpt_ioapic *ioapic = (व्योम*)bus;
    स_रखो(ioapic, 0, माप(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    काष्ठा mpt_intsrc *intsrcs = (व्योम*)&ioapic[1], *intsrc = intsrcs;
    पूर्णांक dev = -1;
    अचिन्हित लघु pinmask = 0;

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        अगर (pci_bdf_to_bus(bdf) != 0)
            अवरोध;
        पूर्णांक pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        पूर्णांक irq = pci_config_readb(bdf, PCI_INTERRUPT_पंक्ति);
        अगर (pin == 0)
            जारी;
        अगर (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        अगर (pinmask & (1 << pin)) /* pin was seen already */
            जारी;
        pinmask |= (1 << pin);
        स_रखो(intsrc, 0, माप(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    पूर्णांक irq0_override = romfile_loadint("etc/irq0-override", 0);
    क्रम (i = 0; i < 16; i++) {
        स_रखो(intsrc, 0, माप(*intsrc));
        अगर (BUILD_PCI_IRQS & (1 << i))
            जारी;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        अगर (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            अगर (i == 0)
                intsrc->dstirq = 2;
            अन्यथा अगर (i == 2)
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
    पूर्णांक लंबाई = (व्योम*)intsrc - (व्योम*)config;
    config->entrycount = entrycount;
    config->लंबाई = लंबाई;
    config->checksum -= checksum(config, लंबाई);

    // floating pointer structure
    काष्ठा mptable_floating_s floating;
    स_रखो(&floating, 0, माप(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.लंबाई = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, माप(floating));
    copy_mptable(&floating);
    मुक्त(config);
}
