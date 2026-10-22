// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_XEN
#ଅନ୍ତର୍ଭାଵ "e820map.h" // e820_add
#ଅନ୍ତର୍ଭାଵ "hw/serialio.h" // DebugOutputPort
#ଅନ୍ତର୍ଭାଵ "malloc.h" // memalign_high
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SIZE
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "paravirt.h" // PlatformRunningOn
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "util.h" // copy_acpi_rsdp
#ଅନ୍ତର୍ଭାଵ "x86.h" // cpuid
#ଅନ୍ତର୍ଭାଵ "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

ସଂରଚନା xen_seabios_info {
    ଅକ୍ଷର signature[14]; /* XenHVMSeaBIOS\0 */
    u8 length;     /* Length of this struct */
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

ସ୍ଥିର ସଂରଚନା xen_seabios_info *validate_info(ଶୂନ୍ୟ)
{
    ସଂରଚନା xen_seabios_info *t = (ଶୂନ୍ୟ *)INFO_PHYSICAL_ADDRESS;

    ଯଦି ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        ଫେରନ୍ତୁ ଖାଲି;
    }

    ଯଦି ( t->length < ଆକାର(ସଂରଚନା xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        ଫେରନ୍ତୁ ଖାଲି;
    }

    ଯଦି (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ଫେରନ୍ତୁ t;
}

ଶୂନ୍ୟ xen_preinit(ଶୂନ୍ୟ)
{
    u32 base, eax, ebx, ecx, edx;
    ଅକ୍ଷର signature[13];

    ଯଦି (!CONFIG_XEN)
        ଫେରନ୍ତୁ;

    ପାଇଁ (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        ଯଦି (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            ଯଦି ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            ଭାଙ୍ଗ;
        }
    }
    ଯଦି (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        ଫେରନ୍ତୁ;
    }
    ଯଦି (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    ଅନ୍ୟଥା
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hypercall_xen_version( ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd, ଶୂନ୍ୟ *arg)
{
    ଫେରନ୍ତୁ _hypercall2(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
ଶୂନ୍ୟ xen_hypercall_setup(ଶୂନ୍ୟ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    ଯଦି (!runningOnXen())
        ଫେରନ୍ତୁ;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_ଉପର(PAGE_SIZE, eax*PAGE_SIZE);
    ଯଦି (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    ପାଇଁ ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

ଶୂନ୍ୟ xen_biostable_setup(ଶୂନ୍ୟ)
{
    ସଂରଚନା xen_seabios_info *info = validate_info();
    ଶୂନ୍ୟ **tables;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    ଯଦି (!info)
        panic("Xen info corrupted\n");

    tables = (ଶୂନ୍ୟ*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    ପାଇଁ (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

ଶୂନ୍ୟ xen_ramsize_preinit(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ସଂରଚନା xen_seabios_info *info = validate_info();
    ସଂରଚନା e820entry *e820;

    ଯଦି (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (ସଂରଚନା e820entry *)info->e820;
    ପାଇଁ (i = 0; i < info->e820_nr; i++) {
        ସଂରଚନା e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
