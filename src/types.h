// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#اگر_تعریف_نشده __TYPES_H
#تعریف_کن __TYPES_H

تعريفنوع بیعلامت حرف u8;
تعريفنوع علامتدار حرف s8;
تعريفنوع بیعلامت کوتاه u16;
تعريفنوع علامتدار کوتاه s16;
تعريفنوع بیعلامت صحيح u32;
تعريفنوع علامتدار صحيح s32;
تعريفنوع بیعلامت دراز دراز u64;
تعريفنوع علامتدار دراز دراز s64;
تعريفنوع u32 اندازه_t;

اتحاد u64_u32_u {
    بنية { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
بنية segoff_s {
    اتحاد {
        بنية {
            u16 جابه_جایی_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#اگر_تعریف_شده MANUAL_NO_JUMP_جدول
# define افتراضي حالة 775324556: asm(""); افتراضي
#پایان_اگر

#اگر_تعریف_شده کامل_برنامه
# define __VISIBLE __attribute__((externally_visible))
#وإلا
# define __VISIBLE
#پایان_اگر

#تعریف_کن UNIQSEC __FILE__ "." __stringify(__LINE__)

#تعریف_کن __noreturn __attribute__((noreturn))
خارجي فراغ __force_link_error__only_in_32bit_flat(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_32bit_segmented(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_16bit(فراغ) __noreturn;

#تعریف_کن __ASM(کد) asm(".section .text.asm." UNIQSEC "\n\t" کد)

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
# define ASM16(کد) __ASM(کد)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(کد)
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
# define ASM16(کد)
# define ASM32FLAT(کد)
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
# define ASM16(کد)
# define ASM32FLAT(کد) __ASM(کد)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() افعل { } طالما (0)
#پایان_اگر

#تعریف_کن offsetof(TYPE, MEMBER) ((اندازه_t) &((TYPE *)0)->MEMBER)
#تعریف_کن ARRAY_اندازه(a) (حجم(a) / حجم(a[0]))
#تعریف_کن فیلد_SIZEOF(t, f) (حجم(((t*)0)->f))
#تعریف_کن DIV_دور_به_بالا(n,d) (((n) + (d) - 1) / (d))
#تعریف_کن DIV_دور_CLOSEST(x, مقسوم_علیه)({                 \
            typeof(مقسوم_علیه) __divisor = مقسوم_علیه;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#تعریف_کن ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#تعریف_کن __ALIGN_MASK(x,ماسک)    (((x)+(ماسک))&~(ماسک))
#تعریف_کن ALIGN_پایین(x,a)         ((x) & ~((typeof(x))(a)-1))
#تعریف_کن container_از(ptr, type, member) ({                      \
        ثابت typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (حرف *)__mptr - offsetof(type,member) );})
#تعریف_کن container_از_or_null(ptr, type, member) ({              \
        ثابت typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_از(___mptr, type, member) : عدم; })

#تعریف_کن likely(x)       __builtin_expect(!!(x), 1)
#تعریف_کن unlikely(x)     __builtin_expect(!!(x), 0)

#تعریف_کن عدم ((فراغ*)0)

#تعریف_کن __weak __attribute__((weak))
#تعریف_کن __section(S) __attribute__((section(S)))

#تعریف_کن PACKED __attribute__((packed))
#تعریف_کن __aligned(x) __attribute__((aligned(x)))

#تعریف_کن barrier() __asm__ __volatile__("": : :"memory")

#تعریف_کن noinline __attribute__((noinline))
#تعریف_کن __always_inline مضمن __attribute__((always_inline))
#تعریف_کن __malloc __attribute__((__malloc__))
#تعریف_کن __attribute_const __attribute__((__const__))

#تعریف_کن __stringify_1(x)        #x
#تعریف_کن __stringify(x)          __stringify_1(x)

#پایان_اگر // types.h
