// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_XEN
#अन्तर्भाव "e820map.h" // e820_add
#अन्तर्भाव "hw/serialio.h" // DebugOutputPort
#अन्तर्भाव "malloc.h" // memalign_high
#अन्तर्भाव "memmap.h" // PAGE_SIZE
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // PlatformRunningOn
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "util.h" // copy_acpi_rsdp
#अन्तर्भाव "x86.h" // cpuid
#अन्तर्भाव "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
अचिन्हित दीर्घ xen_hypercall_page = 0;

संरचना xen_seabios_info {
    वर्ण signature[14]; /* XenHVMSeaBIOS\0 */
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

स्थिर संरचना xen_seabios_info *validate_info(शून्य)
{
    संरचना xen_seabios_info *t = (शून्य *)INFO_PHYSICAL_ADDRESS;

    यदि ( स_भेद(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        प्रत्यागम रिक्त;
    }

    यदि ( t->length < परिमाण(संरचना xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        प्रत्यागम रिक्त;
    }

    यदि (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        प्रत्यागम रिक्त;
    }
    प्रत्यागम t;
}

शून्य xen_preinit(शून्य)
{
    u32 base, eax, ebx, ecx, edx;
    वर्ण signature[13];

    यदि (!CONFIG_XEN)
        प्रत्यागम;

    प्रति (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        स_नकल(signature + 0, &ebx, 4);
        स_नकल(signature + 4, &ecx, 4);
        स_नकल(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        यदि (म_भेद(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            यदि ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            भङ्ग;
        }
    }
    यदि (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        प्रत्यागम;
    }
    यदि (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    अन्यथा
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

स्थिर पूर्णाङ्क hypercall_xen_version( पूर्णाङ्क cmd, शून्य *arg)
{
    प्रत्यागम _hypercall2(पूर्णाङ्क, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
शून्य xen_hypercall_setup(शून्य)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    अचिन्हित दीर्घ i;

    यदि (!runningOnXen())
        प्रत्यागम;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (अचिन्हित दीर्घ)memalign_ऊपरी(PAGE_SIZE, eax*PAGE_SIZE);
    यदि (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    प्रति ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

शून्य xen_biostable_setup(शून्य)
{
    संरचना xen_seabios_info *info = validate_info();
    शून्य **tables;
    पूर्णाङ्क i;

    यदि (!info)
        panic("Xen info corrupted\n");

    tables = (शून्य*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    प्रति (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

शून्य xen_ramsize_preinit(शून्य)
{
    पूर्णाङ्क i;
    संरचना xen_seabios_info *info = validate_info();
    संरचना e820entry *e820;

    यदि (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (संरचना e820entry *)info->e820;
    प्रति (i = 0; i < info->e820_nr; i++) {
        संरचना e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
