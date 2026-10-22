// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_LOW
#ضمن "rtc.h" // rtc_read
#ضمن "stacks.h" // yield
#ضمن "util.h" // timer_calc
#ضمن "x86.h" // inb

u8
rtc_read(u8 index)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    إرجاع inb(PORT_CMOS_DATA);
}

فراغ
rtc_write(u8 index, u8 val)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    outb(val, PORT_CMOS_DATA);
}

فراغ
rtc_mask(u8 index, u8 off, u8 on)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    u8 val = inb(PORT_CMOS_DATA);
    outb((val & ~off) | on, PORT_CMOS_DATA);
}

صحيح
rtc_updating(فراغ)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    إذا ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        إرجاع 0;
    u32 end = timer_calc(15);
    لكل (;;) {
        إذا ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            إرجاع 0;
        إذا (timer_check(end))
            // update-in-progress never transitioned to 0
            إرجاع -1;
        yield();
    }
}

فراغ
rtc_setup(فراغ)
{
    إذا (!CONFIG_RTC_TIMER)
        إرجاع;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

صحيح RTCusers VARLOW;

فراغ
rtc_use(فراغ)
{
    إذا (!CONFIG_RTC_TIMER)
        إرجاع;
    صحيح تعداد_2 = GET_أدنى(RTCusers);
    SET_أدنى(RTCusers, تعداد_2+1);
    إذا (تعداد_2)
        إرجاع;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

فراغ
rtc_release(فراغ)
{
    إذا (!CONFIG_RTC_TIMER)
        إرجاع;
    صحيح تعداد_2 = GET_أدنى(RTCusers);
    SET_أدنى(RTCusers, تعداد_2-1);
    إذا (تعداد_2 != 1)
        إرجاع;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
