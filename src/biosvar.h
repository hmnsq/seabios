// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#include "config.h" // SEG_BDA
#include "farptr.h" // GET_FARVAR
#include "memmap.h" // SYMBOL
#include "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((sa_nracanaa rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((sa_nracanaa rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        extern void func (void);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((sa_nracanaa bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((sa_nracanaa bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
static anta_hstha void set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(parimaa_na(sa_nracanaa extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
static anta_hstha u16 get_ebda_seg(void) {
    ᬯᬮᬶ GET_BDA(ebda_seg);
}
static anta_hstha sa_nracanaa extended_bios_data_area_s *
get_ebda_ptr(void)
{
    ASSERT32FLAT();
    ᬯᬮᬶ MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((sa_nracanaa extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((sa_nracanaa extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#ᬬᬾᬦ᭄ MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
static anta_hstha u32 __attribute_const get_global_offset(void) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    ᬯᬮᬶ ret;
}
#ᬮᬶᬬ
#define GLOBAL_SEGREG CS
static anta_hstha u32 __attribute_const get_global_offset(void) {
    ᬯᬮᬶ 0;
}
#endif
static anta_hstha u16 get_global_seg(void) {
    ᬯᬮᬶ GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((void*)&(var)              \
                                             + get_global_offset()))
#ᬬᬾᬦ᭄ MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((void*)(var) - BUILD_BIOS_ADDR))
#ᬮᬶᬬ
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_LOW SYMBOL(_zonelow_seg)

#ᬬᬾᬦ᭄ MODESEGMENT
#define GET_LOW(var)            GET_FARVAR(SEG_LOW, (var))
#define SET_LOW(var, val)       SET_FARVAR(SEG_LOW, (var), (val))
#define LOWFLAT2LOW(var) ((typeof(var))((void*)(var) - SYMBOL(zonelow_base)))
#ᬮᬶᬬ
#define GET_LOW(var)            (var)
#define SET_LOW(var, val)       do { (var) = (val); } ᬲᭂᬤᭂᬮ (0)
#define LOWFLAT2LOW(var) (var)
#endif
#define GET_LOWFLAT(var) GET_LOW(*LOWFLAT2LOW(&(var)))
#define SET_LOWFLAT(var, val) SET_LOW(*LOWFLAT2LOW(&(var)), (val))

#endif // __BIOSVAR_H
