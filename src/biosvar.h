// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#અન્તર્ભાવ "config.h" // SEG_BDA
#અન્તર્ભાવ "farptr.h" // GET_FARVAR
#અન્તર્ભાવ "memmap.h" //પ્રતીક
#અન્તર્ભાવ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((સંરચના rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((સંરચના rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        બાહ્ય શૂન્ય func (શૂન્ય);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((સંરચના bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((સંરચના bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
સ્થિર અંતઃરેખ શૂન્ય set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(કદ(સંરચના extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
સ્થિર અંતઃરેખ u16 get_ebda_seg(શૂન્ય) {
    પાછા GET_BDA(ebda_seg);
}
સ્થિર અંતઃરેખ સંરચના extended_bios_data_area_s *
get_ebda_ptr(શૂન્ય)
{
    ASSERT32FLAT();
    પાછા MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((સંરચના extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((સંરચના extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#જો MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
સ્થિર અંતઃરેખ u32 __attribute_const get_global_offset(શૂન્ય) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    પાછા ret;
}
#નહીંતર
#define GLOBAL_SEGREG CS
સ્થિર અંતઃરેખ u32 __attribute_const get_global_offset(શૂન્ય) {
    પાછા 0;
}
#endif
સ્થિર અંતઃરેખ u16 get_global_seg(શૂન્ય) {
    પાછા GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((શૂન્ય*)&(var)              \
                                             + get_global_offset()))
#જો MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((શૂન્ય*)(var) - BUILD_BIOS_ADDR))
#નહીંતર
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_નીચલું પ્રતીક(_zonelow_seg)

#જો MODESEGMENT
#define GET_નીચલું(var)            GET_FARVAR(SEG_નીચલું, (var))
#define SET_નીચલું(var, val)       SET_FARVAR(SEG_નીચલું, (var), (val))
#define LOWFLAT2_નીચલું(var) ((typeof(var))((શૂન્ય*)(var) - પ્રતીક(zonelow_base)))
#નહીંતર
#define GET_નીચલું(var)            (var)
#define SET_નીચલું(var, val)       કરો { (var) = (val); } જ્યારે (0)
#define LOWFLAT2_નીચલું(var) (var)
#endif
#define GET_LOWFLAT(var) GET_નીચલું(*LOWFLAT2_નીચલું(&(var)))
#define SET_LOWFLAT(var, val) SET_નીચલું(*LOWFLAT2_નીચલું(&(var)), (val))

#endif // __BIOSVAR_H
