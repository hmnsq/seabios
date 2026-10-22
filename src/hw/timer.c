// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_LOW
#समावेश "config.h" // CONFIG_*
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // yield
#समावेश "util.h" // timer_setup
#समावेश "x86.h" // cpuid

#घोषणा PORT_PIT_COUNTER0      0x0040
#घोषणा PORT_PIT_COUNTER1      0x0041
#घोषणा PORT_PIT_COUNTER2      0x0042
#घोषणा PORT_PIT_MODE          0x0043
#घोषणा PORT_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#घोषणा PM_SEL_TIMER0   (0<<6)
#घोषणा PM_SEL_TIMER1   (1<<6)
#घोषणा PM_SEL_TIMER2   (2<<6)
#घोषणा PM_SEL_READBACK (3<<6)
#घोषणा PM_ACCESS_LATCH  (0<<4)
#घोषणा PM_ACCESS_LOBYTE (1<<4)
#घोषणा PM_ACCESS_HIBYTE (2<<4)
#घोषणा PM_ACCESS_WORD   (3<<4)
#घोषणा PM_MODE0 (0<<1)
#घोषणा PM_MODE1 (1<<1)
#घोषणा PM_MODE2 (2<<1)
#घोषणा PM_MODE3 (3<<1)
#घोषणा PM_MODE4 (4<<1)
#घोषणा PM_MODE5 (5<<1)
#घोषणा PM_CNT_द्विआधारी (0<<0)
#घोषणा PM_CNT_BCD    (1<<0)
#घोषणा PM_READ_COUNTER0 (1<<1)
#घोषणा PM_READ_COUNTER1 (1<<2)
#घोषणा PM_READ_COUNTER2 (1<<3)
#घोषणा PM_READ_STATUSVALUE (0<<4)
#घोषणा PM_READ_मूल्य       (1<<4)
#घोषणा PM_READ_STATUS      (2<<4)

// Bits for PORT_PS2_CTRLB
#घोषणा PPCB_T2GATE (1<<0)
#घोषणा PPCB_SPKR   (1<<1)
#घोषणा PPCB_T2OUT  (1<<5)

#घोषणा PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#घोषणा PMTIMER_TO_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_ROUND_UP(PMTIMER_HZ, 1000 * PMTIMER_TO_PIT);
u16 TimerPort VARFSEG = PORT_PIT_COUNTER0;
u8 ShiftTSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#घोषणा CALIBRATE_गिनती 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
अटल व्योम
tsctimer_setup(व्योम)
{
    // Setup "timer2"
    u8 orig = inb(PORT_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, PORT_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_WORD|PM_MODE0|PM_CNT_द्विआधारी, PORT_PIT_MODE);
    /* LSB of ticks */
    outb(CALIBRATE_गिनती & 0xFF, PORT_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_गिनती >> 8, PORT_PIT_COUNTER2);

    u64 start = rdtscll();
    जबतक ((inb(PORT_PS2_CTRLB) & PPCB_T2OUT) == 0)
        ;
    u64 end = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, PORT_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = end - start;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)start, (u32)end, (u32)diff);
    u64 t = DIV_ROUND_UP(diff * PMTIMER_HZ, CALIBRATE_गिनती);
    जबतक (t >= (1<<24)) {
        ShiftTSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_ROUND_UP((u32)t, 1000 * PMTIMER_TO_PIT);
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << ShiftTSC) / 1000);
}

// Setup internal timers.
व्योम
timer_setup(व्योम)
{
    अगर (!CONFIG_TSC_TIMER)
        वापस;
    अगर (TimerPort != PORT_PIT_COUNTER0)
        वापस; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    अगर (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    अगर (cpuid_features & CPUID_TSC)
        tsctimer_setup();
}

व्योम
tsctimer_setfreq(u32 khz, स्थिर अक्षर *src)
{
    अगर (!CONFIG_TSC_TIMER)
        वापस;
    अगर (TimerPort != PORT_PIT_COUNTER0)
        वापस; // have timer already

    TimerKHz = khz;
    ShiftTSC = 0;
    जबतक (TimerKHz >= 6000) {
        ShiftTSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << ShiftTSC) / 1000, src);
}

व्योम
pmtimer_setup(u16 ioport)
{
    अगर (!CONFIG_PMTIMER)
        वापस;
    अगर (TimerPort != PORT_PIT_COUNTER0)
        वापस; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    TimerPort = ioport;
    TimerKHz = DIV_ROUND_UP(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 TimerLast VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
अटल u32
timer_adjust_bits(u32 मूल्य, u32 validbits)
{
    u32 last = GET_निचला(TimerLast);
    मूल्य = (last & ~validbits) | (मूल्य & validbits);
    अगर (मूल्य < last)
        मूल्य += validbits + 1;
    SET_निचला(TimerLast, मूल्य);
    वापस मूल्य;
}

// Sample the current timer value.
अटल u32
timer_read(व्योम)
{
    u16 port = GET_GLOBAL(TimerPort);
    अगर (CONFIG_TSC_TIMER && !port)
        // Read from CPU TSC
        वापस rdtscll() >> GET_GLOBAL(ShiftTSC);
    अगर (CONFIG_PMTIMER && port != PORT_PIT_COUNTER0)
        // Read from PMTIMER
        वापस timer_adjust_bits(inl(port), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_मूल्य | PM_READ_COUNTER0, PORT_PIT_MODE);
    u16 v = inb(PORT_PIT_COUNTER0) | (inb(PORT_PIT_COUNTER0) << 8);
    वापस timer_adjust_bits(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    वापस timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    अगर (usecs > 500000)
        वापस cur + DIV_ROUND_UP(usecs, 1000) * khz;
    वापस cur + DIV_ROUND_UP(usecs * khz, 1000);
}
अटल u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    अगर (nsecs > 500000)
        वापस cur + DIV_ROUND_UP(nsecs, 1000000) * khz;
    वापस cur + DIV_ROUND_UP(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
पूर्णांक
timer_check(u32 end)
{
    वापस (s32)(timer_read() - end) > 0;
}

अटल व्योम
timer_delay(u32 end)
{
    जबतक (!timer_check(end))
        cpu_relax();
}

अटल व्योम
timer_sleep(u32 end)
{
    जबतक (!timer_check(end))
        yield();
}

व्योम ndelay(u32 गिनती) {
    timer_delay(timer_calc_nsec(गिनती));
}
व्योम udelay(u32 गिनती) {
    timer_delay(timer_calc_usec(गिनती));
}
व्योम mdelay(u32 गिनती) {
    timer_delay(timer_calc(गिनती));
}

व्योम nsleep(u32 गिनती) {
    timer_sleep(timer_calc_nsec(गिनती));
}
व्योम usleep(u32 गिनती) {
    timer_sleep(timer_calc_usec(गिनती));
}
व्योम msleep(u32 गिनती) {
    timer_sleep(timer_calc(गिनती));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#घोषणा PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_to_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_TO_PIT * ticks;
    वापस DIV_ROUND_UP(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_ROUND_UP((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    वापस DIV_ROUND_UP(t, 1000 * PMTIMER_TO_PIT);
}

व्योम
pit_setup(व्योम)
{
    अगर (!CONFIG_HARDWARE_IRQ)
        वापस;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_WORD|PM_MODE2|PM_CNT_द्विआधारी, PORT_PIT_MODE);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, PORT_PIT_COUNTER0);
    outb(0x0, PORT_PIT_COUNTER0);
}
