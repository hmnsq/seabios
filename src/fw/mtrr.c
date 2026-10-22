// Initialize MTRRs - mostly useful on KVM.
//
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // RamSize
#అన్తర్భావ "util.h" // mtrr_setup
#అన్తర్భావ "x86.h" // cpuid

#నిర్వచించు MSR_MTRRcap                    0x000000fe
#నిర్వచించు MSR_MTRRfix64K_00000           0x00000250
#నిర్వచించు MSR_MTRRfix16K_80000           0x00000258
#నిర్వచించు MSR_MTRRfix16K_A0000           0x00000259
#నిర్వచించు MSR_MTRRfix4K_C0000            0x00000268
#నిర్వచించు MSR_MTRRfix4K_C8000            0x00000269
#నిర్వచించు MSR_MTRRfix4K_D0000            0x0000026a
#నిర్వచించు MSR_MTRRfix4K_D8000            0x0000026b
#నిర్వచించు MSR_MTRRfix4K_E0000            0x0000026c
#నిర్వచించు MSR_MTRRfix4K_E8000            0x0000026d
#నిర్వచించు MSR_MTRRfix4K_F0000            0x0000026e
#నిర్వచించు MSR_MTRRfix4K_F8000            0x0000026f
#నిర్వచించు MSR_MTRRdefType                0x000002ff

#నిర్వచించు MTRRphys_పునాది_MSR(reg) (0x200 + 2 * (reg))
#నిర్వచించు MTRRphys_ముసుగు_MSR(reg) (0x200 + 2 * (reg) + 1)

#నిర్వచించు MTRR_MEMTYPE_UC 0
#నిర్వచించు MTRR_MEMTYPE_WC 1
#నిర్వచించు MTRR_MEMTYPE_WT 4
#నిర్వచించు MTRR_MEMTYPE_WP 5
#నిర్వచించు MTRR_MEMTYPE_WB 6

శూన్య mtrr_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_MTRR_INIT)
        తిరిగి;

    u32 eax, ebx, ecx, edx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ఒకవేళ (!(cpuid_features & CPUID_MTRR))
        తిరిగి;
    ఒకవేళ (!(cpuid_features & CPUID_MSR))
        తిరిగి;

    dprintf(3, "init mtrr\n");

    u32 mtrr_cap = rdmsr(MSR_MTRRcap);
    పూర్ణాంకం vcnt = mtrr_cap & 0xff;
    పూర్ణాంకం fix = mtrr_cap & 0x100;
    ఒకవేళ (!vcnt || !fix)
       తిరిగి;

    // Disable MTRRs
    wrmsr_smp(MSR_MTRRdefType, 0);

    // Set fixed MTRRs
    సంఘం u64b {
        u8 valb[8];
        u64 val;
    } u;
    u.val = 0;
    పూర్ణాంకం i;
    కొరకు (i = 0; i < 8; i++)
        ఒకవేళ (Ram_కొలత >= 65536 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix64K_00000, u.val);
    u.val = 0;
    కొరకు (i = 0; i < 8; i++)
        ఒకవేళ (Ram_కొలత >= 0x80000 + 16384 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix16K_80000, u.val);
    wrmsr_smp(MSR_MTRRfix16K_A0000, 0);   // 0xA0000-0xC0000 is uncached
    పూర్ణాంకం j;
    కొరకు (j = 0; j < 8; j++) {
        u.val = 0;
        కొరకు (i = 0; i < 8; i++)
            ఒకవేళ (Ram_కొలత >= 0xC0000 + j * 0x8000 + 4096 * (i + 1))
                u.valb[i] = MTRR_MEMTYPE_WP;
        wrmsr_smp(MSR_MTRRfix4K_C0000 + j, u.val);
    }

    // Set variable MTRRs
    పూర్ణాంకం phys_బిట్లు = 36;
    cpuid(0x80000000u, &eax, &ebx, &ecx, &edx);
    ఒకవేళ (eax >= 0x80000008) {
        /* Get physical bits from leaf 0x80000008 (if available) */
        cpuid(0x80000008u, &eax, &ebx, &ecx, &edx);
        phys_బిట్లు = eax & 0xff;
    }
    u64 phys_ముసుగు = ((1ull << phys_బిట్లు) - 1);
    కొరకు (i=0; i<vcnt; i++) {
        wrmsr_smp(MTRRphys_పునాది_MSR(i), 0);
        wrmsr_smp(MTRRphys_ముసుగు_MSR(i), 0);
    }
    /* Mark 3.5-4GB as UC, anything not specified defaults to WB */
    wrmsr_smp(MTRRphys_పునాది_MSR(0), pcimem_ప్రారంభం | MTRR_MEMTYPE_UC);
    wrmsr_smp(MTRRphys_ముసుగు_MSR(0)
              , (-((1ull<<32)-pcimem_ప్రారంభం) & phys_ముసుగు) | 0x800);

    // Enable fixed and variable MTRRs; set default type.
    wrmsr_smp(MSR_MTRRdefType, 0xc00 | MTRR_MEMTYPE_WB);
}
