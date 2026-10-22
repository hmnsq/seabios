// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#اگر_تعریف_نشده __BIOSVAR_H
#تعریف_کن __BIOSVAR_H

#ضمن "config.h" // SEG_BDA
#ضمن "farptr.h" // GET_FARVAR
#ضمن "memmap.h" //نماد
#ضمن "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#تعریف_کن GET_IVT(وکتور)                                         \
    GET_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[وکتور])
#تعریف_کن مجموعه_IVT(وکتور, segoff)                                         \
    مجموعه_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[وکتور], segoff)

#تعریف_کن FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        خارجي فراغ func (فراغ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#تعریف_کن GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((بنية bios_داده_area_s *)0)->var)
#تعریف_کن مجموعه_BDA(var, val) \
    مجموعه_FARVAR(SEG_BDA, ((بنية bios_داده_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ساكن مضمن فراغ مجموعه_equipment_flags(u16 clear, u16 مجموعه) {
    u16 eqf = GET_BDA(equipment_list_flags);
    مجموعه_BDA(equipment_list_flags, (eqf & ~clear) | مجموعه);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#تعریف_کن EBDA_اندازه_شروع \
    DIV_دور_به_بالا(حجم(بنية extended_bios_داده_area_s), 1024)
#تعریف_کن EBDA_SEGMENT_شروع \
    FLATPTR_مقصد_SEG(BUILD_LOWRAM_انتها - EBDA_اندازه_شروع*1024)

// Accessor functions
ساكن مضمن u16 get_ebda_seg(فراغ) {
    أرجع GET_BDA(ebda_seg);
}
ساكن مضمن بنية extended_bios_داده_area_s *
get_ebda_ptr(فراغ)
{
    ASSERT32FLAT();
    أرجع MAKE_FLATPTR(get_ebda_seg(), 0);
}
#تعریف_کن GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((بنية extended_bios_داده_area_s *)0)->var)
#تعریف_کن مجموعه_EBDA(eseg, var, val)                                        \
    مجموعه_FARVAR(eseg, ((بنية extended_bios_داده_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#إذا MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#تعریف_کن GLOBAL_SEGREG GS
ساكن مضمن u32 __attribute_const get_global_جابه_جایی(فراغ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    أرجع ret;
}
#وإلا
#تعریف_کن GLOBAL_SEGREG CS
ساكن مضمن u32 __attribute_const get_global_جابه_جایی(فراغ) {
    أرجع 0;
}
#پایان_اگر
ساكن مضمن u16 get_global_seg(فراغ) {
    أرجع GET_SEG(GLOBAL_SEGREG);
}
#تعریف_کن GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((فراغ*)&(var)              \
                                             + get_global_جابه_جایی()))
#إذا MODESEGMENT
#تعریف_کن GLOBALFLAT2GLOBAL(var) ((typeof(var))((فراغ*)(var) - BUILD_BIOS_ADDR))
#وإلا
#تعریف_کن GLOBALFLAT2GLOBAL(var) (var)
#پایان_اگر
// Access a "flat" pointer known to point to the f-segment.
#تعریف_کن GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#تعریف_کن SEG_کم نماد(_zonelow_seg)

#إذا MODESEGMENT
#تعریف_کن GET_کم(var)            GET_FARVAR(SEG_کم, (var))
#تعریف_کن مجموعه_کم(var, val)       مجموعه_FARVAR(SEG_کم, (var), (val))
#تعریف_کن LOWFLAT2_کم(var) ((typeof(var))((فراغ*)(var) - نماد(zonelow_پایه)))
#وإلا
#تعریف_کن GET_کم(var)            (var)
#تعریف_کن مجموعه_کم(var, val)       افعل { (var) = (val); } طالما (0)
#تعریف_کن LOWFLAT2_کم(var) (var)
#پایان_اگر
#تعریف_کن GET_LOWFLAT(var) GET_کم(*LOWFLAT2_کم(&(var)))
#تعریف_کن مجموعه_LOWFLAT(var, val) مجموعه_کم(*LOWFLAT2_کم(&(var)), (val))

#پایان_اگر // __BIOSVAR_H
