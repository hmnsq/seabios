// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#اگر_تعریف_نشده __GEN_DEFS_H
#تعریف_کن __GEN_DEFS_H


#تعریف_کن DEFINE(sym, val) \
    asm متطاير("\n->" #sym " %0 " #val : : "i" (val))

#تعریف_کن BLANK() \
    asm متطاير("\n->" : : )

#تعریف_کن جابه_جایی(sym, str, mem) \
    DEFINE(sym, offsetof(بنية str, mem))

#تعریف_کن COMMENT(x) \
    asm متطاير("\n->#" x)

#پایان_اگر // gen-defs.h
