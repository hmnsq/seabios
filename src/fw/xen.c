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

#تعریف_کن INFO_PHYSICAL_آدرس 0x00001000

u32 xen_cpuid_پایه = 0;
بیعلامت دراز xen_hypercall_صفحه = 0;

بنية xen_seabios_info {
    حرف signature[14]; /* XenHVMSeaBIOS\0 */
    u8 درازی;     /* Length of this struct */
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
    بنية xen_seabios_info *t = (فراغ *)INFO_PHYSICAL_آدرس;

    إذا ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        أرجع عدم;
    }

    إذا ( t->درازی < حجم(بنية xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        أرجع عدم;
    }

    إذا (checksum(t, t->درازی) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        أرجع عدم;
    }
    أرجع t;
}

فراغ xen_preinit(فراغ)
{
    u32 پایه, eax, ebx, ecx, edx;
    حرف signature[13];

    إذا (!CONFIG_XEN)
        أرجع;

    لكل (پایه = 0x40000000; پایه < 0x40010000; پایه += 0x100) {
        cpuid(پایه, &eax, &ebx, &ecx, &edx);
        کپی_حافظه(signature + 0, &ebx, 4);
        کپی_حافظه(signature + 4, &ecx, 4);
        کپی_حافظه(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, پایه);
        إذا (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            کد_mutable_preinit();
            Debug_Output_پورت = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", پایه);
            إذا ((eax - پایه) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, پایه);
            xen_cpuid_پایه = پایه;
            اكسر;
        }
    }
    إذا (!xen_cpuid_پایه) {
        dprintf(1, "No Xen hypervisor found.\n");
        أرجع;
    }
    إذا (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    وإلا
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ساكن صحيح hypercall_xen_نسخه( صحيح cmd, فراغ *arg)
{
    أرجع _hypercall2(صحيح, xen_نسخه, cmd, arg);
}

/* Fill in hypercall transfer pages. */
فراغ xen_hypercall_راهاندازی(فراغ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    بیعلامت دراز i;

    إذا (!runningOnXen())
        أرجع;

    cpuid(xen_cpuid_پایه + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_صفحه = (بیعلامت دراز)memalign_بالا(صفحه_اندازه, eax*صفحه_اندازه);
    إذا (!xen_hypercall_صفحه)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_صفحه);
    لكل ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_صفحه + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_پایه + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_نسخه(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

فراغ xen_biostable_راهاندازی(فراغ)
{
    بنية xen_seabios_info *info = validate_info();
    فراغ **tables;
    صحيح i;

    إذا (!info)
        panic("Xen info corrupted\n");

    tables = (فراغ*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    لكل (i=0; i<info->tables_nr; i++)
        copy_جدول(tables[i]);

    پیدا_کن_acpi_features();
}

فراغ xen_ramsize_preinit(فراغ)
{
    صحيح i;
    بنية xen_seabios_info *info = validate_info();
    بنية e820entry *e820;

    إذا (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (بنية e820entry *)info->e820;
    لكل (i = 0; i < info->e820_nr; i++) {
        بنية e820entry *e = &e820[i];
        e820_جمع_کن(e->شروع, e->اندازه, e->type);
    }
}
