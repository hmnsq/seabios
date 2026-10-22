// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // SET_BDA
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "hw/pic.h" // pic_eoi1
#అన్తర్భావ "hw/ps2port.h" // ps2_check_event
#అన్తర్భావ "hw/rtc.h" // rtc_read
#అన్తర్భావ "hw/usb-hid.h" // usb_check_event
#అన్తర్భావ "output.h" // debug_enter
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

స్థిర u32
bcd2bin(u8 val)
{
    తిరిగి (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

శూన్య
clock_అమరిక(శూన్య)
{
    dprintf(3, "init timer\n");
    pit_అమరిక();

    rtc_అమరిక();
    rtc_updating();
    u32 seconds = bcd2bin(rtc_read(CMOS_RTC_SECONDS));
    u32 minutes = bcd2bin(rtc_read(CMOS_RTC_MINUTES));
    u32 hours = bcd2bin(rtc_read(CMOS_RTC_HOURS));
    u32 ticks = ticks_from_ms(((hours * 60 + minutes) * 60 + seconds) * 1000);
    సమితి_BDA(timer_counter, ticks % TICKS_చొప్పున_DAY);

    // Setup Century storage
    ఒకవేళ (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } లేకపోతే {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        ఒకవేళ (year > 0x80)
            Century = 0x19;
        లేకపోతే
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(నమోదు_08));
    ఒకవేళ (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(నమోదు_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
స్థిర శూన్య
handle_1a00(నిర్మాణం bregs *regs)
{
    yield();
    u32 ticks = GET_BDA(timer_counter);
    regs->cx = ticks >> 16;
    regs->dx = ticks;
    regs->al = GET_BDA(timer_rollover);
    సమితి_BDA(timer_rollover, 0); // reset flag
    సమితి_success(regs);
}

// Set Current Clock Count
స్థిర శూన్య
handle_1a01(నిర్మాణం bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    సమితి_BDA(timer_counter, ticks);
    సమితి_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    సమితి_success(regs);
}

// Read CMOS Time
స్థిర శూన్య
handle_1a02(నిర్మాణం bregs *regs)
{
    ఒకవేళ (rtc_updating()) {
        సమితి_invalid(regs);
        తిరిగి;
    }

    regs->dh = rtc_read(CMOS_RTC_SECONDS);
    regs->cl = rtc_read(CMOS_RTC_MINUTES);
    regs->ch = rtc_read(CMOS_RTC_HOURS);
    regs->dl = rtc_read(CMOS_స్థితి_B) & RTC_B_DSE;
    regs->ah = 0;
    regs->al = regs->ch;
    సమితి_success(regs);
}

// Set CMOS Time
స్థిర శూన్య
handle_1a03(నిర్మాణం bregs *regs)
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
    ఒకవేళ (rtc_updating()) {
        rtc_అమరిక();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS, regs->dh);
    rtc_write(CMOS_RTC_MINUTES, regs->cl);
    rtc_write(CMOS_RTC_HOURS, regs->ch);
    // Set Daylight Savings time enabled bit to requested value
    u8 val8 = ((rtc_read(CMOS_స్థితి_B) & (RTC_B_PIE|RTC_B_AIE))
               | RTC_B_24HR | (regs->dl & RTC_B_DSE));
    rtc_write(CMOS_స్థితి_B, val8);
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    సమితి_success(regs);
}

// Read CMOS Date
స్థిర శూన్య
handle_1a04(నిర్మాణం bregs *regs)
{
    regs->ah = 0;
    ఒకవేళ (rtc_updating()) {
        సమితి_invalid(regs);
        తిరిగి;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_దిగువ(Century);
    regs->al = regs->ch;
    సమితి_success(regs);
}

// Set CMOS Date
స్థిర శూన్య
handle_1a05(నిర్మాణం bregs *regs)
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
    ఒకవేళ (rtc_updating()) {
        rtc_అమరిక();
        సమితి_invalid(regs);
        తిరిగి;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    సమితి_దిగువ(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_స్థితి_B) & ~RTC_B_సమితి;
    rtc_write(CMOS_స్థితి_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    సమితి_success(regs);
}

// Set Alarm Time in CMOS
స్థిర శూన్య
handle_1a06(నిర్మాణం bregs *regs)
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
    u8 val8 = rtc_read(CMOS_స్థితి_B); // Get Status Reg B
    regs->ax = 0;
    ఒకవేళ (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        సమితి_invalid(regs);
        తిరిగి;
    }
    ఒకవేళ (rtc_updating()) {
        rtc_అమరిక();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS_ALARM, regs->dh);
    rtc_write(CMOS_RTC_MINUTES_ALARM, regs->cl);
    rtc_write(CMOS_RTC_HOURS_ALARM, regs->ch);
    // enable Status Reg B alarm bit, clear halt clock bit
    rtc_write(CMOS_స్థితి_B, (val8 & ~RTC_B_సమితి) | RTC_B_AIE);
    సమితి_success(regs);
}

// Turn off Alarm
స్థిర శూన్య
handle_1a07(నిర్మాణం bregs *regs)
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
    u8 val8 = rtc_read(CMOS_స్థితి_B); // Get Status Reg B
    // clear clock-halt bit, disable alarm bit
    rtc_write(CMOS_స్థితి_B, val8 & ~(RTC_B_సమితి|RTC_B_AIE));
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    సమితి_success(regs);
}

స్థిర శూన్య
handle_1abb(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_అంతరాయం_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
స్థిర శూన్య
handle_1aXX(నిర్మాణం bregs *regs)
{
    సమితి_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
శూన్య VISIBLE16
handle_1a(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    ఎంపిక (regs->ah) {
    స్థితి 0x00: handle_1a00(regs); ఆపు;
    స్థితి 0x01: handle_1a01(regs); ఆపు;
    స్థితి 0x02: handle_1a02(regs); ఆపు;
    స్థితి 0x03: handle_1a03(regs); ఆపు;
    స్థితి 0x04: handle_1a04(regs); ఆపు;
    స్థితి 0x05: handle_1a05(regs); ఆపు;
    స్థితి 0x06: handle_1a06(regs); ఆపు;
    స్థితి 0x07: handle_1a07(regs); ఆపు;
    స్థితి 0xbb: handle_1abb(regs); ఆపు;
    అప్రమేయ:   handle_1aXX(regs); ఆపు;
    }
}

// Update main tick counter
స్థిర శూన్య
clock_update(శూన్య)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    ఒకవేళ (counter >= TICKS_చొప్పున_DAY) {
        // there has been a midnight rollover at this point
        counter = 0;
        సమితి_BDA(timer_rollover, GET_BDA(timer_rollover) + 1);
    }
    సమితి_BDA(timer_counter, counter);

    // Check for internal events.
    floppy_tick();
    usb_check_event();
    ps2_check_event();
    sercon_check_event();
}

// INT 08h System Timer ISR Entry Point
శూన్య VISIBLE16
handle_08(శూన్య)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 చివరి_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
శూన్య
clock_poll_irq(శూన్య)
{
    ఒకవేళ (CONFIG_HARDWARE_IRQ)
        తిరిగి;
    ఒకవేళ (!timer_check(GET_దిగువ(చివరి_timer_check)))
        తిరిగి;
    సమితి_దిగువ(చివరి_timer_check, timer_calc(ticks_గమ్యం_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 లెక్క)
{
    తిరిగి (GET_BDA(timer_counter) + లెక్క + 1) % TICKS_చొప్పున_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    ఒకవేళ (!msecs)
        తిరిగి GET_BDA(timer_counter);
    తిరిగి irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
పూర్ణాంకం
irqtimer_check(u32 చివర)
{
    తిరిగి (((GET_BDA(timer_counter) + TICKS_చొప్పున_DAY - చివర) % TICKS_చొప్పున_DAY)
            < (TICKS_చొప్పున_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

స్థిర పూర్ణాంకం
సమితి_usertimer(u32 usecs, u16 seg, u16 స్థానభ్రంశం_2)
{
    ఒకవేళ (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        తిరిగి -1;

    // Interval not already set.
    సమితి_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    సమితి_BDA(user_wait_complete_flag, SEGOFF(seg, స్థానభ్రంశం_2));
    సమితి_BDA(user_wait_timeout, usecs);
    rtc_use();
    తిరిగి 0;
}

స్థిర శూన్య
clear_usertimer(శూన్య)
{
    ఒకవేళ (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        తిరిగి;
    // Turn off status byte.
    సమితి_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#నిర్వచించు RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
శూన్య
handle_1586(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_RTC_TIMER) {
        సమితి_సంకేతం_unimplemented(regs, RET_EUNSUPPORTED);
        తిరిగి;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 లెక్క = (regs->cx << 16) | regs->dx;
    పూర్ణాంకం ret = సమితి_usertimer(లెక్క, GET_SEG(SS), (u32)&statusflag);
    ఒకవేళ (ret) {
        సమితి_సంకేతం_invalid(regs, RET_ECLOCKINUSE);
        తిరిగి;
    }
    అయితే (!statusflag)
        yield_toirq();
    సమితి_success(regs);
}

// Set Interval requested.
స్థిర శూన్య
handle_158300(నిర్మాణం bregs *regs)
{
    పూర్ణాంకం ret = సమితి_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    ఒకవేళ (ret)
        // Interval already set.
        సమితి_సంకేతం_invalid(regs, RET_EUNSUPPORTED);
    లేకపోతే
        సమితి_success(regs);
}

// Clear interval requested
స్థిర శూన్య
handle_158301(నిర్మాణం bregs *regs)
{
    clear_usertimer();
    సమితి_success(regs);
}

స్థిర శూన్య
handle_1583XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

శూన్య
handle_1583(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        తిరిగి;
    }
    ఎంపిక (regs->al) {
    స్థితి 0x00: handle_158300(regs); ఆపు;
    స్థితి 0x01: handle_158301(regs); ఆపు;
    అప్రమేయ:   handle_1583XX(regs); ఆపు;
    }
}

#నిర్వచించు USEC_చొప్పున_RTC DIV_విడత_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
శూన్య VISIBLE16
handle_70(శూన్య)
{
    ఒకవేళ (!CONFIG_RTC_TIMER)
        తిరిగి;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 రిజిస్టరు_B = rtc_read(CMOS_స్థితి_B);
    u8 రిజిస్టరు_C = rtc_read(CMOS_స్థితి_C);

    ఒకవేళ (!(రిజిస్టరు_B & (RTC_B_PIE|RTC_B_AIE)))
        వెళ్ళు పూర్తి;
    ఒకవేళ (రిజిస్టరు_C & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        నిర్మాణం bregs br;
        memset(&br, 0, పరిమాణం(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    ఒకవేళ (!(రిజిస్టరు_C & RTC_B_PIE))
        వెళ్ళు పూర్తి;

    // Handle Periodic Interrupt.

    check_preempt();

    ఒకవేళ (!GET_BDA(rtc_wait_flag))
        వెళ్ళు పూర్తి;

    // Wait Interval (Int 15, AH=83) active.
    u32 సమయం = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    ఒకవేళ (సమయం < USEC_చొప్పున_RTC) {
        // Done waiting - write to specified flag byte.
        నిర్మాణం segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.స్థానభ్రంశం_2+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        సమితి_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } లేకపోతే {
        // Continue waiting.
        సమయం -= USEC_చొప్పున_RTC;
        సమితి_BDA(user_wait_timeout, సమయం);
    }

పూర్తి:
    pic_eoi2();
}
