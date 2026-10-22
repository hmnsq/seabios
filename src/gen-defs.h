// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm चञ्चल("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm चञ्चल("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(संरचना str, mem))

#define COMMENT(x) \
    asm चञ्चल("\n->#" x)

#endif // gen-defs.h
