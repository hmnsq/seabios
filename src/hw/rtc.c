// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_LOW
#ਅਨ੍ਤਰ੍ਭਾਵ "rtc.h" // rtc_read
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // inb

u8
rtc_read(u8 ਸੂਚਕਾਂਕ)
{
    ਸੂਚਕਾਂਕ |= NMI_DISABLE_ਦੋਆਧਾਰੀ_ਅੰਕ;
    outb(ਸੂਚਕਾਂਕ, ਪੋਰਟ_CMOS_ਸੂਚਕਾਂਕ);
    ਵਾਪਸ inb(ਪੋਰਟ_CMOS_ਅੰਕੜੇ);
}

ਖਾਲੀ
rtc_write(u8 ਸੂਚਕਾਂਕ, u8 val)
{
    ਸੂਚਕਾਂਕ |= NMI_DISABLE_ਦੋਆਧਾਰੀ_ਅੰਕ;
    outb(ਸੂਚਕਾਂਕ, ਪੋਰਟ_CMOS_ਸੂਚਕਾਂਕ);
    outb(val, ਪੋਰਟ_CMOS_ਅੰਕੜੇ);
}

ਖਾਲੀ
rtc_ਨਕਾਬ(u8 ਸੂਚਕਾਂਕ, u8 off, u8 on)
{
    ਸੂਚਕਾਂਕ |= NMI_DISABLE_ਦੋਆਧਾਰੀ_ਅੰਕ;
    outb(ਸੂਚਕਾਂਕ, ਪੋਰਟ_CMOS_ਸੂਚਕਾਂਕ);
    u8 val = inb(ਪੋਰਟ_CMOS_ਅੰਕੜੇ);
    outb((val & ~off) | on, ਪੋਰਟ_CMOS_ਅੰਕੜੇ);
}

ਪੂਰਨ
rtc_updating(ਖਾਲੀ)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    ਜੇ ((rtc_read(CMOS_ਸਥਿਤੀ_A) & RTC_A_UIP) == 0)
        ਵਾਪਸ 0;
    u32 ਸਿਰਾ = timer_calc(15);
    ਲਈ (;;) {
        ਜੇ ((rtc_read(CMOS_ਸਥਿਤੀ_A) & RTC_A_UIP) == 0)
            ਵਾਪਸ 0;
        ਜੇ (timer_check(ਸਿਰਾ))
            // update-in-progress never transitioned to 0
            ਵਾਪਸ -1;
        yield();
    }
}

ਖਾਲੀ
rtc_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_RTC_TIMER)
        ਵਾਪਸ;
    rtc_write(CMOS_ਸਥਿਤੀ_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_ਨਕਾਬ(CMOS_ਸਥਿਤੀ_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_ਸਥਿਤੀ_C);
    rtc_read(CMOS_ਸਥਿਤੀ_D);
}

ਪੂਰਨ RTCusers VARLOW;

ਖਾਲੀ
rtc_use(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_RTC_TIMER)
        ਵਾਪਸ;
    ਪੂਰਨ ਗਿਣਤੀ_2 = GET_ਨੀਵਾਂ(RTCusers);
    ਸੈੱਟ_ਨੀਵਾਂ(RTCusers, ਗਿਣਤੀ_2+1);
    ਜੇ (ਗਿਣਤੀ_2)
        ਵਾਪਸ;
    // Turn on the Periodic Interrupt timer
    rtc_ਨਕਾਬ(CMOS_ਸਥਿਤੀ_B, 0, RTC_B_PIE);
}

ਖਾਲੀ
rtc_release(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_RTC_TIMER)
        ਵਾਪਸ;
    ਪੂਰਨ ਗਿਣਤੀ_2 = GET_ਨੀਵਾਂ(RTCusers);
    ਸੈੱਟ_ਨੀਵਾਂ(RTCusers, ਗਿਣਤੀ_2-1);
    ਜੇ (ਗਿਣਤੀ_2 != 1)
        ਵਾਪਸ;
    // Clear the Periodic Interrupt.
    rtc_ਨਕਾਬ(CMOS_ਸਥਿਤੀ_B, RTC_B_PIE, 0);
}
