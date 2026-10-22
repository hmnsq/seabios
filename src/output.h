#अगर_अघोषित __OUTPUT_H
#घोषणा __OUTPUT_H

#समावेश "config.h" // CONFIG_DEBUG_LEVEL
#समावेश "types.h" // u32

// output.c
व्योम debug_banner(व्योम);
व्योम panic(स्थिर अक्षर *fmt, ...)
    __attribute__ ((format (म_लिखो, 1, 2))) __noreturn;
व्योम म_लिखो(स्थिर अक्षर *fmt, ...)
    __attribute__ ((format (म_लिखो, 1, 2)));
पूर्णांक snprintf(अक्षर *str, माप_प्रकार आकार, स्थिर अक्षर *fmt, ...)
    __attribute__ ((format (म_लिखो, 3, 4)));
अक्षर * znprintf(माप_प्रकार आकार, स्थिर अक्षर *fmt, ...)
    __attribute__ ((format (म_लिखो, 2, 3)));
व्योम __dprintf(स्थिर अक्षर *fmt, ...)
    __attribute__ ((format (म_लिखो, 1, 2)));
काष्ठा bregs;
व्योम __debug_enter(काष्ठा bregs *regs, स्थिर अक्षर *fname);
व्योम __debug_isr(स्थिर अक्षर *fname);
व्योम __debug_stub(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __warn_invalid(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __warn_unimplemented(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __warn_internalerror(पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __warn_noalloc(पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __warn_timeout(पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __set_invalid(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __set_unimplemented(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname);
व्योम __set_code_invalid(काष्ठा bregs *regs, u32 linecode, स्थिर अक्षर *fname);
व्योम __set_code_unimplemented(काष्ठा bregs *regs, u32 linecode
                              , स्थिर अक्षर *fname);
व्योम hexdump(स्थिर व्योम *d, पूर्णांक len);

#घोषणा dprintf(lvl, fmt, args...) करो {                         \
        अगर (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } जबतक (0)
#घोषणा debug_enter(regs, lvl) करो {                     \
        अगर ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } जबतक (0)
#घोषणा debug_isr(lvl) करो {                             \
        अगर ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } जबतक (0)
#घोषणा debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#घोषणा warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#घोषणा warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#घोषणा warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#घोषणा warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#घोषणा warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#घोषणा set_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#घोषणा set_code_invalid(regs, code)                                    \
    __set_code_invalid((regs), (code) | (__LINE__ << 8), __func__)
#घोषणा set_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#घोषणा set_code_unimplemented(regs, code)                              \
    __set_code_unimplemented((regs), (code) | (__LINE__ << 8), __func__)

#पूर्ण_अगर // output.h
