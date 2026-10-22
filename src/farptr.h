// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#अगर_अघोषित __FARPTR_H
#घोषणा __FARPTR_H

#समावेश "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
बाह्य u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
बाह्य u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#घोषणा READ8_SEG(prefix, SEG, मूल्य, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(मूल्य)     \
            : "m"(var), "m"(__segment_ ## SEG))
#घोषणा READ16_SEG(prefix, SEG, मूल्य, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(मूल्य)     \
            : "m"(var), "m"(__segment_ ## SEG))
#घोषणा READ32_SEG(prefix, SEG, मूल्य, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(मूल्य)      \
            : "m"(var), "m"(__segment_ ## SEG))
#घोषणा READ64_SEG(prefix, SEG, मूल्य, var) करो {                \
        जोड़ u64_u32_u __value;                                \
        जोड़ u64_u32_u *__r64_ptr = (जोड़ u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(मूल्य) = __value.val;                          \
    } जबतक (0)
#घोषणा WRITE8_SEG(prefix, SEG, var, मूल्य)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(मूल्य), "m"(__segment_ ## SEG))
#घोषणा WRITE16_SEG(prefix, SEG, var, मूल्य)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(मूल्य), "m"(__segment_ ## SEG))
#घोषणा WRITE32_SEG(prefix, SEG, var, मूल्य)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(मूल्य), "m"(__segment_ ## SEG))
#घोषणा WRITE64_SEG(prefix, SEG, var, मूल्य) करो {               \
        जोड़ u64_u32_u __value;                                \
        जोड़ u64_u32_u *__w64_ptr = (जोड़ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (मूल्य);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } जबतक (0)

// Macros for automatically choosing the appropriate memory size
// access method.
बाह्य व्योम __force_link_error__unknown_type(व्योम);

#घोषणा __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    अगर (माप(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    अन्यथा अगर (माप(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    अन्यथा अगर (माप(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    अन्यथा अगर (माप(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    अन्यथा                                        \
        __force_link_error__unknown_type();     \
    __val; })

#घोषणा __SET_VAR(prefix, seg, var, val) करो {           \
        अगर (माप(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        अन्यथा अगर (माप(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        अन्यथा अगर (माप(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        अन्यथा अगर (माप(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        अन्यथा                                            \
            __force_link_error__unknown_type();         \
    } जबतक (0)

#घोषणा DECL_SEGFUNCS(SEG)                              \
अटल अंतरभूत व्योम __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
अटल अंतरभूत u16 __get_seg_##SEG(व्योम) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    वापस res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#घोषणा __SET_SEG(SEG, मूल्य)                   \
    __set_seg_##SEG(मूल्य)
#घोषणा __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#घोषणा __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#घोषणा __SET_FARVAR(seg, var, val) करो {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } जबतक (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#घोषणा __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#घोषणा __SET_FLATPTR(ptr, val) करो {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } जबतक (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#घोषणा FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#घोषणा FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#घोषणा MAKE_FLATPTR(seg,off) ((व्योम*)(((u32)(seg)<<4)+(u32)(off)))


#अगर MODESEGMENT == 1

// Definitions when using segmented mode.
#घोषणा GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#घोषणा SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#घोषणा GET_VAR(seg, var) __GET_VAR("", seg, (var))
#घोषणा SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#घोषणा SET_SEG(SEG, मूल्य) __SET_SEG(SEG, (मूल्य))
#घोषणा GET_SEG(SEG) __GET_SEG(SEG)
#घोषणा GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#घोषणा SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

अटल अंतरभूत व्योम insb_fl(u16 port, व्योम *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
अटल अंतरभूत व्योम insw_fl(u16 port, व्योम *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
अटल अंतरभूत व्योम insl_fl(u16 port, व्योम *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
अटल अंतरभूत व्योम outsb_fl(u16 port, व्योम *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
अटल अंतरभूत व्योम outsw_fl(u16 port, व्योम *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}
अटल अंतरभूत व्योम outsl_fl(u16 port, व्योम *ptr_fl, u16 गिनती) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), गिनती);
}

#अन्यथा

// In 32-bit flat mode there is no need to mess with the segments.
#घोषणा GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#घोषणा SET_FARVAR(seg, var, val) \
    करो { GET_FARVAR((seg), (var)) = (val); } जबतक (0)
#घोषणा GET_VAR(seg, var) (var)
#घोषणा SET_VAR(seg, var, val) करो { (var) = (val); } जबतक (0)
#घोषणा SET_SEG(SEG, मूल्य) ((व्योम)(मूल्य))
#घोषणा GET_SEG(SEG) 0
#घोषणा GET_FLATPTR(ptr) (ptr)
#घोषणा SET_FLATPTR(ptr, val) करो { (ptr) = (val); } जबतक (0)

#घोषणा insb_fl(port, ptr_fl, गिनती) insb(port, ptr_fl, गिनती)
#घोषणा insw_fl(port, ptr_fl, गिनती) insw(port, ptr_fl, गिनती)
#घोषणा insl_fl(port, ptr_fl, गिनती) insl(port, ptr_fl, गिनती)
#घोषणा outsb_fl(port, ptr_fl, गिनती) outsb(port, ptr_fl, गिनती)
#घोषणा outsw_fl(port, ptr_fl, गिनती) outsw(port, ptr_fl, गिनती)
#घोषणा outsl_fl(port, ptr_fl, गिनती) outsl(port, ptr_fl, गिनती)

#पूर्ण_अगर

#घोषणा SEGOFF(s,o) ({काष्ठा segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

अटल अंतरभूत काष्ठा segoff_s FLATPTR_TO_SEGOFF(व्योम *p) {
    वापस SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
अटल अंतरभूत व्योम *SEGOFF_TO_FLATPTR(काष्ठा segoff_s so) {
    वापस MAKE_FLATPTR(so.seg, so.offset);
}

#पूर्ण_अगर // farptr.h
