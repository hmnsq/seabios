// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __FARPTR_H
#ವ್ಯಾಖ್ಯಾನಿಸು __FARPTR_H

#ಅನ್ತರ್ಭಾವ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
ಬಾಹ್ಯ u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
ಬಾಹ್ಯ u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#ವ್ಯಾಖ್ಯಾನಿಸು READ8_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, ಮೌಲ್ಯ, var)                      \
    __asm__(ಪೂರ್ವಪ್ರತ್ಯಯ "movb %%" #SEG ":%1, %b0" : "=Qi"(ಮೌಲ್ಯ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#ವ್ಯಾಖ್ಯಾನಿಸು READ16_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, ಮೌಲ್ಯ, var)                     \
    __asm__(ಪೂರ್ವಪ್ರತ್ಯಯ "movw %%" #SEG ":%1, %w0" : "=ri"(ಮೌಲ್ಯ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#ವ್ಯಾಖ್ಯಾನಿಸು READ32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, ಮೌಲ್ಯ, var)                     \
    __asm__(ಪೂರ್ವಪ್ರತ್ಯಯ "movl %%" #SEG ":%1, %0" : "=ri"(ಮೌಲ್ಯ)      \
            : "m"(var), "m"(__segment_ ## SEG))
#ವ್ಯಾಖ್ಯಾನಿಸು READ64_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, ಮೌಲ್ಯ, var) ಮಾಡು {                \
        ಒಕ್ಕೂಟ u64_u32_u __value;                                \
        ಒಕ್ಕೂಟ u64_u32_u *__r64_ptr = (ಒಕ್ಕೂಟ u64_u32_u *)&(var); \
        READ32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(ಮೌಲ್ಯ) = __value.val;                          \
    } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು WRITE8_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, var, ಮೌಲ್ಯ)                     \
    __asm__(ಪೂರ್ವಪ್ರತ್ಯಯ "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(ಮೌಲ್ಯ), "m"(__segment_ ## SEG))
#ವ್ಯಾಖ್ಯಾನಿಸು WRITE16_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, var, ಮೌಲ್ಯ)                    \
    __asm__(ಪೂರ್ವಪ್ರತ್ಯಯ "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(ಮೌಲ್ಯ), "m"(__segment_ ## SEG))
#ವ್ಯಾಖ್ಯಾನಿಸು WRITE32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, var, ಮೌಲ್ಯ)                    \
    __asm__(ಪೂರ್ವಪ್ರತ್ಯಯ "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(ಮೌಲ್ಯ), "m"(__segment_ ## SEG))
#ವ್ಯಾಖ್ಯಾನಿಸು WRITE64_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, var, ಮೌಲ್ಯ) ಮಾಡು {               \
        ಒಕ್ಕೂಟ u64_u32_u __value;                                \
        ಒಕ್ಕೂಟ u64_u32_u *__w64_ptr = (ಒಕ್ಕೂಟ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (ಮೌಲ್ಯ);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, SEG, __w64_ptr->hi, __value.hi);    \
    } ಆಗ (0)

// Macros for automatically choosing the appropriate memory size
// access method.
ಬಾಹ್ಯ ಶೂನ್ಯ __force_link_error__unknown_type(ಶೂನ್ಯ);

#ವ್ಯಾಖ್ಯಾನಿಸು __GET_VAR(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, var) ({          \
    typeof(var) __val;                          \
    ಆದರೆ (ಗಾತ್ರ(__val) == 1)                     \
        READ8_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, __val, var);     \
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(__val) == 2)                \
        READ16_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, __val, var);    \
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(__val) == 4)                \
        READ32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, __val, var);    \
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(__val) == 8)                \
        READ64_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, __val, var);    \
    ಇಲ್ಲದಿದ್ದರೆ                                        \
        __force_link_error__unknown_type();     \
    __val; })

#ವ್ಯಾಖ್ಯಾನಿಸು __SET_VAR(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, var, val) ಮಾಡು {           \
        ಆದರೆ (ಗಾತ್ರ(var) == 1)                           \
            WRITE8_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, var, (val));        \
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(var) == 2)                      \
            WRITE16_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, var, (val));       \
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(var) == 4)                      \
            WRITE32_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, var, (val));       \
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(var) == 8)                      \
            WRITE64_SEG(ಪೂರ್ವಪ್ರತ್ಯಯ, seg, var, (val));       \
        ಇಲ್ಲದಿದ್ದರೆ                                            \
            __force_link_error__unknown_type();         \
    } ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು DECL_SEGFUNCS(SEG)                              \
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 __get_seg_##SEG(ಶೂನ್ಯ) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    ಹಿಂತಿರುಗಿ res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#ವ್ಯಾಖ್ಯಾನಿಸು __SET_SEG(SEG, ಮೌಲ್ಯ)                   \
    __set_seg_##SEG(ಮೌಲ್ಯ)
