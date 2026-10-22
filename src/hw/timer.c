// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOW
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "util.h" // timer_setup
#ಅನ್ತರ್ಭಾವ "x86.h" // cpuid

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIT_COUNTER0      0x0040
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIT_COUNTER1      0x0041
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIT_COUNTER2      0x0042
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PIT_ಬಹುಲಕ          0x0043
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#ವ್ಯಾಖ್ಯಾನಿಸು PM_SEL_TIMER0   (0<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_SEL_TIMER1   (1<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_SEL_TIMER2   (2<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_SEL_READBACK (3<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_ACCESS_LATCH  (0<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_ACCESS_LOBYTE (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_ACCESS_HIBYTE (2<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_ACCESS_ಶಬ್ದ   (3<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_MODE0 (0<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_MODE1 (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_MODE2 (2<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_MODE3 (3<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_MODE4 (4<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_MODE5 (5<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_CNT_ದ್ವಿಮಾನ (0<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_CNT_BCD    (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_READ_COUNTER0 (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_READ_COUNTER1 (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_READ_COUNTER2 (1<<3)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_READ_STATUSVALUE (0<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_READ_ಮೌಲ್ಯ       (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು PM_READ_ಸ್ಥಿತಿ      (2<<4)

// Bits for PORT_PS2_CTRLB
#ವ್ಯಾಖ್ಯಾನಿಸು PPCB_T2GATE (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು PPCB_SPKR   (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PPCB_T2_ಹೊರಹೋಗು  (1<<5)

#ವ್ಯಾಖ್ಯಾನಿಸು PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#ವ್ಯಾಖ್ಯಾನಿಸು PMTIMER_ಗಮ್ಯ_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_ಸುತ್ತು_ಮೇಲೆ(PMTIMER_HZ, 1000 * PMTIMER_ಗಮ್ಯ_PIT);
u16 Timer_ಪೋರ್ಟ್ VARFSEG = ಪೋರ್ಟ್_PIT_COUNTER0;
u8 ಸರಿಸು_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು CALIBRATE_ಎಣಿಕೆ 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
ಸ್ಥಿರ ಶೂನ್ಯ
tsctimer_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    // Setup "timer2"
    u8 orig = inb(ಪೋರ್ಟ್_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, ಪೋರ್ಟ್_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_ಶಬ್ದ|PM_MODE0|PM_CNT_ದ್ವಿಮಾನ, ಪೋರ್ಟ್_PIT_ಬಹುಲಕ);
    /* LSB of ticks */
    outb(CALIBRATE_ಎಣಿಕೆ & 0xFF, ಪೋರ್ಟ್_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_ಎಣಿಕೆ >> 8, ಪೋರ್ಟ್_PIT_COUNTER2);

    u64 ಆರಂಭ = rdtscll();
    ಆಗ ((inb(ಪೋರ್ಟ್_PS2_CTRLB) & PPCB_T2_ಹೊರಹೋಗು) == 0)
        ;
    u64 ಕೊನೆ = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, ಪೋರ್ಟ್_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = ಕೊನೆ - ಆರಂಭ;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)ಆರಂಭ, (u32)ಕೊನೆ, (u32)diff);
    u64 t = DIV_ಸುತ್ತು_ಮೇಲೆ(diff * PMTIMER_HZ, CALIBRATE_ಎಣಿಕೆ);
    ಆಗ (t >= (1<<24)) {
        ಸರಿಸು_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_ಸುತ್ತು_ಮೇಲೆ((u32)t, 1000 * PMTIMER_ಗಮ್ಯ_PIT);
    Timer_ಪೋರ್ಟ್ = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << ಸರಿಸು_TSC) / 1000);
}

// Setup internal timers.
ಶೂನ್ಯ
timer_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TSC_TIMER)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (Timer_ಪೋರ್ಟ್ != ಪೋರ್ಟ್_PIT_COUNTER0)
        ಹಿಂತಿರುಗಿ; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    ಆದರೆ (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ಆದರೆ (cpuid_features & CPUID_TSC)
        tsctimer_ಸಿದ್ಧತೆ();
}

ಶೂನ್ಯ
tsctimer_setfreq(u32 khz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src)
{
    ಆದರೆ (!CONFIG_TSC_TIMER)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (Timer_ಪೋರ್ಟ್ != ಪೋರ್ಟ್_PIT_COUNTER0)
        ಹಿಂತಿರುಗಿ; // have timer already

    TimerKHz = khz;
    ಸರಿಸು_TSC = 0;
    ಆಗ (TimerKHz >= 6000) {
        ಸರಿಸು_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_ಪೋರ್ಟ್ = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << ಸರಿಸು_TSC) / 1000, src);
}

