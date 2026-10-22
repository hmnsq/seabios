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
rtc_read(u8 اشاریہ)
{
    اشاریہ |= NMI_DISABLE_ثنائی_ہندسہ;
    outb(اشاریہ, پورٹ_CMOS_اشاریہ);
    أرجع inb(پورٹ_CMOS_انکڑے);
}

فراغ
rtc_write(u8 اشاریہ, u8 val)
{
    اشاریہ |= NMI_DISABLE_ثنائی_ہندسہ;
    outb(اشاریہ, پورٹ_CMOS_اشاریہ);
    outb(val, پورٹ_CMOS_انکڑے);
}

فراغ
rtc_نقاب(u8 اشاریہ, u8 off, u8 on)
{
    اشاریہ |= NMI_DISABLE_ثنائی_ہندسہ;
    outb(اشاریہ, پورٹ_CMOS_اشاریہ);
    u8 val = inb(پورٹ_CMOS_انکڑے);
    outb((val & ~off) | on, پورٹ_CMOS_انکڑے);
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

    إذا ((rtc_read(CMOS_حالت_A) & RTC_A_UIP) == 0)
        أرجع 0;
    u32 سرا = timer_calc(15);
    لكل (;;) {
        إذا ((rtc_read(CMOS_حالت_A) & RTC_A_UIP) == 0)
            أرجع 0;
        إذا (timer_check(سرا))
            // update-in-progress never transitioned to 0
            أرجع -1;
        yield();
    }
}

فراغ
rtc_تیاری(فراغ)
{
    إذا (!CONFIG_RTC_TIMER)
        أرجع;
    rtc_write(CMOS_حالت_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_نقاب(CMOS_حالت_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_حالت_C);
    rtc_read(CMOS_حالت_D);
}

صحيح RTCusers VARLOW;

فراغ
rtc_use(فراغ)
{
    إذا (!CONFIG_RTC_TIMER)
        أرجع;
    صحيح گنتی = GET_کم(RTCusers);
    سیٹ_کم(RTCusers, گنتی+1);
    إذا (گنتی)
        أرجع;
    // Turn on the Periodic Interrupt timer
    rtc_نقاب(CMOS_حالت_B, 0, RTC_B_PIE);
}

فراغ
rtc_release(فراغ)
{
    إذا (!CONFIG_RTC_TIMER)
        أرجع;
    صحيح گنتی = GET_کم(RTCusers);
    سیٹ_کم(RTCusers, گنتی-1);
    إذا (گنتی != 1)
        أرجع;
    // Clear the Periodic Interrupt.
    rtc_نقاب(CMOS_حالت_B, RTC_B_PIE, 0);
}
