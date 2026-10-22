// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#جے_بنا_تعریف __TYPES_H
#تعریف_کرو __TYPES_H

تعريفنوع بنانشانی حرف u8;
تعريفنوع نشانیوالا حرف s8;
تعريفنوع بنانشانی چھوٹا u16;
تعريفنوع نشانیوالا چھوٹا s16;
تعريفنوع بنانشانی صحيح u32;
تعريفنوع نشانیوالا صحيح s32;
تعريفنوع بنانشانی لما لما u64;
تعريفنوع نشانیوالا لما لما s64;
تعريفنوع u32 ناپ_t;

اتحاد u64_u32_u {
    بنية { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
بنية segoff_s {
    اتحاد {
        بنية {
            u16 وتھ_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#جے_تعریف_شدہ MANUAL_NO_JUMP_جدول
# define افتراضي حالة 775324556: asm(""); افتراضي
#جے_انت

#جے_تعریف_شدہ سارا_پروگرام
# define __VISIBLE __attribute__((externally_visible))
#وإلا
# define __VISIBLE
#جے_انت

#تعریف_کرو UNIQSEC __FILE__ "." __stringify(__LINE__)

#تعریف_کرو __noreturn __attribute__((noreturn))
خارجي فراغ __force_link_error__only_in_32bit_flat(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_32bit_segmented(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_16bit(فراغ) __noreturn;

#تعریف_کرو __ASM(کوڈ) asm(".section .text.asm." UNIQSEC "\n\t" کوڈ)

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
# define ASM16(کوڈ) __ASM(کوڈ)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(کوڈ)
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
# define ASM16(کوڈ)
# define ASM32FLAT(کوڈ)
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
# define ASM16(کوڈ)
# define ASM32FLAT(کوڈ) __ASM(کوڈ)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() افعل { } طالما (0)
#جے_انت

#تعریف_کرو offsetof(TYPE, MEMBER) ((ناپ_t) &((TYPE *)0)->MEMBER)
#تعریف_کرو ARRAY_ناپ(a) (حجم(a) / حجم(a[0]))
#تعریف_کرو میدان_SIZEOF(t, f) (حجم(((t*)0)->f))
#تعریف_کرو DIV_گیڑا_اتے(n,d) (((n) + (d) - 1) / (d))
#تعریف_کرو DIV_گیڑا_CLOSEST(x, مقسوم_علیہ)({                 \
            typeof(مقسوم_علیہ) __divisor = مقسوم_علیہ;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#تعریف_کرو ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#تعریف_کرو __ALIGN_MASK(x,نقاب)    (((x)+(نقاب))&~(نقاب))
#تعریف_کرو ALIGN_تھلے(x,a)         ((x) & ~((typeof(x))(a)-1))
#تعریف_کرو container_دا(ptr, type, member) ({                      \
        ثابت typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (حرف *)__mptr - offsetof(type,member) );})
#تعریف_کرو container_دا_or_null(ptr, type, member) ({              \
        ثابت typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_دا(___mptr, type, member) : عدم; })

#تعریف_کرو likely(x)       __builtin_expect(!!(x), 1)
#تعریف_کرو unlikely(x)     __builtin_expect(!!(x), 0)

#تعریف_کرو عدم ((فراغ*)0)

#تعریف_کرو __weak __attribute__((weak))
#تعریف_کرو __section(S) __attribute__((section(S)))

#تعریف_کرو PACKED __attribute__((packed))
#تعریف_کرو __aligned(x) __attribute__((aligned(x)))

#تعریف_کرو barrier() __asm__ __volatile__("": : :"memory")

#تعریف_کرو noinline __attribute__((noinline))
#تعریف_کرو __always_inline مضمن __attribute__((always_inline))
#تعریف_کرو __malloc __attribute__((__malloc__))
#تعریف_کرو __attribute_const __attribute__((__const__))

#تعریف_کرو __stringify_1(x)        #x
#تعریف_کرو __stringify(x)          __stringify_1(x)

#جے_انت // types.h
