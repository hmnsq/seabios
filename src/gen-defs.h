// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm ಚಞ್ಚಲ("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm ಚಞ್ಚಲ("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(ಸಂರಚನಾ str, mem))

#define COMMENT(x) \
    asm ಚಞ್ಚಲ("\n->#" x)

#endif // gen-defs.h
