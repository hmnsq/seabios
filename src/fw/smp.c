// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "util.h" // smp_setup, msr_feature_control_setup
#অন্তর্ভাব "x86.h" // wrmsr
#অন্তর্ভাব "paravirt.h" // qemu_*_present_cpus_count

#define APIC_ICR_নিচের ((u8*)BUILD_APIC_ADDR + 0x300)
#define APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#define APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#define APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#define APIC_ENABLED 0x0100
#define MSR_IA32_APIC_BASE 0x01B
#define MSR_LOCAL_APIC_ID 0x802
#define MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

স্থির কাঠামো { u32 সূচক; u64 val; } smp_msr[32];
স্থির u32 smp_msr_গুনতি;

শূন্য
wrmsr_smp(u32 সূচক, u64 val)
{
    wrmsr(সূচক, val);
    যদি (smp_msr_গুনতি >= ARRAY_আকার(smp_msr)) {
        warn_noalloc();
        ফেরত;
    }
    smp_msr[smp_msr_গুনতি].সূচক = সূচক;
    smp_msr[smp_msr_গুনতি].val = val;
    smp_msr_গুনতি++;
}

স্থির শূন্য
smp_write_msrs(শূন্য)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<smp_msr_গুনতি; i++)
        wrmsr(smp_msr[i].সূচক, smp_msr[i].val);
}

u32 Max_গুনতি_CPUs;
স্থির u32 গুনতি_CPUs;
// 256 bits for the found APIC IDs
স্থির u32 FoundAPICIDs[256/32];

পূর্ণসংখ্যা apic_id_is_present(u8 apic_id)
{
    ফেরত !!(FoundAPICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

স্থির পূর্ণসংখ্যা
apic_id_init(শূন্য)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    যদি (Max_গুনতি_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        FoundAPICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } অন্যথা যদি (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_base = rdmsr(MSR_IA32_APIC_BASE);
        wrmsr(MSR_IA32_APIC_BASE, apic_base | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } অন্যথা {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    ফেরত apic_id;
}

শূন্য VISIBLE32FLAT
handle_smp(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    // Track this CPU and detect the apic_id
    পূর্ণসংখ্যা apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    গুনতি_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
স্থির শূন্য
smp_scan(শূন্য)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    যদি (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        গুনতি_CPUs= 1;
        ফেরত;
    }

    // mark the BSP initial APIC ID as found, too:
    গুনতি_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    বাহ্যিক শূন্য entry_smp(শূন্য);
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
    writel(APIC_ICR_নিচের, 0x000C4500);
    u32 sipi_vector = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_নিচের, 0x000C4600 | sipi_vector);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_গুনতি = qemu_get_present_cpus_গুনতি();
    যখন (expected_cpus_গুনতি != গুনতি_CPUs)
        asm অস্থির(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", গুনতি_CPUs,
            Max_গুনতি_CPUs);
}

শূন্য
smp_setup(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    Max_গুনতি_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_গুনতি = qemu_get_present_cpus_গুনতি();
    যদি (Max_গুনতি_CPUs < smp_গুনতি)
        Max_গুনতি_CPUs = smp_গুনতি;

    smp_scan();
}

শূন্য
smp_resume(শূন্য)
{
    যদি (!CONFIG_QEMU)
        ফেরত;

    smp_write_msrs();
    smp_scan();
}
