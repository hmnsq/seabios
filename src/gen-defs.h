// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ifndef __GEN_DEFS_H
#define __GEN_DEFS_H


#define DEFINE(sym, val) \
    asm ca_ncala("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() \
    asm ca_ncala("\n->" : : )

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(sa_nracanaa str, mem))

#define COMMENT(x) \
    asm ca_ncala("\n->#" x)

#endif // gen-defs.h
