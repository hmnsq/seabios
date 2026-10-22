// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#جيڪڏهن_وصف_نه_ٿيل __GEN_DEFS_H
#وصف_ڪريو __GEN_DEFS_H


#وصف_ڪريو DEFINE(sym, val) \
    asm متطاير("\n->" #sym " %0 " #val : : "i" (val))

#وصف_ڪريو BLANK() \
    asm متطاير("\n->" : : )

#وصف_ڪريو وٿي(sym, str, mem) \
    DEFINE(sym, offsetof(بنية str, mem))

#وصف_ڪريو COMMENT(x) \
    asm متطاير("\n->#" x)

#جيڪڏهن_پورو // gen-defs.h
