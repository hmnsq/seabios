// MPTable generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "hw/pci.h" // pci_bdf_to_bus
#ضمن "hw/pcidevice.h" // foreachpci
#ضمن "hw/pci_regs.h" // PCI_INTERRUPT_PIN
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadint
#ضمن "std/mptable.h" // MPTABLE_SIGNATURE
#ضمن "string.h" // memset
#ضمن "util.h" // MaxCountCPUs
#ضمن "x86.h" // cpuid

فراغ
mptable_setup(فراغ)
{
    إذا (! CONFIG_MPTABLE)
        أرجع;

    dprintf(3, "init MPTable\n");

    // Config structure in temp area.
    بنية mptable_config_s *config = malloc_tmp(32*1024);
    إذا (!config) {
        warn_noalloc();
        أرجع;
    }
    memset(config, 0, حجم(*config));
    config->signature = MPCONFIG_SIGNATURE;
    config->spec = 4;
    memcpy(config->oemid, BUILD_CPUNAME8, حجم(config->oemid));
    memcpy(config->productid, "0.1         ", حجم(config->productid));
    config->lapic = BUILD_APIC_ADDR;

    // Detect cpu info
    u32 cpuid_signature, ebx, ecx, cpuid_features;
    cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
    إذا (! cpuid_signature) {
        // Use default values.
        cpuid_signature = 0x600;
        cpuid_features = 0x201;
    }
    صحيح pkgcpus = 1;
    إذا (cpuid_features & (1 << 28)) {
        /* Only populate the MPS tables with the first logical CPU in
           each package */
        pkgcpus = (ebx >> 16) & 0xff;
        pkgcpus = 1 << (__fls(pkgcpus - 1) + 1); /* round up to power of 2 */
    }
    u8 apic_version = readl((u8*)BUILD_APIC_ADDR + 0x30) & 0xff;

    // CPU definitions.
    بنية mpt_cpu *cpus = (فراغ*)&config[1], *cpu = cpus;
    صحيح i;
    لكل (i = 0; i < Max_كدي_CPUs; i+=pkgcpus) {
        memset(cpu, 0, حجم(*cpu));
        cpu->type = MPT_TYPE_CPU;
        cpu->apicid = i;
        cpu->apicver = apic_version;
        /* cpu flags: enabled, bootstrap cpu */
        cpu->cpuflag = (apic_id_is_present(i) ? 0x01 : 0x00) | ((i==0) ? 0x02 : 0x00);
        cpu->cpusignature = cpuid_signature;
        cpu->featureflag = cpuid_features;
        cpu++;
    }
    صحيح entrycount = cpu - cpus;

    // PCI bus
    بنية mpt_bus *buses = (فراغ*)cpu, *bus = buses;
    إذا (!hlist_empty(&PCIDevices)) {
        memset(bus, 0, حجم(*bus));
        bus->type = MPT_TYPE_BUS;
        bus->busid = 0;
        memcpy(bus->bustype, "PCI   ", حجم(bus->bustype));
        bus++;
        entrycount++;
    }

    /* isa bus */
    صحيح isabusid = bus - buses;
    memset(bus, 0, حجم(*bus));
    bus->type = MPT_TYPE_BUS;
    bus->busid = isabusid;
    memcpy(bus->bustype, "ISA   ", حجم(bus->bustype));
    bus++;
    entrycount++;

    /* ioapic */
    u8 ioapic_id = BUILD_IOAPIC_ID;
    بنية mpt_ioapic *ioapic = (فراغ*)bus;
    memset(ioapic, 0, حجم(*ioapic));
    ioapic->type = MPT_TYPE_IOAPIC;
    ioapic->apicid = ioapic_id;
    ioapic->apicver = 0x11;
    ioapic->flags = 1; // enable
    ioapic->apicaddr = BUILD_IOAPIC_ADDR;
    entrycount++;

    /* irqs */
    بنية mpt_intsrc *intsrcs = (فراغ*)&ioapic[1], *intsrc = intsrcs;
    صحيح dev = -1;
    unsigned short pinmask = 0;

    بنية pci_device *pci;
    foreachpci(pci) {
        u16 bdf = pci->bdf;
        إذا (pci_bdf_to_bus(bdf) != 0)
            اكسر;
        صحيح pin = pci_config_readb(bdf, PCI_INTERRUPT_PIN);
        صحيح irq = pci_config_readb(bdf, PCI_INTERRUPT_لي);
        إذا (pin == 0)
            استمر;
        إذا (dev != pci_bdf_to_busdev(bdf)) {
            dev = pci_bdf_to_busdev(bdf);
            pinmask = 0;
        }
        إذا (pinmask & (1 << pin)) /* pin was seen already */
            استمر;
        pinmask |= (1 << pin);
        memset(intsrc, 0, حجم(*intsrc));
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 1; /* active high */
        intsrc->srcbus = pci_bdf_to_bus(bdf); /* PCI bus */
        intsrc->srcbusirq = (pci_bdf_to_dev(bdf) << 2) | (pin - 1);
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = irq;
        intsrc++;
    }

    صحيح irq0_override = romfile_loadint("etc/irq0-override", 0);
    لكل (i = 0; i < 16; i++) {
        memset(intsrc, 0, حجم(*intsrc));
        إذا (BUILD_PCI_IRQS & (1 << i))
            استمر;
        intsrc->type = MPT_TYPE_INTSRC;
        intsrc->irqtype = 0; /* INT */
        intsrc->irqflag = 0; /* conform to bus spec */
        intsrc->srcbus = isabusid; /* ISA bus */
        intsrc->srcbusirq = i;
        intsrc->dstapic = ioapic_id;
        intsrc->dstirq = i;
        إذا (irq0_override) {
            /* Destination 2 is covered by irq0->inti2 override (i ==
               0). Source IRQ 2 is unused */
            إذا (i == 0)
                intsrc->dstirq = 2;
            وإلا إذا (i == 2)
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
    صحيح length = (فراغ*)intsrc - (فراغ*)config;
    config->entrycount = entrycount;
    config->length = length;
    config->checksum -= checksum(config, length);

    // floating pointer structure
    بنية mptable_floating_s floating;
    memset(&floating, 0, حجم(floating));
    floating.signature = MPTABLE_SIGNATURE;
    floating.physaddr = (u32)config;
    floating.length = 1;
    floating.spec_rev = 4;
    floating.checksum -= checksum(&floating, حجم(floating));
    copy_mptable(&floating);
    حرر(config);
}
