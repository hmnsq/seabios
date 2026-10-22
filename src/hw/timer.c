// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_LOW
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "util.h" // timer_setup
#അന്തര്ഭാവ "x86.h" // cpuid

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
#define PM_CNT_ദ്വിമാന (0<<0)
#define PM_CNT_BCD    (1<<0)
#define PM_READ_COUNTER0 (1<<1)
#define PM_READ_COUNTER1 (1<<2)
#define PM_READ_COUNTER2 (1<<3)
#define PM_READ_STATUSVALUE (0<<4)
#define PM_READ_മൂല്യം       (1<<4)
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

#define CALIBRATE_എണ്ണം 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
സ്ഥിതം ശൂന്യം
tsctimer_setup(ശൂന്യം)
{
    // Setup "timer2"
    u8 orig = inb(PORT_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, PORT_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_WORD|PM_MODE0|PM_CNT_ദ്വിമാന, PORT_PIT_MODE);
    /* LSB of ticks */
    outb(CALIBRATE_എണ്ണം & 0xFF, PORT_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_എണ്ണം >> 8, PORT_PIT_COUNTER2);

    u64 start = rdtscll();
    ആകുമ്പോൾ ((inb(PORT_PS2_CTRLB) & PPCB_T2OUT) == 0)
        ;
    u64 end = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, PORT_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = end - start;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)start, (u32)end, (u32)diff);
    u64 t = DIV_ROUND_UP(diff * PMTIMER_HZ, CALIBRATE_എണ്ണം);
    ആകുമ്പോൾ (t >= (1<<24)) {
        ShiftTSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_ROUND_UP((u32)t, 1000 * PMTIMER_TO_PIT);
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << ShiftTSC) / 1000);
}

// Setup internal timers.
ശൂന്യം
timer_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TSC_TIMER)
        തിരികെ;
    എങ്കിൽ (TimerPort != PORT_PIT_COUNTER0)
        തിരികെ; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    എങ്കിൽ (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    എങ്കിൽ (cpuid_features & CPUID_TSC)
        tsctimer_setup();
}

ശൂന്യം
tsctimer_setfreq(u32 khz, സ്ഥിരം അക്ഷരം *src)
{
    എങ്കിൽ (!CONFIG_TSC_TIMER)
        തിരികെ;
    എങ്കിൽ (TimerPort != PORT_PIT_COUNTER0)
        തിരികെ; // have timer already

    TimerKHz = khz;
    ShiftTSC = 0;
    ആകുമ്പോൾ (TimerKHz >= 6000) {
        ShiftTSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    TimerPort = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << ShiftTSC) / 1000, src);
}

ശൂന്യം
pmtimer_setup(u16 ioport)
{
    എങ്കിൽ (!CONFIG_PMTIMER)
        തിരികെ;
    എങ്കിൽ (TimerPort != PORT_PIT_COUNTER0)
        തിരികെ; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    TimerPort = ioport;
    TimerKHz = DIV_ROUND_UP(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 TimerLast VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
സ്ഥിതം u32
timer_adjust_bits(u32 മൂല്യം, u32 validbits)
{
    u32 last = GET_താഴെയുള്ള(TimerLast);
    മൂല്യം = (last & ~validbits) | (മൂല്യം & validbits);
    എങ്കിൽ (മൂല്യം < last)
        മൂല്യം += validbits + 1;
    SET_താഴെയുള്ള(TimerLast, മൂല്യം);
    തിരികെ മൂല്യം;
}

// Sample the current timer value.
സ്ഥിതം u32
timer_read(ശൂന്യം)
{
    u16 port = GET_GLOBAL(TimerPort);
    എങ്കിൽ (CONFIG_TSC_TIMER && !port)
        // Read from CPU TSC
        തിരികെ rdtscll() >> GET_GLOBAL(ShiftTSC);
    എങ്കിൽ (CONFIG_PMTIMER && port != PORT_PIT_COUNTER0)
        // Read from PMTIMER
        തിരികെ timer_adjust_bits(inl(port), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_മൂല്യം | PM_READ_COUNTER0, PORT_PIT_MODE);
    u16 v = inb(PORT_PIT_COUNTER0) | (inb(PORT_PIT_COUNTER0) << 8);
    തിരികെ timer_adjust_bits(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    തിരികെ timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    എങ്കിൽ (usecs > 500000)
        തിരികെ cur + DIV_ROUND_UP(usecs, 1000) * khz;
    തിരികെ cur + DIV_ROUND_UP(usecs * khz, 1000);
}
സ്ഥിതം u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    എങ്കിൽ (nsecs > 500000)
        തിരികെ cur + DIV_ROUND_UP(nsecs, 1000000) * khz;
    തിരികെ cur + DIV_ROUND_UP(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
പൂർണ്ണസംഖ്യ
timer_check(u32 end)
{
    തിരികെ (s32)(timer_read() - end) > 0;
}

സ്ഥിതം ശൂന്യം
timer_delay(u32 end)
{
    ആകുമ്പോൾ (!timer_check(end))
        cpu_relax();
}

സ്ഥിതം ശൂന്യം
timer_sleep(u32 end)
{
    ആകുമ്പോൾ (!timer_check(end))
        yield();
}

ശൂന്യം ndelay(u32 എണ്ണം) {
    timer_delay(timer_calc_nsec(എണ്ണം));
}
ശൂന്യം udelay(u32 എണ്ണം) {
    timer_delay(timer_calc_usec(എണ്ണം));
}
ശൂന്യം mdelay(u32 എണ്ണം) {
    timer_delay(timer_calc(എണ്ണം));
}

ശൂന്യം nsleep(u32 എണ്ണം) {
    timer_sleep(timer_calc_nsec(എണ്ണം));
}
ശൂന്യം usleep(u32 എണ്ണം) {
    timer_sleep(timer_calc_usec(എണ്ണം));
}
ശൂന്യം msleep(u32 എണ്ണം) {
    timer_sleep(timer_calc(എണ്ണം));
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
    തിരികെ DIV_ROUND_UP(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_ROUND_UP((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    തിരികെ DIV_ROUND_UP(t, 1000 * PMTIMER_TO_PIT);
}

ശൂന്യം
pit_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_HARDWARE_IRQ)
        തിരികെ;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_WORD|PM_MODE2|PM_CNT_ദ്വിമാന, PORT_PIT_MODE);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, PORT_PIT_COUNTER0);
    outb(0x0, PORT_PIT_COUNTER0);
}
