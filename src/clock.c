// 16bit code to handle system clocks.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // SET_BDA
#شامل "bregs.h" // struct bregs
#شامل "hw/pic.h" // pic_eoi1
#شامل "hw/ps2port.h" // ps2_check_event
#شامل "hw/rtc.h" // rtc_read
#شامل "hw/usb-hid.h" // usb_check_event
#شامل "output.h" // debug_enter
#شامل "stacks.h" // yield
#شامل "string.h" // memset
#شامل "util.h" // clock_setup


/****************************************************************
 * Init
 ****************************************************************/

ساکن u32
bcd2bin(u8 val)
{
    واپس (val & 0xf) + ((val >> 4) * 10);
}

u8 Century VARLOW;

خالی
clock_تیاری(خالی)
{
    dprintf(3, "init timer\n");
    pit_تیاری();

    rtc_تیاری();
    rtc_updating();
    u32 seconds = bcd2bin(rtc_read(CMOS_RTC_SECONDS));
    u32 minutes = bcd2bin(rtc_read(CMOS_RTC_MINUTES));
    u32 hours = bcd2bin(rtc_read(CMOS_RTC_HOURS));
    u32 ticks = ticks_from_ms(((hours * 60 + minutes) * 60 + seconds) * 1000);
    سیٹ_BDA(timer_شمار, ticks % TICKS_فی_DAY);

    // Setup Century storage
    اگر (CONFIG_QEMU) {
        Century = rtc_read(CMOS_CENTURY);
    } ورنہ {
        // Infer current century from the year.
        u8 year = rtc_read(CMOS_RTC_YEAR);
        اگر (year > 0x80)
            Century = 0x19;
        ورنہ
            Century = 0x20;
    }

    enable_hwirq(0, FUNC16(اندراج_08));
    اگر (CONFIG_RTC_TIMER)
        enable_hwirq(8, FUNC16(اندراج_70));
}


/****************************************************************
 * Standard clock functions
 ****************************************************************/

// get current clock count
ساکن خالی
handle_1a00(ساخت bregs *regs)
{
    yield();
    u32 ticks = GET_BDA(timer_شمار);
    regs->cx = ticks >> 16;
    regs->dx = ticks;
    regs->al = GET_BDA(timer_rollover);
    سیٹ_BDA(timer_rollover, 0); // reset flag
    سیٹ_success(regs);
}

// Set Current Clock Count
ساکن خالی
handle_1a01(ساخت bregs *regs)
{
    u32 ticks = (regs->cx << 16) | regs->dx;
    سیٹ_BDA(timer_شمار, ticks);
    سیٹ_BDA(timer_rollover, 0); // reset flag
    // XXX - should use set_code_success()?
    regs->ah = 0;
    سیٹ_success(regs);
}

// Read CMOS Time
ساکن خالی
handle_1a02(ساخت bregs *regs)
{
    اگر (rtc_updating()) {
        سیٹ_invalid(regs);
        واپس;
    }

    regs->dh = rtc_read(CMOS_RTC_SECONDS);
    regs->cl = rtc_read(CMOS_RTC_MINUTES);
    regs->ch = rtc_read(CMOS_RTC_HOURS);
    regs->dl = rtc_read(CMOS_حالت_B) & RTC_B_DSE;
    regs->ah = 0;
    regs->al = regs->ch;
    سیٹ_success(regs);
}

// Set CMOS Time
ساکن خالی
handle_1a03(ساخت bregs *regs)
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
    اگر (rtc_updating()) {
        rtc_تیاری();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS, regs->dh);
    rtc_write(CMOS_RTC_MINUTES, regs->cl);
    rtc_write(CMOS_RTC_HOURS, regs->ch);
    // Set Daylight Savings time enabled bit to requested value
    u8 val8 = ((rtc_read(CMOS_حالت_B) & (RTC_B_PIE|RTC_B_AIE))
               | RTC_B_24HR | (regs->dl & RTC_B_DSE));
    rtc_write(CMOS_حالت_B, val8);
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    سیٹ_success(regs);
}

// Read CMOS Date
ساکن خالی
handle_1a04(ساخت bregs *regs)
{
    regs->ah = 0;
    اگر (rtc_updating()) {
        سیٹ_invalid(regs);
        واپس;
    }
    regs->cl = rtc_read(CMOS_RTC_YEAR);
    regs->dh = rtc_read(CMOS_RTC_MONTH);
    regs->dl = rtc_read(CMOS_RTC_DAY_MONTH);
    regs->ch = GET_کم(Century);
    regs->al = regs->ch;
    سیٹ_success(regs);
}

// Set CMOS Date
ساکن خالی
handle_1a05(ساخت bregs *regs)
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
    اگر (rtc_updating()) {
        rtc_تیاری();
        سیٹ_invalid(regs);
        واپس;
    }
    rtc_write(CMOS_RTC_YEAR, regs->cl);
    rtc_write(CMOS_RTC_MONTH, regs->dh);
    rtc_write(CMOS_RTC_DAY_MONTH, regs->dl);
    سیٹ_کم(Century, regs->ch);
    // clear halt-clock bit
    u8 val8 = rtc_read(CMOS_حالت_B) & ~RTC_B_سیٹ;
    rtc_write(CMOS_حالت_B, val8);
    regs->ah = 0;
    regs->al = val8; // AL = val last written to Reg B
    سیٹ_success(regs);
}

