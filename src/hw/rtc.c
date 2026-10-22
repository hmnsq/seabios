// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_LOW
#অন্তর্ভাব "rtc.h" // rtc_read
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "util.h" // timer_calc
#অন্তর্ভাব "x86.h" // inb

u8
rtc_read(u8 সূচক)
{
    সূচক |= NMI_DISABLE_BIT;
    outb(সূচক, PORT_CMOS_সূচক);
    ফেরত inb(PORT_CMOS_তথ্য);
}

শূন্য
rtc_write(u8 সূচক, u8 val)
{
    সূচক |= NMI_DISABLE_BIT;
    outb(সূচক, PORT_CMOS_সূচক);
    outb(val, PORT_CMOS_তথ্য);
}

শূন্য
rtc_mask(u8 সূচক, u8 off, u8 on)
{
    সূচক |= NMI_DISABLE_BIT;
    outb(সূচক, PORT_CMOS_সূচক);
    u8 val = inb(PORT_CMOS_তথ্য);
    outb((val & ~off) | on, PORT_CMOS_তথ্য);
}

পূর্ণসংখ্যা
rtc_updating(শূন্য)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    যদি ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        ফেরত 0;
    u32 end = timer_calc(15);
    জন্য (;;) {
        যদি ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            ফেরত 0;
        যদি (timer_check(end))
            // update-in-progress never transitioned to 0
            ফেরত -1;
        yield();
    }
}

শূন্য
rtc_setup(শূন্য)
{
    যদি (!CONFIG_RTC_TIMER)
        ফেরত;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

পূর্ণসংখ্যা RTCusers VARLOW;

শূন্য
rtc_use(শূন্য)
{
    যদি (!CONFIG_RTC_TIMER)
        ফেরত;
    পূর্ণসংখ্যা গুনতি = GET_নিচের(RTCusers);
    SET_নিচের(RTCusers, গুনতি+1);
    যদি (গুনতি)
        ফেরত;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

শূন্য
rtc_release(শূন্য)
{
    যদি (!CONFIG_RTC_TIMER)
        ফেরত;
    পূর্ণসংখ্যা গুনতি = GET_নিচের(RTCusers);
    SET_নিচের(RTCusers, গুনতি-1);
    যদি (গুনতি != 1)
        ফেরত;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
