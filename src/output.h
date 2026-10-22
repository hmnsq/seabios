#ifndef __OUTPUT_H
#define __OUTPUT_H

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_DEBUG_LEVEL
#ಅನ್ತರ್ಭಾವ "types.h" // u32

// output.c
ಶೂನ್ಯ debug_banner(ಶೂನ್ಯ);
ಶೂನ್ಯ panic(ಧ್ರುವ ವರ್ಣ *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
ಶೂನ್ಯ printf(ಧ್ರುವ ವರ್ಣ *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
ಪೂರ್ಣಾಙ್ಕ snprintf(ವರ್ಣ *str, size_t size, ಧ್ರುವ ವರ್ಣ *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
ವರ್ಣ * znprintf(size_t size, ಧ್ರುವ ವರ್ಣ *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
ಶೂನ್ಯ __dprintf(ಧ್ರುವ ವರ್ಣ *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
ಸಂರಚನಾ bregs;
ಶೂನ್ಯ __debug_enter(ಸಂರಚನಾ bregs *regs, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __debug_isr(ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __debug_stub(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __warn_invalid(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __warn_unimplemented(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __warn_internalerror(ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __warn_noalloc(ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __warn_timeout(ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __set_invalid(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __set_unimplemented(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __set_code_invalid(ಸಂರಚನಾ bregs *regs, u32 linecode, ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ __set_code_unimplemented(ಸಂರಚನಾ bregs *regs, u32 linecode
                              , ಧ್ರುವ ವರ್ಣ *fname);
ಶೂನ್ಯ hexdump(ಧ್ರುವ ಶೂನ್ಯ *d, ಪೂರ್ಣಾಙ್ಕ len);

#define dprintf(lvl, fmt, args...) ಕುರು {                         \
        ಯದಿ (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } ಯಾವತ್ (0)
#define debug_enter(regs, lvl) ಕುರು {                     \
        ಯದಿ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } ಯಾವತ್ (0)
#define debug_isr(lvl) ಕುರು {                             \
        ಯದಿ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } ಯಾವತ್ (0)
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
