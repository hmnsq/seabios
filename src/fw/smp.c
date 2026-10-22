// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "util.h" // smp_setup, msr_feature_control_setup
#ಅನ್ತರ್ಭಾವ "x86.h" // wrmsr
#ಅನ್ತರ್ಭಾವ "paravirt.h" // qemu_*_present_cpus_count

#ವ್ಯಾಖ್ಯಾನಿಸು APIC_ICR_ಕೆಳಗಿನ ((u8*)BUILD_APIC_ADDR + 0x300)
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#ವ್ಯಾಖ್ಯಾನಿಸು APIC_ENABLED 0x0100
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_IA32_APIC_ತಳ 0x01B
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_LOCAL_APIC_ID 0x802
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

ಸ್ಥಿರ ರಚನೆ { u32 ಸೂಚ್ಯಂಕ; u64 val; } smp_msr[32];
ಸ್ಥಿರ u32 smp_msr_ಎಣಿಕೆ;

ಶೂನ್ಯ
wrmsr_smp(u32 ಸೂಚ್ಯಂಕ, u64 val)
{
    wrmsr(ಸೂಚ್ಯಂಕ, val);
    ಆದರೆ (smp_msr_ಎಣಿಕೆ >= ARRAY_ಗಾತ್ರ(smp_msr)) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    smp_msr[smp_msr_ಎಣಿಕೆ].ಸೂಚ್ಯಂಕ = ಸೂಚ್ಯಂಕ;
    smp_msr[smp_msr_ಎಣಿಕೆ].val = val;
    smp_msr_ಎಣಿಕೆ++;
}

ಸ್ಥಿರ ಶೂನ್ಯ
smp_write_msrs(ಶೂನ್ಯ)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<smp_msr_ಎಣಿಕೆ; i++)
        wrmsr(smp_msr[i].ಸೂಚ್ಯಂಕ, smp_msr[i].val);
}

u32 Max_ಎಣಿಕೆ_CPUs;
ಸ್ಥಿರ u32 ಎಣಿಕೆ_CPUs;
// 256 bits for the found APIC IDs
ಸ್ಥಿರ u32 ಸಿಕ್ಕಿತು_APICIDs[256/32];

ಪೂರ್ಣಾಂಕ apic_id_is_present(u8 apic_id)
{
    ಹಿಂತಿರುಗಿ !!(ಸಿಕ್ಕಿತು_APICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
apic_id_init(ಶೂನ್ಯ)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    ಆದರೆ (Max_ಎಣಿಕೆ_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        ಸಿಕ್ಕಿತು_APICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_ತಳ = rdmsr(MSR_IA32_APIC_ತಳ);
        wrmsr(MSR_IA32_APIC_ತಳ, apic_ತಳ | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } ಇಲ್ಲದಿದ್ದರೆ {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    ಹಿಂತಿರುಗಿ apic_id;
}

ಶೂನ್ಯ VISIBLE32FLAT
handle_smp(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    // Track this CPU and detect the apic_id
    ಪೂರ್ಣಾಂಕ apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    ಎಣಿಕೆ_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
ಸ್ಥಿರ ಶೂನ್ಯ
smp_ಪರಿಶೀಲನೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ಆದರೆ (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        ಎಣಿಕೆ_CPUs= 1;
        ಹಿಂತಿರುಗಿ;
    }

    // mark the BSP initial APIC ID as found, too:
    ಎಣಿಕೆ_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_smp(ಶೂನ್ಯ);
    u64 new = (0xea | ((u64)SEG_BIOS<<24)
               | (((u32)ನಮೂದು_smp - BUILD_BIOS_ADDR) << 8));
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
    writel(APIC_ICR_ಕೆಳಗಿನ, 0x000C4500);
    u32 sipi_ಸದಿಶ = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_ಕೆಳಗಿನ, 0x000C4600 | sipi_ಸದಿಶ);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_ಎಣಿಕೆ = qemu_get_present_cpus_ಎಣಿಕೆ();
    ಆಗ (expected_cpus_ಎಣಿಕೆ != ಎಣಿಕೆ_CPUs)
        asm ಅಸ್ಥಿರ(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", ಎಣಿಕೆ_CPUs,
            Max_ಎಣಿಕೆ_CPUs);
}

ಶೂನ್ಯ
smp_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    Max_ಎಣಿಕೆ_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_ಎಣಿಕೆ = qemu_get_present_cpus_ಎಣಿಕೆ();
    ಆದರೆ (Max_ಎಣಿಕೆ_CPUs < smp_ಎಣಿಕೆ)
        Max_ಎಣಿಕೆ_CPUs = smp_ಎಣಿಕೆ;

    smp_ಪರಿಶೀಲನೆ();
}

ಶೂನ್ಯ
smp_resume(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU)
        ಹಿಂತಿರುಗಿ;

    smp_write_msrs();
    smp_ಪರಿಶೀಲನೆ();
}
