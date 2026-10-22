// Generate assembler offsets.

#ಅನ್ತರ್ಭಾವ "gen-defs.h" //ಸ್ಥಾನಾಂತರ
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
ಶೂನ್ಯ foo(ಶೂನ್ಯ) VISIBLE16;

ಶೂನ್ಯ foo(ಶೂನ್ಯ)
{
    COMMENT("BREGS");
    ಸ್ಥಾನಾಂತರ(BREGS_es, bregs, es);
    ಸ್ಥಾನಾಂತರ(BREGS_ds, bregs, ds);
    ಸ್ಥಾನಾಂತರ(BREGS_eax, bregs, eax);
    ಸ್ಥಾನಾಂತರ(BREGS_ebx, bregs, ebx);
    ಸ್ಥಾನಾಂತರ(BREGS_ecx, bregs, ecx);
    ಸ್ಥಾನಾಂತರ(BREGS_edx, bregs, edx);
    ಸ್ಥಾನಾಂತರ(BREGS_ebp, bregs, ebp);
    ಸ್ಥಾನಾಂತರ(BREGS_esi, bregs, esi);
    ಸ್ಥಾನಾಂತರ(BREGS_edi, bregs, edi);
    ಸ್ಥಾನಾಂತರ(BREGS_flags, bregs, flags);
    ಸ್ಥಾನಾಂತರ(BREGS_ಸಂಕೇತ, bregs, ಸಂಕೇತ_2);
}
