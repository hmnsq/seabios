// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_XEN
#ಅನ್ತರ್ಭಾವ "e820map.h" // e820_add
#ಅನ್ತರ್ಭಾವ "hw/serialio.h" // DebugOutputPort
#ಅನ್ತರ್ಭಾವ "malloc.h" // memalign_high
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // PlatformRunningOn
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "util.h" // copy_acpi_rsdp
#ಅನ್ತರ್ಭಾವ "x86.h" // cpuid
#ಅನ್ತರ್ಭಾವ "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

ಸಂರಚನಾ xen_seabios_info {
    ವರ್ಣ signature[14]; /* XenHVMSeaBIOS\0 */
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

ಸ್ಥಿರ ಸಂರಚನಾ xen_seabios_info *validate_info(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ xen_seabios_info *t = (ಶೂನ್ಯ *)INFO_PHYSICAL_ADDRESS;

    ಯದಿ ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    ಯದಿ ( t->length < ಪರಿಮಾಣ(ಸಂರಚನಾ xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    ಯದಿ (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    ಪ್ರತ್ಯಾಗಮ t;
}

ಶೂನ್ಯ xen_preinit(ಶೂನ್ಯ)
{
    u32 base, eax, ebx, ecx, edx;
    ವರ್ಣ signature[13];

    ಯದಿ (!CONFIG_XEN)
        ಪ್ರತ್ಯಾಗಮ;

    ಪ್ರತಿ (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        ಯದಿ (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            ಯದಿ ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            ಭಙ್ಗ;
        }
    }
    ಯದಿ (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    ಅನ್ಯಥಾ
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hypercall_xen_version( ಪೂರ್ಣಾಙ್ಕ cmd, ಶೂನ್ಯ *arg)
{
    ಪ್ರತ್ಯಾಗಮ _hypercall2(ಪೂರ್ಣಾಙ್ಕ, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
ಶೂನ್ಯ xen_hypercall_setup(ಶೂನ್ಯ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    ಯದಿ (!runningOnXen())
        ಪ್ರತ್ಯಾಗಮ;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_ಮೇಲಿನ(PAGE_SIZE, eax*PAGE_SIZE);
    ಯದಿ (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    ಪ್ರತಿ ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

ಶೂನ್ಯ xen_biostable_setup(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ xen_seabios_info *info = validate_info();
    ಶೂನ್ಯ **tables;
    ಪೂರ್ಣಾಙ್ಕ i;

    ಯದಿ (!info)
        panic("Xen info corrupted\n");

    tables = (ಶೂನ್ಯ*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    ಪ್ರತಿ (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

ಶೂನ್ಯ xen_ramsize_preinit(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಸಂರಚನಾ xen_seabios_info *info = validate_info();
    ಸಂರಚನಾ e820entry *e820;

    ಯದಿ (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (ಸಂರಚನಾ e820entry *)info->e820;
    ಪ್ರತಿ (i = 0; i < info->e820_nr; i++) {
        ಸಂರಚನಾ e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
