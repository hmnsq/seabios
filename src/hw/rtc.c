// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_LOW
#अन्तर्भाव "rtc.h" // rtc_read
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "util.h" // timer_calc
#अन्तर्भाव "x86.h" // inb

u8
rtc_read(u8 index)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    पटिनिवत्त inb(PORT_CMOS_DATA);
}

शून्य
rtc_write(u8 index, u8 val)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    outb(val, PORT_CMOS_DATA);
}

शून्य
rtc_mask(u8 index, u8 off, u8 on)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    u8 val = inb(PORT_CMOS_DATA);
    outb((val & ~off) | on, PORT_CMOS_DATA);
}

पूर्णाङ्क
rtc_updating(शून्य)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    सचे ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        पटिनिवत्त 0;
    u32 end = timer_calc(15);
    अत्थाय (;;) {
        सचे ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            पटिनिवत्त 0;
        सचे (timer_check(end))
            // update-in-progress never transitioned to 0
            पटिनिवत्त -1;
        yield();
    }
}

शून्य
rtc_setup(शून्य)
{
    सचे (!CONFIG_RTC_TIMER)
        पटिनिवत्त;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

पूर्णाङ्क RTCusers VARLOW;

शून्य
rtc_use(शून्य)
{
    सचे (!CONFIG_RTC_TIMER)
        पटिनिवत्त;
    पूर्णाङ्क गणनम् = GET_नीचः(RTCusers);
    SET_नीचः(RTCusers, गणनम्+1);
    सचे (गणनम्)
        पटिनिवत्त;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

शून्य
rtc_release(शून्य)
{
    सचे (!CONFIG_RTC_TIMER)
        पटिनिवत्त;
    पूर्णाङ्क गणनम् = GET_नीचः(RTCusers);
    SET_नीचः(RTCusers, गणनम्-1);
    सचे (गणनम् != 1)
        पटिनिवत्त;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
