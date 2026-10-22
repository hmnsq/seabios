// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#अगर_अघोषित __TYPES_H
#घोषणा __TYPES_H

प्रकार अचिन्हित अक्षर u8;
प्रकार चिन्हित अक्षर s8;
प्रकार अचिन्हित लघु u16;
प्रकार चिन्हित लघु s16;
प्रकार अचिन्हित पूर्णांक u32;
प्रकार चिन्हित पूर्णांक s32;
प्रकार अचिन्हित दीर्घ दीर्घ u64;
प्रकार चिन्हित दीर्घ दीर्घ s64;
प्रकार u32 माप_प्रकार;

जोड़ u64_u32_u {
    काष्ठा { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
काष्ठा segoff_s {
    जोड़ {
        काष्ठा {
            u16 offset;
            u16 seg;
        };
        u32 segoff;
    };
};

#अगर_घोषित MANUAL_NO_JUMP_TABLE
# define शेष हाल 775324556: यंत्र(""); शेष
#पूर्ण_अगर

#अगर_घोषित WHOLE_PROGRAM
# define __VISIBLE __attribute__((externally_visible))
#अन्यथा
# define __VISIBLE
#पूर्ण_अगर

#घोषणा UNIQSEC __खाता__ "." __stringify(__LINE__)

#घोषणा __noreturn __attribute__((noreturn))
बाह्य व्योम __force_link_error__only_in_32bit_flat(व्योम) __noreturn;
बाह्य व्योम __force_link_error__only_in_32bit_segmented(व्योम) __noreturn;
बाह्य व्योम __force_link_error__only_in_16bit(व्योम) __noreturn;

#घोषणा __ASM(code) यंत्र(".section .text.asm." UNIQSEC "\n\t" code)

#अगर MODE16 == 1
// Notes a function as externally visible in the 16bit code chunk.
# define VISIBLE16 __VISIBLE
// Notes a function as externally visible in the 32bit flat code chunk.
# define VISIBLE32FLAT
// Notes a 32bit flat function that will only be called during init.
# define VISIBLE32INIT
// Notes a function as externally visible in the 32bit segmented code chunk.
# define VISIBLE32SEG
// Designate a variable as (only) visible to 16bit code.
# define VAR16 __section(".data16." UNIQSEC)
// Designate a variable as (only) visible to 32bit segmented code.
# define VAR32SEG __section(".discard.var32seg." UNIQSEC)
// Designate a variable as visible and located in the e-segment.
# define VARLOW __section(".discard.varlow." UNIQSEC) __VISIBLE __weak
// Designate a variable as visible and located in the f-segment.
# define VARFSEG __section(".discard.varfseg." UNIQSEC) __VISIBLE __weak
// Designate a variable at a specific address in the f-segment.
# define VARFSEGFIXED(addr) __section(".discard.varfixed." UNIQSEC) __VISIBLE __weak
// Verify a variable is only accessible via 32bit "init" functions
# define VARVERIFY32INIT __section(".discard.varinit." UNIQSEC)
// Designate top-level assembler as 16bit only.
# define ASM16(code) __ASM(code)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(code)
// Compile time check for a given mode.
# define ASSERT16() करो { } जबतक (0)
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#या_अगर MODESEGMENT == 1
# define VISIBLE16
# define VISIBLE32FLAT
# define VISIBLE32INIT
# define VISIBLE32SEG __VISIBLE
# define VAR16 __section(".discard.var16." UNIQSEC)
# define VAR32SEG __section(".data32seg." UNIQSEC)
# define VARLOW __section(".discard.varlow." UNIQSEC) __VISIBLE __weak
# define VARFSEG __section(".discard.varfseg." UNIQSEC) __VISIBLE __weak
# define VARFSEGFIXED(addr) __section(".discard.varfixed." UNIQSEC) __VISIBLE __weak
# define VARVERIFY32INIT __section(".discard.varinit." UNIQSEC)
# define ASM16(code)
# define ASM32FLAT(code)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() करो { } जबतक (0)
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#अन्यथा
# define VISIBLE16
# define VISIBLE32FLAT __section(".text.runtime." UNIQSEC) __VISIBLE
# define VISIBLE32INIT __section(".text.init." UNIQSEC) __VISIBLE
# define VISIBLE32SEG
# define VAR16 __section(".discard.var16." UNIQSEC)
# define VAR32SEG __section(".discard.var32seg." UNIQSEC)
# define VARLOW __section(".data.varlow." UNIQSEC) __VISIBLE __weak
# define VARFSEG __section(".data.varfseg." UNIQSEC) __VISIBLE
# define VARFSEGFIXED(addr) __section(".fixedaddr." __stringify(addr)) __VISIBLE __aligned(1)
# define VARVERIFY32INIT __section(".data.varinit." UNIQSEC)
# define ASM16(code)
# define ASM32FLAT(code) __ASM(code)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() करो { } जबतक (0)
#पूर्ण_अगर

#घोषणा दुरत्व(TYPE, MEMBER) ((माप_प्रकार) &((TYPE *)0)->MEMBER)
#घोषणा ARRAY_आकार(a) (माप(a) / माप(a[0]))
#घोषणा FIELD_SIZEOF(t, f) (माप(((t*)0)->f))
#घोषणा DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#घोषणा DIV_ROUND_CLOSEST(x, भाजक)({                 \
            typeof(भाजक) __divisor = भाजक;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#घोषणा ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#घोषणा __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#घोषणा ALIGN_DOWN(x,a)         ((x) & ~((typeof(x))(a)-1))
#घोषणा container_of(ptr, type, member) ({                      \
        स्थिर typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (अक्षर *)__mptr - दुरत्व(type,member) );})
#घोषणा container_of_or_null(ptr, type, member) ({              \
        स्थिर typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_of(___mptr, type, member) : शून्य; })

#घोषणा likely(x)       __builtin_expect(!!(x), 1)
#घोषणा unlikely(x)     __builtin_expect(!!(x), 0)

#घोषणा शून्य ((व्योम*)0)

#घोषणा __weak __attribute__((weak))
#घोषणा __section(S) __attribute__((section(S)))

#घोषणा PACKED __attribute__((packed))
#घोषणा __aligned(x) __attribute__((aligned(x)))

#घोषणा barrier() __asm__ __volatile__("": : :"memory")

#घोषणा noinline __attribute__((noinline))
#घोषणा __always_inline अंतरभूत __attribute__((always_inline))
#घोषणा __malloc __attribute__((__malloc__))
#घोषणा __attribute_const __attribute__((__const__))

#घोषणा __stringify_1(x)        #x
#घोषणा __stringify(x)          __stringify_1(x)

#पूर्ण_अगर // types.h
