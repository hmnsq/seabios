// Generate assembler offsets.

#ضمن "gen-defs.h" //وٿي
#ضمن "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
فراغ foo(فراغ) VISIBLE16;

فراغ foo(فراغ)
{
    COMMENT("BREGS");
    وٿي(BREGS_es, bregs, es);
    وٿي(BREGS_ds, bregs, ds);
    وٿي(BREGS_eax, bregs, eax);
    وٿي(BREGS_ebx, bregs, ebx);
    وٿي(BREGS_ecx, bregs, ecx);
    وٿي(BREGS_edx, bregs, edx);
    وٿي(BREGS_ebp, bregs, ebp);
    وٿي(BREGS_esi, bregs, esi);
    وٿي(BREGS_edi, bregs, edi);
    وٿي(BREGS_flags, bregs, flags);
    وٿي(BREGS_ڪوڊ, bregs, ڪوڊ);
}
