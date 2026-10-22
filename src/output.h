#اگر_تعریف_نشده __OUTPUT_H
#تعریف_کن __OUTPUT_H

#ضمن "config.h" // CONFIG_DEBUG_LEVEL
#ضمن "types.h" // u32

// output.c
فراغ debug_banner(فراغ);
فراغ panic(ثابت حرف *fmt, ...)
    __attribute__ ((format (چاپ_کن, 1, 2))) __noreturn;
فراغ چاپ_کن(ثابت حرف *fmt, ...)
    __attribute__ ((format (چاپ_کن, 1, 2)));
صحيح snprintf(حرف *str, اندازه_t اندازه, ثابت حرف *fmt, ...)
    __attribute__ ((format (چاپ_کن, 3, 4)));
حرف * znprintf(اندازه_t اندازه, ثابت حرف *fmt, ...)
    __attribute__ ((format (چاپ_کن, 2, 3)));
فراغ __dprintf(ثابت حرف *fmt, ...)
    __attribute__ ((format (چاپ_کن, 1, 2)));
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

#تعریف_کن dprintf(lvl, fmt, آرگومانها...) افعل {                         \
        إذا (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } طالما (0)
#تعریف_کن debug_enter(regs, lvl) افعل {                     \
        إذا ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } طالما (0)
#تعریف_کن debug_isr(lvl) افعل {                             \
        إذا ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } طالما (0)
#تعریف_کن debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#تعریف_کن warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#تعریف_کن warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#تعریف_کن warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#تعریف_کن warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#تعریف_کن warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#تعریف_کن مجموعه_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#تعریف_کن مجموعه_کد_invalid(regs, کد)                                    \
    __set_code_invalid((regs), (کد) | (__LINE__ << 8), __func__)
#تعریف_کن مجموعه_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#تعریف_کن مجموعه_کد_unimplemented(regs, کد)                              \
    __set_code_unimplemented((regs), (کد) | (__LINE__ << 8), __func__)

#پایان_اگر // output.h
