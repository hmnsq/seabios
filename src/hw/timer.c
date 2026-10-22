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

#تعریف_کن پورت_PIT_COUNTER0      0x0040
#تعریف_کن پورت_PIT_COUNTER1      0x0041
#تعریف_کن پورت_PIT_COUNTER2      0x0042
#تعریف_کن پورت_PIT_نما          0x0043
#تعریف_کن پورت_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#تعریف_کن PM_SEL_TIMER0   (0<<6)
#تعریف_کن PM_SEL_TIMER1   (1<<6)
#تعریف_کن PM_SEL_TIMER2   (2<<6)
#تعریف_کن PM_SEL_READBACK (3<<6)
#تعریف_کن PM_ACCESS_LATCH  (0<<4)
#تعریف_کن PM_ACCESS_LOBYTE (1<<4)
#تعریف_کن PM_ACCESS_HIBYTE (2<<4)
#تعریف_کن PM_ACCESS_کلمه   (3<<4)
#تعریف_کن PM_MODE0 (0<<1)
#تعریف_کن PM_MODE1 (1<<1)
#تعریف_کن PM_MODE2 (2<<1)
#تعریف_کن PM_MODE3 (3<<1)
#تعریف_کن PM_MODE4 (4<<1)
#تعریف_کن PM_MODE5 (5<<1)
#تعریف_کن PM_CNT_دودویی (0<<0)
#تعریف_کن PM_CNT_BCD    (1<<0)
#تعریف_کن PM_READ_COUNTER0 (1<<1)
#تعریف_کن PM_READ_COUNTER1 (1<<2)
#تعریف_کن PM_READ_COUNTER2 (1<<3)
#تعریف_کن PM_READ_STATUSVALUE (0<<4)
#تعریف_کن PM_READ_مقدار       (1<<4)
#تعریف_کن PM_READ_وضعیت      (2<<4)

// Bits for PORT_PS2_CTRLB
#تعریف_کن PPCB_T2GATE (1<<0)
#تعریف_کن PPCB_SPKR   (1<<1)
#تعریف_کن PPCB_T2_بیرون  (1<<5)

#تعریف_کن PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#تعریف_کن PMTIMER_مقصد_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_دور_به_بالا(PMTIMER_HZ, 1000 * PMTIMER_مقصد_PIT);
u16 Timer_پورت VARFSEG = پورت_PIT_COUNTER0;
u8 لغزش_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#تعریف_کن CALIBRATE_شمار 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
ساكن فراغ
tsctimer_راهاندازی(فراغ)
{
    // Setup "timer2"
    u8 orig = inb(پورت_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, پورت_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_کلمه|PM_MODE0|PM_CNT_دودویی, پورت_PIT_نما);
    /* LSB of ticks */
    outb(CALIBRATE_شمار & 0xFF, پورت_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_شمار >> 8, پورت_PIT_COUNTER2);

    u64 شروع = rdtscll();
    طالما ((inb(پورت_PS2_CTRLB) & PPCB_T2_بیرون) == 0)
        ;
    u64 انتها = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, پورت_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = انتها - شروع;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)شروع, (u32)انتها, (u32)diff);
    u64 t = DIV_دور_به_بالا(diff * PMTIMER_HZ, CALIBRATE_شمار);
    طالما (t >= (1<<24)) {
        لغزش_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_دور_به_بالا((u32)t, 1000 * PMTIMER_مقصد_PIT);
    Timer_پورت = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << لغزش_TSC) / 1000);
}

// Setup internal timers.
فراغ
timer_راهاندازی(فراغ)
{
    إذا (!CONFIG_TSC_TIMER)
        أرجع;
    إذا (Timer_پورت != پورت_PIT_COUNTER0)
        أرجع; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    إذا (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    إذا (cpuid_features & CPUID_TSC)
        tsctimer_راهاندازی();
}

فراغ
tsctimer_setfreq(u32 khz, ثابت حرف *src)
{
    إذا (!CONFIG_TSC_TIMER)
        أرجع;
    إذا (Timer_پورت != پورت_PIT_COUNTER0)
        أرجع; // have timer already

    TimerKHz = khz;
    لغزش_TSC = 0;
    طالما (TimerKHz >= 6000) {
        لغزش_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_پورت = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << لغزش_TSC) / 1000, src);
}

فراغ
pmtimer_راهاندازی(u16 ioport)
{
    إذا (!CONFIG_PMTIMER)
        أرجع;
    إذا (Timer_پورت != پورت_PIT_COUNTER0)
        أرجع; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_پورت = ioport;
    TimerKHz = DIV_دور_به_بالا(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_آخرین VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
ساكن u32
timer_adjust_بیتها(u32 مقدار, u32 validbits)
{
    u32 آخرین = GET_کم(Timer_آخرین);
    مقدار = (آخرین & ~validbits) | (مقدار & validbits);
    إذا (مقدار < آخرین)
        مقدار += validbits + 1;
    مجموعه_کم(Timer_آخرین, مقدار);
    أرجع مقدار;
}

// Sample the current timer value.
ساكن u32
timer_read(فراغ)
{
    u16 پورت = GET_GLOBAL(Timer_پورت);
    إذا (CONFIG_TSC_TIMER && !پورت)
        // Read from CPU TSC
        أرجع rdtscll() >> GET_GLOBAL(لغزش_TSC);
    إذا (CONFIG_PMTIMER && پورت != پورت_PIT_COUNTER0)
        // Read from PMTIMER
        أرجع timer_adjust_بیتها(inl(پورت), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_مقدار | PM_READ_COUNTER0, پورت_PIT_نما);
    u16 v = inb(پورت_PIT_COUNTER0) | (inb(پورت_PIT_COUNTER0) << 8);
    أرجع timer_adjust_بیتها(-v, 0xffff);
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
        أرجع cur + DIV_دور_به_بالا(usecs, 1000) * khz;
    أرجع cur + DIV_دور_به_بالا(usecs * khz, 1000);
}
ساكن u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    إذا (nsecs > 500000)
        أرجع cur + DIV_دور_به_بالا(nsecs, 1000000) * khz;
    أرجع cur + DIV_دور_به_بالا(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
صحيح
timer_check(u32 انتها)
{
    أرجع (s32)(timer_read() - انتها) > 0;
}

ساكن فراغ
timer_delay(u32 انتها)
{
    طالما (!timer_check(انتها))
        cpu_relax();
}

ساكن فراغ
timer_sleep(u32 انتها)
{
    طالما (!timer_check(انتها))
        yield();
}

فراغ ndelay(u32 شمار) {
    timer_delay(timer_calc_nsec(شمار));
}
فراغ udelay(u32 شمار) {
    timer_delay(timer_calc_usec(شمار));
}
فراغ mdelay(u32 شمار) {
    timer_delay(timer_calc(شمار));
}

فراغ nsleep(u32 شمار) {
    timer_sleep(timer_calc_nsec(شمار));
}
فراغ usleep(u32 شمار) {
    timer_sleep(timer_calc_usec(شمار));
}
فراغ msleep(u32 شمار) {
    timer_sleep(timer_calc(شمار));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#تعریف_کن PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_مقصد_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_مقصد_PIT * ticks;
    أرجع DIV_دور_به_بالا(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_دور_به_بالا((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    أرجع DIV_دور_به_بالا(t, 1000 * PMTIMER_مقصد_PIT);
}

فراغ
pit_راهاندازی(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_کلمه|PM_MODE2|PM_CNT_دودویی, پورت_PIT_نما);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, پورت_PIT_COUNTER0);
    outb(0x0, پورت_PIT_COUNTER0);
}
