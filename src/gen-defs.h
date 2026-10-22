// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm متطاير("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm متطاير("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(بنية str, mem))

#define COMMENT(x) \
    asm متطاير("\n->#" x)

#endif // gen-defs.h
