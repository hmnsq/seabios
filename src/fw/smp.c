// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // smp_setup, msr_feature_control_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // wrmsr
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // qemu_*_present_cpus_count

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APIC_ICR_ਨੀਵਾਂ ((u8*)BUILD_APIC_ADDR + 0x300)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APIC_ENABLED 0x0100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_IA32_APIC_ਅਧਾਰ 0x01B
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_LOCAL_APIC_ID 0x802
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

ਸਥਿਰ ਬਣਤਰ { u32 ਸੂਚਕਾਂਕ; u64 val; } smp_msr[32];
ਸਥਿਰ u32 smp_msr_ਗਿਣਤੀ;

ਖਾਲੀ
wrmsr_smp(u32 ਸੂਚਕਾਂਕ, u64 val)
{
    wrmsr(ਸੂਚਕਾਂਕ, val);
    ਜੇ (smp_msr_ਗਿਣਤੀ >= ARRAY_ਮਾਪ(smp_msr)) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    smp_msr[smp_msr_ਗਿਣਤੀ].ਸੂਚਕਾਂਕ = ਸੂਚਕਾਂਕ;
    smp_msr[smp_msr_ਗਿਣਤੀ].val = val;
    smp_msr_ਗਿਣਤੀ++;
}

ਸਥਿਰ ਖਾਲੀ
smp_write_msrs(ਖਾਲੀ)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    ਪੂਰਨ i;
    ਲਈ (i=0; i<smp_msr_ਗਿਣਤੀ; i++)
        wrmsr(smp_msr[i].ਸੂਚਕਾਂਕ, smp_msr[i].val);
}

u32 Max_ਗਿਣਤੀ_CPUs;
ਸਥਿਰ u32 ਗਿਣਤੀ_CPUs;
// 256 bits for the found APIC IDs
ਸਥਿਰ u32 ਮਿਲਿਆ_APICIDs[256/32];

ਪੂਰਨ apic_id_is_present(u8 apic_id)
{
    ਵਾਪਸ !!(ਮਿਲਿਆ_APICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

ਸਥਿਰ ਪੂਰਨ
apic_id_init(ਖਾਲੀ)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    ਜੇ (Max_ਗਿਣਤੀ_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        ਮਿਲਿਆ_APICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } ਨਹੀਂਤਾਂ ਜੇ (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_ਅਧਾਰ = rdmsr(MSR_IA32_APIC_ਅਧਾਰ);
        wrmsr(MSR_IA32_APIC_ਅਧਾਰ, apic_ਅਧਾਰ | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } ਨਹੀਂਤਾਂ {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    ਵਾਪਸ apic_id;
}

ਖਾਲੀ VISIBLE32FLAT
handle_smp(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    // Track this CPU and detect the apic_id
    ਪੂਰਨ apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    ਗਿਣਤੀ_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
ਸਥਿਰ ਖਾਲੀ
smp_ਜਾਂਚ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ਜੇ (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        ਗਿਣਤੀ_CPUs= 1;
        ਵਾਪਸ;
    }

    // mark the BSP initial APIC ID as found, too:
    ਗਿਣਤੀ_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_smp(ਖਾਲੀ);
    u64 new = (0xea | ((u64)SEG_BIOS<<24)
               | (((u32)ਇੰਦਰਾਜ_smp - BUILD_BIOS_ADDR) << 8));
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
    writel(APIC_ICR_ਨੀਵਾਂ, 0x000C4500);
    u32 sipi_ਸਦਿਸ਼ = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_ਨੀਵਾਂ, 0x000C4600 | sipi_ਸਦਿਸ਼);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_ਗਿਣਤੀ = qemu_get_present_cpus_ਗਿਣਤੀ();
    ਜਦੋਂ (expected_cpus_ਗਿਣਤੀ != ਗਿਣਤੀ_CPUs)
        asm ਅਸਥਿਰ(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", ਗਿਣਤੀ_CPUs,
            Max_ਗਿਣਤੀ_CPUs);
}

ਖਾਲੀ
smp_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    Max_ਗਿਣਤੀ_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_ਗਿਣਤੀ = qemu_get_present_cpus_ਗਿਣਤੀ();
    ਜੇ (Max_ਗਿਣਤੀ_CPUs < smp_ਗਿਣਤੀ)
        Max_ਗਿਣਤੀ_CPUs = smp_ਗਿਣਤੀ;

    smp_ਜਾਂਚ();
}

ਖਾਲੀ
smp_resume(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU)
        ਵਾਪਸ;

    smp_write_msrs();
    smp_ਜਾਂਚ();
}
