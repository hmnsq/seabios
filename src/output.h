#ifndef __OUTPUT_H
#define __OUTPUT_H

#include "config.h" // CONFIG_DEBUG_LEVEL
#include "types.h" // u32

// output.c
void debug_banner(void);
void panic(const var_na *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
void printf(const var_na *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
පූර්ණ_සංඛ්‍යා snprintf(var_na *str, size_t size, const var_na *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
var_na * znprintf(size_t size, const var_na *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
void __dprintf(const var_na *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
sa_nracanaa bregs;
void __debug_enter(sa_nracanaa bregs *regs, const var_na *fname);
void __debug_isr(const var_na *fname);
void __debug_stub(sa_nracanaa bregs *regs, පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __warn_invalid(sa_nracanaa bregs *regs, පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __warn_unimplemented(sa_nracanaa bregs *regs, පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __warn_internalerror(පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __warn_noalloc(පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __warn_timeout(පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __set_invalid(sa_nracanaa bregs *regs, පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __set_unimplemented(sa_nracanaa bregs *regs, පූර්ණ_සංඛ්‍යා lineno, const var_na *fname);
void __set_code_invalid(sa_nracanaa bregs *regs, u32 linecode, const var_na *fname);
void __set_code_unimplemented(sa_nracanaa bregs *regs, u32 linecode
                              , const var_na *fname);
void hexdump(const void *d, පූර්ණ_සංඛ්‍යා len);

#define dprintf(lvl, fmt, args...) do {                         \
        නම් (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } අතරතුර (0)
#define debug_enter(regs, lvl) do {                     \
        නම් ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } අතරතුර (0)
#define debug_isr(lvl) do {                             \
        නම් ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } අතරතුර (0)
#define debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#define warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#define warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#define warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#define warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#define warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#define set_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#define set_code_invalid(regs, code)                                    \
    __set_code_invalid((regs), (code) | (__LINE__ << 8), __func__)
#define set_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#define set_code_unimplemented(regs, code)                              \
    __set_code_unimplemented((regs), (code) | (__LINE__ << 8), __func__)

#endif // output.h
