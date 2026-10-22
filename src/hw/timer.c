// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_LOW
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "util.h" // timer_setup
#अन्तर्भाव "x86.h" // cpuid

#define PORT_PIT_COUNTER0      0x0040
#define PORT_PIT_COUNTER1      0x0041
#define PORT_PIT_COUNTER2      0x0042
#define PORT_PIT_MODE          0x0043
#define PORT_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#define PM_SEL_TIMER0   (0<<6)
#define PM_SEL_TIMER1   (1<<6)
#define PM_SEL_TIMER2   (2<<6)
#define PM_SEL_READBACK (3<<6)
#define PM_ACCESS_LATCH  (0<<4)
#define PM_ACCESS_LOBYTE (1<<4)
#define PM_ACCESS_HIBYTE (2<<4)
#define PM_ACCESS_WORD   (3<<4)
#define PM_MODE0 (0<<1)
#define PM_MODE1 (1<<1)
#define PM_MODE2 (2<<1)
#define PM_MODE3 (3<<1)
#define PM_MODE4 (4<<1)
#define PM_MODE5 (5<<1)
#define PM_CNT_द्विमान (0<<0)
#define PM_CNT_BCD    (1<<0)
#define PM_READ_COUNTER0 (1<<1)
#define PM_READ_COUNTER1 (1<<2)
#define PM_READ_COUNTER2 (1<<3)
#define PM_READ_STATUSVALUE (0<<4)
#define PM_READ_मानम्       (1<<4)
#define PM_READ_STATUS      (2<<4)

// Bits for PORT_PS2_CTRLB
#define PPCB_T2GATE (1<<0)
#define PPCB_SPKR   (1<<1)
#define PPCB_T2OUT  (1<<5)

#define PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#define PMTIMER_TO_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_ROUND_UP(PMTIMER_HZ, 1000 * PMTIMER_TO_PIT);
u16 TimerPort VARFSEG = PORT_PIT_COUNTER0;
u8 ShiftTSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#define CALIBRATE_गणनम् 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
स्थिर शून्य
tsctimer_setup(शून्य)
{
    // Setup "timer2"
    u8 orig = inb(PORT_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, PORT_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_WORD|PM_MODE0|PM_CNT_द्विमान, PORT_PIT_MODE);
    /* LSB of ticks */
    outb(CALIBRATE_गणनम् & 0xFF, PORT_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_गणनम् >> 8, PORT_PIT_COUNTER2);

    u64 start = rdtscll();
    जाव ((inb(PORT_PS2_CTRLB) & PPCB_T2OUT) == 0)
        ;
    u64 end = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, PORT_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = end - start;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)start, (u32)end, (u32)diff);
    u64 t = DIV_ROUND_UP(diff * PMTIMER_HZ, CALIBRATE_गणनम्);
    जाव (t >= (1<<24)) {
        ShiftTSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_ROUND_UP((u32)t, 1000 * PMTIMER_TO_PIT);
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << ShiftTSC) / 1000);
}

// Setup internal timers.
शून्य
timer_setup(शून्य)
{
    जइ (!CONFIG_TSC_TIMER)
        प्रत्यागम;
    जइ (TimerPort != PORT_PIT_COUNTER0)
        प्रत्यागम; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    जइ (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    जइ (cpuid_features & CPUID_TSC)
        tsctimer_setup();
}

शून्य
tsctimer_setfreq(u32 khz, ध्रुव वर्ण *src)
{
    जइ (!CONFIG_TSC_TIMER)
        प्रत्यागम;
    जइ (TimerPort != PORT_PIT_COUNTER0)
        प्रत्यागम; // have timer already

    TimerKHz = khz;
    ShiftTSC = 0;
    जाव (TimerKHz >= 6000) {
        ShiftTSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << ShiftTSC) / 1000, src);
}

शून्य
pmtimer_setup(u16 ioport)
{
    जइ (!CONFIG_PMTIMER)
        प्रत्यागम;
    जइ (TimerPort != PORT_PIT_COUNTER0)
        प्रत्यागम; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    TimerPort = ioport;
    TimerKHz = DIV_ROUND_UP(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 TimerLast VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
स्थिर u32
timer_adjust_bits(u32 मानम्, u32 validbits)
{
    u32 last = GET_नीचः(TimerLast);
    मानम् = (last & ~validbits) | (मानम् & validbits);
    जइ (मानम् < last)
        मानम् += validbits + 1;
    SET_नीचः(TimerLast, मानम्);
    प्रत्यागम मानम्;
}

// Sample the current timer value.
स्थिर u32
timer_read(शून्य)
{
    u16 port = GET_GLOBAL(TimerPort);
    जइ (CONFIG_TSC_TIMER && !port)
        // Read from CPU TSC
        प्रत्यागम rdtscll() >> GET_GLOBAL(ShiftTSC);
    जइ (CONFIG_PMTIMER && port != PORT_PIT_COUNTER0)
        // Read from PMTIMER
        प्रत्यागम timer_adjust_bits(inl(port), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_मानम् | PM_READ_COUNTER0, PORT_PIT_MODE);
    u16 v = inb(PORT_PIT_COUNTER0) | (inb(PORT_PIT_COUNTER0) << 8);
    प्रत्यागम timer_adjust_bits(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    प्रत्यागम timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    जइ (usecs > 500000)
        प्रत्यागम cur + DIV_ROUND_UP(usecs, 1000) * khz;
    प्रत्यागम cur + DIV_ROUND_UP(usecs * khz, 1000);
}
स्थिर u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    जइ (nsecs > 500000)
        प्रत्यागम cur + DIV_ROUND_UP(nsecs, 1000000) * khz;
    प्रत्यागम cur + DIV_ROUND_UP(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
पूर्णाङ्क
timer_check(u32 end)
{
    प्रत्यागम (s32)(timer_read() - end) > 0;
}

स्थिर शून्य
timer_delay(u32 end)
{
    जाव (!timer_check(end))
        cpu_relax();
}

स्थिर शून्य
timer_sleep(u32 end)
{
    जाव (!timer_check(end))
        yield();
}

शून्य ndelay(u32 गणनम्) {
    timer_delay(timer_calc_nsec(गणनम्));
}
शून्य udelay(u32 गणनम्) {
    timer_delay(timer_calc_usec(गणनम्));
}
शून्य mdelay(u32 गणनम्) {
    timer_delay(timer_calc(गणनम्));
}

शून्य nsleep(u32 गणनम्) {
    timer_sleep(timer_calc_nsec(गणनम्));
}
शून्य usleep(u32 गणनम्) {
    timer_sleep(timer_calc_usec(गणनम्));
}
शून्य msleep(u32 गणनम्) {
    timer_sleep(timer_calc(गणनम्));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#define PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_to_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_TO_PIT * ticks;
    प्रत्यागम DIV_ROUND_UP(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_ROUND_UP((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    प्रत्यागम DIV_ROUND_UP(t, 1000 * PMTIMER_TO_PIT);
}

शून्य
pit_setup(शून्य)
{
    जइ (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_WORD|PM_MODE2|PM_CNT_द्विमान, PORT_PIT_MODE);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, PORT_PIT_COUNTER0);
    outb(0x0, PORT_PIT_COUNTER0);
}
