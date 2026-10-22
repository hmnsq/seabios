// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "util.h" // smp_setup, msr_feature_control_setup
#అన్తర్భావ "x86.h" // wrmsr
#అన్తర్భావ "paravirt.h" // qemu_*_present_cpus_count

#నిర్వచించు APIC_ICR_దిగువ ((u8*)BUILD_APIC_ADDR + 0x300)
#నిర్వచించు APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#నిర్వచించు APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#నిర్వచించు APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#నిర్వచించు APIC_ENABLED 0x0100
#నిర్వచించు MSR_IA32_APIC_పునాది 0x01B
#నిర్వచించు MSR_LOCAL_APIC_ID 0x802
#నిర్వచించు MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

స్థిర నిర్మాణం { u32 సూచీ; u64 val; } smp_msr[32];
స్థిర u32 smp_msr_లెక్క;

శూన్య
wrmsr_smp(u32 సూచీ, u64 val)
{
    wrmsr(సూచీ, val);
    ఒకవేళ (smp_msr_లెక్క >= ARRAY_కొలత(smp_msr)) {
        warn_noalloc();
        తిరిగి;
    }
    smp_msr[smp_msr_లెక్క].సూచీ = సూచీ;
    smp_msr[smp_msr_లెక్క].val = val;
    smp_msr_లెక్క++;
}

స్థిర శూన్య
smp_write_msrs(శూన్య)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    పూర్ణాంకం i;
    కొరకు (i=0; i<smp_msr_లెక్క; i++)
        wrmsr(smp_msr[i].సూచీ, smp_msr[i].val);
}

u32 Max_లెక్క_CPUs;
స్థిర u32 లెక్క_CPUs;
// 256 bits for the found APIC IDs
స్థిర u32 దొరికింది_APICIDs[256/32];

పూర్ణాంకం apic_id_is_present(u8 apic_id)
{
    తిరిగి !!(దొరికింది_APICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

స్థిర పూర్ణాంకం
apic_id_init(శూన్య)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    ఒకవేళ (Max_లెక్క_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        దొరికింది_APICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } లేకపోతే ఒకవేళ (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_పునాది = rdmsr(MSR_IA32_APIC_పునాది);
        wrmsr(MSR_IA32_APIC_పునాది, apic_పునాది | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } లేకపోతే {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    తిరిగి apic_id;
}

శూన్య VISIBLE32FLAT
handle_smp(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    // Track this CPU and detect the apic_id
    పూర్ణాంకం apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    లెక్క_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
స్థిర శూన్య
smp_పరిశీలన(శూన్య)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ఒకవేళ (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        లెక్క_CPUs= 1;
        తిరిగి;
    }

    // mark the BSP initial APIC ID as found, too:
    లెక్క_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    బాహ్య శూన్య నమోదు_smp(శూన్య);
    u64 new = (0xea | ((u64)SEG_BIOS<<24)
               | (((u32)నమోదు_smp - BUILD_BIOS_ADDR) << 8));
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
    writel(APIC_ICR_దిగువ, 0x000C4500);
    u32 sipi_సదిశ = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_దిగువ, 0x000C4600 | sipi_సదిశ);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_లెక్క = qemu_get_present_cpus_లెక్క();
    అయితే (expected_cpus_లెక్క != లెక్క_CPUs)
        asm అస్థిర(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", లెక్క_CPUs,
            Max_లెక్క_CPUs);
}

శూన్య
smp_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    Max_లెక్క_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_లెక్క = qemu_get_present_cpus_లెక్క();
    ఒకవేళ (Max_లెక్క_CPUs < smp_లెక్క)
        Max_లెక్క_CPUs = smp_లెక్క;

    smp_పరిశీలన();
}

శూన్య
smp_resume(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU)
        తిరిగి;

    smp_write_msrs();
    smp_పరిశీలన();
}
