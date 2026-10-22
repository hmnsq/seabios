// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_XEN
#समावेश "e820map.h" // e820_add
#समावेश "hw/serialio.h" // DebugOutputPort
#समावेश "malloc.h" // memalign_high
#समावेश "memmap.h" // PAGE_SIZE
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // PlatformRunningOn
#समावेश "string.h" // memcpy
#समावेश "util.h" // copy_acpi_rsdp
#समावेश "x86.h" // cpuid
#समावेश "xen.h" // xen_extraversion_t

#घोषणा INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
अचिन्हित दीर्घ xen_hypercall_page = 0;

काष्ठा xen_seabios_info {
    अक्षर signature[14]; /* XenHVMSeaBIOS\0 */
    u8 लंबाई;     /* Length of this struct */
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

अटल काष्ठा xen_seabios_info *validate_info(व्योम)
{
    काष्ठा xen_seabios_info *t = (व्योम *)INFO_PHYSICAL_ADDRESS;

    अगर ( स_भेद(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        वापस शून्य;
    }

    अगर ( t->लंबाई < माप(काष्ठा xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        वापस शून्य;
    }

    अगर (checksum(t, t->लंबाई) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        वापस शून्य;
    }
    वापस t;
}

व्योम xen_preinit(व्योम)
{
    u32 base, eax, ebx, ecx, edx;
    अक्षर signature[13];

    अगर (!CONFIG_XEN)
        वापस;

    क्रम (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        स_नकल(signature + 0, &ebx, 4);
        स_नकल(signature + 4, &ecx, 4);
        स_नकल(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        अगर (म_भेद(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            अगर ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            अवरोध;
        }
    }
    अगर (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        वापस;
    }
    अगर (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    अन्यथा
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

अटल पूर्णांक hypercall_xen_version( पूर्णांक cmd, व्योम *arg)
{
    वापस _hypercall2(पूर्णांक, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
व्योम xen_hypercall_setup(व्योम)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    अचिन्हित दीर्घ i;

    अगर (!runningOnXen())
        वापस;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (अचिन्हित दीर्घ)memalign_ऊपरी(PAGE_आकार, eax*PAGE_आकार);
    अगर (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    क्रम ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

व्योम xen_biostable_setup(व्योम)
{
    काष्ठा xen_seabios_info *info = validate_info();
    व्योम **tables;
    पूर्णांक i;

    अगर (!info)
        panic("Xen info corrupted\n");

    tables = (व्योम*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    क्रम (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

व्योम xen_ramsize_preinit(व्योम)
{
    पूर्णांक i;
    काष्ठा xen_seabios_info *info = validate_info();
    काष्ठा e820entry *e820;

    अगर (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (काष्ठा e820entry *)info->e820;
    क्रम (i = 0; i < info->e820_nr; i++) {
        काष्ठा e820entry *e = &e820[i];
        e820_add(e->start, e->आकार, e->type);
    }
}
