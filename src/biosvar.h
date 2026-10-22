// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#यदि_अपरिभाषितम् __BIOSVAR_H
#परिभाषय __BIOSVAR_H

#अन्तर्भाव "config.h" // SEG_BDA
#अन्तर्भाव "farptr.h" // GET_FARVAR
#अन्तर्भाव "memmap.h" //चिह्नम्
#अन्तर्भाव "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#परिभाषय GET_IVT(सदिशम्)                                         \
    GET_FARVAR(SEG_IVT, ((संरचना rmode_IVT *)0)->ivec[सदिशम्])
#परिभाषय समुच्चयः_IVT(सदिशम्, segoff)                                         \
    समुच्चयः_FARVAR(SEG_IVT, ((संरचना rmode_IVT *)0)->ivec[सदिशम्], segoff)

#परिभाषय FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        बाह्य शून्य func (शून्य);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#परिभाषय GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((संरचना bios_दत्त_area_s *)0)->var)
#परिभाषय समुच्चयः_BDA(var, val) \
    समुच्चयः_FARVAR(SEG_BDA, ((संरचना bios_दत्त_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
स्थिर अन्तःस्थ शून्य समुच्चयः_equipment_flags(u16 clear, u16 समुच्चयः) {
    u16 eqf = GET_BDA(equipment_list_flags);
    समुच्चयः_BDA(equipment_list_flags, (eqf & ~clear) | समुच्चयः);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#परिभाषय EBDA_आकार_START \
    DIV_आवर्तनम्_ऊर्ध्वम्(परिमाण(संरचना extended_bios_दत्त_area_s), 1024)
#परिभाषय EBDA_SEGMENT_START \
    FLATPTR_गन्तव्यम्_SEG(BUILD_LOWRAM_अन्तः - EBDA_आकार_START*1024)

// Accessor functions
स्थिर अन्तःस्थ u16 get_ebda_seg(शून्य) {
    प्रत्यागम GET_BDA(ebda_seg);
}
स्थिर अन्तःस्थ संरचना extended_bios_दत्त_area_s *
get_ebda_ptr(शून्य)
{
    ASSERT32FLAT();
    प्रत्यागम MAKE_FLATPTR(get_ebda_seg(), 0);
}
#परिभाषय GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((संरचना extended_bios_दत्त_area_s *)0)->var)
#परिभाषय समुच्चयः_EBDA(eseg, var, val)                                        \
    समुच्चयः_FARVAR(eseg, ((संरचना extended_bios_दत्त_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#यदि MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#परिभाषय GLOBAL_SEGREG GS
स्थिर अन्तःस्थ u32 __attribute_const get_global_विस्थापनम्(शून्य) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    प्रत्यागम ret;
}
#अन्यथा
#परिभाषय GLOBAL_SEGREG CS
स्थिर अन्तःस्थ u32 __attribute_const get_global_विस्थापनम्(शून्य) {
    प्रत्यागम 0;
}
#यदि_अन्तः
स्थिर अन्तःस्थ u16 get_global_seg(शून्य) {
    प्रत्यागम GET_SEG(GLOBAL_SEGREG);
}
#परिभाषय GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((शून्य*)&(var)              \
                                             + get_global_विस्थापनम्()))
#यदि MODESEGMENT
#परिभाषय GLOBALFLAT2GLOBAL(var) ((typeof(var))((शून्य*)(var) - BUILD_BIOS_ADDR))
#अन्यथा
#परिभाषय GLOBALFLAT2GLOBAL(var) (var)
#यदि_अन्तः
// Access a "flat" pointer known to point to the f-segment.
#परिभाषय GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#परिभाषय SEG_नीचः चिह्नम्(_zonelow_seg)

#यदि MODESEGMENT
#परिभाषय GET_नीचः(var)            GET_FARVAR(SEG_नीचः, (var))
#परिभाषय समुच्चयः_नीचः(var, val)       समुच्चयः_FARVAR(SEG_नीचः, (var), (val))
#परिभाषय LOWFLAT2_नीचः(var) ((typeof(var))((शून्य*)(var) - चिह्नम्(zonelow_आधारः)))
#अन्यथा
#परिभाषय GET_नीचः(var)            (var)
#परिभाषय समुच्चयः_नीचः(var, val)       कुरु { (var) = (val); } यावत् (0)
#परिभाषय LOWFLAT2_नीचः(var) (var)
#यदि_अन्तः
#परिभाषय GET_LOWFLAT(var) GET_नीचः(*LOWFLAT2_नीचः(&(var)))
#परिभाषय समुच्चयः_LOWFLAT(var, val) समुच्चयः_नीचः(*LOWFLAT2_नीचः(&(var)), (val))

#यदि_अन्तः // __BIOSVAR_H
