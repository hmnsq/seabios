// Generate assembler offsets.

#अन्तर्भाव "gen-defs.h" //विस्थापनम्
#अन्तर्भाव "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
शून्य foo(शून्य) VISIBLE16;

शून्य foo(शून्य)
{
    COMMENT("BREGS");
    विस्थापनम्(BREGS_es, bregs, es);
    विस्थापनम्(BREGS_ds, bregs, ds);
    विस्थापनम्(BREGS_eax, bregs, eax);
    विस्थापनम्(BREGS_ebx, bregs, ebx);
    विस्थापनम्(BREGS_ecx, bregs, ecx);
    विस्थापनम्(BREGS_edx, bregs, edx);
    विस्थापनम्(BREGS_ebp, bregs, ebp);
    विस्थापनम्(BREGS_esi, bregs, esi);
    विस्थापनम्(BREGS_edi, bregs, edi);
    विस्थापनम्(BREGS_flags, bregs, flags);
    विस्थापनम्(BREGS_संहिता, bregs, संहिता);
}
