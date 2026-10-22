#ifndef __OUTPUT_H
#define __OUTPUT_H

#شامل "config.h" // CONFIG_DEBUG_LEVEL
#شامل "types.h" // u32

// output.c
تهی debug_banner(تهی);
تهی panic(ثابت نویسه *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
تهی printf(ثابت نویسه *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
صحیح snprintf(نویسه *str, size_t size, ثابت نویسه *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
نویسه * znprintf(size_t size, ثابت نویسه *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
تهی __dprintf(ثابت نویسه *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
ساختار bregs;
تهی __debug_enter(ساختار bregs *regs, ثابت نویسه *fname);
تهی __debug_isr(ثابت نویسه *fname);
تهی __debug_stub(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname);
تهی __warn_invalid(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname);
تهی __warn_unimplemented(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname);
تهی __warn_internalerror(صحیح lineno, ثابت نویسه *fname);
تهی __warn_noalloc(صحیح lineno, ثابت نویسه *fname);
تهی __warn_timeout(صحیح lineno, ثابت نویسه *fname);
تهی __set_invalid(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname);
تهی __set_unimplemented(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname);
تهی __set_code_invalid(ساختار bregs *regs, u32 linecode, ثابت نویسه *fname);
تهی __set_code_unimplemented(ساختار bregs *regs, u32 linecode
                              , ثابت نویسه *fname);
تهی hexdump(ثابت تهی *d, صحیح len);

#define dprintf(lvl, fmt, args...) انجامبده {                         \
        اگر (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } وقتی (0)
#define debug_enter(regs, lvl) انجامبده {                     \
        اگر ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } وقتی (0)
#define debug_isr(lvl) انجامبده {                             \
        اگر ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } وقتی (0)
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
