// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm অস্থির("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm অস্থির("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(কাঠামো str, mem))

#define COMMENT(x) \
    asm অস্থির("\n->#" x)

#endif // gen-defs.h
