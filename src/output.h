#ifndef __OUTPUT_H
#define __OUTPUT_H

#அந்தர்பாவ "config.h" // CONFIG_DEBUG_LEVEL
#அந்தர்பாவ "types.h" // u32

// output.c
வெறுமை debug_banner(வெறுமை);
வெறுமை panic(மாறிலி எழுத்து *fmt, ...)
    __attribute__ ((format (printf, 1, 2))) __noreturn;
வெறுமை printf(மாறிலி எழுத்து *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
முழு snprintf(எழுத்து *str, size_t size, மாறிலி எழுத்து *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
எழுத்து * znprintf(size_t size, மாறிலி எழுத்து *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));
வெறுமை __dprintf(மாறிலி எழுத்து *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));
கட்டமைப்பு bregs;
வெறுமை __debug_enter(கட்டமைப்பு bregs *regs, மாறிலி எழுத்து *fname);
வெறுமை __debug_isr(மாறிலி எழுத்து *fname);
வெறுமை __debug_stub(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __warn_invalid(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __warn_unimplemented(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __warn_internalerror(முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __warn_noalloc(முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __warn_timeout(முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __set_invalid(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __set_unimplemented(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname);
வெறுமை __set_code_invalid(கட்டமைப்பு bregs *regs, u32 linecode, மாறிலி எழுத்து *fname);
வெறுமை __set_code_unimplemented(கட்டமைப்பு bregs *regs, u32 linecode
                              , மாறிலி எழுத்து *fname);
வெறுமை hexdump(மாறிலி வெறுமை *d, முழு len);

#define dprintf(lvl, fmt, args...) செய் {                         \
        எனில் (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } போது (0)
#define debug_enter(regs, lvl) செய் {                     \
        எனில் ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } போது (0)
#define debug_isr(lvl) செய் {                             \
        எனில் ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } போது (0)
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
