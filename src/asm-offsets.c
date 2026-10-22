// Generate assembler offsets.

#ضمن "gen-defs.h" //دننه_واټن
#ضمن "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
فراغ foo(فراغ) VISIBLE16;

فراغ foo(فراغ)
{
    COMMENT("BREGS");
    دننه_واټن(BREGS_es, bregs, es);
    دننه_واټن(BREGS_ds, bregs, ds);
    دننه_واټن(BREGS_eax, bregs, eax);
    دننه_واټن(BREGS_ebx, bregs, ebx);
    دننه_واټن(BREGS_ecx, bregs, ecx);
    دننه_واټن(BREGS_edx, bregs, edx);
    دننه_واټن(BREGS_ebp, bregs, ebp);
    دننه_واټن(BREGS_esi, bregs, esi);
    دننه_واټن(BREGS_edi, bregs, edi);
    دننه_واټن(BREGS_flags, bregs, flags);
    دننه_واټن(BREGS_کوډ, bregs, کوډ);
}
