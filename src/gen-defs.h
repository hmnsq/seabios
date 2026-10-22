// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#اگر_غیر_تعریف_شدہ __GEN_DEFS_H
#تعریف_کرو __GEN_DEFS_H


#تعریف_کرو DEFINE(sym, val) \
    asm متلون("\n->" #sym " %0 " #val : : "i" (val))

#تعریف_کرو BLANK() \
    asm متلون("\n->" : : )

#تعریف_کرو انحراف(sym, str, mem) \
    DEFINE(sym, offsetof(ساخت str, mem))

#تعریف_کرو COMMENT(x) \
    asm متلون("\n->#" x)

#اگر_ختم // gen-defs.h
