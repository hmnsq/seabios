#ifndef __OUTPUT_H
#define __OUTPUT_H

#അന്തര്ഭാവ "config.h" // CONFIG_DEBUG_LEVEL
#അന്തര്ഭാവ "types.h" // u32

// output.c
ശൂന്യം debug_banner(ശൂന്യം);
ശൂന്യം panic(സ്ഥിരം അക്ഷരം *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
ശൂന്യം printf(സ്ഥിരം അക്ഷരം *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
പൂർണ്ണസംഖ്യ snprintf(അക്ഷരം *str, size_t size, സ്ഥിരം അക്ഷരം *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
അക്ഷരം * znprintf(size_t size, സ്ഥിരം അക്ഷരം *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
ശൂന്യം __dprintf(സ്ഥിരം അക്ഷരം *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
ഘടന bregs;
ശൂന്യം __debug_enter(ഘടന bregs *regs, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __debug_isr(സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __debug_stub(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __warn_invalid(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __warn_unimplemented(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __warn_internalerror(പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __warn_noalloc(പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __warn_timeout(പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __set_invalid(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __set_unimplemented(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __set_code_invalid(ഘടന bregs *regs, u32 linecode, സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം __set_code_unimplemented(ഘടന bregs *regs, u32 linecode
                              , സ്ഥിരം അക്ഷരം *fname);
ശൂന്യം hexdump(സ്ഥിരം ശൂന്യം *d, പൂർണ്ണസംഖ്യ len);

#define dprintf(lvl, fmt, args...) ചെയ്യുക {                         \
        എങ്കിൽ (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } ആകുമ്പോൾ (0)
#define debug_enter(regs, lvl) ചെയ്യുക {                     \
        എങ്കിൽ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } ആകുമ്പോൾ (0)
#define debug_isr(lvl) ചെയ്യുക {                             \
        എങ്കിൽ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } ആകുമ്പോൾ (0)
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