// Set Alarm Time in CMOS
ساکن خالی
handle_1a06(ساخت bregs *regs)
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
    u8 val8 = rtc_read(CMOS_حالت_B); // Get Status Reg B
    regs->ax = 0;
    اگر (val8 & RTC_B_AIE) {
        // Alarm interrupt enabled already
        سیٹ_invalid(regs);
        واپس;
    }
    اگر (rtc_updating()) {
        rtc_تیاری();
        // fall through as if an update were not in progress
    }
    rtc_write(CMOS_RTC_SECONDS_ALARM, regs->dh);
    rtc_write(CMOS_RTC_MINUTES_ALARM, regs->cl);
    rtc_write(CMOS_RTC_HOURS_ALARM, regs->ch);
    // enable Status Reg B alarm bit, clear halt clock bit
    rtc_write(CMOS_حالت_B, (val8 & ~RTC_B_سیٹ) | RTC_B_AIE);
    سیٹ_success(regs);
}

// Turn off Alarm
ساکن خالی
handle_1a07(ساخت bregs *regs)
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
    u8 val8 = rtc_read(CMOS_حالت_B); // Get Status Reg B
    // clear clock-halt bit, disable alarm bit
    rtc_write(CMOS_حالت_B, val8 & ~(RTC_B_سیٹ|RTC_B_AIE));
    regs->ah = 0;
    regs->al = val8; // val last written to Reg B
    سیٹ_success(regs);
}

ساکن خالی
handle_1abb(ساخت bregs *regs)
{
    اگر (!CONFIG_TCGBIOS)
        واپس;

    dprintf(DEBUG_tcg, "16: Calling tpm_interrupt_handler\n");
    call32(tpm_مداخلت_handler32, MAKE_FLATPTR(GET_SEG(SS), regs), 0);
}

// Unsupported
ساکن خالی
handle_1aXX(ساخت bregs *regs)
{
    سیٹ_unimplemented(regs);
}

// INT 1Ah Time-of-day Service Entry Point
خالی VISIBLE16
handle_1a(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_1a);
    انتخاب (regs->ah) {
    حالت 0x00: handle_1a00(regs); توڑو;
    حالت 0x01: handle_1a01(regs); توڑو;
    حالت 0x02: handle_1a02(regs); توڑو;
    حالت 0x03: handle_1a03(regs); توڑو;
    حالت 0x04: handle_1a04(regs); توڑو;
    حالت 0x05: handle_1a05(regs); توڑو;
    حالت 0x06: handle_1a06(regs); توڑو;
    حالت 0x07: handle_1a07(regs); توڑو;
    حالت 0xbb: handle_1abb(regs); توڑو;
    طےشدہ:   handle_1aXX(regs); توڑو;
    }
}

// Update main tick counter
ساکن خالی
clock_update(خالی)
{
    u32 شمار_2 = GET_BDA(timer_شمار);
    شمار_2++;
    // compare to one days worth of timer ticks at 18.2 hz
    اگر (شمار_2 >= TICKS_فی_DAY) {
        // there has been a midnight rollover at this point
        شمار_2 = 0;
        سیٹ_BDA(timer_rollover, GET_BDA(timer_rollover) + 1);
    }
    سیٹ_BDA(timer_شمار, شمار_2);

    // Check for internal events.
    floppy_tick();
    usb_check_event();
    ps2_check_event();
    sercon_check_event();
}

// INT 08h System Timer ISR Entry Point
خالی VISIBLE16
handle_08(خالی)
{
    debug_isr(DEBUG_ISR_08);
    clock_update();

    // chain to user timer tick INT #0x1c
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x1c, &br);

    pic_eoi1();
}

u32 آخری_timer_check VARLOW;

// Simulate timer irq on machines without hardware irqs
خالی
clock_poll_irq(خالی)
{
    اگر (CONFIG_HARDWARE_IRQ)
        واپس;
    اگر (!timer_check(GET_کم(آخری_timer_check)))
        واپس;
    سیٹ_کم(آخری_timer_check, timer_calc(ticks_مقصد_ms(1)));
    clock_update();
}


/****************************************************************
 * IRQ based timer
 ****************************************************************/

// Calculate the timer value at 'count' number of full timer ticks in
// the future.
u32
irqtimer_calc_ticks(u32 گنتی)
{
    واپس (GET_BDA(timer_شمار) + گنتی + 1) % TICKS_فی_DAY;
}

// Return the timer value that is 'msecs' time in the future.
u32
irqtimer_calc(u32 msecs)
{
    اگر (!msecs)
        واپس GET_BDA(timer_شمار);
    واپس irqtimer_calc_ticks(ticks_from_ms(msecs));
}

