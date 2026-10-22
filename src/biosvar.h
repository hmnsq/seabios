// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#অন্তর্ভাব "config.h" // SEG_BDA
#অন্তর্ভাব "farptr.h" // GET_FARVAR
#অন্তর্ভাব "memmap.h" //প্রতীক
#অন্তর্ভাব "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((কাঠামো rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((কাঠামো rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        বাহ্যিক শূন্য func (শূন্য);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((কাঠামো bios_তথ্য_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((কাঠামো bios_তথ্য_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
স্থির অন্তঃরেখ শূন্য set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_আকার_START \
    DIV_ROUND_UP(আকার(কাঠামো extended_bios_তথ্য_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_আকার_START*1024)

// Accessor functions
স্থির অন্তঃরেখ u16 get_ebda_seg(শূন্য) {
    ফেরত GET_BDA(ebda_seg);
}
স্থির অন্তঃরেখ কাঠামো extended_bios_তথ্য_area_s *
get_ebda_ptr(শূন্য)
{
    ASSERT32FLAT();
    ফেরত MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((কাঠামো extended_bios_তথ্য_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((কাঠামো extended_bios_তথ্য_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#যদি MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
স্থির অন্তঃরেখ u32 __attribute_const get_global_offset(শূন্য) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    ফেরত ret;
}
#অন্যথা
#define GLOBAL_SEGREG CS
স্থির অন্তঃরেখ u32 __attribute_const get_global_offset(শূন্য) {
    ফেরত 0;
}
#endif
স্থির অন্তঃরেখ u16 get_global_seg(শূন্য) {
    ফেরত GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((শূন্য*)&(var)              \
                                             + get_global_offset()))
#যদি MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((শূন্য*)(var) - BUILD_BIOS_ADDR))
#অন্যথা
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_নিচের প্রতীক(_zonelow_seg)

#যদি MODESEGMENT
#define GET_নিচের(var)            GET_FARVAR(SEG_নিচের, (var))
#define SET_নিচের(var, val)       SET_FARVAR(SEG_নিচের, (var), (val))
#define LOWFLAT2_নিচের(var) ((typeof(var))((শূন্য*)(var) - প্রতীক(zonelow_base)))
#অন্যথা
#define GET_নিচের(var)            (var)
#define SET_নিচের(var, val)       করো { (var) = (val); } যখন (0)
#define LOWFLAT2_নিচের(var) (var)
#endif
#define GET_LOWFLAT(var) GET_নিচের(*LOWFLAT2_নিচের(&(var)))
#define SET_LOWFLAT(var, val) SET_নিচের(*LOWFLAT2_নিচের(&(var)), (val))

#endif // __BIOSVAR_H
