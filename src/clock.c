// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // SET_BDA
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "hw/pic.h" // pic_eoi1
#அந்தர்பாவ "hw/ps2port.h" // ps2_check_event
#அந்தர்பாவ "hw/rtc.h" // rtc_read
#அந்தர்பாவ "hw/usb-hid.h" // usb_check_event
#அந்தர்பாவ "output.h" // debug_enter
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

நிலையான u32
bcd2bin(u8 val)
{
    திரும்பு (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

வெறுமை
clock_setup(வெறுமை)
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
    எனில் (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } இல்லையெனில் {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        எனில் (year > 0x80)
            Century = 0x19;
        இல்லையெனில்
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(entry_08));
    எனில் (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(entry_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
நிலையான வெறுமை
handle_1a00(கட்டமைப்பு bregs *regs)
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
நிலையான வெறுமை
handle_1a01(கட்டமைப்பு bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    SET_BDA(timer_counter, ticks);
    SET_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    set_success(regs);
}

// Read CMOS Time
நிலையான வெறுமை
handle_1a02(கட்டமைப்பு bregs *regs)
{
    எனில் (rtc_updating()) {
        set_invalid(regs);
        திரும்பு;
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
நிலையான வெறுமை
handle_1a03(கட்டமைப்பு bregs *regs)
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
    எனில் (rtc_updating()) {
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
நிலையான வெறுமை
handle_1a04(கட்டமைப்பு bregs *regs)
{
    regs->ah = 0;
    எனில் (rtc_updating()) {
        set_invalid(regs);
        திரும்பு;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_கீழ்(Century);
    regs->al = regs->ch;
    set_success(regs);
}

// Set CMOS Date
நிலையான வெறுமை
handle_1a05(கட்டமைப்பு bregs *regs)
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
    எனில் (rtc_updating()) {
        rtc_setup();
        set_invalid(regs);
        திரும்பு;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    SET_கீழ்(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_STATUS_B) & ~RTC_B_SET;
    rtc_write(CMOS_STATUS_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    set_success(regs);
}

// Set Alarm Time in CMOS
நிலையான வெறுமை
handle_1a06(கட்டமைப்பு bregs *regs)
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
    எனில் (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        set_invalid(regs);
        திரும்பு;
    }
    எனில் (rtc_updating()) {
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
நிலையான வெறுமை
handle_1a07(கட்டமைப்பு bregs *regs)
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

நிலையான வெறுமை
handle_1abb(கட்டமைப்பு bregs *regs)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_interrupt_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
நிலையான வெறுமை
handle_1aXX(கட்டமைப்பு bregs *regs)
{
    set_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
வெறுமை VISIBLE16
handle_1a(கட்டமைப்பு bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    தேர்வு (regs->ah) {
    நிலை 0x00: handle_1a00(regs); நிறுத்து;
    நிலை 0x01: handle_1a01(regs); நிறுத்து;
    நிலை 0x02: handle_1a02(regs); நிறுத்து;
    நிலை 0x03: handle_1a03(regs); நிறுத்து;
    நிலை 0x04: handle_1a04(regs); நிறுத்து;
    நிலை 0x05: handle_1a05(regs); நிறுத்து;
    நிலை 0x06: handle_1a06(regs); நிறுத்து;
    நிலை 0x07: handle_1a07(regs); நிறுத்து;
    நிலை 0xbb: handle_1abb(regs); நிறுத்து;
    இயல்பு:   handle_1aXX(regs); நிறுத்து;
    }
}

// Update main tick counter
நிலையான வெறுமை
clock_update(வெறுமை)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    எனில் (counter >= TICKS_PER_DAY) {
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
வெறுமை VISIBLE16
handle_08(வெறுமை)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 last_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
வெறுமை
clock_poll_irq(வெறுமை)
{
    எனில் (CONFIG_HARDWARE_IRQ)
        திரும்பு;
    எனில் (!timer_check(GET_கீழ்(last_timer_check)))
        திரும்பு;
    SET_கீழ்(last_timer_check, timer_calc(ticks_to_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 மொத்தம்)
{
    திரும்பு (GET_BDA(timer_counter) + மொத்தம் + 1) % TICKS_PER_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    எனில் (!msecs)
        திரும்பு GET_BDA(timer_counter);
    திரும்பு irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
முழு
irqtimer_check(u32 end)
{
    திரும்பு (((GET_BDA(timer_counter) + TICKS_PER_DAY - end) % TICKS_PER_DAY)
            < (TICKS_PER_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

நிலையான முழு
set_usertimer(u32 usecs, u16 seg, u16 offset)
{
    எனில் (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        திரும்பு -1;

    // Interval not already set.
    SET_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    SET_BDA(user_wait_complete_flag, SEGOFF(seg, offset));
    SET_BDA(user_wait_timeout, usecs);
    rtc_use();
    திரும்பு 0;
}

நிலையான வெறுமை
clear_usertimer(வெறுமை)
{
    எனில் (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        திரும்பு;
    // Turn off status byte.
    SET_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#define RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
வெறுமை
handle_1586(கட்டமைப்பு bregs *regs)
{
    எனில் (!CONFIG_RTC_TIMER) {
        set_code_unimplemented(regs, RET_EUNSUPPORTED);
        திரும்பு;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 மொத்தம் = (regs->cx << 16) | regs->dx;
    முழு ret = set_usertimer(மொத்தம், GET_SEG(SS), (u32)&statusflag);
    எனில் (ret) {
        set_code_invalid(regs, RET_ECLOCKINUSE);
        திரும்பு;
    }
    போது (!statusflag)
        yield_toirq();
    set_success(regs);
}

// Set Interval requested.
நிலையான வெறுமை
handle_158300(கட்டமைப்பு bregs *regs)
{
    முழு ret = set_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    எனில் (ret)
        // Interval already set.
        set_code_invalid(regs, RET_EUNSUPPORTED);
    இல்லையெனில்
        set_success(regs);
}

// Clear interval requested
நிலையான வெறுமை
handle_158301(கட்டமைப்பு bregs *regs)
{
    clear_usertimer();
    set_success(regs);
}

நிலையான வெறுமை
handle_1583XX(கட்டமைப்பு bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

வெறுமை
handle_1583(கட்டமைப்பு bregs *regs)
{
    எனில் (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        திரும்பு;
    }
    தேர்வு (regs->al) {
    நிலை 0x00: handle_158300(regs); நிறுத்து;
    நிலை 0x01: handle_158301(regs); நிறுத்து;
    இயல்பு:   handle_1583XX(regs); நிறுத்து;
    }
}

#define USEC_PER_RTC DIV_ROUND_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
வெறுமை VISIBLE16
handle_70(வெறுமை)
{
    எனில் (!CONFIG_RTC_TIMER)
        திரும்பு;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 registerB = rtc_read(CMOS_STATUS_B);
    u8 registerC = rtc_read(CMOS_STATUS_C);

    எனில் (!(registerB & (RTC_B_PIE|RTC_B_AIE)))
        செல் done;
    எனில் (registerC & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        கட்டமைப்பு bregs br;
        memset(&br, 0, அளவு(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    எனில் (!(registerC & RTC_B_PIE))
        செல் done;

    // Handle Periodic Interrupt.

    check_preempt();

    எனில் (!GET_BDA(rtc_wait_flag))
        செல் done;

    // Wait Interval (Int 15, AH=83) active.
    u32 time = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    எனில் (time < USEC_PER_RTC) {
        // Done waiting - write to specified flag byte.
        கட்டமைப்பு segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.offset+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        SET_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } இல்லையெனில் {
        // Continue waiting.
        time -= USEC_PER_RTC;
        SET_BDA(user_wait_timeout, time);
    }

done:
    pic_eoi2();
}
