#اگر_غیر_تعریف_شدہ __OUTPUT_H
#تعریف_کرو __OUTPUT_H

#شامل "config.h" // CONFIG_DEBUG_LEVEL
#شامل "types.h" // u32

// output.c
خالی debug_banner(خالی);
خالی panic(ثابت حرف *fmt, ...)
    __attribute__ ((format (لکھو, 1, 2))) __noreturn;
خالی لکھو(ثابت حرف *fmt, ...)
    __attribute__ ((format (لکھو, 1, 2)));
صحیح snprintf(حرف *str, سائز_t سائز, ثابت حرف *fmt, ...)
    __attribute__ ((format (لکھو, 3, 4)));
حرف * znprintf(سائز_t سائز, ثابت حرف *fmt, ...)
    __attribute__ ((format (لکھو, 2, 3)));
خالی __dprintf(ثابت حرف *fmt, ...)
    __attribute__ ((format (لکھو, 1, 2)));
ساخت bregs;
خالی __debug_enter(ساخت bregs *regs, ثابت حرف *fname);
خالی __debug_isr(ثابت حرف *fname);
خالی __debug_stub(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname);
خالی __warn_invalid(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname);
خالی __warn_unimplemented(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname);
خالی __warn_internalerror(صحیح lineno, ثابت حرف *fname);
خالی __warn_noalloc(صحیح lineno, ثابت حرف *fname);
خالی __warn_timeout(صحیح lineno, ثابت حرف *fname);
خالی __set_invalid(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname);
خالی __set_unimplemented(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname);
خالی __set_code_invalid(ساخت bregs *regs, u32 linecode, ثابت حرف *fname);
خالی __set_code_unimplemented(ساخت bregs *regs, u32 linecode
                              , ثابت حرف *fname);
خالی hexdump(ثابت خالی *d, صحیح len);

#تعریف_کرو dprintf(lvl, fmt, دلائل...) کرو {                         \
        اگر (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } جب (0)
#تعریف_کرو debug_enter(regs, lvl) کرو {                     \
        اگر ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } جب (0)
#تعریف_کرو debug_isr(lvl) کرو {                             \
        اگر ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } جب (0)
#تعریف_کرو debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#تعریف_کرو warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#تعریف_کرو warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#تعریف_کرو warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#تعریف_کرو warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#تعریف_کرو warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#تعریف_کرو سیٹ_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#تعریف_کرو سیٹ_رمز_invalid(regs, رمز)                                    \
    __set_code_invalid((regs), (رمز) | (__LINE__ << 8), __func__)
#تعریف_کرو سیٹ_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#تعریف_کرو سیٹ_رمز_unimplemented(regs, رمز)                              \
    __set_code_unimplemented((regs), (رمز) | (__LINE__ << 8), __func__)

#اگر_ختم // output.h
