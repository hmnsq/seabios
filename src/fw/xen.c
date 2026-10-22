// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_XEN
#అన్తర్భావ "e820map.h" // e820_add
#అన్తర్భావ "hw/serialio.h" // DebugOutputPort
#అన్తర్భావ "malloc.h" // memalign_high
#అన్తర్భావ "memmap.h" // PAGE_SIZE
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // PlatformRunningOn
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "util.h" // copy_acpi_rsdp
#అన్తర్భావ "x86.h" // cpuid
#అన్తర్భావ "xen.h" // xen_extraversion_t

#నిర్వచించు INFO_PHYSICAL_చిరునామా 0x00001000

u32 xen_cpuid_పునాది = 0;
గుర్తులేని దీర్ఘ xen_hypercall_పుట = 0;

నిర్మాణం xen_seabios_info {
    అక్షరం signature[14]; /* XenHVMSeaBIOS\0 */
    u8 నిడివి;     /* Length of this struct */
    u8 checksum;   /* Set such that the sum over bytes 0..length == 0 */
    /*
     * Physical address of an array of tables_nr elements.
     *
     * Each element is a 32 bit value contianing the physical address
     * of a BIOS table.
     */
    u32 tables;
    u32 tables_nr;
    /*
     * Physical address of the e820 table, contains e820_nr entries.
     */
    u32 e820;
    u32 e820_nr;
} PACKED;

స్థిర నిర్మాణం xen_seabios_info *validate_info(శూన్య)
{
    నిర్మాణం xen_seabios_info *t = (శూన్య *)INFO_PHYSICAL_చిరునామా;

    ఒకవేళ ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        తిరిగి ఖాళీ;
    }

    ఒకవేళ ( t->నిడివి < పరిమాణం(నిర్మాణం xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        తిరిగి ఖాళీ;
    }

    ఒకవేళ (checksum(t, t->నిడివి) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        తిరిగి ఖాళీ;
    }
    తిరిగి t;
}

శూన్య xen_preinit(శూన్య)
{
    u32 పునాది, eax, ebx, ecx, edx;
    అక్షరం signature[13];

    ఒకవేళ (!CONFIG_XEN)
        తిరిగి;

    కొరకు (పునాది = 0x40000000; పునాది < 0x40010000; పునాది += 0x100) {
        cpuid(పునాది, &eax, &ebx, &ecx, &edx);
        జ్ఞాపకం_నకలు(signature + 0, &ebx, 4);
        జ్ఞాపకం_నకలు(signature + 4, &ecx, 4);
        జ్ఞాపకం_నకలు(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, పునాది);
        ఒకవేళ (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            సంకేతం_mutable_preinit();
            Debug_Output_పోర్టు = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", పునాది);
            ఒకవేళ ((eax - పునాది) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, పునాది);
            xen_cpuid_పునాది = పునాది;
            ఆపు;
        }
    }
    ఒకవేళ (!xen_cpuid_పునాది) {
        dprintf(1, "No Xen hypervisor found.\n");
        తిరిగి;
    }
    ఒకవేళ (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    లేకపోతే
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

స్థిర పూర్ణాంకం hypercall_xen_సంస్కరణ( పూర్ణాంకం cmd, శూన్య *arg)
{
    తిరిగి _hypercall2(పూర్ణాంకం, xen_సంస్కరణ, cmd, arg);
}

/* Fill in hypercall transfer pages. */
శూన్య xen_hypercall_అమరిక(శూన్య)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    గుర్తులేని దీర్ఘ i;

    ఒకవేళ (!runningOnXen())
        తిరిగి;

    cpuid(xen_cpuid_పునాది + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_పుట = (గుర్తులేని దీర్ఘ)memalign_ఎగువ(పుట_కొలత, eax*పుట_కొలత);
    ఒకవేళ (!xen_hypercall_పుట)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_పుట);
    కొరకు ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_పుట + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_పునాది + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_సంస్కరణ(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

శూన్య xen_biostable_అమరిక(శూన్య)
{
    నిర్మాణం xen_seabios_info *info = validate_info();
    శూన్య **tables;
    పూర్ణాంకం i;

    ఒకవేళ (!info)
        panic("Xen info corrupted\n");

    tables = (శూన్య*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    కొరకు (i=0; i<info->tables_nr; i++)
        copy_పట్టిక(tables[i]);

    కనుగొను_acpi_features();
}

శూన్య xen_ramsize_preinit(శూన్య)
{
    పూర్ణాంకం i;
    నిర్మాణం xen_seabios_info *info = validate_info();
    నిర్మాణం e820entry *e820;

    ఒకవేళ (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (నిర్మాణం e820entry *)info->e820;
    కొరకు (i = 0; i < info->e820_nr; i++) {
        నిర్మాణం e820entry *e = &e820[i];
        e820_కూడు(e->ప్రారంభం, e->కొలత, e->type);
    }
}
