// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#ضمن "config.h" // SEG_BDA
#ضمن "farptr.h" // GET_FARVAR
#ضمن "memmap.h" // SYMBOL
#ضمن "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        خارجي فراغ func (فراغ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((بنية bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((بنية bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ساكن مضمن فراغ set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(حجم(بنية extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
ساكن مضمن u16 get_ebda_seg(فراغ) {
    أرجع GET_BDA(ebda_seg);
}
ساكن مضمن بنية extended_bios_data_area_s *
get_ebda_ptr(فراغ)
{
    ASSERT32FLAT();
    أرجع MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((بنية extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((بنية extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#𐡄𐡍 MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
ساكن مضمن u32 __attribute_const get_global_offset(فراغ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    أرجع ret;
}
#وإلا
#define GLOBAL_SEGREG CS
ساكن مضمن u32 __attribute_const get_global_offset(فراغ) {
    أرجع 0;
}
#endif
ساكن مضمن u16 get_global_seg(فراغ) {
    أرجع GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((فراغ*)&(var)              \
                                             + get_global_offset()))
#𐡄𐡍 MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((فراغ*)(var) - BUILD_BIOS_ADDR))
#وإلا
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_LOW SYMBOL(_zonelow_seg)

#𐡄𐡍 MODESEGMENT
#define GET_LOW(var)            GET_FARVAR(SEG_LOW, (var))
#define SET_LOW(var, val)       SET_FARVAR(SEG_LOW, (var), (val))
#define LOWFLAT2LOW(var) ((typeof(var))((فراغ*)(var) - SYMBOL(zonelow_base)))
#وإلا
#define GET_LOW(var)            (var)
#define SET_LOW(var, val)       𐡏𐡁𐡃 { (var) = (val); } طالما (0)
#define LOWFLAT2LOW(var) (var)
#endif
#define GET_LOWFLAT(var) GET_LOW(*LOWFLAT2LOW(&(var)))
#define SET_LOWFLAT(var, val) SET_LOW(*LOWFLAT2LOW(&(var)), (val))

#endif // __BIOSVAR_H
