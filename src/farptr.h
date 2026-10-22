// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __FARPTR_H
#define __FARPTR_H

#अन्तर्भाव "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
बाह्य u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
बाह्य u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#define READ8_SEG(prefix, SEG, मूल्य, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(मूल्य)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ16_SEG(prefix, SEG, मूल्य, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(मूल्य)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ32_SEG(prefix, SEG, मूल्य, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(मूल्य)      \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ64_SEG(prefix, SEG, मूल्य, var) कुरु {                \
        सङ्घ u64_u32_u __value;                                \
        सङ्घ u64_u32_u *__r64_ptr = (सङ्घ u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(मूल्य) = __value.val;                          \
    } यावत् (0)
#define WRITE8_SEG(prefix, SEG, var, मूल्य)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(मूल्य), "m"(__segment_ ## SEG))
#define WRITE16_SEG(prefix, SEG, var, मूल्य)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(मूल्य), "m"(__segment_ ## SEG))
#define WRITE32_SEG(prefix, SEG, var, मूल्य)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(मूल्य), "m"(__segment_ ## SEG))
#define WRITE64_SEG(prefix, SEG, var, मूल्य) कुरु {               \
        सङ्घ u64_u32_u __value;                                \
        सङ्घ u64_u32_u *__w64_ptr = (सङ्घ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (मूल्य);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } यावत् (0)

// Macros for automatically choosing the appropriate memory size
// access method.
बाह्य शून्य __force_link_error__unknown_type(शून्य);

#define __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    यदि (परिमाण(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    अन्यथा यदि (परिमाण(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    अन्यथा यदि (परिमाण(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    अन्यथा यदि (परिमाण(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    अन्यथा                                        \
        __force_link_error__unknown_type();     \
    __val; })

#define __SET_VAR(prefix, seg, var, val) कुरु {           \
        यदि (परिमाण(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        अन्यथा यदि (परिमाण(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        अन्यथा यदि (परिमाण(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        अन्यथा यदि (परिमाण(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        अन्यथा                                            \
            __force_link_error__unknown_type();         \
    } यावत् (0)

#define DECL_SEGFUNCS(SEG)                              \
स्थिर अन्तःस्थ शून्य __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
स्थिर अन्तःस्थ u16 __get_seg_##SEG(शून्य) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    प्रत्यागम res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, मूल्य)                   \
    __set_seg_##SEG(मूल्य)
#define __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#define __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#define __SET_FARVAR(seg, var, val) कुरु {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } यावत् (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#define __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#define __SET_FLATPTR(ptr, val) कुरु {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } यावत् (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#define FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#define FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#define MAKE_FLATPTR(seg,off) ((शून्य*)(((u32)(seg)<<4)+(u32)(off)))


#यदि MODESEGMENT == 1

// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, मूल्य) __SET_SEG(SEG, (मूल्य))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

स्थिर अन्तःस्थ शून्य insb_fl(u16 port, शून्य *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
स्थिर अन्तःस्थ शून्य insw_fl(u16 port, शून्य *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
स्थिर अन्तःस्थ शून्य insl_fl(u16 port, शून्य *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
स्थिर अन्तःस्थ शून्य outsb_fl(u16 port, शून्य *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
स्थिर अन्तःस्थ शून्य outsw_fl(u16 port, शून्य *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
स्थिर अन्तःस्थ शून्य outsl_fl(u16 port, शून्य *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}

#अन्यथा

// In 32-bit flat mode there is no need to mess with the segments.
#define GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#define SET_FARVAR(seg, var, val) \
    कुरु { GET_FARVAR((seg), (var)) = (val); } यावत् (0)
#define GET_VAR(seg, var) (var)
#define SET_VAR(seg, var, val) कुरु { (var) = (val); } यावत् (0)
#define SET_SEG(SEG, मूल्य) ((शून्य)(मूल्य))
#define GET_SEG(SEG) 0
#define GET_FLATPTR(ptr) (ptr)
#define SET_FLATPTR(ptr, val) कुरु { (ptr) = (val); } यावत् (0)

#define insb_fl(port, ptr_fl, गिनती) insb(port, ptr_fl, गिनती)
#define insw_fl(port, ptr_fl, गिनती) insw(port, ptr_fl, गिनती)
#define insl_fl(port, ptr_fl, गिनती) insl(port, ptr_fl, गिनती)
#define outsb_fl(port, ptr_fl, गिनती) outsb(port, ptr_fl, गिनती)
#define outsw_fl(port, ptr_fl, गिनती) outsw(port, ptr_fl, गिनती)
#define outsl_fl(port, ptr_fl, गिनती) outsl(port, ptr_fl, गिनती)

#endif

#define SEGOFF(s,o) ({संरचना segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

स्थिर अन्तःस्थ संरचना segoff_s FLATPTR_TO_SEGOFF(शून्य *p) {
    प्रत्यागम SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
स्थिर अन्तःस्थ शून्य *SEGOFF_TO_FLATPTR(संरचना segoff_s so) {
    प्रत्यागम MAKE_FLATPTR(so.seg, so.offset);
}

#endif // farptr.h
