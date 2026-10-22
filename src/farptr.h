// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#నిర్వచితం_కాకపోతే __FARPTR_H
#నిర్వచించు __FARPTR_H

#అన్తర్భావ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
బాహ్య u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
బాహ్య u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#నిర్వచించు READ8_SEG(ఉపసర్గ, SEG, విలువ, var)                      \
    __asm__(ఉపసర్గ "movb %%" #SEG ":%1, %b0" : "=Qi"(విలువ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#నిర్వచించు READ16_SEG(ఉపసర్గ, SEG, విలువ, var)                     \
    __asm__(ఉపసర్గ "movw %%" #SEG ":%1, %w0" : "=ri"(విలువ)     \
            : "m"(var), "m"(__segment_ ## SEG))
#నిర్వచించు READ32_SEG(ఉపసర్గ, SEG, విలువ, var)                     \
    __asm__(ఉపసర్గ "movl %%" #SEG ":%1, %0" : "=ri"(విలువ)      \
            : "m"(var), "m"(__segment_ ## SEG))
#నిర్వచించు READ64_SEG(ఉపసర్గ, SEG, విలువ, var) చేయి {                \
        సంఘం u64_u32_u __value;                                \
        సంఘం u64_u32_u *__r64_ptr = (సంఘం u64_u32_u *)&(var); \
        READ32_SEG(ఉపసర్గ, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(ఉపసర్గ, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(విలువ) = __value.val;                          \
    } అయితే (0)
#నిర్వచించు WRITE8_SEG(ఉపసర్గ, SEG, var, విలువ)                     \
    __asm__(ఉపసర్గ "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(విలువ), "m"(__segment_ ## SEG))
#నిర్వచించు WRITE16_SEG(ఉపసర్గ, SEG, var, విలువ)                    \
    __asm__(ఉపసర్గ "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(విలువ), "m"(__segment_ ## SEG))
#నిర్వచించు WRITE32_SEG(ఉపసర్గ, SEG, var, విలువ)                    \
    __asm__(ఉపసర్గ "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(విలువ), "m"(__segment_ ## SEG))
#నిర్వచించు WRITE64_SEG(ఉపసర్గ, SEG, var, విలువ) చేయి {               \
        సంఘం u64_u32_u __value;                                \
        సంఘం u64_u32_u *__w64_ptr = (సంఘం u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (విలువ);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(ఉపసర్గ, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(ఉపసర్గ, SEG, __w64_ptr->hi, __value.hi);    \
    } అయితే (0)

// Macros for automatically choosing the appropriate memory size
// access method.
బాహ్య శూన్య __force_link_error__unknown_type(శూన్య);

#నిర్వచించు __GET_VAR(ఉపసర్గ, seg, var) ({          \
    typeof(var) __val;                          \
    ఒకవేళ (పరిమాణం(__val) == 1)                     \
        READ8_SEG(ఉపసర్గ, seg, __val, var);     \
    లేకపోతే ఒకవేళ (పరిమాణం(__val) == 2)                \
        READ16_SEG(ఉపసర్గ, seg, __val, var);    \
    లేకపోతే ఒకవేళ (పరిమాణం(__val) == 4)                \
        READ32_SEG(ఉపసర్గ, seg, __val, var);    \
    లేకపోతే ఒకవేళ (పరిమాణం(__val) == 8)                \
        READ64_SEG(ఉపసర్గ, seg, __val, var);    \
    లేకపోతే                                        \
        __force_link_error__unknown_type();     \
    __val; })

#నిర్వచించు __SET_VAR(ఉపసర్గ, seg, var, val) చేయి {           \
        ఒకవేళ (పరిమాణం(var) == 1)                           \
            WRITE8_SEG(ఉపసర్గ, seg, var, (val));        \
        లేకపోతే ఒకవేళ (పరిమాణం(var) == 2)                      \
            WRITE16_SEG(ఉపసర్గ, seg, var, (val));       \
        లేకపోతే ఒకవేళ (పరిమాణం(var) == 4)                      \
            WRITE32_SEG(ఉపసర్గ, seg, var, (val));       \
        లేకపోతే ఒకవేళ (పరిమాణం(var) == 8)                      \
            WRITE64_SEG(ఉపసర్గ, seg, var, (val));       \
        లేకపోతే                                            \
            __force_link_error__unknown_type();         \
    } అయితే (0)

#నిర్వచించు DECL_SEGFUNCS(SEG)                              \
స్థిర అంతఃవరుస శూన్య __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
స్థిర అంతఃవరుస u16 __get_seg_##SEG(శూన్య) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    తిరిగి res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#నిర్వచించు __SET_SEG(SEG, విలువ)                   \
    __set_seg_##SEG(విలువ)
#నిర్వచించు __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#నిర్వచించు __GET_FARVAR(seg, var) ({               \
    సమితి_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#నిర్వచించు __SET_FARVAR(seg, var, val) చేయి {        \
        typeof(var) __sfv_val = (val);          \
        సమితి_SEG(ES, (seg));                     \
        సమితి_VAR(ES, (var), __sfv_val);          \
    } అయితే (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#నిర్వచించు __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_గమ్యం_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_గమ్యం_స్థానభ్రంశం(__ptr)); })
#నిర్వచించు __SET_FLATPTR(ptr, val) చేయి {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        సమితి_FARVAR(FLATPTR_గమ్యం_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_గమ్యం_స్థానభ్రంశం(__ptr)   \
                   , (val));                                    \
    } అయితే (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#నిర్వచించు FLATPTR_గమ్యం_SEG(p) (((u32)(p)) >> 4)
#నిర్వచించు FLATPTR_గమ్యం_స్థానభ్రంశం(p) (((u32)(p)) & 0xf)
#నిర్వచించు MAKE_FLATPTR(seg,off) ((శూన్య*)(((u32)(seg)<<4)+(u32)(off)))


#ఒకవేళ MODESEGMENT == 1

// Definitions when using segmented mode.
#నిర్వచించు GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#నిర్వచించు సమితి_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#నిర్వచించు GET_VAR(seg, var) __GET_VAR("", seg, (var))
#నిర్వచించు సమితి_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#నిర్వచించు సమితి_SEG(SEG, విలువ) __SET_SEG(SEG, (విలువ))
#నిర్వచించు GET_SEG(SEG) __GET_SEG(SEG)
#నిర్వచించు GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#నిర్వచించు సమితి_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

స్థిర అంతఃవరుస శూన్య insb_fl(u16 పోర్టు, శూన్య *ptr_fl, u16 లెక్క) {
    సమితి_SEG(ES, FLATPTR_గమ్యం_SEG(ptr_fl));
    insb(పోర్టు, (u8*)FLATPTR_గమ్యం_స్థానభ్రంశం(ptr_fl), లెక్క);
}
స్థిర అంతఃవరుస శూన్య insw_fl(u16 పోర్టు, శూన్య *ptr_fl, u16 లెక్క) {
    సమితి_SEG(ES, FLATPTR_గమ్యం_SEG(ptr_fl));
    insw(పోర్టు, (u16*)FLATPTR_గమ్యం_స్థానభ్రంశం(ptr_fl), లెక్క);
}
స్థిర అంతఃవరుస శూన్య insl_fl(u16 పోర్టు, శూన్య *ptr_fl, u16 లెక్క) {
    సమితి_SEG(ES, FLATPTR_గమ్యం_SEG(ptr_fl));
    insl(పోర్టు, (u32*)FLATPTR_గమ్యం_స్థానభ్రంశం(ptr_fl), లెక్క);
}
స్థిర అంతఃవరుస శూన్య outsb_fl(u16 పోర్టు, శూన్య *ptr_fl, u16 లెక్క) {
    సమితి_SEG(ES, FLATPTR_గమ్యం_SEG(ptr_fl));
    outsb(పోర్టు, (u8*)FLATPTR_గమ్యం_స్థానభ్రంశం(ptr_fl), లెక్క);
}
స్థిర అంతఃవరుస శూన్య outsw_fl(u16 పోర్టు, శూన్య *ptr_fl, u16 లెక్క) {
    సమితి_SEG(ES, FLATPTR_గమ్యం_SEG(ptr_fl));
    outsw(పోర్టు, (u16*)FLATPTR_గమ్యం_స్థానభ్రంశం(ptr_fl), లెక్క);
}
స్థిర అంతఃవరుస శూన్య outsl_fl(u16 పోర్టు, శూన్య *ptr_fl, u16 లెక్క) {
    సమితి_SEG(ES, FLATPTR_గమ్యం_SEG(ptr_fl));
    outsl(పోర్టు, (u32*)FLATPTR_గమ్యం_స్థానభ్రంశం(ptr_fl), లెక్క);
}

#లేకపోతే

// In 32-bit flat mode there is no need to mess with the segments.
#నిర్వచించు GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#నిర్వచించు సమితి_FARVAR(seg, var, val) \
    చేయి { GET_FARVAR((seg), (var)) = (val); } అయితే (0)
#నిర్వచించు GET_VAR(seg, var) (var)
#నిర్వచించు సమితి_VAR(seg, var, val) చేయి { (var) = (val); } అయితే (0)
#నిర్వచించు సమితి_SEG(SEG, విలువ) ((శూన్య)(విలువ))
#నిర్వచించు GET_SEG(SEG) 0
#నిర్వచించు GET_FLATPTR(ptr) (ptr)
#నిర్వచించు సమితి_FLATPTR(ptr, val) చేయి { (ptr) = (val); } అయితే (0)

#నిర్వచించు insb_fl(పోర్టు, ptr_fl, లెక్క) insb(పోర్టు, ptr_fl, లెక్క)
#నిర్వచించు insw_fl(పోర్టు, ptr_fl, లెక్క) insw(పోర్టు, ptr_fl, లెక్క)
#నిర్వచించు insl_fl(పోర్టు, ptr_fl, లెక్క) insl(పోర్టు, ptr_fl, లెక్క)
#నిర్వచించు outsb_fl(పోర్టు, ptr_fl, లెక్క) outsb(పోర్టు, ptr_fl, లెక్క)
#నిర్వచించు outsw_fl(పోర్టు, ptr_fl, లెక్క) outsw(పోర్టు, ptr_fl, లెక్క)
#నిర్వచించు outsl_fl(పోర్టు, ptr_fl, లెక్క) outsl(పోర్టు, ptr_fl, లెక్క)

#అయితే_ముగింపు

#నిర్వచించు SEGOFF(s,o) ({నిర్మాణం segoff_s __so; __so.స్థానభ్రంశం_2=(o); __so.seg=(s); __so;})

స్థిర అంతఃవరుస నిర్మాణం segoff_s FLATPTR_గమ్యం_SEGOFF(శూన్య *p) {
    తిరిగి SEGOFF(FLATPTR_గమ్యం_SEG(p), FLATPTR_గమ్యం_స్థానభ్రంశం(p));
}
స్థిర అంతఃవరుస శూన్య *SEGOFF_గమ్యం_FLATPTR(నిర్మాణం segoff_s so) {
    తిరిగి MAKE_FLATPTR(so.seg, so.స్థానభ్రంశం_2);
}

#అయితే_ముగింపు // farptr.h
