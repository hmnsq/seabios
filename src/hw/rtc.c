// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_LOW
#અન્તર્ભાવ "rtc.h" // rtc_read
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "util.h" // timer_calc
#અન્તર્ભાવ "x86.h" // inb

u8
rtc_read(u8 index)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    પાછા inb(PORT_CMOS_DATA);
}

શૂન્ય
rtc_write(u8 index, u8 val)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    outb(val, PORT_CMOS_DATA);
}

શૂન્ય
rtc_mask(u8 index, u8 off, u8 on)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    u8 val = inb(PORT_CMOS_DATA);
    outb((val & ~off) | on, PORT_CMOS_DATA);
}

પૂર્ણાંક
rtc_updating(શૂન્ય)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    જો ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        પાછા 0;
    u32 end = timer_calc(15);
    માટે (;;) {
        જો ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            પાછા 0;
        જો (timer_check(end))
            // update-in-progress never transitioned to 0
            પાછા -1;
        yield();
    }
}

શૂન્ય
rtc_setup(શૂન્ય)
{
    જો (!CONFIG_RTC_TIMER)
        પાછા;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

પૂર્ણાંક RTCusers VARLOW;

શૂન્ય
rtc_use(શૂન્ય)
{
    જો (!CONFIG_RTC_TIMER)
        પાછા;
    પૂર્ણાંક ગણતરી = GET_નીચલું(RTCusers);
    SET_નીચલું(RTCusers, ગણતરી+1);
    જો (ગણતરી)
        પાછા;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

શૂન્ય
rtc_release(શૂન્ય)
{
    જો (!CONFIG_RTC_TIMER)
        પાછા;
    પૂર્ણાંક ગણતરી = GET_નીચલું(RTCusers);
    SET_નીચલું(RTCusers, ગણતરી-1);
    જો (ગણતરી != 1)
        પાછા;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
