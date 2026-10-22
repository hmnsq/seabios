// Initialize MTRRs - mostly useful on KVM.
//
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "output.h" // dprintf
#شامل "paravirt.h" // RamSize
#شامل "util.h" // mtrr_setup
#شامل "x86.h" // cpuid

#تعریف_کرو MSR_MTRRcap                    0x000000fe
#تعریف_کرو MSR_MTRRfix64K_00000           0x00000250
#تعریف_کرو MSR_MTRRfix16K_80000           0x00000258
#تعریف_کرو MSR_MTRRfix16K_A0000           0x00000259
#تعریف_کرو MSR_MTRRfix4K_C0000            0x00000268
#تعریف_کرو MSR_MTRRfix4K_C8000            0x00000269
#تعریف_کرو MSR_MTRRfix4K_D0000            0x0000026a
#تعریف_کرو MSR_MTRRfix4K_D8000            0x0000026b
#تعریف_کرو MSR_MTRRfix4K_E0000            0x0000026c
#تعریف_کرو MSR_MTRRfix4K_E8000            0x0000026d
#تعریف_کرو MSR_MTRRfix4K_F0000            0x0000026e
#تعریف_کرو MSR_MTRRfix4K_F8000            0x0000026f
#تعریف_کرو MSR_MTRRdefType                0x000002ff

#تعریف_کرو MTRRphys_بنیاد_MSR(reg) (0x200 + 2 * (reg))
#تعریف_کرو MTRRphys_نقاب_MSR(reg) (0x200 + 2 * (reg) + 1)

#تعریف_کرو MTRR_MEMTYPE_UC 0
#تعریف_کرو MTRR_MEMTYPE_WC 1
#تعریف_کرو MTRR_MEMTYPE_WT 4
#تعریف_کرو MTRR_MEMTYPE_WP 5
#تعریف_کرو MTRR_MEMTYPE_WB 6

خالی mtrr_تیاری(خالی)
{
    اگر (!CONFIG_MTRR_INIT)
        واپس;

    u32 eax, ebx, ecx, edx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    اگر (!(cpuid_features & CPUID_MTRR))
        واپس;
    اگر (!(cpuid_features & CPUID_MSR))
        واپس;

    dprintf(3, "init mtrr\n");

    u32 mtrr_cap = rdmsr(MSR_MTRRcap);
    صحیح vcnt = mtrr_cap & 0xff;
    صحیح fix = mtrr_cap & 0x100;
    اگر (!vcnt || !fix)
       واپس;

    // Disable MTRRs
    wrmsr_smp(MSR_MTRRdefType, 0);

    // Set fixed MTRRs
    اتحاد u64b {
        u8 valb[8];
        u64 val;
    } u;
    u.val = 0;
    صحیح i;
    برائے (i = 0; i < 8; i++)
        اگر (Ram_سائز >= 65536 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix64K_00000, u.val);
    u.val = 0;
    برائے (i = 0; i < 8; i++)
        اگر (Ram_سائز >= 0x80000 + 16384 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix16K_80000, u.val);
    wrmsr_smp(MSR_MTRRfix16K_A0000, 0);   // 0xA0000-0xC0000 is uncached
    صحیح j;
    برائے (j = 0; j < 8; j++) {
        u.val = 0;
        برائے (i = 0; i < 8; i++)
            اگر (Ram_سائز >= 0xC0000 + j * 0x8000 + 4096 * (i + 1))
                u.valb[i] = MTRR_MEMTYPE_WP;
        wrmsr_smp(MSR_MTRRfix4K_C0000 + j, u.val);
    }

    // Set variable MTRRs
    صحیح phys_بٹ = 36;
    cpuid(0x80000000u, &eax, &ebx, &ecx, &edx);
    اگر (eax >= 0x80000008) {
        /* Get physical bits from leaf 0x80000008 (if available) */
        cpuid(0x80000008u, &eax, &ebx, &ecx, &edx);
        phys_بٹ = eax & 0xff;
    }
    u64 phys_نقاب = ((1ull << phys_بٹ) - 1);
    برائے (i=0; i<vcnt; i++) {
        wrmsr_smp(MTRRphys_بنیاد_MSR(i), 0);
        wrmsr_smp(MTRRphys_نقاب_MSR(i), 0);
    }
    /* Mark 3.5-4GB as UC, anything not specified defaults to WB */
    wrmsr_smp(MTRRphys_بنیاد_MSR(0), pcimem_آغاز | MTRR_MEMTYPE_UC);
    wrmsr_smp(MTRRphys_نقاب_MSR(0)
              , (-((1ull<<32)-pcimem_آغاز) & phys_نقاب) | 0x800);

    // Enable fixed and variable MTRRs; set default type.
    wrmsr_smp(MSR_MTRRdefType, 0xc00 | MTRR_MEMTYPE_WB);
}
