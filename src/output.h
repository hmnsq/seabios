#جيڪڏهن_وصف_نه_ٿيل __OUTPUT_H
#وصف_ڪريو __OUTPUT_H

#ضمن "config.h" // CONFIG_DEBUG_LEVEL
#ضمن "types.h" // u32

// output.c
فراغ debug_banner(فراغ);
فراغ panic(ثابت حرف *fmt, ...)
    __attribute__ ((format (لکو, 1, 2))) __noreturn;
فراغ لکو(ثابت حرف *fmt, ...)
    __attribute__ ((format (لکو, 1, 2)));
صحيح snprintf(حرف *str, سائيز_t سائيز, ثابت حرف *fmt, ...)
    __attribute__ ((format (لکو, 3, 4)));
حرف * znprintf(سائيز_t سائيز, ثابت حرف *fmt, ...)
    __attribute__ ((format (لکو, 2, 3)));
فراغ __dprintf(ثابت حرف *fmt, ...)
    __attribute__ ((format (لکو, 1, 2)));
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

#وصف_ڪريو dprintf(lvl, fmt, دليلون...) افعل {                         \
        إذا (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } طالما (0)
#وصف_ڪريو debug_enter(regs, lvl) افعل {                     \
        إذا ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } طالما (0)
#وصف_ڪريو debug_isr(lvl) افعل {                             \
        إذا ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } طالما (0)
#وصف_ڪريو debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#وصف_ڪريو warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#وصف_ڪريو warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#وصف_ڪريو warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#وصف_ڪريو warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#وصف_ڪريو warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#وصف_ڪريو سيٽ_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#وصف_ڪريو سيٽ_ڪوڊ_invalid(regs, ڪوڊ)                                    \
    __set_code_invalid((regs), (ڪوڊ) | (__LINE__ << 8), __func__)
#وصف_ڪريو سيٽ_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#وصف_ڪريو سيٽ_ڪوڊ_unimplemented(regs, ڪوڊ)                              \
    __set_code_unimplemented((regs), (ڪوڊ) | (__LINE__ << 8), __func__)

#جيڪڏهن_پورو // output.h
