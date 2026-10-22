// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#جيڪڏهن_وصف_نه_ٿيل __TYPES_H
#وصف_ڪريو __TYPES_H

تعريفنوع بنانشان حرف u8;
تعريفنوع نشانوارو حرف s8;
تعريفنوع بنانشان ننڍو u16;
تعريفنوع نشانوارو ننڍو s16;
تعريفنوع بنانشان صحيح u32;
تعريفنوع نشانوارو صحيح s32;
تعريفنوع بنانشان ڊگهو ڊگهو u64;
تعريفنوع نشانوارو ڊگهو ڊگهو s64;
تعريفنوع u32 سائيز_t;

اتحاد u64_u32_u {
    بنية { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
بنية segoff_s {
    اتحاد {
        بنية {
            u16 وٿي_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#جيڪڏهن_وصف_ٿيل MANUAL_NO_JUMP_جدول
# define افتراضي حالة 775324556: asm(""); افتراضي
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل سڄو_پروگرام
# define __VISIBLE __attribute__((externally_visible))
#وإلا
# define __VISIBLE
#جيڪڏهن_پورو

#وصف_ڪريو UNIQSEC __FILE__ "." __stringify(__LINE__)

#وصف_ڪريو __noreturn __attribute__((noreturn))
خارجي فراغ __force_link_error__only_in_32bit_flat(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_32bit_segmented(فراغ) __noreturn;
خارجي فراغ __force_link_error__only_in_16bit(فراغ) __noreturn;

#وصف_ڪريو __ASM(ڪوڊ) asm(".section .text.asm." UNIQSEC "\n\t" ڪوڊ)

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
# define ASM16(ڪوڊ) __ASM(ڪوڊ)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(ڪوڊ)
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
# define ASM16(ڪوڊ)
# define ASM32FLAT(ڪوڊ)
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
# define ASM16(ڪوڊ)
# define ASM32FLAT(ڪوڊ) __ASM(ڪوڊ)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() افعل { } طالما (0)
#جيڪڏهن_پورو

#وصف_ڪريو offsetof(TYPE, MEMBER) ((سائيز_t) &((TYPE *)0)->MEMBER)
#وصف_ڪريو ARRAY_سائيز(a) (حجم(a) / حجم(a[0]))
#وصف_ڪريو ميدان_SIZEOF(t, f) (حجم(((t*)0)->f))
#وصف_ڪريو DIV_ڦيرو_مٿي(n,d) (((n) + (d) - 1) / (d))
#وصف_ڪريو DIV_ڦيرو_CLOSEST(x, ورهائيندڙ)({                 \
            typeof(ورهائيندڙ) __divisor = ورهائيندڙ;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#وصف_ڪريو ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#وصف_ڪريو __ALIGN_MASK(x,نقاب)    (((x)+(نقاب))&~(نقاب))
#وصف_ڪريو ALIGN_هيٺ_ڏانهن(x,a)         ((x) & ~((typeof(x))(a)-1))
#وصف_ڪريو container_جو(ptr, type, member) ({                      \
        ثابت typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (حرف *)__mptr - offsetof(type,member) );})
#وصف_ڪريو container_جو_or_null(ptr, type, member) ({              \
        ثابت typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_جو(___mptr, type, member) : عدم; })

#وصف_ڪريو likely(x)       __builtin_expect(!!(x), 1)
#وصف_ڪريو unlikely(x)     __builtin_expect(!!(x), 0)

#وصف_ڪريو عدم ((فراغ*)0)

#وصف_ڪريو __weak __attribute__((weak))
#وصف_ڪريو __section(S) __attribute__((section(S)))

#وصف_ڪريو PACKED __attribute__((packed))
#وصف_ڪريو __aligned(x) __attribute__((aligned(x)))

#وصف_ڪريو barrier() __asm__ __volatile__("": : :"memory")

#وصف_ڪريو noinline __attribute__((noinline))
#وصف_ڪريو __always_inline مضمن __attribute__((always_inline))
#وصف_ڪريو __malloc __attribute__((__malloc__))
#وصف_ڪريو __attribute_const __attribute__((__const__))

#وصف_ڪريو __stringify_1(x)        #x
#وصف_ڪريو __stringify(x)          __stringify_1(x)

#جيڪڏهن_پورو // types.h
