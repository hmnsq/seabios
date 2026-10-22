// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // SET_BDA
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "hw/pic.h" // pic_eoi1
#अन्तर्भाव "hw/ps2port.h" // ps2_check_event
#अन्तर्भाव "hw/rtc.h" // rtc_read
#अन्तर्भाव "hw/usb-hid.h" // usb_check_event
#अन्तर्भाव "output.h" // debug_enter
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

स्थिर u32
bcd2bin(u8 val)
{
    प्रत्यागम (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

शून्य
clock_setup(शून्य)
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
    यदि (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } अन्यथा {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        यदि (year > 0x80)
            Century = 0x19;
        अन्यथा
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(entry_08));
    यदि (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(entry_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
स्थिर शून्य
handle_1a00(संरचना bregs *regs)
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
स्थिर शून्य
handle_1a01(संरचना bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    SET_BDA(timer_counter, ticks);
    SET_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    set_success(regs);
}

// Read CMOS Time
स्थिर शून्य
handle_1a02(संरचना bregs *regs)
{
    यदि (rtc_updating()) {
        set_invalid(regs);
        प्रत्यागम;
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
स्थिर शून्य
handle_1a03(संरचना bregs *regs)
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
    यदि (rtc_updating()) {
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
स्थिर शून्य
handle_1a04(संरचना bregs *regs)
{
    regs->ah = 0;
    यदि (rtc_updating()) {
        set_invalid(regs);
        प्रत्यागम;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_खालचा(Century);
    regs->al = regs->ch;
    set_success(regs);
}

// Set CMOS Date
स्थिर शून्य
handle_1a05(संरचना bregs *regs)
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
    यदि (rtc_updating()) {
        rtc_setup();
        set_invalid(regs);
        प्रत्यागम;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    SET_खालचा(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_STATUS_B) & ~RTC_B_SET;
    rtc_write(CMOS_STATUS_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    set_success(regs);
}

// Set Alarm Time in CMOS
स्थिर शून्य
handle_1a06(संरचना bregs *regs)
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
    यदि (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        set_invalid(regs);
        प्रत्यागम;
    }
    यदि (rtc_updating()) {
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
स्थिर शून्य
handle_1a07(संरचना bregs *regs)
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

स्थिर शून्य
handle_1abb(संरचना bregs *regs)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_interrupt_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
स्थिर शून्य
handle_1aXX(संरचना bregs *regs)
{
    set_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
शून्य VISIBLE16
handle_1a(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    विकल्प (regs->ah) {
    पक्ष 0x00: handle_1a00(regs); भङ्ग;
    पक्ष 0x01: handle_1a01(regs); भङ्ग;
    पक्ष 0x02: handle_1a02(regs); भङ्ग;
    पक्ष 0x03: handle_1a03(regs); भङ्ग;
    पक्ष 0x04: handle_1a04(regs); भङ्ग;
    पक्ष 0x05: handle_1a05(regs); भङ्ग;
    पक्ष 0x06: handle_1a06(regs); भङ्ग;
    पक्ष 0x07: handle_1a07(regs); भङ्ग;
    पक्ष 0xbb: handle_1abb(regs); भङ्ग;
    स्वभाव:   handle_1aXX(regs); भङ्ग;
    }
}

// Update main tick counter
स्थिर शून्य
clock_update(शून्य)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    यदि (counter >= TICKS_PER_DAY) {
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
शून्य VISIBLE16
handle_08(शून्य)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    संरचना bregs br;
    memset(&br, 0, परिमाण(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 last_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
शून्य
clock_poll_irq(शून्य)
{
    यदि (CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    यदि (!timer_check(GET_खालचा(last_timer_check)))
        प्रत्यागम;
    SET_खालचा(last_timer_check, timer_calc(ticks_to_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 मोजणी)
{
    प्रत्यागम (GET_BDA(timer_counter) + मोजणी + 1) % TICKS_PER_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    यदि (!msecs)
        प्रत्यागम GET_BDA(timer_counter);
    प्रत्यागम irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
पूर्णाङ्क
irqtimer_check(u32 end)
{
    प्रत्यागम (((GET_BDA(timer_counter) + TICKS_PER_DAY - end) % TICKS_PER_DAY)
            < (TICKS_PER_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

स्थिर पूर्णाङ्क
set_usertimer(u32 usecs, u16 seg, u16 offset)
{
    यदि (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        प्रत्यागम -1;

    // Interval not already set.
    SET_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    SET_BDA(user_wait_complete_flag, SEGOFF(seg, offset));
    SET_BDA(user_wait_timeout, usecs);
    rtc_use();
    प्रत्यागम 0;
}

स्थिर शून्य
clear_usertimer(शून्य)
{
    यदि (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        प्रत्यागम;
    // Turn off status byte.
    SET_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#define RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
शून्य
handle_1586(संरचना bregs *regs)
{
    यदि (!CONFIG_RTC_TIMER) {
        set_code_unimplemented(regs, RET_EUNSUPPORTED);
        प्रत्यागम;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 मोजणी = (regs->cx << 16) | regs->dx;
    पूर्णाङ्क ret = set_usertimer(मोजणी, GET_SEG(SS), (u32)&statusflag);
    यदि (ret) {
        set_code_invalid(regs, RET_ECLOCKINUSE);
        प्रत्यागम;
    }
    यावत् (!statusflag)
        yield_toirq();
    set_success(regs);
}

// Set Interval requested.
स्थिर शून्य
handle_158300(संरचना bregs *regs)
{
    पूर्णाङ्क ret = set_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    यदि (ret)
        // Interval already set.
        set_code_invalid(regs, RET_EUNSUPPORTED);
    अन्यथा
        set_success(regs);
}

// Clear interval requested
स्थिर शून्य
handle_158301(संरचना bregs *regs)
{
    clear_usertimer();
    set_success(regs);
}

स्थिर शून्य
handle_1583XX(संरचना bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

शून्य
handle_1583(संरचना bregs *regs)
{
    यदि (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        प्रत्यागम;
    }
    विकल्प (regs->al) {
    पक्ष 0x00: handle_158300(regs); भङ्ग;
    पक्ष 0x01: handle_158301(regs); भङ्ग;
    स्वभाव:   handle_1583XX(regs); भङ्ग;
    }
}

#define USEC_PER_RTC DIV_ROUND_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
शून्य VISIBLE16
handle_70(शून्य)
{
    यदि (!CONFIG_RTC_TIMER)
        प्रत्यागम;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 registerB = rtc_read(CMOS_STATUS_B);
    u8 registerC = rtc_read(CMOS_STATUS_C);

    यदि (!(registerB & (RTC_B_PIE|RTC_B_AIE)))
        गच्छ done;
    यदि (registerC & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        संरचना bregs br;
        memset(&br, 0, परिमाण(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    यदि (!(registerC & RTC_B_PIE))
        गच्छ done;

    // Handle Periodic Interrupt.

    check_preempt();

    यदि (!GET_BDA(rtc_wait_flag))
        गच्छ done;

    // Wait Interval (Int 15, AH=83) active.
    u32 time = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    यदि (time < USEC_PER_RTC) {
        // Done waiting - write to specified flag byte.
        संरचना segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.offset+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        SET_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } अन्यथा {
        // Continue waiting.
        time -= USEC_PER_RTC;
        SET_BDA(user_wait_timeout, time);
    }

done:
    pic_eoi2();
}
