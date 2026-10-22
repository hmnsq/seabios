// Generate assembler offsets.

#অন্তর্ভাব "gen-defs.h" // OFFSET
#অন্তর্ভাব "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
শূন্য foo(শূন্য) VISIBLE16;

শূন্য foo(শূন্য)
{
    COMMENT("BREGS");
    OFFSET(BREGS_es, bregs, es);
    OFFSET(BREGS_ds, bregs, ds);
    OFFSET(BREGS_eax, bregs, eax);
    OFFSET(BREGS_ebx, bregs, ebx);
    OFFSET(BREGS_ecx, bregs, ecx);
    OFFSET(BREGS_edx, bregs, edx);
    OFFSET(BREGS_ebp, bregs, ebp);
    OFFSET(BREGS_esi, bregs, esi);
    OFFSET(BREGS_edi, bregs, edi);
    OFFSET(BREGS_flags, bregs, flags);
    OFFSET(BREGS_code, bregs, code);
}
