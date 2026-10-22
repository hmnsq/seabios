// Initialize MTRRs - mostly useful on KVM.
//
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // RamSize
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // mtrr_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // cpuid

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRcap                    0x000000fe
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix64K_00000           0x00000250
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix16K_80000           0x00000258
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix16K_A0000           0x00000259
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_C0000            0x00000268
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_C8000            0x00000269
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_D0000            0x0000026a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_D8000            0x0000026b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_E0000            0x0000026c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_E8000            0x0000026d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_F0000            0x0000026e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRfix4K_F8000            0x0000026f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSR_MTRRdefType                0x000002ff

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MTRRphys_ਅਧਾਰ_MSR(reg) (0x200 + 2 * (reg))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MTRRphys_ਨਕਾਬ_MSR(reg) (0x200 + 2 * (reg) + 1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MTRR_MEMTYPE_UC 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MTRR_MEMTYPE_WC 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MTRR_MEMTYPE_WT 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MTRR_MEMTYPE_WP 5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MTRR_MEMTYPE_WB 6

ਖਾਲੀ mtrr_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_MTRR_INIT)
        ਵਾਪਸ;

    u32 eax, ebx, ecx, edx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ਜੇ (!(cpuid_features & CPUID_MTRR))
        ਵਾਪਸ;
    ਜੇ (!(cpuid_features & CPUID_MSR))
        ਵਾਪਸ;

    dprintf(3, "init mtrr\n");

    u32 mtrr_cap = rdmsr(MSR_MTRRcap);
    ਪੂਰਨ vcnt = mtrr_cap & 0xff;
    ਪੂਰਨ fix = mtrr_cap & 0x100;
    ਜੇ (!vcnt || !fix)
       ਵਾਪਸ;

    // Disable MTRRs
    wrmsr_smp(MSR_MTRRdefType, 0);

    // Set fixed MTRRs
    ਸੰਘ u64b {
        u8 valb[8];
        u64 val;
    } u;
    u.val = 0;
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < 8; i++)
        ਜੇ (Ram_ਮਾਪ >= 65536 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix64K_00000, u.val);
    u.val = 0;
    ਲਈ (i = 0; i < 8; i++)
        ਜੇ (Ram_ਮਾਪ >= 0x80000 + 16384 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix16K_80000, u.val);
    wrmsr_smp(MSR_MTRRfix16K_A0000, 0);   // 0xA0000-0xC0000 is uncached
    ਪੂਰਨ j;
    ਲਈ (j = 0; j < 8; j++) {
        u.val = 0;
        ਲਈ (i = 0; i < 8; i++)
            ਜੇ (Ram_ਮਾਪ >= 0xC0000 + j * 0x8000 + 4096 * (i + 1))
                u.valb[i] = MTRR_MEMTYPE_WP;
        wrmsr_smp(MSR_MTRRfix4K_C0000 + j, u.val);
    }

    // Set variable MTRRs
    ਪੂਰਨ phys_ਬਿੱਟ = 36;
    cpuid(0x80000000u, &eax, &ebx, &ecx, &edx);
    ਜੇ (eax >= 0x80000008) {
        /* Get physical bits from leaf 0x80000008 (if available) */
        cpuid(0x80000008u, &eax, &ebx, &ecx, &edx);
        phys_ਬਿੱਟ = eax & 0xff;
    }
    u64 phys_ਨਕਾਬ = ((1ull << phys_ਬਿੱਟ) - 1);
    ਲਈ (i=0; i<vcnt; i++) {
        wrmsr_smp(MTRRphys_ਅਧਾਰ_MSR(i), 0);
        wrmsr_smp(MTRRphys_ਨਕਾਬ_MSR(i), 0);
    }
    /* Mark 3.5-4GB as UC, anything not specified defaults to WB */
    wrmsr_smp(MTRRphys_ਅਧਾਰ_MSR(0), pcimem_ਸ਼ੁਰੂ | MTRR_MEMTYPE_UC);
    wrmsr_smp(MTRRphys_ਨਕਾਬ_MSR(0)
              , (-((1ull<<32)-pcimem_ਸ਼ੁਰੂ) & phys_ਨਕਾਬ) | 0x800);

    // Enable fixed and variable MTRRs; set default type.
    wrmsr_smp(MSR_MTRRdefType, 0xc00 | MTRR_MEMTYPE_WB);
}
