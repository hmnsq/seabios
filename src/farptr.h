// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#اگر_تعریف_نشده __FARPTR_H
#تعریف_کن __FARPTR_H

#ضمن "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
خارجي u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
خارجي u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#تعریف_کن READ8_SEG(پیشوند, SEG, مقدار, var)                      \
    __asm__(پیشوند "movb %%" #SEG ":%1, %b0" : "=Qi"(مقدار)     \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کن READ16_SEG(پیشوند, SEG, مقدار, var)                     \
    __asm__(پیشوند "movw %%" #SEG ":%1, %w0" : "=ri"(مقدار)     \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کن READ32_SEG(پیشوند, SEG, مقدار, var)                     \
    __asm__(پیشوند "movl %%" #SEG ":%1, %0" : "=ri"(مقدار)      \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کن READ64_SEG(پیشوند, SEG, مقدار, var) افعل {                \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__r64_ptr = (اتحاد u64_u32_u *)&(var); \
        READ32_SEG(پیشوند, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(پیشوند, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(مقدار) = __value.val;                          \
    } طالما (0)
#تعریف_کن WRITE8_SEG(پیشوند, SEG, var, مقدار)                     \
    __asm__(پیشوند "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(مقدار), "m"(__segment_ ## SEG))
#تعریف_کن WRITE16_SEG(پیشوند, SEG, var, مقدار)                    \
    __asm__(پیشوند "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(مقدار), "m"(__segment_ ## SEG))
#تعریف_کن WRITE32_SEG(پیشوند, SEG, var, مقدار)                    \
    __asm__(پیشوند "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(مقدار), "m"(__segment_ ## SEG))
#تعریف_کن WRITE64_SEG(پیشوند, SEG, var, مقدار) افعل {               \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__w64_ptr = (اتحاد u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (مقدار);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(پیشوند, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(پیشوند, SEG, __w64_ptr->hi, __value.hi);    \
    } طالما (0)

// Macros for automatically choosing the appropriate memory size
// access method.
خارجي فراغ __force_link_error__unknown_type(فراغ);

#تعریف_کن __GET_VAR(پیشوند, seg, var) ({          \
    typeof(var) __val;                          \
    إذا (حجم(__val) == 1)                     \
        READ8_SEG(پیشوند, seg, __val, var);     \
    وإلا إذا (حجم(__val) == 2)                \
        READ16_SEG(پیشوند, seg, __val, var);    \
    وإلا إذا (حجم(__val) == 4)                \
        READ32_SEG(پیشوند, seg, __val, var);    \
    وإلا إذا (حجم(__val) == 8)                \
        READ64_SEG(پیشوند, seg, __val, var);    \
    وإلا                                        \
        __force_link_error__unknown_type();     \
    __val; })

#تعریف_کن __SET_VAR(پیشوند, seg, var, val) افعل {           \
        إذا (حجم(var) == 1)                           \
            WRITE8_SEG(پیشوند, seg, var, (val));        \
        وإلا إذا (حجم(var) == 2)                      \
            WRITE16_SEG(پیشوند, seg, var, (val));       \
        وإلا إذا (حجم(var) == 4)                      \
            WRITE32_SEG(پیشوند, seg, var, (val));       \
        وإلا إذا (حجم(var) == 8)                      \
            WRITE64_SEG(پیشوند, seg, var, (val));       \
        وإلا                                            \
            __force_link_error__unknown_type();         \
    } طالما (0)

#تعریف_کن DECL_SEGFUNCS(SEG)                              \
ساكن مضمن فراغ __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
ساكن مضمن u16 __get_seg_##SEG(فراغ) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    أرجع res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#تعریف_کن __SET_SEG(SEG, مقدار)                   \
    __set_seg_##SEG(مقدار)
#تعریف_کن __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#تعریف_کن __GET_FARVAR(seg, var) ({               \
    مجموعه_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#تعریف_کن __SET_FARVAR(seg, var, val) افعل {        \
        typeof(var) __sfv_val = (val);          \
        مجموعه_SEG(ES, (seg));                     \
        مجموعه_VAR(ES, (var), __sfv_val);          \
    } طالما (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#تعریف_کن __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_مقصد_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_مقصد_جابه_جایی(__ptr)); })
#تعریف_کن __SET_FLATPTR(ptr, val) افعل {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        مجموعه_FARVAR(FLATPTR_مقصد_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_مقصد_جابه_جایی(__ptr)   \
                   , (val));                                    \
    } طالما (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#تعریف_کن FLATPTR_مقصد_SEG(p) (((u32)(p)) >> 4)
#تعریف_کن FLATPTR_مقصد_جابه_جایی(p) (((u32)(p)) & 0xf)
#تعریف_کن MAKE_FLATPTR(seg,off) ((فراغ*)(((u32)(seg)<<4)+(u32)(off)))


#إذا MODESEGMENT == 1

// Definitions when using segmented mode.
#تعریف_کن GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#تعریف_کن مجموعه_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#تعریف_کن GET_VAR(seg, var) __GET_VAR("", seg, (var))
#تعریف_کن مجموعه_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#تعریف_کن مجموعه_SEG(SEG, مقدار) __SET_SEG(SEG, (مقدار))
#تعریف_کن GET_SEG(SEG) __GET_SEG(SEG)
#تعریف_کن GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#تعریف_کن مجموعه_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ساكن مضمن فراغ insb_fl(u16 پورت, فراغ *ptr_fl, u16 شمار) {
    مجموعه_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    insb(پورت, (u8*)FLATPTR_مقصد_جابه_جایی(ptr_fl), شمار);
}
ساكن مضمن فراغ insw_fl(u16 پورت, فراغ *ptr_fl, u16 شمار) {
    مجموعه_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    insw(پورت, (u16*)FLATPTR_مقصد_جابه_جایی(ptr_fl), شمار);
}
ساكن مضمن فراغ insl_fl(u16 پورت, فراغ *ptr_fl, u16 شمار) {
    مجموعه_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    insl(پورت, (u32*)FLATPTR_مقصد_جابه_جایی(ptr_fl), شمار);
}
ساكن مضمن فراغ outsb_fl(u16 پورت, فراغ *ptr_fl, u16 شمار) {
    مجموعه_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    outsb(پورت, (u8*)FLATPTR_مقصد_جابه_جایی(ptr_fl), شمار);
}
ساكن مضمن فراغ outsw_fl(u16 پورت, فراغ *ptr_fl, u16 شمار) {
    مجموعه_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    outsw(پورت, (u16*)FLATPTR_مقصد_جابه_جایی(ptr_fl), شمار);
}
ساكن مضمن فراغ outsl_fl(u16 پورت, فراغ *ptr_fl, u16 شمار) {
    مجموعه_SEG(ES, FLATPTR_مقصد_SEG(ptr_fl));
    outsl(پورت, (u32*)FLATPTR_مقصد_جابه_جایی(ptr_fl), شمار);
}

#وإلا

// In 32-bit flat mode there is no need to mess with the segments.
#تعریف_کن GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#تعریف_کن مجموعه_FARVAR(seg, var, val) \
    افعل { GET_FARVAR((seg), (var)) = (val); } طالما (0)
#تعریف_کن GET_VAR(seg, var) (var)
#تعریف_کن مجموعه_VAR(seg, var, val) افعل { (var) = (val); } طالما (0)
#تعریف_کن مجموعه_SEG(SEG, مقدار) ((فراغ)(مقدار))
#تعریف_کن GET_SEG(SEG) 0
#تعریف_کن GET_FLATPTR(ptr) (ptr)
#تعریف_کن مجموعه_FLATPTR(ptr, val) افعل { (ptr) = (val); } طالما (0)

#تعریف_کن insb_fl(پورت, ptr_fl, شمار) insb(پورت, ptr_fl, شمار)
#تعریف_کن insw_fl(پورت, ptr_fl, شمار) insw(پورت, ptr_fl, شمار)
#تعریف_کن insl_fl(پورت, ptr_fl, شمار) insl(پورت, ptr_fl, شمار)
#تعریف_کن outsb_fl(پورت, ptr_fl, شمار) outsb(پورت, ptr_fl, شمار)
#تعریف_کن outsw_fl(پورت, ptr_fl, شمار) outsw(پورت, ptr_fl, شمار)
#تعریف_کن outsl_fl(پورت, ptr_fl, شمار) outsl(پورت, ptr_fl, شمار)

#پایان_اگر

#تعریف_کن SEGOFF(s,o) ({بنية segoff_s __so; __so.جابه_جایی_2=(o); __so.seg=(s); __so;})

ساكن مضمن بنية segoff_s FLATPTR_مقصد_SEGOFF(فراغ *p) {
    أرجع SEGOFF(FLATPTR_مقصد_SEG(p), FLATPTR_مقصد_جابه_جایی(p));
}
ساكن مضمن فراغ *SEGOFF_مقصد_FLATPTR(بنية segoff_s so) {
    أرجع MAKE_FLATPTR(so.seg, so.جابه_جایی_2);
}

#پایان_اگر // farptr.h
