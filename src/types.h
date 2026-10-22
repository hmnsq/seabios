// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#यदि_अपरिभाषितम् __TYPES_H
#परिभाषय __TYPES_H

प्रकारनिर्देश अचिह्नित वर्ण u8;
प्रकारनिर्देश चिह्नित वर्ण s8;
प्रकारनिर्देश अचिह्नित ह्रस्व u16;
प्रकारनिर्देश चिह्नित ह्रस्व s16;
प्रकारनिर्देश अचिह्नित पूर्णाङ्क u32;
प्रकारनिर्देश चिह्नित पूर्णाङ्क s32;
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ u64;
प्रकारनिर्देश चिह्नित दीर्घ दीर्घ s64;
प्रकारनिर्देश u32 आकार_t;

सङ्घ u64_u32_u {
    संरचना { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
संरचना segoff_s {
    सङ्घ {
        संरचना {
            u16 विस्थापनम्_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#यदि_परिभाषितम् MANUAL_NO_JUMP_सारणी
# define स्वभाव पक्ष 775324556: asm(""); स्वभाव
#यदि_अन्तः

#यदि_परिभाषितम् सम्पूर्णम्_क्रमादेशः
# define __VISIBLE __attribute__((externally_visible))
#अन्यथा
# define __VISIBLE
#यदि_अन्तः

#परिभाषय UNIQSEC __FILE__ "." __stringify(__LINE__)

#परिभाषय __noreturn __attribute__((noreturn))
बाह्य शून्य __force_link_error__only_in_32bit_flat(शून्य) __noreturn;
बाह्य शून्य __force_link_error__only_in_32bit_segmented(शून्य) __noreturn;
बाह्य शून्य __force_link_error__only_in_16bit(शून्य) __noreturn;

#परिभाषय __ASM(संहिता) asm(".section .text.asm." UNIQSEC "\n\t" संहिता)

#यदि MODE16 == 1
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
# define ASM16(संहिता) __ASM(संहिता)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(संहिता)
// Compile time check for a given mode.
# define ASSERT16() कुरु { } यावत् (0)
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#नोचेद्यदि MODESEGMENT == 1
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
# define ASM16(संहिता)
# define ASM32FLAT(संहिता)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() कुरु { } यावत् (0)
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
# define ASM16(संहिता)
# define ASM32FLAT(संहिता) __ASM(संहिता)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() कुरु { } यावत् (0)
#यदि_अन्तः

#परिभाषय offsetof(TYPE, MEMBER) ((आकार_t) &((TYPE *)0)->MEMBER)
#परिभाषय ARRAY_आकार(a) (परिमाण(a) / परिमाण(a[0]))
#परिभाषय क्षेत्रम्_SIZEOF(t, f) (परिमाण(((t*)0)->f))
#परिभाषय DIV_आवर्तनम्_ऊर्ध्वम्(n,d) (((n) + (d) - 1) / (d))
#परिभाषय DIV_आवर्तनम्_CLOSEST(x, भाजकः)({                 \
            typeof(भाजकः) __divisor = भाजकः;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#परिभाषय ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#परिभाषय __ALIGN_MASK(x,आवरणम्)    (((x)+(आवरणम्))&~(आवरणम्))
#परिभाषय ALIGN_अधस्तात्(x,a)         ((x) & ~((typeof(x))(a)-1))
#परिभाषय container_सम्बन्धि(ptr, type, member) ({                      \
        ध्रुव typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (वर्ण *)__mptr - offsetof(type,member) );})
#परिभाषय container_सम्बन्धि_or_null(ptr, type, member) ({              \
        ध्रुव typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_सम्बन्धि(___mptr, type, member) : रिक्त; })

#परिभाषय likely(x)       __builtin_expect(!!(x), 1)
#परिभाषय unlikely(x)     __builtin_expect(!!(x), 0)

#परिभाषय रिक्त ((शून्य*)0)

#परिभाषय __weak __attribute__((weak))
#परिभाषय __section(S) __attribute__((section(S)))

#परिभाषय PACKED __attribute__((packed))
#परिभाषय __aligned(x) __attribute__((aligned(x)))

#परिभाषय barrier() __asm__ __volatile__("": : :"memory")

#परिभाषय noinline __attribute__((noinline))
#परिभाषय __always_inline अन्तःस्थ __attribute__((always_inline))
#परिभाषय __malloc __attribute__((__malloc__))
#परिभाषय __attribute_const __attribute__((__const__))

#परिभाषय __stringify_1(x)        #x
#परिभाषय __stringify(x)          __stringify_1(x)

#यदि_अन्तः // types.h
