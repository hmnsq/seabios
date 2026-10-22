// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm فرار("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm فرار("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(ساختار str, mem))

#define COMMENT(x) \
    asm فرار("\n->#" x)

#endif // gen-defs.h
