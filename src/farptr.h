// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#यदि_अपरिभाषितम् __FARPTR_H
#परिभाषय __FARPTR_H

#अन्तर्भाव "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
बाह्य u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
बाह्य u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#परिभाषय READ8_SEG(उपसर्गः, SEG, मानम्, var)                      \
    __asm__(उपसर्गः "movb %%" #SEG ":%1, %b0" : "=Qi"(मानम्)     \
            : "m"(var), "m"(__segment_ ## SEG))
#परिभाषय READ16_SEG(उपसर्गः, SEG, मानम्, var)                     \
    __asm__(उपसर्गः "movw %%" #SEG ":%1, %w0" : "=ri"(मानम्)     \
            : "m"(var), "m"(__segment_ ## SEG))
#परिभाषय READ32_SEG(उपसर्गः, SEG, मानम्, var)                     \
    __asm__(उपसर्गः "movl %%" #SEG ":%1, %0" : "=ri"(मानम्)      \
            : "m"(var), "m"(__segment_ ## SEG))
#परिभाषय READ64_SEG(उपसर्गः, SEG, मानम्, var) कुरु {                \
        सङ्घ u64_u32_u __value;                                \
        सङ्घ u64_u32_u *__r64_ptr = (सङ्घ u64_u32_u *)&(var); \
        READ32_SEG(उपसर्गः, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(उपसर्गः, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(मानम्) = __value.val;                          \
    } यावत् (0)
#परिभाषय WRITE8_SEG(उपसर्गः, SEG, var, मानम्)                     \
    __asm__(उपसर्गः "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(मानम्), "m"(__segment_ ## SEG))
#परिभाषय WRITE16_SEG(उपसर्गः, SEG, var, मानम्)                    \
    __asm__(उपसर्गः "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(मानम्), "m"(__segment_ ## SEG))
#परिभाषय WRITE32_SEG(उपसर्गः, SEG, var, मानम्)                    \
    __asm__(उपसर्गः "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(मानम्), "m"(__segment_ ## SEG))
#परिभाषय WRITE64_SEG(उपसर्गः, SEG, var, मानम्) कुरु {               \
        सङ्घ u64_u32_u __value;                                \
        सङ्घ u64_u32_u *__w64_ptr = (सङ्घ u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (मानम्);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(उपसर्गः, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(उपसर्गः, SEG, __w64_ptr->hi, __value.hi);    \
    } यावत् (0)

// Macros for automatically choosing the appropriate memory size
// access method.
बाह्य शून्य __force_link_error__unknown_type(शून्य);

#परिभाषय __GET_VAR(उपसर्गः, seg, var) ({          \
    typeof(var) __val;                          \
    यदि (परिमाण(__val) == 1)                     \
        READ8_SEG(उपसर्गः, seg, __val, var);     \
    अन्यथा यदि (परिमाण(__val) == 2)                \
        READ16_SEG(उपसर्गः, seg, __val, var);    \
    अन्यथा यदि (परिमाण(__val) == 4)                \
        READ32_SEG(उपसर्गः, seg, __val, var);    \
    अन्यथा यदि (परिमाण(__val) == 8)                \
        READ64_SEG(उपसर्गः, seg, __val, var);    \
    अन्यथा                                        \
        __force_link_error__unknown_type();     \
    __val; })

#परिभाषय __SET_VAR(उपसर्गः, seg, var, val) कुरु {           \
        यदि (परिमाण(var) == 1)                           \
            WRITE8_SEG(उपसर्गः, seg, var, (val));        \
        अन्यथा यदि (परिमाण(var) == 2)                      \
            WRITE16_SEG(उपसर्गः, seg, var, (val));       \
        अन्यथा यदि (परिमाण(var) == 4)                      \
            WRITE32_SEG(उपसर्गः, seg, var, (val));       \
        अन्यथा यदि (परिमाण(var) == 8)                      \
            WRITE64_SEG(उपसर्गः, seg, var, (val));       \
        अन्यथा                                            \
            __force_link_error__unknown_type();         \
    } यावत् (0)

#परिभाषय DECL_SEGFUNCS(SEG)                              \
स्थिर अन्तःस्थ शून्य __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
स्थिर अन्तःस्थ u16 __get_seg_##SEG(शून्य) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    प्रत्यागम res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#परिभाषय __SET_SEG(SEG, मानम्)                   \
    __set_seg_##SEG(मानम्)
#परिभाषय __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#परिभाषय __GET_FARVAR(seg, var) ({               \
    समुच्चयः_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#परिभाषय __SET_FARVAR(seg, var, val) कुरु {        \
        typeof(var) __sfv_val = (val);          \
        समुच्चयः_SEG(ES, (seg));                     \
        समुच्चयः_VAR(ES, (var), __sfv_val);          \
    } यावत् (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#परिभाषय __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_गन्तव्यम्_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_गन्तव्यम्_विस्थापनम्(__ptr)); })
#परिभाषय __SET_FLATPTR(ptr, val) कुरु {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        समुच्चयः_FARVAR(FLATPTR_गन्तव्यम्_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_गन्तव्यम्_विस्थापनम्(__ptr)   \
                   , (val));                                    \
    } यावत् (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#परिभाषय FLATPTR_गन्तव्यम्_SEG(p) (((u32)(p)) >> 4)
#परिभाषय FLATPTR_गन्तव्यम्_विस्थापनम्(p) (((u32)(p)) & 0xf)
#परिभाषय MAKE_FLATPTR(seg,off) ((शून्य*)(((u32)(seg)<<4)+(u32)(off)))


#यदि MODESEGMENT == 1

// Definitions when using segmented mode.
#परिभाषय GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#परिभाषय समुच्चयः_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#परिभाषय GET_VAR(seg, var) __GET_VAR("", seg, (var))
#परिभाषय समुच्चयः_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#परिभाषय समुच्चयः_SEG(SEG, मानम्) __SET_SEG(SEG, (मानम्))
#परिभाषय GET_SEG(SEG) __GET_SEG(SEG)
#परिभाषय GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#परिभाषय समुच्चयः_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

स्थिर अन्तःस्थ शून्य insb_fl(u16 पत्तनम्, शून्य *ptr_fl, u16 गणनम्) {
    समुच्चयः_SEG(ES, FLATPTR_गन्तव्यम्_SEG(ptr_fl));
    insb(पत्तनम्, (u8*)FLATPTR_गन्तव्यम्_विस्थापनम्(ptr_fl), गणनम्);
}
स्थिर अन्तःस्थ शून्य insw_fl(u16 पत्तनम्, शून्य *ptr_fl, u16 गणनम्) {
    समुच्चयः_SEG(ES, FLATPTR_गन्तव्यम्_SEG(ptr_fl));
    insw(पत्तनम्, (u16*)FLATPTR_गन्तव्यम्_विस्थापनम्(ptr_fl), गणनम्);
}
स्थिर अन्तःस्थ शून्य insl_fl(u16 पत्तनम्, शून्य *ptr_fl, u16 गणनम्) {
    समुच्चयः_SEG(ES, FLATPTR_गन्तव्यम्_SEG(ptr_fl));
    insl(पत्तनम्, (u32*)FLATPTR_गन्तव्यम्_विस्थापनम्(ptr_fl), गणनम्);
}
स्थिर अन्तःस्थ शून्य outsb_fl(u16 पत्तनम्, शून्य *ptr_fl, u16 गणनम्) {
    समुच्चयः_SEG(ES, FLATPTR_गन्तव्यम्_SEG(ptr_fl));
    outsb(पत्तनम्, (u8*)FLATPTR_गन्तव्यम्_विस्थापनम्(ptr_fl), गणनम्);
}
स्थिर अन्तःस्थ शून्य outsw_fl(u16 पत्तनम्, शून्य *ptr_fl, u16 गणनम्) {
    समुच्चयः_SEG(ES, FLATPTR_गन्तव्यम्_SEG(ptr_fl));
    outsw(पत्तनम्, (u16*)FLATPTR_गन्तव्यम्_विस्थापनम्(ptr_fl), गणनम्);
}
स्थिर अन्तःस्थ शून्य outsl_fl(u16 पत्तनम्, शून्य *ptr_fl, u16 गणनम्) {
    समुच्चयः_SEG(ES, FLATPTR_गन्तव्यम्_SEG(ptr_fl));
    outsl(पत्तनम्, (u32*)FLATPTR_गन्तव्यम्_विस्थापनम्(ptr_fl), गणनम्);
}

#अन्यथा

// In 32-bit flat mode there is no need to mess with the segments.
#परिभाषय GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#परिभाषय समुच्चयः_FARVAR(seg, var, val) \
    कुरु { GET_FARVAR((seg), (var)) = (val); } यावत् (0)
#परिभाषय GET_VAR(seg, var) (var)
#परिभाषय समुच्चयः_VAR(seg, var, val) कुरु { (var) = (val); } यावत् (0)
#परिभाषय समुच्चयः_SEG(SEG, मानम्) ((शून्य)(मानम्))
#परिभाषय GET_SEG(SEG) 0
#परिभाषय GET_FLATPTR(ptr) (ptr)
#परिभाषय समुच्चयः_FLATPTR(ptr, val) कुरु { (ptr) = (val); } यावत् (0)

#परिभाषय insb_fl(पत्तनम्, ptr_fl, गणनम्) insb(पत्तनम्, ptr_fl, गणनम्)
#परिभाषय insw_fl(पत्तनम्, ptr_fl, गणनम्) insw(पत्तनम्, ptr_fl, गणनम्)
#परिभाषय insl_fl(पत्तनम्, ptr_fl, गणनम्) insl(पत्तनम्, ptr_fl, गणनम्)
#परिभाषय outsb_fl(पत्तनम्, ptr_fl, गणनम्) outsb(पत्तनम्, ptr_fl, गणनम्)
#परिभाषय outsw_fl(पत्तनम्, ptr_fl, गणनम्) outsw(पत्तनम्, ptr_fl, गणनम्)
#परिभाषय outsl_fl(पत्तनम्, ptr_fl, गणनम्) outsl(पत्तनम्, ptr_fl, गणनम्)

#यदि_अन्तः

#परिभाषय SEGOFF(s,o) ({संरचना segoff_s __so; __so.विस्थापनम्_2=(o); __so.seg=(s); __so;})

स्थिर अन्तःस्थ संरचना segoff_s FLATPTR_गन्तव्यम्_SEGOFF(शून्य *p) {
    प्रत्यागम SEGOFF(FLATPTR_गन्तव्यम्_SEG(p), FLATPTR_गन्तव्यम्_विस्थापनम्(p));
}
स्थिर अन्तःस्थ शून्य *SEGOFF_गन्तव्यम्_FLATPTR(संरचना segoff_s so) {
    प्रत्यागम MAKE_FLATPTR(so.seg, so.विस्थापनम्_2);
}

#यदि_अन्तः // farptr.h
