// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#که_نه_تعریف_شوی __FARPTR_H
#تعریف_کړه __FARPTR_H

#ضمن "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
خارجي u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
خارجي u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#تعریف_کړه READ8_SEG(مختاړی, SEG, ارزښت, var)                      \
    __asm__(مختاړی "movb %%" #SEG ":%1, %b0" : "=Qi"(ارزښت)     \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کړه READ16_SEG(مختاړی, SEG, ارزښت, var)                     \
    __asm__(مختاړی "movw %%" #SEG ":%1, %w0" : "=ri"(ارزښت)     \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کړه READ32_SEG(مختاړی, SEG, ارزښت, var)                     \
    __asm__(مختاړی "movl %%" #SEG ":%1, %0" : "=ri"(ارزښت)      \
            : "m"(var), "m"(__segment_ ## SEG))
#تعریف_کړه READ64_SEG(مختاړی, SEG, ارزښت, var) افعل {                \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__r64_ptr = (اتحاد u64_u32_u *)&(var); \
        READ32_SEG(مختاړی, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(مختاړی, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(ارزښت) = __value.val;                          \
    } طالما (0)
#تعریف_کړه WRITE8_SEG(مختاړی, SEG, var, ارزښت)                     \
    __asm__(مختاړی "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(ارزښت), "m"(__segment_ ## SEG))
#تعریف_کړه WRITE16_SEG(مختاړی, SEG, var, ارزښت)                    \
    __asm__(مختاړی "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(ارزښت), "m"(__segment_ ## SEG))
#تعریف_کړه WRITE32_SEG(مختاړی, SEG, var, ارزښت)                    \
    __asm__(مختاړی "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(ارزښت), "m"(__segment_ ## SEG))
#تعریف_کړه WRITE64_SEG(مختاړی, SEG, var, ارزښت) افعل {               \
        اتحاد u64_u32_u __value;                                \
        اتحاد u64_u32_u *__w64_ptr = (اتحاد u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (ارزښت);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(مختاړی, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(مختاړی, SEG, __w64_ptr->hi, __value.hi);    \
    } طالما (0)

// Macros for automatically choosing the appropriate memory size
// access method.
خارجي فراغ __force_link_error__unknown_type(فراغ);

#تعریف_کړه __GET_VAR(مختاړی, seg, var) ({          \
    typeof(var) __val;                          \
    إذا (حجم(__val) == 1)                     \
        READ8_SEG(مختاړی, seg, __val, var);     \
    وإلا إذا (حجم(__val) == 2)                \
        READ16_SEG(مختاړی, seg, __val, var);    \
    وإلا إذا (حجم(__val) == 4)                \
        READ32_SEG(مختاړی, seg, __val, var);    \
    وإلا إذا (حجم(__val) == 8)                \
        READ64_SEG(مختاړی, seg, __val, var);    \
    وإلا                                        \
        __force_link_error__unknown_type();     \
    __val; })

#تعریف_کړه __SET_VAR(مختاړی, seg, var, val) افعل {           \
        إذا (حجم(var) == 1)                           \
            WRITE8_SEG(مختاړی, seg, var, (val));        \
        وإلا إذا (حجم(var) == 2)                      \
            WRITE16_SEG(مختاړی, seg, var, (val));       \
        وإلا إذا (حجم(var) == 4)                      \
            WRITE32_SEG(مختاړی, seg, var, (val));       \
        وإلا إذا (حجم(var) == 8)                      \
            WRITE64_SEG(مختاړی, seg, var, (val));       \
        وإلا                                            \
            __force_link_error__unknown_type();         \
    } طالما (0)

#تعریف_کړه DECL_SEGFUNCS(SEG)                              \
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
#تعریف_کړه __SET_SEG(SEG, ارزښت)                   \
    __set_seg_##SEG(ارزښت)
#تعریف_کړه __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#تعریف_کړه __GET_FARVAR(seg, var) ({               \
    سېټ_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#تعریف_کړه __SET_FARVAR(seg, var, val) افعل {        \
        typeof(var) __sfv_val = (val);          \
        سېټ_SEG(ES, (seg));                     \
        سېټ_VAR(ES, (var), __sfv_val);          \
    } طالما (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#تعریف_کړه __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_منزل_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_منزل_دننه_واټن(__ptr)); })
#تعریف_کړه __SET_FLATPTR(ptr, val) افعل {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        سېټ_FARVAR(FLATPTR_منزل_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_منزل_دننه_واټن(__ptr)   \
                   , (val));                                    \
    } طالما (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#تعریف_کړه FLATPTR_منزل_SEG(p) (((u32)(p)) >> 4)
#تعریف_کړه FLATPTR_منزل_دننه_واټن(p) (((u32)(p)) & 0xf)
#تعریف_کړه MAKE_FLATPTR(seg,off) ((فراغ*)(((u32)(seg)<<4)+(u32)(off)))


#إذا MODESEGMENT == 1

// Definitions when using segmented mode.
#تعریف_کړه GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#تعریف_کړه سېټ_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#تعریف_کړه GET_VAR(seg, var) __GET_VAR("", seg, (var))
#تعریف_کړه سېټ_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#تعریف_کړه سېټ_SEG(SEG, ارزښت) __SET_SEG(SEG, (ارزښت))
#تعریف_کړه GET_SEG(SEG) __GET_SEG(SEG)
#تعریف_کړه GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#تعریف_کړه سېټ_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ساكن مضمن فراغ insb_fl(u16 پورټ, فراغ *ptr_fl, u16 شمېر) {
    سېټ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    insb(پورټ, (u8*)FLATPTR_منزل_دننه_واټن(ptr_fl), شمېر);
}
ساكن مضمن فراغ insw_fl(u16 پورټ, فراغ *ptr_fl, u16 شمېر) {
    سېټ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    insw(پورټ, (u16*)FLATPTR_منزل_دننه_واټن(ptr_fl), شمېر);
}
ساكن مضمن فراغ insl_fl(u16 پورټ, فراغ *ptr_fl, u16 شمېر) {
    سېټ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    insl(پورټ, (u32*)FLATPTR_منزل_دننه_واټن(ptr_fl), شمېر);
}
ساكن مضمن فراغ outsb_fl(u16 پورټ, فراغ *ptr_fl, u16 شمېر) {
    سېټ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    outsb(پورټ, (u8*)FLATPTR_منزل_دننه_واټن(ptr_fl), شمېر);
}
ساكن مضمن فراغ outsw_fl(u16 پورټ, فراغ *ptr_fl, u16 شمېر) {
    سېټ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    outsw(پورټ, (u16*)FLATPTR_منزل_دننه_واټن(ptr_fl), شمېر);
}
ساكن مضمن فراغ outsl_fl(u16 پورټ, فراغ *ptr_fl, u16 شمېر) {
    سېټ_SEG(ES, FLATPTR_منزل_SEG(ptr_fl));
    outsl(پورټ, (u32*)FLATPTR_منزل_دننه_واټن(ptr_fl), شمېر);
}

#وإلا

// In 32-bit flat mode there is no need to mess with the segments.
#تعریف_کړه GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#تعریف_کړه سېټ_FARVAR(seg, var, val) \
    افعل { GET_FARVAR((seg), (var)) = (val); } طالما (0)
#تعریف_کړه GET_VAR(seg, var) (var)
#تعریف_کړه سېټ_VAR(seg, var, val) افعل { (var) = (val); } طالما (0)
#تعریف_کړه سېټ_SEG(SEG, ارزښت) ((فراغ)(ارزښت))
#تعریف_کړه GET_SEG(SEG) 0
#تعریف_کړه GET_FLATPTR(ptr) (ptr)
#تعریف_کړه سېټ_FLATPTR(ptr, val) افعل { (ptr) = (val); } طالما (0)

#تعریف_کړه insb_fl(پورټ, ptr_fl, شمېر) insb(پورټ, ptr_fl, شمېر)
#تعریف_کړه insw_fl(پورټ, ptr_fl, شمېر) insw(پورټ, ptr_fl, شمېر)
#تعریف_کړه insl_fl(پورټ, ptr_fl, شمېر) insl(پورټ, ptr_fl, شمېر)
#تعریف_کړه outsb_fl(پورټ, ptr_fl, شمېر) outsb(پورټ, ptr_fl, شمېر)
#تعریف_کړه outsw_fl(پورټ, ptr_fl, شمېر) outsw(پورټ, ptr_fl, شمېر)
#تعریف_کړه outsl_fl(پورټ, ptr_fl, شمېر) outsl(پورټ, ptr_fl, شمېر)

#د_که_پای

#تعریف_کړه SEGOFF(s,o) ({بنية segoff_s __so; __so.دننه_واټن_2=(o); __so.seg=(s); __so;})

ساكن مضمن بنية segoff_s FLATPTR_منزل_SEGOFF(فراغ *p) {
    أرجع SEGOFF(FLATPTR_منزل_SEG(p), FLATPTR_منزل_دننه_واټن(p));
}
ساكن مضمن فراغ *SEGOFF_منزل_FLATPTR(بنية segoff_s so) {
    أرجع MAKE_FLATPTR(so.seg, so.دننه_واټن_2);
}

#د_که_پای // farptr.h
