#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __OUTPUT_H
#ವ್ಯಾಖ್ಯಾನಿಸು __OUTPUT_H

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_DEBUG_LEVEL
#ಅನ್ತರ್ಭಾವ "types.h" // u32

// output.c
ಶೂನ್ಯ debug_banner(ಶೂನ್ಯ);
ಶೂನ್ಯ panic(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
    __attribute__ ((format (ಬರೆ, 1, 2))) __noreturn;
ಶೂನ್ಯ ಬರೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
    __attribute__ ((format (ಬರೆ, 1, 2)));
ಪೂರ್ಣಾಂಕ snprintf(ಅಕ್ಷರ *str, ಗಾತ್ರ_t ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
    __attribute__ ((format (ಬರೆ, 3, 4)));
ಅಕ್ಷರ * znprintf(ಗಾತ್ರ_t ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
    __attribute__ ((format (ಬರೆ, 2, 3)));
ಶೂನ್ಯ __dprintf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
    __attribute__ ((format (ಬರೆ, 1, 2)));
ರಚನೆ bregs;
ಶೂನ್ಯ __debug_enter(ರಚನೆ bregs *regs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __debug_isr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __debug_stub(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __warn_invalid(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __warn_unimplemented(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __warn_internalerror(ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __warn_noalloc(ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __warn_timeout(ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __set_invalid(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __set_unimplemented(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __set_code_invalid(ರಚನೆ bregs *regs, u32 linecode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ __set_code_unimplemented(ರಚನೆ bregs *regs, u32 linecode
                              , ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
ಶೂನ್ಯ hexdump(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *d, ಪೂರ್ಣಾಂಕ len);

#ವ್ಯಾಖ್ಯಾನಿಸು dprintf(lvl, fmt, ವಾದಗಳು...) ಮಾಡು {                         \
        ಆದರೆ (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_enter(regs, lvl) ಮಾಡು {                     \
        ಆದರೆ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_isr(lvl) ಮಾಡು {                             \
        ಆದರೆ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_ಸಂಕೇತ_invalid(regs, ಸಂಕೇತ_2)                                    \
    __set_code_invalid((regs), (ಸಂಕೇತ_2) | (__LINE__ << 8), __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_ಸಂಕೇತ_unimplemented(regs, ಸಂಕೇತ_2)                              \
    __set_code_unimplemented((regs), (ಸಂಕೇತ_2) | (__LINE__ << 8), __func__)

#ಆದರೆ_ಅಂತ್ಯ // output.h
