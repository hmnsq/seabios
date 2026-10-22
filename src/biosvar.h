// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#അന്തര്ഭാവ "config.h" // SEG_BDA
#അന്തര്ഭാവ "farptr.h" // GET_FARVAR
#അന്തര്ഭാവ "memmap.h" //ചിഹ്നം
#അന്തര്ഭാവ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((ഘടന rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((ഘടന rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        ബാഹ്യ ശൂന്യം func (ശൂന്യം);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((ഘടന bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((ഘടന bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
സ്ഥിതം വരിയിൽ ശൂന്യം set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(വലുപ്പം(ഘടന extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
സ്ഥിതം വരിയിൽ u16 get_ebda_seg(ശൂന്യം) {
    തിരികെ GET_BDA(ebda_seg);
}
സ്ഥിതം വരിയിൽ ഘടന extended_bios_data_area_s *
get_ebda_ptr(ശൂന്യം)
{
    ASSERT32FLAT();
    തിരികെ MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((ഘടന extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((ഘടന extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#എങ്കിൽ MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
സ്ഥിതം വരിയിൽ u32 __attribute_const get_global_offset(ശൂന്യം) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    തിരികെ ret;
}
#അല്ലെങ്കിൽ
#define GLOBAL_SEGREG CS
സ്ഥിതം വരിയിൽ u32 __attribute_const get_global_offset(ശൂന്യം) {
    തിരികെ 0;
}
#endif
സ്ഥിതം വരിയിൽ u16 get_global_seg(ശൂന്യം) {
    തിരികെ GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((ശൂന്യം*)&(var)              \
                                             + get_global_offset()))
#എങ്കിൽ MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((ശൂന്യം*)(var) - BUILD_BIOS_ADDR))
#അല്ലെങ്കിൽ
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_താഴെയുള്ള ചിഹ്നം(_zonelow_seg)

#എങ്കിൽ MODESEGMENT
#define GET_താഴെയുള്ള(var)            GET_FARVAR(SEG_താഴെയുള്ള, (var))
#define SET_താഴെയുള്ള(var, val)       SET_FARVAR(SEG_താഴെയുള്ള, (var), (val))
#define LOWFLAT2_താഴെയുള്ള(var) ((typeof(var))((ശൂന്യം*)(var) - ചിഹ്നം(zonelow_base)))
#അല്ലെങ്കിൽ
#define GET_താഴെയുള്ള(var)            (var)
#define SET_താഴെയുള്ള(var, val)       ചെയ്യുക { (var) = (val); } ആകുമ്പോൾ (0)
#define LOWFLAT2_താഴെയുള്ള(var) (var)
#endif
#define GET_LOWFLAT(var) GET_താഴെയുള്ള(*LOWFLAT2_താഴെയുള്ള(&(var)))
#define SET_LOWFLAT(var, val) SET_താഴെയുള്ള(*LOWFLAT2_താഴെയുള്ള(&(var)), (val))

#endif // __BIOSVAR_H
