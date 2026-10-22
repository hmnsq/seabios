// Initialize MTRRs - mostly useful on KVM.
//
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // RamSize
#समावेश "util.h" // mtrr_setup
#समावेश "x86.h" // cpuid

#घोषणा MSR_MTRRcap                    0x000000fe
#घोषणा MSR_MTRRfix64K_00000           0x00000250
#घोषणा MSR_MTRRfix16K_80000           0x00000258
#घोषणा MSR_MTRRfix16K_A0000           0x00000259
#घोषणा MSR_MTRRfix4K_C0000            0x00000268
#घोषणा MSR_MTRRfix4K_C8000            0x00000269
#घोषणा MSR_MTRRfix4K_D0000            0x0000026a
#घोषणा MSR_MTRRfix4K_D8000            0x0000026b
#घोषणा MSR_MTRRfix4K_E0000            0x0000026c
#घोषणा MSR_MTRRfix4K_E8000            0x0000026d
#घोषणा MSR_MTRRfix4K_F0000            0x0000026e
#घोषणा MSR_MTRRfix4K_F8000            0x0000026f
#घोषणा MSR_MTRRdefType                0x000002ff

#घोषणा MTRRphysBase_MSR(reg) (0x200 + 2 * (reg))
#घोषणा MTRRphysMask_MSR(reg) (0x200 + 2 * (reg) + 1)

#घोषणा MTRR_MEMTYPE_UC 0
#घोषणा MTRR_MEMTYPE_WC 1
#घोषणा MTRR_MEMTYPE_WT 4
#घोषणा MTRR_MEMTYPE_WP 5
#घोषणा MTRR_MEMTYPE_WB 6

व्योम mtrr_setup(व्योम)
{
    अगर (!CONFIG_MTRR_INIT)
        वापस;

    u32 eax, ebx, ecx, edx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    अगर (!(cpuid_features & CPUID_MTRR))
        वापस;
    अगर (!(cpuid_features & CPUID_MSR))
        वापस;

    dprintf(3, "init mtrr\n");

    u32 mtrr_cap = rdmsr(MSR_MTRRcap);
    पूर्णांक vcnt = mtrr_cap & 0xff;
    पूर्णांक fix = mtrr_cap & 0x100;
    अगर (!vcnt || !fix)
       वापस;

    // Disable MTRRs
    wrmsr_smp(MSR_MTRRdefType, 0);

    // Set fixed MTRRs
    जोड़ u64b {
        u8 valb[8];
        u64 val;
    } u;
    u.val = 0;
    पूर्णांक i;
    क्रम (i = 0; i < 8; i++)
        अगर (Ram_आकार >= 65536 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix64K_00000, u.val);
    u.val = 0;
    क्रम (i = 0; i < 8; i++)
        अगर (Ram_आकार >= 0x80000 + 16384 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix16K_80000, u.val);
    wrmsr_smp(MSR_MTRRfix16K_A0000, 0);   // 0xA0000-0xC0000 is uncached
    पूर्णांक j;
    क्रम (j = 0; j < 8; j++) {
        u.val = 0;
        क्रम (i = 0; i < 8; i++)
            अगर (Ram_आकार >= 0xC0000 + j * 0x8000 + 4096 * (i + 1))
                u.valb[i] = MTRR_MEMTYPE_WP;
        wrmsr_smp(MSR_MTRRfix4K_C0000 + j, u.val);
    }

    // Set variable MTRRs
    पूर्णांक phys_bits = 36;
    cpuid(0x80000000u, &eax, &ebx, &ecx, &edx);
    अगर (eax >= 0x80000008) {
        /* Get physical bits from leaf 0x80000008 (if available) */
        cpuid(0x80000008u, &eax, &ebx, &ecx, &edx);
        phys_bits = eax & 0xff;
    }
    u64 phys_mask = ((1ull << phys_bits) - 1);
    क्रम (i=0; i<vcnt; i++) {
        wrmsr_smp(MTRRphysBase_MSR(i), 0);
        wrmsr_smp(MTRRphysMask_MSR(i), 0);
    }
    /* Mark 3.5-4GB as UC, anything not specified defaults to WB */
    wrmsr_smp(MTRRphysBase_MSR(0), pcimem_start | MTRR_MEMTYPE_UC);
    wrmsr_smp(MTRRphysMask_MSR(0)
              , (-((1ull<<32)-pcimem_start) & phys_mask) | 0x800);

    // Enable fixed and variable MTRRs; set default type.
    wrmsr_smp(MSR_MTRRdefType, 0xc00 | MTRR_MEMTYPE_WB);
}
