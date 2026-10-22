// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __FARPTR_H
#define __FARPTR_H

#شامل "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
بیرونی u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
بیرونی u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#define READ8_SEG(prefix, SEG, مقدار, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(مقدار)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ16_SEG(prefix, SEG, مقدار, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(مقدار)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ32_SEG(prefix, SEG, مقدار, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(مقدار)      \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ64_SEG(prefix, SEG, مقدار, var) انجامبده {                \
        اجتماع u64_u32_u __value;                                \
        اجتماع u64_u32_u *__r64_ptr = (اجتماع u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(مقدار) = __value.val;                          \
    } وقتی (0)
#define WRITE8_SEG(prefix, SEG, var, مقدار)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(مقدار), "m"(__segment_ ## SEG))
#define WRITE16_SEG(prefix, SEG, var, مقدار)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(مقدار), "m"(__segment_ ## SEG))
#define WRITE32_SEG(prefix, SEG, var, مقدار)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(مقدار), "m"(__segment_ ## SEG))
#define WRITE64_SEG(prefix, SEG, var, مقدار) انجامبده {               \
        اجتماع u64_u32_u __value;                                \
        اجتماع u64_u32_u *__w64_ptr = (اجتماع u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (مقدار);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } وقتی (0)

// Macros for automatically choosing the appropriate memory size
// access method.
بیرونی تهی __force_link_error__unknown_type(تهی);

#define __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    اگر (اندازه(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    وگرنه اگر (اندازه(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    وگرنه اگر (اندازه(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    وگرنه اگر (اندازه(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    وگرنه                                        \
        __force_link_error__unknown_type();     \
    __val; })

#define __SET_VAR(prefix, seg, var, val) انجامبده {           \
        اگر (اندازه(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        وگرنه اگر (اندازه(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        وگرنه اگر (اندازه(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        وگرنه اگر (اندازه(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        وگرنه                                            \
            __force_link_error__unknown_type();         \
    } وقتی (0)

#define DECL_SEGFUNCS(SEG)                              \
ایستا درونخطی تهی __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
ایستا درونخطی u16 __get_seg_##SEG(تهی) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    بازگشت res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, مقدار)                   \
    __set_seg_##SEG(مقدار)
#define __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#define __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#define __SET_FARVAR(seg, var, val) انجامبده {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } وقتی (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#define __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#define __SET_FLATPTR(ptr, val) انجامبده {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } وقتی (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#define FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#define FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#define MAKE_FLATPTR(seg,off) ((تهی*)(((u32)(seg)<<4)+(u32)(off)))


#اگر MODESEGMENT == 1

// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, مقدار) __SET_SEG(SEG, (مقدار))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ایستا درونخطی تهی insb_fl(u16 port, تهی *ptr_fl, u16 شمار) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), شمار);
}
ایستا درونخطی تهی insw_fl(u16 port, تهی *ptr_fl, u16 شمار) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), شمار);
}
ایستا درونخطی تهی insl_fl(u16 port, تهی *ptr_fl, u16 شمار) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), شمار);
}
ایستا درونخطی تهی outsb_fl(u16 port, تهی *ptr_fl, u16 شمار) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), شمار);
}
ایستا درونخطی تهی outsw_fl(u16 port, تهی *ptr_fl, u16 شمار) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), شمار);
}
ایستا درونخطی تهی outsl_fl(u16 port, تهی *ptr_fl, u16 شمار) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), شمار);
}

#وگرنه

// In 32-bit flat mode there is no need to mess with the segments.
#define GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#define SET_FARVAR(seg, var, val) \
    انجامبده { GET_FARVAR((seg), (var)) = (val); } وقتی (0)
#define GET_VAR(seg, var) (var)
#define SET_VAR(seg, var, val) انجامبده { (var) = (val); } وقتی (0)
#define SET_SEG(SEG, مقدار) ((تهی)(مقدار))
#define GET_SEG(SEG) 0
#define GET_FLATPTR(ptr) (ptr)
#define SET_FLATPTR(ptr, val) انجامبده { (ptr) = (val); } وقتی (0)

#define insb_fl(port, ptr_fl, شمار) insb(port, ptr_fl, شمار)
#define insw_fl(port, ptr_fl, شمار) insw(port, ptr_fl, شمار)
#define insl_fl(port, ptr_fl, شمار) insl(port, ptr_fl, شمار)
#define outsb_fl(port, ptr_fl, شمار) outsb(port, ptr_fl, شمار)
#define outsw_fl(port, ptr_fl, شمار) outsw(port, ptr_fl, شمار)
#define outsl_fl(port, ptr_fl, شمار) outsl(port, ptr_fl, شمار)

#endif

#define SEGOFF(s,o) ({ساختار segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

ایستا درونخطی ساختار segoff_s FLATPTR_TO_SEGOFF(تهی *p) {
    بازگشت SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
ایستا درونخطی تهی *SEGOFF_TO_FLATPTR(ساختار segoff_s so) {
    بازگشت MAKE_FLATPTR(so.seg, so.offset);
}

#endif // farptr.h
