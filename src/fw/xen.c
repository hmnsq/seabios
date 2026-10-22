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

#وصف_ڪريو INFO_PHYSICAL_پتو 0x00001000

u32 xen_cpuid_بنياد = 0;
بنانشان ڊگهو xen_hypercall_صفحو = 0;

بنية xen_seabios_info {
    حرف signature[14]; /* XenHVMSeaBIOS\0 */
    u8 ڊيگهائي;     /* Length of this struct */
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
    بنية xen_seabios_info *t = (فراغ *)INFO_PHYSICAL_پتو;

    إذا ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        أرجع عدم;
    }

    إذا ( t->ڊيگهائي < حجم(بنية xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        أرجع عدم;
    }

    إذا (checksum(t, t->ڊيگهائي) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        أرجع عدم;
    }
    أرجع t;
}

فراغ xen_preinit(فراغ)
{
    u32 بنياد, eax, ebx, ecx, edx;
    حرف signature[13];

    إذا (!CONFIG_XEN)
        أرجع;

    لكل (بنياد = 0x40000000; بنياد < 0x40010000; بنياد += 0x100) {
        cpuid(بنياد, &eax, &ebx, &ecx, &edx);
        يادگيري_نقل(signature + 0, &ebx, 4);
        يادگيري_نقل(signature + 4, &ecx, 4);
        يادگيري_نقل(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, بنياد);
        إذا (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            ڪوڊ_mutable_preinit();
            Debug_Output_پورٽ = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", بنياد);
            إذا ((eax - بنياد) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, بنياد);
            xen_cpuid_بنياد = بنياد;
            اكسر;
        }
    }
    إذا (!xen_cpuid_بنياد) {
        dprintf(1, "No Xen hypervisor found.\n");
        أرجع;
    }
    إذا (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    وإلا
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ساكن صحيح hypercall_xen_نسخو( صحيح cmd, فراغ *arg)
{
    أرجع _hypercall2(صحيح, xen_نسخو, cmd, arg);
}

/* Fill in hypercall transfer pages. */
فراغ xen_hypercall_تياري(فراغ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    بنانشان ڊگهو i;

    إذا (!runningOnXen())
        أرجع;

    cpuid(xen_cpuid_بنياد + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_صفحو = (بنانشان ڊگهو)memalign_مٿاهون(صفحو_سائيز, eax*صفحو_سائيز);
    إذا (!xen_hypercall_صفحو)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_صفحو);
    لكل ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_صفحو + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_بنياد + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_نسخو(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

فراغ xen_biostable_تياري(فراغ)
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

    ڳوليو_acpi_features();
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
        e820_جوڙيو(e->شروعات, e->سائيز, e->type);
    }
}
