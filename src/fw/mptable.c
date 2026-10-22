// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_bdf_to_bus
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "std/mptable.h" // MPTABLE_SIGNATURE
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // MaxCountCPUs
#ಅನ್ತರ್ಭಾವ "x86.h" // cpuid

ಶೂನ್ಯ
mptable_setup(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_MPTABLE)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    ಸಂರಚನಾ mptable_config_s *config = malloc_tmp(32*1024);
    ಯದಿ (!config) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(config, 0, ಪರಿಮಾಣ(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    memcpy(config->oemid, BUILD_CPUNAME8, ಪರಿಮಾಣ(config->oemid));
    memcpy(config->productid, "0.1         ", ಪರಿಮಾಣ(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    ಯದಿ (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    ಪೂರ್ಣಾಙ್ಕ pkgcpus = 1;
    ಯದಿ (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    ಸಂರಚನಾ mpt_cpu *cpus = (ಶೂನ್ಯ*)&config[1], *cpu = cpus;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < Max_ಎಣಿಕೆ_CPUs; i+=pkgcpus) {
        memset(cpu, 0, ಪರಿಮಾಣ(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    ಪೂರ್ಣಾಙ್ಕ entrycount = cpu - cpus;

    // PCI bus
    ಸಂರಚನಾ mpt_bus *buses = (ಶೂನ್ಯ*)cpu, *bus = buses;
    ಯದಿ (!hlist_empty(&PCIDevices)) {
        memset(bus, 0, ಪರಿಮಾಣ(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        memcpy(bus->bustype, "PCI   ", ಪರಿಮಾಣ(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    ಪೂರ್ಣಾಙ್ಕ isabusid = bus - buses;
    memset(bus, 0, ಪರಿಮಾಣ(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    memcpy(bus->bustype, "ISA   ", ಪರಿಮಾಣ(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    ಸಂರಚನಾ mpt_ioapic *ioapic = (ಶೂನ್ಯ*)bus;
    memset(ioapic, 0, ಪರಿಮಾಣ(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    ಸಂರಚನಾ mpt_intsrc *intsrcs = (ಶೂನ್ಯ*)&ioapic[1], *intsrc = intsrcs;
    ಪೂರ್ಣಾಙ್ಕ dev = -1;
    unsigned short pinmask = 0;

    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        ಯದಿ (pci_bdf_to_bus(bdf) != 0)
            ಭಙ್ಗ;
        ಪೂರ್ಣಾಙ್ಕ pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        ಪೂರ್ಣಾಙ್ಕ irq = pci_config_readb(bdf, PCI_INTERRUPT_ಸಾಲು);
        ಯದಿ (pin == 0)
            ಅನುವರ್ತ;
        ಯದಿ (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        ಯದಿ (pinmask & (1 << pin)) /* pin was seen already */
            ಅನುವರ್ತ;
        pinmask |= (1 << pin);
        memset(intsrc, 0, ಪರಿಮಾಣ(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    ಪೂರ್ಣಾಙ್ಕ irq0_override = romfile_loadint("etc/irq0-override", 0);
    ಪ್ರತಿ (i = 0; i < 16; i++) {
        memset(intsrc, 0, ಪರಿಮಾಣ(*intsrc));
        ಯದಿ (BUILD_PCI_IRQS & (1 << i))
            ಅನುವರ್ತ;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        ಯದಿ (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            ಯದಿ (i == 0)
                intsrc->dstirq = 2;
            ಅನ್ಯಥಾ ಯದಿ (i == 2)
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
    ಪೂರ್ಣಾಙ್ಕ length = (ಶೂನ್ಯ*)intsrc - (ಶೂನ್ಯ*)config;
    config->entrycount = entrycount;
    config->length = length;
    config->checksum -= checksum(config, length);

    // floating pointer structure
    ಸಂರಚನಾ mptable_floating_s floating;
    memset(&floating, 0, ಪರಿಮಾಣ(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.length = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, ಪರಿಮಾಣ(floating));
    copy_mptable(&floating);
    ಮುಕ್ತ(config);
}
