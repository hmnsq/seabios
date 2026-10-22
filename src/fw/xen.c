// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_XEN
#ضمن "e820map.h" // e820_add
#ضمن "hw/serialio.h" // DebugOutputPort
#ضمن "malloc.h" // memalign_high
#ضمن "memmap.h" // PAGE_SIZE
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // PlatformRunningOn
#ضمن "string.h" // memcpy
#ضمن "util.h" // copy_acpi_rsdp
#ضمن "x86.h" // cpuid
#ضمن "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

بنية xen_seabios_info {
    حرف signature[14]; /* XenHVMSeaBIOS\0 */
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

ساكن بنية xen_seabios_info *validate_info(فراغ)
{
    بنية xen_seabios_info *t = (فراغ *)INFO_PHYSICAL_ADDRESS;

    ܐܢ ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        ܦܢܐ عدم;
    }

    ܐܢ ( t->length < حجم(بنية xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        ܦܢܐ عدم;
    }

    ܐܢ (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        ܦܢܐ عدم;
    }
    ܦܢܐ t;
}

فراغ xen_preinit(فراغ)
{
    u32 base, eax, ebx, ecx, edx;
    حرف signature[13];

    ܐܢ (!CONFIG_XEN)
        ܦܢܐ;

    لكل (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        ܐܢ (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            ܐܢ ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            اكسر;
        }
    }
    ܐܢ (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        ܦܢܐ;
    }
    ܐܢ (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    ܐܚܪܢܐ
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ساكن صحيح hypercall_xen_version( صحيح cmd, فراغ *arg)
{
    ܦܢܐ _hypercall2(صحيح, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
فراغ xen_hypercall_setup(فراغ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    ܐܢ (!runningOnXen())
        ܦܢܐ;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_high(PAGE_SIZE, eax*PAGE_SIZE);
    ܐܢ (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    لكل ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

فراغ xen_biostable_setup(فراغ)
{
    بنية xen_seabios_info *info = validate_info();
    فراغ **tables;
    صحيح i;

    ܐܢ (!info)
        panic("Xen info corrupted\n");

    tables = (فراغ*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    لكل (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

فراغ xen_ramsize_preinit(فراغ)
{
    صحيح i;
    بنية xen_seabios_info *info = validate_info();
    بنية e820entry *e820;

    ܐܢ (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (بنية e820entry *)info->e820;
    لكل (i = 0; i < info->e820_nr; i++) {
        بنية e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
