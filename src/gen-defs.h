// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm અસ્થિર("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm અસ્થિર("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(સંરચના str, mem))

#define COMMENT(x) \
    asm અસ્થિર("\n->#" x)

#endif // gen-defs.h
