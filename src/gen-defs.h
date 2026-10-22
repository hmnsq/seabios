// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#که_نه_تعریف_شوی __GEN_DEFS_H
#تعریف_کړه __GEN_DEFS_H


#تعریف_کړه DEFINE(sym, val) \
    asm متطاير("\n->" #sym " %0 " #val : : "i" (val))

#تعریف_کړه BLANK() \
    asm متطاير("\n->" : : )

#تعریف_کړه دننه_واټن(sym, str, mem) \
    DEFINE(sym, offsetof(بنية str, mem))

#تعریف_کړه COMMENT(x) \
    asm متطاير("\n->#" x)

#د_که_پای // gen-defs.h
