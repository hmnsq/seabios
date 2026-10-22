#ifndef __OUTPUT_H
#define __OUTPUT_H

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_DEBUG_LEVEL
#ଅନ୍ତର୍ଭାଵ "types.h" // u32

// output.c
ଶୂନ୍ୟ debug_banner(ଶୂନ୍ୟ);
ଶୂନ୍ୟ panic(ଅଚଳ ଅକ୍ଷର *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
ଶୂନ୍ୟ printf(ଅଚଳ ଅକ୍ଷର *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
ପୂର୍ଣ୍ଣସଂଖ୍ୟା snprintf(ଅକ୍ଷର *str, size_t size, ଅଚଳ ଅକ୍ଷର *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
ଅକ୍ଷର * znprintf(size_t size, ଅଚଳ ଅକ୍ଷର *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
ଶୂନ୍ୟ __dprintf(ଅଚଳ ଅକ୍ଷର *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
ସଂରଚନା bregs;
ଶୂନ୍ୟ __debug_enter(ସଂରଚନା bregs *regs, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __debug_isr(ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __debug_stub(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __warn_invalid(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __warn_unimplemented(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __warn_internalerror(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __warn_noalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __warn_timeout(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __set_invalid(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __set_unimplemented(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __set_code_invalid(ସଂରଚନା bregs *regs, u32 linecode, ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ __set_code_unimplemented(ସଂରଚନା bregs *regs, u32 linecode
                              , ଅଚଳ ଅକ୍ଷର *fname);
ଶୂନ୍ୟ hexdump(ଅଚଳ ଶୂନ୍ୟ *d, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len);

#define dprintf(lvl, fmt, args...) କର {                         \
        ଯଦି (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } ଯେତେବେଳେ (0)
#define debug_enter(regs, lvl) କର {                     \
        ଯଦି ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } ଯେତେବେଳେ (0)
#define debug_isr(lvl) କର {                             \
        ଯଦି ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } ଯେତେବେଳେ (0)
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
