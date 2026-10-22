// Initialize MTRRs - mostly useful on KVM.
//
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "paravirt.h" // RamSize
#অন্তর্ভাৱ "util.h" // mtrr_setup
#অন্তর্ভাৱ "x86.h" // cpuid

#define MSR_MTRRcap                    0x000000fe
#define MSR_MTRRfix64K_00000           0x00000250
#define MSR_MTRRfix16K_80000           0x00000258
#define MSR_MTRRfix16K_A0000           0x00000259
#define MSR_MTRRfix4K_C0000            0x00000268
#define MSR_MTRRfix4K_C8000            0x00000269
#define MSR_MTRRfix4K_D0000            0x0000026a
#define MSR_MTRRfix4K_D8000            0x0000026b
#define MSR_MTRRfix4K_E0000            0x0000026c
#define MSR_MTRRfix4K_E8000            0x0000026d
#define MSR_MTRRfix4K_F0000            0x0000026e
#define MSR_MTRRfix4K_F8000            0x0000026f
#define MSR_MTRRdefType                0x000002ff

#define MTRRphysBase_MSR(reg) (0x200 + 2 * (reg))
#define MTRRphysMask_MSR(reg) (0x200 + 2 * (reg) + 1)

#define MTRR_MEMTYPE_UC 0
#define MTRR_MEMTYPE_WC 1
#define MTRR_MEMTYPE_WT 4
#define MTRR_MEMTYPE_WP 5
#define MTRR_MEMTYPE_WB 6

শূন্য mtrr_setup(শূন্য)
{
    যদি (!CONFIG_MTRR_INIT)
        ঘূৰাই;

    u32 eax, ebx, ecx, edx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    যদি (!(cpuid_features & CPUID_MTRR))
        ঘূৰাই;
    যদি (!(cpuid_features & CPUID_MSR))
        ঘূৰাই;

    dprintf(3, "init mtrr\n");

    u32 mtrr_cap = rdmsr(MSR_MTRRcap);
    পূর্ণাঙ্ক vcnt = mtrr_cap & 0xff;
    পূর্ণাঙ্ক fix = mtrr_cap & 0x100;
    যদি (!vcnt || !fix)
       ঘূৰাই;

    // Disable MTRRs
    wrmsr_smp(MSR_MTRRdefType, 0);

    // Set fixed MTRRs
    সঙ্ঘ u64b {
        u8 valb[8];
        u64 val;
    } u;
    u.val = 0;
    পূর্ণাঙ্ক i;
    বাবে (i = 0; i < 8; i++)
        যদি (RamSize >= 65536 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix64K_00000, u.val);
    u.val = 0;
    বাবে (i = 0; i < 8; i++)
        যদি (RamSize >= 0x80000 + 16384 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix16K_80000, u.val);
    wrmsr_smp(MSR_MTRRfix16K_A0000, 0);   // 0xA0000-0xC0000 is uncached
    পূর্ণাঙ্ক j;
    বাবে (j = 0; j < 8; j++) {
        u.val = 0;
        বাবে (i = 0; i < 8; i++)
            যদি (RamSize >= 0xC0000 + j * 0x8000 + 4096 * (i + 1))
                u.valb[i] = MTRR_MEMTYPE_WP;
        wrmsr_smp(MSR_MTRRfix4K_C0000 + j, u.val);
    }

    // Set variable MTRRs
    পূর্ণাঙ্ক phys_bits = 36;
    cpuid(0x80000000u, &eax, &ebx, &ecx, &edx);
    যদি (eax >= 0x80000008) {
        /* Get physical bits from leaf 0x80000008 (if available) */
        cpuid(0x80000008u, &eax, &ebx, &ecx, &edx);
        phys_bits = eax & 0xff;
    }
    u64 phys_mask = ((1ull << phys_bits) - 1);
    বাবে (i=0; i<vcnt; i++) {
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
