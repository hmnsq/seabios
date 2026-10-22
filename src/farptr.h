// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __FARPTR_H
#define __FARPTR_H

#অন্তর্ভাৱ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
বাহ্য u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
বাহ্য u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#define READ8_SEG(prefix, SEG, মান, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(মান)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ16_SEG(prefix, SEG, মান, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(মান)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ32_SEG(prefix, SEG, মান, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(মান)      \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ64_SEG(prefix, SEG, মান, var) কৰা {                \
        সঙ্ঘ u64_u32_u __value;                                \
        সঙ্ঘ u64_u32_u *__r64_ptr = (সঙ্ঘ u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(মান) = __value.val;                          \
    } যেতিয়ালৈ (0)
#define WRITE8_SEG(prefix, SEG, var, মান)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(মান), "m"(__segment_ ## SEG))
#define WRITE16_SEG(prefix, SEG, var, মান)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(মান), "m"(__segment_ ## SEG))
#define WRITE32_SEG(prefix, SEG, var, মান)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(মান), "m"(__segment_ ## SEG))
#define WRITE64_SEG(prefix, SEG, var, মান) কৰা {               \
        সঙ্ঘ u64_u32_u __value;                                \
        সঙ্ঘ u64_u32_u *__w64_ptr = (সঙ্ঘ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (মান);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } যেতিয়ালৈ (0)

// Macros for automatically choosing the appropriate memory size
// access method.
বাহ্য শূন্য __force_link_error__unknown_type(শূন্য);

#define __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    যদি (পরিমাণ(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    অন্যথা যদি (পরিমাণ(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    অন্যথা যদি (পরিমাণ(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    অন্যথা যদি (পরিমাণ(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    অন্যথা                                        \
        __force_link_error__unknown_type();     \
    __val; })

#define __SET_VAR(prefix, seg, var, val) কৰা {           \
        যদি (পরিমাণ(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        অন্যথা যদি (পরিমাণ(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        অন্যথা যদি (পরিমাণ(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        অন্যথা যদি (পরিমাণ(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        অন্যথা                                            \
            __force_link_error__unknown_type();         \
    } যেতিয়ালৈ (0)

#define DECL_SEGFUNCS(SEG)                              \
স্থির অন্তঃস্থ শূন্য __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
স্থির অন্তঃস্থ u16 __get_seg_##SEG(শূন্য) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    ঘূৰাই res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, মান)                   \
    __set_seg_##SEG(মান)
#define __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#define __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#define __SET_FARVAR(seg, var, val) কৰা {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } যেতিয়ালৈ (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#define __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#define __SET_FLATPTR(ptr, val) কৰা {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } যেতিয়ালৈ (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#define FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#define FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#define MAKE_FLATPTR(seg,off) ((শূন্য*)(((u32)(seg)<<4)+(u32)(off)))


#যদি MODESEGMENT == 1

// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, মান) __SET_SEG(SEG, (মান))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

স্থির অন্তঃস্থ শূন্য insb_fl(u16 port, শূন্য *ptr_fl, u16 গুনতি) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), গুনতি);
}
স্থির অন্তঃস্থ শূন্য insw_fl(u16 port, শূন্য *ptr_fl, u16 গুনতি) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), গুনতি);
}
স্থির অন্তঃস্থ শূন্য insl_fl(u16 port, শূন্য *ptr_fl, u16 গুনতি) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), গুনতি);
}
স্থির অন্তঃস্থ শূন্য outsb_fl(u16 port, শূন্য *ptr_fl, u16 গুনতি) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), গুনতি);
}
স্থির অন্তঃস্থ শূন্য outsw_fl(u16 port, শূন্য *ptr_fl, u16 গুনতি) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), গুনতি);
}
স্থির অন্তঃস্থ শূন্য outsl_fl(u16 port, শূন্য *ptr_fl, u16 গুনতি) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), গুনতি);
}

#অন্যথা

// In 32-bit flat mode there is no need to mess with the segments.
#define GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#define SET_FARVAR(seg, var, val) \
    কৰা { GET_FARVAR((seg), (var)) = (val); } যেতিয়ালৈ (0)
#define GET_VAR(seg, var) (var)
#define SET_VAR(seg, var, val) কৰা { (var) = (val); } যেতিয়ালৈ (0)
#define SET_SEG(SEG, মান) ((শূন্য)(মান))
#define GET_SEG(SEG) 0
#define GET_FLATPTR(ptr) (ptr)
#define SET_FLATPTR(ptr, val) কৰা { (ptr) = (val); } যেতিয়ালৈ (0)

#define insb_fl(port, ptr_fl, গুনতি) insb(port, ptr_fl, গুনতি)
#define insw_fl(port, ptr_fl, গুনতি) insw(port, ptr_fl, গুনতি)
#define insl_fl(port, ptr_fl, গুনতি) insl(port, ptr_fl, গুনতি)
#define outsb_fl(port, ptr_fl, গুনতি) outsb(port, ptr_fl, গুনতি)
#define outsw_fl(port, ptr_fl, গুনতি) outsw(port, ptr_fl, গুনতি)
#define outsl_fl(port, ptr_fl, গুনতি) outsl(port, ptr_fl, গুনতি)

#endif

#define SEGOFF(s,o) ({সংরচনা segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

স্থির অন্তঃস্থ সংরচনা segoff_s FLATPTR_TO_SEGOFF(শূন্য *p) {
    ঘূৰাই SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
স্থির অন্তঃস্থ শূন্য *SEGOFF_TO_FLATPTR(সংরচনা segoff_s so) {
    ঘূৰাই MAKE_FLATPTR(so.seg, so.offset);
}

#endif // farptr.h
