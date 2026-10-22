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

#وصف_ڪريو پورٽ_PIT_COUNTER0      0x0040
#وصف_ڪريو پورٽ_PIT_COUNTER1      0x0041
#وصف_ڪريو پورٽ_PIT_COUNTER2      0x0042
#وصف_ڪريو پورٽ_PIT_گهڻائي          0x0043
#وصف_ڪريو پورٽ_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#وصف_ڪريو PM_SEL_TIMER0   (0<<6)
#وصف_ڪريو PM_SEL_TIMER1   (1<<6)
#وصف_ڪريو PM_SEL_TIMER2   (2<<6)
#وصف_ڪريو PM_SEL_READBACK (3<<6)
#وصف_ڪريو PM_ACCESS_LATCH  (0<<4)
#وصف_ڪريو PM_ACCESS_LOBYTE (1<<4)
#وصف_ڪريو PM_ACCESS_HIBYTE (2<<4)
#وصف_ڪريو PM_ACCESS_لفظ   (3<<4)
#وصف_ڪريو PM_MODE0 (0<<1)
#وصف_ڪريو PM_MODE1 (1<<1)
#وصف_ڪريو PM_MODE2 (2<<1)
#وصف_ڪريو PM_MODE3 (3<<1)
#وصف_ڪريو PM_MODE4 (4<<1)
#وصف_ڪريو PM_MODE5 (5<<1)
#وصف_ڪريو PM_CNT_ٻٽو (0<<0)
#وصف_ڪريو PM_CNT_BCD    (1<<0)
#وصف_ڪريو PM_READ_COUNTER0 (1<<1)
#وصف_ڪريو PM_READ_COUNTER1 (1<<2)
#وصف_ڪريو PM_READ_COUNTER2 (1<<3)
#وصف_ڪريو PM_READ_STATUSVALUE (0<<4)
#وصف_ڪريو PM_READ_قدر       (1<<4)
#وصف_ڪريو PM_READ_حالت      (2<<4)

// Bits for PORT_PS2_CTRLB
#وصف_ڪريو PPCB_T2GATE (1<<0)
#وصف_ڪريو PPCB_SPKR   (1<<1)
#وصف_ڪريو PPCB_T2_ٻاهريون  (1<<5)

#وصف_ڪريو PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#وصف_ڪريو PMTIMER_منزل_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_ڦيرو_مٿي(PMTIMER_HZ, 1000 * PMTIMER_منزل_PIT);
u16 Timer_پورٽ VARFSEG = پورٽ_PIT_COUNTER0;
u8 سرڪايو_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#وصف_ڪريو CALIBRATE_ڳڻپ 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
ساكن فراغ
tsctimer_تياري(فراغ)
{
    // Setup "timer2"
    u8 orig = inb(پورٽ_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, پورٽ_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_لفظ|PM_MODE0|PM_CNT_ٻٽو, پورٽ_PIT_گهڻائي);
    /* LSB of ticks */
    outb(CALIBRATE_ڳڻپ & 0xFF, پورٽ_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_ڳڻپ >> 8, پورٽ_PIT_COUNTER2);

    u64 شروعات = rdtscll();
    طالما ((inb(پورٽ_PS2_CTRLB) & PPCB_T2_ٻاهريون) == 0)
        ;
    u64 پڇاڙي = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, پورٽ_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = پڇاڙي - شروعات;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)شروعات, (u32)پڇاڙي, (u32)diff);
    u64 t = DIV_ڦيرو_مٿي(diff * PMTIMER_HZ, CALIBRATE_ڳڻپ);
    طالما (t >= (1<<24)) {
        سرڪايو_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_ڦيرو_مٿي((u32)t, 1000 * PMTIMER_منزل_PIT);
    Timer_پورٽ = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << سرڪايو_TSC) / 1000);
}

// Setup internal timers.
فراغ
timer_تياري(فراغ)
{
    إذا (!CONFIG_TSC_TIMER)
        أرجع;
    إذا (Timer_پورٽ != پورٽ_PIT_COUNTER0)
        أرجع; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    إذا (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    إذا (cpuid_features & CPUID_TSC)
        tsctimer_تياري();
}

فراغ
tsctimer_setfreq(u32 khz, ثابت حرف *src)
{
    إذا (!CONFIG_TSC_TIMER)
        أرجع;
    إذا (Timer_پورٽ != پورٽ_PIT_COUNTER0)
        أرجع; // have timer already

    TimerKHz = khz;
    سرڪايو_TSC = 0;
    طالما (TimerKHz >= 6000) {
        سرڪايو_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_پورٽ = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << سرڪايو_TSC) / 1000, src);
}

