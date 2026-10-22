// Generate assembler offsets.

#അന്തര്ഭാവ "gen-defs.h" // OFFSET
#അന്തര്ഭാവ "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
ശൂന്യം foo(ശൂന്യം) VISIBLE16;

ശൂന്യം foo(ശൂന്യം)
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
