// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_LOW
#அந்தர்பாவ "rtc.h" // rtc_read
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "util.h" // timer_calc
#அந்தர்பாவ "x86.h" // inb

u8
rtc_read(u8 index)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    திரும்பு inb(PORT_CMOS_DATA);
}

வெறுமை
rtc_write(u8 index, u8 val)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    outb(val, PORT_CMOS_DATA);
}

வெறுமை
rtc_mask(u8 index, u8 off, u8 on)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    u8 val = inb(PORT_CMOS_DATA);
    outb((val & ~off) | on, PORT_CMOS_DATA);
}

முழு
rtc_updating(வெறுமை)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    எனில் ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        திரும்பு 0;
    u32 end = timer_calc(15);
    க்கு (;;) {
        எனில் ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            திரும்பு 0;
        எனில் (timer_check(end))
            // update-in-progress never transitioned to 0
            திரும்பு -1;
        yield();
    }
}

வெறுமை
rtc_setup(வெறுமை)
{
    எனில் (!CONFIG_RTC_TIMER)
        திரும்பு;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

முழு RTCusers VARLOW;

வெறுமை
rtc_use(வெறுமை)
{
    எனில் (!CONFIG_RTC_TIMER)
        திரும்பு;
    முழு மொத்தம் = GET_கீழ்(RTCusers);
    SET_கீழ்(RTCusers, மொத்தம்+1);
    எனில் (மொத்தம்)
        திரும்பு;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

வெறுமை
rtc_release(வெறுமை)
{
    எனில் (!CONFIG_RTC_TIMER)
        திரும்பு;
    முழு மொத்தம் = GET_கீழ்(RTCusers);
    SET_கீழ்(RTCusers, மொத்தம்-1);
    எனில் (மொத்தம் != 1)
        திரும்பு;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
