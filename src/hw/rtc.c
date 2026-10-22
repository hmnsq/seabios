// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_LOW
#समावेश "rtc.h" // rtc_read
#समावेश "stacks.h" // yield
#समावेश "util.h" // timer_calc
#समावेश "x86.h" // inb

u8
rtc_read(u8 सूचकांक)
{
    सूचकांक |= NMI_DISABLE_BIT;
    outb(सूचकांक, PORT_CMOS_सूचकांक);
    वापस inb(PORT_CMOS_तथ्य);
}

व्योम
rtc_write(u8 सूचकांक, u8 val)
{
    सूचकांक |= NMI_DISABLE_BIT;
    outb(सूचकांक, PORT_CMOS_सूचकांक);
    outb(val, PORT_CMOS_तथ्य);
}

व्योम
rtc_mask(u8 सूचकांक, u8 off, u8 on)
{
    सूचकांक |= NMI_DISABLE_BIT;
    outb(सूचकांक, PORT_CMOS_सूचकांक);
    u8 val = inb(PORT_CMOS_तथ्य);
    outb((val & ~off) | on, PORT_CMOS_तथ्य);
}

पूर्णांक
rtc_updating(व्योम)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    अगर ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        वापस 0;
    u32 end = timer_calc(15);
    क्रम (;;) {
        अगर ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            वापस 0;
        अगर (timer_check(end))
            // update-in-progress never transitioned to 0
            वापस -1;
        yield();
    }
}

व्योम
rtc_setup(व्योम)
{
    अगर (!CONFIG_RTC_TIMER)
        वापस;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

पूर्णांक RTCusers VARLOW;

व्योम
rtc_use(व्योम)
{
    अगर (!CONFIG_RTC_TIMER)
        वापस;
    पूर्णांक गिनती = GET_निचला(RTCusers);
    SET_निचला(RTCusers, गिनती+1);
    अगर (गिनती)
        वापस;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

व्योम
rtc_release(व्योम)
{
    अगर (!CONFIG_RTC_TIMER)
        वापस;
    पूर्णांक गिनती = GET_निचला(RTCusers);
    SET_निचला(RTCusers, गिनती-1);
    अगर (गिनती != 1)
        वापस;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
