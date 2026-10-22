// Basic type definitions for X86 cpus.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#నిర్వచితం_కాకపోతే __TYPES_H
#నిర్వచించు __TYPES_H

రకనిర్వచనం గుర్తులేని అక్షరం u8;
రకనిర్వచనం గుర్తుగల అక్షరం s8;
రకనిర్వచనం గుర్తులేని పొట్టి u16;
రకనిర్వచనం గుర్తుగల పొట్టి s16;
రకనిర్వచనం గుర్తులేని పూర్ణాంకం u32;
రకనిర్వచనం గుర్తుగల పూర్ణాంకం s32;
రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ u64;
రకనిర్వచనం గుర్తుగల దీర్ఘ దీర్ఘ s64;
రకనిర్వచనం u32 కొలత_t;

సంఘం u64_u32_u {
    నిర్మాణం { u32 lo, hi; };
    u64 val;
};

// Definition for common 16bit segment/offset pointers.
నిర్మాణం segoff_s {
    సంఘం {
        నిర్మాణం {
            u16 స్థానభ్రంశం_2;
            u16 seg;
        };
        u32 segoff;
    };
};

#నిర్వచితమైతే MANUAL_NO_JUMP_పట్టిక
# define అప్రమేయ స్థితి 775324556: asm(""); అప్రమేయ
#అయితే_ముగింపు

#నిర్వచితమైతే సంపూర్ణ_కార్యక్రమం
# define __VISIBLE __attribute__((externally_visible))
#లేకపోతే
# define __VISIBLE
#అయితే_ముగింపు

#నిర్వచించు UNIQSEC __FILE__ "." __stringify(__LINE__)

#నిర్వచించు __noreturn __attribute__((noreturn))
బాహ్య శూన్య __force_link_error__only_in_32bit_flat(శూన్య) __noreturn;
బాహ్య శూన్య __force_link_error__only_in_32bit_segmented(శూన్య) __noreturn;
బాహ్య శూన్య __force_link_error__only_in_16bit(శూన్య) __noreturn;

#నిర్వచించు __ASM(సంకేతం_2) asm(".section .text.asm." UNIQSEC "\n\t" సంకేతం_2)

#ఒకవేళ MODE16 == 1
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
# define ASM16(సంకేతం_2) __ASM(సంకేతం_2)
// Designate top-level assembler as 32bit flat only.
# define ASM32FLAT(సంకేతం_2)
// Compile time check for a given mode.
# define ASSERT16() చేయి { } అయితే (0)
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#లేదావేళ MODESEGMENT == 1
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
# define ASM16(సంకేతం_2)
# define ASM32FLAT(సంకేతం_2)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() చేయి { } అయితే (0)
# define ASSERT32FLAT() __force_link_error__only_in_32bit_flat()
#లేకపోతే
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
# define ASM16(సంకేతం_2)
# define ASM32FLAT(సంకేతం_2) __ASM(సంకేతం_2)
# define ASSERT16() __force_link_error__only_in_16bit()
# define ASSERT32SEG() __force_link_error__only_in_32bit_segmented()
# define ASSERT32FLAT() చేయి { } అయితే (0)
#అయితే_ముగింపు

#నిర్వచించు offsetof(TYPE, MEMBER) ((కొలత_t) &((TYPE *)0)->MEMBER)
#నిర్వచించు ARRAY_కొలత(a) (పరిమాణం(a) / పరిమాణం(a[0]))
#నిర్వచించు క్షేత్రం_SIZEOF(t, f) (పరిమాణం(((t*)0)->f))
#నిర్వచించు DIV_విడత_పైకి(n,d) (((n) + (d) - 1) / (d))
#నిర్వచించు DIV_విడత_CLOSEST(x, భాజకం)({                 \
            typeof(భాజకం) __divisor = భాజకం;        \
            (((x) + ((__divisor) / 2)) / (__divisor));  \
        })
#నిర్వచించు ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#నిర్వచించు __ALIGN_MASK(x,ముసుగు)    (((x)+(ముసుగు))&~(ముసుగు))
#నిర్వచించు ALIGN_కిందికి(x,a)         ((x) & ~((typeof(x))(a)-1))
#నిర్వచించు container_యొక్క(ptr, type, member) ({                      \
        స్థిరరాశి typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (అక్షరం *)__mptr - offsetof(type,member) );})
#నిర్వచించు container_యొక్క_or_null(ptr, type, member) ({              \
        స్థిరరాశి typeof( ((type *)0)->member ) *___mptr = (ptr);   \
        ___mptr ? container_యొక్క(___mptr, type, member) : ఖాళీ; })

#నిర్వచించు likely(x)       __builtin_expect(!!(x), 1)
#నిర్వచించు unlikely(x)     __builtin_expect(!!(x), 0)

#నిర్వచించు ఖాళీ ((శూన్య*)0)

#నిర్వచించు __weak __attribute__((weak))
#నిర్వచించు __section(S) __attribute__((section(S)))

#నిర్వచించు PACKED __attribute__((packed))
#నిర్వచించు __aligned(x) __attribute__((aligned(x)))

#నిర్వచించు barrier() __asm__ __volatile__("": : :"memory")

#నిర్వచించు noinline __attribute__((noinline))
#నిర్వచించు __always_inline అంతఃవరుస __attribute__((always_inline))
#నిర్వచించు __malloc __attribute__((__malloc__))
#నిర్వచించు __attribute_const __attribute__((__const__))

#నిర్వచించు __stringify_1(x)        #x
#నిర్వచించు __stringify(x)          __stringify_1(x)

#అయితే_ముగింపు // types.h
