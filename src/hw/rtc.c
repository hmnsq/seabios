// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_LOW
#అన్తర్భావ "rtc.h" // rtc_read
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "util.h" // timer_calc
#అన్తర్భావ "x86.h" // inb

u8
rtc_read(u8 సూచీ)
{
    సూచీ |= NMI_DISABLE_బిట్టు;
    outb(సూచీ, పోర్టు_CMOS_సూచీ);
    తిరిగి inb(పోర్టు_CMOS_దత్తాంశం);
}

శూన్య
rtc_write(u8 సూచీ, u8 val)
{
    సూచీ |= NMI_DISABLE_బిట్టు;
    outb(సూచీ, పోర్టు_CMOS_సూచీ);
    outb(val, పోర్టు_CMOS_దత్తాంశం);
}

శూన్య
rtc_ముసుగు(u8 సూచీ, u8 off, u8 on)
{
    సూచీ |= NMI_DISABLE_బిట్టు;
    outb(సూచీ, పోర్టు_CMOS_సూచీ);
    u8 val = inb(పోర్టు_CMOS_దత్తాంశం);
    outb((val & ~off) | on, పోర్టు_CMOS_దత్తాంశం);
}

పూర్ణాంకం
rtc_updating(శూన్య)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    ఒకవేళ ((rtc_read(CMOS_స్థితి_A) & RTC_A_UIP) == 0)
        తిరిగి 0;
    u32 చివర = timer_calc(15);
    కొరకు (;;) {
        ఒకవేళ ((rtc_read(CMOS_స్థితి_A) & RTC_A_UIP) == 0)
            తిరిగి 0;
        ఒకవేళ (timer_check(చివర))
            // update-in-progress never transitioned to 0
            తిరిగి -1;
        yield();
    }
}

శూన్య
rtc_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_RTC_TIMER)
        తిరిగి;
    rtc_write(CMOS_స్థితి_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_ముసుగు(CMOS_స్థితి_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_స్థితి_C);
    rtc_read(CMOS_స్థితి_D);
}

పూర్ణాంకం RTCusers VARLOW;

శూన్య
rtc_use(శూన్య)
{
    ఒకవేళ (!CONFIG_RTC_TIMER)
        తిరిగి;
    పూర్ణాంకం లెక్క = GET_దిగువ(RTCusers);
    సమితి_దిగువ(RTCusers, లెక్క+1);
    ఒకవేళ (లెక్క)
        తిరిగి;
    // Turn on the Periodic Interrupt timer
    rtc_ముసుగు(CMOS_స్థితి_B, 0, RTC_B_PIE);
}

శూన్య
rtc_release(శూన్య)
{
    ఒకవేళ (!CONFIG_RTC_TIMER)
        తిరిగి;
    పూర్ణాంకం లెక్క = GET_దిగువ(RTCusers);
    సమితి_దిగువ(RTCusers, లెక్క-1);
    ఒకవేళ (లెక్క != 1)
        తిరిగి;
    // Clear the Periodic Interrupt.
    rtc_ముసుగు(CMOS_స్థితి_B, RTC_B_PIE, 0);
}
