// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __GEN_DEFS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __GEN_DEFS_H


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFINE(sym, val) \
    asm ਅਸਥਿਰ("\n->" #sym " %0 " #val : : "i" (val))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLANK() \
    asm ਅਸਥਿਰ("\n->" : : )

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਥ(sym, str, mem) \
    DEFINE(sym, offsetof(ਬਣਤਰ str, mem))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COMMENT(x) \
    asm ਅਸਥਿਰ("\n->#" x)

#ਜੇ_ਅੰਤ // gen-defs.h