// Check if the given timer value has passed.
صحیح
irqtimer_check(u32 انتہا)
{
    واپس (((GET_BDA(timer_شمار) + TICKS_فی_DAY - انتہا) % TICKS_فی_DAY)
            < (TICKS_فی_DAY/2));
}


/****************************************************************
 * Periodic timer
 ****************************************************************/

ساکن صحیح
سیٹ_usertimer(u32 usecs, u16 seg, u16 انحراف_2)
{
    اگر (GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING)
        واپس -1;

    // Interval not already set.
    سیٹ_BDA(rtc_wait_flag, RWS_WAIT_PENDING);  // Set status byte.
    سیٹ_BDA(user_wait_complete_flag, SEGOFF(seg, انحراف_2));
    سیٹ_BDA(user_wait_timeout, usecs);
    rtc_use();
    واپس 0;
}

ساکن خالی
clear_usertimer(خالی)
{
    اگر (!(GET_BDA(rtc_wait_flag) & RWS_WAIT_PENDING))
        واپس;
    // Turn off status byte.
    سیٹ_BDA(rtc_wait_flag, 0);
    rtc_release();
}

#تعریف_کرو RET_ECLOCKINUSE  0x83

// Wait for CX:DX microseconds
خالی
handle_1586(ساخت bregs *regs)
{
    اگر (!CONFIG_RTC_TIMER) {
        سیٹ_رمز_unimplemented(regs, RET_EUNSUPPORTED);
        واپس;
    }
    // Use the rtc to wait for the specified time.
    u8 statusflag = 0;
    u32 گنتی = (regs->cx << 16) | regs->dx;
    صحیح ret = سیٹ_usertimer(گنتی, GET_SEG(SS), (u32)&statusflag);
    اگر (ret) {
        سیٹ_رمز_invalid(regs, RET_ECLOCKINUSE);
        واپس;
    }
    جب (!statusflag)
        yield_toirq();
    سیٹ_success(regs);
}

// Set Interval requested.
ساکن خالی
handle_158300(ساخت bregs *regs)
{
    صحیح ret = سیٹ_usertimer((regs->cx << 16) | regs->dx, regs->es, regs->bx);
    اگر (ret)
        // Interval already set.
        سیٹ_رمز_invalid(regs, RET_EUNSUPPORTED);
    ورنہ
        سیٹ_success(regs);
}

// Clear interval requested
ساکن خالی
handle_158301(ساخت bregs *regs)
{
    clear_usertimer();
    سیٹ_success(regs);
}

ساکن خالی
handle_1583XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EUNSUPPORTED);
    regs->al--;
}

خالی
handle_1583(ساخت bregs *regs)
{
    اگر (!CONFIG_RTC_TIMER) {
        handle_1583XX(regs);
        واپس;
    }
    انتخاب (regs->al) {
    حالت 0x00: handle_158300(regs); توڑو;
    حالت 0x01: handle_158301(regs); توڑو;
    طےشدہ:   handle_1583XX(regs); توڑو;
    }
}

#تعریف_کرو USEC_فی_RTC DIV_باری_CLOSEST(1000000, 1024)

// int70h: IRQ8 - CMOS RTC
خالی VISIBLE16
handle_70(خالی)
{
    اگر (!CONFIG_RTC_TIMER)
        واپس;
    debug_isr(DEBUG_ISR_70);

    // Check which modes are enabled and have occurred.
    u8 رجسٹر_B = rtc_read(CMOS_حالت_B);
    u8 رجسٹر_C = rtc_read(CMOS_حالت_C);

    اگر (!(رجسٹر_B & (RTC_B_PIE|RTC_B_AIE)))
        جاؤ مکمل;
    اگر (رجسٹر_C & RTC_B_AIE) {
        // Handle Alarm Interrupt.
        ساخت bregs br;
        memset(&br, 0, حجم(br));
        br.flags = F_IF;
        call16_int(0x4a, &br);
    }
    اگر (!(رجسٹر_C & RTC_B_PIE))
        جاؤ مکمل;

    // Handle Periodic Interrupt.

    check_preempt();

    اگر (!GET_BDA(rtc_wait_flag))
        جاؤ مکمل;

    // Wait Interval (Int 15, AH=83) active.
    u32 وقت = GET_BDA(user_wait_timeout);  // Time left in microseconds.
    اگر (وقت < USEC_فی_RTC) {
        // Done waiting - write to specified flag byte.
        ساخت segoff_s segoff = GET_BDA(user_wait_complete_flag);
        u16 ptr_seg = segoff.seg;
        u8 *ptr_far = (u8*)(segoff.انحراف_2+0);
        u8 oldval = GET_FARVAR(ptr_seg, *ptr_far);
        سیٹ_FARVAR(ptr_seg, *ptr_far, oldval | 0x80);

        clear_usertimer();
    } ورنہ {
        // Continue waiting.
        وقت -= USEC_فی_RTC;
        سیٹ_BDA(user_wait_timeout, وقت);
    }

مکمل:
    pic_eoi2();
}
