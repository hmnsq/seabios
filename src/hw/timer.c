// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_LOW
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "util.h" // timer_setup
#అన్తర్భావ "x86.h" // cpuid

#నిర్వచించు పోర్టు_PIT_COUNTER0      0x0040
#నిర్వచించు పోర్టు_PIT_COUNTER1      0x0041
#నిర్వచించు పోర్టు_PIT_COUNTER2      0x0042
#నిర్వచించు పోర్టు_PIT_బహుళకం          0x0043
#నిర్వచించు పోర్టు_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#నిర్వచించు PM_SEL_TIMER0   (0<<6)
#నిర్వచించు PM_SEL_TIMER1   (1<<6)
#నిర్వచించు PM_SEL_TIMER2   (2<<6)
#నిర్వచించు PM_SEL_READBACK (3<<6)
#నిర్వచించు PM_ACCESS_LATCH  (0<<4)
#నిర్వచించు PM_ACCESS_LOBYTE (1<<4)
#నిర్వచించు PM_ACCESS_HIBYTE (2<<4)
#నిర్వచించు PM_ACCESS_మాట   (3<<4)
#నిర్వచించు PM_MODE0 (0<<1)
#నిర్వచించు PM_MODE1 (1<<1)
#నిర్వచించు PM_MODE2 (2<<1)
#నిర్వచించు PM_MODE3 (3<<1)
#నిర్వచించు PM_MODE4 (4<<1)
#నిర్వచించు PM_MODE5 (5<<1)
#నిర్వచించు PM_CNT_ద్విమాన (0<<0)
#నిర్వచించు PM_CNT_BCD    (1<<0)
#నిర్వచించు PM_READ_COUNTER0 (1<<1)
#నిర్వచించు PM_READ_COUNTER1 (1<<2)
#నిర్వచించు PM_READ_COUNTER2 (1<<3)
#నిర్వచించు PM_READ_STATUSVALUE (0<<4)
#నిర్వచించు PM_READ_విలువ       (1<<4)
#నిర్వచించు PM_READ_స్థితి      (2<<4)

// Bits for PORT_PS2_CTRLB
#నిర్వచించు PPCB_T2GATE (1<<0)
#నిర్వచించు PPCB_SPKR   (1<<1)
#నిర్వచించు PPCB_T2_బహిర్గతం  (1<<5)

#నిర్వచించు PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#నిర్వచించు PMTIMER_గమ్యం_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_విడత_పైకి(PMTIMER_HZ, 1000 * PMTIMER_గమ్యం_PIT);
u16 Timer_పోర్టు VARFSEG = పోర్టు_PIT_COUNTER0;
u8 జరుపు_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#నిర్వచించు CALIBRATE_లెక్క 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
స్థిర శూన్య
tsctimer_అమరిక(శూన్య)
{
    // Setup "timer2"
    u8 orig = inb(పోర్టు_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, పోర్టు_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_మాట|PM_MODE0|PM_CNT_ద్విమాన, పోర్టు_PIT_బహుళకం);
    /* LSB of ticks */
    outb(CALIBRATE_లెక్క & 0xFF, పోర్టు_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_లెక్క >> 8, పోర్టు_PIT_COUNTER2);

    u64 ప్రారంభం = rdtscll();
    అయితే ((inb(పోర్టు_PS2_CTRLB) & PPCB_T2_బహిర్గతం) == 0)
        ;
    u64 చివర = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, పోర్టు_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = చివర - ప్రారంభం;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)ప్రారంభం, (u32)చివర, (u32)diff);
    u64 t = DIV_విడత_పైకి(diff * PMTIMER_HZ, CALIBRATE_లెక్క);
    అయితే (t >= (1<<24)) {
        జరుపు_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_విడత_పైకి((u32)t, 1000 * PMTIMER_గమ్యం_PIT);
    Timer_పోర్టు = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << జరుపు_TSC) / 1000);
}

// Setup internal timers.
శూన్య
timer_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_TSC_TIMER)
        తిరిగి;
    ఒకవేళ (Timer_పోర్టు != పోర్టు_PIT_COUNTER0)
        తిరిగి; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    ఒకవేళ (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    ఒకవేళ (cpuid_features & CPUID_TSC)
        tsctimer_అమరిక();
}

