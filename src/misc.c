// Code for misc 16bit handlers and variables.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_BDA
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "hw/pic.h" // enable_hwirq
#अन्तर्भाव "output.h" // debug_enter
#अन्तर्भाव "stacks.h" // call16_int
#अन्तर्भाव "string.h" // memset

#परिभाषय पत्तनम्_MATH_CLEAR        0x00f0

// Indicator if POST phase has been started (and if it has completed).
पूर्णाङ्क Have_सततम्_Post VARFSEG;

पूर्णाङ्क
अन्तर्_post(शून्य)
{
    प्रत्यागम GET_GLOBAL(Have_सततम्_Post) == 1;
}


/****************************************************************
 * Misc 16bit ISRs
 ****************************************************************/

// INT 12h Memory Size Service Entry Point
शून्य VISIBLE16
handle_12(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_12);
    regs->ax = GET_BDA(mem_आकार_kb);
}

// INT 11h Equipment List Service Entry Point
शून्य VISIBLE16
handle_11(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_11);
    regs->ax = GET_BDA(equipment_list_flags);
}

// INT 05h Print Screen Service Entry Point
शून्य VISIBLE16
handle_05(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_05);
}

// NMI handler
शून्य VISIBLE16
handle_02(शून्य)
{
    debug_isr(DEBUG_ISR_02);
}

शून्य
mathcp_सज्जता(शून्य)
{
    dprintf(3, "math cp init\n");
    // 80x87 coprocessor installed
    समुच्चयः_equipment_flags(0x02, 0x02);
    enable_hwirq(13, FUNC16(प्रविष्टिः_75));
}

// INT 75 - IRQ13 - MATH COPROCESSOR EXCEPTION
शून्य VISIBLE16
handle_75(शून्य)
{
    debug_isr(DEBUG_ISR_75);

    // clear irq13
    outb(0, पत्तनम्_MATH_CLEAR);
    // clear interrupt
    pic_eoi2();
    // legacy nmi call
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF;
    call16_int(0x02, &br);
}


/****************************************************************
 * BIOS_CONFIG_TABLE
 ****************************************************************/

// DMA channel 3 used by hard disk BIOS
#परिभाषय CBT_F1_DMA3_प्रयुक्तम् (1<<7)
// 2nd interrupt controller (8259) installed
#परिभाषय CBT_F1_2NDPIC   (1<<6)
// Real-Time Clock installed
#परिभाषय CBT_F1_RTC      (1<<5)
// INT 15/AH=4Fh called upon INT 09h
#परिभाषय CBT_F1_INT154F  (1<<4)
// wait for external event (INT 15/AH=41h) supported
#परिभाषय CBT_F1_WAITEXT  (1<<3)
// extended BIOS area allocated (usually at top of RAM)
#परिभाषय CBT_F1_EBDA     (1<<2)
// bus is Micro Channel instead of ISA
#परिभाषय CBT_F1_MCA      (1<<1)
// system has dual bus (Micro Channel + ISA)
#परिभाषय CBT_F1_MCAISA   (1<<0)

// INT 16/AH=09h (keyboard functionality) supported
#परिभाषय CBT_F2_INT1609  (1<<6)

संरचना bios_config_सारणी_s BIOS_CONFIG_सारणी VARFSEGFIXED(0xe6f5) = {
    .आकार     = परिमाण(BIOS_CONFIG_सारणी) - 2,
    .model    = BUILD_MODEL_ID,
    .submodel = BUILD_SUBMODEL_ID,
    .biosrev  = BUILD_BIOS_REVISION,
    .feature1 = (
        CBT_F1_2NDPIC | CBT_F1_RTC | CBT_F1_EBDA
        | (CONFIG_KBD_CALL_INT15_4F ? CBT_F1_INT154F : 0)),
    .feature2 = CBT_F2_INT1609,
    .feature3 = 0,
    .feature4 = 0,
    .feature5 = 0,
};


/****************************************************************
 * GDT and IDT tables
 ****************************************************************/

// Real mode IDT descriptor
संरचना descloc_s rmode_IDT_info VARFSEG = {
    .दीर्घता = परिमाण(संरचना rmode_IVT) - 1,
    .addr = (u32)MAKE_FLATPTR(SEG_IVT, 0),
};

// Dummy IDT that forces a machine shutdown if an irq happens in
// protected mode.
u8 dummy_IDT VARFSEG;

// Protected mode IDT descriptor
संरचना descloc_s pmode_IDT_info VARFSEG = {
    .दीर्घता = परिमाण(dummy_IDT) - 1,
    .addr = (u32)&dummy_IDT,
};

// GDT
u64 rombios32_gdt[] VARFSEG __aligned(8) = {
    // First entry can't be used.
    0x0000000000000000LL,
    // 32 bit flat code segment (SEG32_MODE32_CS)
    GDT_GRANLIMIT(0xffffffff) | GDT_संहिता | GDT_B,
    // 32 bit flat data segment (SEG32_MODE32_DS)
    GDT_GRANLIMIT(0xffffffff) | GDT_दत्त | GDT_B,
    // 16 bit code segment base=0xf0000 limit=0xffff (SEG32_MODE16_CS)
    GDT_सीमा(BUILD_BIOS_आकार-1) | GDT_संहिता | GDT_आधारः(BUILD_BIOS_ADDR),
    // 16 bit data segment base=0x0 limit=0xffff (SEG32_MODE16_DS)
    GDT_सीमा(0x0ffff) | GDT_दत्त,
    // 16 bit code segment base=0xf0000 limit=0xffffffff (SEG32_MODE16BIG_CS)
    GDT_GRANLIMIT(0xffffffff) | GDT_संहिता | GDT_आधारः(BUILD_BIOS_ADDR),
    // 16 bit data segment base=0 limit=0xffffffff (SEG32_MODE16BIG_DS)
    GDT_GRANLIMIT(0xffffffff) | GDT_दत्त,
};

// GDT descriptor
संरचना descloc_s rombios32_gdt_48 VARFSEG = {
    .दीर्घता = परिमाण(rombios32_gdt) - 1,
    .addr = (u32)rombios32_gdt,
};


/****************************************************************
 * Misc fixed vars
 ****************************************************************/

// BIOS build date
वर्ण BiosDate[] VARFSEGFIXED(0xfff5) = "06/23/99";

u8 BiosModelId VARFSEGFIXED(0xfffe) = BUILD_MODEL_ID;

u8 BiosChecksum VARFSEGFIXED(0xffff);

संरचना floppy_dbt_s diskette_param_सारणी VARFSEGFIXED(0xefc7);

// Old Fixed Disk Parameter Table (newer tables are in the ebda).
संरचना fdpt_s OldFDPT VARFSEGFIXED(0xe401);

// XXX - Baud Rate Generator Table
u8 Baud_सारणी[16] VARFSEGFIXED(0xe729);

// XXX - Initial Interrupt Vector Offsets Loaded by POST
u8 InitVectors[13] VARFSEGFIXED(0xfef3);

// XXX - INT 1D - SYSTEM DATA - VIDEO PARAMETER TABLES
u8 VideoParams[88] VARFSEGFIXED(0xf0a4);
