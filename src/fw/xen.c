// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_XEN
#অন্তর্ভাব "e820map.h" // e820_add
#অন্তর্ভাব "hw/serialio.h" // DebugOutputPort
#অন্তর্ভাব "malloc.h" // memalign_high
#অন্তর্ভাব "memmap.h" // PAGE_SIZE
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "paravirt.h" // PlatformRunningOn
#অন্তর্ভাব "string.h" // memcpy
#অন্তর্ভাব "util.h" // copy_acpi_rsdp
#অন্তর্ভাব "x86.h" // cpuid
#অন্তর্ভাব "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

সংরচনা xen_seabios_info {
    বর্ণ signature[14]; /* XenHVMSeaBIOS\0 */
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

স্থির সংরচনা xen_seabios_info *validate_info(শূন্য)
{
    সংরচনা xen_seabios_info *t = (শূন্য *)INFO_PHYSICAL_ADDRESS;

    যদি ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        প্রত্যাগম রিক্ত;
    }

    যদি ( t->length < পরিমাণ(সংরচনা xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        প্রত্যাগম রিক্ত;
    }

    যদি (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        প্রত্যাগম রিক্ত;
    }
    প্রত্যাগম t;
}

শূন্য xen_preinit(শূন্য)
{
    u32 base, eax, ebx, ecx, edx;
    বর্ণ signature[13];

    যদি (!CONFIG_XEN)
        প্রত্যাগম;

    প্রতি (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        যদি (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            যদি ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            ভঙ্গ;
        }
    }
    যদি (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        প্রত্যাগম;
    }
    যদি (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    অন্যথা
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

স্থির পূর্ণাঙ্ক hypercall_xen_version( পূর্ণাঙ্ক cmd, শূন্য *arg)
{
    প্রত্যাগম _hypercall2(পূর্ণাঙ্ক, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
শূন্য xen_hypercall_setup(শূন্য)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    যদি (!runningOnXen())
        প্রত্যাগম;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_উপরের(PAGE_SIZE, eax*PAGE_SIZE);
    যদি (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    প্রতি ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

শূন্য xen_biostable_setup(শূন্য)
{
    সংরচনা xen_seabios_info *info = validate_info();
    শূন্য **tables;
    পূর্ণাঙ্ক i;

    যদি (!info)
        panic("Xen info corrupted\n");

    tables = (শূন্য*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    প্রতি (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

শূন্য xen_ramsize_preinit(শূন্য)
{
    পূর্ণাঙ্ক i;
    সংরচনা xen_seabios_info *info = validate_info();
    সংরচনা e820entry *e820;

    যদি (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (সংরচনা e820entry *)info->e820;
    প্রতি (i = 0; i < info->e820_nr; i++) {
        সংরচনা e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
