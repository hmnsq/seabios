// Internal timer and Intel 8253 Programmable Interrupt Timer (PIT) support.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_LOW
#ضمن "config.h" // CONFIG_*
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // yield
#ضمن "util.h" // timer_setup
#ضمن "x86.h" // cpuid

#تعریف_کرو پورٹ_PIT_COUNTER0      0x0040
#تعریف_کرو پورٹ_PIT_COUNTER1      0x0041
#تعریف_کرو پورٹ_PIT_COUNTER2      0x0042
#تعریف_کرو پورٹ_PIT_منوال          0x0043
#تعریف_کرو پورٹ_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#تعریف_کرو PM_SEL_TIMER0   (0<<6)
#تعریف_کرو PM_SEL_TIMER1   (1<<6)
#تعریف_کرو PM_SEL_TIMER2   (2<<6)
#تعریف_کرو PM_SEL_READBACK (3<<6)
#تعریف_کرو PM_ACCESS_LATCH  (0<<4)
#تعریف_کرو PM_ACCESS_LOBYTE (1<<4)
#تعریف_کرو PM_ACCESS_HIBYTE (2<<4)
#تعریف_کرو PM_ACCESS_لفظ   (3<<4)
#تعریف_کرو PM_MODE0 (0<<1)
#تعریف_کرو PM_MODE1 (1<<1)
#تعریف_کرو PM_MODE2 (2<<1)
#تعریف_کرو PM_MODE3 (3<<1)
#تعریف_کرو PM_MODE4 (4<<1)
#تعریف_کرو PM_MODE5 (5<<1)
#تعریف_کرو PM_CNT_ثنائی (0<<0)
#تعریف_کرو PM_CNT_BCD    (1<<0)
#تعریف_کرو PM_READ_COUNTER0 (1<<1)
#تعریف_کرو PM_READ_COUNTER1 (1<<2)
#تعریف_کرو PM_READ_COUNTER2 (1<<3)
#تعریف_کرو PM_READ_STATUSVALUE (0<<4)
#تعریف_کرو PM_READ_قدر       (1<<4)
#تعریف_کرو PM_READ_حالت      (2<<4)

// Bits for PORT_PS2_CTRLB
#تعریف_کرو PPCB_T2GATE (1<<0)
#تعریف_کرو PPCB_SPKR   (1<<1)
#تعریف_کرو PPCB_T2_باہرلا  (1<<5)

#تعریف_کرو PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#تعریف_کرو PMTIMER_منزل_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_گیڑا_اتے(PMTIMER_HZ, 1000 * PMTIMER_منزل_PIT);
u16 Timer_پورٹ VARFSEG = پورٹ_PIT_COUNTER0;
u8 کھسکاؤ_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#تعریف_کرو CALIBRATE_گنتی 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
ساكن فراغ
tsctimer_تیاری(فراغ)
{
    // Setup "timer2"
    u8 orig = inb(پورٹ_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, پورٹ_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_لفظ|PM_MODE0|PM_CNT_ثنائی, پورٹ_PIT_منوال);
    /* LSB of ticks */
    outb(CALIBRATE_گنتی & 0xFF, پورٹ_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_گنتی >> 8, پورٹ_PIT_COUNTER2);

    u64 شروع = rdtscll();
    طالما ((inb(پورٹ_PS2_CTRLB) & PPCB_T2_باہرلا) == 0)
        ;
    u64 سرا = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, پورٹ_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = سرا - شروع;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)شروع, (u32)سرا, (u32)diff);
    u64 t = DIV_گیڑا_اتے(diff * PMTIMER_HZ, CALIBRATE_گنتی);
    طالما (t >= (1<<24)) {
        کھسکاؤ_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_گیڑا_اتے((u32)t, 1000 * PMTIMER_منزل_PIT);
    Timer_پورٹ = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << کھسکاؤ_TSC) / 1000);
}

