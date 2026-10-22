// Initialize MTRRs - mostly useful on KVM.
//
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // RamSize
#ಅನ್ತರ್ಭಾವ "util.h" // mtrr_setup
#ಅನ್ತರ್ಭಾವ "x86.h" // cpuid

#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRcap                    0x000000fe
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix64K_00000           0x00000250
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix16K_80000           0x00000258
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix16K_A0000           0x00000259
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_C0000            0x00000268
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_C8000            0x00000269
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_D0000            0x0000026a
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_D8000            0x0000026b
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_E0000            0x0000026c
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_E8000            0x0000026d
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_F0000            0x0000026e
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRfix4K_F8000            0x0000026f
#ವ್ಯಾಖ್ಯಾನಿಸು MSR_MTRRdefType                0x000002ff

#ವ್ಯಾಖ್ಯಾನಿಸು MTRRphys_ತಳ_MSR(reg) (0x200 + 2 * (reg))
#ವ್ಯಾಖ್ಯಾನಿಸು MTRRphys_ಮುಖವಾಡ_MSR(reg) (0x200 + 2 * (reg) + 1)

#ವ್ಯಾಖ್ಯಾನಿಸು MTRR_MEMTYPE_UC 0
#ವ್ಯಾಖ್ಯಾನಿಸು MTRR_MEMTYPE_WC 1
#ವ್ಯಾಖ್ಯಾನಿಸು MTRR_MEMTYPE_WT 4
#ವ್ಯಾಖ್ಯಾನಿಸು MTRR_MEMTYPE_WP 5
#ವ್ಯಾಖ್ಯಾನಿಸು MTRR_MEMTYPE_WB 6

ಶೂನ್ಯ mtrr_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_MTRR_INIT)
        ಹಿಂತಿರುಗಿ;

    u32 eax, ebx, ecx, edx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ಆದರೆ (!(cpuid_features & CPUID_MTRR))
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!(cpuid_features & CPUID_MSR))
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init mtrr\n");

    u32 mtrr_cap = rdmsr(MSR_MTRRcap);
    ಪೂರ್ಣಾಂಕ vcnt = mtrr_cap & 0xff;
    ಪೂರ್ಣಾಂಕ fix = mtrr_cap & 0x100;
    ಆದರೆ (!vcnt || !fix)
       ಹಿಂತಿರುಗಿ;

    // Disable MTRRs
    wrmsr_smp(MSR_MTRRdefType, 0);

    // Set fixed MTRRs
    ಒಕ್ಕೂಟ u64b {
        u8 valb[8];
        u64 val;
    } u;
    u.val = 0;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < 8; i++)
        ಆದರೆ (Ram_ಗಾತ್ರ >= 65536 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix64K_00000, u.val);
    u.val = 0;
    ಗಾಗಿ (i = 0; i < 8; i++)
        ಆದರೆ (Ram_ಗಾತ್ರ >= 0x80000 + 16384 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix16K_80000, u.val);
    wrmsr_smp(MSR_MTRRfix16K_A0000, 0);   // 0xA0000-0xC0000 is uncached
    ಪೂರ್ಣಾಂಕ j;
    ಗಾಗಿ (j = 0; j < 8; j++) {
        u.val = 0;
        ಗಾಗಿ (i = 0; i < 8; i++)
            ಆದರೆ (Ram_ಗಾತ್ರ >= 0xC0000 + j * 0x8000 + 4096 * (i + 1))
                u.valb[i] = MTRR_MEMTYPE_WP;
        wrmsr_smp(MSR_MTRRfix4K_C0000 + j, u.val);
    }

    // Set variable MTRRs
    ಪೂರ್ಣಾಂಕ phys_ಬಿಟ್ಟುಗಳು = 36;
    cpuid(0x80000000u, &eax, &ebx, &ecx, &edx);
    ಆದರೆ (eax >= 0x80000008) {
        /* Get physical bits from leaf 0x80000008 (if available) */
        cpuid(0x80000008u, &eax, &ebx, &ecx, &edx);
        phys_ಬಿಟ್ಟುಗಳು = eax & 0xff;
    }
    u64 phys_ಮುಖವಾಡ = ((1ull << phys_ಬಿಟ್ಟುಗಳು) - 1);
    ಗಾಗಿ (i=0; i<vcnt; i++) {
        wrmsr_smp(MTRRphys_ತಳ_MSR(i), 0);
        wrmsr_smp(MTRRphys_ಮುಖವಾಡ_MSR(i), 0);
    }
    /* Mark 3.5-4GB as UC, anything not specified defaults to WB */
    wrmsr_smp(MTRRphys_ತಳ_MSR(0), pcimem_ಆರಂಭ | MTRR_MEMTYPE_UC);
    wrmsr_smp(MTRRphys_ಮುಖವಾಡ_MSR(0)
              , (-((1ull<<32)-pcimem_ಆರಂಭ) & phys_ಮುಖವಾಡ) | 0x800);

    // Enable fixed and variable MTRRs; set default type.
    wrmsr_smp(MSR_MTRRdefType, 0xc00 | MTRR_MEMTYPE_WB);
}
