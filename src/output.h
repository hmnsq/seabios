#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __OUTPUT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __OUTPUT_H

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_DEBUG_LEVEL
#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

// output.c
ਖਾਲੀ debug_banner(ਖਾਲੀ);
ਖਾਲੀ panic(ਅਚਲ ਅੱਖਰ *fmt, ...)
    __attribute__ ((format (ਲਿਖੋ, 1, 2))) __noreturn;
ਖਾਲੀ ਲਿਖੋ(ਅਚਲ ਅੱਖਰ *fmt, ...)
    __attribute__ ((format (ਲਿਖੋ, 1, 2)));
ਪੂਰਨ snprintf(ਅੱਖਰ *str, ਮਾਪ_t ਮਾਪ, ਅਚਲ ਅੱਖਰ *fmt, ...)
    __attribute__ ((format (ਲਿਖੋ, 3, 4)));
ਅੱਖਰ * znprintf(ਮਾਪ_t ਮਾਪ, ਅਚਲ ਅੱਖਰ *fmt, ...)
    __attribute__ ((format (ਲਿਖੋ, 2, 3)));
ਖਾਲੀ __dprintf(ਅਚਲ ਅੱਖਰ *fmt, ...)
    __attribute__ ((format (ਲਿਖੋ, 1, 2)));
ਬਣਤਰ bregs;
ਖਾਲੀ __debug_enter(ਬਣਤਰ bregs *regs, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __debug_isr(ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __debug_stub(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __warn_invalid(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __warn_unimplemented(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __warn_internalerror(ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __warn_noalloc(ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __warn_timeout(ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __set_invalid(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __set_unimplemented(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __set_code_invalid(ਬਣਤਰ bregs *regs, u32 linecode, ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ __set_code_unimplemented(ਬਣਤਰ bregs *regs, u32 linecode
                              , ਅਚਲ ਅੱਖਰ *fname);
ਖਾਲੀ hexdump(ਅਚਲ ਖਾਲੀ *d, ਪੂਰਨ len);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dprintf(lvl, fmt, ਦਲੀਲਾਂ...) ਕਰੋ {                         \
        ਜੇ (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_enter(regs, lvl) ਕਰੋ {                     \
        ਜੇ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_isr(lvl) ਕਰੋ {                             \
        ਜੇ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਕੋਡ_invalid(regs, ਕੋਡ)                                    \
    __set_code_invalid((regs), (ਕੋਡ) | (__LINE__ << 8), __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਕੋਡ_unimplemented(regs, ਕੋਡ)                              \
    __set_code_unimplemented((regs), (ਕੋਡ) | (__LINE__ << 8), __func__)

#ਜੇ_ਅੰਤ // output.h
