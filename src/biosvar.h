// Memory access to BIOS variables.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
#నిర్వచితం_కాకపోతే __BIOSVAR_H
#నిర్వచించు __BIOSVAR_H

#అన్తర్భావ "config.h" // SEG_BDA
#అన్తర్భావ "farptr.h" // GET_FARVAR
#అన్తర్భావ "memmap.h" //చిహ్నం
#అన్తర్భావ "std/bda.h" // struct bios_data_area_s


/****************************************************************
 * Interrupt vector table
 ****************************************************************/

#నిర్వచించు GET_IVT(సదిశ)                                         \
    GET_FARVAR(SEG_IVT, ((నిర్మాణం rmode_IVT *)0)->ivec[సదిశ])
#నిర్వచించు సమితి_IVT(సదిశ, segoff)                                         \
    సమితి_FARVAR(SEG_IVT, ((నిర్మాణం rmode_IVT *)0)->ivec[సదిశ], segoff)

#నిర్వచించు FUNC16(func) ({                                 \
        ASSERT32FLAT();                                 \
        బాహ్య శూన్య func (శూన్య);                        \
        SEGOFF(SEG_BIOS, (u32)func - BUILD_BIOS_ADDR);  \
    })


/****************************************************************
 * Bios Data Area (BDA)
 ****************************************************************/

// Accessor functions
#నిర్వచించు GET_BDA(var) \
    GET_FARVAR(SEG_BDA, ((నిర్మాణం bios_దత్తాంశం_area_s *)0)->var)
#నిర్వచించు సమితి_BDA(var, val) \
    సమితి_FARVAR(SEG_BDA, ((నిర్మాణం bios_దత్తాంశం_area_s *)0)->var, (val))

// Helper function to set the bits of the equipment_list_flags variable.
స్థిర అంతఃవరుస శూన్య సమితి_equipment_flags(u16 clear, u16 సమితి) {
    u16 eqf = GET_BDA(equipment_list_flags);
    సమితి_BDA(equipment_list_flags, (eqf & ~clear) | సమితి);
}


/****************************************************************
 * Extended Bios Data Area (EBDA)
 ****************************************************************/

// The initial size and location of EBDA
#నిర్వచించు EBDA_కొలత_ప్రారంభం \
    DIV_విడత_పైకి(పరిమాణం(నిర్మాణం extended_bios_దత్తాంశం_area_s), 1024)
#నిర్వచించు EBDA_SEGMENT_ప్రారంభం \
    FLATPTR_గమ్యం_SEG(BUILD_LOWRAM_చివర - EBDA_కొలత_ప్రారంభం*1024)

// Accessor functions
స్థిర అంతఃవరుస u16 get_ebda_seg(శూన్య) {
    తిరిగి GET_BDA(ebda_seg);
}
స్థిర అంతఃవరుస నిర్మాణం extended_bios_దత్తాంశం_area_s *
get_ebda_ptr(శూన్య)
{
    ASSERT32FLAT();
    తిరిగి MAKE_FLATPTR(get_ebda_seg(), 0);
}
#నిర్వచించు GET_EBDA(eseg, var)                                             \
    GET_FARVAR(eseg, ((నిర్మాణం extended_bios_దత్తాంశం_area_s *)0)->var)
#నిర్వచించు సమితి_EBDA(eseg, var, val)                                        \
    సమితి_FARVAR(eseg, ((నిర్మాణం extended_bios_దత్తాంశం_area_s *)0)->var, (val))


/****************************************************************
 * Global variables
 ****************************************************************/

#ఒకవేళ MODE16 == 0 && MODESEGMENT == 1
// In 32bit segmented mode %cs may not be readable and the code may be
// relocated.  The entry code sets up %gs with a readable segment and
// the code offset can be determined by get_global_offset().
#నిర్వచించు GLOBAL_SEGREG GS
స్థిర అంతఃవరుస u32 __attribute_const get_global_స్థానభ్రంశం(శూన్య) {
    u32 ret;
    asm("  calll 1f\n"
        "1:popl %0\n"
        "  subl $1b, %0"
        : "=r"(ret));
    తిరిగి ret;
}
#లేకపోతే
#నిర్వచించు GLOBAL_SEGREG CS
స్థిర అంతఃవరుస u32 __attribute_const get_global_స్థానభ్రంశం(శూన్య) {
    తిరిగి 0;
}
#అయితే_ముగింపు
స్థిర అంతఃవరుస u16 get_global_seg(శూన్య) {
    తిరిగి GET_SEG(GLOBAL_SEGREG);
}
#నిర్వచించు GET_GLOBAL(var)                                                 \
    GET_VAR(GLOBAL_SEGREG, *(typeof(&(var)))((శూన్య*)&(var)              \
                                             + get_global_స్థానభ్రంశం()))
#ఒకవేళ MODESEGMENT
#నిర్వచించు GLOBALFLAT2GLOBAL(var) ((typeof(var))((శూన్య*)(var) - BUILD_BIOS_ADDR))
#లేకపోతే
#నిర్వచించు GLOBALFLAT2GLOBAL(var) (var)
#అయితే_ముగింపు
// Access a "flat" pointer known to point to the f-segment.
#నిర్వచించు GET_GLOBALFLAT(var) GET_GLOBAL(*GLOBALFLAT2GLOBAL(&(var)))


/****************************************************************
 * "Low" memory variables
 ****************************************************************/

#నిర్వచించు SEG_దిగువ చిహ్నం(_zonelow_seg)

#ఒకవేళ MODESEGMENT
#నిర్వచించు GET_దిగువ(var)            GET_FARVAR(SEG_దిగువ, (var))
#నిర్వచించు సమితి_దిగువ(var, val)       సమితి_FARVAR(SEG_దిగువ, (var), (val))
#నిర్వచించు LOWFLAT2_దిగువ(var) ((typeof(var))((శూన్య*)(var) - చిహ్నం(zonelow_పునాది)))
#లేకపోతే
#నిర్వచించు GET_దిగువ(var)            (var)
#నిర్వచించు సమితి_దిగువ(var, val)       చేయి { (var) = (val); } అయితే (0)
#నిర్వచించు LOWFLAT2_దిగువ(var) (var)
#అయితే_ముగింపు
#నిర్వచించు GET_LOWFLAT(var) GET_దిగువ(*LOWFLAT2_దిగువ(&(var)))
#నిర్వచించు సమితి_LOWFLAT(var, val) సమితి_దిగువ(*LOWFLAT2_దిగువ(&(var)), (val))

#అయితే_ముగింపు // __BIOSVAR_H
