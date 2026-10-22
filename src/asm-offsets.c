// Generate assembler offsets.

#અન્તર્ભાવ "gen-defs.h" // OFFSET
#અન્તર્ભાવ "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
શૂન્ય foo(શૂન્ય) VISIBLE16;

શૂન્ય foo(શૂન્ય)
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
