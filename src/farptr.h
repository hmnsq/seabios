// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __FARPTR_H
#define __FARPTR_H

#അന്തര്ഭാവ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
ബാഹ്യ u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
ബാഹ്യ u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#define READ8_SEG(prefix, SEG, മൂല്യം, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(മൂല്യം)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ16_SEG(prefix, SEG, മൂല്യം, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(മൂല്യം)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ32_SEG(prefix, SEG, മൂല്യം, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(മൂല്യം)      \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ64_SEG(prefix, SEG, മൂല്യം, var) ചെയ്യുക {                \
        സംഘം u64_u32_u __value;                                \
        സംഘം u64_u32_u *__r64_ptr = (സംഘം u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(മൂല്യം) = __value.val;                          \
    } ആകുമ്പോൾ (0)
#define WRITE8_SEG(prefix, SEG, var, മൂല്യം)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(മൂല്യം), "m"(__segment_ ## SEG))
#define WRITE16_SEG(prefix, SEG, var, മൂല്യം)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(മൂല്യം), "m"(__segment_ ## SEG))
#define WRITE32_SEG(prefix, SEG, var, മൂല്യം)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(മൂല്യം), "m"(__segment_ ## SEG))
#define WRITE64_SEG(prefix, SEG, var, മൂല്യം) ചെയ്യുക {               \
        സംഘം u64_u32_u __value;                                \
        സംഘം u64_u32_u *__w64_ptr = (സംഘം u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (മൂല്യം);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } ആകുമ്പോൾ (0)

// Macros for automatically choosing the appropriate memory size
// access method.
ബാഹ്യ ശൂന്യം __force_link_error__unknown_type(ശൂന്യം);

#define __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    എങ്കിൽ (വലുപ്പം(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    അല്ലെങ്കിൽ                                        \
        __force_link_error__unknown_type();     \
    __val; })

#define __SET_VAR(prefix, seg, var, val) ചെയ്യുക {           \
        എങ്കിൽ (വലുപ്പം(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        അല്ലെങ്കിൽ                                            \
            __force_link_error__unknown_type();         \
    } ആകുമ്പോൾ (0)

#define DECL_SEGFUNCS(SEG)                              \
സ്ഥിതം വരിയിൽ ശൂന്യം __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
സ്ഥിതം വരിയിൽ u16 __get_seg_##SEG(ശൂന്യം) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    തിരികെ res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, മൂല്യം)                   \
    __set_seg_##SEG(മൂല്യം)
#define __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#define __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#define __SET_FARVAR(seg, var, val) ചെയ്യുക {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } ആകുമ്പോൾ (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#define __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#define __SET_FLATPTR(ptr, val) ചെയ്യുക {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } ആകുമ്പോൾ (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#define FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#define FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#define MAKE_FLATPTR(seg,off) ((ശൂന്യം*)(((u32)(seg)<<4)+(u32)(off)))


#എങ്കിൽ MODESEGMENT == 1

// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, മൂല്യം) __SET_SEG(SEG, (മൂല്യം))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

സ്ഥിതം വരിയിൽ ശൂന്യം insb_fl(u16 port, ശൂന്യം *ptr_fl, u16 എണ്ണം) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), എണ്ണം);
}
സ്ഥിതം വരിയിൽ ശൂന്യം insw_fl(u16 port, ശൂന്യം *ptr_fl, u16 എണ്ണം) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), എണ്ണം);
}
സ്ഥിതം വരിയിൽ ശൂന്യം insl_fl(u16 port, ശൂന്യം *ptr_fl, u16 എണ്ണം) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), എണ്ണം);
}
സ്ഥിതം വരിയിൽ ശൂന്യം outsb_fl(u16 port, ശൂന്യം *ptr_fl, u16 എണ്ണം) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), എണ്ണം);
}
സ്ഥിതം വരിയിൽ ശൂന്യം outsw_fl(u16 port, ശൂന്യം *ptr_fl, u16 എണ്ണം) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), എണ്ണം);
}
സ്ഥിതം വരിയിൽ ശൂന്യം outsl_fl(u16 port, ശൂന്യം *ptr_fl, u16 എണ്ണം) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), എണ്ണം);
}

#അല്ലെങ്കിൽ

// In 32-bit flat mode there is no need to mess with the segments.
#define GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#define SET_FARVAR(seg, var, val) \
    ചെയ്യുക { GET_FARVAR((seg), (var)) = (val); } ആകുമ്പോൾ (0)
#define GET_VAR(seg, var) (var)
#define SET_VAR(seg, var, val) ചെയ്യുക { (var) = (val); } ആകുമ്പോൾ (0)
#define SET_SEG(SEG, മൂല്യം) ((ശൂന്യം)(മൂല്യം))
#define GET_SEG(SEG) 0
#define GET_FLATPTR(ptr) (ptr)
#define SET_FLATPTR(ptr, val) ചെയ്യുക { (ptr) = (val); } ആകുമ്പോൾ (0)

#define insb_fl(port, ptr_fl, എണ്ണം) insb(port, ptr_fl, എണ്ണം)
#define insw_fl(port, ptr_fl, എണ്ണം) insw(port, ptr_fl, എണ്ണം)
#define insl_fl(port, ptr_fl, എണ്ണം) insl(port, ptr_fl, എണ്ണം)
#define outsb_fl(port, ptr_fl, എണ്ണം) outsb(port, ptr_fl, എണ്ണം)
#define outsw_fl(port, ptr_fl, എണ്ണം) outsw(port, ptr_fl, എണ്ണം)
#define outsl_fl(port, ptr_fl, എണ്ണം) outsl(port, ptr_fl, എണ്ണം)

#endif

#define SEGOFF(s,o) ({ഘടന segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

സ്ഥിതം വരിയിൽ ഘടന segoff_s FLATPTR_TO_SEGOFF(ശൂന്യം *p) {
    തിരികെ SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
സ്ഥിതം വരിയിൽ ശൂന്യം *SEGOFF_TO_FLATPTR(ഘടന segoff_s so) {
    തിരികെ MAKE_FLATPTR(so.seg, so.offset);
}

#endif // farptr.h
