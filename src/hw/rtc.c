// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_LOW
#include "rtc.h" // rtc_read
#include "stacks.h" // yield
#include "util.h" // timer_calc
#include "x86.h" // inb

u8
rtc_read(u8 index)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    ត្រឡប់ inb(PORT_CMOS_DATA);
}

void
rtc_write(u8 index, u8 val)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    outb(val, PORT_CMOS_DATA);
}

void
rtc_mask(u8 index, u8 off, u8 on)
{
    index |= NMI_DISABLE_BIT;
    outb(index, PORT_CMOS_INDEX);
    u8 val = inb(PORT_CMOS_DATA);
    outb((val & ~off) | on, PORT_CMOS_DATA);
}

ចំនួនគត់
rtc_updating(void)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    ប្រសិនបើ ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
        ត្រឡប់ 0;
    u32 end = timer_calc(15);
    សម្រាប់ (;;) {
        ប្រសិនបើ ((rtc_read(CMOS_STATUS_A) & RTC_A_UIP) == 0)
            ត្រឡប់ 0;
        ប្រសិនបើ (timer_check(end))
            // update-in-progress never transitioned to 0
            ត្រឡប់ -1;
        yield();
    }
}

void
rtc_setup(void)
{
    ប្រសិនបើ (!CONFIG_RTC_TIMER)
        ត្រឡប់;
    rtc_write(CMOS_STATUS_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_mask(CMOS_STATUS_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_STATUS_C);
    rtc_read(CMOS_STATUS_D);
}

ចំនួនគត់ RTCusers VARLOW;

void
rtc_use(void)
{
    ប្រសិនបើ (!CONFIG_RTC_TIMER)
        ត្រឡប់;
    ចំនួនគត់ count = GET_LOW(RTCusers);
    SET_LOW(RTCusers, count+1);
    ប្រសិនបើ (count)
        ត្រឡប់;
    // Turn on the Periodic Interrupt timer
    rtc_mask(CMOS_STATUS_B, 0, RTC_B_PIE);
}

void
rtc_release(void)
{
    ប្រសិនបើ (!CONFIG_RTC_TIMER)
        ត្រឡប់;
    ចំនួនគត់ count = GET_LOW(RTCusers);
    SET_LOW(RTCusers, count-1);
    ប្រសិនបើ (count != 1)
        ត្រឡប់;
    // Clear the Periodic Interrupt.
    rtc_mask(CMOS_STATUS_B, RTC_B_PIE, 0);
}
