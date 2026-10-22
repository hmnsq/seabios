// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_loadint
#समावेश "stacks.h" // yield
#समावेश "util.h" // smp_setup, msr_feature_control_setup
#समावेश "x86.h" // wrmsr
#समावेश "paravirt.h" // qemu_*_present_cpus_count

#घोषणा APIC_ICR_निचला ((u8*)BUILD_APIC_ADDR + 0x300)
#घोषणा APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#घोषणा APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#घोषणा APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#घोषणा APIC_ENABLED 0x0100
#घोषणा MSR_IA32_APIC_BASE 0x01B
#घोषणा MSR_LOCAL_APIC_ID 0x802
#घोषणा MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

अटल काष्ठा { u32 सूचकांक; u64 val; } smp_msr[32];
अटल u32 smp_msr_गिनती;

व्योम
wrmsr_smp(u32 सूचकांक, u64 val)
{
    wrmsr(सूचकांक, val);
    अगर (smp_msr_गिनती >= ARRAY_आकार(smp_msr)) {
        warn_noalloc();
        वापस;
    }
    smp_msr[smp_msr_गिनती].सूचकांक = सूचकांक;
    smp_msr[smp_msr_गिनती].val = val;
    smp_msr_गिनती++;
}

अटल व्योम
smp_write_msrs(व्योम)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    पूर्णांक i;
    क्रम (i=0; i<smp_msr_गिनती; i++)
        wrmsr(smp_msr[i].सूचकांक, smp_msr[i].val);
}

u32 Max_गिनती_CPUs;
अटल u32 गिनती_CPUs;
// 256 bits for the found APIC IDs
अटल u32 FoundAPICIDs[256/32];

पूर्णांक apic_id_is_present(u8 apic_id)
{
    वापस !!(FoundAPICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

अटल पूर्णांक
apic_id_init(व्योम)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    अगर (Max_गिनती_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        FoundAPICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } अन्यथा अगर (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_base = rdmsr(MSR_IA32_APIC_BASE);
        wrmsr(MSR_IA32_APIC_BASE, apic_base | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } अन्यथा {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    वापस apic_id;
}

व्योम VISIBLE32FLAT
handle_smp(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    // Track this CPU and detect the apic_id
    पूर्णांक apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    गिनती_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
अटल व्योम
smp_scan(व्योम)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    अगर (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        गिनती_CPUs= 1;
        वापस;
    }

    // mark the BSP initial APIC ID as found, too:
    गिनती_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    बाह्य व्योम entry_smp(व्योम);
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
    writel(APIC_ICR_निचला, 0x000C4500);
    u32 sipi_vector = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_निचला, 0x000C4600 | sipi_vector);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_गिनती = qemu_get_present_cpus_गिनती();
    जबतक (expected_cpus_गिनती != गिनती_CPUs)
        यंत्र अस्थिर(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", गिनती_CPUs,
            Max_गिनती_CPUs);
}

व्योम
smp_setup(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    Max_गिनती_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_गिनती = qemu_get_present_cpus_गिनती();
    अगर (Max_गिनती_CPUs < smp_गिनती)
        Max_गिनती_CPUs = smp_गिनती;

    smp_scan();
}

व्योम
smp_resume(व्योम)
{
    अगर (!CONFIG_QEMU)
        वापस;

    smp_write_msrs();
    smp_scan();
}
