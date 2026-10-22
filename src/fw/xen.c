// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_XEN
#شامل "e820map.h" // e820_add
#شامل "hw/serialio.h" // DebugOutputPort
#شامل "malloc.h" // memalign_high
#شامل "memmap.h" // PAGE_SIZE
#شامل "output.h" // dprintf
#شامل "paravirt.h" // PlatformRunningOn
#شامل "string.h" // memcpy
#شامل "util.h" // copy_acpi_rsdp
#شامل "x86.h" // cpuid
#شامل "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

ساختار xen_seabios_info {
    نویسه signature[14]; /* XenHVMSeaBIOS\0 */
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

ایستا ساختار xen_seabios_info *validate_info(تهی)
{
    ساختار xen_seabios_info *t = (تهی *)INFO_PHYSICAL_ADDRESS;

    اگر ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        بازگشت پوچ;
    }

    اگر ( t->length < اندازه(ساختار xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        بازگشت پوچ;
    }

    اگر (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        بازگشت پوچ;
    }
    بازگشت t;
}

تهی xen_preinit(تهی)
{
    u32 base, eax, ebx, ecx, edx;
    نویسه signature[13];

    اگر (!CONFIG_XEN)
        بازگشت;

    برای (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        اگر (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            اگر ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            بشکن;
        }
    }
    اگر (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        بازگشت;
    }
    اگر (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    وگرنه
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ایستا صحیح hypercall_xen_version( صحیح cmd, تهی *arg)
{
    بازگشت _hypercall2(صحیح, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
تهی xen_hypercall_setup(تهی)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    اگر (!runningOnXen())
        بازگشت;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_بالا(PAGE_SIZE, eax*PAGE_SIZE);
    اگر (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    برای ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

تهی xen_biostable_setup(تهی)
{
    ساختار xen_seabios_info *info = validate_info();
    تهی **tables;
    صحیح i;

    اگر (!info)
        panic("Xen info corrupted\n");

    tables = (تهی*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    برای (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

تهی xen_ramsize_preinit(تهی)
{
    صحیح i;
    ساختار xen_seabios_info *info = validate_info();
    ساختار e820entry *e820;

    اگر (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (ساختار e820entry *)info->e820;
    برای (i = 0; i < info->e820_nr; i++) {
        ساختار e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
