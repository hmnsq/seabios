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

#ವ್ಯಾಖ್ಯಾನಿಸು INFO_PHYSICAL_ವಿಳಾಸ 0x00001000

u32 xen_cpuid_ತಳ = 0;
ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ xen_hypercall_ಪುಟ = 0;

ರಚನೆ xen_seabios_info {
    ಅಕ್ಷರ signature[14]; /* XenHVMSeaBIOS\0 */
    u8 ಉದ್ದಳತೆ;     /* Length of this struct */
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

ಸ್ಥಿರ ರಚನೆ xen_seabios_info *validate_info(ಶೂನ್ಯ)
{
    ರಚನೆ xen_seabios_info *t = (ಶೂನ್ಯ *)INFO_PHYSICAL_ವಿಳಾಸ;

    ಆದರೆ ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    ಆದರೆ ( t->ಉದ್ದಳತೆ < ಗಾತ್ರ(ರಚನೆ xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    ಆದರೆ (checksum(t, t->ಉದ್ದಳತೆ) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಹಿಂತಿರುಗಿ t;
}

ಶೂನ್ಯ xen_preinit(ಶೂನ್ಯ)
{
    u32 ತಳ, eax, ebx, ecx, edx;
    ಅಕ್ಷರ signature[13];

    ಆದರೆ (!CONFIG_XEN)
        ಹಿಂತಿರುಗಿ;

    ಗಾಗಿ (ತಳ = 0x40000000; ತಳ < 0x40010000; ತಳ += 0x100) {
        cpuid(ತಳ, &eax, &ebx, &ecx, &edx);
        ಸ್ಮರಣೆ_ನಕಲು(signature + 0, &ebx, 4);
        ಸ್ಮರಣೆ_ನಕಲು(signature + 4, &ecx, 4);
        ಸ್ಮರಣೆ_ನಕಲು(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, ತಳ);
        ಆದರೆ (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            ಸಂಕೇತ_mutable_preinit();
            Debug_Output_ಪೋರ್ಟ್ = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", ತಳ);
            ಆದರೆ ((eax - ತಳ) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, ತಳ);
            xen_cpuid_ತಳ = ತಳ;
            ನಿಲ್ಲಿಸು;
        }
    }
    ಆದರೆ (!xen_cpuid_ತಳ) {
        dprintf(1, "No Xen hypervisor found.\n");
        ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    ಇಲ್ಲದಿದ್ದರೆ
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hypercall_xen_ಆವೃತ್ತಿ( ಪೂರ್ಣಾಂಕ cmd, ಶೂನ್ಯ *arg)
{
    ಹಿಂತಿರುಗಿ _hypercall2(ಪೂರ್ಣಾಂಕ, xen_ಆವೃತ್ತಿ, cmd, arg);
}

/* Fill in hypercall transfer pages. */
ಶೂನ್ಯ xen_hypercall_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ i;

    ಆದರೆ (!runningOnXen())
        ಹಿಂತಿರುಗಿ;

    cpuid(xen_cpuid_ತಳ + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_ಪುಟ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)memalign_ಮೇಲಿನ(ಪುಟ_ಗಾತ್ರ, eax*ಪುಟ_ಗಾತ್ರ);
    ಆದರೆ (!xen_hypercall_ಪುಟ)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_ಪುಟ);
    ಗಾಗಿ ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_ಪುಟ + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_ತಳ + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_ಆವೃತ್ತಿ(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

ಶೂನ್ಯ xen_biostable_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ರಚನೆ xen_seabios_info *info = validate_info();
    ಶೂನ್ಯ **tables;
    ಪೂರ್ಣಾಂಕ i;

    ಆದರೆ (!info)
        panic("Xen info corrupted\n");

    tables = (ಶೂನ್ಯ*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    ಗಾಗಿ (i=0; i<info->tables_nr; i++)
        copy_ಕೋಷ್ಟಕ(tables[i]);

    ಕಂಡುಹಿಡಿ_acpi_features();
}

ಶೂನ್ಯ xen_ramsize_preinit(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ i;
    ರಚನೆ xen_seabios_info *info = validate_info();
    ರಚನೆ e820entry *e820;

    ಆದರೆ (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (ರಚನೆ e820entry *)info->e820;
    ಗಾಗಿ (i = 0; i < info->e820_nr; i++) {
        ರಚನೆ e820entry *e = &e820[i];
        e820_ಕೂಡಿಸು(e->ಆರಂಭ, e->ಗಾತ್ರ_2, e->type);
    }
}
