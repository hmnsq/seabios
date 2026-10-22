// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "util.h" // smp_setup, msr_feature_control_setup
#અન્તર્ભાવ "x86.h" // wrmsr
#અન્તર્ભાવ "paravirt.h" // qemu_*_present_cpus_count

#define APIC_ICR_નીચલું ((u8*)BUILD_APIC_ADDR + 0x300)
#define APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#define APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#define APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#define APIC_ENABLED 0x0100
#define MSR_IA32_APIC_BASE 0x01B
#define MSR_LOCAL_APIC_ID 0x802
#define MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

સ્થિર સંરચના { u32 index; u64 val; } smp_msr[32];
સ્થિર u32 smp_msr_ગણતરી;

શૂન્ય
wrmsr_smp(u32 index, u64 val)
{
    wrmsr(index, val);
    જો (smp_msr_ગણતરી >= ARRAY_SIZE(smp_msr)) {
        warn_noalloc();
        પાછા;
    }
    smp_msr[smp_msr_ગણતરી].index = index;
    smp_msr[smp_msr_ગણતરી].val = val;
    smp_msr_ગણતરી++;
}

સ્થિર શૂન્ય
smp_write_msrs(શૂન્ય)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    પૂર્ણાંક i;
    માટે (i=0; i<smp_msr_ગણતરી; i++)
        wrmsr(smp_msr[i].index, smp_msr[i].val);
}

u32 Max_ગણતરી_CPUs;
સ્થિર u32 ગણતરી_CPUs;
// 256 bits for the found APIC IDs
સ્થિર u32 FoundAPICIDs[256/32];

પૂર્ણાંક apic_id_is_present(u8 apic_id)
{
    પાછા !!(FoundAPICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

સ્થિર પૂર્ણાંક
apic_id_init(શૂન્ય)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    જો (Max_ગણતરી_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        FoundAPICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } નહીંતર જો (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_base = rdmsr(MSR_IA32_APIC_BASE);
        wrmsr(MSR_IA32_APIC_BASE, apic_base | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } નહીંતર {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    પાછા apic_id;
}

શૂન્ય VISIBLE32FLAT
handle_smp(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    // Track this CPU and detect the apic_id
    પૂર્ણાંક apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    ગણતરી_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
સ્થિર શૂન્ય
smp_scan(શૂન્ય)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    જો (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        ગણતરી_CPUs= 1;
        પાછા;
    }

    // mark the BSP initial APIC ID as found, too:
    ગણતરી_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    બાહ્ય શૂન્ય entry_smp(શૂન્ય);
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
    writel(APIC_ICR_નીચલું, 0x000C4500);
    u32 sipi_vector = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_નીચલું, 0x000C4600 | sipi_vector);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_ગણતરી = qemu_get_present_cpus_ગણતરી();
    જ્યારે (expected_cpus_ગણતરી != ગણતરી_CPUs)
        asm અસ્થિર(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", ગણતરી_CPUs,
            Max_ગણતરી_CPUs);
}

શૂન્ય
smp_setup(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    Max_ગણતરી_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_ગણતરી = qemu_get_present_cpus_ગણતરી();
    જો (Max_ગણતરી_CPUs < smp_ગણતરી)
        Max_ગણતરી_CPUs = smp_ગણતરી;

    smp_scan();
}

શૂન્ય
smp_resume(શૂન્ય)
{
    જો (!CONFIG_QEMU)
        પાછા;

    smp_write_msrs();
    smp_scan();
}
