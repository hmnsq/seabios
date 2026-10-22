// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#اگر_غیر_تعریف_شدہ __BIOSVAR_H
#تعریف_کرو __BIOSVAR_H

#شامل "config.h" // SEG_BDA
#شامل "farptr.h" // GET_FARVAR
#شامل "memmap.h" //علامت
#شامل "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#تعریف_کرو GET_IVT(سمتیہ)                                         \
    GET_FARVAR(SEG_IVT, ((ساخت rmode_IVT *)0)->ivec[سمتیہ])
#تعریف_کرو سیٹ_IVT(سمتیہ, segoff)                                         \
    سیٹ_FARVAR(SEG_IVT, ((ساخت rmode_IVT *)0)->ivec[سمتیہ], segoff)

#تعریف_کرو FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        خارجی خالی func (خالی);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#تعریف_کرو GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((ساخت bios_ڈیٹا_area_s *)0)->var)
#تعریف_کرو سیٹ_BDA(var, val) \
    سیٹ_FARVAR(SEG_BDA, ((ساخت bios_ڈیٹا_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
ساکن اندرونی خالی سیٹ_equipment_flags(u16 clear, u16 سیٹ) {
    u16 eqf = GET_BDA(equipment_list_flags);
    سیٹ_BDA(equipment_list_flags, (eqf & ~clear) | سیٹ);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#تعریف_کرو EBDA_سائز_آغاز \
    DIV_باری_اوپر(حجم(ساخت extended_bios_ڈیٹا_area_s), 1024)
#تعریف_کرو EBDA_SEGMENT_آغاز \
    FLATPTR_مقصد_SEG(BUILD_LOWRAM_انتہا - EBDA_سائز_آغاز*1024)

// Accessor functions
ساکن اندرونی u16 get_ebda_seg(خالی) {
    واپس GET_BDA(ebda_seg);
}
ساکن اندرونی ساخت extended_bios_ڈیٹا_area_s *
get_ebda_ptr(خالی)
{
    ASSERT32FLAT();
    واپس MAKE_FLATPTR(get_ebda_seg(), 0);
}
#تعریف_کرو GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((ساخت extended_bios_ڈیٹا_area_s *)0)->var)
#تعریف_کرو سیٹ_EBDA(eseg, var, val)                                        \
    سیٹ_FARVAR(eseg, ((ساخت extended_bios_ڈیٹا_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#اگر MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#تعریف_کرو GLOBAL_SEGREG GS
ساکن اندرونی u32 __attribute_const get_global_انحراف(خالی) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    واپس ret;
}
#ورنہ
#تعریف_کرو GLOBAL_SEGREG CS
ساکن اندرونی u32 __attribute_const get_global_انحراف(خالی) {
    واپس 0;
}
#اگر_ختم
ساکن اندرونی u16 get_global_seg(خالی) {
    واپس GET_SEG(GLOBAL_SEGREG);
}
#تعریف_کرو GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((خالی*)&(var)              \
                                             + get_global_انحراف()))
#اگر MODESEGMENT
#تعریف_کرو GLOBALFLAT2GLOBAL(var) ((typeof(var))((خالی*)(var) - BUILD_BIOS_ADDR))
#ورنہ
#تعریف_کرو GLOBALFLAT2GLOBAL(var) (var)
#اگر_ختم
// Access a "flat" pointer known to point to the f-segment.
#تعریف_کرو GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#تعریف_کرو SEG_کم علامت(_zonelow_seg)

#اگر MODESEGMENT
#تعریف_کرو GET_کم(var)            GET_FARVAR(SEG_کم, (var))
#تعریف_کرو سیٹ_کم(var, val)       سیٹ_FARVAR(SEG_کم, (var), (val))
#تعریف_کرو LOWFLAT2_کم(var) ((typeof(var))((خالی*)(var) - علامت(zonelow_بنیاد)))
#ورنہ
#تعریف_کرو GET_کم(var)            (var)
#تعریف_کرو سیٹ_کم(var, val)       کرو { (var) = (val); } جب (0)
#تعریف_کرو LOWFLAT2_کم(var) (var)
#اگر_ختم
#تعریف_کرو GET_LOWFLAT(var) GET_کم(*LOWFLAT2_کم(&(var)))
#تعریف_کرو سیٹ_LOWFLAT(var, val) سیٹ_کم(*LOWFLAT2_کم(&(var)), (val))

#اگر_ختم // __BIOSVAR_H
