// Generate assembler offsets.

#ਅਨ੍ਤਰ੍ਭਾਵ "gen-defs.h" //ਵਿੱਥ
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
ਖਾਲੀ foo(ਖਾਲੀ) VISIBLE16;

ਖਾਲੀ foo(ਖਾਲੀ)
{
    COMMENT("BREGS");
    ਵਿੱਥ(BREGS_es, bregs, es);
    ਵਿੱਥ(BREGS_ds, bregs, ds);
    ਵਿੱਥ(BREGS_eax, bregs, eax);
    ਵਿੱਥ(BREGS_ebx, bregs, ebx);
    ਵਿੱਥ(BREGS_ecx, bregs, ecx);
    ਵਿੱਥ(BREGS_edx, bregs, edx);
    ਵਿੱਥ(BREGS_ebp, bregs, ebp);
    ਵਿੱਥ(BREGS_esi, bregs, esi);
    ਵਿੱਥ(BREGS_edi, bregs, edi);
    ਵਿੱਥ(BREGS_flags, bregs, flags);
    ਵਿੱਥ(BREGS_ਕੋਡ, bregs, ਕੋਡ);
}
