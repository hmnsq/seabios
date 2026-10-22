// Generate assembler offsets.

#ضمن "gen-defs.h" //جابه_جایی
#ضمن "bregs.h" // struct bregs

/* workaround for a warning with -Wmissing-prototypes */
فراغ foo(فراغ) VISIBLE16;

فراغ foo(فراغ)
{
    COMMENT("BREGS");
    جابه_جایی(BREGS_es, bregs, es);
    جابه_جایی(BREGS_ds, bregs, ds);
    جابه_جایی(BREGS_eax, bregs, eax);
    جابه_جایی(BREGS_ebx, bregs, ebx);
    جابه_جایی(BREGS_ecx, bregs, ecx);
    جابه_جایی(BREGS_edx, bregs, edx);
    جابه_جایی(BREGS_ebp, bregs, ebp);
    جابه_جایی(BREGS_esi, bregs, esi);
    جابه_جایی(BREGS_edi, bregs, edi);
    جابه_جایی(BREGS_flags, bregs, flags);
    جابه_جایی(BREGS_کد, bregs, کد);
}
