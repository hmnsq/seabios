// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "util.h" // smp_setup, msr_feature_control_setup
#ଅନ୍ତର୍ଭାଵ "x86.h" // wrmsr
#ଅନ୍ତର୍ଭାଵ "paravirt.h" // qemu_*_present_cpus_count

#define APIC_ICR_ତଳ ((u8*)BUILD_APIC_ADDR + 0x300)
#define APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#define APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#define APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#define APIC_ENABLED 0x0100
#define MSR_IA32_APIC_BASE 0x01B
#define MSR_LOCAL_APIC_ID 0x802
#define MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

ସ୍ଥିର ସଂରଚନା { u32 index; u64 val; } smp_msr[32];
ସ୍ଥିର u32 smp_msr_ଗଣନା;

ଶୂନ୍ୟ
wrmsr_smp(u32 index, u64 val)
{
    wrmsr(index, val);
    ଯଦି (smp_msr_ଗଣନା >= ARRAY_SIZE(smp_msr)) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    smp_msr[smp_msr_ଗଣନା].index = index;
    smp_msr[smp_msr_ଗଣନା].val = val;
    smp_msr_ଗଣନା++;
}

ସ୍ଥିର ଶୂନ୍ୟ
smp_write_msrs(ଶୂନ୍ୟ)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<smp_msr_ଗଣନା; i++)
        wrmsr(smp_msr[i].index, smp_msr[i].val);
}

u32 Max_ଗଣନା_CPUs;
ସ୍ଥିର u32 ଗଣନା_CPUs;
// 256 bits for the found APIC IDs
ସ୍ଥିର u32 FoundAPICIDs[256/32];

ପୂର୍ଣ୍ଣସଂଖ୍ୟା apic_id_is_present(u8 apic_id)
{
    ଫେରନ୍ତୁ !!(FoundAPICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
apic_id_init(ଶୂନ୍ୟ)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    ଯଦି (Max_ଗଣନା_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        FoundAPICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } ଅନ୍ୟଥା ଯଦି (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_base = rdmsr(MSR_IA32_APIC_BASE);
        wrmsr(MSR_IA32_APIC_BASE, apic_base | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } ଅନ୍ୟଥା {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    ଫେରନ୍ତୁ apic_id;
}

ଶୂନ୍ୟ VISIBLE32FLAT
handle_smp(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    // Track this CPU and detect the apic_id
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    ଗଣନା_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
ସ୍ଥିର ଶୂନ୍ୟ
smp_scan(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ଯଦି (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        ଗଣନା_CPUs= 1;
        ଫେରନ୍ତୁ;
    }

    // mark the BSP initial APIC ID as found, too:
    ଗଣନା_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    ବାହ୍ୟ ଶୂନ୍ୟ entry_smp(ଶୂନ୍ୟ);
    u64 new = (0xea | ((u64)SEG_BIOS<<24)
               | (((u32)entry_smp - BUILD_BIOS_ADDR) << 8));
    *(u64*)BUILD_AP_BOOT_ADDR = new;

    // enable local APIC
    u32 val = readl(APIC_SVR);
    writel(APIC_SVR, val | APIC_ENABLED);

    /* Set LINT0 as Ext_INT, level triggered */
    writel(APIC_LINT0, 0x8700);

    /* Set LINT1 as NMI, level triggered */
    writel(APIC_LINT1, 0x8400);

    // Init the lock.
    writel(&SMPLock, 1);

    // broadcast SIPI
    barrier();
    writel(APIC_ICR_ତଳ, 0x000C4500);
    u32 sipi_vector = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_ତଳ, 0x000C4600 | sipi_vector);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_ଗଣନା = qemu_get_present_cpus_ଗଣନା();
    ଯେତେବେଳେ (expected_cpus_ଗଣନା != ଗଣନା_CPUs)
        asm ଅସ୍ଥିର(
            // Release lock and allow other processors to use the stack.
            "  movl %%esp, %1\n"
            "  movl $0, %0\n"
            // Reacquire lock and take back ownership of stack.
            "1:rep ; nop\n"
            "  lock btsl $0, %0\n"
            "  jc 1b\n"
            : "+m" (SMPLock), "+m" (SMPStack)
            : : "cc", "memory");
    yield();

    // Restore memory.
    *(u64*)BUILD_AP_BOOT_ADDR = old;

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", ଗଣନା_CPUs,
            Max_ଗଣନା_CPUs);
}

ଶୂନ୍ୟ
smp_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    Max_ଗଣନା_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_ଗଣନା = qemu_get_present_cpus_ଗଣନା();
    ଯଦି (Max_ଗଣନା_CPUs < smp_ଗଣନା)
        Max_ଗଣନା_CPUs = smp_ଗଣନା;

    smp_scan();
}

ଶୂନ୍ୟ
smp_resume(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_QEMU)
        ଫେରନ୍ତୁ;

    smp_write_msrs();
    smp_scan();
}
