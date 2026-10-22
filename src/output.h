#ifndef __OUTPUT_H
#define __OUTPUT_H

#अन्तर्भाव "config.h" // CONFIG_DEBUG_LEVEL
#अन्तर्भाव "types.h" // u32

// output.c
शून्य debug_banner(शून्य);
शून्य panic(ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
शून्य printf(ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
पूर्णाङ्क snprintf(वर्ण *str, size_t size, ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
वर्ण * znprintf(size_t size, ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
शून्य __dprintf(ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
संरचना bregs;
शून्य __debug_enter(संरचना bregs *regs, ध्रुव वर्ण *fname);
शून्य __debug_isr(ध्रुव वर्ण *fname);
शून्य __debug_stub(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __warn_invalid(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __warn_unimplemented(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __warn_internalerror(पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __warn_noalloc(पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __warn_timeout(पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __set_invalid(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __set_unimplemented(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname);
शून्य __set_code_invalid(संरचना bregs *regs, u32 linecode, ध्रुव वर्ण *fname);
शून्य __set_code_unimplemented(संरचना bregs *regs, u32 linecode
                              , ध्रुव वर्ण *fname);
शून्य hexdump(ध्रुव शून्य *d, पूर्णाङ्क len);

#define dprintf(lvl, fmt, args...) करोहि {                         \
        सचे (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } याव (0)
#define debug_enter(regs, lvl) करोहि {                     \
        सचे ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } याव (0)
#define debug_isr(lvl) करोहि {                             \
        सचे ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } याव (0)
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
