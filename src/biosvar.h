// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#ଅନ୍ତର୍ଭାଵ "config.h" // SEG_BDA
#ଅନ୍ତର୍ଭାଵ "farptr.h" // GET_FARVAR
#ଅନ୍ତର୍ଭାଵ "memmap.h" //ପ୍ରତୀକ
#ଅନ୍ତର୍ଭାଵ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((ସଂରଚନା rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((ସଂରଚନା rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        ବାହ୍ୟ ଶୂନ୍ୟ func (ଶୂନ୍ୟ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((ସଂରଚନା bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((ସଂରଚନା bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(ଆକାର(ସଂରଚନା extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 get_ebda_seg(ଶୂନ୍ୟ) {
    ଫେରନ୍ତୁ GET_BDA(ebda_seg);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା extended_bios_data_area_s *
get_ebda_ptr(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଫେରନ୍ତୁ MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((ସଂରଚନା extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((ସଂରଚନା extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#ଯଦି MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 __attribute_const get_global_offset(ଶୂନ୍ୟ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    ଫେରନ୍ତୁ ret;
}
#ଅନ୍ୟଥା
#define GLOBAL_SEGREG CS
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 __attribute_const get_global_offset(ଶୂନ୍ୟ) {
    ଫେରନ୍ତୁ 0;
}
#endif
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 get_global_seg(ଶୂନ୍ୟ) {
    ଫେରନ୍ତୁ GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((ଶୂନ୍ୟ*)&(var)              \
                                             + get_global_offset()))
#ଯଦି MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((ଶୂନ୍ୟ*)(var) - BUILD_BIOS_ADDR))
#ଅନ୍ୟଥା
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_ତଳ ପ୍ରତୀକ(_zonelow_seg)

#ଯଦି MODESEGMENT
#define GET_ତଳ(var)            GET_FARVAR(SEG_ତଳ, (var))
#define SET_ତଳ(var, val)       SET_FARVAR(SEG_ତଳ, (var), (val))
#define LOWFLAT2_ତଳ(var) ((typeof(var))((ଶୂନ୍ୟ*)(var) - ପ୍ରତୀକ(zonelow_base)))
#ଅନ୍ୟଥା
#define GET_ତଳ(var)            (var)
#define SET_ତଳ(var, val)       କର { (var) = (val); } ଯେତେବେଳେ (0)
#define LOWFLAT2_ତଳ(var) (var)
#endif
#define GET_LOWFLAT(var) GET_ତଳ(*LOWFLAT2_ତଳ(&(var)))
#define SET_LOWFLAT(var, val) SET_ତଳ(*LOWFLAT2_ତଳ(&(var)), (val))

#endif // __BIOSVAR_H