فراغ
pmtimer_تياري(u16 ioport)
{
    إذا (!CONFIG_PMTIMER)
        أرجع;
    إذا (Timer_پورٽ != پورٽ_PIT_COUNTER0)
        أرجع; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_پورٽ = ioport;
    TimerKHz = DIV_ڦيرو_مٿي(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_آخري VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
ساكن u32
timer_adjust_بٽ(u32 قدر, u32 validbits)
{
    u32 آخري = GET_هيٺيون(Timer_آخري);
    قدر = (آخري & ~validbits) | (قدر & validbits);
    إذا (قدر < آخري)
        قدر += validbits + 1;
    سيٽ_هيٺيون(Timer_آخري, قدر);
    أرجع قدر;
}

// Sample the current timer value.
ساكن u32
timer_read(فراغ)
{
    u16 پورٽ = GET_GLOBAL(Timer_پورٽ);
    إذا (CONFIG_TSC_TIMER && !پورٽ)
        // Read from CPU TSC
        أرجع rdtscll() >> GET_GLOBAL(سرڪايو_TSC);
    إذا (CONFIG_PMTIMER && پورٽ != پورٽ_PIT_COUNTER0)
        // Read from PMTIMER
        أرجع timer_adjust_بٽ(inl(پورٽ), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_قدر | PM_READ_COUNTER0, پورٽ_PIT_گهڻائي);
    u16 v = inb(پورٽ_PIT_COUNTER0) | (inb(پورٽ_PIT_COUNTER0) << 8);
    أرجع timer_adjust_بٽ(-v, 0xffff);
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
        أرجع cur + DIV_ڦيرو_مٿي(usecs, 1000) * khz;
    أرجع cur + DIV_ڦيرو_مٿي(usecs * khz, 1000);
}
ساكن u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    إذا (nsecs > 500000)
        أرجع cur + DIV_ڦيرو_مٿي(nsecs, 1000000) * khz;
    أرجع cur + DIV_ڦيرو_مٿي(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
صحيح
timer_check(u32 پڇاڙي)
{
    أرجع (s32)(timer_read() - پڇاڙي) > 0;
}

ساكن فراغ
timer_delay(u32 پڇاڙي)
{
    طالما (!timer_check(پڇاڙي))
        cpu_relax();
}

ساكن فراغ
timer_sleep(u32 پڇاڙي)
{
    طالما (!timer_check(پڇاڙي))
        yield();
}

فراغ ndelay(u32 ڳڻپ) {
    timer_delay(timer_calc_nsec(ڳڻپ));
}
فراغ udelay(u32 ڳڻپ) {
    timer_delay(timer_calc_usec(ڳڻپ));
}
فراغ mdelay(u32 ڳڻپ) {
    timer_delay(timer_calc(ڳڻپ));
}

فراغ nsleep(u32 ڳڻپ) {
    timer_sleep(timer_calc_nsec(ڳڻپ));
}
فراغ usleep(u32 ڳڻپ) {
    timer_sleep(timer_calc_usec(ڳڻپ));
}
فراغ msleep(u32 ڳڻپ) {
    timer_sleep(timer_calc(ڳڻپ));
}


/****************************************************************
 * PIT setup
 ****************************************************************/

#وصف_ڪريو PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_منزل_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_منزل_PIT * ticks;
    أرجع DIV_ڦيرو_مٿي(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_ڦيرو_مٿي((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    أرجع DIV_ڦيرو_مٿي(t, 1000 * PMTIMER_منزل_PIT);
}

فراغ
pit_تياري(فراغ)
{
    إذا (!CONFIG_HARDWARE_IRQ)
        أرجع;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_لفظ|PM_MODE2|PM_CNT_ٻٽو, پورٽ_PIT_گهڻائي);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, پورٽ_PIT_COUNTER0);
    outb(0x0, پورٽ_PIT_COUNTER0);
}