శూన్య
tsctimer_setfreq(u32 khz, స్థిరరాశి అక్షరం *src)
{
    ఒకవేళ (!CONFIG_TSC_TIMER)
        తిరిగి;
    ఒకవేళ (Timer_పోర్టు != పోర్టు_PIT_COUNTER0)
        తిరిగి; // have timer already

    TimerKHz = khz;
    జరుపు_TSC = 0;
    అయితే (TimerKHz >= 6000) {
        జరుపు_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_పోర్టు = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << జరుపు_TSC) / 1000, src);
}

శూన్య
pmtimer_అమరిక(u16 ioport)
{
    ఒకవేళ (!CONFIG_PMTIMER)
        తిరిగి;
    ఒకవేళ (Timer_పోర్టు != పోర్టు_PIT_COUNTER0)
        తిరిగి; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_పోర్టు = ioport;
    TimerKHz = DIV_విడత_పైకి(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_చివరి VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
స్థిర u32
timer_adjust_బిట్లు(u32 విలువ, u32 validbits)
{
    u32 చివరి = GET_దిగువ(Timer_చివరి);
    విలువ = (చివరి & ~validbits) | (విలువ & validbits);
    ఒకవేళ (విలువ < చివరి)
        విలువ += validbits + 1;
    సమితి_దిగువ(Timer_చివరి, విలువ);
    తిరిగి విలువ;
}

// Sample the current timer value.
స్థిర u32
timer_read(శూన్య)
{
    u16 పోర్టు = GET_GLOBAL(Timer_పోర్టు);
    ఒకవేళ (CONFIG_TSC_TIMER && !పోర్టు)
        // Read from CPU TSC
        తిరిగి rdtscll() >> GET_GLOBAL(జరుపు_TSC);
    ఒకవేళ (CONFIG_PMTIMER && పోర్టు != పోర్టు_PIT_COUNTER0)
        // Read from PMTIMER
        తిరిగి timer_adjust_బిట్లు(inl(పోర్టు), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_విలువ | PM_READ_COUNTER0, పోర్టు_PIT_బహుళకం);
    u16 v = inb(పోర్టు_PIT_COUNTER0) | (inb(పోర్టు_PIT_COUNTER0) << 8);
    తిరిగి timer_adjust_బిట్లు(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    తిరిగి timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    ఒకవేళ (usecs > 500000)
        తిరిగి cur + DIV_విడత_పైకి(usecs, 1000) * khz;
    తిరిగి cur + DIV_విడత_పైకి(usecs * khz, 1000);
}
స్థిర u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    ఒకవేళ (nsecs > 500000)
        తిరిగి cur + DIV_విడత_పైకి(nsecs, 1000000) * khz;
    తిరిగి cur + DIV_విడత_పైకి(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
పూర్ణాంకం
timer_check(u32 చివర)
{
    తిరిగి (s32)(timer_read() - చివర) > 0;
}

స్థిర శూన్య
timer_delay(u32 చివర)
{
    అయితే (!timer_check(చివర))
        cpu_relax();
}

స్థిర శూన్య
timer_sleep(u32 చివర)
{
    అయితే (!timer_check(చివర))
        yield();
}

శూన్య ndelay(u32 లెక్క) {
    timer_delay(timer_calc_nsec(లెక్క));
}
శూన్య udelay(u32 లెక్క) {
    timer_delay(timer_calc_usec(లెక్క));
}
శూన్య mdelay(u32 లెక్క) {
    timer_delay(timer_calc(లెక్క));
}

శూన్య nsleep(u32 లెక్క) {
    timer_sleep(timer_calc_nsec(లెక్క));
}
శూన్య usleep(u32 లెక్క) {
    timer_sleep(timer_calc_usec(లెక్క));
}
శూన్య msleep(u32 లెక్క) {
    timer_sleep(timer_calc(లెక్క));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#నిర్వచించు PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_గమ్యం_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_గమ్యం_PIT * ticks;
    తిరిగి DIV_విడత_పైకి(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_విడత_పైకి((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    తిరిగి DIV_విడత_పైకి(t, 1000 * PMTIMER_గమ్యం_PIT);
}

శూన్య
pit_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ)
        తిరిగి;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_మాట|PM_MODE2|PM_CNT_ద్విమాన, పోర్టు_PIT_బహుళకం);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, పోర్టు_PIT_COUNTER0);
    outb(0x0, పోర్టు_PIT_COUNTER0);
}
