// Code to access multiple segments within gcc.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __FARPTR_H
#define __FARPTR_H

#அந்தர்பாவ "x86.h" // insb

// Dummy definitions used to make sure gcc understands dependencies
// between SET_SEG and GET/READ/WRITE_SEG macros.
வெளி u16 __segment_ES, __segment_CS, __segment_DS, __segment_SS;
வெளி u16 __segment_FS, __segment_GS;

// Low level macros for reading/writing memory via a segment selector.
#define READ8_SEG(prefix, SEG, மதிப்பு, var)                      \
    __asm__(prefix "movb %%" #SEG ":%1, %b0" : "=Qi"(மதிப்பு)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ16_SEG(prefix, SEG, மதிப்பு, var)                     \
    __asm__(prefix "movw %%" #SEG ":%1, %w0" : "=ri"(மதிப்பு)     \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ32_SEG(prefix, SEG, மதிப்பு, var)                     \
    __asm__(prefix "movl %%" #SEG ":%1, %0" : "=ri"(மதிப்பு)      \
            : "m"(var), "m"(__segment_ ## SEG))
#define READ64_SEG(prefix, SEG, மதிப்பு, var) செய் {                \
        ஒன்றிப்பு u64_u32_u __value;                                \
        ஒன்றிப்பு u64_u32_u *__r64_ptr = (ஒன்றிப்பு u64_u32_u *)&(var); \
        READ32_SEG(prefix, SEG, __value.lo, __r64_ptr->lo);     \
        READ32_SEG(prefix, SEG, __value.hi, __r64_ptr->hi);     \
        *(u64*)&(மதிப்பு) = __value.val;                          \
    } போது (0)
#define WRITE8_SEG(prefix, SEG, var, மதிப்பு)                     \
    __asm__(prefix "movb %b1, %%" #SEG ":%0" : "=m"(var)        \
            : "Q"(மதிப்பு), "m"(__segment_ ## SEG))
#define WRITE16_SEG(prefix, SEG, var, மதிப்பு)                    \
    __asm__(prefix "movw %w1, %%" #SEG ":%0" : "=m"(var)        \
            : "r"(மதிப்பு), "m"(__segment_ ## SEG))
#define WRITE32_SEG(prefix, SEG, var, மதிப்பு)                    \
    __asm__(prefix "movl %1, %%" #SEG ":%0" : "=m"(var)         \
            : "r"(மதிப்பு), "m"(__segment_ ## SEG))
#define WRITE64_SEG(prefix, SEG, var, மதிப்பு) செய் {               \
        ஒன்றிப்பு u64_u32_u __value;                                \
        ஒன்றிப்பு u64_u32_u *__w64_ptr = (ஒன்றிப்பு u64_u32_u *)&(var); \
        typeof(var) __value_tmp = (மதிப்பு);                      \
        __value.val = *(u64*)&__value_tmp;                      \
        WRITE32_SEG(prefix, SEG, __w64_ptr->lo, __value.lo);    \
        WRITE32_SEG(prefix, SEG, __w64_ptr->hi, __value.hi);    \
    } போது (0)

// Macros for automatically choosing the appropriate memory size
// access method.
வெளி வெறுமை __force_link_error__unknown_type(வெறுமை);

#define __GET_VAR(prefix, seg, var) ({          \
    typeof(var) __val;                          \
    எனில் (அளவு(__val) == 1)                     \
        READ8_SEG(prefix, seg, __val, var);     \
    இல்லையெனில் எனில் (அளவு(__val) == 2)                \
        READ16_SEG(prefix, seg, __val, var);    \
    இல்லையெனில் எனில் (அளவு(__val) == 4)                \
        READ32_SEG(prefix, seg, __val, var);    \
    இல்லையெனில் எனில் (அளவு(__val) == 8)                \
        READ64_SEG(prefix, seg, __val, var);    \
    இல்லையெனில்                                        \
        __force_link_error__unknown_type();     \
    __val; })

#define __SET_VAR(prefix, seg, var, val) செய் {           \
        எனில் (அளவு(var) == 1)                           \
            WRITE8_SEG(prefix, seg, var, (val));        \
        இல்லையெனில் எனில் (அளவு(var) == 2)                      \
            WRITE16_SEG(prefix, seg, var, (val));       \
        இல்லையெனில் எனில் (அளவு(var) == 4)                      \
            WRITE32_SEG(prefix, seg, var, (val));       \
        இல்லையெனில் எனில் (அளவு(var) == 8)                      \
            WRITE64_SEG(prefix, seg, var, (val));       \
        இல்லையெனில்                                            \
            __force_link_error__unknown_type();         \
    } போது (0)

#define DECL_SEGFUNCS(SEG)                              \
நிலையான உள்வரி வெறுமை __set_seg_##SEG(u16 seg) {           \
    __asm__("movw %w1, %%" #SEG : "=m"(__segment_##SEG) \
            : "rm"(seg));                               \
}                                                       \
நிலையான உள்வரி u16 __get_seg_##SEG(வெறுமை) {               \
    u16 res;                                            \
    __asm__("movw %%" #SEG ", %w0" : "=rm"(res)         \
            : "m"(__segment_##SEG));                    \
    திரும்பு res;                                         \
}
DECL_SEGFUNCS(CS)
DECL_SEGFUNCS(DS)
DECL_SEGFUNCS(ES)
DECL_SEGFUNCS(FS)
DECL_SEGFUNCS(GS)
DECL_SEGFUNCS(SS)

// Low level macros for getting/setting a segment register.
#define __SET_SEG(SEG, மதிப்பு)                   \
    __set_seg_##SEG(மதிப்பு)
#define __GET_SEG(SEG)                          \
    __get_seg_##SEG()

// Macros for accessing a variable in another segment.  (They
// automatically update the %es segment and then make the appropriate
// access.)
#define __GET_FARVAR(seg, var) ({               \
    SET_SEG(ES, (seg));                         \
    GET_VAR(ES, (var)); })
#define __SET_FARVAR(seg, var, val) செய் {        \
        typeof(var) __sfv_val = (val);          \
        SET_SEG(ES, (seg));                     \
        SET_VAR(ES, (var), __sfv_val);          \
    } போது (0)

// Macros for accesssing a 32bit flat mode pointer from 16bit real
// mode.  (They automatically update the %es segment, break the
// pointer into segment/offset, and then make the access.)
#define __GET_FLATPTR(ptr) ({                                   \
    typeof(&(ptr)) __ptr = &(ptr);                              \
    GET_FARVAR(FLATPTR_TO_SEG(__ptr)                            \
               , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)); })
