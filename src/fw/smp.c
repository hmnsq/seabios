// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadint
#ضمن "stacks.h" // yield
#ضمن "util.h" // smp_setup, msr_feature_control_setup
#ضمن "x86.h" // wrmsr
#ضمن "paravirt.h" // qemu_*_present_cpus_count

#define APIC_ICR_أدنى ((u8*)BUILD_APIC_ADDR + 0x300)
#define APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#define APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#define APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#define APIC_ENABLED 0x0100
#define MSR_IA32_APIC_BASE 0x01B
#define MSR_LOCAL_APIC_ID 0x802
#define MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

ساكن بنية { u32 index; u64 val; } smp_msr[32];
ساكن u32 smp_msr_تعداد;

فراغ
wrmsr_smp(u32 index, u64 val)
{
    wrmsr(index, val);
    إذا (smp_msr_تعداد >= ARRAY_SIZE(smp_msr)) {
        warn_noalloc();
        إرجاع;
    }
    smp_msr[smp_msr_تعداد].index = index;
    smp_msr[smp_msr_تعداد].val = val;
    smp_msr_تعداد++;
}

ساكن فراغ
smp_write_msrs(فراغ)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    صحيح i;
    لكل (i=0; i<smp_msr_تعداد; i++)
        wrmsr(smp_msr[i].index, smp_msr[i].val);
}

u32 Max_تعداد_CPUs;
ساكن u32 تعداد_CPUs;
// 256 bits for the found APIC IDs
ساكن u32 FoundAPICIDs[256/32];

صحيح apic_id_is_present(u8 apic_id)
{
    إرجاع !!(FoundAPICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

ساكن صحيح
apic_id_init(فراغ)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    إذا (Max_تعداد_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        FoundAPICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } وإلا إذا (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_base = rdmsr(MSR_IA32_APIC_BASE);
        wrmsr(MSR_IA32_APIC_BASE, apic_base | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } وإلا {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    إرجاع apic_id;
}

فراغ VISIBLE32FLAT
handle_smp(فراغ)
{
    إذا (!CONFIG_QEMU)
        إرجاع;

    // Track this CPU and detect the apic_id
    صحيح apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    تعداد_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
ساكن فراغ
smp_scan(فراغ)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    إذا (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        تعداد_CPUs= 1;
        إرجاع;
    }

    // mark the BSP initial APIC ID as found, too:
    تعداد_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    خارجي فراغ entry_smp(فراغ);
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
    writel(APIC_ICR_أدنى, 0x000C4500);
    u32 sipi_vector = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_أدنى, 0x000C4600 | sipi_vector);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_تعداد = qemu_get_present_cpus_تعداد();
    بينما (expected_cpus_تعداد != تعداد_CPUs)
        asm متطاير(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", تعداد_CPUs,
            Max_تعداد_CPUs);
}

فراغ
smp_setup(فراغ)
{
    إذا (!CONFIG_QEMU)
        إرجاع;

    Max_تعداد_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_تعداد = qemu_get_present_cpus_تعداد();
    إذا (Max_تعداد_CPUs < smp_تعداد)
        Max_تعداد_CPUs = smp_تعداد;

    smp_scan();
}

فراغ
smp_resume(فراغ)
{
    إذا (!CONFIG_QEMU)
        إرجاع;

    smp_write_msrs();
    smp_scan();
}
