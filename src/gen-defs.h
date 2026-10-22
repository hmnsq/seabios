// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#جے_بنا_تعریف __GEN_DEFS_H
#تعریف_کرو __GEN_DEFS_H


#تعریف_کرو DEFINE(sym, val) \
    asm متطاير("\n->" #sym " %0 " #val : : "i" (val))

#تعریف_کرو BLANK() \
    asm متطاير("\n->" : : )

#تعریف_کرو وتھ(sym, str, mem) \
    DEFINE(sym, offsetof(بنية str, mem))

#تعریف_کرو COMMENT(x) \
    asm متطاير("\n->#" x)

#جے_انت // gen-defs.h