#define __SET_FLATPTR(ptr, val) செய் {                            \
        typeof (&(ptr)) __ptr = &(ptr);                         \
        SET_FARVAR(FLATPTR_TO_SEG(__ptr)                        \
                   , *(typeof(__ptr))FLATPTR_TO_OFFSET(__ptr)   \
                   , (val));                                    \
    } போது (0)

// Macros for converting to/from 32bit flat mode pointers to their
// equivalent 16bit segment/offset values.
#define FLATPTR_TO_SEG(p) (((u32)(p)) >> 4)
#define FLATPTR_TO_OFFSET(p) (((u32)(p)) & 0xf)
#define MAKE_FLATPTR(seg,off) ((வெறுமை*)(((u32)(seg)<<4)+(u32)(off)))


#எனில் MODESEGMENT == 1

// Definitions when using segmented mode.
#define GET_FARVAR(seg, var) __GET_FARVAR((seg), (var))
#define SET_FARVAR(seg, var, val) __SET_FARVAR((seg), (var), (val))
#define GET_VAR(seg, var) __GET_VAR("", seg, (var))
#define SET_VAR(seg, var, val) __SET_VAR("", seg, (var), (val))
#define SET_SEG(SEG, மதிப்பு) __SET_SEG(SEG, (மதிப்பு))
#define GET_SEG(SEG) __GET_SEG(SEG)
#define GET_FLATPTR(ptr) __GET_FLATPTR(ptr)
#define SET_FLATPTR(ptr, val) __SET_FLATPTR((ptr), (val))

