// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __FARPTR_H
#define __FARPTR_H

#અન્તર્ભાવ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
બાહ્ય u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
બાહ્ય u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#define READ8_SEG(prefix, SEG, કિંમત, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(કિંમત)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ16_SEG(prefix, SEG, કિંમત, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(કિંમત)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ32_SEG(prefix, SEG, કિંમત, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(કિંમત)      \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ64_SEG(prefix, SEG, કિંમત, var) કરો {                \
        સંઘ u64_u32_u __value;                                \
        સંઘ u64_u32_u *__r64_ptr = (સંઘ u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(કિંમત) = __value.val;                          \
    } જ્યારે (0)
#define WRITE8_SEG(prefix, SEG, var, કિંમત)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(કિંમત), "m"(__segment_ ## SEG))
#define WRITE16_SEG(prefix, SEG, var, કિંમત)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(કિંમત), "m"(__segment_ ## SEG))
#define WRITE32_SEG(prefix, SEG, var, કિંમત)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(કિંમત), "m"(__segment_ ## SEG))
#define WRITE64_SEG(prefix, SEG, var, કિંમત) કરો {               \
        સંઘ u64_u32_u __value;                                \
        સંઘ u64_u32_u *__w64_ptr = (સંઘ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (કિંમત);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } જ્યારે (0)

// Macros for automatically choosing the appropriate memory size
// access method.
બાહ્ય શૂન્ય __force_link_error__unknown_type(શૂન્ય);

#define __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    જો (કદ(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    નહીંતર જો (કદ(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    નહીંતર જો (કદ(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    નહીંતર જો (કદ(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    નહીંતર                                        \
        __force_link_error__unknown_type();     \
    __val; })

#define __SET_VAR(prefix, seg, var, val) કરો {           \
        જો (કદ(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        નહીંતર જો (કદ(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        નહીંતર જો (કદ(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        નહીંતર જો (કદ(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        નહીંતર                                            \
            __force_link_error__unknown_type();         \
    } જ્યારે (0)

#define DECL_SEGFUNCS(SEG)                              \
સ્થિર અંતઃરેખ શૂન્ય __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
સ્થિર અંતઃરેખ u16 __get_seg_##SEG(શૂન્ય) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    પાછા res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, કિંમત)                   \
    __set_seg_##SEG(કિંમત)
#define __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#define __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#define __SET_FARVAR(seg, var, val) કરો {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } જ્યારે (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#define __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#define __SET_FLATPTR(ptr, val) કરો {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } જ્યારે (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#define FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#define FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#define MAKE_FLATPTR(seg,off) ((શૂન્ય*)(((u32)(seg)<<4)+(u32)(off)))


#જો MODESEGMENT == 1

// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, કિંમત) __SET_SEG(SEG, (કિંમત))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

સ્થિર અંતઃરેખ શૂન્ય insb_fl(u16 port, શૂન્ય *ptr_fl, u16 ગણતરી) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), ગણતરી);
}
સ્થિર અંતઃરેખ શૂન્ય insw_fl(u16 port, શૂન્ય *ptr_fl, u16 ગણતરી) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), ગણતરી);
}
સ્થિર અંતઃરેખ શૂન્ય insl_fl(u16 port, શૂન્ય *ptr_fl, u16 ગણતરી) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), ગણતરી);
}
સ્થિર અંતઃરેખ શૂન્ય outsb_fl(u16 port, શૂન્ય *ptr_fl, u16 ગણતરી) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), ગણતરી);
}
સ્થિર અંતઃરેખ શૂન્ય outsw_fl(u16 port, શૂન્ય *ptr_fl, u16 ગણતરી) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), ગણતરી);
}
સ્થિર અંતઃરેખ શૂન્ય outsl_fl(u16 port, શૂન્ય *ptr_fl, u16 ગણતરી) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), ગણતરી);
}

#નહીંતર

// In 32-bit flat mode there is no need to mess with the segments.
#define GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#define SET_FARVAR(seg, var, val) \
    કરો { GET_FARVAR((seg), (var)) = (val); } જ્યારે (0)
#define GET_VAR(seg, var) (var)
#define SET_VAR(seg, var, val) કરો { (var) = (val); } જ્યારે (0)
#define SET_SEG(SEG, કિંમત) ((શૂન્ય)(કિંમત))
#define GET_SEG(SEG) 0
#define GET_FLATPTR(ptr) (ptr)
#define SET_FLATPTR(ptr, val) કરો { (ptr) = (val); } જ્યારે (0)

#define insb_fl(port, ptr_fl, ગણતરી) insb(port, ptr_fl, ગણતરી)
#define insw_fl(port, ptr_fl, ગણતરી) insw(port, ptr_fl, ગણતરી)
#define insl_fl(port, ptr_fl, ગણતરી) insl(port, ptr_fl, ગણતરી)
#define outsb_fl(port, ptr_fl, ગણતરી) outsb(port, ptr_fl, ગણતરી)
#define outsw_fl(port, ptr_fl, ગણતરી) outsw(port, ptr_fl, ગણતરી)
#define outsl_fl(port, ptr_fl, ગણતરી) outsl(port, ptr_fl, ગણતરી)

#endif

#define SEGOFF(s,o) ({સંરચના segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

સ્થિર અંતઃરેખ સંરચના segoff_s FLATPTR_TO_SEGOFF(શૂન્ય *p) {
    પાછા SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
સ્થિર અંતઃરેખ શૂન્ય *SEGOFF_TO_FLATPTR(સંરચના segoff_s so) {
    પાછા MAKE_FLATPTR(so.seg, so.offset);
}

#endif // farptr.h
