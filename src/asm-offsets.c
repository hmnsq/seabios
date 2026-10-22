// Generate assembler offsets.

#అన్తర్భావ "gen-defs.h" //స్థానభ్రంశం
#అన్తర్భావ "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
శూన్య foo(శూన్య) VISIBLE16;

శూన్య foo(శూన్య)
{
    COMMENT("BREGS");
    స్థానభ్రంశం(BREGS_es, bregs, es);
    స్థానభ్రంశం(BREGS_ds, bregs, ds);
    స్థానభ్రంశం(BREGS_eax, bregs, eax);
    స్థానభ్రంశం(BREGS_ebx, bregs, ebx);
    స్థానభ్రంశం(BREGS_ecx, bregs, ecx);
    స్థానభ్రంశం(BREGS_edx, bregs, edx);
    స్థానభ్రంశం(BREGS_ebp, bregs, ebp);
    స్థానభ్రంశం(BREGS_esi, bregs, esi);
    స్థానభ్రంశం(BREGS_edi, bregs, edi);
    స్థానభ్రంశం(BREGS_flags, bregs, flags);
    స్థానభ్రంశం(BREGS_సంకేతం, bregs, సంకేతం_2);
}
