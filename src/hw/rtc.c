// Support for MC146818 Real Time Clock chip.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOW
#ಅನ್ತರ್ಭಾವ "rtc.h" // rtc_read
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc
#ಅನ್ತರ್ಭಾವ "x86.h" // inb

u8
rtc_read(u8 ಸೂಚ್ಯಂಕ)
{
    ಸೂಚ್ಯಂಕ |= NMI_DISABLE_ಬಿಟ್;
    outb(ಸೂಚ್ಯಂಕ, ಪೋರ್ಟ್_CMOS_ಸೂಚ್ಯಂಕ);
    ಹಿಂತಿರುಗಿ inb(ಪೋರ್ಟ್_CMOS_ದತ್ತಾಂಶ);
}

ಶೂನ್ಯ
rtc_write(u8 ಸೂಚ್ಯಂಕ, u8 val)
{
    ಸೂಚ್ಯಂಕ |= NMI_DISABLE_ಬಿಟ್;
    outb(ಸೂಚ್ಯಂಕ, ಪೋರ್ಟ್_CMOS_ಸೂಚ್ಯಂಕ);
    outb(val, ಪೋರ್ಟ್_CMOS_ದತ್ತಾಂಶ);
}

ಶೂನ್ಯ
rtc_ಮುಖವಾಡ(u8 ಸೂಚ್ಯಂಕ, u8 off, u8 on)
{
    ಸೂಚ್ಯಂಕ |= NMI_DISABLE_ಬಿಟ್;
    outb(ಸೂಚ್ಯಂಕ, ಪೋರ್ಟ್_CMOS_ಸೂಚ್ಯಂಕ);
    u8 val = inb(ಪೋರ್ಟ್_CMOS_ದತ್ತಾಂಶ);
    outb((val & ~off) | on, ಪೋರ್ಟ್_CMOS_ದತ್ತಾಂಶ);
}

ಪೂರ್ಣಾಂಕ
rtc_updating(ಶೂನ್ಯ)
{
    // This function checks to see if the update-in-progress bit
    // is set in CMOS Status Register A.  If not, it returns 0.
    // If it is set, it tries to wait until there is a transition
    // to 0, and will return 0 if such a transition occurs.  A -1
    // is returned only after timing out.  The maximum period
    // that this bit should be set is constrained to (1984+244)
    // useconds, but we wait for longer just to be sure.

    ಆದರೆ ((rtc_read(CMOS_ಸ್ಥಿತಿ_A) & RTC_A_UIP) == 0)
        ಹಿಂತಿರುಗಿ 0;
    u32 ಕೊನೆ = timer_calc(15);
    ಗಾಗಿ (;;) {
        ಆದರೆ ((rtc_read(CMOS_ಸ್ಥಿತಿ_A) & RTC_A_UIP) == 0)
            ಹಿಂತಿರುಗಿ 0;
        ಆದರೆ (timer_check(ಕೊನೆ))
            // update-in-progress never transitioned to 0
            ಹಿಂತಿರುಗಿ -1;
        yield();
    }
}

ಶೂನ್ಯ
rtc_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_RTC_TIMER)
        ಹಿಂತಿರುಗಿ;
    rtc_write(CMOS_ಸ್ಥಿತಿ_A, 0x26);    // 32,768Khz src, 976.5625us updates
    rtc_ಮುಖವಾಡ(CMOS_ಸ್ಥಿತಿ_B, ~RTC_B_DSE, RTC_B_24HR);
    rtc_read(CMOS_ಸ್ಥಿತಿ_C);
    rtc_read(CMOS_ಸ್ಥಿತಿ_D);
}

ಪೂರ್ಣಾಂಕ RTCusers VARLOW;

ಶೂನ್ಯ
rtc_use(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_RTC_TIMER)
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = GET_ಕೆಳಗಿನ(RTCusers);
    ಗಣ_ಕೆಳಗಿನ(RTCusers, ಎಣಿಕೆ_2+1);
    ಆದರೆ (ಎಣಿಕೆ_2)
        ಹಿಂತಿರುಗಿ;
    // Turn on the Periodic Interrupt timer
    rtc_ಮುಖವಾಡ(CMOS_ಸ್ಥಿತಿ_B, 0, RTC_B_PIE);
}

ಶೂನ್ಯ
rtc_release(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_RTC_TIMER)
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = GET_ಕೆಳಗಿನ(RTCusers);
    ಗಣ_ಕೆಳಗಿನ(RTCusers, ಎಣಿಕೆ_2-1);
    ಆದರೆ (ಎಣಿಕೆ_2 != 1)
        ಹಿಂತಿರುಗಿ;
    // Clear the Periodic Interrupt.
    rtc_ಮುಖವಾಡ(CMOS_ಸ್ಥಿತಿ_B, RTC_B_PIE, 0);
}
