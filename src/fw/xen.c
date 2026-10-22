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

#تعریف_کرو INFO_PHYSICAL_پتہ 0x00001000

u32 xen_cpuid_بنیاد = 0;
غیرعلامتی طویل xen_hypercall_صفحہ = 0;

ساخت xen_seabios_info {
    حرف signature[14]; /* XenHVMSeaBIOS\0 */
    u8 لمبائی;     /* Length of this struct */
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

ساکن ساخت xen_seabios_info *validate_info(خالی)
{
    ساخت xen_seabios_info *t = (خالی *)INFO_PHYSICAL_پتہ;

    اگر ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        واپس تہی;
    }

    اگر ( t->لمبائی < حجم(ساخت xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        واپس تہی;
    }

    اگر (checksum(t, t->لمبائی) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        واپس تہی;
    }
    واپس t;
}

خالی xen_preinit(خالی)
{
    u32 بنیاد, eax, ebx, ecx, edx;
    حرف signature[13];

    اگر (!CONFIG_XEN)
        واپس;

    برائے (بنیاد = 0x40000000; بنیاد < 0x40010000; بنیاد += 0x100) {
        cpuid(بنیاد, &eax, &ebx, &ecx, &edx);
        حافظہ_نقل(signature + 0, &ebx, 4);
        حافظہ_نقل(signature + 4, &ecx, 4);
        حافظہ_نقل(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, بنیاد);
        اگر (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            رمز_mutable_preinit();
            Debug_Output_پورٹ = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", بنیاد);
            اگر ((eax - بنیاد) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, بنیاد);
            xen_cpuid_بنیاد = بنیاد;
            توڑو;
        }
    }
    اگر (!xen_cpuid_بنیاد) {
        dprintf(1, "No Xen hypervisor found.\n");
        واپس;
    }
    اگر (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    ورنہ
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ساکن صحیح hypercall_xen_نسخہ( صحیح cmd, خالی *arg)
{
    واپس _hypercall2(صحیح, xen_نسخہ, cmd, arg);
}

/* Fill in hypercall transfer pages. */
خالی xen_hypercall_تیاری(خالی)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    غیرعلامتی طویل i;

    اگر (!runningOnXen())
        واپس;

    cpuid(xen_cpuid_بنیاد + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_صفحہ = (غیرعلامتی طویل)memalign_بلند(صفحہ_سائز, eax*صفحہ_سائز);
    اگر (!xen_hypercall_صفحہ)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_صفحہ);
    برائے ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_صفحہ + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_بنیاد + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_نسخہ(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

خالی xen_biostable_تیاری(خالی)
{
    ساخت xen_seabios_info *info = validate_info();
    خالی **tables;
    صحیح i;

    اگر (!info)
        panic("Xen info corrupted\n");

    tables = (خالی*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    برائے (i=0; i<info->tables_nr; i++)
        copy_جدول(tables[i]);

    ڈھونڈو_acpi_features();
}

خالی xen_ramsize_preinit(خالی)
{
    صحیح i;
    ساخت xen_seabios_info *info = validate_info();
    ساخت e820entry *e820;

    اگر (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (ساخت e820entry *)info->e820;
    برائے (i = 0; i < info->e820_nr; i++) {
        ساخت e820entry *e = &e820[i];
        e820_جوڑو(e->آغاز, e->سائز, e->type);
    }
}
