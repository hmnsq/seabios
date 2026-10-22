// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // SET_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pic.h" // pic_eoi1
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ps2port.h" // ps2_check_event
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // rtc_read
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/usb-hid.h" // usb_check_event
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // debug_enter
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

ਸਥਿਰ u32
bcd2bin(u8 val)
{
    ਵਾਪਸ (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

ਖਾਲੀ
clock_ਤਿਆਰੀ(ਖਾਲੀ)
{
    dprintf(3, "init timer\n");
    pit_ਤਿਆਰੀ();

    rtc_ਤਿਆਰੀ();
    rtc_updating();
    u32 seconds = bcd2bin(rtc_read(CMOS_RTC_SECONDS));
    u32 minutes = bcd2bin(rtc_read(CMOS_RTC_MINUTES));
    u32 hours = bcd2bin(rtc_read(CMOS_RTC_HOURS));
    u32 ticks = ticks_from_ms(((hours * 60 + minutes) * 60 + seconds) * 1000);
    ਸੈੱਟ_BDA(timer_counter, ticks % TICKS_ਪ੍ਰਤੀ_DAY);

    // Setup Century storage
    ਜੇ (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } ਨਹੀਂਤਾਂ {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        ਜੇ (year > 0x80)
            Century = 0x19;
        ਨਹੀਂਤਾਂ
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(ਇੰਦਰਾਜ_08));
    ਜੇ (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(ਇੰਦਰਾਜ_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
ਸਥਿਰ ਖਾਲੀ
handle_1a00(ਬਣਤਰ bregs *regs)
{
    yield();
    u32 ticks = GET_BDA(timer_counter);
    regs->cx = ticks >> 16;
    regs->dx = ticks;
    regs->al = GET_BDA(timer_rollover);
    ਸੈੱਟ_BDA(timer_rollover, 0); // reset flag
    ਸੈੱਟ_success(regs);
}

// Set Current Clock Count
ਸਥਿਰ ਖਾਲੀ
handle_1a01(ਬਣਤਰ bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    ਸੈੱਟ_BDA(timer_counter, ticks);
    ਸੈੱਟ_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    ਸੈੱਟ_success(regs);
}

// Read CMOS Time
ਸਥਿਰ ਖਾਲੀ
handle_1a02(ਬਣਤਰ bregs *regs)
{
    ਜੇ (rtc_updating()) {
        ਸੈੱਟ_invalid(regs);
        ਵਾਪਸ;
    }

    regs->dh = rtc_read(CMOS_RTC_SECONDS);
    regs->cl = rtc_read(CMOS_RTC_MINUTES);
    regs->ch = rtc_read(CMOS_RTC_HOURS);
    regs->dl = rtc_read(CMOS_ਸਥਿਤੀ_B) & RTC_B_DSE;
    regs->ah = 0;
    regs->al = regs->ch;
    ਸੈੱਟ_success(regs);
}

// Set CMOS Time
ਸਥਿਰ ਖਾਲੀ
handle_1a03(ਬਣਤਰ bregs *regs)
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
    ਜੇ (rtc_updating()) {
        rtc_ਤਿਆਰੀ();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS, regs->dh);
    rtc_write(CMOS_RTC_MINUTES, regs->cl);
    rtc_write(CMOS_RTC_HOURS, regs->ch);
    // Set Daylight Savings time enabled bit to requested value
    u8 val8 = ((rtc_read(CMOS_ਸਥਿਤੀ_B) & (RTC_B_PIE|RTC_B_AIE))
               | RTC_B_24HR | (regs->dl & RTC_B_DSE));
    rtc_write(CMOS_ਸਥਿਤੀ_B, val8);
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    ਸੈੱਟ_success(regs);
}

// Read CMOS Date
ਸਥਿਰ ਖਾਲੀ
handle_1a04(ਬਣਤਰ bregs *regs)
{
    regs->ah = 0;
    ਜੇ (rtc_updating()) {
        ਸੈੱਟ_invalid(regs);
        ਵਾਪਸ;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_ਨੀਵਾਂ(Century);
    regs->al = regs->ch;
    ਸੈੱਟ_success(regs);
}

// Set CMOS Date
ਸਥਿਰ ਖਾਲੀ
handle_1a05(ਬਣਤਰ bregs *regs)
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
    ਜੇ (rtc_updating()) {
        rtc_ਤਿਆਰੀ();
        ਸੈੱਟ_invalid(regs);
        ਵਾਪਸ;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    ਸੈੱਟ_ਨੀਵਾਂ(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_ਸਥਿਤੀ_B) & ~RTC_B_ਸੈੱਟ;
    rtc_write(CMOS_ਸਥਿਤੀ_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    ਸੈੱਟ_success(regs);
}

// Set Alarm Time in CMOS
ਸਥਿਰ ਖਾਲੀ
handle_1a06(ਬਣਤਰ bregs *regs)
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
    u8 val8 = rtc_read(CMOS_ਸਥਿਤੀ_B); // Get Status Reg B
    regs->ax = 0;
    ਜੇ (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        ਸੈੱਟ_invalid(regs);
        ਵਾਪਸ;
    }
    ਜੇ (rtc_updating()) {
        rtc_ਤਿਆਰੀ();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS_ALARM, regs->dh);
    rtc_write(CMOS_RTC_MINUTES_ALARM, regs->cl);
    rtc_write(CMOS_RTC_HOURS_ALARM, regs->ch);
    // enable Status Reg B alarm bit, clear halt clock bit
    rtc_write(CMOS_ਸਥਿਤੀ_B, (val8 & ~RTC_B_ਸੈੱਟ) | RTC_B_AIE);
    ਸੈੱਟ_success(regs);
}

// Turn off Alarm
ਸਥਿਰ ਖਾਲੀ
handle_1a07(ਬਣਤਰ bregs *regs)
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
    u8 val8 = rtc_read(CMOS_ਸਥਿਤੀ_B); // Get Status Reg B
    // clear clock-halt bit, disable alarm bit
    rtc_write(CMOS_ਸਥਿਤੀ_B, val8 & ~(RTC_B_ਸੈੱਟ|RTC_B_AIE));
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_1abb(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_ਰੁਕਾਵਟ_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
ਸਥਿਰ ਖਾਲੀ
handle_1aXX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
ਖਾਲੀ VISIBLE16
handle_1a(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x00: handle_1a00(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: handle_1a01(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: handle_1a02(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x03: handle_1a03(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x04: handle_1a04(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x05: handle_1a05(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x06: handle_1a06(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x07: handle_1a07(regs); ਰੋਕੋ;
    ਹਾਲਤ 0xbb: handle_1abb(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_1aXX(regs); ਰੋਕੋ;
    }
}

// Update main tick counter
ਸਥਿਰ ਖਾਲੀ
clock_update(ਖਾਲੀ)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    ਜੇ (counter >= TICKS_ਪ੍ਰਤੀ_DAY) {
        // there has been a midnight rollover at this point
        counter = 0;
        ਸੈੱਟ_BDA(timer_rollover, GET_BDA(timer_rollover) + 1);
    }
    ਸੈੱਟ_BDA(timer_counter, counter);

    // Check for internal events.
    floppy_tick();
    usb_check_event();
    ps2_check_event();
    sercon_check_event();
}

// INT 08h System Timer ISR Entry Point
ਖਾਲੀ VISIBLE16
handle_08(ਖਾਲੀ)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 ਆਖਰੀ_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
ਖਾਲੀ
clock_poll_irq(ਖਾਲੀ)
{
    ਜੇ (CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    ਜੇ (!timer_check(GET_ਨੀਵਾਂ(ਆਖਰੀ_timer_check)))
        ਵਾਪਸ;
    ਸੈੱਟ_ਨੀਵਾਂ(ਆਖਰੀ_timer_check, timer_calc(ticks_ਮੰਜ਼ਿਲ_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 ਗਿਣਤੀ_2)
{
    ਵਾਪਸ (GET_BDA(timer_counter) + ਗਿਣਤੀ_2 + 1) % TICKS_ਪ੍ਰਤੀ_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    ਜੇ (!msecs)
        ਵਾਪਸ GET_BDA(timer_counter);
    ਵਾਪਸ irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
ਪੂਰਨ
irqtimer_check(u32 ਸਿਰਾ)
{
    ਵਾਪਸ (((GET_BDA(timer_counter) + TICKS_ਪ੍ਰਤੀ_DAY - ਸਿਰਾ) % TICKS_ਪ੍ਰਤੀ_DAY)
            < (TICKS_ਪ੍ਰਤੀ_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ
ਸੈੱਟ_usertimer(u32 usecs, u16 seg, u16 ਵਿੱਥ_2)
{
    ਜੇ (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        ਵਾਪਸ -1;

    // Interval not already set.
    ਸੈੱਟ_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    ਸੈੱਟ_BDA(user_wait_complete_flag, SEGOFF(seg, ਵਿੱਥ_2));
    ਸੈੱਟ_BDA(user_wait_timeout, usecs);
    rtc_use();
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ
clear_usertimer(ਖਾਲੀ)
{
    ਜੇ (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        ਵਾਪਸ;
    // Turn off status byte.
    ਸੈੱਟ_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
ਖਾਲੀ
handle_1586(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_RTC_TIMER) {
        ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EUNSUPPORTED);
        ਵਾਪਸ;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 ਗਿਣਤੀ_2 = (regs->cx << 16) | regs->dx;
    ਪੂਰਨ ret = ਸੈੱਟ_usertimer(ਗਿਣਤੀ_2, GET_SEG(SS), (u32)&statusflag);
    ਜੇ (ret) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ECLOCKINUSE);
        ਵਾਪਸ;
    }
    ਜਦੋਂ (!statusflag)
        yield_toirq();
    ਸੈੱਟ_success(regs);
}

// Set Interval requested.
ਸਥਿਰ ਖਾਲੀ
handle_158300(ਬਣਤਰ bregs *regs)
{
    ਪੂਰਨ ret = ਸੈੱਟ_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    ਜੇ (ret)
        // Interval already set.
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EUNSUPPORTED);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_success(regs);
}

// Clear interval requested
ਸਥਿਰ ਖਾਲੀ
handle_158301(ਬਣਤਰ bregs *regs)
{
    clear_usertimer();
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_1583XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

ਖਾਲੀ
handle_1583(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        ਵਾਪਸ;
    }
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x00: handle_158300(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: handle_158301(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_1583XX(regs); ਰੋਕੋ;
    }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USEC_ਪ੍ਰਤੀ_RTC DIV_ਗੇੜਾ_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
ਖਾਲੀ VISIBLE16
handle_70(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_RTC_TIMER)
        ਵਾਪਸ;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 ਰਜਿਸਟਰ_B = rtc_read(CMOS_ਸਥਿਤੀ_B);
    u8 ਰਜਿਸਟਰ_C = rtc_read(CMOS_ਸਥਿਤੀ_C);

    ਜੇ (!(ਰਜਿਸਟਰ_B & (RTC_B_PIE|RTC_B_AIE)))
        ਜਾਓ ਪੂਰਾ;
    ਜੇ (ਰਜਿਸਟਰ_C & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        ਬਣਤਰ bregs br;
        memset(&br, 0, ਆਕਾਰ(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    ਜੇ (!(ਰਜਿਸਟਰ_C & RTC_B_PIE))
        ਜਾਓ ਪੂਰਾ;

    // Handle Periodic Interrupt.

    check_preempt();

    ਜੇ (!GET_BDA(rtc_wait_flag))
        ਜਾਓ ਪੂਰਾ;

    // Wait Interval (Int 15, AH=83) active.
    u32 ਸਮਾਂ = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    ਜੇ (ਸਮਾਂ < USEC_ਪ੍ਰਤੀ_RTC) {
        // Done waiting - write to specified flag byte.
        ਬਣਤਰ segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.ਵਿੱਥ_2+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        ਸੈੱਟ_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } ਨਹੀਂਤਾਂ {
        // Continue waiting.
        ਸਮਾਂ -= USEC_ਪ੍ਰਤੀ_RTC;
        ਸੈੱਟ_BDA(user_wait_timeout, ਸਮਾਂ);
    }

ਪੂਰਾ:
    pic_eoi2();
}
