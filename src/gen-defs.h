// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#यदि_अपरिभाषितम् __GEN_DEFS_H
#परिभाषय __GEN_DEFS_H


#परिभाषय DEFINE(sym, val) \
    asm चञ्चल("\n->" #sym " %0 " #val : : "i" (val))

#परिभाषय BLANK() \
    asm चञ्चल("\n->" : : )

#परिभाषय विस्थापनम्(sym, str, mem) \
    DEFINE(sym, offsetof(संरचना str, mem))

#परिभाषय COMMENT(x) \
    asm चञ्चल("\n->#" x)

#यदि_अन्तः // gen-defs.h
