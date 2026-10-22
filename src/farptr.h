// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __FARPTR_H
#define __FARPTR_H

#ଅନ୍ତର୍ଭାଵ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
ବାହ୍ୟ u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
ବାହ୍ୟ u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#define READ8_SEG(prefix, SEG, ମୂଲ୍ୟ, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(ମୂଲ୍ୟ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ16_SEG(prefix, SEG, ମୂଲ୍ୟ, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(ମୂଲ୍ୟ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ32_SEG(prefix, SEG, ମୂଲ୍ୟ, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(ମୂଲ୍ୟ)      \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ64_SEG(prefix, SEG, ମୂଲ୍ୟ, var) କର {                \
        ସଂଘ u64_u32_u __value;                                \
        ସଂଘ u64_u32_u *__r64_ptr = (ସଂଘ u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(ମୂଲ୍ୟ) = __value.val;                          \
    } ଯେତେବେଳେ (0)
#define WRITE8_SEG(prefix, SEG, var, ମୂଲ୍ୟ)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(ମୂଲ୍ୟ), "m"(__segment_ ## SEG))
#define WRITE16_SEG(prefix, SEG, var, ମୂଲ୍ୟ)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(ମୂଲ୍ୟ), "m"(__segment_ ## SEG))
#define WRITE32_SEG(prefix, SEG, var, ମୂଲ୍ୟ)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(ମୂଲ୍ୟ), "m"(__segment_ ## SEG))
#define WRITE64_SEG(prefix, SEG, var, ମୂଲ୍ୟ) କର {               \
        ସଂଘ u64_u32_u __value;                                \
        ସଂଘ u64_u32_u *__w64_ptr = (ସଂଘ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (ମୂଲ୍ୟ);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } ଯେତେବେଳେ (0)

// Macros for automatically choosing the appropriate memory size
// access method.
ବାହ୍ୟ ଶୂନ୍ୟ __force_link_error__unknown_type(ଶୂନ୍ୟ);

#define __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    ଯଦି (ଆକାର(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    ଅନ୍ୟଥା ଯଦି (ଆକାର(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    ଅନ୍ୟଥା ଯଦି (ଆକାର(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    ଅନ୍ୟଥା ଯଦି (ଆକାର(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    ଅନ୍ୟଥା                                        \
        __force_link_error__unknown_type();     \
    __val; })

#define __SET_VAR(prefix, seg, var, val) କର {           \
        ଯଦି (ଆକାର(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        ଅନ୍ୟଥା ଯଦି (ଆକାର(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        ଅନ୍ୟଥା ଯଦି (ଆକାର(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        ଅନ୍ୟଥା ଯଦି (ଆକାର(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        ଅନ୍ୟଥା                                            \
            __force_link_error__unknown_type();         \
    } ଯେତେବେଳେ (0)

#define DECL_SEGFUNCS(SEG)                              \
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 __get_seg_##SEG(ଶୂନ୍ୟ) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    ଫେରନ୍ତୁ res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, ମୂଲ୍ୟ)                   \
    __set_seg_##SEG(ମୂଲ୍ୟ)
#define __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#define __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#define __SET_FARVAR(seg, var, val) କର {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } ଯେତେବେଳେ (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#define __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#define __SET_FLATPTR(ptr, val) କର {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } ଯେତେବେଳେ (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#define FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#define FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#define MAKE_FLATPTR(seg,off) ((ଶୂନ୍ୟ*)(((u32)(seg)<<4)+(u32)(off)))


#ଯଦି MODESEGMENT == 1

// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, ମୂଲ୍ୟ) __SET_SEG(SEG, (ମୂଲ୍ୟ))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ insb_fl(u16 port, ଶୂନ୍ୟ *ptr_fl, u16 ଗଣନା_2) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), ଗଣନା_2);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ insw_fl(u16 port, ଶୂନ୍ୟ *ptr_fl, u16 ଗଣନା_2) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), ଗଣନା_2);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ insl_fl(u16 port, ଶୂନ୍ୟ *ptr_fl, u16 ଗଣନା_2) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), ଗଣନା_2);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outsb_fl(u16 port, ଶୂନ୍ୟ *ptr_fl, u16 ଗଣନା_2) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), ଗଣନା_2);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outsw_fl(u16 port, ଶୂନ୍ୟ *ptr_fl, u16 ଗଣନା_2) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), ଗଣନା_2);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outsl_fl(u16 port, ଶୂନ୍ୟ *ptr_fl, u16 ଗଣନା_2) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), ଗଣନା_2);
}

#ଅନ୍ୟଥା

// In 32-bit flat mode there is no need to mess with the segments.
#define GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#define SET_FARVAR(seg, var, val) \
    କର { GET_FARVAR((seg), (var)) = (val); } ଯେତେବେଳେ (0)
#define GET_VAR(seg, var) (var)
#define SET_VAR(seg, var, val) କର { (var) = (val); } ଯେତେବେଳେ (0)
#define SET_SEG(SEG, ମୂଲ୍ୟ) ((ଶୂନ୍ୟ)(ମୂଲ୍ୟ))
#define GET_SEG(SEG) 0
#define GET_FLATPTR(ptr) (ptr)
#define SET_FLATPTR(ptr, val) କର { (ptr) = (val); } ଯେତେବେଳେ (0)

#define insb_fl(port, ptr_fl, ଗଣନା_2) insb(port, ptr_fl, ଗଣନା_2)
#define insw_fl(port, ptr_fl, ଗଣନା_2) insw(port, ptr_fl, ଗଣନା_2)
#define insl_fl(port, ptr_fl, ଗଣନା_2) insl(port, ptr_fl, ଗଣନା_2)
#define outsb_fl(port, ptr_fl, ଗଣନା_2) outsb(port, ptr_fl, ଗଣନା_2)
#define outsw_fl(port, ptr_fl, ଗଣନା_2) outsw(port, ptr_fl, ଗଣନା_2)
#define outsl_fl(port, ptr_fl, ଗଣନା_2) outsl(port, ptr_fl, ଗଣନା_2)

#endif

#define SEGOFF(s,o) ({ସଂରଚନା segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା segoff_s FLATPTR_TO_SEGOFF(ଶୂନ୍ୟ *p) {
    ଫେରନ୍ତୁ SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *SEGOFF_TO_FLATPTR(ସଂରଚନା segoff_s so) {
    ଫେରନ୍ତୁ MAKE_FLATPTR(so.seg, so.offset);
}

#endif // farptr.h
