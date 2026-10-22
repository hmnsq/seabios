// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __TYPES_H
#ವ್ಯಾಖ್ಯಾನಿಸು __TYPES_H

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ u8;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ s8;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ u16;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ s16;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ u32;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ s32;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ u64;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ ದೀರ್ಘ s64;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u32 ಗಾತ್ರ_t;

ಒಕ್ಕೂಟ u64_u32_u {
    ರಚನೆ { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
ರಚನೆ segoff_s {
    ಒಕ್ಕೂಟ {
        ರಚನೆ {
            u16 ಸ್ಥಾನಾಂತರ_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MANUAL_NO_JUMP_ಕೋಷ್ಟಕ
# define ಪೂರ್ವನಿಯೋಜಿತ ಸ್ಥಿತಿ 775324556: asm(""); ಪೂರ್ವನಿಯೋಜಿತ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಸಂಪೂರ್ಣ_ಕಾರ್ಯಕ್ರಮ
# define __VISIBLE __attribute__((externally_visible))
#ಇಲ್ಲದಿದ್ದರೆ
# define __VISIBLE
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು UNIQSEC __FILE__ "." __stringify(__LINE__)

#ವ್ಯಾಖ್ಯಾನಿಸು __noreturn __attribute__((noreturn))
ಬಾಹ್ಯ ಶೂನ್ಯ __force_link_error__only_in_32bit_flat(ಶೂನ್ಯ) __noreturn;
ಬಾಹ್ಯ ಶೂನ್ಯ __force_link_error__only_in_32bit_segmented(ಶೂನ್ಯ) __noreturn;
ಬಾಹ್ಯ ಶೂನ್ಯ __force_link_error__only_in_16bit(ಶೂನ್ಯ) __noreturn;

#ವ್ಯಾಖ್ಯಾನಿಸು __ASM(ಸಂಕೇತ_2) asm(".section .text.asm." UNIQSEC "\n\t" ಸಂಕೇತ_2)

#ಆದರೆ MODE16 == 1
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
# define ASM16(ಸಂಕೇತ_2) __ASM(ಸಂಕೇತ_2)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(ಸಂಕೇತ_2)
// Compile time check for a given mode.
# define ASSERT16() ಮಾಡು { } ಆಗ (0)
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#ಅಥವಾಆದರೆ MODESEGMENT == 1
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
# define ASM16(ಸಂಕೇತ_2)
# define ASM32FLAT(ಸಂಕೇತ_2)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() ಮಾಡು { } ಆಗ (0)
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#ಇಲ್ಲದಿದ್ದರೆ
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
# define ASM16(ಸಂಕೇತ_2)
# define ASM32FLAT(ಸಂಕೇತ_2) __ASM(ಸಂಕೇತ_2)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() ಮಾಡು { } ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು offsetof(TYPE, MEMBER) ((ಗಾತ್ರ_t) &((TYPE *)0)->MEMBER)
#ವ್ಯಾಖ್ಯಾನಿಸು ARRAY_ಗಾತ್ರ(a) (ಗಾತ್ರ(a) / ಗಾತ್ರ(a[0]))
#ವ್ಯಾಖ್ಯಾನಿಸು ಕ್ಷೇತ್ರ_SIZEOF(t, f) (ಗಾತ್ರ(((t*)0)->f))
#ವ್ಯಾಖ್ಯಾನಿಸು DIV_ಸುತ್ತು_ಮೇಲೆ(n,d) (((n) + (d) - 1) / (d))
#ವ್ಯಾಖ್ಯಾನಿಸು DIV_ಸುತ್ತು_CLOSEST(x, ಭಾಜಕ)({                 \
            typeof(ಭಾಜಕ) __divisor = ಭಾಜಕ;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#ವ್ಯಾಖ್ಯಾನಿಸು __ALIGN_MASK(x,ಮುಖವಾಡ)    (((x)+(ಮುಖವಾಡ))&~(ಮುಖವಾಡ))
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN_ಕೆಳಕ್ಕೆ(x,a)         ((x) & ~((typeof(x))(a)-1))
#ವ್ಯಾಖ್ಯಾನಿಸು container_ದ(ptr, type, member) ({                      \
        ಸ್ಥಿರಾಂಕ typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (ಅಕ್ಷರ *)__mptr - offsetof(type,member) );})
#ವ್ಯಾಖ್ಯಾನಿಸು container_ದ_or_null(ptr, type, member) ({              \
        ಸ್ಥಿರಾಂಕ typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_ದ(___mptr, type, member) : ಖಾಲಿ; })

#ವ್ಯಾಖ್ಯಾನಿಸು likely(x)       __builtin_expect(!!(x), 1)
#ವ್ಯಾಖ್ಯಾನಿಸು unlikely(x)     __builtin_expect(!!(x), 0)

#ವ್ಯಾಖ್ಯಾನಿಸು ಖಾಲಿ ((ಶೂನ್ಯ*)0)

#ವ್ಯಾಖ್ಯಾನಿಸು __weak __attribute__((weak))
#ವ್ಯಾಖ್ಯಾನಿಸು __section(S) __attribute__((section(S)))

#ವ್ಯಾಖ್ಯಾನಿಸು PACKED __attribute__((packed))
#ವ್ಯಾಖ್ಯಾನಿಸು __aligned(x) __attribute__((aligned(x)))

#ವ್ಯಾಖ್ಯಾನಿಸು barrier() __asm__ __volatile__("": : :"memory")

#ವ್ಯಾಖ್ಯಾನಿಸು noinline __attribute__((noinline))
#ವ್ಯಾಖ್ಯಾನಿಸು __always_inline ಸಾಲೊಳಗೆ __attribute__((always_inline))
#ವ್ಯಾಖ್ಯಾನಿಸು __malloc __attribute__((__malloc__))
#ವ್ಯಾಖ್ಯಾನಿಸು __attribute_const __attribute__((__const__))

#ವ್ಯಾಖ್ಯಾನಿಸು __stringify_1(x)        #x
#ವ್ಯಾಖ್ಯಾನಿಸು __stringify(x)          __stringify_1(x)

#ಆದರೆ_ಅಂತ್ಯ // types.h
