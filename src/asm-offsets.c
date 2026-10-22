// Generate assembler offsets.

#ضمن "gen-defs.h" //وتھ
#ضمن "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
فراغ foo(فراغ) VISIBLE16;

فراغ foo(فراغ)
{
    COMMENT("BREGS");
    وتھ(BREGS_es, bregs, es);
    وتھ(BREGS_ds, bregs, ds);
    وتھ(BREGS_eax, bregs, eax);
    وتھ(BREGS_ebx, bregs, ebx);
    وتھ(BREGS_ecx, bregs, ecx);
    وتھ(BREGS_edx, bregs, edx);
    وتھ(BREGS_ebp, bregs, ebp);
    وتھ(BREGS_esi, bregs, esi);
    وتھ(BREGS_edi, bregs, edi);
    وتھ(BREGS_flags, bregs, flags);
    وتھ(BREGS_کوڈ, bregs, کوڈ);
}
