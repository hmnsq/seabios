// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __BIOSVAR_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __BIOSVAR_H

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // SEG_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // GET_FARVAR
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" //ਚਿੰਨ੍ਹ
#ਅਨ੍ਤਰ੍ਭਾਵ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_IVT(ਸਦਿਸ਼)                                         \
    GET_FARVAR(SEG_IVT, ((ਬਣਤਰ rmode_IVT *)0)->ivec[ਸਦਿਸ਼])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_IVT(ਸਦਿਸ਼, segoff)                                         \
    ਸੈੱਟ_FARVAR(SEG_IVT, ((ਬਣਤਰ rmode_IVT *)0)->ivec[ਸਦਿਸ਼], segoff)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        ਬਾਹਰੀ ਖਾਲੀ func (ਖਾਲੀ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((ਬਣਤਰ bios_ਅੰਕੜੇ_area_s *)0)->var)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_BDA(var, val) \
    ਸੈੱਟ_FARVAR(SEG_BDA, ((ਬਣਤਰ bios_ਅੰਕੜੇ_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਸੈੱਟ_equipment_flags(u16 clear, u16 ਸੈੱਟ) {
    u16 eqf = GET_BDA(equipment_list_flags);
    ਸੈੱਟ_BDA(equipment_list_flags, (eqf & ~clear) | ਸੈੱਟ);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EBDA_ਮਾਪ_ਸ਼ੁਰੂ \
    DIV_ਗੇੜਾ_ਉੱਪਰ(ਆਕਾਰ(ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s), 1024)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EBDA_SEGMENT_ਸ਼ੁਰੂ \
    FLATPTR_ਮੰਜ਼ਿਲ_SEG(BUILD_LOWRAM_ਸਿਰਾ - EBDA_ਮਾਪ_ਸ਼ੁਰੂ*1024)

// Accessor functions
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 get_ebda_seg(ਖਾਲੀ) {
    ਵਾਪਸ GET_BDA(ebda_seg);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s *
get_ebda_ptr(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਵਾਪਸ MAKE_FLATPTR(get_ebda_seg(), 0);
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s *)0)->var)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_EBDA(eseg, var, val)                                        \
    ਸੈੱਟ_FARVAR(eseg, ((ਬਣਤਰ extended_bios_ਅੰਕੜੇ_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#ਜੇ MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GLOBAL_SEGREG GS
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 __attribute_const get_global_ਵਿੱਥ(ਖਾਲੀ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    ਵਾਪਸ ret;
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GLOBAL_SEGREG CS
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 __attribute_const get_global_ਵਿੱਥ(ਖਾਲੀ) {
    ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 get_global_seg(ਖਾਲੀ) {
    ਵਾਪਸ GET_SEG(GLOBAL_SEGREG);
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((ਖਾਲੀ*)&(var)              \
                                             + get_global_ਵਿੱਥ()))
#ਜੇ MODESEGMENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GLOBALFLAT2GLOBAL(var) ((typeof(var))((ਖਾਲੀ*)(var) - BUILD_BIOS_ADDR))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GLOBALFLAT2GLOBAL(var) (var)
#ਜੇ_ਅੰਤ
// Access a "flat" pointer known to point to the f-segment.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEG_ਨੀਵਾਂ ਚਿੰਨ੍ਹ(_zonelow_seg)

#ਜੇ MODESEGMENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_ਨੀਵਾਂ(var)            GET_FARVAR(SEG_ਨੀਵਾਂ, (var))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਨੀਵਾਂ(var, val)       ਸੈੱਟ_FARVAR(SEG_ਨੀਵਾਂ, (var), (val))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOWFLAT2_ਨੀਵਾਂ(var) ((typeof(var))((ਖਾਲੀ*)(var) - ਚਿੰਨ੍ਹ(zonelow_ਅਧਾਰ)))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_ਨੀਵਾਂ(var)            (var)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਨੀਵਾਂ(var, val)       ਕਰੋ { (var) = (val); } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOWFLAT2_ਨੀਵਾਂ(var) (var)
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_LOWFLAT(var) GET_ਨੀਵਾਂ(*LOWFLAT2_ਨੀਵਾਂ(&(var)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_LOWFLAT(var, val) ਸੈੱਟ_ਨੀਵਾਂ(*LOWFLAT2_ਨੀਵਾਂ(&(var)), (val))

#ਜੇ_ਅੰਤ // __BIOSVAR_H
