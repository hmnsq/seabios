// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_XEN
#அந்தர்பாவ "e820map.h" // e820_add
#அந்தர்பாவ "hw/serialio.h" // DebugOutputPort
#அந்தர்பாவ "malloc.h" // memalign_high
#அந்தர்பாவ "memmap.h" // PAGE_SIZE
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "paravirt.h" // PlatformRunningOn
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "util.h" // copy_acpi_rsdp
#அந்தர்பாவ "x86.h" // cpuid
#அந்தர்பாவ "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

கட்டமைப்பு xen_seabios_info {
    எழுத்து signature[14]; /* XenHVMSeaBIOS\0 */
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

நிலையான கட்டமைப்பு xen_seabios_info *validate_info(வெறுமை)
{
    கட்டமைப்பு xen_seabios_info *t = (வெறுமை *)INFO_PHYSICAL_ADDRESS;

    எனில் ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        திரும்பு வெற்று;
    }

    எனில் ( t->length < அளவு(கட்டமைப்பு xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        திரும்பு வெற்று;
    }

    எனில் (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        திரும்பு வெற்று;
    }
    திரும்பு t;
}

வெறுமை xen_preinit(வெறுமை)
{
    u32 base, eax, ebx, ecx, edx;
    எழுத்து signature[13];

    எனில் (!CONFIG_XEN)
        திரும்பு;

    க்கு (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        எனில் (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            எனில் ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            நிறுத்து;
        }
    }
    எனில் (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        திரும்பு;
    }
    எனில் (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    இல்லையெனில்
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

நிலையான முழு hypercall_xen_version( முழு cmd, வெறுமை *arg)
{
    திரும்பு _hypercall2(முழு, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
வெறுமை xen_hypercall_setup(வெறுமை)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    எனில் (!runningOnXen())
        திரும்பு;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_உயர்வு(PAGE_SIZE, eax*PAGE_SIZE);
    எனில் (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    க்கு ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

வெறுமை xen_biostable_setup(வெறுமை)
{
    கட்டமைப்பு xen_seabios_info *info = validate_info();
    வெறுமை **tables;
    முழு i;

    எனில் (!info)
        panic("Xen info corrupted\n");

    tables = (வெறுமை*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    க்கு (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

வெறுமை xen_ramsize_preinit(வெறுமை)
{
    முழு i;
    கட்டமைப்பு xen_seabios_info *info = validate_info();
    கட்டமைப்பு e820entry *e820;

    எனில் (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (கட்டமைப்பு e820entry *)info->e820;
    க்கு (i = 0; i < info->e820_nr; i++) {
        கட்டமைப்பு e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
