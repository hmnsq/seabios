#यदि_अपरिभाषितम् __OUTPUT_H
#परिभाषय __OUTPUT_H

#अन्तर्भाव "config.h" // CONFIG_DEBUG_LEVEL
#अन्तर्भाव "types.h" // u32

// output.c
शून्य debug_banner(शून्य);
शून्य panic(ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (प्रारूप_लिख, 1, 2))) __noreturn;
शून्य प्रारूप_लिख(ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (प्रारूप_लिख, 1, 2)));
पूर्णाङ्क snprintf(वर्ण *str, आकार_t आकार, ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (प्रारूप_लिख, 3, 4)));
वर्ण * znprintf(आकार_t आकार, ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (प्रारूप_लिख, 2, 3)));
शून्य __dprintf(ध्रुव वर्ण *fmt, ...)
    __attribute__ ((format (प्रारूप_लिख, 1, 2)));
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

#परिभाषय dprintf(lvl, fmt, तर्काः...) कुरु {                         \
        यदि (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } यावत् (0)
#परिभाषय debug_enter(regs, lvl) कुरु {                     \
        यदि ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } यावत् (0)
#परिभाषय debug_isr(lvl) कुरु {                             \
        यदि ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } यावत् (0)
#परिभाषय debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#परिभाषय warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#परिभाषय warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#परिभाषय warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#परिभाषय warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#परिभाषय warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#परिभाषय समुच्चयः_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#परिभाषय समुच्चयः_संहिता_invalid(regs, संहिता)                                    \
    __set_code_invalid((regs), (संहिता) | (__LINE__ << 8), __func__)
#परिभाषय समुच्चयः_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#परिभाषय समुच्चयः_संहिता_unimplemented(regs, संहिता)                              \
    __set_code_unimplemented((regs), (संहिता) | (__LINE__ << 8), __func__)

#यदि_अन्तः // output.h
