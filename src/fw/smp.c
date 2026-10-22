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

#define APIC_ICR_ಕೆಳಗಿನ ((u8*)BUILD_APIC_ADDR + 0x300)
#define APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#define APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#define APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#define APIC_ENABLED 0x0100
#define MSR_IA32_APIC_BASE 0x01B
#define MSR_LOCAL_APIC_ID 0x802
#define MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

ಸ್ಥಿರ ಸಂರಚನಾ { u32 index; u64 val; } smp_msr[32];
ಸ್ಥಿರ u32 smp_msr_ಎಣಿಕೆ;

ಶೂನ್ಯ
wrmsr_smp(u32 index, u64 val)
{
    wrmsr(index, val);
    ಯದಿ (smp_msr_ಎಣಿಕೆ >= ARRAY_SIZE(smp_msr)) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    smp_msr[smp_msr_ಎಣಿಕೆ].index = index;
    smp_msr[smp_msr_ಎಣಿಕೆ].val = val;
    smp_msr_ಎಣಿಕೆ++;
}

ಸ್ಥಿರ ಶೂನ್ಯ
smp_write_msrs(ಶೂನ್ಯ)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<smp_msr_ಎಣಿಕೆ; i++)
        wrmsr(smp_msr[i].index, smp_msr[i].val);
}

u32 Max_ಎಣಿಕೆ_CPUs;
ಸ್ಥಿರ u32 ಎಣಿಕೆ_CPUs;
// 256 bits for the found APIC IDs
ಸ್ಥಿರ u32 FoundAPICIDs[256/32];

ಪೂರ್ಣಾಙ್ಕ apic_id_is_present(u8 apic_id)
{
    ಪ್ರತ್ಯಾಗಮ !!(FoundAPICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
apic_id_init(ಶೂನ್ಯ)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    ಯದಿ (Max_ಎಣಿಕೆ_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        FoundAPICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } ಅನ್ಯಥಾ ಯದಿ (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_base = rdmsr(MSR_IA32_APIC_BASE);
        wrmsr(MSR_IA32_APIC_BASE, apic_base | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } ಅನ್ಯಥಾ {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    ಪ್ರತ್ಯಾಗಮ apic_id;
}

ಶೂನ್ಯ VISIBLE32FLAT
handle_smp(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    // Track this CPU and detect the apic_id
    ಪೂರ್ಣಾಙ್ಕ apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    ಎಣಿಕೆ_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
ಸ್ಥಿರ ಶೂನ್ಯ
smp_scan(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ಯದಿ (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        ಎಣಿಕೆ_CPUs= 1;
        ಪ್ರತ್ಯಾಗಮ;
    }

    // mark the BSP initial APIC ID as found, too:
    ಎಣಿಕೆ_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    ಬಾಹ್ಯ ಶೂನ್ಯ entry_smp(ಶೂನ್ಯ);
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
    writel(APIC_ICR_ಕೆಳಗಿನ, 0x000C4500);
    u32 sipi_vector = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_ಕೆಳಗಿನ, 0x000C4600 | sipi_vector);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_ಎಣಿಕೆ = qemu_get_present_cpus_ಎಣಿಕೆ();
    ಯಾವತ್ (expected_cpus_ಎಣಿಕೆ != ಎಣಿಕೆ_CPUs)
        asm ಚಞ್ಚಲ(
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
smp_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    Max_ಎಣಿಕೆ_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_ಎಣಿಕೆ = qemu_get_present_cpus_ಎಣಿಕೆ();
    ಯದಿ (Max_ಎಣಿಕೆ_CPUs < smp_ಎಣಿಕೆ)
        Max_ಎಣಿಕೆ_CPUs = smp_ಎಣಿಕೆ;

    smp_scan();
}

ಶೂನ್ಯ
smp_resume(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_QEMU)
        ಪ್ರತ್ಯಾಗಮ;

    smp_write_msrs();
    smp_scan();
}
