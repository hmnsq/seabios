// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_LOW
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "util.h" // timer_setup
#અન્તર્ભાવ "x86.h" // cpuid

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
#define PM_CNT_દ્વિઅંકી (0<<0)
#define PM_CNT_BCD    (1<<0)
#define PM_READ_COUNTER0 (1<<1)
#define PM_READ_COUNTER1 (1<<2)
#define PM_READ_COUNTER2 (1<<3)
#define PM_READ_STATUSVALUE (0<<4)
#define PM_READ_કિંમત       (1<<4)
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

#define CALIBRATE_ગણતરી 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
સ્થિર શૂન્ય
tsctimer_setup(શૂન્ય)
{
    // Setup "timer2"
    u8 orig = inb(PORT_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, PORT_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_WORD|PM_MODE0|PM_CNT_દ્વિઅંકી, PORT_PIT_MODE);
    /* LSB of ticks */
    outb(CALIBRATE_ગણતરી & 0xFF, PORT_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_ગણતરી >> 8, PORT_PIT_COUNTER2);

    u64 start = rdtscll();
    જ્યારે ((inb(PORT_PS2_CTRLB) & PPCB_T2OUT) == 0)
        ;
    u64 end = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, PORT_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = end - start;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)start, (u32)end, (u32)diff);
    u64 t = DIV_ROUND_UP(diff * PMTIMER_HZ, CALIBRATE_ગણતરી);
    જ્યારે (t >= (1<<24)) {
        ShiftTSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_ROUND_UP((u32)t, 1000 * PMTIMER_TO_PIT);
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << ShiftTSC) / 1000);
}

// Setup internal timers.
શૂન્ય
timer_setup(શૂન્ય)
{
    જો (!CONFIG_TSC_TIMER)
        પાછા;
    જો (TimerPort != PORT_PIT_COUNTER0)
        પાછા; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    જો (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    જો (cpuid_features & CPUID_TSC)
        tsctimer_setup();
}

શૂન્ય
tsctimer_setfreq(u32 khz, અચલ અક્ષર *src)
{
    જો (!CONFIG_TSC_TIMER)
        પાછા;
    જો (TimerPort != PORT_PIT_COUNTER0)
        પાછા; // have timer already

    TimerKHz = khz;
    ShiftTSC = 0;
    જ્યારે (TimerKHz >= 6000) {
        ShiftTSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << ShiftTSC) / 1000, src);
}

શૂન્ય
pmtimer_setup(u16 ioport)
{
    જો (!CONFIG_PMTIMER)
        પાછા;
    જો (TimerPort != PORT_PIT_COUNTER0)
        પાછા; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    TimerPort = ioport;
    TimerKHz = DIV_ROUND_UP(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 TimerLast VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
સ્થિર u32
timer_adjust_bits(u32 કિંમત, u32 validbits)
{
    u32 last = GET_નીચલું(TimerLast);
    કિંમત = (last & ~validbits) | (કિંમત & validbits);
    જો (કિંમત < last)
        કિંમત += validbits + 1;
    SET_નીચલું(TimerLast, કિંમત);
    પાછા કિંમત;
}

// Sample the current timer value.
સ્થિર u32
timer_read(શૂન્ય)
{
    u16 port = GET_GLOBAL(TimerPort);
    જો (CONFIG_TSC_TIMER && !port)
        // Read from CPU TSC
        પાછા rdtscll() >> GET_GLOBAL(ShiftTSC);
    જો (CONFIG_PMTIMER && port != PORT_PIT_COUNTER0)
        // Read from PMTIMER
        પાછા timer_adjust_bits(inl(port), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_કિંમત | PM_READ_COUNTER0, PORT_PIT_MODE);
    u16 v = inb(PORT_PIT_COUNTER0) | (inb(PORT_PIT_COUNTER0) << 8);
    પાછા timer_adjust_bits(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    પાછા timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    જો (usecs > 500000)
        પાછા cur + DIV_ROUND_UP(usecs, 1000) * khz;
    પાછા cur + DIV_ROUND_UP(usecs * khz, 1000);
}
સ્થિર u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    જો (nsecs > 500000)
        પાછા cur + DIV_ROUND_UP(nsecs, 1000000) * khz;
    પાછા cur + DIV_ROUND_UP(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
પૂર્ણાંક
timer_check(u32 end)
{
    પાછા (s32)(timer_read() - end) > 0;
}

સ્થિર શૂન્ય
timer_delay(u32 end)
{
    જ્યારે (!timer_check(end))
        cpu_relax();
}

સ્થિર શૂન્ય
timer_sleep(u32 end)
{
    જ્યારે (!timer_check(end))
        yield();
}

શૂન્ય ndelay(u32 ગણતરી) {
    timer_delay(timer_calc_nsec(ગણતરી));
}
શૂન્ય udelay(u32 ગણતરી) {
    timer_delay(timer_calc_usec(ગણતરી));
}
શૂન્ય mdelay(u32 ગણતરી) {
    timer_delay(timer_calc(ગણતરી));
}

શૂન્ય nsleep(u32 ગણતરી) {
    timer_sleep(timer_calc_nsec(ગણતરી));
}
શૂન્ય usleep(u32 ગણતરી) {
    timer_sleep(timer_calc_usec(ગણતરી));
}
શૂન્ય msleep(u32 ગણતરી) {
    timer_sleep(timer_calc(ગણતરી));
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
    પાછા DIV_ROUND_UP(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_ROUND_UP((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    પાછા DIV_ROUND_UP(t, 1000 * PMTIMER_TO_PIT);
}

શૂન્ય
pit_setup(શૂન્ય)
{
    જો (!CONFIG_HARDWARE_IRQ)
        પાછા;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_WORD|PM_MODE2|PM_CNT_દ્વિઅંકી, PORT_PIT_MODE);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, PORT_PIT_COUNTER0);
    outb(0x0, PORT_PIT_COUNTER0);
}
