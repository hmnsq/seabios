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
    ಪ್ರತ್ಯಾಗಮ (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

ಶೂನ್ಯ
clock_setup(ಶೂನ್ಯ)
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
    ಯದಿ (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } ಅನ್ಯಥಾ {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        ಯದಿ (year > 0x80)
            Century = 0x19;
        ಅನ್ಯಥಾ
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(entry_08));
    ಯದಿ (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(entry_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a00(ಸಂರಚನಾ bregs *regs)
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
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a01(ಸಂರಚನಾ bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    SET_BDA(timer_counter, ticks);
    SET_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    set_success(regs);
}

// Read CMOS Time
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a02(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (rtc_updating()) {
        set_invalid(regs);
        ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a03(ಸಂರಚನಾ bregs *regs)
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
    ಯದಿ (rtc_updating()) {
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
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a04(ಸಂರಚನಾ bregs *regs)
{
    regs->ah = 0;
    ಯದಿ (rtc_updating()) {
        set_invalid(regs);
        ಪ್ರತ್ಯಾಗಮ;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_ಕೆಳಗಿನ(Century);
    regs->al = regs->ch;
    set_success(regs);
}

// Set CMOS Date
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a05(ಸಂರಚನಾ bregs *regs)
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
    ಯದಿ (rtc_updating()) {
        rtc_setup();
        set_invalid(regs);
        ಪ್ರತ್ಯಾಗಮ;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    SET_ಕೆಳಗಿನ(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_STATUS_B) & ~RTC_B_SET;
    rtc_write(CMOS_STATUS_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    set_success(regs);
}

// Set Alarm Time in CMOS
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a06(ಸಂರಚನಾ bregs *regs)
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
    ಯದಿ (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        set_invalid(regs);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (rtc_updating()) {
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
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1a07(ಸಂರಚನಾ bregs *regs)
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

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1abb(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_interrupt_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1aXX(ಸಂರಚನಾ bregs *regs)
{
    set_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
ಶೂನ್ಯ VISIBLE16
handle_1a(ಸಂರಚನಾ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    ವಿಕಲ್ಪ (regs->ah) {
    ಪಕ್ಷ 0x00: handle_1a00(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: handle_1a01(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: handle_1a02(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x03: handle_1a03(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x04: handle_1a04(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x05: handle_1a05(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x06: handle_1a06(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x07: handle_1a07(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0xbb: handle_1abb(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   handle_1aXX(regs); ಭಙ್ಗ;
    }
}

// Update main tick counter
ಸ್ಥಿರ ಶೂನ್ಯ
clock_update(ಶೂನ್ಯ)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    ಯದಿ (counter >= TICKS_PER_DAY) {
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
ಶೂನ್ಯ VISIBLE16
handle_08(ಶೂನ್ಯ)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 last_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
ಶೂನ್ಯ
clock_poll_irq(ಶೂನ್ಯ)
{
    ಯದಿ (CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (!timer_check(GET_ಕೆಳಗಿನ(last_timer_check)))
        ಪ್ರತ್ಯಾಗಮ;
    SET_ಕೆಳಗಿನ(last_timer_check, timer_calc(ticks_to_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 ಎಣಿಕೆ)
{
    ಪ್ರತ್ಯಾಗಮ (GET_BDA(timer_counter) + ಎಣಿಕೆ + 1) % TICKS_PER_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    ಯದಿ (!msecs)
        ಪ್ರತ್ಯಾಗಮ GET_BDA(timer_counter);
    ಪ್ರತ್ಯಾಗಮ irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
ಪೂರ್ಣಾಙ್ಕ
irqtimer_check(u32 end)
{
    ಪ್ರತ್ಯಾಗಮ (((GET_BDA(timer_counter) + TICKS_PER_DAY - end) % TICKS_PER_DAY)
            < (TICKS_PER_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
set_usertimer(u32 usecs, u16 seg, u16 offset)
{
    ಯದಿ (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        ಪ್ರತ್ಯಾಗಮ -1;

    // Interval not already set.
    SET_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    SET_BDA(user_wait_complete_flag, SEGOFF(seg, offset));
    SET_BDA(user_wait_timeout, usecs);
    rtc_use();
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
clear_usertimer(ಶೂನ್ಯ)
{
    ಯದಿ (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        ಪ್ರತ್ಯಾಗಮ;
    // Turn off status byte.
    SET_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#define RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
ಶೂನ್ಯ
handle_1586(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_RTC_TIMER) {
        set_code_unimplemented(regs, RET_EUNSUPPORTED);
        ಪ್ರತ್ಯಾಗಮ;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 ಎಣಿಕೆ = (regs->cx << 16) | regs->dx;
    ಪೂರ್ಣಾಙ್ಕ ret = set_usertimer(ಎಣಿಕೆ, GET_SEG(SS), (u32)&statusflag);
    ಯದಿ (ret) {
        set_code_invalid(regs, RET_ECLOCKINUSE);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಯಾವತ್ (!statusflag)
        yield_toirq();
    set_success(regs);
}

// Set Interval requested.
ಸ್ಥಿರ ಶೂನ್ಯ
handle_158300(ಸಂರಚನಾ bregs *regs)
{
    ಪೂರ್ಣಾಙ್ಕ ret = set_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    ಯದಿ (ret)
        // Interval already set.
        set_code_invalid(regs, RET_EUNSUPPORTED);
    ಅನ್ಯಥಾ
        set_success(regs);
}

// Clear interval requested
ಸ್ಥಿರ ಶೂನ್ಯ
handle_158301(ಸಂರಚನಾ bregs *regs)
{
    clear_usertimer();
    set_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1583XX(ಸಂರಚನಾ bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

ಶೂನ್ಯ
handle_1583(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x00: handle_158300(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: handle_158301(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   handle_1583XX(regs); ಭಙ್ಗ;
    }
}

#define USEC_PER_RTC DIV_ROUND_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
ಶೂನ್ಯ VISIBLE16
handle_70(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_RTC_TIMER)
        ಪ್ರತ್ಯಾಗಮ;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 registerB = rtc_read(CMOS_STATUS_B);
    u8 registerC = rtc_read(CMOS_STATUS_C);

    ಯದಿ (!(registerB & (RTC_B_PIE|RTC_B_AIE)))
        ಗಚ್ಛ done;
    ಯದಿ (registerC & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        ಸಂರಚನಾ bregs br;
        memset(&br, 0, ಪರಿಮಾಣ(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    ಯದಿ (!(registerC & RTC_B_PIE))
        ಗಚ್ಛ done;

    // Handle Periodic Interrupt.

    check_preempt();

    ಯದಿ (!GET_BDA(rtc_wait_flag))
        ಗಚ್ಛ done;

    // Wait Interval (Int 15, AH=83) active.
    u32 time = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    ಯದಿ (time < USEC_PER_RTC) {
        // Done waiting - write to specified flag byte.
        ಸಂರಚನಾ segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.offset+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        SET_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } ಅನ್ಯಥಾ {
        // Continue waiting.
        time -= USEC_PER_RTC;
        SET_BDA(user_wait_timeout, time);
    }

done:
    pic_eoi2();
}
