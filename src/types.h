// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __TYPES_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __TYPES_H

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ u8;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ s8;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ u16;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ s16;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ u32;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ s32;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ u64;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ ਲੰਮਾ s64;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ u32 ਮਾਪ_t;

ਸੰਘ u64_u32_u {
    ਬਣਤਰ { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
ਬਣਤਰ segoff_s {
    ਸੰਘ {
        ਬਣਤਰ {
            u16 ਵਿੱਥ_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MANUAL_NO_JUMP_ਸਾਰਣੀ
# define ਮੂਲ ਹਾਲਤ 775324556: asm(""); ਮੂਲ
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸਾਰਾ_ਪ੍ਰੋਗਰਾਮ
# define __VISIBLE __attribute__((externally_visible))
#ਨਹੀਂਤਾਂ
# define __VISIBLE
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIQSEC __FILE__ "." __stringify(__LINE__)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __noreturn __attribute__((noreturn))
ਬਾਹਰੀ ਖਾਲੀ __force_link_error__only_in_32bit_flat(ਖਾਲੀ) __noreturn;
ਬਾਹਰੀ ਖਾਲੀ __force_link_error__only_in_32bit_segmented(ਖਾਲੀ) __noreturn;
ਬਾਹਰੀ ਖਾਲੀ __force_link_error__only_in_16bit(ਖਾਲੀ) __noreturn;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __ASM(ਕੋਡ) asm(".section .text.asm." UNIQSEC "\n\t" ਕੋਡ)

#ਜੇ MODE16 == 1
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
# define ASM16(ਕੋਡ) __ASM(ਕੋਡ)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(ਕੋਡ)
// Compile time check for a given mode.
# define ASSERT16() ਕਰੋ { } ਜਦੋਂ (0)
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#ਜਾਂਜੇ MODESEGMENT == 1
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
# define ASM16(ਕੋਡ)
# define ASM32FLAT(ਕੋਡ)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() ਕਰੋ { } ਜਦੋਂ (0)
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#ਨਹੀਂਤਾਂ
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
# define ASM16(ਕੋਡ)
# define ASM32FLAT(ਕੋਡ) __ASM(ਕੋਡ)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() ਕਰੋ { } ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ offsetof(TYPE, MEMBER) ((ਮਾਪ_t) &((TYPE *)0)->MEMBER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ARRAY_ਮਾਪ(a) (ਆਕਾਰ(a) / ਆਕਾਰ(a[0]))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੇਤਰ_SIZEOF(t, f) (ਆਕਾਰ(((t*)0)->f))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIV_ਗੇੜਾ_ਉੱਪਰ(n,d) (((n) + (d) - 1) / (d))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIV_ਗੇੜਾ_CLOSEST(x, ਭਾਜਕ)({                 \
            typeof(ਭਾਜਕ) __divisor = ਭਾਜਕ;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __ALIGN_MASK(x,ਨਕਾਬ)    (((x)+(ਨਕਾਬ))&~(ਨਕਾਬ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN_ਥੱਲੇ(x,a)         ((x) & ~((typeof(x))(a)-1))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ container_ਦਾ(ptr, type, member) ({                      \
        ਅਚਲ typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (ਅੱਖਰ *)__mptr - offsetof(type,member) );})
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ container_ਦਾ_or_null(ptr, type, member) ({              \
        ਅਚਲ typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_ਦਾ(___mptr, type, member) : ਨਿਰਾ; })

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ likely(x)       __builtin_expect(!!(x), 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unlikely(x)     __builtin_expect(!!(x), 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਨਿਰਾ ((ਖਾਲੀ*)0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __weak __attribute__((weak))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __section(S) __attribute__((section(S)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PACKED __attribute__((packed))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __aligned(x) __attribute__((aligned(x)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ barrier() __asm__ __volatile__("": : :"memory")

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ noinline __attribute__((noinline))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __always_inline ਸਤਰਅੰਦਰ __attribute__((always_inline))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __malloc __attribute__((__malloc__))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __attribute_const __attribute__((__const__))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __stringify_1(x)        #x
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __stringify(x)          __stringify_1(x)

#ਜੇ_ਅੰਤ // types.h
