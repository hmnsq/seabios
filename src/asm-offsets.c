// Generate assembler offsets.

#شامل "gen-defs.h" //انحراف
#شامل "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
خالی foo(خالی) VISIBLE16;

خالی foo(خالی)
{
    COMMENT("BREGS");
    انحراف(BREGS_es, bregs, es);
    انحراف(BREGS_ds, bregs, ds);
    انحراف(BREGS_eax, bregs, eax);
    انحراف(BREGS_ebx, bregs, ebx);
    انحراف(BREGS_ecx, bregs, ecx);
    انحراف(BREGS_edx, bregs, edx);
    انحراف(BREGS_ebp, bregs, ebp);
    انحراف(BREGS_esi, bregs, esi);
    انحراف(BREGS_edi, bregs, edi);
    انحراف(BREGS_flags, bregs, flags);
    انحراف(BREGS_رمز, bregs, رمز);
}
