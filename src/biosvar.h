// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#अगर_अघोषित __BIOSVAR_H
#घोषणा __BIOSVAR_H

#समावेश "config.h" // SEG_BDA
#समावेश "farptr.h" // GET_FARVAR
#समावेश "memmap.h" //प्रतीक
#समावेश "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#घोषणा GET_IVT(vector)                                         \
    GET_FARVAR(SEG_IVT, ((काष्ठा rmode_IVT *)0)->ivec[vector])
#घोषणा SET_IVT(vector, segoff)                                         \
    SET_FARVAR(SEG_IVT, ((काष्ठा rmode_IVT *)0)->ivec[vector], segoff)

#घोषणा FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        बाह्य व्योम func (व्योम);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#घोषणा GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((काष्ठा bios_तथ्य_area_s *)0)->var)
#घोषणा SET_BDA(var, val) \
    SET_FARVAR(SEG_BDA, ((काष्ठा bios_तथ्य_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
अटल अंतरभूत व्योम set_equipment_flags(u16 clear, u16 set) {
    u16 eqf = GET_BDA(equipment_list_flags);
    SET_BDA(equipment_list_flags, (eqf & ~clear) | set);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#घोषणा EBDA_आकार_START \
    DIV_ROUND_UP(माप(काष्ठा extended_bios_तथ्य_area_s), 1024)
#घोषणा EBDA_SEGMENT_START \
    FLATPTR_TO_SEG(BUILD_LOWRAM_END - EBDA_आकार_START*1024)

// Accessor functions
अटल अंतरभूत u16 get_ebda_seg(व्योम) {
    वापस GET_BDA(ebda_seg);
}
अटल अंतरभूत काष्ठा extended_bios_तथ्य_area_s *
get_ebda_ptr(व्योम)
{
    ASSERT32FLAT();
    वापस MAKE_FLATPTR(get_ebda_seg(), 0);
}
#घोषणा GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((काष्ठा extended_bios_तथ्य_area_s *)0)->var)
#घोषणा SET_EBDA(eseg, var, val)                                        \
    SET_FARVAR(eseg, ((काष्ठा extended_bios_तथ्य_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#अगर MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#घोषणा GLOBAL_SEGREG GS
अटल अंतरभूत u32 __attribute_const get_global_offset(व्योम) {
    u32 ret;
    यंत्र("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    वापस ret;
}
#अन्यथा
#घोषणा GLOBAL_SEGREG CS
अटल अंतरभूत u32 __attribute_const get_global_offset(व्योम) {
    वापस 0;
}
#पूर्ण_अगर
अटल अंतरभूत u16 get_global_seg(व्योम) {
    वापस GET_SEG(GLOBAL_SEGREG);
}
#घोषणा GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((व्योम*)&(var)              \
                                             + get_global_offset()))
#अगर MODESEGMENT
#घोषणा GLOBALFLAT2GLOBAL(var) ((typeof(var))((व्योम*)(var) - BUILD_BIOS_ADDR))
#अन्यथा
#घोषणा GLOBALFLAT2GLOBAL(var) (var)
#पूर्ण_अगर
// Access a "flat" pointer known to point to the f-segment.
#घोषणा GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#घोषणा SEG_निचला प्रतीक(_zonelow_seg)

#अगर MODESEGMENT
#घोषणा GET_निचला(var)            GET_FARVAR(SEG_निचला, (var))
#घोषणा SET_निचला(var, val)       SET_FARVAR(SEG_निचला, (var), (val))
#घोषणा LOWFLAT2_निचला(var) ((typeof(var))((व्योम*)(var) - प्रतीक(zonelow_base)))
#अन्यथा
#घोषणा GET_निचला(var)            (var)
#घोषणा SET_निचला(var, val)       करो { (var) = (val); } जबतक (0)
#घोषणा LOWFLAT2_निचला(var) (var)
#पूर्ण_अगर
#घोषणा GET_LOWFLAT(var) GET_निचला(*LOWFLAT2_निचला(&(var)))
#घोषणा SET_LOWFLAT(var, val) SET_निचला(*LOWFLAT2_निचला(&(var)), (val))

#पूर्ण_अगर // __BIOSVAR_H
