// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // SET_BDA
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "hw/pic.h" // pic_eoi1
#ଅନ୍ତର୍ଭାଵ "hw/ps2port.h" // ps2_check_event
#ଅନ୍ତର୍ଭାଵ "hw/rtc.h" // rtc_read
#ଅନ୍ତର୍ଭାଵ "hw/usb-hid.h" // usb_check_event
#ଅନ୍ତର୍ଭାଵ "output.h" // debug_enter
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

ସ୍ଥିର u32
bcd2bin(u8 val)
{
    ଫେରନ୍ତୁ (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

ଶୂନ୍ୟ
clock_setup(ଶୂନ୍ୟ)
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
    ଯଦି (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } ଅନ୍ୟଥା {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        ଯଦି (year > 0x80)
            Century = 0x19;
        ଅନ୍ୟଥା
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(entry_08));
    ଯଦି (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(entry_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a00(ସଂରଚନା bregs *regs)
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a01(ସଂରଚନା bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    SET_BDA(timer_counter, ticks);
    SET_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    set_success(regs);
}

// Read CMOS Time
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a02(ସଂରଚନା bregs *regs)
{
    ଯଦି (rtc_updating()) {
        set_invalid(regs);
        ଫେରନ୍ତୁ;
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a03(ସଂରଚନା bregs *regs)
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
    ଯଦି (rtc_updating()) {
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a04(ସଂରଚନା bregs *regs)
{
    regs->ah = 0;
    ଯଦି (rtc_updating()) {
        set_invalid(regs);
        ଫେରନ୍ତୁ;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_ତଳ(Century);
    regs->al = regs->ch;
    set_success(regs);
}

// Set CMOS Date
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a05(ସଂରଚନା bregs *regs)
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
    ଯଦି (rtc_updating()) {
        rtc_setup();
        set_invalid(regs);
        ଫେରନ୍ତୁ;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    SET_ତଳ(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_STATUS_B) & ~RTC_B_SET;
    rtc_write(CMOS_STATUS_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    set_success(regs);
}

// Set Alarm Time in CMOS
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a06(ସଂରଚନା bregs *regs)
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
    ଯଦି (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        set_invalid(regs);
        ଫେରନ୍ତୁ;
    }
    ଯଦି (rtc_updating()) {
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_1a07(ସଂରଚନା bregs *regs)
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

ସ୍ଥିର ଶୂନ୍ୟ
handle_1abb(ସଂରଚନା bregs *regs)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_interrupt_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
ସ୍ଥିର ଶୂନ୍ୟ
handle_1aXX(ସଂରଚନା bregs *regs)
{
    set_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
ଶୂନ୍ୟ VISIBLE16
handle_1a(ସଂରଚନା bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    ବାଛ (regs->ah) {
    ସ୍ଥିତି 0x00: handle_1a00(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: handle_1a01(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: handle_1a02(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: handle_1a03(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x04: handle_1a04(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x05: handle_1a05(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x06: handle_1a06(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x07: handle_1a07(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0xbb: handle_1abb(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_1aXX(regs); ଭାଙ୍ଗ;
    }
}

// Update main tick counter
ସ୍ଥିର ଶୂନ୍ୟ
clock_update(ଶୂନ୍ୟ)
{
    u32 counter = GET_BDA(timer_counter);
    counter++;
    // compare to one days worth of timer ticks at 18.2 hz
    ଯଦି (counter >= TICKS_PER_DAY) {
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
ଶୂନ୍ୟ VISIBLE16
handle_08(ଶୂନ୍ୟ)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 last_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
ଶୂନ୍ୟ
clock_poll_irq(ଶୂନ୍ୟ)
{
    ଯଦି (CONFIG_HARDWARE_IRQ)
        ଫେରନ୍ତୁ;
    ଯଦି (!timer_check(GET_ତଳ(last_timer_check)))
        ଫେରନ୍ତୁ;
    SET_ତଳ(last_timer_check, timer_calc(ticks_to_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 ଗଣନା_2)
{
    ଫେରନ୍ତୁ (GET_BDA(timer_counter) + ଗଣନା_2 + 1) % TICKS_PER_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    ଯଦି (!msecs)
        ଫେରନ୍ତୁ GET_BDA(timer_counter);
    ଫେରନ୍ତୁ irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
irqtimer_check(u32 end)
{
    ଫେରନ୍ତୁ (((GET_BDA(timer_counter) + TICKS_PER_DAY - end) % TICKS_PER_DAY)
            < (TICKS_PER_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
set_usertimer(u32 usecs, u16 seg, u16 offset)
{
    ଯଦି (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        ଫେରନ୍ତୁ -1;

    // Interval not already set.
    SET_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    SET_BDA(user_wait_complete_flag, SEGOFF(seg, offset));
    SET_BDA(user_wait_timeout, usecs);
    rtc_use();
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ
clear_usertimer(ଶୂନ୍ୟ)
{
    ଯଦି (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        ଫେରନ୍ତୁ;
    // Turn off status byte.
    SET_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#define RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
ଶୂନ୍ୟ
handle_1586(ସଂରଚନା bregs *regs)
{
    ଯଦି (!CONFIG_RTC_TIMER) {
        set_code_unimplemented(regs, RET_EUNSUPPORTED);
        ଫେରନ୍ତୁ;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 ଗଣନା_2 = (regs->cx << 16) | regs->dx;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = set_usertimer(ଗଣନା_2, GET_SEG(SS), (u32)&statusflag);
    ଯଦି (ret) {
        set_code_invalid(regs, RET_ECLOCKINUSE);
        ଫେରନ୍ତୁ;
    }
    ଯେତେବେଳେ (!statusflag)
        yield_toirq();
    set_success(regs);
}

// Set Interval requested.
ସ୍ଥିର ଶୂନ୍ୟ
handle_158300(ସଂରଚନା bregs *regs)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = set_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    ଯଦି (ret)
        // Interval already set.
        set_code_invalid(regs, RET_EUNSUPPORTED);
    ଅନ୍ୟଥା
        set_success(regs);
}

// Clear interval requested
ସ୍ଥିର ଶୂନ୍ୟ
handle_158301(ସଂରଚନା bregs *regs)
{
    clear_usertimer();
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_1583XX(ସଂରଚନା bregs *regs)
{
    set_code_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

ଶୂନ୍ୟ
handle_1583(ସଂରଚନା bregs *regs)
{
    ଯଦି (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        ଫେରନ୍ତୁ;
    }
    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x00: handle_158300(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: handle_158301(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_1583XX(regs); ଭାଙ୍ଗ;
    }
}

#define USEC_PER_RTC DIV_ROUND_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
ଶୂନ୍ୟ VISIBLE16
handle_70(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_RTC_TIMER)
        ଫେରନ୍ତୁ;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 registerB = rtc_read(CMOS_STATUS_B);
    u8 registerC = rtc_read(CMOS_STATUS_C);

    ଯଦି (!(registerB & (RTC_B_PIE|RTC_B_AIE)))
        ଯାଅ done;
    ଯଦି (registerC & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        ସଂରଚନା bregs br;
        memset(&br, 0, ଆକାର(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    ଯଦି (!(registerC & RTC_B_PIE))
        ଯାଅ done;

    // Handle Periodic Interrupt.

    check_preempt();

    ଯଦି (!GET_BDA(rtc_wait_flag))
        ଯାଅ done;

    // Wait Interval (Int 15, AH=83) active.
    u32 time = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    ଯଦି (time < USEC_PER_RTC) {
        // Done waiting - write to specified flag byte.
        ସଂରଚନା segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.offset+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        SET_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } ଅନ୍ୟଥା {
        // Continue waiting.
        time -= USEC_PER_RTC;
        SET_BDA(user_wait_timeout, time);
    }

done:
    pic_eoi2();
}
