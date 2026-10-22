// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_XEN
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // e820_add
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/serialio.h" // DebugOutputPort
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // memalign_high
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SIZE
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // PlatformRunningOn
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // copy_acpi_rsdp
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // cpuid
#ਅਨ੍ਤਰ੍ਭਾਵ "xen.h" // xen_extraversion_t

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INFO_PHYSICAL_ਪਤਾ 0x00001000

u32 xen_cpuid_ਅਧਾਰ = 0;
ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ xen_hypercall_ਪੰਨਾ = 0;

ਬਣਤਰ xen_seabios_info {
    ਅੱਖਰ signature[14]; /* XenHVMSeaBIOS\0 */
    u8 ਲੰਮਾਈ;     /* Length of this struct */
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

ਸਥਿਰ ਬਣਤਰ xen_seabios_info *validate_info(ਖਾਲੀ)
{
    ਬਣਤਰ xen_seabios_info *t = (ਖਾਲੀ *)INFO_PHYSICAL_ਪਤਾ;

    ਜੇ ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        ਵਾਪਸ ਨਿਰਾ;
    }

    ਜੇ ( t->ਲੰਮਾਈ < ਆਕਾਰ(ਬਣਤਰ xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        ਵਾਪਸ ਨਿਰਾ;
    }

    ਜੇ (checksum(t, t->ਲੰਮਾਈ) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਵਾਪਸ t;
}

ਖਾਲੀ xen_preinit(ਖਾਲੀ)
{
    u32 ਅਧਾਰ, eax, ebx, ecx, edx;
    ਅੱਖਰ signature[13];

    ਜੇ (!CONFIG_XEN)
        ਵਾਪਸ;

    ਲਈ (ਅਧਾਰ = 0x40000000; ਅਧਾਰ < 0x40010000; ਅਧਾਰ += 0x100) {
        cpuid(ਅਧਾਰ, &eax, &ebx, &ecx, &edx);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 0, &ebx, 4);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 4, &ecx, 4);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, ਅਧਾਰ);
        ਜੇ (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            ਕੋਡ_mutable_preinit();
            Debug_Output_ਪੋਰਟ = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", ਅਧਾਰ);
            ਜੇ ((eax - ਅਧਾਰ) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, ਅਧਾਰ);
            xen_cpuid_ਅਧਾਰ = ਅਧਾਰ;
            ਰੋਕੋ;
        }
    }
    ਜੇ (!xen_cpuid_ਅਧਾਰ) {
        dprintf(1, "No Xen hypervisor found.\n");
        ਵਾਪਸ;
    }
    ਜੇ (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    ਨਹੀਂਤਾਂ
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ਸਥਿਰ ਪੂਰਨ hypercall_xen_ਸੰਸਕਰਣ( ਪੂਰਨ cmd, ਖਾਲੀ *arg)
{
    ਵਾਪਸ _hypercall2(ਪੂਰਨ, xen_ਸੰਸਕਰਣ, cmd, arg);
}

/* Fill in hypercall transfer pages. */
ਖਾਲੀ xen_hypercall_ਤਿਆਰੀ(ਖਾਲੀ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ i;

    ਜੇ (!runningOnXen())
        ਵਾਪਸ;

    cpuid(xen_cpuid_ਅਧਾਰ + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_ਪੰਨਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)memalign_ਉੱਚਾ(ਪੰਨਾ_ਮਾਪ, eax*ਪੰਨਾ_ਮਾਪ);
    ਜੇ (!xen_hypercall_ਪੰਨਾ)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_ਪੰਨਾ);
    ਲਈ ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_ਪੰਨਾ + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_ਅਧਾਰ + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_ਸੰਸਕਰਣ(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

ਖਾਲੀ xen_biostable_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਬਣਤਰ xen_seabios_info *info = validate_info();
    ਖਾਲੀ **tables;
    ਪੂਰਨ i;

    ਜੇ (!info)
        panic("Xen info corrupted\n");

    tables = (ਖਾਲੀ*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    ਲਈ (i=0; i<info->tables_nr; i++)
        copy_ਸਾਰਣੀ(tables[i]);

    ਲੱਭੋ_acpi_features();
}

ਖਾਲੀ xen_ramsize_preinit(ਖਾਲੀ)
{
    ਪੂਰਨ i;
    ਬਣਤਰ xen_seabios_info *info = validate_info();
    ਬਣਤਰ e820entry *e820;

    ਜੇ (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (ਬਣਤਰ e820entry *)info->e820;
    ਲਈ (i = 0; i < info->e820_nr; i++) {
        ਬਣਤਰ e820entry *e = &e820[i];
        e820_ਜੋੜੋ(e->ਸ਼ੁਰੂ, e->ਮਾਪ, e->type);
    }
}
