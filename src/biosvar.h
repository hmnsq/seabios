// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#شامل "config.h" // SEG_BDA
#شامل "farptr.h" // GET_FARVAR
#شامل "memmap.h" //نماد
#شامل "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((ساختار rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((ساختار rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        بیرونی تهی func (تهی);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((ساختار bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((ساختار bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ایستا درونخطی تهی set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(اندازه(ساختار extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
ایستا درونخطی u16 get_ebda_seg(تهی) {
    بازگشت GET_BDA(ebda_seg);
}
ایستا درونخطی ساختار extended_bios_data_area_s *
get_ebda_ptr(تهی)
{
    ASSERT32FLAT();
    بازگشت MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((ساختار extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((ساختار extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#اگر MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
ایستا درونخطی u32 __attribute_const get_global_offset(تهی) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    بازگشت ret;
}
#وگرنه
#define GLOBAL_SEGREG CS
ایستا درونخطی u32 __attribute_const get_global_offset(تهی) {
    بازگشت 0;
}
#endif
ایستا درونخطی u16 get_global_seg(تهی) {
    بازگشت GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((تهی*)&(var)              \
                                             + get_global_offset()))
#اگر MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((تهی*)(var) - BUILD_BIOS_ADDR))
#وگرنه
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_کم نماد(_zonelow_seg)

#اگر MODESEGMENT
#define GET_کم(var)            GET_FARVAR(SEG_کم, (var))
#define SET_کم(var, val)       SET_FARVAR(SEG_کم, (var), (val))
#define LOWFLAT2_کم(var) ((typeof(var))((تهی*)(var) - نماد(zonelow_base)))
#وگرنه
#define GET_کم(var)            (var)
#define SET_کم(var, val)       انجامبده { (var) = (val); } وقتی (0)
#define LOWFLAT2_کم(var) (var)
#endif
#define GET_LOWFLAT(var) GET_کم(*LOWFLAT2_کم(&(var)))
#define SET_LOWFLAT(var, val) SET_کم(*LOWFLAT2_کم(&(var)), (val))

#endif // __BIOSVAR_H
