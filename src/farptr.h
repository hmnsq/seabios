// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __FARPTR_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __FARPTR_H

#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
ਬਾਹਰੀ u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
ਬਾਹਰੀ u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ8_SEG(ਅਗੇਤਰ, SEG, ਮੁੱਲ, var)                      \
    __asm__(ਅਗੇਤਰ "movb %%" #SEG ":%1, %b0" : "=Qi"(ਮੁੱਲ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ16_SEG(ਅਗੇਤਰ, SEG, ਮੁੱਲ, var)                     \
    __asm__(ਅਗੇਤਰ "movw %%" #SEG ":%1, %w0" : "=ri"(ਮੁੱਲ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ32_SEG(ਅਗੇਤਰ, SEG, ਮੁੱਲ, var)                     \
    __asm__(ਅਗੇਤਰ "movl %%" #SEG ":%1, %0" : "=ri"(ਮੁੱਲ)      \
            : "m"(var), "m"(__segment_ ## SEG))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ64_SEG(ਅਗੇਤਰ, SEG, ਮੁੱਲ, var) ਕਰੋ {                \
        ਸੰਘ u64_u32_u __value;                                \
        ਸੰਘ u64_u32_u *__r64_ptr = (ਸੰਘ u64_u32_u *)&(var); \
        READ32_SEG(ਅਗੇਤਰ, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(ਅਗੇਤਰ, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(ਮੁੱਲ) = __value.val;                          \
    } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE8_SEG(ਅਗੇਤਰ, SEG, var, ਮੁੱਲ)                     \
    __asm__(ਅਗੇਤਰ "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(ਮੁੱਲ), "m"(__segment_ ## SEG))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE16_SEG(ਅਗੇਤਰ, SEG, var, ਮੁੱਲ)                    \
    __asm__(ਅਗੇਤਰ "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(ਮੁੱਲ), "m"(__segment_ ## SEG))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE32_SEG(ਅਗੇਤਰ, SEG, var, ਮੁੱਲ)                    \
    __asm__(ਅਗੇਤਰ "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(ਮੁੱਲ), "m"(__segment_ ## SEG))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE64_SEG(ਅਗੇਤਰ, SEG, var, ਮੁੱਲ) ਕਰੋ {               \
        ਸੰਘ u64_u32_u __value;                                \
        ਸੰਘ u64_u32_u *__w64_ptr = (ਸੰਘ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (ਮੁੱਲ);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(ਅਗੇਤਰ, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(ਅਗੇਤਰ, SEG, __w64_ptr->hi, __value.hi);    \
    } ਜਦੋਂ (0)

// Macros for automatically choosing the appropriate memory size
// access method.
ਬਾਹਰੀ ਖਾਲੀ __force_link_error__unknown_type(ਖਾਲੀ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __GET_VAR(ਅਗੇਤਰ, seg, var) ({          \
    typeof(var) __val;                          \
    ਜੇ (ਆਕਾਰ(__val) == 1)                     \
        READ8_SEG(ਅਗੇਤਰ, seg, __val, var);     \
    ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(__val) == 2)                \
        READ16_SEG(ਅਗੇਤਰ, seg, __val, var);    \
    ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(__val) == 4)                \
        READ32_SEG(ਅਗੇਤਰ, seg, __val, var);    \
    ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(__val) == 8)                \
        READ64_SEG(ਅਗੇਤਰ, seg, __val, var);    \
    ਨਹੀਂਤਾਂ                                        \
        __force_link_error__unknown_type();     \
    __val; })

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __SET_VAR(ਅਗੇਤਰ, seg, var, val) ਕਰੋ {           \
        ਜੇ (ਆਕਾਰ(var) == 1)                           \
            WRITE8_SEG(ਅਗੇਤਰ, seg, var, (val));        \
        ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(var) == 2)                      \
            WRITE16_SEG(ਅਗੇਤਰ, seg, var, (val));       \
        ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(var) == 4)                      \
            WRITE32_SEG(ਅਗੇਤਰ, seg, var, (val));       \
        ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(var) == 8)                      \
            WRITE64_SEG(ਅਗੇਤਰ, seg, var, (val));       \
        ਨਹੀਂਤਾਂ                                            \
            __force_link_error__unknown_type();         \
    } ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DECL_SEGFUNCS(SEG)                              \
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 __get_seg_##SEG(ਖਾਲੀ) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    ਵਾਪਸ res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __SET_SEG(SEG, ਮੁੱਲ)                   \
    __set_seg_##SEG(ਮੁੱਲ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __GET_FARVAR(seg, var) ({               \
    ਸੈੱਟ_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __SET_FARVAR(seg, var, val) ਕਰੋ {        \
        typeof(var) __sfv_val = (val);          \
        ਸੈੱਟ_SEG(ES, (seg));                     \
        ਸੈੱਟ_VAR(ES, (var), __sfv_val);          \
    } ਜਦੋਂ (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_ਮੰਜ਼ਿਲ_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(__ptr)); })
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __SET_FLATPTR(ptr, val) ਕਰੋ {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        ਸੈੱਟ_FARVAR(FLATPTR_ਮੰਜ਼ਿਲ_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(__ptr)   \
                   , (val));                                    \
    } ਜਦੋਂ (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLATPTR_ਮੰਜ਼ਿਲ_SEG(p) (((u32)(p)) >> 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(p) (((u32)(p)) & 0xf)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAKE_FLATPTR(seg,off) ((ਖਾਲੀ*)(((u32)(seg)<<4)+(u32)(off)))


#ਜੇ MODESEGMENT == 1

// Definitions when using segmented mode.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_VAR(seg, var) __GET_VAR("", seg, (var))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_SEG(SEG, ਮੁੱਲ) __SET_SEG(SEG, (ਮੁੱਲ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_SEG(SEG) __GET_SEG(SEG)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ insb_fl(u16 ਪੋਰਟ, ਖਾਲੀ *ptr_fl, u16 ਗਿਣਤੀ_2) {
    ਸੈੱਟ_SEG(ES, FLATPTR_ਮੰਜ਼ਿਲ_SEG(ptr_fl));
    insb(ਪੋਰਟ, (u8*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ptr_fl), ਗਿਣਤੀ_2);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ insw_fl(u16 ਪੋਰਟ, ਖਾਲੀ *ptr_fl, u16 ਗਿਣਤੀ_2) {
    ਸੈੱਟ_SEG(ES, FLATPTR_ਮੰਜ਼ਿਲ_SEG(ptr_fl));
    insw(ਪੋਰਟ, (u16*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ptr_fl), ਗਿਣਤੀ_2);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ insl_fl(u16 ਪੋਰਟ, ਖਾਲੀ *ptr_fl, u16 ਗਿਣਤੀ_2) {
    ਸੈੱਟ_SEG(ES, FLATPTR_ਮੰਜ਼ਿਲ_SEG(ptr_fl));
    insl(ਪੋਰਟ, (u32*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ptr_fl), ਗਿਣਤੀ_2);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outsb_fl(u16 ਪੋਰਟ, ਖਾਲੀ *ptr_fl, u16 ਗਿਣਤੀ_2) {
    ਸੈੱਟ_SEG(ES, FLATPTR_ਮੰਜ਼ਿਲ_SEG(ptr_fl));
    outsb(ਪੋਰਟ, (u8*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ptr_fl), ਗਿਣਤੀ_2);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outsw_fl(u16 ਪੋਰਟ, ਖਾਲੀ *ptr_fl, u16 ਗਿਣਤੀ_2) {
    ਸੈੱਟ_SEG(ES, FLATPTR_ਮੰਜ਼ਿਲ_SEG(ptr_fl));
    outsw(ਪੋਰਟ, (u16*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ptr_fl), ਗਿਣਤੀ_2);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outsl_fl(u16 ਪੋਰਟ, ਖਾਲੀ *ptr_fl, u16 ਗਿਣਤੀ_2) {
    ਸੈੱਟ_SEG(ES, FLATPTR_ਮੰਜ਼ਿਲ_SEG(ptr_fl));
    outsl(ਪੋਰਟ, (u32*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ptr_fl), ਗਿਣਤੀ_2);
}

#ਨਹੀਂਤਾਂ

// In 32-bit flat mode there is no need to mess with the segments.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_FARVAR(seg, var, val) \
    ਕਰੋ { GET_FARVAR((seg), (var)) = (val); } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_VAR(seg, var) (var)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_VAR(seg, var, val) ਕਰੋ { (var) = (val); } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_SEG(SEG, ਮੁੱਲ) ((ਖਾਲੀ)(ਮੁੱਲ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_SEG(SEG) 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_FLATPTR(ptr) (ptr)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_FLATPTR(ptr, val) ਕਰੋ { (ptr) = (val); } ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ insb_fl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2) insb(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ insw_fl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2) insw(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ insl_fl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2) insl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ outsb_fl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2) outsb(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ outsw_fl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2) outsw(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ outsl_fl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2) outsl(ਪੋਰਟ, ptr_fl, ਗਿਣਤੀ_2)

#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEGOFF(s,o) ({ਬਣਤਰ segoff_s __so; __so.ਵਿੱਥ_2=(o); __so.seg=(s); __so;})

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ segoff_s FLATPTR_ਮੰਜ਼ਿਲ_SEGOFF(ਖਾਲੀ *p) {
    ਵਾਪਸ SEGOFF(FLATPTR_ਮੰਜ਼ਿਲ_SEG(p), FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(p));
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *SEGOFF_ਮੰਜ਼ਿਲ_FLATPTR(ਬਣਤਰ segoff_s so) {
    ਵਾਪਸ MAKE_FLATPTR(so.seg, so.ਵਿੱਥ_2);
}

#ਜੇ_ਅੰਤ // farptr.h
