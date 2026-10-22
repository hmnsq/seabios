// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __BIOSVAR_H
#ವ್ಯಾಖ್ಯಾನಿಸು __BIOSVAR_H

#ಅನ್ತರ್ಭಾವ "config.h" // SEG_BDA
#ಅನ್ತರ್ಭಾವ "farptr.h" // GET_FARVAR
#ಅನ್ತರ್ಭಾವ "memmap.h" //ಚಿಹ್ನೆ
#ಅನ್ತರ್ಭಾವ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು GET_IVT(ಸದಿಶ)                                         \
    GET_FARVAR(SEG_IVT, ((ರಚನೆ rmode_IVT *)0)->ivec[ಸದಿಶ])
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_IVT(ಸದಿಶ, segoff)                                         \
    ಗಣ_FARVAR(SEG_IVT, ((ರಚನೆ rmode_IVT *)0)->ivec[ಸದಿಶ], segoff)

#ವ್ಯಾಖ್ಯಾನಿಸು FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        ಬಾಹ್ಯ ಶೂನ್ಯ func (ಶೂನ್ಯ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#ವ್ಯಾಖ್ಯಾನಿಸು GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((ರಚನೆ bios_ದತ್ತಾಂಶ_area_s *)0)->var)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_BDA(var, val) \
    ಗಣ_FARVAR(SEG_BDA, ((ರಚನೆ bios_ದತ್ತಾಂಶ_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಗಣ_equipment_flags(u16 clear, u16 ಗಣ) {
    u16 eqf = GET_BDA(equipment_list_flags);
    ಗಣ_BDA(equipment_list_flags, (eqf & ~clear) | ಗಣ);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#ವ್ಯಾಖ್ಯಾನಿಸು EBDA_ಗಾತ್ರ_ಆರಂಭ \
    DIV_ಸುತ್ತು_ಮೇಲೆ(ಗಾತ್ರ(ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s), 1024)
#ವ್ಯಾಖ್ಯಾನಿಸು EBDA_SEGMENT_ಆರಂಭ \
    FLATPTR_ಗಮ್ಯ_SEG(BUILD_LOWRAM_ಕೊನೆ - EBDA_ಗಾತ್ರ_ಆರಂಭ*1024)

// Accessor functions
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 get_ebda_seg(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ GET_BDA(ebda_seg);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s *
get_ebda_ptr(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಹಿಂತಿರುಗಿ MAKE_FLATPTR(get_ebda_seg(), 0);
}
#ವ್ಯಾಖ್ಯಾನಿಸು GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s *)0)->var)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_EBDA(eseg, var, val)                                        \
    ಗಣ_FARVAR(eseg, ((ರಚನೆ extended_bios_ದತ್ತಾಂಶ_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#ಆದರೆ MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#ವ್ಯಾಖ್ಯಾನಿಸು GLOBAL_SEGREG GS
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 __attribute_const get_global_ಸ್ಥಾನಾಂತರ(ಶೂನ್ಯ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    ಹಿಂತಿರುಗಿ ret;
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು GLOBAL_SEGREG CS
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 __attribute_const get_global_ಸ್ಥಾನಾಂತರ(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 get_global_seg(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ GET_SEG(GLOBAL_SEGREG);
}
#ವ್ಯಾಖ್ಯಾನಿಸು GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((ಶೂನ್ಯ*)&(var)              \
                                             + get_global_ಸ್ಥಾನಾಂತರ()))
#ಆದರೆ MODESEGMENT
#ವ್ಯಾಖ್ಯಾನಿಸು GLOBALFLAT2GLOBAL(var) ((typeof(var))((ಶೂನ್ಯ*)(var) - BUILD_BIOS_ADDR))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು GLOBALFLAT2GLOBAL(var) (var)
#ಆದರೆ_ಅಂತ್ಯ
// Access a "flat" pointer known to point to the f-segment.
#ವ್ಯಾಖ್ಯಾನಿಸು GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು SEG_ಕೆಳಗಿನ ಚಿಹ್ನೆ(_zonelow_seg)

#ಆದರೆ MODESEGMENT
#ವ್ಯಾಖ್ಯಾನಿಸು GET_ಕೆಳಗಿನ(var)            GET_FARVAR(SEG_ಕೆಳಗಿನ, (var))
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_ಕೆಳಗಿನ(var, val)       ಗಣ_FARVAR(SEG_ಕೆಳಗಿನ, (var), (val))
#ವ್ಯಾಖ್ಯಾನಿಸು LOWFLAT2_ಕೆಳಗಿನ(var) ((typeof(var))((ಶೂನ್ಯ*)(var) - ಚಿಹ್ನೆ(zonelow_ತಳ)))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು GET_ಕೆಳಗಿನ(var)            (var)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_ಕೆಳಗಿನ(var, val)       ಮಾಡು { (var) = (val); } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು LOWFLAT2_ಕೆಳಗಿನ(var) (var)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು GET_LOWFLAT(var) GET_ಕೆಳಗಿನ(*LOWFLAT2_ಕೆಳಗಿನ(&(var)))
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_LOWFLAT(var, val) ಗಣ_ಕೆಳಗಿನ(*LOWFLAT2_ಕೆಳಗಿನ(&(var)), (val))

#ಆದರೆ_ಅಂತ್ಯ // __BIOSVAR_H
