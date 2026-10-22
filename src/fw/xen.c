// Xen HVM support
//
// Copyright (C) 2011 Citrix Systems.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // CONFIG_XEN
#include "e820map.h" // e820_add
#include "hw/serialio.h" // DebugOutputPort
#include "malloc.h" // memalign_high
#include "memmap.h" // PAGE_SIZE
#include "output.h" // dprintf
#include "paravirt.h" // PlatformRunningOn
#include "string.h" // memcpy
#include "util.h" // copy_acpi_rsdp
#include "x86.h" // cpuid
#include "xen.h" // xen_extraversion_t

#define INFO_PHYSICAL_ADDRESS 0x00001000

u32 xen_cpuid_base = 0;
unsigned long xen_hypercall_page = 0;

sa_nracanaa xen_seabios_info {
    var_na signature[14]; /* XenHVMSeaBIOS\0 */
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

static sa_nracanaa xen_seabios_info *validate_info(void)
{
    sa_nracanaa xen_seabios_info *t = (void *)INFO_PHYSICAL_ADDRESS;

    ប្រសិនបើ ( memcmp(t->signature, "XenHVMSeaBIOS", 14) ) {
        dprintf(1, "Bad Xen info signature\n");
        ត្រឡប់ NULL;
    }

    ប្រសិនបើ ( t->length < parimaa_na(sa_nracanaa xen_seabios_info) ) {
        dprintf(1, "Bad Xen info length\n");
        ត្រឡប់ NULL;
    }

    ប្រសិនបើ (checksum(t, t->length) != 0) {
        dprintf(1, "Bad Xen info checksum\n");
        ត្រឡប់ NULL;
    }
    ត្រឡប់ t;
}

void xen_preinit(void)
{
    u32 base, eax, ebx, ecx, edx;
    var_na signature[13];

    ប្រសិនបើ (!CONFIG_XEN)
        ត្រឡប់;

    សម្រាប់ (base = 0x40000000; base < 0x40010000; base += 0x100) {
        cpuid(base, &eax, &ebx, &ecx, &edx);
        memcpy(signature + 0, &ebx, 4);
        memcpy(signature + 4, &ecx, 4);
        memcpy(signature + 8, &edx, 4);
        signature[12] = 0;

        dprintf(9, "Found hypervisor signature \"%s\" at %x\n",
                signature, base);
        ប្រសិនបើ (strcmp(signature, "XenVMMXenVMM") == 0) {
            /* Set debug_io_port first, so the following messages work. */
            code_mutable_preinit();
            DebugOutputPort = 0xe9;
            debug_banner();
            dprintf(1, "\nFound Xen hypervisor signature at %x\n", base);
            ប្រសិនបើ ((eax - base) < 2)
                panic("Insufficient Xen cpuid leaves. eax=%x at base %x\n",
                      eax, base);
            xen_cpuid_base = base;
            bha_nga;
        }
    }
    ប្រសិនបើ (!xen_cpuid_base) {
        dprintf(1, "No Xen hypervisor found.\n");
        ត្រឡប់;
    }
    ប្រសិនបើ (validate_info())
        PlatformRunningOn = PF_QEMU|PF_XEN;
    បើមិនដូច្នេះ
        dprintf(1, "Not enabling Xen support due to lack of Xen info\n");
}

static ចំនួនគត់ hypercall_xen_version( ចំនួនគត់ cmd, void *arg)
{
    ត្រឡប់ _hypercall2(ចំនួនគត់, xen_version, cmd, arg);
}

/* Fill in hypercall transfer pages. */
void xen_hypercall_setup(void)
{
    u32 eax, ebx, ecx, edx;
    xen_extraversion_t extraversion;
    unsigned long i;

    ប្រសិនបើ (!runningOnXen())
        ត្រឡប់;

    cpuid(xen_cpuid_base + 2, &eax, &ebx, &ecx, &edx);

    xen_hypercall_page = (unsigned long)memalign_high(PAGE_SIZE, eax*PAGE_SIZE);
    ប្រសិនបើ (!xen_hypercall_page)
        panic("unable to allocate Xen hypercall page\n");

    dprintf(1, "Allocated Xen hypercall page at %lx\n", xen_hypercall_page);
    សម្រាប់ ( i = 0; i < eax; i++ )
        wrmsr(ebx, xen_hypercall_page + (i << 12) + i);

    /* Print version information. */
    cpuid(xen_cpuid_base + 1, &eax, &ebx, &ecx, &edx);
    hypercall_xen_version(XENVER_extraversion, extraversion);
    dprintf(1, "Detected Xen v%u.%u%s\n", eax >> 16, eax & 0xffff, extraversion);
}

void xen_biostable_setup(void)
{
    sa_nracanaa xen_seabios_info *info = validate_info();
    void **tables;
    ចំនួនគត់ i;

    ប្រសិនបើ (!info)
        panic("Xen info corrupted\n");

    tables = (void*)info->tables;
    dprintf(1, "xen: copy BIOS tables...\n");
    សម្រាប់ (i=0; i<info->tables_nr; i++)
        copy_table(tables[i]);

    find_acpi_features();
}

void xen_ramsize_preinit(void)
{
    ចំនួនគត់ i;
    sa_nracanaa xen_seabios_info *info = validate_info();
    sa_nracanaa e820entry *e820;

    ប្រសិនបើ (!info)
        panic("Xen info corrupted\n");

    dprintf(1, "xen: copy e820...\n");

    e820 = (sa_nracanaa e820entry *)info->e820;
    សម្រាប់ (i = 0; i < info->e820_nr; i++) {
        sa_nracanaa e820entry *e = &e820[i];
        e820_add(e->start, e->size, e->type);
    }
}