#ವ್ಯಾಖ್ಯಾನಿಸು __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#ವ್ಯಾಖ್ಯಾನಿಸು __GET_FARVAR(seg, var) ({               \
    ಗಣ_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#ವ್ಯಾಖ್ಯಾನಿಸು __SET_FARVAR(seg, var, val) ಮಾಡು {        \
        typeof(var) __sfv_val = (val);          \
        ಗಣ_SEG(ES, (seg));                     \
        ಗಣ_VAR(ES, (var), __sfv_val);          \
    } ಆಗ (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#ವ್ಯಾಖ್ಯಾನಿಸು __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_ಗಮ್ಯ_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(__ptr)); })
#ವ್ಯಾಖ್ಯಾನಿಸು __SET_FLATPTR(ptr, val) ಮಾಡು {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        ಗಣ_FARVAR(FLATPTR_ಗಮ್ಯ_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(__ptr)   \
                   , (val));                                    \
    } ಆಗ (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#ವ್ಯಾಖ್ಯಾನಿಸು FLATPTR_ಗಮ್ಯ_SEG(p) (((u32)(p)) >> 4)
#ವ್ಯಾಖ್ಯಾನಿಸು FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(p) (((u32)(p)) & 0xf)
#ವ್ಯಾಖ್ಯಾನಿಸು MAKE_FLATPTR(seg,off) ((ಶೂನ್ಯ*)(((u32)(seg)<<4)+(u32)(off)))


#ಆದರೆ MODESEGMENT == 1

// Definitions when using segmented mode.
#ವ್ಯಾಖ್ಯಾನಿಸು GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#ವ್ಯಾಖ್ಯಾನಿಸು GET_VAR(seg, var) __GET_VAR("", seg, (var))
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_SEG(SEG, ಮೌಲ್ಯ) __SET_SEG(SEG, (ಮೌಲ್ಯ))
#ವ್ಯಾಖ್ಯಾನಿಸು GET_SEG(SEG) __GET_SEG(SEG)
#ವ್ಯಾಖ್ಯಾನಿಸು GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ insb_fl(u16 ಪೋರ್ಟ್, ಶೂನ್ಯ *ptr_fl, u16 ಎಣಿಕೆ_2) {
    ಗಣ_SEG(ES, FLATPTR_ಗಮ್ಯ_SEG(ptr_fl));
    insb(ಪೋರ್ಟ್, (u8*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ptr_fl), ಎಣಿಕೆ_2);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ insw_fl(u16 ಪೋರ್ಟ್, ಶೂನ್ಯ *ptr_fl, u16 ಎಣಿಕೆ_2) {
    ಗಣ_SEG(ES, FLATPTR_ಗಮ್ಯ_SEG(ptr_fl));
    insw(ಪೋರ್ಟ್, (u16*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ptr_fl), ಎಣಿಕೆ_2);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ insl_fl(u16 ಪೋರ್ಟ್, ಶೂನ್ಯ *ptr_fl, u16 ಎಣಿಕೆ_2) {
    ಗಣ_SEG(ES, FLATPTR_ಗಮ್ಯ_SEG(ptr_fl));
    insl(ಪೋರ್ಟ್, (u32*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ptr_fl), ಎಣಿಕೆ_2);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outsb_fl(u16 ಪೋರ್ಟ್, ಶೂನ್ಯ *ptr_fl, u16 ಎಣಿಕೆ_2) {
    ಗಣ_SEG(ES, FLATPTR_ಗಮ್ಯ_SEG(ptr_fl));
    outsb(ಪೋರ್ಟ್, (u8*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ptr_fl), ಎಣಿಕೆ_2);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outsw_fl(u16 ಪೋರ್ಟ್, ಶೂನ್ಯ *ptr_fl, u16 ಎಣಿಕೆ_2) {
    ಗಣ_SEG(ES, FLATPTR_ಗಮ್ಯ_SEG(ptr_fl));
    outsw(ಪೋರ್ಟ್, (u16*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ptr_fl), ಎಣಿಕೆ_2);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outsl_fl(u16 ಪೋರ್ಟ್, ಶೂನ್ಯ *ptr_fl, u16 ಎಣಿಕೆ_2) {
    ಗಣ_SEG(ES, FLATPTR_ಗಮ್ಯ_SEG(ptr_fl));
    outsl(ಪೋರ್ಟ್, (u32*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ptr_fl), ಎಣಿಕೆ_2);
}

#ಇಲ್ಲದಿದ್ದರೆ

// In 32-bit flat mode there is no need to mess with the segments.
#ವ್ಯಾಖ್ಯಾನಿಸು GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_FARVAR(seg, var, val) \
    ಮಾಡು { GET_FARVAR((seg), (var)) = (val); } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು GET_VAR(seg, var) (var)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_VAR(seg, var, val) ಮಾಡು { (var) = (val); } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_SEG(SEG, ಮೌಲ್ಯ) ((ಶೂನ್ಯ)(ಮೌಲ್ಯ))
#ವ್ಯಾಖ್ಯಾನಿಸು GET_SEG(SEG) 0
#ವ್ಯಾಖ್ಯಾನಿಸು GET_FLATPTR(ptr) (ptr)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_FLATPTR(ptr, val) ಮಾಡು { (ptr) = (val); } ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು insb_fl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2) insb(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2)
#ವ್ಯಾಖ್ಯಾನಿಸು insw_fl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2) insw(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2)
#ವ್ಯಾಖ್ಯಾನಿಸು insl_fl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2) insl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2)
#ವ್ಯಾಖ್ಯಾನಿಸು outsb_fl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2) outsb(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2)
#ವ್ಯಾಖ್ಯಾನಿಸು outsw_fl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2) outsw(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2)
#ವ್ಯಾಖ್ಯಾನಿಸು outsl_fl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2) outsl(ಪೋರ್ಟ್, ptr_fl, ಎಣಿಕೆ_2)

#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು SEGOFF(s,o) ({ರಚನೆ segoff_s __so; __so.ಸ್ಥಾನಾಂತರ_2=(o); __so.seg=(s); __so;})

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ segoff_s FLATPTR_ಗಮ್ಯ_SEGOFF(ಶೂನ್ಯ *p) {
    ಹಿಂತಿರುಗಿ SEGOFF(FLATPTR_ಗಮ್ಯ_SEG(p), FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(p));
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *SEGOFF_ಗಮ್ಯ_FLATPTR(ರಚನೆ segoff_s so) {
    ಹಿಂತಿರುಗಿ MAKE_FLATPTR(so.seg, so.ಸ್ಥಾನಾಂತರ_2);
}

#ಆದರೆ_ಅಂತ್ಯ // farptr.h
