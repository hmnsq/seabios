// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // SET_BDA
#समावेश "bregs.h" // struct bregs
#समावेश "hw/pic.h" // pic_eoi1
#समावेश "hw/ps2port.h" // ps2_check_event
#समावेश "hw/rtc.h" // rtc_read
#समावेश "hw/usb-hid.h" // usb_check_event
#समावेश "output.h" // debug_enter
#समावेश "stacks.h" // yield
#समावेश "string.h" // memset
#समावेश "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

अटल u32
bcd2bin(u8 val)
{
    वापस (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

व्योम
clock_setup(व्योम)
{
    dprintf(3, "init timer\n");
    pit_setup();

    rtc_setup();
    rtc_updating();
    u32 seconds = bcd2bin(rtc_read(CMOS_RTC_SECONDS));
    u32 minutes = bcd2bin(rtc_read(CMOS_RTC_MINUTES));
    u32 hours = bcd2bin(rtc_read(CMOS_RTC_HOURS));
    u32 ticks = ticks_from_ms(((hours * 60 + minutes) * 60 + seconds) * 1000);
    SET_BDA(timer_गणक, ticks % TICKS_PER_DAY);

    // Setup Century storage
    अगर (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } अन्यथा {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        अगर (year > 0x80)
            Century = 0x19;
        अन्यथा
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(entry_08));
    अगर (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(entry_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
अटल व्योम
handle_1a00(काष्ठा bregs *regs)
{
    yield();
    u32 ticks = GET_BDA(timer_गणक);
    regs->cx = ticks >> 16;
    regs->dx = ticks;
    regs->al = GET_BDA(timer_rollover);
    SET_BDA(timer_rollover, 0); // reset flag
    set_success(regs);
}

// Set Current Clock Count
अटल व्योम
handle_1a01(काष्ठा bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    SET_BDA(timer_गणक, ticks);
    SET_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    set_success(regs);
}

// Read CMOS Time
अटल व्योम
handle_1a02(काष्ठा bregs *regs)
{
    अगर (rtc_updating()) {
        set_invalid(regs);
        वापस;
    }

    regs->dh = rtc_read(CMOS_RTC_SECONDS);
    regs->cl = rtc_read(CMOS_RTC_MINUTES);
    regs->ch = rtc_read(CMOS_RTC_HOURS);
    regs->dl = rtc_read(CMOS_STATUS_B) & RTC_B_DSE;
    regs->ah = 0;
    regs->al = regs->ch;
    set_success(regs);
}

// Set CMOS Time
अटल व्योम
handle_1a03(काष्ठा bregs *regs)
{
    // Using a debugger, I notice the following masking/setting
    // of bits in Status Register B, by setting Reg B to
    // a few values and getting its value after INT 1A was called.
    //
    //        try#1       try#2       try#3
    // before 1111 1101   0111 1101   0000 0000
    // after  0110 0010   0110 0010   0000 0010
    //
    // Bit4 in try#1 flipped in hardware (forced low) due to bit7=1
    // My assumption: RegB = ((RegB & 01100000b) | 00000010b)
    अगर (rtc_updating()) {
        rtc_setup();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS, regs->dh);
    rtc_write(CMOS_RTC_MINUTES, regs->cl);
    rtc_write(CMOS_RTC_HOURS, regs->ch);
    // Set Daylight Savings time enabled bit to requested value
    u8 val8 = ((rtc_read(CMOS_STATUS_B) & (RTC_B_PIE|RTC_B_AIE))
               | RTC_B_24HR | (regs->dl & RTC_B_DSE));
    rtc_write(CMOS_STATUS_B, val8);
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    set_success(regs);
}

// Read CMOS Date
अटल व्योम
handle_1a04(काष्ठा bregs *regs)
{
    regs->ah = 0;
    अगर (rtc_updating()) {
        set_invalid(regs);
        वापस;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_निचला(Century);
    regs->al = regs->ch;
    set_success(regs);
}

// Set CMOS Date
अटल व्योम
handle_1a05(काष्ठा bregs *regs)
{
    // Using a debugger, I notice the following masking/setting
    // of bits in Status Register B, by setting Reg B to
    // a few values and getting its value after INT 1A was called.
    //
    //        try#1       try#2       try#3       try#4
    // before 1111 1101   0111 1101   0000 0010   0000 0000
    // after  0110 1101   0111 1101   0000 0010   0000 0000
    //
    // Bit4 in try#1 flipped in hardware (forced low) due to bit7=1
    // My assumption: RegB = (RegB & 01111111b)
    अगर (rtc_updating()) {
        rtc_setup();
        set_invalid(regs);
        वापस;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    SET_निचला(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_STATUS_B) & ~RTC_B_SET;
    rtc_write(CMOS_STATUS_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    set_success(regs);
}

// Set Alarm Time in CMOS
अटल व्योम
handle_1a06(काष्ठा bregs *regs)
{
    // Using a debugger, I notice the following masking/setting
    // of bits in Status Register B, by setting Reg B to
    // a few values and getting its value after INT 1A was called.
    //
    //        try#1       try#2       try#3
    // before 1101 1111   0101 1111   0000 0000
    // after  0110 1111   0111 1111   0010 0000
    //
    // Bit4 in try#1 flipped in hardware (forced low) due to bit7=1
    // My assumption: RegB = ((RegB & 01111111b) | 00100000b)
    u8 val8 = rtc_read(CMOS_STATUS_B); // Get Status Reg B
    regs->ax = 0;
    अगर (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        set_invalid(regs);
        वापस;
    }
    अगर (rtc_updating()) {
        rtc_setup();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS_ALARM, regs->dh);
    rtc_write(CMOS_RTC_MINUTES_ALARM, regs->cl);
    rtc_write(CMOS_RTC_HOURS_ALARM, regs->ch);
    // enable Status Reg B alarm bit, clear halt clock bit
    rtc_write(CMOS_STATUS_B, (val8 & ~RTC_B_SET) | RTC_B_AIE);
    set_success(regs);
}

// Turn off Alarm
अटल व्योम
handle_1a07(काष्ठा bregs *regs)
{
    // Using a debugger, I notice the following masking/setting
    // of bits in Status Register B, by setting Reg B to
    // a few values and getting its value after INT 1A was called.
    //
    //        try#1       try#2       try#3       try#4
    // before 1111 1101   0111 1101   0010 0000   0010 0010
    // after  0100 0101   0101 0101   0000 0000   0000 0010
    //
    // Bit4 in try#1 flipped in hardware (forced low) due to bit7=1
    // My assumption: RegB = (RegB & 01010111b)
    u8 val8 = rtc_read(CMOS_STATUS_B); // Get Status Reg B
    // clear clock-halt bit, disable alarm bit
    rtc_write(CMOS_STATUS_B, val8 & ~(RTC_B_SET|RTC_B_AIE));
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    set_success(regs);
}

अटल व्योम
handle_1abb(काष्ठा bregs *regs)
{
    अगर (!CONFIG_TCGBIOS)
        वापस;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_interrupt_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
अटल व्योम
handle_1aXX(काष्ठा bregs *regs)
{
    set_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
व्योम VISIBLE16
handle_1a(काष्ठा bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    चयन (regs->ah) {
    हाल 0x00: handle_1a00(regs); अवरोध;
    हाल 0x01: handle_1a01(regs); अवरोध;
    हाल 0x02: handle_1a02(regs); अवरोध;
    हाल 0x03: handle_1a03(regs); अवरोध;
    हाल 0x04: handle_1a04(regs); अवरोध;
    हाल 0x05: handle_1a05(regs); अवरोध;
    हाल 0x06: handle_1a06(regs); अवरोध;
    हाल 0x07: handle_1a07(regs); अवरोध;
    हाल 0xbb: handle_1abb(regs); अवरोध;
    शेष:   handle_1aXX(regs); अवरोध;
    }
}

// Update main tick counter
अटल व्योम
clock_update(व्योम)
{
    u32 गणक = GET_BDA(timer_गणक);
    गणक++;
    // compare to one days worth of timer ticks at 18.2 hz
    अगर (गणक >= TICKS_PER_DAY) {
        // there has been a midnight rollover at this point
        गणक = 0;
        SET_BDA(timer_rollover, GET_BDA(timer_rollover) + 1);
    }
    SET_BDA(timer_गणक, गणक);

    // Check for internal events.
    floppy_tick();
    usb_check_event();
    ps2_check_event();
    sercon_check_event();
}

// INT 08h System Timer ISR Entry Point
व्योम VISIBLE16
handle_08(व्योम)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 last_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
व्योम
clock_poll_irq(व्योम)
{
    अगर (CONFIG_HARDWARE_IRQ)
        वापस;
    अगर (!timer_check(GET_निचला(last_timer_check)))
        वापस;
    SET_निचला(last_timer_check, timer_calc(ticks_to_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 गिनती)
{
    वापस (GET_BDA(timer_गणक) + गिनती + 1) % TICKS_PER_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    अगर (!msecs)
        वापस GET_BDA(timer_गणक);
    वापस irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
पूर्णांक
irqtimer_check(u32 end)
{
    वापस (((GET_BDA(timer_गणक) + TICKS_PER_DAY - end) % TICKS_PER_DAY)
            < (TICKS_PER_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

अटल पूर्णांक
set_usertimer(u32 usecs, u16 seg, u16 offset)
{
    अगर (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        वापस -1;

    // Interval not already set.
    SET_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    SET_BDA(user_wait_complete_flag, SEGOFF(seg, offset));
    SET_BDA(user_wait_timeout, usecs);
    rtc_use();
    वापस 0;
}

अटल व्योम
clear_usertimer(व्योम)
{
    अगर (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        वापस;
    // Turn off status byte.
    SET_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#घोषणा RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
व्योम
handle_1586(काष्ठा bregs *regs)
{
    अगर (!CONFIG_RTC_TIMER) {
        set_code_unimplemented(regs, RET_EUNSUPPORTED);
        वापस;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 गिनती = (regs->cx << 16) | regs->dx;
    पूर्णांक ret = set_usertimer(गिनती, GET_SEG(SS), (u32)&statusflag);
    अगर (ret) {
        set_code_invalid(regs, RET_ECLOCKINUSE);
        वापस;
    }
    जबतक (!statusflag)
        yield_toirq();
    set_success(regs);
}

// Set Interval requested.
अटल व्योम
handle_158300(काष्ठा bregs *regs)
{
    पूर्णांक ret = set_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    अगर (ret)
        // Interval already set.
        set_code_invalid(regs, RET_EUNSUPPORTED);
    अन्यथा
        set_success(regs);
}

// Clear interval requested
अटल व्योम
handle_158301(काष्ठा bregs *regs)
{
    clear_usertimer();
    set_success(regs);
}

अटल व्योम
handle_1583XX(काष्ठा bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

व्योम
handle_1583(काष्ठा bregs *regs)
{
    अगर (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        वापस;
    }
    चयन (regs->al) {
    हाल 0x00: handle_158300(regs); अवरोध;
    हाल 0x01: handle_158301(regs); अवरोध;
    शेष:   handle_1583XX(regs); अवरोध;
    }
}

#घोषणा USEC_PER_RTC DIV_ROUND_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
व्योम VISIBLE16
handle_70(व्योम)
{
    अगर (!CONFIG_RTC_TIMER)
        वापस;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 registerB = rtc_read(CMOS_STATUS_B);
    u8 registerC = rtc_read(CMOS_STATUS_C);

    अगर (!(registerB & (RTC_B_PIE|RTC_B_AIE)))
        जाओ done;
    अगर (registerC & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        काष्ठा bregs br;
        स_रखो(&br, 0, माप(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    अगर (!(registerC & RTC_B_PIE))
        जाओ done;

    // Handle Periodic Interrupt.

    check_preempt();

    अगर (!GET_BDA(rtc_wait_flag))
        जाओ done;

    // Wait Interval (Int 15, AH=83) active.
    u32 समय = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    अगर (समय < USEC_PER_RTC) {
        // Done waiting - write to specified flag byte.
        काष्ठा segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.offset+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        SET_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } अन्यथा {
        // Continue waiting.
        समय -= USEC_PER_RTC;
        SET_BDA(user_wait_timeout, समय);
    }

done:
    pic_eoi2();
}
