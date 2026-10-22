// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_LOW
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // cpuid

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIT_COUNTER0      0x0040
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIT_COUNTER1      0x0041
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIT_COUNTER2      0x0042
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PIT_ਬਹੁਲਕ          0x0043
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_SEL_TIMER0   (0<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_SEL_TIMER1   (1<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_SEL_TIMER2   (2<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_SEL_READBACK (3<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_ACCESS_LATCH  (0<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_ACCESS_LOBYTE (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_ACCESS_HIBYTE (2<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_ACCESS_ਸ਼ਬਦ   (3<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_MODE0 (0<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_MODE1 (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_MODE2 (2<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_MODE3 (3<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_MODE4 (4<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_MODE5 (5<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_CNT_ਦੁਆਧਾਰੀ (0<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_CNT_BCD    (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_READ_COUNTER0 (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_READ_COUNTER1 (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_READ_COUNTER2 (1<<3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_READ_STATUSVALUE (0<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_READ_ਮੁੱਲ       (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PM_READ_ਸਥਿਤੀ      (2<<4)

// Bits for PORT_PS2_CTRLB
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPCB_T2GATE (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPCB_SPKR   (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPCB_T2_ਬਾਹਰਲਾ  (1<<5)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PMTIMER_ਮੰਜ਼ਿਲ_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_ਗੇੜਾ_ਉੱਪਰ(PMTIMER_HZ, 1000 * PMTIMER_ਮੰਜ਼ਿਲ_PIT);
u16 Timer_ਪੋਰਟ VARFSEG = ਪੋਰਟ_PIT_COUNTER0;
u8 ਖਿਸਕਾਓ_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALIBRATE_ਗਿਣਤੀ 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
ਸਥਿਰ ਖਾਲੀ
tsctimer_ਤਿਆਰੀ(ਖਾਲੀ)
{
    // Setup "timer2"
    u8 orig = inb(ਪੋਰਟ_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, ਪੋਰਟ_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_ਸ਼ਬਦ|PM_MODE0|PM_CNT_ਦੁਆਧਾਰੀ, ਪੋਰਟ_PIT_ਬਹੁਲਕ);
    /* LSB of ticks */
    outb(CALIBRATE_ਗਿਣਤੀ & 0xFF, ਪੋਰਟ_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_ਗਿਣਤੀ >> 8, ਪੋਰਟ_PIT_COUNTER2);

    u64 ਸ਼ੁਰੂ = rdtscll();
    ਜਦੋਂ ((inb(ਪੋਰਟ_PS2_CTRLB) & PPCB_T2_ਬਾਹਰਲਾ) == 0)
        ;
    u64 ਸਿਰਾ = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, ਪੋਰਟ_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = ਸਿਰਾ - ਸ਼ੁਰੂ;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)ਸ਼ੁਰੂ, (u32)ਸਿਰਾ, (u32)diff);
    u64 t = DIV_ਗੇੜਾ_ਉੱਪਰ(diff * PMTIMER_HZ, CALIBRATE_ਗਿਣਤੀ);
    ਜਦੋਂ (t >= (1<<24)) {
        ਖਿਸਕਾਓ_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_ਗੇੜਾ_ਉੱਪਰ((u32)t, 1000 * PMTIMER_ਮੰਜ਼ਿਲ_PIT);
    Timer_ਪੋਰਟ = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << ਖਿਸਕਾਓ_TSC) / 1000);
}

// Setup internal timers.
ਖਾਲੀ
timer_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TSC_TIMER)
        ਵਾਪਸ;
    ਜੇ (Timer_ਪੋਰਟ != ਪੋਰਟ_PIT_COUNTER0)
        ਵਾਪਸ; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    ਜੇ (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ਜੇ (cpuid_features & CPUID_TSC)
        tsctimer_ਤਿਆਰੀ();
}

ਖਾਲੀ
tsctimer_setfreq(u32 khz, ਅਚਲ ਅੱਖਰ *src)
{
    ਜੇ (!CONFIG_TSC_TIMER)
        ਵਾਪਸ;
    ਜੇ (Timer_ਪੋਰਟ != ਪੋਰਟ_PIT_COUNTER0)
        ਵਾਪਸ; // have timer already

    TimerKHz = khz;
    ਖਿਸਕਾਓ_TSC = 0;
    ਜਦੋਂ (TimerKHz >= 6000) {
        ਖਿਸਕਾਓ_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_ਪੋਰਟ = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << ਖਿਸਕਾਓ_TSC) / 1000, src);
}

ਖਾਲੀ
pmtimer_ਤਿਆਰੀ(u16 ioport)
{
    ਜੇ (!CONFIG_PMTIMER)
        ਵਾਪਸ;
    ਜੇ (Timer_ਪੋਰਟ != ਪੋਰਟ_PIT_COUNTER0)
        ਵਾਪਸ; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_ਪੋਰਟ = ioport;
    TimerKHz = DIV_ਗੇੜਾ_ਉੱਪਰ(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_ਆਖਰੀ VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
ਸਥਿਰ u32
timer_adjust_ਬਿੱਟ(u32 ਮੁੱਲ, u32 validbits)
{
    u32 ਆਖਰੀ = GET_ਨੀਵਾਂ(Timer_ਆਖਰੀ);
    ਮੁੱਲ = (ਆਖਰੀ & ~validbits) | (ਮੁੱਲ & validbits);
    ਜੇ (ਮੁੱਲ < ਆਖਰੀ)
        ਮੁੱਲ += validbits + 1;
    ਸੈੱਟ_ਨੀਵਾਂ(Timer_ਆਖਰੀ, ਮੁੱਲ);
    ਵਾਪਸ ਮੁੱਲ;
}

// Sample the current timer value.
ਸਥਿਰ u32
timer_read(ਖਾਲੀ)
{
    u16 ਪੋਰਟ = GET_GLOBAL(Timer_ਪੋਰਟ);
    ਜੇ (CONFIG_TSC_TIMER && !ਪੋਰਟ)
        // Read from CPU TSC
        ਵਾਪਸ rdtscll() >> GET_GLOBAL(ਖਿਸਕਾਓ_TSC);
    ਜੇ (CONFIG_PMTIMER && ਪੋਰਟ != ਪੋਰਟ_PIT_COUNTER0)
        // Read from PMTIMER
        ਵਾਪਸ timer_adjust_ਬਿੱਟ(inl(ਪੋਰਟ), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_ਮੁੱਲ | PM_READ_COUNTER0, ਪੋਰਟ_PIT_ਬਹੁਲਕ);
    u16 v = inb(ਪੋਰਟ_PIT_COUNTER0) | (inb(ਪੋਰਟ_PIT_COUNTER0) << 8);
    ਵਾਪਸ timer_adjust_ਬਿੱਟ(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    ਵਾਪਸ timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    ਜੇ (usecs > 500000)
        ਵਾਪਸ cur + DIV_ਗੇੜਾ_ਉੱਪਰ(usecs, 1000) * khz;
    ਵਾਪਸ cur + DIV_ਗੇੜਾ_ਉੱਪਰ(usecs * khz, 1000);
}
ਸਥਿਰ u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    ਜੇ (nsecs > 500000)
        ਵਾਪਸ cur + DIV_ਗੇੜਾ_ਉੱਪਰ(nsecs, 1000000) * khz;
    ਵਾਪਸ cur + DIV_ਗੇੜਾ_ਉੱਪਰ(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
ਪੂਰਨ
timer_check(u32 ਸਿਰਾ)
{
    ਵਾਪਸ (s32)(timer_read() - ਸਿਰਾ) > 0;
}

ਸਥਿਰ ਖਾਲੀ
timer_delay(u32 ਸਿਰਾ)
{
    ਜਦੋਂ (!timer_check(ਸਿਰਾ))
        cpu_relax();
}

ਸਥਿਰ ਖਾਲੀ
timer_sleep(u32 ਸਿਰਾ)
{
    ਜਦੋਂ (!timer_check(ਸਿਰਾ))
        yield();
}

ਖਾਲੀ ndelay(u32 ਗਿਣਤੀ_2) {
    timer_delay(timer_calc_nsec(ਗਿਣਤੀ_2));
}
ਖਾਲੀ udelay(u32 ਗਿਣਤੀ_2) {
    timer_delay(timer_calc_usec(ਗਿਣਤੀ_2));
}
ਖਾਲੀ mdelay(u32 ਗਿਣਤੀ_2) {
    timer_delay(timer_calc(ਗਿਣਤੀ_2));
}

ਖਾਲੀ nsleep(u32 ਗਿਣਤੀ_2) {
    timer_sleep(timer_calc_nsec(ਗਿਣਤੀ_2));
}
ਖਾਲੀ usleep(u32 ਗਿਣਤੀ_2) {
    timer_sleep(timer_calc_usec(ਗਿਣਤੀ_2));
}
ਖਾਲੀ msleep(u32 ਗਿਣਤੀ_2) {
    timer_sleep(timer_calc(ਗਿਣਤੀ_2));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_ਮੰਜ਼ਿਲ_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_ਮੰਜ਼ਿਲ_PIT * ticks;
    ਵਾਪਸ DIV_ਗੇੜਾ_ਉੱਪਰ(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_ਗੇੜਾ_ਉੱਪਰ((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    ਵਾਪਸ DIV_ਗੇੜਾ_ਉੱਪਰ(t, 1000 * PMTIMER_ਮੰਜ਼ਿਲ_PIT);
}

ਖਾਲੀ
pit_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_ਸ਼ਬਦ|PM_MODE2|PM_CNT_ਦੁਆਧਾਰੀ, ਪੋਰਟ_PIT_ਬਹੁਲਕ);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, ਪੋਰਟ_PIT_COUNTER0);
    outb(0x0, ਪੋਰਟ_PIT_COUNTER0);
}
