// Initialize MTRRs - mostly useful on KVM.
//
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // RamSize
#ضمن "util.h" // mtrr_setup
#ضمن "x86.h" // cpuid

#تعریف_کړه MSR_MTRRcap                    0x000000fe
#تعریف_کړه MSR_MTRRfix64K_00000           0x00000250
#تعریف_کړه MSR_MTRRfix16K_80000           0x00000258
#تعریف_کړه MSR_MTRRfix16K_A0000           0x00000259
#تعریف_کړه MSR_MTRRfix4K_C0000            0x00000268
#تعریف_کړه MSR_MTRRfix4K_C8000            0x00000269
#تعریف_کړه MSR_MTRRfix4K_D0000            0x0000026a
#تعریف_کړه MSR_MTRRfix4K_D8000            0x0000026b
#تعریف_کړه MSR_MTRRfix4K_E0000            0x0000026c
#تعریف_کړه MSR_MTRRfix4K_E8000            0x0000026d
#تعریف_کړه MSR_MTRRfix4K_F0000            0x0000026e
#تعریف_کړه MSR_MTRRfix4K_F8000            0x0000026f
#تعریف_کړه MSR_MTRRdefType                0x000002ff

#تعریف_کړه MTRRphys_بنسټ_MSR(reg) (0x200 + 2 * (reg))
#تعریف_کړه MTRRphys_ماسک_MSR(reg) (0x200 + 2 * (reg) + 1)

#تعریف_کړه MTRR_MEMTYPE_UC 0
#تعریف_کړه MTRR_MEMTYPE_WC 1
#تعریف_کړه MTRR_MEMTYPE_WT 4
#تعریف_کړه MTRR_MEMTYPE_WP 5
#تعریف_کړه MTRR_MEMTYPE_WB 6

فراغ mtrr_تیاری(فراغ)
{
    إذا (!CONFIG_MTRR_INIT)
        أرجع;

    u32 eax, ebx, ecx, edx, cpuid_features;
    cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    إذا (!(cpuid_features & CPUID_MTRR))
        أرجع;
    إذا (!(cpuid_features & CPUID_MSR))
        أرجع;

    dprintf(3, "init mtrr\n");

    u32 mtrr_cap = rdmsr(MSR_MTRRcap);
    صحيح vcnt = mtrr_cap & 0xff;
    صحيح fix = mtrr_cap & 0x100;
    إذا (!vcnt || !fix)
       أرجع;

    // Disable MTRRs
    wrmsr_smp(MSR_MTRRdefType, 0);

    // Set fixed MTRRs
    اتحاد u64b {
        u8 valb[8];
        u64 val;
    } u;
    u.val = 0;
    صحيح i;
    لكل (i = 0; i < 8; i++)
        إذا (Ram_اندازه >= 65536 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix64K_00000, u.val);
    u.val = 0;
    لكل (i = 0; i < 8; i++)
        إذا (Ram_اندازه >= 0x80000 + 16384 * (i + 1))
            u.valb[i] = MTRR_MEMTYPE_WB;
    wrmsr_smp(MSR_MTRRfix16K_80000, u.val);
    wrmsr_smp(MSR_MTRRfix16K_A0000, 0);   // 0xA0000-0xC0000 is uncached
    صحيح j;
    لكل (j = 0; j < 8; j++) {
        u.val = 0;
        لكل (i = 0; i < 8; i++)
            إذا (Ram_اندازه >= 0xC0000 + j * 0x8000 + 4096 * (i + 1))
                u.valb[i] = MTRR_MEMTYPE_WP;
        wrmsr_smp(MSR_MTRRfix4K_C0000 + j, u.val);
    }

    // Set variable MTRRs
    صحيح phys_بټونه = 36;
    cpuid(0x80000000u, &eax, &ebx, &ecx, &edx);
    إذا (eax >= 0x80000008) {
        /* Get physical bits from leaf 0x80000008 (if available) */
        cpuid(0x80000008u, &eax, &ebx, &ecx, &edx);
        phys_بټونه = eax & 0xff;
    }
    u64 phys_ماسک = ((1ull << phys_بټونه) - 1);
    لكل (i=0; i<vcnt; i++) {
        wrmsr_smp(MTRRphys_بنسټ_MSR(i), 0);
        wrmsr_smp(MTRRphys_ماسک_MSR(i), 0);
    }
    /* Mark 3.5-4GB as UC, anything not specified defaults to WB */
    wrmsr_smp(MTRRphys_بنسټ_MSR(0), pcimem_پیل | MTRR_MEMTYPE_UC);
    wrmsr_smp(MTRRphys_ماسک_MSR(0)
              , (-((1ull<<32)-pcimem_پیل) & phys_ماسک) | 0x800);

    // Enable fixed and variable MTRRs; set default type.
    wrmsr_smp(MSR_MTRRdefType, 0xc00 | MTRR_MEMTYPE_WB);
}
