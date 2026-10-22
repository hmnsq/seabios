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

#تعریف_کړه پورټ_PIT_COUNTER0      0x0040
#تعریف_کړه پورټ_PIT_COUNTER1      0x0041
#تعریف_کړه پورټ_PIT_COUNTER2      0x0042
#تعریف_کړه پورټ_PIT_ډېرپېښ          0x0043
#تعریف_کړه پورټ_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#تعریف_کړه PM_SEL_TIMER0   (0<<6)
#تعریف_کړه PM_SEL_TIMER1   (1<<6)
#تعریف_کړه PM_SEL_TIMER2   (2<<6)
#تعریف_کړه PM_SEL_READBACK (3<<6)
#تعریف_کړه PM_ACCESS_LATCH  (0<<4)
#تعریف_کړه PM_ACCESS_LOBYTE (1<<4)
#تعریف_کړه PM_ACCESS_HIBYTE (2<<4)
#تعریف_کړه PM_ACCESS_کلمه   (3<<4)
#تعریف_کړه PM_MODE0 (0<<1)
#تعریف_کړه PM_MODE1 (1<<1)
#تعریف_کړه PM_MODE2 (2<<1)
#تعریف_کړه PM_MODE3 (3<<1)
#تعریف_کړه PM_MODE4 (4<<1)
#تعریف_کړه PM_MODE5 (5<<1)
#تعریف_کړه PM_CNT_دوګونی (0<<0)
#تعریف_کړه PM_CNT_BCD    (1<<0)
#تعریف_کړه PM_READ_COUNTER0 (1<<1)
#تعریف_کړه PM_READ_COUNTER1 (1<<2)
#تعریف_کړه PM_READ_COUNTER2 (1<<3)
#تعریف_کړه PM_READ_STATUSVALUE (0<<4)
#تعریف_کړه PM_READ_ارزښت       (1<<4)
#تعریف_کړه PM_READ_حالت      (2<<4)

// Bits for PORT_PS2_CTRLB
#تعریف_کړه PPCB_T2GATE (1<<0)
#تعریف_کړه PPCB_SPKR   (1<<1)
#تعریف_کړه PPCB_T2_بهر  (1<<5)

#تعریف_کړه PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#تعریف_کړه PMTIMER_منزل_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_پړاو_پورته(PMTIMER_HZ, 1000 * PMTIMER_منزل_PIT);
u16 Timer_پورټ VARFSEG = پورټ_PIT_COUNTER0;
u8 ښويدنه_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#تعریف_کړه CALIBRATE_شمېر 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
ساكن فراغ
tsctimer_تیاری(فراغ)
{
    // Setup "timer2"
    u8 orig = inb(پورټ_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, پورټ_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_کلمه|PM_MODE0|PM_CNT_دوګونی, پورټ_PIT_ډېرپېښ);
    /* LSB of ticks */
    outb(CALIBRATE_شمېر & 0xFF, پورټ_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_شمېر >> 8, پورټ_PIT_COUNTER2);

    u64 پیل = rdtscll();
    طالما ((inb(پورټ_PS2_CTRLB) & PPCB_T2_بهر) == 0)
        ;
    u64 پای_ټکی = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, پورټ_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = پای_ټکی - پیل;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)پیل, (u32)پای_ټکی, (u32)diff);
    u64 t = DIV_پړاو_پورته(diff * PMTIMER_HZ, CALIBRATE_شمېر);
    طالما (t >= (1<<24)) {
        ښويدنه_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_پړاو_پورته((u32)t, 1000 * PMTIMER_منزل_PIT);
    Timer_پورټ = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << ښويدنه_TSC) / 1000);
}

