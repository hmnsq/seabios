// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // SET_BDA
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "hw/pic.h" // pic_eoi1
#অন্তর্ভাব "hw/ps2port.h" // ps2_check_event
#অন্তর্ভাব "hw/rtc.h" // rtc_read
#অন্তর্ভাব "hw/usb-hid.h" // usb_check_event
#অন্তর্ভাব "output.h" // debug_enter
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

স্থির u32
bcd2bin(u8 val)
{
    প্রত্যাগম (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

শূন্য
clock_setup(শূন্য)
{
    dprintf(3, "init timer\n");
    pit_setup();

    rtc_setup();
    rtc_updating();
    u32 seconds = bcd2bin(rtc_read(CMOS_RTC_SECONDS));
    u32 minutes = bcd2bin(rtc_read(CMOS_RTC_MINUTES));
    u32 hours = bcd2bin(rtc_read(CMOS_RTC_HOURS));
    u32 ticks = ticks_from_ms(((hours * 60 + minutes) * 60 + seconds) * 1000);
    SET_BDA(timer_counter, ticks % TICKS_PER_DAY);

    // Setup Century storage
    যদি (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } অন্যথা {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        যদি (year > 0x80)
            Century = 0x19;
        অন্যথা
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(entry_08));
    যদি (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(entry_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
স্থির শূন্য
handle_1a00(সংরচনা bregs *regs)
{
    yield();
    u32 ticks = GET_BDA(timer_counter);
    regs->cx = ticks >> 16;
    regs->dx = ticks;
    regs->al = GET_BDA(timer_rollover);
    SET_BDA(timer_rollover, 0); // reset flag
    set_success(regs);
}

// Set Current Clock Count
স্থির শূন্য
handle_1a01(সংরচনা bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    SET_BDA(timer_counter, ticks);
    SET_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    set_success(regs);
}

// Read CMOS Time
স্থির শূন্য
handle_1a02(সংরচনা bregs *regs)
{
    যদি (rtc_updating()) {
        set_invalid(regs);
        প্রত্যাগম;
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
স্থির শূন্য
handle_1a03(সংরচনা bregs *regs)
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
    যদি (rtc_updating()) {
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
স্থির শূন্য
handle_1a04(সংরচনা bregs *regs)
{
    regs->ah = 0;
    যদি (rtc_updating()) {
        set_invalid(regs);
        প্রত্যাগম;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_নিচের(Century);
    regs->al = regs->ch;
    set_success(regs);
}

// Set CMOS Date
স্থির শূন্য
handle_1a05(সংরচনা bregs *regs)
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
    যদি (rtc_updating()) {
        rtc_setup();
        set_invalid(regs);
        প্রত্যাগম;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    SET_নিচের(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_STATUS_B) & ~RTC_B_SET;
    rtc_write(CMOS_STATUS_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    set_success(regs);
}

// Set Alarm Time in CMOS
স্থির শূন্য
handle_1a06(সংরচনা bregs *regs)
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
    যদি (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        set_invalid(regs);
        প্রত্যাগম;
    }
    যদি (rtc_updating()) {
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
স্থির শূন্য
handle_1a07(সংরচনা bregs *regs)
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

স্থির শূন্য
handle_1abb(সংরচনা bregs *regs)
{
    যদি (!CONFIG_TCGBIOS)
        প্রত্যাগম;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_interrupt_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
স্থির শূন্য
handle_1aXX(সংরচনা bregs *regs)
{
    set_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
শূন্য VISIBLE16
handle_1a(সংরচনা bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    বিকল্প (regs->ah) {
    পক্ষ 0x00: handle_1a00(regs); ভঙ্গ;
    পক্ষ 0x01: handle_1a01(regs); ভঙ্গ;
    পক্ষ 0x02: handle_1a02(regs); ভঙ্গ;
    পক্ষ 0x03: handle_1a03(regs); ভঙ্গ;
    পক্ষ 0x04: handle_1a04(regs); ভঙ্গ;
    পক্ষ 0x05: handle_1a05(regs); ভঙ্গ;
    পক্ষ 0x06: handle_1a06(regs); ভঙ্গ;
    পক্ষ 0x07: handle_1a07(regs); ভঙ্গ;
    পক্ষ 0xbb: handle_1abb(regs); ভঙ্গ;
    স্বভাব:   handle_1aXX(regs); ভঙ্গ;
    }
}

// Update main tick counter
স্থির শূন্য
clock_update(শূন্য)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    যদি (counter >= TICKS_PER_DAY) {
        // there has been a midnight rollover at this point
        counter = 0;
        SET_BDA(timer_rollover, GET_BDA(timer_rollover) + 1);
    }
    SET_BDA(timer_counter, counter);

    // Check for internal events.
    floppy_tick();
    usb_check_event();
    ps2_check_event();
    sercon_check_event();
}

// INT 08h System Timer ISR Entry Point
শূন্য VISIBLE16
handle_08(শূন্য)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 last_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
শূন্য
clock_poll_irq(শূন্য)
{
    যদি (CONFIG_HARDWARE_IRQ)
        প্রত্যাগম;
    যদি (!timer_check(GET_নিচের(last_timer_check)))
        প্রত্যাগম;
    SET_নিচের(last_timer_check, timer_calc(ticks_to_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 গুনতি)
{
    প্রত্যাগম (GET_BDA(timer_counter) + গুনতি + 1) % TICKS_PER_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    যদি (!msecs)
        প্রত্যাগম GET_BDA(timer_counter);
    প্রত্যাগম irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
পূর্ণাঙ্ক
irqtimer_check(u32 end)
{
    প্রত্যাগম (((GET_BDA(timer_counter) + TICKS_PER_DAY - end) % TICKS_PER_DAY)
            < (TICKS_PER_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

স্থির পূর্ণাঙ্ক
set_usertimer(u32 usecs, u16 seg, u16 offset)
{
    যদি (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        প্রত্যাগম -1;

    // Interval not already set.
    SET_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    SET_BDA(user_wait_complete_flag, SEGOFF(seg, offset));
    SET_BDA(user_wait_timeout, usecs);
    rtc_use();
    প্রত্যাগম 0;
}

স্থির শূন্য
clear_usertimer(শূন্য)
{
    যদি (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        প্রত্যাগম;
    // Turn off status byte.
    SET_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#define RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
শূন্য
handle_1586(সংরচনা bregs *regs)
{
    যদি (!CONFIG_RTC_TIMER) {
        set_code_unimplemented(regs, RET_EUNSUPPORTED);
        প্রত্যাগম;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 গুনতি = (regs->cx << 16) | regs->dx;
    পূর্ণাঙ্ক ret = set_usertimer(গুনতি, GET_SEG(SS), (u32)&statusflag);
    যদি (ret) {
        set_code_invalid(regs, RET_ECLOCKINUSE);
        প্রত্যাগম;
    }
    যাবত্ (!statusflag)
        yield_toirq();
    set_success(regs);
}

// Set Interval requested.
স্থির শূন্য
handle_158300(সংরচনা bregs *regs)
{
    পূর্ণাঙ্ক ret = set_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    যদি (ret)
        // Interval already set.
        set_code_invalid(regs, RET_EUNSUPPORTED);
    অন্যথা
        set_success(regs);
}

// Clear interval requested
স্থির শূন্য
handle_158301(সংরচনা bregs *regs)
{
    clear_usertimer();
    set_success(regs);
}

স্থির শূন্য
handle_1583XX(সংরচনা bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

শূন্য
handle_1583(সংরচনা bregs *regs)
{
    যদি (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        প্রত্যাগম;
    }
    বিকল্প (regs->al) {
    পক্ষ 0x00: handle_158300(regs); ভঙ্গ;
    পক্ষ 0x01: handle_158301(regs); ভঙ্গ;
    স্বভাব:   handle_1583XX(regs); ভঙ্গ;
    }
}

#define USEC_PER_RTC DIV_ROUND_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
শূন্য VISIBLE16
handle_70(শূন্য)
{
    যদি (!CONFIG_RTC_TIMER)
        প্রত্যাগম;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 registerB = rtc_read(CMOS_STATUS_B);
    u8 registerC = rtc_read(CMOS_STATUS_C);

    যদি (!(registerB & (RTC_B_PIE|RTC_B_AIE)))
        গচ্ছ done;
    যদি (registerC & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        সংরচনা bregs br;
        memset(&br, 0, পরিমাণ(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    যদি (!(registerC & RTC_B_PIE))
        গচ্ছ done;

    // Handle Periodic Interrupt.

    check_preempt();

    যদি (!GET_BDA(rtc_wait_flag))
        গচ্ছ done;

    // Wait Interval (Int 15, AH=83) active.
    u32 time = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    যদি (time < USEC_PER_RTC) {
        // Done waiting - write to specified flag byte.
        সংরচনা segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.offset+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        SET_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } অন্যথা {
        // Continue waiting.
        time -= USEC_PER_RTC;
        SET_BDA(user_wait_timeout, time);
    }

done:
    pic_eoi2();
}
