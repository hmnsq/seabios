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

#تعریف_کرو INFO_PHYSICAL_پتہ 0x00001000

u32 xen_cpuid_ادھار = 0;
بنانشانی لما xen_hypercall_ورقہ = 0;

بنية xen_seabios_info {
    حرف signature[14]; /* XenHVMSeaBIOS\0 */
    u8 لمائی;     /* Length of this struct */
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
    بنية xen_seabios_info *t = (فراغ *)INFO_PHYSICAL_پتہ;

    إذا ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        أرجع عدم;
    }

    إذا ( t->لمائی < حجم(بنية xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        أرجع عدم;
    }

    إذا (checksum(t, t->لمائی) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        أرجع عدم;
    }
    أرجع t;
}

فراغ xen_preinit(فراغ)
{
    u32 ادھار, eax, ebx, ecx, edx;
    حرف signature[13];

    إذا (!CONFIG_XEN)
        أرجع;

    لكل (ادھار = 0x40000000; ادھار < 0x40010000; ادھار += 0x100) {
        cpuid(ادھار, &eax, &ebx, &ecx, &edx);
        حافظہ_نقل(signature + 0, &ebx, 4);
        حافظہ_نقل(signature + 4, &ecx, 4);
        حافظہ_نقل(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, ادھار);
        إذا (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            کوڈ_mutable_preinit();
            Debug_Output_پورٹ = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", ادھار);
            إذا ((eax - ادھار) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, ادھار);
            xen_cpuid_ادھار = ادھار;
            اكسر;
        }
    }
    إذا (!xen_cpuid_ادھار) {
        dprintf(1, "No Xen hypervisor found.\n");
        أرجع;
    }
    إذا (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    وإلا
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ساكن صحيح hypercall_xen_نسخہ( صحيح cmd, فراغ *arg)
{
    أرجع _hypercall2(صحيح, xen_نسخہ, cmd, arg);
}

/* Fill in hypercall transfer pages. */
فراغ xen_hypercall_تیاری(فراغ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    بنانشانی لما i;

    إذا (!runningOnXen())
        أرجع;

    cpuid(xen_cpuid_ادھار + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_ورقہ = (بنانشانی لما)memalign_بلند(ورقہ_ناپ, eax*ورقہ_ناپ);
    إذا (!xen_hypercall_ورقہ)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_ورقہ);
    لكل ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_ورقہ + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_ادھار + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_نسخہ(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

فراغ xen_biostable_تیاری(فراغ)
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

    لبھو_acpi_features();
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
        e820_جوڑو(e->شروع, e->ناپ, e->type);
    }
}
