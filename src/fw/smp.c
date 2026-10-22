// QEMU multi-CPU initialization code
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "hw/rtc.h" // CMOS_BIOS_SMP_COUNT
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadint
#شامل "stacks.h" // yield
#شامل "util.h" // smp_setup, msr_feature_control_setup
#شامل "x86.h" // wrmsr
#شامل "paravirt.h" // qemu_*_present_cpus_count

#تعریف_کرو APIC_ICR_کم ((u8*)BUILD_APIC_ADDR + 0x300)
#تعریف_کرو APIC_SVR     ((u8*)BUILD_APIC_ADDR + 0x0F0)
#تعریف_کرو APIC_LINT0   ((u8*)BUILD_APIC_ADDR + 0x350)
#تعریف_کرو APIC_LINT1   ((u8*)BUILD_APIC_ADDR + 0x360)

#تعریف_کرو APIC_ENABLED 0x0100
#تعریف_کرو MSR_IA32_APIC_بنیاد 0x01B
#تعریف_کرو MSR_LOCAL_APIC_ID 0x802
#تعریف_کرو MSR_IA32_APICBASE_EXTD (1ULL << 10) /* Enable x2APIC mode */

ساکن ساخت { u32 اشاریہ; u64 val; } smp_msr[32];
ساکن u32 smp_msr_گنتی;

خالی
wrmsr_smp(u32 اشاریہ, u64 val)
{
    wrmsr(اشاریہ, val);
    اگر (smp_msr_گنتی >= ARRAY_سائز(smp_msr)) {
        warn_noalloc();
        واپس;
    }
    smp_msr[smp_msr_گنتی].اشاریہ = اشاریہ;
    smp_msr[smp_msr_گنتی].val = val;
    smp_msr_گنتی++;
}

ساکن خالی
smp_write_msrs(خالی)
{
    // MTRR and MSR_IA32_FEATURE_CONTROL setup
    صحیح i;
    برائے (i=0; i<smp_msr_گنتی; i++)
        wrmsr(smp_msr[i].اشاریہ, smp_msr[i].val);
}

u32 Max_گنتی_CPUs;
ساکن u32 گنتی_CPUs;
// 256 bits for the found APIC IDs
ساکن u32 ملا_APICIDs[256/32];

صحیح apic_id_is_present(u8 apic_id)
{
    واپس !!(ملا_APICIDs[apic_id/32] & (1ul << (apic_id % 32)));
}

ساکن صحیح
apic_id_init(خالی)
{
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    u32 apic_id = ebx>>24;
    اگر (Max_گنتی_CPUs < 256) { // xAPIC mode
        // Track found apic id for use in legacy internal bios tables
        ملا_APICIDs[apic_id/32] |= 1 << (apic_id % 32);
    } ورنہ اگر (ecx & CPUID_X2APIC) {
        // switch to x2APIC mode
        u64 apic_بنیاد = rdmsr(MSR_IA32_APIC_بنیاد);
        wrmsr(MSR_IA32_APIC_بنیاد, apic_بنیاد | MSR_IA32_APICBASE_EXTD);
        apic_id = rdmsr(MSR_LOCAL_APIC_ID);
    } ورنہ {
        // x2APIC is masked by CPUID
        apic_id = -1;
    }
    واپس apic_id;
}

خالی VISIBLE32FLAT
handle_smp(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    // Track this CPU and detect the apic_id
    صحیح apic_id = apic_id_init();
    dprintf(DEBUG_HDL_smp, "handle_smp: apic_id=0x%x\n", apic_id);

    smp_write_msrs();

    گنتی_CPUs++;
}

// Atomic lock for shared stack across processors.
u32 SMPLock __VISIBLE;
u32 SMPStack __VISIBLE;

// find and initialize the CPUs by launching a SIPI to them
ساکن خالی
smp_جائزہ(خالی)
{
    ASSERT32FLAT();
    u32 eax, ebx, ecx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    اگر (eax < 1 || !(cpuid_features & CPUID_APIC)) {
        // No apic - only the main cpu is present.
        dprintf(1, "No apic - only the main cpu is present.\n");
        گنتی_CPUs= 1;
        واپس;
    }

    // mark the BSP initial APIC ID as found, too:
    گنتی_CPUs = 1;

    // Setup jump trampoline to counter code.
    u64 old = *(u64*)BUILD_AP_BOOT_ADDR;
    // ljmpw $SEG_BIOS, $(entry_smp - BUILD_BIOS_ADDR)
    خارجی خالی اندراج_smp(خالی);
    u64 new = (0xea | ((u64)SEG_BIOS<<24)
               | (((u32)اندراج_smp - BUILD_BIOS_ADDR) << 8));
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
    writel(APIC_ICR_کم, 0x000C4500);
    u32 sipi_سمتیہ = BUILD_AP_BOOT_ADDR >> 12;
    writel(APIC_ICR_کم, 0x000C4600 | sipi_سمتیہ);

    // switch to x2APIC mode after sending SIPI so that
    // x2APIC and xAPIC mode could share AP wake up code
    apic_id_init();

    // Wait for other CPUs to process the SIPI.
    u16 expected_cpus_گنتی = qemu_get_present_cpus_گنتی();
    جب (expected_cpus_گنتی != گنتی_CPUs)
        asm متلون(
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

    dprintf(1, "Found %d cpu(s) max supported %d cpu(s)\n", گنتی_CPUs,
            Max_گنتی_CPUs);
}

خالی
smp_تیاری(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    Max_گنتی_CPUs = romfile_loadint("etc/max-cpus", 0);
    u16 smp_گنتی = qemu_get_present_cpus_گنتی();
    اگر (Max_گنتی_CPUs < smp_گنتی)
        Max_گنتی_CPUs = smp_گنتی;

    smp_جائزہ();
}

خالی
smp_resume(خالی)
{
    اگر (!CONFIG_QEMU)
        واپس;

    smp_write_msrs();
    smp_جائزہ();
}
