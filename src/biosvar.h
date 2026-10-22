// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#ಅನ್ತರ್ಭಾವ "config.h" // SEG_BDA
#ಅನ್ತರ್ಭಾವ "farptr.h" // GET_FARVAR
#ಅನ್ತರ್ಭಾವ "memmap.h" //ಚಿಹ್ನೆ
#ಅನ್ತರ್ಭಾವ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((ಸಂರಚನಾ rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((ಸಂರಚನಾ rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        ಬಾಹ್ಯ ಶೂನ್ಯ func (ಶೂನ್ಯ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((ಸಂರಚನಾ bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((ಸಂರಚನಾ bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(ಪರಿಮಾಣ(ಸಂರಚನಾ extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 get_ebda_seg(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ GET_BDA(ebda_seg);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ extended_bios_data_area_s *
get_ebda_ptr(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಪ್ರತ್ಯಾಗಮ MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((ಸಂರಚನಾ extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((ಸಂರಚನಾ extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#ಯದಿ MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 __attribute_const get_global_offset(ಶೂನ್ಯ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    ಪ್ರತ್ಯಾಗಮ ret;
}
#ಅನ್ಯಥಾ
#define GLOBAL_SEGREG CS
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 __attribute_const get_global_offset(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ 0;
}
#endif
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 get_global_seg(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((ಶೂನ್ಯ*)&(var)              \
                                             + get_global_offset()))
#ಯದಿ MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((ಶೂನ್ಯ*)(var) - BUILD_BIOS_ADDR))
#ಅನ್ಯಥಾ
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_ಕೆಳಗಿನ ಚಿಹ್ನೆ(_zonelow_seg)

#ಯದಿ MODESEGMENT
#define GET_ಕೆಳಗಿನ(var)            GET_FARVAR(SEG_ಕೆಳಗಿನ, (var))
#define SET_ಕೆಳಗಿನ(var, val)       SET_FARVAR(SEG_ಕೆಳಗಿನ, (var), (val))
#define LOWFLAT2_ಕೆಳಗಿನ(var) ((typeof(var))((ಶೂನ್ಯ*)(var) - ಚಿಹ್ನೆ(zonelow_base)))
#ಅನ್ಯಥಾ
#define GET_ಕೆಳಗಿನ(var)            (var)
#define SET_ಕೆಳಗಿನ(var, val)       ಕುರು { (var) = (val); } ಯಾವತ್ (0)
#define LOWFLAT2_ಕೆಳಗಿನ(var) (var)
#endif
#define GET_LOWFLAT(var) GET_ಕೆಳಗಿನ(*LOWFLAT2_ಕೆಳಗಿನ(&(var)))
#define SET_LOWFLAT(var, val) SET_ಕೆಳಗಿನ(*LOWFLAT2_ಕೆಳಗಿನ(&(var)), (val))

#endif // __BIOSVAR_H
