// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "hw/pci.h" // pci_bdf_to_bus
#અન્તર્ભાવ "hw/pcidevice.h" // foreachpci
#અન્તર્ભાવ "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "std/mptable.h" // MPTABLE_SIGNATURE
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // MaxCountCPUs
#અન્તર્ભાવ "x86.h" // cpuid

શૂન્ય
mptable_setup(શૂન્ય)
{
    જો (! CONFIG_MPTABLE)
        પાછા;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    સંરચના mptable_config_s *config = malloc_tmp(32*1024);
    જો (!config) {
        warn_noalloc();
        પાછા;
    }
    memset(config, 0, કદ(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    memcpy(config->oemid, BUILD_CPUNAME8, કદ(config->oemid));
    memcpy(config->productid, "0.1         ", કદ(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    જો (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    પૂર્ણાંક pkgcpus = 1;
    જો (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    સંરચના mpt_cpu *cpus = (શૂન્ય*)&config[1], *cpu = cpus;
    પૂર્ણાંક i;
    માટે (i = 0; i < Max_ગણતરી_CPUs; i+=pkgcpus) {
        memset(cpu, 0, કદ(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    પૂર્ણાંક entrycount = cpu - cpus;

    // PCI bus
    સંરચના mpt_bus *buses = (શૂન્ય*)cpu, *bus = buses;
    જો (!hlist_empty(&PCIDevices)) {
        memset(bus, 0, કદ(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        memcpy(bus->bustype, "PCI   ", કદ(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    પૂર્ણાંક isabusid = bus - buses;
    memset(bus, 0, કદ(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    memcpy(bus->bustype, "ISA   ", કદ(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    સંરચના mpt_ioapic *ioapic = (શૂન્ય*)bus;
    memset(ioapic, 0, કદ(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    સંરચના mpt_intsrc *intsrcs = (શૂન્ય*)&ioapic[1], *intsrc = intsrcs;
    પૂર્ણાંક dev = -1;
    unsigned short pinmask = 0;

    સંરચના pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        જો (pci_bdf_to_bus(bdf) != 0)
            તોડો;
        પૂર્ણાંક pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        પૂર્ણાંક irq = pci_config_readb(bdf, PCI_INTERRUPT_લીટી);
        જો (pin == 0)
            ચાલુ;
        જો (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        જો (pinmask & (1 << pin)) /* pin was seen already */
            ચાલુ;
        pinmask |= (1 << pin);
        memset(intsrc, 0, કદ(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    પૂર્ણાંક irq0_override = romfile_loadint("etc/irq0-override", 0);
    માટે (i = 0; i < 16; i++) {
        memset(intsrc, 0, કદ(*intsrc));
        જો (BUILD_PCI_IRQS & (1 << i))
            ચાલુ;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        જો (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            જો (i == 0)
                intsrc->dstirq = 2;
            નહીંતર જો (i == 2)
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
    પૂર્ણાંક length = (શૂન્ય*)intsrc - (શૂન્ય*)config;
    config->entrycount = entrycount;
    config->length = length;
    config->checksum -= checksum(config, length);

    // floating pointer structure
    સંરચના mptable_floating_s floating;
    memset(&floating, 0, કદ(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.length = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, કદ(floating));
    copy_mptable(&floating);
    મુક્ત(config);
}
