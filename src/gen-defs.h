// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm நிலையற்ற("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm நிலையற்ற("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(கட்டமைப்பு str, mem))

#define COMMENT(x) \
    asm நிலையற்ற("\n->#" x)

#endif // gen-defs.h
