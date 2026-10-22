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
rtc_read(u8 सूचकांक)
{
    सूचकांक |= NMI_DISABLE_BIT;
    outb(सूचकांक, पत्तनम्_CMOS_सूचकांक);
    प्रत्यागम inb(पत्तनम्_CMOS_दत्त);
}

शून्य
rtc_write(u8 सूचकांक, u8 val)
{
    सूचकांक |= NMI_DISABLE_BIT;
    outb(सूचकांक, पत्तनम्_CMOS_सूचकांक);
    outb(val, पत्तनम्_CMOS_दत्त);
}

शून्य
rtc_आवरणम्(u8 सूचकांक, u8 off, u8 on)
{
    सूचकांक |= NMI_DISABLE_BIT;
    outb(सूचकांक, पत्तनम्_CMOS_सूचकांक);
    u8 val = inb(पत्तनम्_CMOS_दत्त);
    outb((val & ~off) | on, पत्तनम्_CMOS_दत्त);
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

    यदि ((rtc_read(CMOS_स्थितिः_A) & RTC_A_UIP) == 0)
        प्रत्यागम 0;
    u32 अन्तः = timer_calc(15);
    प्रति (;;) {
        यदि ((rtc_read(CMOS_स्थितिः_A) & RTC_A_UIP) == 0)
            प्रत्यागम 0;
        यदि (timer_check(अन्तः))
            // update-in-progress never transitioned to 0
            प्रत्यागम -1;
        yield();
    }
}

शून्य
rtc_सज्जता(शून्य)
{
    यदि (!CONFIG_RTC_TIMER)
        प्रत्यागम;
    rtc_write(CMOS_स्थितिः_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_आवरणम्(CMOS_स्थितिः_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_स्थितिः_C);
    rtc_read(CMOS_स्थितिः_D);
}

पूर्णाङ्क RTCusers VARLOW;

शून्य
rtc_use(शून्य)
{
    यदि (!CONFIG_RTC_TIMER)
        प्रत्यागम;
    पूर्णाङ्क गणनम् = GET_नीचः(RTCusers);
    समुच्चयः_नीचः(RTCusers, गणनम्+1);
    यदि (गणनम्)
        प्रत्यागम;
    // Turn on the Periodic Interrupt timer
    rtc_आवरणम्(CMOS_स्थितिः_B, 0, RTC_B_PIE);
}

शून्य
rtc_release(शून्य)
{
    यदि (!CONFIG_RTC_TIMER)
        प्रत्यागम;
    पूर्णाङ्क गणनम् = GET_नीचः(RTCusers);
    समुच्चयः_नीचः(RTCusers, गणनम्-1);
    यदि (गणनम् != 1)
        प्रत्यागम;
    // Clear the Periodic Interrupt.
    rtc_आवरणम्(CMOS_स्थितिः_B, RTC_B_PIE, 0);
}
