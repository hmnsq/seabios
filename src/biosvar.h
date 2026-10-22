// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#که_نه_تعریف_شوی __BIOSVAR_H
#تعریف_کړه __BIOSVAR_H

#ضمن "config.h" // SEG_BDA
#ضمن "farptr.h" // GET_FARVAR
#ضمن "memmap.h" //نښه
#ضمن "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#تعریف_کړه GET_IVT(ویکټور)                                         \
    GET_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[ویکټور])
#تعریف_کړه سېټ_IVT(ویکټور, segoff)                                         \
    سېټ_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[ویکټور], segoff)

#تعریف_کړه FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        خارجي فراغ func (فراغ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#تعریف_کړه GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((بنية bios_معلومات_area_s *)0)->var)
#تعریف_کړه سېټ_BDA(var, val) \
    سېټ_FARVAR(SEG_BDA, ((بنية bios_معلومات_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ساكن مضمن فراغ سېټ_equipment_flags(u16 clear, u16 سېټ) {
    u16 eqf = GET_BDA(equipment_list_flags);
    سېټ_BDA(equipment_list_flags, (eqf & ~clear) | سېټ);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#تعریف_کړه EBDA_اندازه_پیل \
    DIV_پړاو_پورته(حجم(بنية extended_bios_معلومات_area_s), 1024)
#تعریف_کړه EBDA_SEGMENT_پیل \
    FLATPTR_منزل_SEG(BUILD_LOWRAM_پای_ټکی - EBDA_اندازه_پیل*1024)

// Accessor functions
ساكن مضمن u16 get_ebda_seg(فراغ) {
    أرجع GET_BDA(ebda_seg);
}
ساكن مضمن بنية extended_bios_معلومات_area_s *
get_ebda_ptr(فراغ)
{
    ASSERT32FLAT();
    أرجع MAKE_FLATPTR(get_ebda_seg(), 0);
}
#تعریف_کړه GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((بنية extended_bios_معلومات_area_s *)0)->var)
#تعریف_کړه سېټ_EBDA(eseg, var, val)                                        \
    سېټ_FARVAR(eseg, ((بنية extended_bios_معلومات_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#إذا MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#تعریف_کړه GLOBAL_SEGREG GS
ساكن مضمن u32 __attribute_const get_global_دننه_واټن(فراغ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    أرجع ret;
}
#وإلا
#تعریف_کړه GLOBAL_SEGREG CS
ساكن مضمن u32 __attribute_const get_global_دننه_واټن(فراغ) {
    أرجع 0;
}
#د_که_پای
ساكن مضمن u16 get_global_seg(فراغ) {
    أرجع GET_SEG(GLOBAL_SEGREG);
}
#تعریف_کړه GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((فراغ*)&(var)              \
                                             + get_global_دننه_واټن()))
#إذا MODESEGMENT
#تعریف_کړه GLOBALFLAT2GLOBAL(var) ((typeof(var))((فراغ*)(var) - BUILD_BIOS_ADDR))
#وإلا
#تعریف_کړه GLOBALFLAT2GLOBAL(var) (var)
#د_که_پای
// Access a "flat" pointer known to point to the f-segment.
#تعریف_کړه GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#تعریف_کړه SEG_ټیټ نښه(_zonelow_seg)

#إذا MODESEGMENT
#تعریف_کړه GET_ټیټ(var)            GET_FARVAR(SEG_ټیټ, (var))
#تعریف_کړه سېټ_ټیټ(var, val)       سېټ_FARVAR(SEG_ټیټ, (var), (val))
#تعریف_کړه LOWFLAT2_ټیټ(var) ((typeof(var))((فراغ*)(var) - نښه(zonelow_بنسټ)))
#وإلا
#تعریف_کړه GET_ټیټ(var)            (var)
#تعریف_کړه سېټ_ټیټ(var, val)       افعل { (var) = (val); } طالما (0)
#تعریف_کړه LOWFLAT2_ټیټ(var) (var)
#د_که_پای
#تعریف_کړه GET_LOWFLAT(var) GET_ټیټ(*LOWFLAT2_ټیټ(&(var)))
#تعریف_کړه سېټ_LOWFLAT(var, val) سېټ_ټیټ(*LOWFLAT2_ټیټ(&(var)), (val))

#د_که_پای // __BIOSVAR_H