// Setup internal timers.
فراغ
timer_تیاری(فراغ)
{
    إذا (!CONFIG_TSC_TIMER)
        أرجع;
    إذا (Timer_پورټ != پورټ_PIT_COUNTER0)
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
    إذا (Timer_پورټ != پورټ_PIT_COUNTER0)
        أرجع; // have timer already

    TimerKHz = khz;
    ښويدنه_TSC = 0;
    طالما (TimerKHz >= 6000) {
        ښويدنه_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_پورټ = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << ښويدنه_TSC) / 1000, src);
}

فراغ
pmtimer_تیاری(u16 ioport)
{
    إذا (!CONFIG_PMTIMER)
        أرجع;
    إذا (Timer_پورټ != پورټ_PIT_COUNTER0)
        أرجع; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_پورټ = ioport;
    TimerKHz = DIV_پړاو_پورته(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_وروستی VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
ساكن u32
timer_adjust_بټونه(u32 ارزښت, u32 validbits)
{
    u32 وروستی = GET_ټیټ(Timer_وروستی);
    ارزښت = (وروستی & ~validbits) | (ارزښت & validbits);
    إذا (ارزښت < وروستی)
        ارزښت += validbits + 1;
    سېټ_ټیټ(Timer_وروستی, ارزښت);
    أرجع ارزښت;
}

// Sample the current timer value.
ساكن u32
timer_read(فراغ)
{
    u16 پورټ = GET_GLOBAL(Timer_پورټ);
    إذا (CONFIG_TSC_TIMER && !پورټ)
        // Read from CPU TSC
        أرجع rdtscll() >> GET_GLOBAL(ښويدنه_TSC);
    إذا (CONFIG_PMTIMER && پورټ != پورټ_PIT_COUNTER0)
        // Read from PMTIMER
        أرجع timer_adjust_بټونه(inl(پورټ), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_ارزښت | PM_READ_COUNTER0, پورټ_PIT_ډېرپېښ);
    u16 v = inb(پورټ_PIT_COUNTER0) | (inb(پورټ_PIT_COUNTER0) << 8);
    أرجع timer_adjust_بټونه(-v, 0xffff);
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
        أرجع cur + DIV_پړاو_پورته(usecs, 1000) * khz;
    أرجع cur + DIV_پړاو_پورته(usecs * khz, 1000);
}
ساكن u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    إذا (nsecs > 500000)
        أرجع cur + DIV_پړاو_پورته(nsecs, 1000000) * khz;
    أرجع cur + DIV_پړاو_پورته(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
صحيح
timer_check(u32 پای_ټکی)
{
    أرجع (s32)(timer_read() - پای_ټکی) > 0;
}

ساكن فراغ
timer_delay(u32 پای_ټکی)
{
    طالما (!timer_check(پای_ټکی))
        cpu_relax();
}

ساكن فراغ
timer_sleep(u32 پای_ټکی)
{
    طالما (!timer_check(پای_ټکی))
        yield();
}

فراغ ndelay(u32 شمېر) {
    timer_delay(timer_calc_nsec(شمېر));
}
فراغ udelay(u32 شمېر) {
    timer_delay(timer_calc_usec(شمېر));
}
فراغ mdelay(u32 شمېر) {
    timer_delay(timer_calc(شمېر));
}

فراغ nsleep(u32 شمېر) {
    timer_sleep(timer_calc_nsec(شمېر));
}
فراغ usleep(u32 شمېر) {
    timer_sleep(timer_calc_usec(شمېر));
}
فراغ msleep(u32 شمېر) {
    timer_sleep(timer_calc(شمېر));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#تعریف_کړه PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_منزل_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_منزل_PIT * ticks;
    أرجع DIV_پړاو_پورته(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_پړاو_پورته((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    أرجع DIV_پړاو_پورته(t, 1000 * PMTIMER_منزل_PIT);
}

فراغ
pit_تیاری(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_کلمه|PM_MODE2|PM_CNT_دوګونی, پورټ_PIT_ډېرپېښ);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, پورټ_PIT_COUNTER0);
    outb(0x0, پورټ_PIT_COUNTER0);
}
