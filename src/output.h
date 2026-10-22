#ifndef __OUTPUT_H
#define __OUTPUT_H

#ضمن "config.h" // CONFIG_DEBUG_LEVEL
#ضمن "types.h" // u32

// output.c
فراغ debug_banner(فراغ);
فراغ panic(ثابت حرف *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
فراغ printf(ثابت حرف *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
صحيح snprintf(حرف *str, size_t size, ثابت حرف *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
حرف * znprintf(size_t size, ثابت حرف *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
فراغ __dprintf(ثابت حرف *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
بنية bregs;
فراغ __debug_enter(بنية bregs *regs, ثابت حرف *fname);
فراغ __debug_isr(ثابت حرف *fname);
فراغ __debug_stub(بنية bregs *regs, صحيح lineno, ثابت حرف *fname);
فراغ __warn_invalid(بنية bregs *regs, صحيح lineno, ثابت حرف *fname);
فراغ __warn_unimplemented(بنية bregs *regs, صحيح lineno, ثابت حرف *fname);
فراغ __warn_internalerror(صحيح lineno, ثابت حرف *fname);
فراغ __warn_noalloc(صحيح lineno, ثابت حرف *fname);
فراغ __warn_timeout(صحيح lineno, ثابت حرف *fname);
فراغ __set_invalid(بنية bregs *regs, صحيح lineno, ثابت حرف *fname);
فراغ __set_unimplemented(بنية bregs *regs, صحيح lineno, ثابت حرف *fname);
فراغ __set_code_invalid(بنية bregs *regs, u32 linecode, ثابت حرف *fname);
فراغ __set_code_unimplemented(بنية bregs *regs, u32 linecode
                              , ثابت حرف *fname);
فراغ hexdump(ثابت فراغ *d, صحيح len);

#define dprintf(lvl, fmt, args...) 𐤐𐤏𐤋 {                         \
        𐤀𐤌 (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } طالما (0)
#define debug_enter(regs, lvl) 𐤐𐤏𐤋 {                     \
        𐤀𐤌 ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } طالما (0)
#define debug_isr(lvl) 𐤐𐤏𐤋 {                             \
        𐤀𐤌 ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } طالما (0)
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
