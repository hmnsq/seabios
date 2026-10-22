// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#اگر_غیر_تعریف_شدہ __FARPTR_H
#تعریف_کرو __FARPTR_H

#شامل "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
خارجی u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
خارجی u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#تعریف_کرو READ8_SEG(سابقہ, SEG, قدر, var)                      \
    __asm__(سابقہ "movb %%" #SEG ":%1, %b0" : "=Qi"(قدر)     \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کرو READ16_SEG(سابقہ, SEG, قدر, var)                     \
    __asm__(سابقہ "movw %%" #SEG ":%1, %w0" : "=ri"(قدر)     \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کرو READ32_SEG(سابقہ, SEG, قدر, var)                     \
    __asm__(سابقہ "movl %%" #SEG ":%1, %0" : "=ri"(قدر)      \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کرو READ64_SEG(سابقہ, SEG, قدر, var) کرو {                \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__r64_ptr = (اتحاد u64_u32_u *)&(var); \
        READ32_SEG(سابقہ, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(سابقہ, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(قدر) = __value.val;                          \
    } جب (0)
#تعریف_کرو WRITE8_SEG(سابقہ, SEG, var, قدر)                     \
    __asm__(سابقہ "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(قدر), "m"(__segment_ ## SEG))
#تعریف_کرو WRITE16_SEG(سابقہ, SEG, var, قدر)                    \
    __asm__(سابقہ "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(قدر), "m"(__segment_ ## SEG))
#تعریف_کرو WRITE32_SEG(سابقہ, SEG, var, قدر)                    \
    __asm__(سابقہ "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(قدر), "m"(__segment_ ## SEG))
#تعریف_کرو WRITE64_SEG(سابقہ, SEG, var, قدر) کرو {               \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__w64_ptr = (اتحاد u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (قدر);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(سابقہ, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(سابقہ, SEG, __w64_ptr->hi, __value.hi);    \
    } جب (0)

// Macros for automatically choosing the appropriate memory size
// access method.
خارجی خالی __force_link_error__unknown_type(خالی);

#تعریف_کرو __GET_VAR(سابقہ, seg, var) ({          \
    typeof(var) __val;                          \
    اگر (حجم(__val) == 1)                     \
        READ8_SEG(سابقہ, seg, __val, var);     \
    ورنہ اگر (حجم(__val) == 2)                \
        READ16_SEG(سابقہ, seg, __val, var);    \
    ورنہ اگر (حجم(__val) == 4)                \
        READ32_SEG(سابقہ, seg, __val, var);    \
    ورنہ اگر (حجم(__val) == 8)                \
        READ64_SEG(سابقہ, seg, __val, var);    \
    ورنہ                                        \
        __force_link_error__unknown_type();     \
    __val; })

#تعریف_کرو __SET_VAR(سابقہ, seg, var, val) کرو {           \
        اگر (حجم(var) == 1)                           \
            WRITE8_SEG(سابقہ, seg, var, (val));        \
        ورنہ اگر (حجم(var) == 2)                      \
            WRITE16_SEG(سابقہ, seg, var, (val));       \
        ورنہ اگر (حجم(var) == 4)                      \
            WRITE32_SEG(سابقہ, seg, var, (val));       \
        ورنہ اگر (حجم(var) == 8)                      \
            WRITE64_SEG(سابقہ, seg, var, (val));       \
        ورنہ                                            \
            __force_link_error__unknown_type();         \
    } جب (0)

#تعریف_کرو DECL_SEGFUNCS(SEG)                              \
ساکن اندرونی خالی __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
ساکن اندرونی u16 __get_seg_##SEG(خالی) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    واپس res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#تعریف_کرو __SET_SEG(SEG, قدر)                   \
    __set_seg_##SEG(قدر)
#تعریف_کرو __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#تعریف_کرو __GET_FARVAR(seg, var) ({               \
    سیٹ_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#تعریف_کرو __SET_FARVAR(seg, var, val) کرو {        \
        typeof(var) __sfv_val = (val);          \
        سیٹ_SEG(ES, (seg));                     \
        سیٹ_VAR(ES, (var), __sfv_val);          \
    } جب (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#تعریف_کرو __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_مقصد_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_مقصد_انحراف(__ptr)); })
#تعریف_کرو __SET_FLATPTR(ptr, val) کرو {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        سیٹ_FARVAR(FLATPTR_مقصد_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_مقصد_انحراف(__ptr)   \
                   , (val));                                    \
    } جب (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#تعریف_کرو FLATPTR_مقصد_SEG(p) (((u32)(p)) >> 4)
#تعریف_کرو FLATPTR_مقصد_انحراف(p) (((u32)(p)) & 0xf)
#تعریف_کرو MAKE_FLATPTR(seg,off) ((خالی*)(((u32)(seg)<<4)+(u32)(off)))


#اگر MODESEGMENT == 1

// Definitions when using segmented mode.
#تعریف_کرو GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#تعریف_کرو سیٹ_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#تعریف_کرو GET_VAR(seg, var) __GET_VAR("", seg, (var))
#تعریف_کرو سیٹ_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#تعریف_کرو سیٹ_SEG(SEG, قدر) __SET_SEG(SEG, (قدر))
#تعریف_کرو GET_SEG(SEG) __GET_SEG(SEG)
#تعریف_کرو GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#تعریف_کرو سیٹ_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ساکن اندرونی خالی insb_fl(u16 پورٹ, خالی *ptr_fl, u16 گنتی) {
    سیٹ_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    insb(پورٹ, (u8*)FLATPTR_مقصد_انحراف(ptr_fl), گنتی);
}
ساکن اندرونی خالی insw_fl(u16 پورٹ, خالی *ptr_fl, u16 گنتی) {
    سیٹ_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    insw(پورٹ, (u16*)FLATPTR_مقصد_انحراف(ptr_fl), گنتی);
}
ساکن اندرونی خالی insl_fl(u16 پورٹ, خالی *ptr_fl, u16 گنتی) {
    سیٹ_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    insl(پورٹ, (u32*)FLATPTR_مقصد_انحراف(ptr_fl), گنتی);
}
ساکن اندرونی خالی outsb_fl(u16 پورٹ, خالی *ptr_fl, u16 گنتی) {
    سیٹ_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    outsb(پورٹ, (u8*)FLATPTR_مقصد_انحراف(ptr_fl), گنتی);
}
ساکن اندرونی خالی outsw_fl(u16 پورٹ, خالی *ptr_fl, u16 گنتی) {
    سیٹ_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    outsw(پورٹ, (u16*)FLATPTR_مقصد_انحراف(ptr_fl), گنتی);
}
ساکن اندرونی خالی outsl_fl(u16 پورٹ, خالی *ptr_fl, u16 گنتی) {
    سیٹ_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    outsl(پورٹ, (u32*)FLATPTR_مقصد_انحراف(ptr_fl), گنتی);
}

#ورنہ

// In 32-bit flat mode there is no need to mess with the segments.
#تعریف_کرو GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#تعریف_کرو سیٹ_FARVAR(seg, var, val) \
    کرو { GET_FARVAR((seg), (var)) = (val); } جب (0)
#تعریف_کرو GET_VAR(seg, var) (var)
#تعریف_کرو سیٹ_VAR(seg, var, val) کرو { (var) = (val); } جب (0)
#تعریف_کرو سیٹ_SEG(SEG, قدر) ((خالی)(قدر))
#تعریف_کرو GET_SEG(SEG) 0
#تعریف_کرو GET_FLATPTR(ptr) (ptr)
#تعریف_کرو سیٹ_FLATPTR(ptr, val) کرو { (ptr) = (val); } جب (0)

#تعریف_کرو insb_fl(پورٹ, ptr_fl, گنتی) insb(پورٹ, ptr_fl, گنتی)
#تعریف_کرو insw_fl(پورٹ, ptr_fl, گنتی) insw(پورٹ, ptr_fl, گنتی)
#تعریف_کرو insl_fl(پورٹ, ptr_fl, گنتی) insl(پورٹ, ptr_fl, گنتی)
#تعریف_کرو outsb_fl(پورٹ, ptr_fl, گنتی) outsb(پورٹ, ptr_fl, گنتی)
#تعریف_کرو outsw_fl(پورٹ, ptr_fl, گنتی) outsw(پورٹ, ptr_fl, گنتی)
#تعریف_کرو outsl_fl(پورٹ, ptr_fl, گنتی) outsl(پورٹ, ptr_fl, گنتی)

#اگر_ختم

#تعریف_کرو SEGOFF(s,o) ({ساخت segoff_s __so; __so.انحراف_2=(o); __so.seg=(s); __so;})

ساکن اندرونی ساخت segoff_s FLATPTR_مقصد_SEGOFF(خالی *p) {
    واپس SEGOFF(FLATPTR_مقصد_SEG(p), FLATPTR_مقصد_انحراف(p));
}
ساکن اندرونی خالی *SEGOFF_مقصد_FLATPTR(ساخت segoff_s so) {
    واپس MAKE_FLATPTR(so.seg, so.انحراف_2);
}

#اگر_ختم // farptr.h
