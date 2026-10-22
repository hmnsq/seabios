// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#ifndef __BIOSVAR_H
#define __BIOSVAR_H

#அந்தர்பாவ "config.h" // SEG_BDA
#அந்தர்பாவ "farptr.h" // GET_FARVAR
#அந்தர்பாவ "memmap.h" //குறியீடு
#அந்தர்பாவ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#define GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((கட்டமைப்பு rmode_IVT *)0)->ivec[vector])
#define SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((கட்டமைப்பு rmode_IVT *)0)->ivec[vector], segoff)

#define FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        வெளி வெறுமை func (வெறுமை);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#define GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((கட்டமைப்பு bios_data_area_s *)0)->var)
#define SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((கட்டமைப்பு bios_data_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
நிலையான உள்வரி வெறுமை set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#define EBDA_SIZE_START \
    DIV_ROUND_UP(அளவு(கட்டமைப்பு extended_bios_data_area_s), 1024)
#define EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_SIZE_START*1024)

// Accessor functions
நிலையான உள்வரி u16 get_ebda_seg(வெறுமை) {
    திரும்பு GET_BDA(ebda_seg);
}
நிலையான உள்வரி கட்டமைப்பு extended_bios_data_area_s *
get_ebda_ptr(வெறுமை)
{
    ASSERT32FLAT();
    திரும்பு MAKE_FLATPTR(get_ebda_seg(), 0);
}
#define GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((கட்டமைப்பு extended_bios_data_area_s *)0)->var)
#define SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((கட்டமைப்பு extended_bios_data_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#எனில் MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#define GLOBAL_SEGREG GS
நிலையான உள்வரி u32 __attribute_const get_global_offset(வெறுமை) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    திரும்பு ret;
}
#இல்லையெனில்
#define GLOBAL_SEGREG CS
நிலையான உள்வரி u32 __attribute_const get_global_offset(வெறுமை) {
    திரும்பு 0;
}
#endif
நிலையான உள்வரி u16 get_global_seg(வெறுமை) {
    திரும்பு GET_SEG(GLOBAL_SEGREG);
}
#define GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((வெறுமை*)&(var)              \
                                             + get_global_offset()))
#எனில் MODESEGMENT
#define GLOBALFLAT2GLOBAL(var) ((typeof(var))((வெறுமை*)(var) - BUILD_BIOS_ADDR))
#இல்லையெனில்
#define GLOBALFLAT2GLOBAL(var) (var)
#endif
// Access a "flat" pointer known to point to the f-segment.
#define GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#define SEG_கீழ் குறியீடு(_zonelow_seg)

#எனில் MODESEGMENT
#define GET_கீழ்(var)            GET_FARVAR(SEG_கீழ், (var))
#define SET_கீழ்(var, val)       SET_FARVAR(SEG_கீழ், (var), (val))
#define LOWFLAT2_கீழ்(var) ((typeof(var))((வெறுமை*)(var) - குறியீடு(zonelow_base)))
#இல்லையெனில்
#define GET_கீழ்(var)            (var)
#define SET_கீழ்(var, val)       செய் { (var) = (val); } போது (0)
#define LOWFLAT2_கீழ்(var) (var)
#endif
#define GET_LOWFLAT(var) GET_கீழ்(*LOWFLAT2_கீழ்(&(var)))
#define SET_LOWFLAT(var, val) SET_கீழ்(*LOWFLAT2_கீழ்(&(var)), (val))

#endif // __BIOSVAR_H
