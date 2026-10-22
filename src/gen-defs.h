// Tool for building defintions accessible from assembler code.  This
// is based on code from the Linux Kernel.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __GEN_DEFS_H
#ವ್ಯಾಖ್ಯಾನಿಸು __GEN_DEFS_H


#ವ್ಯಾಖ್ಯಾನಿಸು DEFINE(sym, val) \
    asm ಅಸ್ಥಿರ("\n->" #sym " %0 " #val : : "i" (val))

#ವ್ಯಾಖ್ಯಾನಿಸು BLANK() \
    asm ಅಸ್ಥಿರ("\n->" : : )

#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಥಾನಾಂತರ(sym, str, mem) \
    DEFINE(sym, offsetof(ರಚನೆ str, mem))

#ವ್ಯಾಖ್ಯಾನಿಸು COMMENT(x) \
    asm ಅಸ್ಥಿರ("\n->#" x)

#ಆದರೆ_ಅಂತ್ಯ // gen-defs.h
