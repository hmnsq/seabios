// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // SET_BDA
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "hw/pic.h" // pic_eoi1
#ಅನ್ತರ್ಭಾವ "hw/ps2port.h" // ps2_check_event
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // rtc_read
#ಅನ್ತರ್ಭಾವ "hw/usb-hid.h" // usb_check_event
#ಅನ್ತರ್ಭಾವ "output.h" // debug_enter
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

ಸ್ಥಿರ u32
bcd2bin(u8 val)
{
    ಹಿಂತಿರುಗಿ (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

ಶೂನ್ಯ
clock_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    dprintf(3, "init timer\n");
    pit_ಸಿದ್ಧತೆ();

    rtc_ಸಿದ್ಧತೆ();
    rtc_updating();
    u32 seconds = bcd2bin(rtc_read(CMOS_RTC_SECONDS));
    u32 minutes = bcd2bin(rtc_read(CMOS_RTC_MINUTES));
    u32 hours = bcd2bin(rtc_read(CMOS_RTC_HOURS));
    u32 ticks = ticks_from_ms(((hours * 60 + minutes) * 60 + seconds) * 1000);
    ಗಣ_BDA(timer_counter, ticks % TICKS_ಪ್ರತಿ_DAY);

    // Setup Century storage
    ಆದರೆ (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } ಇಲ್ಲದಿದ್ದರೆ {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        ಆದರೆ (year > 0x80)
            Century = 0x19;
        ಇಲ್ಲದಿದ್ದರೆ
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(ನಮೂದು_08));
    ಆದರೆ (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(ನಮೂದು_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a00(ರಚನೆ bregs *regs)
{
    yield();
    u32 ticks = GET_BDA(timer_counter);
    regs->cx = ticks >> 16;
    regs->dx = ticks;
    regs->al = GET_BDA(timer_rollover);
    ಗಣ_BDA(timer_rollover, 0); // reset flag
    ಗಣ_success(regs);
}

// Set Current Clock Count
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a01(ರಚನೆ bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    ಗಣ_BDA(timer_counter, ticks);
    ಗಣ_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    ಗಣ_success(regs);
}

// Read CMOS Time
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a02(ರಚನೆ bregs *regs)
{
    ಆದರೆ (rtc_updating()) {
        ಗಣ_invalid(regs);
        ಹಿಂತಿರುಗಿ;
    }

    regs->dh = rtc_read(CMOS_RTC_SECONDS);
    regs->cl = rtc_read(CMOS_RTC_MINUTES);
    regs->ch = rtc_read(CMOS_RTC_HOURS);
    regs->dl = rtc_read(CMOS_ಸ್ಥಿತಿ_B) & RTC_B_DSE;
    regs->ah = 0;
    regs->al = regs->ch;
    ಗಣ_success(regs);
}

// Set CMOS Time
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a03(ರಚನೆ bregs *regs)
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
    ಆದರೆ (rtc_updating()) {
        rtc_ಸಿದ್ಧತೆ();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS, regs->dh);
    rtc_write(CMOS_RTC_MINUTES, regs->cl);
    rtc_write(CMOS_RTC_HOURS, regs->ch);
    // Set Daylight Savings time enabled bit to requested value
    u8 val8 = ((rtc_read(CMOS_ಸ್ಥಿತಿ_B) & (RTC_B_PIE|RTC_B_AIE))
               | RTC_B_24HR | (regs->dl & RTC_B_DSE));
    rtc_write(CMOS_ಸ್ಥಿತಿ_B, val8);
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    ಗಣ_success(regs);
}

// Read CMOS Date
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a04(ರಚನೆ bregs *regs)
{
    regs->ah = 0;
    ಆದರೆ (rtc_updating()) {
        ಗಣ_invalid(regs);
        ಹಿಂತಿರುಗಿ;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_ಕೆಳಗಿನ(Century);
    regs->al = regs->ch;
    ಗಣ_success(regs);
}

// Set CMOS Date
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a05(ರಚನೆ bregs *regs)
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
    ಆದರೆ (rtc_updating()) {
        rtc_ಸಿದ್ಧತೆ();
        ಗಣ_invalid(regs);
        ಹಿಂತಿರುಗಿ;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    ಗಣ_ಕೆಳಗಿನ(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_ಸ್ಥಿತಿ_B) & ~RTC_B_ಗಣ;
    rtc_write(CMOS_ಸ್ಥಿತಿ_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    ಗಣ_success(regs);
}

// Set Alarm Time in CMOS
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a06(ರಚನೆ bregs *regs)
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
    u8 val8 = rtc_read(CMOS_ಸ್ಥಿತಿ_B); // Get Status Reg B
    regs->ax = 0;
    ಆದರೆ (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        ಗಣ_invalid(regs);
        ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (rtc_updating()) {
        rtc_ಸಿದ್ಧತೆ();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS_ALARM, regs->dh);
    rtc_write(CMOS_RTC_MINUTES_ALARM, regs->cl);
    rtc_write(CMOS_RTC_HOURS_ALARM, regs->ch);
    // enable Status Reg B alarm bit, clear halt clock bit
    rtc_write(CMOS_ಸ್ಥಿತಿ_B, (val8 & ~RTC_B_ಗಣ) | RTC_B_AIE);
    ಗಣ_success(regs);
}

// Turn off Alarm
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a07(ರಚನೆ bregs *regs)
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
    u8 val8 = rtc_read(CMOS_ಸ್ಥಿತಿ_B); // Get Status Reg B
    // clear clock-halt bit, disable alarm bit
    rtc_write(CMOS_ಸ್ಥಿತಿ_B, val8 & ~(RTC_B_ಗಣ|RTC_B_AIE));
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1abb(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_ಅಡಚಣೆ_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1aXX(ರಚನೆ bregs *regs)
{
    ಗಣ_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
ಶೂನ್ಯ VISIBLE16
handle_1a(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x00: handle_1a00(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: handle_1a01(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: handle_1a02(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: handle_1a03(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x04: handle_1a04(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x05: handle_1a05(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x06: handle_1a06(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x07: handle_1a07(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0xbb: handle_1abb(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_1aXX(regs); ನಿಲ್ಲಿಸು;
    }
}

// Update main tick counter
ಸ್ಥಿರ ಶೂನ್ಯ
clock_update(ಶೂನ್ಯ)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    ಆದರೆ (counter >= TICKS_ಪ್ರತಿ_DAY) {
        // there has been a midnight rollover at this point
        counter = 0;
        ಗಣ_BDA(timer_rollover, GET_BDA(timer_rollover) + 1);
    }
    ಗಣ_BDA(timer_counter, counter);

    // Check for internal events.
    floppy_tick();
    usb_check_event();
    ps2_check_event();
    sercon_check_event();
}

// INT 08h System Timer ISR Entry Point
ಶೂನ್ಯ VISIBLE16
handle_08(ಶೂನ್ಯ)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 ಕೊನೆಯ_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
ಶೂನ್ಯ
clock_poll_irq(ಶೂನ್ಯ)
{
    ಆದರೆ (CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!timer_check(GET_ಕೆಳಗಿನ(ಕೊನೆಯ_timer_check)))
        ಹಿಂತಿರುಗಿ;
    ಗಣ_ಕೆಳಗಿನ(ಕೊನೆಯ_timer_check, timer_calc(ticks_ಗಮ್ಯ_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 ಎಣಿಕೆ_2)
{
    ಹಿಂತಿರುಗಿ (GET_BDA(timer_counter) + ಎಣಿಕೆ_2 + 1) % TICKS_ಪ್ರತಿ_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    ಆದರೆ (!msecs)
        ಹಿಂತಿರುಗಿ GET_BDA(timer_counter);
    ಹಿಂತಿರುಗಿ irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
ಪೂರ್ಣಾಂಕ
irqtimer_check(u32 ಕೊನೆ)
{
    ಹಿಂತಿರುಗಿ (((GET_BDA(timer_counter) + TICKS_ಪ್ರತಿ_DAY - ಕೊನೆ) % TICKS_ಪ್ರತಿ_DAY)
            < (TICKS_ಪ್ರತಿ_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಗಣ_usertimer(u32 usecs, u16 seg, u16 ಸ್ಥಾನಾಂತರ_2)
{
    ಆದರೆ (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        ಹಿಂತಿರುಗಿ -1;

    // Interval not already set.
    ಗಣ_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    ಗಣ_BDA(user_wait_complete_flag, SEGOFF(seg, ಸ್ಥಾನಾಂತರ_2));
    ಗಣ_BDA(user_wait_timeout, usecs);
    rtc_use();
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
clear_usertimer(ಶೂನ್ಯ)
{
    ಆದರೆ (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        ಹಿಂತಿರುಗಿ;
    // Turn off status byte.
    ಗಣ_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#ವ್ಯಾಖ್ಯಾನಿಸು RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
ಶೂನ್ಯ
handle_1586(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_RTC_TIMER) {
        ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EUNSUPPORTED);
        ಹಿಂತಿರುಗಿ;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 ಎಣಿಕೆ_2 = (regs->cx << 16) | regs->dx;
    ಪೂರ್ಣಾಂಕ ret = ಗಣ_usertimer(ಎಣಿಕೆ_2, GET_SEG(SS), (u32)&statusflag);
    ಆದರೆ (ret) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ECLOCKINUSE);
        ಹಿಂತಿರುಗಿ;
    }
    ಆಗ (!statusflag)
        yield_toirq();
    ಗಣ_success(regs);
}

// Set Interval requested.
ಸ್ಥಿರ ಶೂನ್ಯ
handle_158300(ರಚನೆ bregs *regs)
{
    ಪೂರ್ಣಾಂಕ ret = ಗಣ_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    ಆದರೆ (ret)
        // Interval already set.
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EUNSUPPORTED);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_success(regs);
}

// Clear interval requested
ಸ್ಥಿರ ಶೂನ್ಯ
handle_158301(ರಚನೆ bregs *regs)
{
    clear_usertimer();
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1583XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

ಶೂನ್ಯ
handle_1583(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        ಹಿಂತಿರುಗಿ;
    }
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x00: handle_158300(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: handle_158301(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_1583XX(regs); ನಿಲ್ಲಿಸು;
    }
}

#ವ್ಯಾಖ್ಯಾನಿಸು USEC_ಪ್ರತಿ_RTC DIV_ಸುತ್ತು_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
ಶೂನ್ಯ VISIBLE16
handle_70(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_RTC_TIMER)
        ಹಿಂತಿರುಗಿ;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 ರಿಜಿಸ್ಟರ್_B = rtc_read(CMOS_ಸ್ಥಿತಿ_B);
    u8 ರಿಜಿಸ್ಟರ್_C = rtc_read(CMOS_ಸ್ಥಿತಿ_C);

    ಆದರೆ (!(ರಿಜಿಸ್ಟರ್_B & (RTC_B_PIE|RTC_B_AIE)))
        ಹೋಗು ಮುಗಿದ;
    ಆದರೆ (ರಿಜಿಸ್ಟರ್_C & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        ರಚನೆ bregs br;
        memset(&br, 0, ಗಾತ್ರ(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    ಆದರೆ (!(ರಿಜಿಸ್ಟರ್_C & RTC_B_PIE))
        ಹೋಗು ಮುಗಿದ;

    // Handle Periodic Interrupt.

    check_preempt();

    ಆದರೆ (!GET_BDA(rtc_wait_flag))
        ಹೋಗು ಮುಗಿದ;

    // Wait Interval (Int 15, AH=83) active.
    u32 ಸಮಯ = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    ಆದರೆ (ಸಮಯ < USEC_ಪ್ರತಿ_RTC) {
        // Done waiting - write to specified flag byte.
        ರಚನೆ segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.ಸ್ಥಾನಾಂತರ_2+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        ಗಣ_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } ಇಲ್ಲದಿದ್ದರೆ {
        // Continue waiting.
        ಸಮಯ -= USEC_ಪ್ರತಿ_RTC;
        ಗಣ_BDA(user_wait_timeout, ಸಮಯ);
    }

ಮುಗಿದ:
    pic_eoi2();
}
