// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#جيڪڏهن_وصف_نه_ٿيل __FARPTR_H
#وصف_ڪريو __FARPTR_H

#ضمن "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
خارجي u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
خارجي u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#وصف_ڪريو READ8_SEG(اڳياڙي, SEG, قدر, var)                      \
    __asm__(اڳياڙي "movb %%" #SEG ":%1, %b0" : "=Qi"(قدر)     \
            : "m"(var), "m"(__segment_ ## SEG))
#وصف_ڪريو READ16_SEG(اڳياڙي, SEG, قدر, var)                     \
    __asm__(اڳياڙي "movw %%" #SEG ":%1, %w0" : "=ri"(قدر)     \
            : "m"(var), "m"(__segment_ ## SEG))
#وصف_ڪريو READ32_SEG(اڳياڙي, SEG, قدر, var)                     \
    __asm__(اڳياڙي "movl %%" #SEG ":%1, %0" : "=ri"(قدر)      \
            : "m"(var), "m"(__segment_ ## SEG))
#وصف_ڪريو READ64_SEG(اڳياڙي, SEG, قدر, var) افعل {                \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__r64_ptr = (اتحاد u64_u32_u *)&(var); \
        READ32_SEG(اڳياڙي, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(اڳياڙي, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(قدر) = __value.val;                          \
    } طالما (0)
#وصف_ڪريو WRITE8_SEG(اڳياڙي, SEG, var, قدر)                     \
    __asm__(اڳياڙي "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(قدر), "m"(__segment_ ## SEG))
#وصف_ڪريو WRITE16_SEG(اڳياڙي, SEG, var, قدر)                    \
    __asm__(اڳياڙي "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(قدر), "m"(__segment_ ## SEG))
#وصف_ڪريو WRITE32_SEG(اڳياڙي, SEG, var, قدر)                    \
    __asm__(اڳياڙي "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(قدر), "m"(__segment_ ## SEG))
#وصف_ڪريو WRITE64_SEG(اڳياڙي, SEG, var, قدر) افعل {               \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__w64_ptr = (اتحاد u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (قدر);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(اڳياڙي, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(اڳياڙي, SEG, __w64_ptr->hi, __value.hi);    \
    } طالما (0)

// Macros for automatically choosing the appropriate memory size
// access method.
خارجي فراغ __force_link_error__unknown_type(فراغ);

#وصف_ڪريو __GET_VAR(اڳياڙي, seg, var) ({          \
    typeof(var) __val;                          \
    إذا (حجم(__val) == 1)                     \
        READ8_SEG(اڳياڙي, seg, __val, var);     \
    وإلا إذا (حجم(__val) == 2)                \
        READ16_SEG(اڳياڙي, seg, __val, var);    \
    وإلا إذا (حجم(__val) == 4)                \
        READ32_SEG(اڳياڙي, seg, __val, var);    \
    وإلا إذا (حجم(__val) == 8)                \
        READ64_SEG(اڳياڙي, seg, __val, var);    \
    وإلا                                        \
        __force_link_error__unknown_type();     \
    __val; })

#وصف_ڪريو __SET_VAR(اڳياڙي, seg, var, val) افعل {           \
        إذا (حجم(var) == 1)                           \
            WRITE8_SEG(اڳياڙي, seg, var, (val));        \
        وإلا إذا (حجم(var) == 2)                      \
            WRITE16_SEG(اڳياڙي, seg, var, (val));       \
        وإلا إذا (حجم(var) == 4)                      \
            WRITE32_SEG(اڳياڙي, seg, var, (val));       \
        وإلا إذا (حجم(var) == 8)                      \
            WRITE64_SEG(اڳياڙي, seg, var, (val));       \
        وإلا                                            \
            __force_link_error__unknown_type();         \
    } طالما (0)

#وصف_ڪريو DECL_SEGFUNCS(SEG)                              \
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
#وصف_ڪريو __SET_SEG(SEG, قدر)                   \
    __set_seg_##SEG(قدر)
#وصف_ڪريو __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#وصف_ڪريو __GET_FARVAR(seg, var) ({               \
    سيٽ_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#وصف_ڪريو __SET_FARVAR(seg, var, val) افعل {        \
        typeof(var) __sfv_val = (val);          \
        سيٽ_SEG(ES, (seg));                     \
        سيٽ_VAR(ES, (var), __sfv_val);          \
    } طالما (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#وصف_ڪريو __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_منزل_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_منزل_وٿي(__ptr)); })
#وصف_ڪريو __SET_FLATPTR(ptr, val) افعل {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        سيٽ_FARVAR(FLATPTR_منزل_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_منزل_وٿي(__ptr)   \
                   , (val));                                    \
    } طالما (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#وصف_ڪريو FLATPTR_منزل_SEG(p) (((u32)(p)) >> 4)
#وصف_ڪريو FLATPTR_منزل_وٿي(p) (((u32)(p)) & 0xf)
#وصف_ڪريو MAKE_FLATPTR(seg,off) ((فراغ*)(((u32)(seg)<<4)+(u32)(off)))


#إذا MODESEGMENT == 1

// Definitions when using segmented mode.
#وصف_ڪريو GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#وصف_ڪريو سيٽ_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#وصف_ڪريو GET_VAR(seg, var) __GET_VAR("", seg, (var))
#وصف_ڪريو سيٽ_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#وصف_ڪريو سيٽ_SEG(SEG, قدر) __SET_SEG(SEG, (قدر))
#وصف_ڪريو GET_SEG(SEG) __GET_SEG(SEG)
#وصف_ڪريو GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#وصف_ڪريو سيٽ_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ساكن مضمن فراغ insb_fl(u16 پورٽ, فراغ *ptr_fl, u16 ڳڻپ) {
    سيٽ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    insb(پورٽ, (u8*)FLATPTR_منزل_وٿي(ptr_fl), ڳڻپ);
}
ساكن مضمن فراغ insw_fl(u16 پورٽ, فراغ *ptr_fl, u16 ڳڻپ) {
    سيٽ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    insw(پورٽ, (u16*)FLATPTR_منزل_وٿي(ptr_fl), ڳڻپ);
}
ساكن مضمن فراغ insl_fl(u16 پورٽ, فراغ *ptr_fl, u16 ڳڻپ) {
    سيٽ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    insl(پورٽ, (u32*)FLATPTR_منزل_وٿي(ptr_fl), ڳڻپ);
}
ساكن مضمن فراغ outsb_fl(u16 پورٽ, فراغ *ptr_fl, u16 ڳڻپ) {
    سيٽ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    outsb(پورٽ, (u8*)FLATPTR_منزل_وٿي(ptr_fl), ڳڻپ);
}
ساكن مضمن فراغ outsw_fl(u16 پورٽ, فراغ *ptr_fl, u16 ڳڻپ) {
    سيٽ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    outsw(پورٽ, (u16*)FLATPTR_منزل_وٿي(ptr_fl), ڳڻپ);
}
ساكن مضمن فراغ outsl_fl(u16 پورٽ, فراغ *ptr_fl, u16 ڳڻپ) {
    سيٽ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    outsl(پورٽ, (u32*)FLATPTR_منزل_وٿي(ptr_fl), ڳڻپ);
}

#وإلا

// In 32-bit flat mode there is no need to mess with the segments.
#وصف_ڪريو GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#وصف_ڪريو سيٽ_FARVAR(seg, var, val) \
    افعل { GET_FARVAR((seg), (var)) = (val); } طالما (0)
#وصف_ڪريو GET_VAR(seg, var) (var)
#وصف_ڪريو سيٽ_VAR(seg, var, val) افعل { (var) = (val); } طالما (0)
#وصف_ڪريو سيٽ_SEG(SEG, قدر) ((فراغ)(قدر))
#وصف_ڪريو GET_SEG(SEG) 0
#وصف_ڪريو GET_FLATPTR(ptr) (ptr)
#وصف_ڪريو سيٽ_FLATPTR(ptr, val) افعل { (ptr) = (val); } طالما (0)

#وصف_ڪريو insb_fl(پورٽ, ptr_fl, ڳڻپ) insb(پورٽ, ptr_fl, ڳڻپ)
#وصف_ڪريو insw_fl(پورٽ, ptr_fl, ڳڻپ) insw(پورٽ, ptr_fl, ڳڻپ)
#وصف_ڪريو insl_fl(پورٽ, ptr_fl, ڳڻپ) insl(پورٽ, ptr_fl, ڳڻپ)
#وصف_ڪريو outsb_fl(پورٽ, ptr_fl, ڳڻپ) outsb(پورٽ, ptr_fl, ڳڻپ)
#وصف_ڪريو outsw_fl(پورٽ, ptr_fl, ڳڻپ) outsw(پورٽ, ptr_fl, ڳڻپ)
#وصف_ڪريو outsl_fl(پورٽ, ptr_fl, ڳڻپ) outsl(پورٽ, ptr_fl, ڳڻپ)

#جيڪڏهن_پورو

#وصف_ڪريو SEGOFF(s,o) ({بنية segoff_s __so; __so.وٿي_2=(o); __so.seg=(s); __so;})

ساكن مضمن بنية segoff_s FLATPTR_منزل_SEGOFF(فراغ *p) {
    أرجع SEGOFF(FLATPTR_منزل_SEG(p), FLATPTR_منزل_وٿي(p));
}
ساكن مضمن فراغ *SEGOFF_منزل_FLATPTR(بنية segoff_s so) {
    أرجع MAKE_FLATPTR(so.seg, so.وٿي_2);
}

#جيڪڏهن_پورو // farptr.h
