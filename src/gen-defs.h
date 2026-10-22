// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm അസ്ഥിര("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm അസ്ഥിര("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(ഘടന str, mem))

#define COMMENT(x) \
    asm അസ്ഥിര("\n->#" x)

#endif // gen-defs.h
