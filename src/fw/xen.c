// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_XEN
#અન્તર્ભાવ "e820map.h" // e820_add
#અન્તર્ભાવ "hw/serialio.h" // DebugOutputPort
#અન્તર્ભાવ "malloc.h" // memalign_high
#અન્તર્ભાવ "memmap.h" // PAGE_SIZE
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "paravirt.h" // PlatformRunningOn
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "util.h" // copy_acpi_rsdp
#અન્તર્ભાવ "x86.h" // cpuid
#અન્તર્ભાવ "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

સંરચના xen_seabios_info {
    અક્ષર signature[14]; /* XenHVMSeaBIOS\0 */
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

સ્થિર સંરચના xen_seabios_info *validate_info(શૂન્ય)
{
    સંરચના xen_seabios_info *t = (શૂન્ય *)INFO_PHYSICAL_ADDRESS;

    જો ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        પાછા ખાલી;
    }

    જો ( t->length < કદ(સંરચના xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        પાછા ખાલી;
    }

    જો (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        પાછા ખાલી;
    }
    પાછા t;
}

શૂન્ય xen_preinit(શૂન્ય)
{
    u32 base, eax, ebx, ecx, edx;
    અક્ષર signature[13];

    જો (!CONFIG_XEN)
        પાછા;

    માટે (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        જો (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            જો ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            તોડો;
        }
    }
    જો (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        પાછા;
    }
    જો (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    નહીંતર
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

સ્થિર પૂર્ણાંક hypercall_xen_version( પૂર્ણાંક cmd, શૂન્ય *arg)
{
    પાછા _hypercall2(પૂર્ણાંક, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
શૂન્ય xen_hypercall_setup(શૂન્ય)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    જો (!runningOnXen())
        પાછા;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_ઉપલું(PAGE_SIZE, eax*PAGE_SIZE);
    જો (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    માટે ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

શૂન્ય xen_biostable_setup(શૂન્ય)
{
    સંરચના xen_seabios_info *info = validate_info();
    શૂન્ય **tables;
    પૂર્ણાંક i;

    જો (!info)
        panic("Xen info corrupted\n");

    tables = (શૂન્ય*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    માટે (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

શૂન્ય xen_ramsize_preinit(શૂન્ય)
{
    પૂર્ણાંક i;
    સંરચના xen_seabios_info *info = validate_info();
    સંરચના e820entry *e820;

    જો (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (સંરચના e820entry *)info->e820;
    માટે (i = 0; i < info->e820_nr; i++) {
        સંરચના e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
