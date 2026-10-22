// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#جيڪڏهن_وصف_نه_ٿيل __BIOSVAR_H
#وصف_ڪريو __BIOSVAR_H

#ضمن "config.h" // SEG_BDA
#ضمن "farptr.h" // GET_FARVAR
#ضمن "memmap.h" //نشان
#ضمن "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#وصف_ڪريو GET_IVT(ويڪٽر)                                         \
    GET_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[ويڪٽر])
#وصف_ڪريو سيٽ_IVT(ويڪٽر, segoff)                                         \
    سيٽ_FARVAR(SEG_IVT, ((بنية rmode_IVT *)0)->ivec[ويڪٽر], segoff)

#وصف_ڪريو FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        خارجي فراغ func (فراغ);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#وصف_ڪريو GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((بنية bios_معلومات_area_s *)0)->var)
#وصف_ڪريو سيٽ_BDA(var, val) \
    سيٽ_FARVAR(SEG_BDA, ((بنية bios_معلومات_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ساكن مضمن فراغ سيٽ_equipment_flags(u16 clear, u16 سيٽ) {
    u16 eqf = GET_BDA(equipment_list_flags);
    سيٽ_BDA(equipment_list_flags, (eqf & ~clear) | سيٽ);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#وصف_ڪريو EBDA_سائيز_شروعات \
    DIV_ڦيرو_مٿي(حجم(بنية extended_bios_معلومات_area_s), 1024)
#وصف_ڪريو EBDA_SEGMENT_شروعات \
    FLATPTR_منزل_SEG(BUILD_LOWRAM_پڇاڙي - EBDA_سائيز_شروعات*1024)

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
#وصف_ڪريو GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((بنية extended_bios_معلومات_area_s *)0)->var)
#وصف_ڪريو سيٽ_EBDA(eseg, var, val)                                        \
    سيٽ_FARVAR(eseg, ((بنية extended_bios_معلومات_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#إذا MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#وصف_ڪريو GLOBAL_SEGREG GS
ساكن مضمن u32 __attribute_const get_global_وٿي(فراغ) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    أرجع ret;
}
#وإلا
#وصف_ڪريو GLOBAL_SEGREG CS
ساكن مضمن u32 __attribute_const get_global_وٿي(فراغ) {
    أرجع 0;
}
#جيڪڏهن_پورو
ساكن مضمن u16 get_global_seg(فراغ) {
    أرجع GET_SEG(GLOBAL_SEGREG);
}
#وصف_ڪريو GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((فراغ*)&(var)              \
                                             + get_global_وٿي()))
#إذا MODESEGMENT
#وصف_ڪريو GLOBALFLAT2GLOBAL(var) ((typeof(var))((فراغ*)(var) - BUILD_BIOS_ADDR))
#وإلا
#وصف_ڪريو GLOBALFLAT2GLOBAL(var) (var)
#جيڪڏهن_پورو
// Access a "flat" pointer known to point to the f-segment.
#وصف_ڪريو GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#وصف_ڪريو SEG_هيٺيون نشان(_zonelow_seg)

#إذا MODESEGMENT
#وصف_ڪريو GET_هيٺيون(var)            GET_FARVAR(SEG_هيٺيون, (var))
#وصف_ڪريو سيٽ_هيٺيون(var, val)       سيٽ_FARVAR(SEG_هيٺيون, (var), (val))
#وصف_ڪريو LOWFLAT2_هيٺيون(var) ((typeof(var))((فراغ*)(var) - نشان(zonelow_بنياد)))
#وإلا
#وصف_ڪريو GET_هيٺيون(var)            (var)
#وصف_ڪريو سيٽ_هيٺيون(var, val)       افعل { (var) = (val); } طالما (0)
#وصف_ڪريو LOWFLAT2_هيٺيون(var) (var)
#جيڪڏهن_پورو
#وصف_ڪريو GET_LOWFLAT(var) GET_هيٺيون(*LOWFLAT2_هيٺيون(&(var)))
#وصف_ڪريو سيٽ_LOWFLAT(var, val) سيٽ_هيٺيون(*LOWFLAT2_هيٺيون(&(var)), (val))

#جيڪڏهن_پورو // __BIOSVAR_H
