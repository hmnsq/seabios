// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_XEN
#അന്തര്ഭാവ "e820map.h" // e820_add
#അന്തര്ഭാവ "hw/serialio.h" // DebugOutputPort
#അന്തര്ഭാവ "malloc.h" // memalign_high
#അന്തര്ഭാവ "memmap.h" // PAGE_SIZE
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "paravirt.h" // PlatformRunningOn
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "util.h" // copy_acpi_rsdp
#അന്തര്ഭാവ "x86.h" // cpuid
#അന്തര്ഭാവ "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

ഘടന xen_seabios_info {
    അക്ഷരം signature[14]; /* XenHVMSeaBIOS\0 */
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

സ്ഥിതം ഘടന xen_seabios_info *validate_info(ശൂന്യം)
{
    ഘടന xen_seabios_info *t = (ശൂന്യം *)INFO_PHYSICAL_ADDRESS;

    എങ്കിൽ ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        തിരികെ ശൂന്യ;
    }

    എങ്കിൽ ( t->length < വലുപ്പം(ഘടന xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        തിരികെ ശൂന്യ;
    }

    എങ്കിൽ (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        തിരികെ ശൂന്യ;
    }
    തിരികെ t;
}

ശൂന്യം xen_preinit(ശൂന്യം)
{
    u32 base, eax, ebx, ecx, edx;
    അക്ഷരം signature[13];

    എങ്കിൽ (!CONFIG_XEN)
        തിരികെ;

    വേണ്ടി (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        എങ്കിൽ (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            എങ്കിൽ ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            നിർത്തുക;
        }
    }
    എങ്കിൽ (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        തിരികെ;
    }
    എങ്കിൽ (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    അല്ലെങ്കിൽ
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ hypercall_xen_version( പൂർണ്ണസംഖ്യ cmd, ശൂന്യം *arg)
{
    തിരികെ _hypercall2(പൂർണ്ണസംഖ്യ, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
ശൂന്യം xen_hypercall_setup(ശൂന്യം)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    എങ്കിൽ (!runningOnXen())
        തിരികെ;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_മുകളിലെ(PAGE_SIZE, eax*PAGE_SIZE);
    എങ്കിൽ (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    വേണ്ടി ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

ശൂന്യം xen_biostable_setup(ശൂന്യം)
{
    ഘടന xen_seabios_info *info = validate_info();
    ശൂന്യം **tables;
    പൂർണ്ണസംഖ്യ i;

    എങ്കിൽ (!info)
        panic("Xen info corrupted\n");

    tables = (ശൂന്യം*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    വേണ്ടി (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

ശൂന്യം xen_ramsize_preinit(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ i;
    ഘടന xen_seabios_info *info = validate_info();
    ഘടന e820entry *e820;

    എങ്കിൽ (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (ഘടന e820entry *)info->e820;
    വേണ്ടി (i = 0; i < info->e820_nr; i++) {
        ഘടന e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