ಶೂನ್ಯ
pmtimer_ಸಿದ್ಧತೆ(u16 ioport)
{
    ಆದರೆ (!CONFIG_PMTIMER)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (Timer_ಪೋರ್ಟ್ != ಪೋರ್ಟ್_PIT_COUNTER0)
        ಹಿಂತಿರುಗಿ; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_ಪೋರ್ಟ್ = ioport;
    TimerKHz = DIV_ಸುತ್ತು_ಮೇಲೆ(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_ಕೊನೆಯ VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
ಸ್ಥಿರ u32
timer_adjust_ಬಿಟ್ಟುಗಳು(u32 ಮೌಲ್ಯ, u32 validbits)
{
    u32 ಕೊನೆಯ = GET_ಕೆಳಗಿನ(Timer_ಕೊನೆಯ);
    ಮೌಲ್ಯ = (ಕೊನೆಯ & ~validbits) | (ಮೌಲ್ಯ & validbits);
    ಆದರೆ (ಮೌಲ್ಯ < ಕೊನೆಯ)
        ಮೌಲ್ಯ += validbits + 1;
    ಗಣ_ಕೆಳಗಿನ(Timer_ಕೊನೆಯ, ಮೌಲ್ಯ);
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

// Sample the current timer value.
ಸ್ಥಿರ u32
timer_read(ಶೂನ್ಯ)
{
    u16 ಪೋರ್ಟ್ = GET_GLOBAL(Timer_ಪೋರ್ಟ್);
    ಆದರೆ (CONFIG_TSC_TIMER && !ಪೋರ್ಟ್)
        // Read from CPU TSC
        ಹಿಂತಿರುಗಿ rdtscll() >> GET_GLOBAL(ಸರಿಸು_TSC);
    ಆದರೆ (CONFIG_PMTIMER && ಪೋರ್ಟ್ != ಪೋರ್ಟ್_PIT_COUNTER0)
        // Read from PMTIMER
        ಹಿಂತಿರುಗಿ timer_adjust_ಬಿಟ್ಟುಗಳು(inl(ಪೋರ್ಟ್), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_ಮೌಲ್ಯ | PM_READ_COUNTER0, ಪೋರ್ಟ್_PIT_ಬಹುಲಕ);
    u16 v = inb(ಪೋರ್ಟ್_PIT_COUNTER0) | (inb(ಪೋರ್ಟ್_PIT_COUNTER0) << 8);
    ಹಿಂತಿರುಗಿ timer_adjust_ಬಿಟ್ಟುಗಳು(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    ಹಿಂತಿರುಗಿ timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    ಆದರೆ (usecs > 500000)
        ಹಿಂತಿರುಗಿ cur + DIV_ಸುತ್ತು_ಮೇಲೆ(usecs, 1000) * khz;
    ಹಿಂತಿರುಗಿ cur + DIV_ಸುತ್ತು_ಮೇಲೆ(usecs * khz, 1000);
}
ಸ್ಥಿರ u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    ಆದರೆ (nsecs > 500000)
        ಹಿಂತಿರುಗಿ cur + DIV_ಸುತ್ತು_ಮೇಲೆ(nsecs, 1000000) * khz;
    ಹಿಂತಿರುಗಿ cur + DIV_ಸುತ್ತು_ಮೇಲೆ(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
ಪೂರ್ಣಾಂಕ
timer_check(u32 ಕೊನೆ)
{
    ಹಿಂತಿರುಗಿ (s32)(timer_read() - ಕೊನೆ) > 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
timer_delay(u32 ಕೊನೆ)
{
    ಆಗ (!timer_check(ಕೊನೆ))
        cpu_relax();
}

ಸ್ಥಿರ ಶೂನ್ಯ
timer_sleep(u32 ಕೊನೆ)
{
    ಆಗ (!timer_check(ಕೊನೆ))
        yield();
}

ಶೂನ್ಯ ndelay(u32 ಎಣಿಕೆ_2) {
    timer_delay(timer_calc_nsec(ಎಣಿಕೆ_2));
}
ಶೂನ್ಯ udelay(u32 ಎಣಿಕೆ_2) {
    timer_delay(timer_calc_usec(ಎಣಿಕೆ_2));
}
ಶೂನ್ಯ mdelay(u32 ಎಣಿಕೆ_2) {
    timer_delay(timer_calc(ಎಣಿಕೆ_2));
}

ಶೂನ್ಯ nsleep(u32 ಎಣಿಕೆ_2) {
    timer_sleep(timer_calc_nsec(ಎಣಿಕೆ_2));
}
ಶೂನ್ಯ usleep(u32 ಎಣಿಕೆ_2) {
    timer_sleep(timer_calc_usec(ಎಣಿಕೆ_2));
}
ಶೂನ್ಯ msleep(u32 ಎಣಿಕೆ_2) {
    timer_sleep(timer_calc(ಎಣಿಕೆ_2));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_ಗಮ್ಯ_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_ಗಮ್ಯ_PIT * ticks;
    ಹಿಂತಿರುಗಿ DIV_ಸುತ್ತು_ಮೇಲೆ(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_ಸುತ್ತು_ಮೇಲೆ((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    ಹಿಂತಿರುಗಿ DIV_ಸುತ್ತು_ಮೇಲೆ(t, 1000 * PMTIMER_ಗಮ್ಯ_PIT);
}

ಶೂನ್ಯ
pit_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_ಶಬ್ದ|PM_MODE2|PM_CNT_ದ್ವಿಮಾನ, ಪೋರ್ಟ್_PIT_ಬಹುಲಕ);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, ಪೋರ್ಟ್_PIT_COUNTER0);
    outb(0x0, ಪೋರ್ಟ್_PIT_COUNTER0);
}
