#ifndef __OUTPUT_H
#define __OUTPUT_H

#অন্তর্ভাব "config.h" // CONFIG_DEBUG_LEVEL
#অন্তর্ভাব "types.h" // u32

// output.c
শূন্য debug_banner(শূন্য);
শূন্য panic(ধ্রুব বর্ণ *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
শূন্য printf(ধ্রুব বর্ণ *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
পূর্ণাঙ্ক snprintf(বর্ণ *str, size_t size, ধ্রুব বর্ণ *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
বর্ণ * znprintf(size_t size, ধ্রুব বর্ণ *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
শূন্য __dprintf(ধ্রুব বর্ণ *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
সংরচনা bregs;
শূন্য __debug_enter(সংরচনা bregs *regs, ধ্রুব বর্ণ *fname);
শূন্য __debug_isr(ধ্রুব বর্ণ *fname);
শূন্য __debug_stub(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __warn_invalid(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __warn_unimplemented(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __warn_internalerror(পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __warn_noalloc(পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __warn_timeout(পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __set_invalid(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __set_unimplemented(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুব বর্ণ *fname);
শূন্য __set_code_invalid(সংরচনা bregs *regs, u32 linecode, ধ্রুব বর্ণ *fname);
শূন্য __set_code_unimplemented(সংরচনা bregs *regs, u32 linecode
                              , ধ্রুব বর্ণ *fname);
শূন্য hexdump(ধ্রুব শূন্য *d, পূর্ণাঙ্ক len);

#define dprintf(lvl, fmt, args...) কুরু {                         \
        যদি (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } যাবত্ (0)
#define debug_enter(regs, lvl) কুরু {                     \
        যদি ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } যাবত্ (0)
#define debug_isr(lvl) কুরু {                             \
        যদি ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } যাবত্ (0)
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
