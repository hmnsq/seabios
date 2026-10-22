// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#که_نه_تعریف_شوی __TYPES_H
#تعریف_کړه __TYPES_H

تعريفنوع بینښه حرف u8;
تعريفنوع نښهدار حرف s8;
تعريفنوع بینښه لنډ u16;
تعريفنوع نښهدار لنډ s16;
تعريفنوع بینښه صحيح u32;
تعريفنوع نښهدار صحيح s32;
تعريفنوع بینښه اوږد اوږد u64;
تعريفنوع نښهدار اوږد اوږد s64;
تعريفنوع u32 اندازه_t;

اتحاد u64_u32_u {
    بنية { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
بنية segoff_s {
    اتحاد {
        بنية {
            u16 دننه_واټن_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#که_تعریف_شوی MANUAL_NO_JUMP_جدول
# define افتراضي حالة 775324556: asm(""); افتراضي
#د_که_پای

#که_تعریف_شوی ټولیز_پروګرام
# define __VISIBLE __attribute__((externally_visible))
#وإلا
# define __VISIBLE
#د_که_پای

#تعریف_کړه UNIQSEC __FILE__ "." __stringify(__LINE__)

#تعریف_کړه __noreturn __attribute__((noreturn))
خارجي فراغ __force_link_error__only_in_32bit_flat(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_32bit_segmented(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_16bit(فراغ) __noreturn;

#تعریف_کړه __ASM(کوډ) asm(".section .text.asm." UNIQSEC "\n\t" کوډ)

#إذا MODE16 == 1
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
# define ASM16(کوډ) __ASM(کوډ)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(کوډ)
// Compile time check for a given mode.
# define ASSERT16() افعل { } طالما (0)
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#وإلاإذا MODESEGMENT == 1
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
# define ASM16(کوډ)
# define ASM32FLAT(کوډ)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() افعل { } طالما (0)
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#وإلا
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
# define ASM16(کوډ)
# define ASM32FLAT(کوډ) __ASM(کوډ)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() افعل { } طالما (0)
#د_که_پای

#تعریف_کړه offsetof(TYPE, MEMBER) ((اندازه_t) &((TYPE *)0)->MEMBER)
#تعریف_کړه ARRAY_اندازه(a) (حجم(a) / حجم(a[0]))
#تعریف_کړه ډګر_SIZEOF(t, f) (حجم(((t*)0)->f))
#تعریف_کړه DIV_پړاو_پورته(n,d) (((n) + (d) - 1) / (d))
#تعریف_کړه DIV_پړاو_CLOSEST(x, وېشونکی)({                 \
            typeof(وېشونکی) __divisor = وېشونکی;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#تعریف_کړه ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#تعریف_کړه __ALIGN_MASK(x,ماسک)    (((x)+(ماسک))&~(ماسک))
#تعریف_کړه ALIGN_ښکته(x,a)         ((x) & ~((typeof(x))(a)-1))
#تعریف_کړه container_د(ptr, type, member) ({                      \
        ثابت typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (حرف *)__mptr - offsetof(type,member) );})
#تعریف_کړه container_د_or_null(ptr, type, member) ({              \
        ثابت typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_د(___mptr, type, member) : عدم; })

#تعریف_کړه likely(x)       __builtin_expect(!!(x), 1)
#تعریف_کړه unlikely(x)     __builtin_expect(!!(x), 0)

#تعریف_کړه عدم ((فراغ*)0)

#تعریف_کړه __weak __attribute__((weak))
#تعریف_کړه __section(S) __attribute__((section(S)))

#تعریف_کړه PACKED __attribute__((packed))
#تعریف_کړه __aligned(x) __attribute__((aligned(x)))

#تعریف_کړه barrier() __asm__ __volatile__("": : :"memory")

#تعریف_کړه noinline __attribute__((noinline))
#تعریف_کړه __always_inline مضمن __attribute__((always_inline))
#تعریف_کړه __malloc __attribute__((__malloc__))
#تعریف_کړه __attribute_const __attribute__((__const__))

#تعریف_کړه __stringify_1(x)        #x
#تعریف_کړه __stringify(x)          __stringify_1(x)

#د_که_پای // types.h