// Setup internal timers.
فراغ
timer_تیاری(فراغ)
{
    إذا (!CONFIG_TSC_TIMER)
        أرجع;
    إذا (Timer_پورٹ != پورٹ_PIT_COUNTER0)
        أرجع; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    إذا (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    إذا (cpuid_features & CPUID_TSC)
        tsctimer_تیاری();
}

فراغ
tsctimer_setfreq(u32 khz, ثابت حرف *src)
{
    إذا (!CONFIG_TSC_TIMER)
        أرجع;
    إذا (Timer_پورٹ != پورٹ_PIT_COUNTER0)
        أرجع; // have timer already

    TimerKHz = khz;
    کھسکاؤ_TSC = 0;
    طالما (TimerKHz >= 6000) {
        کھسکاؤ_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_پورٹ = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << کھسکاؤ_TSC) / 1000, src);
}

فراغ
pmtimer_تیاری(u16 ioport)
{
    إذا (!CONFIG_PMTIMER)
        أرجع;
    إذا (Timer_پورٹ != پورٹ_PIT_COUNTER0)
        أرجع; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_پورٹ = ioport;
    TimerKHz = DIV_گیڑا_اتے(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_آخری VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
ساكن u32
timer_adjust_بٹ(u32 قدر, u32 validbits)
{
    u32 آخری = GET_کم(Timer_آخری);
    قدر = (آخری & ~validbits) | (قدر & validbits);
    إذا (قدر < آخری)
        قدر += validbits + 1;
    سیٹ_کم(Timer_آخری, قدر);
    أرجع قدر;
}

// Sample the current timer value.
ساكن u32
timer_read(فراغ)
{
    u16 پورٹ = GET_GLOBAL(Timer_پورٹ);
    إذا (CONFIG_TSC_TIMER && !پورٹ)
        // Read from CPU TSC
        أرجع rdtscll() >> GET_GLOBAL(کھسکاؤ_TSC);
    إذا (CONFIG_PMTIMER && پورٹ != پورٹ_PIT_COUNTER0)
        // Read from PMTIMER
        أرجع timer_adjust_بٹ(inl(پورٹ), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_قدر | PM_READ_COUNTER0, پورٹ_PIT_منوال);
    u16 v = inb(پورٹ_PIT_COUNTER0) | (inb(پورٹ_PIT_COUNTER0) << 8);
    أرجع timer_adjust_بٹ(-v, 0xffff);
}

// Return the TSC value that is 'msecs' time in the future.
u32
timer_calc(u32 msecs)
{
    أرجع timer_read() + (GET_GLOBAL(TimerKHz) * msecs);
}
u32
timer_calc_usec(u32 usecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    إذا (usecs > 500000)
        أرجع cur + DIV_گیڑا_اتے(usecs, 1000) * khz;
    أرجع cur + DIV_گیڑا_اتے(usecs * khz, 1000);
}
ساكن u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    إذا (nsecs > 500000)
        أرجع cur + DIV_گیڑا_اتے(nsecs, 1000000) * khz;
    أرجع cur + DIV_گیڑا_اتے(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
صحيح
timer_check(u32 سرا)
{
    أرجع (s32)(timer_read() - سرا) > 0;
}

ساكن فراغ
timer_delay(u32 سرا)
{
    طالما (!timer_check(سرا))
        cpu_relax();
}

ساكن فراغ
timer_sleep(u32 سرا)
{
    طالما (!timer_check(سرا))
        yield();
}

فراغ ndelay(u32 گنتی) {
    timer_delay(timer_calc_nsec(گنتی));
}
فراغ udelay(u32 گنتی) {
    timer_delay(timer_calc_usec(گنتی));
}
فراغ mdelay(u32 گنتی) {
    timer_delay(timer_calc(گنتی));
}

فراغ nsleep(u32 گنتی) {
    timer_sleep(timer_calc_nsec(گنتی));
}
فراغ usleep(u32 گنتی) {
    timer_sleep(timer_calc_usec(گنتی));
}
فراغ msleep(u32 گنتی) {
    timer_sleep(timer_calc(گنتی));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#تعریف_کرو PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_منزل_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_منزل_PIT * ticks;
    أرجع DIV_گیڑا_اتے(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_گیڑا_اتے((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    أرجع DIV_گیڑا_اتے(t, 1000 * PMTIMER_منزل_PIT);
}

فراغ
pit_تیاری(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_لفظ|PM_MODE2|PM_CNT_ثنائی, پورٹ_PIT_منوال);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, پورٹ_PIT_COUNTER0);
    outb(0x0, پورٹ_PIT_COUNTER0);
}
