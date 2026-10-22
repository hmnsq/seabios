// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#अन्तर्भाव "config.h" // SEG_BDA
#अन्तर्भाव "farptr.h" // GET_FARVAR
#अन्तर्भाव "memmap.h" //प्रतीक
#अन्तर्भाव "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((संरचना rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((संरचना rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        बाह्य शून्य func (शून्य);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((संरचना bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((संरचना bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
स्थिर अन्तःस्थ शून्य set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(परिमाण(संरचना extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
स्थिर अन्तःस्थ u16 get_ebda_seg(शून्य) {
    प्रत्यागम GET_BDA(ebda_seg);
}
स्थिर अन्तःस्थ संरचना extended_bios_data_area_s *
get_ebda_ptr(शून्य)
{
    ASSERT32FLAT();
    प्रत्यागम MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((संरचना extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((संरचना extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#यदि MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
स्थिर अन्तःस्थ u32 __attribute_const get_global_offset(शून्य) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    प्रत्यागम ret;
}
#अन्यथा
#define GLOBAL_SEGREG CS
स्थिर अन्तःस्थ u32 __attribute_const get_global_offset(शून्य) {
    प्रत्यागम 0;
}
#endif
स्थिर अन्तःस्थ u16 get_global_seg(शून्य) {
    प्रत्यागम GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((शून्य*)&(var)              \
                                             + get_global_offset()))
#यदि MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((शून्य*)(var) - BUILD_BIOS_ADDR))
#अन्यथा
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_निचला प्रतीक(_zonelow_seg)

#यदि MODESEGMENT
#define GET_निचला(var)            GET_FARVAR(SEG_निचला, (var))
#define SET_निचला(var, val)       SET_FARVAR(SEG_निचला, (var), (val))
#define LOWFLAT2_निचला(var) ((typeof(var))((शून्य*)(var) - प्रतीक(zonelow_base)))
#अन्यथा
#define GET_निचला(var)            (var)
#define SET_निचला(var, val)       कुरु { (var) = (val); } यावत् (0)
#define LOWFLAT2_निचला(var) (var)
#endif
#define GET_LOWFLAT(var) GET_निचला(*LOWFLAT2_निचला(&(var)))
#define SET_LOWFLAT(var, val) SET_निचला(*LOWFLAT2_निचला(&(var)), (val))

#endif // __BIOSVAR_H
