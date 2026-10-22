// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_LOW
#شامل "rtc.h" // rtc_read
#شامل "stacks.h" // yield
#شامل "util.h" // timer_calc
#شامل "x86.h" // inb

u8
rtc_read(u8 index)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    بازگشت inb(PORT_CMOS_DATA);
}

تهی
rtc_write(u8 index, u8 val)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    outb(val, PORT_CMOS_DATA);
}

تهی
rtc_mask(u8 index, u8 off, u8 on)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    u8 val = inb(PORT_CMOS_DATA);
    outb((val & ~off) | on, PORT_CMOS_DATA);
}

صحیح
rtc_updating(تهی)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    اگر ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        بازگشت 0;
    u32 end = timer_calc(15);
    برای (;;) {
        اگر ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            بازگشت 0;
        اگر (timer_check(end))
            // update-in-progress never transitioned to 0
            بازگشت -1;
        yield();
    }
}

تهی
rtc_setup(تهی)
{
    اگر (!CONFIG_RTC_TIMER)
        بازگشت;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

صحیح RTCusers VARLOW;

تهی
rtc_use(تهی)
{
    اگر (!CONFIG_RTC_TIMER)
        بازگشت;
    صحیح شمار = GET_کم(RTCusers);
    SET_کم(RTCusers, شمار+1);
    اگر (شمار)
        بازگشت;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

تهی
rtc_release(تهی)
{
    اگر (!CONFIG_RTC_TIMER)
        بازگشت;
    صحیح شمار = GET_کم(RTCusers);
    SET_کم(RTCusers, شمار-1);
    اگر (شمار != 1)
        بازگشت;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
