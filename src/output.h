#ifndef __OUTPUT_H
#define __OUTPUT_H

#অন্তর্ভাব "config.h" // CONFIG_DEBUG_LEVEL
#অন্তর্ভাব "types.h" // u32

// output.c
শূন্য debug_banner(শূন্য);
শূন্য panic(ধ্রুবক অক্ষর *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
শূন্য printf(ধ্রুবক অক্ষর *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
পূর্ণসংখ্যা snprintf(অক্ষর *str, আকার_t আকার_2, ধ্রুবক অক্ষর *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
অক্ষর * znprintf(আকার_t আকার_2, ধ্রুবক অক্ষর *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
শূন্য __dprintf(ধ্রুবক অক্ষর *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
কাঠামো bregs;
শূন্য __debug_enter(কাঠামো bregs *regs, ধ্রুবক অক্ষর *fname);
শূন্য __debug_isr(ধ্রুবক অক্ষর *fname);
শূন্য __debug_stub(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __warn_invalid(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __warn_unimplemented(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __warn_internalerror(পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __warn_noalloc(পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __warn_timeout(পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __set_invalid(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __set_unimplemented(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname);
শূন্য __set_code_invalid(কাঠামো bregs *regs, u32 linecode, ধ্রুবক অক্ষর *fname);
শূন্য __set_code_unimplemented(কাঠামো bregs *regs, u32 linecode
                              , ধ্রুবক অক্ষর *fname);
শূন্য hexdump(ধ্রুবক শূন্য *d, পূর্ণসংখ্যা len);

#define dprintf(lvl, fmt, args...) করো {                         \
        যদি (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } যখন (0)
#define debug_enter(regs, lvl) করো {                     \
        যদি ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } যখন (0)
#define debug_isr(lvl) করো {                             \
        যদি ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } যখন (0)
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
