// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#నిర్వచితం_కాకపోతే __GEN_DEFS_H
#నిర్వచించు __GEN_DEFS_H


#నిర్వచించు DEFINE(sym, val) \
    asm అస్థిర("\n->" #sym " %0 " #val : : "i" (val))

#నిర్వచించు BLANK() \
    asm అస్థిర("\n->" : : )

#నిర్వచించు స్థానభ్రంశం(sym, str, mem) \
    DEFINE(sym, offsetof(నిర్మాణం str, mem))

#నిర్వచించు COMMENT(x) \
    asm అస్థిర("\n->#" x)

#అయితే_ముగింపు // gen-defs.h
