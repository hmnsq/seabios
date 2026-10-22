// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#جے_بنا_تعریف __BIOSVAR_H
#تعریف_کرو __BIOSVAR_H

#ضمن "config.h" // SEG_BDA
#ضمن "farptr.h" // GET_FARVAR
#ضمن "memmap.h" //علامت
#ضمن "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#تعریف_کرو GET_IVT(سمتیہ)                                         \
    GET_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[سمتیہ])
#تعریف_کرو سیٹ_IVT(سمتیہ, segoff)                                         \
    سیٹ_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[سمتیہ], segoff)

#تعریف_کرو FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        خارجي فراغ func (فراغ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#تعریف_کرو GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((بنية bios_انکڑے_area_s *)0)->var)
#تعریف_کرو سیٹ_BDA(var, val) \
    سیٹ_FARVAR(SEG_BDA, ((بنية bios_انکڑے_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ساكن مضمن فراغ سیٹ_equipment_flags(u16 clear, u16 سیٹ) {
    u16 eqf = GET_BDA(equipment_list_flags);
    سیٹ_BDA(equipment_list_flags, (eqf & ~clear) | سیٹ);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#تعریف_کرو EBDA_ناپ_شروع \
    DIV_گیڑا_اتے(حجم(بنية extended_bios_انکڑے_area_s), 1024)
#تعریف_کرو EBDA_SEGMENT_شروع \
    FLATPTR_منزل_SEG(BUILD_LOWRAM_سرا - EBDA_ناپ_شروع*1024)

// Accessor functions
ساكن مضمن u16 get_ebda_seg(فراغ) {
    أرجع GET_BDA(ebda_seg);
}
ساكن مضمن بنية extended_bios_انکڑے_area_s *
get_ebda_ptr(فراغ)
{
    ASSERT32FLAT();
    أرجع MAKE_FLATPTR(get_ebda_seg(), 0);
}
#تعریف_کرو GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((بنية extended_bios_انکڑے_area_s *)0)->var)
#تعریف_کرو سیٹ_EBDA(eseg, var, val)                                        \
    سیٹ_FARVAR(eseg, ((بنية extended_bios_انکڑے_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#إذا MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#تعریف_کرو GLOBAL_SEGREG GS
ساكن مضمن u32 __attribute_const get_global_وتھ(فراغ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    أرجع ret;
}
#وإلا
#تعریف_کرو GLOBAL_SEGREG CS
ساكن مضمن u32 __attribute_const get_global_وتھ(فراغ) {
    أرجع 0;
}
#جے_انت
ساكن مضمن u16 get_global_seg(فراغ) {
    أرجع GET_SEG(GLOBAL_SEGREG);
}
#تعریف_کرو GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((فراغ*)&(var)              \
                                             + get_global_وتھ()))
#إذا MODESEGMENT
#تعریف_کرو GLOBALFLAT2GLOBAL(var) ((typeof(var))((فراغ*)(var) - BUILD_BIOS_ADDR))
#وإلا
#تعریف_کرو GLOBALFLAT2GLOBAL(var) (var)
#جے_انت
// Access a "flat" pointer known to point to the f-segment.
#تعریف_کرو GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#تعریف_کرو SEG_کم علامت(_zonelow_seg)

#إذا MODESEGMENT
#تعریف_کرو GET_کم(var)            GET_FARVAR(SEG_کم, (var))
#تعریف_کرو سیٹ_کم(var, val)       سیٹ_FARVAR(SEG_کم, (var), (val))
#تعریف_کرو LOWFLAT2_کم(var) ((typeof(var))((فراغ*)(var) - علامت(zonelow_ادھار)))
#وإلا
#تعریف_کرو GET_کم(var)            (var)
#تعریف_کرو سیٹ_کم(var, val)       افعل { (var) = (val); } طالما (0)
#تعریف_کرو LOWFLAT2_کم(var) (var)
#جے_انت
#تعریف_کرو GET_LOWFLAT(var) GET_کم(*LOWFLAT2_کم(&(var)))
#تعریف_کرو سیٹ_LOWFLAT(var, val) سیٹ_کم(*LOWFLAT2_کم(&(var)), (val))

#جے_انت // __BIOSVAR_H
