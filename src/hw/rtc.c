// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_LOW
#അന്തര്ഭാവ "rtc.h" // rtc_read
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "util.h" // timer_calc
#അന്തര്ഭാവ "x86.h" // inb

u8
rtc_read(u8 index)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    തിരികെ inb(PORT_CMOS_DATA);
}

ശൂന്യം
rtc_write(u8 index, u8 val)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    outb(val, PORT_CMOS_DATA);
}

ശൂന്യം
rtc_mask(u8 index, u8 off, u8 on)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    u8 val = inb(PORT_CMOS_DATA);
    outb((val & ~off) | on, PORT_CMOS_DATA);
}

പൂർണ്ണസംഖ്യ
rtc_updating(ശൂന്യം)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    എങ്കിൽ ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        തിരികെ 0;
    u32 end = timer_calc(15);
    വേണ്ടി (;;) {
        എങ്കിൽ ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            തിരികെ 0;
        എങ്കിൽ (timer_check(end))
            // update-in-progress never transitioned to 0
            തിരികെ -1;
        yield();
    }
}

ശൂന്യം
rtc_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_RTC_TIMER)
        തിരികെ;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

പൂർണ്ണസംഖ്യ RTCusers VARLOW;

ശൂന്യം
rtc_use(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_RTC_TIMER)
        തിരികെ;
    പൂർണ്ണസംഖ്യ എണ്ണം = GET_താഴെയുള്ള(RTCusers);
    SET_താഴെയുള്ള(RTCusers, എണ്ണം+1);
    എങ്കിൽ (എണ്ണം)
        തിരികെ;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

ശൂന്യം
rtc_release(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_RTC_TIMER)
        തിരികെ;
    പൂർണ്ണസംഖ്യ എണ്ണം = GET_താഴെയുള്ള(RTCusers);
    SET_താഴെയുള്ള(RTCusers, എണ്ണം-1);
    എങ്കിൽ (എണ്ണം != 1)
        തിരികെ;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