நிலையான உள்வரி வெறுமை insb_fl(u16 port, வெறுமை *ptr_fl, u16 மொத்தம்) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), மொத்தம்);
}
நிலையான உள்வரி வெறுமை insw_fl(u16 port, வெறுமை *ptr_fl, u16 மொத்தம்) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), மொத்தம்);
}
நிலையான உள்வரி வெறுமை insl_fl(u16 port, வெறுமை *ptr_fl, u16 மொத்தம்) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    insl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), மொத்தம்);
}
நிலையான உள்வரி வெறுமை outsb_fl(u16 port, வெறுமை *ptr_fl, u16 மொத்தம்) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsb(port, (u8*)FLATPTR_TO_OFFSET(ptr_fl), மொத்தம்);
}
நிலையான உள்வரி வெறுமை outsw_fl(u16 port, வெறுமை *ptr_fl, u16 மொத்தம்) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsw(port, (u16*)FLATPTR_TO_OFFSET(ptr_fl), மொத்தம்);
}
நிலையான உள்வரி வெறுமை outsl_fl(u16 port, வெறுமை *ptr_fl, u16 மொத்தம்) {
    SET_SEG(ES, FLATPTR_TO_SEG(ptr_fl));
    outsl(port, (u32*)FLATPTR_TO_OFFSET(ptr_fl), மொத்தம்);
}

#இல்லையெனில்

// In 32-bit flat mode there is no need to mess with the segments.
#define GET_FARVAR(seg, var) \
    (*((typeof(&(var)))MAKE_FLATPTR((seg), &(var))))
#define SET_FARVAR(seg, var, val) \
    செய் { GET_FARVAR((seg), (var)) = (val); } போது (0)
#define GET_VAR(seg, var) (var)
#define SET_VAR(seg, var, val) செய் { (var) = (val); } போது (0)
#define SET_SEG(SEG, மதிப்பு) ((வெறுமை)(மதிப்பு))
#define GET_SEG(SEG) 0
#define GET_FLATPTR(ptr) (ptr)
#define SET_FLATPTR(ptr, val) செய் { (ptr) = (val); } போது (0)

#define insb_fl(port, ptr_fl, மொத்தம்) insb(port, ptr_fl, மொத்தம்)
#define insw_fl(port, ptr_fl, மொத்தம்) insw(port, ptr_fl, மொத்தம்)
#define insl_fl(port, ptr_fl, மொத்தம்) insl(port, ptr_fl, மொத்தம்)
#define outsb_fl(port, ptr_fl, மொத்தம்) outsb(port, ptr_fl, மொத்தம்)
#define outsw_fl(port, ptr_fl, மொத்தம்) outsw(port, ptr_fl, மொத்தம்)
#define outsl_fl(port, ptr_fl, மொத்தம்) outsl(port, ptr_fl, மொத்தம்)

#endif

#define SEGOFF(s,o) ({கட்டமைப்பு segoff_s __so; __so.offset=(o); __so.seg=(s); __so;})

நிலையான உள்வரி கட்டமைப்பு segoff_s FLATPTR_TO_SEGOFF(வெறுமை *p) {
    திரும்பு SEGOFF(FLATPTR_TO_SEG(p), FLATPTR_TO_OFFSET(p));
}
நிலையான உள்வரி வெறுமை *SEGOFF_TO_FLATPTR(கட்டமைப்பு segoff_s so) {
    திரும்பு MAKE_FLATPTR(so.seg, so.offset);
}

#endif // farptr.h
