#ifndef __OUTPUT_H
#define __OUTPUT_H

#અન્તર્ભાવ "config.h" // CONFIG_DEBUG_LEVEL
#અન્તર્ભાવ "types.h" // u32

// output.c
શૂન્ય debug_banner(શૂન્ય);
શૂન્ય panic(અચલ અક્ષર *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
શૂન્ય printf(અચલ અક્ષર *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
પૂર્ણાંક snprintf(અક્ષર *str, size_t size, અચલ અક્ષર *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
અક્ષર * znprintf(size_t size, અચલ અક્ષર *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
શૂન્ય __dprintf(અચલ અક્ષર *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
સંરચના bregs;
શૂન્ય __debug_enter(સંરચના bregs *regs, અચલ અક્ષર *fname);
શૂન્ય __debug_isr(અચલ અક્ષર *fname);
શૂન્ય __debug_stub(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __warn_invalid(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __warn_unimplemented(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __warn_internalerror(પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __warn_noalloc(પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __warn_timeout(પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __set_invalid(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __set_unimplemented(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname);
શૂન્ય __set_code_invalid(સંરચના bregs *regs, u32 linecode, અચલ અક્ષર *fname);
શૂન્ય __set_code_unimplemented(સંરચના bregs *regs, u32 linecode
                              , અચલ અક્ષર *fname);
શૂન્ય hexdump(અચલ શૂન્ય *d, પૂર્ણાંક len);

#define dprintf(lvl, fmt, args...) કરો {                         \
        જો (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } જ્યારે (0)
#define debug_enter(regs, lvl) કરો {                     \
        જો ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } જ્યારે (0)
#define debug_isr(lvl) કરો {                             \
        જો ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } જ્યારે (0)
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
