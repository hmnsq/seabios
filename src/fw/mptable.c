// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_bdf_to_bus
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "std/mptable.h" // MPTABLE_SIGNATURE
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // MaxCountCPUs
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // cpuid

ਖਾਲੀ
mptable_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_MPTABLE)
        ਵਾਪਸ;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    ਬਣਤਰ mptable_config_s *config = malloc_tmp(32*1024);
    ਜੇ (!config) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(config, 0, ਆਕਾਰ(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(config->oemid, BUILD_CPUNAME8, ਆਕਾਰ(config->oemid));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(config->productid, "0.1         ", ਆਕਾਰ(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    ਜੇ (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    ਪੂਰਨ pkgcpus = 1;
    ਜੇ (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_ਸੰਸਕਰਣ = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    ਬਣਤਰ mpt_cpu *cpus = (ਖਾਲੀ*)&config[1], *cpu = cpus;
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < Max_ਗਿਣਤੀ_CPUs; i+=pkgcpus) {
        memset(cpu, 0, ਆਕਾਰ(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_ਸੰਸਕਰਣ;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    ਪੂਰਨ entrycount = cpu - cpus;

    // PCI bus
    ਬਣਤਰ mpt_bus *buses = (ਖਾਲੀ*)cpu, *bus = buses;
    ਜੇ (!hlist_ਖਾਲੀ_ਥਾਵਾਂ(&PCIDevices)) {
        memset(bus, 0, ਆਕਾਰ(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(bus->bustype, "PCI   ", ਆਕਾਰ(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    ਪੂਰਨ isabusid = bus - buses;
    memset(bus, 0, ਆਕਾਰ(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(bus->bustype, "ISA   ", ਆਕਾਰ(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    ਬਣਤਰ mpt_ioapic *ioapic = (ਖਾਲੀ*)bus;
    memset(ioapic, 0, ਆਕਾਰ(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    ਬਣਤਰ mpt_intsrc *intsrcs = (ਖਾਲੀ*)&ioapic[1], *intsrc = intsrcs;
    ਪੂਰਨ dev = -1;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ pinmask = 0;

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        ਜੇ (pci_bdf_ਮੰਜ਼ਿਲ_bus(bdf) != 0)
            ਰੋਕੋ;
        ਪੂਰਨ pin = pci_config_readb(bdf, PCI_ਰੁਕਾਵਟ_PIN);
        ਪੂਰਨ irq = pci_config_readb(bdf, PCI_ਰੁਕਾਵਟ_ਸਤਰ);
        ਜੇ (pin == 0)
            ਜਾਰੀ;
        ਜੇ (dev != pci_bdf_ਮੰਜ਼ਿਲ_busdev(bdf)) {
            dev = pci_bdf_ਮੰਜ਼ਿਲ_busdev(bdf);
            pinmask = 0;
        }
        ਜੇ (pinmask & (1 << pin)) /* pin was seen already */
            ਜਾਰੀ;
        pinmask |= (1 << pin);
        memset(intsrc, 0, ਆਕਾਰ(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_ਮੰਜ਼ਿਲ_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_ਮੰਜ਼ਿਲ_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    ਪੂਰਨ irq0_override = romfile_loadint("etc/irq0-override", 0);
    ਲਈ (i = 0; i < 16; i++) {
        memset(intsrc, 0, ਆਕਾਰ(*intsrc));
        ਜੇ (BUILD_PCI_IRQS & (1 << i))
            ਜਾਰੀ;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        ਜੇ (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            ਜੇ (i == 0)
                intsrc->dstirq = 2;
            ਨਹੀਂਤਾਂ ਜੇ (i == 2)
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
    ਪੂਰਨ ਲੰਮਾਈ = (ਖਾਲੀ*)intsrc - (ਖਾਲੀ*)config;
    config->entrycount = entrycount;
    config->ਲੰਮਾਈ = ਲੰਮਾਈ;
    config->checksum -= checksum(config, ਲੰਮਾਈ);

    // floating pointer structure
    ਬਣਤਰ mptable_floating_s floating;
    memset(&floating, 0, ਆਕਾਰ(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.ਲੰਮਾਈ = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, ਆਕਾਰ(floating));
    copy_mptable(&floating);
    ਮੁਕਤ(config);
}
