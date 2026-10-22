// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#অন্তর্ভাৱ "config.h" // SEG_BDA
#অন্তর্ভাৱ "farptr.h" // GET_FARVAR
#অন্তর্ভাৱ "memmap.h" //প্ৰতীক
#অন্তর্ভাৱ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((সংরচনা rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((সংরচনা rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        বাহ্য শূন্য func (শূন্য);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((সংরচনা bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((সংরচনা bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
স্থির অন্তঃস্থ শূন্য set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(পরিমাণ(সংরচনা extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
স্থির অন্তঃস্থ u16 get_ebda_seg(শূন্য) {
    ঘূৰাই GET_BDA(ebda_seg);
}
স্থির অন্তঃস্থ সংরচনা extended_bios_data_area_s *
get_ebda_ptr(শূন্য)
{
    ASSERT32FLAT();
    ঘূৰাই MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((সংরচনা extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((সংরচনা extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#যদি MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
স্থির অন্তঃস্থ u32 __attribute_const get_global_offset(শূন্য) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    ঘূৰাই ret;
}
#অন্যথা
#define GLOBAL_SEGREG CS
স্থির অন্তঃস্থ u32 __attribute_const get_global_offset(শূন্য) {
    ঘূৰাই 0;
}
#endif
স্থির অন্তঃস্থ u16 get_global_seg(শূন্য) {
    ঘূৰাই GET_SEG(GLOBAL_SEGREG);
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

#define SEG_নিচেৰ প্ৰতীক(_zonelow_seg)

#যদি MODESEGMENT
#define GET_নিচেৰ(var)            GET_FARVAR(SEG_নিচেৰ, (var))
#define SET_নিচেৰ(var, val)       SET_FARVAR(SEG_নিচেৰ, (var), (val))
#define LOWFLAT2_নিচেৰ(var) ((typeof(var))((শূন্য*)(var) - প্ৰতীক(zonelow_base)))
#অন্যথা
#define GET_নিচেৰ(var)            (var)
#define SET_নিচেৰ(var, val)       কৰা { (var) = (val); } যেতিয়ালৈ (0)
#define LOWFLAT2_নিচেৰ(var) (var)
#endif
#define GET_LOWFLAT(var) GET_নিচেৰ(*LOWFLAT2_নিচেৰ(&(var)))
#define SET_LOWFLAT(var, val) SET_নিচেৰ(*LOWFLAT2_নিচেৰ(&(var)), (val))

#endif // __BIOSVAR_H
