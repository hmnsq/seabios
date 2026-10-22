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

#परिभाषय पत्तनम्_PIT_COUNTER0      0x0040
#परिभाषय पत्तनम्_PIT_COUNTER1      0x0041
#परिभाषय पत्तनम्_PIT_COUNTER2      0x0042
#परिभाषय पत्तनम्_PIT_बहुलकम्          0x0043
#परिभाषय पत्तनम्_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#परिभाषय PM_SEL_TIMER0   (0<<6)
#परिभाषय PM_SEL_TIMER1   (1<<6)
#परिभाषय PM_SEL_TIMER2   (2<<6)
#परिभाषय PM_SEL_READBACK (3<<6)
#परिभाषय PM_ACCESS_LATCH  (0<<4)
#परिभाषय PM_ACCESS_LOBYTE (1<<4)
#परिभाषय PM_ACCESS_HIBYTE (2<<4)
#परिभाषय PM_ACCESS_WORD   (3<<4)
#परिभाषय PM_MODE0 (0<<1)
#परिभाषय PM_MODE1 (1<<1)
#परिभाषय PM_MODE2 (2<<1)
#परिभाषय PM_MODE3 (3<<1)
#परिभाषय PM_MODE4 (4<<1)
#परिभाषय PM_MODE5 (5<<1)
#परिभाषय PM_CNT_द्विमान (0<<0)
#परिभाषय PM_CNT_BCD    (1<<0)
#परिभाषय PM_READ_COUNTER0 (1<<1)
#परिभाषय PM_READ_COUNTER1 (1<<2)
#परिभाषय PM_READ_COUNTER2 (1<<3)
#परिभाषय PM_READ_STATUSVALUE (0<<4)
#परिभाषय PM_READ_मानम्       (1<<4)
#परिभाषय PM_READ_स्थितिः      (2<<4)

// Bits for PORT_PS2_CTRLB
#परिभाषय PPCB_T2GATE (1<<0)
#परिभाषय PPCB_SPKR   (1<<1)
#परिभाषय PPCB_T2_बहिर्गमनम्  (1<<5)

#परिभाषय PMTIMER_HZ 3579545      // Underlying Hz of the PM Timer
#परिभाषय PMTIMER_गन्तव्यम्_PIT 3        // Ratio of pmtimer rate to pit rate

u32 TimerKHz VARFSEG = DIV_आवर्तनम्_ऊर्ध्वम्(PMTIMER_HZ, 1000 * PMTIMER_गन्तव्यम्_PIT);
u16 Timer_पत्तनम् VARFSEG = पत्तनम्_PIT_COUNTER0;
u8 सरणम्_TSC VARFSEG;


/****************************************************************
 * Internal timer setup
 ****************************************************************/

#परिभाषय CALIBRATE_गणनम् 0x800   // Approx 1.7ms

// Calibrate the CPU time-stamp-counter
स्थिर शून्य
tsctimer_सज्जता(शून्य)
{
    // Setup "timer2"
    u8 orig = inb(पत्तनम्_PS2_CTRLB);
    outb((orig & ~PPCB_SPKR) | PPCB_T2GATE, पत्तनम्_PS2_CTRLB);
    /* binary, mode 0, LSB/MSB, Ch 2 */
    outb(PM_SEL_TIMER2|PM_ACCESS_WORD|PM_MODE0|PM_CNT_द्विमान, पत्तनम्_PIT_बहुलकम्);
    /* LSB of ticks */
    outb(CALIBRATE_गणनम् & 0xFF, पत्तनम्_PIT_COUNTER2);
    /* MSB of ticks */
    outb(CALIBRATE_गणनम् >> 8, पत्तनम्_PIT_COUNTER2);

    u64 start = rdtscll();
    यावत् ((inb(पत्तनम्_PS2_CTRLB) & PPCB_T2_बहिर्गमनम्) == 0)
        ;
    u64 अन्तः = rdtscll();

    // Restore PORT_PS2_CTRLB
    outb(orig, पत्तनम्_PS2_CTRLB);

    // Store calibrated cpu khz.
    u64 diff = अन्तः - start;
    dprintf(6, "tsc calibrate start=%u end=%u diff=%u\n"
            , (u32)start, (u32)अन्तः, (u32)diff);
    u64 t = DIV_आवर्तनम्_ऊर्ध्वम्(diff * PMTIMER_HZ, CALIBRATE_गणनम्);
    यावत् (t >= (1<<24)) {
        सरणम्_TSC++;
        t = (t + 1) >> 1;
    }
    TimerKHz = DIV_आवर्तनम्_ऊर्ध्वम्((u32)t, 1000 * PMTIMER_गन्तव्यम्_PIT);
    Timer_पत्तनम् = 0;

    dprintf(1, "CPU Mhz=%u\n", (TimerKHz << सरणम्_TSC) / 1000);
}

// Setup internal timers.
शून्य
timer_सज्जता(शून्य)
{
    यदि (!CONFIG_TSC_TIMER)
        प्रत्यागम;
    यदि (Timer_पत्तनम् != पत्तनम्_PIT_COUNTER0)
        प्रत्यागम; // have timer already

    // Check if CPU has a timestamp counter
    u32 eax, ebx, ecx, edx, cpuid_features = 0;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    यदि (eax > 0)
        cpuid(1, &eax, &ebx, &ecx, &cpuid_features);
    यदि (cpuid_features & CPUID_TSC)
        tsctimer_सज्जता();
}

शून्य
tsctimer_setfreq(u32 khz, ध्रुव वर्ण *src)
{
    यदि (!CONFIG_TSC_TIMER)
        प्रत्यागम;
    यदि (Timer_पत्तनम् != पत्तनम्_PIT_COUNTER0)
        प्रत्यागम; // have timer already

    TimerKHz = khz;
    सरणम्_TSC = 0;
    यावत् (TimerKHz >= 6000) {
        सरणम्_TSC++;
        TimerKHz = (TimerKHz + 1) >> 1;
    }
    Timer_पत्तनम् = 0;

    dprintf(1, "CPU Mhz=%u (%s)\n", (TimerKHz << सरणम्_TSC) / 1000, src);
}

शून्य
pmtimer_सज्जता(u16 ioport)
{
    यदि (!CONFIG_PMTIMER)
        प्रत्यागम;
    यदि (Timer_पत्तनम् != पत्तनम्_PIT_COUNTER0)
        प्रत्यागम; // have timer already

    dprintf(1, "Using pmtimer, ioport 0x%x\n", ioport);
    Timer_पत्तनम् = ioport;
    TimerKHz = DIV_आवर्तनम्_ऊर्ध्वम्(PMTIMER_HZ, 1000);
}


/****************************************************************
 * Internal timer reading
 ****************************************************************/

u32 Timer_चरमः VARLOW;

// Add extra high bits to timers that have less than 32bits of precision.
स्थिर u32
timer_adjust_द्वयङ्काः(u32 मानम्, u32 validbits)
{
    u32 चरमः = GET_नीचः(Timer_चरमः);
    मानम् = (चरमः & ~validbits) | (मानम् & validbits);
    यदि (मानम् < चरमः)
        मानम् += validbits + 1;
    समुच्चयः_नीचः(Timer_चरमः, मानम्);
    प्रत्यागम मानम्;
}

// Sample the current timer value.
स्थिर u32
timer_read(शून्य)
{
    u16 पत्तनम् = GET_GLOBAL(Timer_पत्तनम्);
    यदि (CONFIG_TSC_TIMER && !पत्तनम्)
        // Read from CPU TSC
        प्रत्यागम rdtscll() >> GET_GLOBAL(सरणम्_TSC);
    यदि (CONFIG_PMTIMER && पत्तनम् != पत्तनम्_PIT_COUNTER0)
        // Read from PMTIMER
        प्रत्यागम timer_adjust_द्वयङ्काः(inl(पत्तनम्), 0xffffff);
    // Read from PIT.
    outb(PM_SEL_READBACK | PM_READ_मानम् | PM_READ_COUNTER0, पत्तनम्_PIT_बहुलकम्);
    u16 v = inb(पत्तनम्_PIT_COUNTER0) | (inb(पत्तनम्_PIT_COUNTER0) << 8);
    प्रत्यागम timer_adjust_द्वयङ्काः(-v, 0xffff);
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
    यदि (usecs > 500000)
        प्रत्यागम cur + DIV_आवर्तनम्_ऊर्ध्वम्(usecs, 1000) * khz;
    प्रत्यागम cur + DIV_आवर्तनम्_ऊर्ध्वम्(usecs * khz, 1000);
}
स्थिर u32
timer_calc_nsec(u32 nsecs)
{
    u32 cur = timer_read(), khz = GET_GLOBAL(TimerKHz);
    यदि (nsecs > 500000)
        प्रत्यागम cur + DIV_आवर्तनम्_ऊर्ध्वम्(nsecs, 1000000) * khz;
    प्रत्यागम cur + DIV_आवर्तनम्_ऊर्ध्वम्(nsecs * khz, 1000000);
}

// Check if the current time is past a previously calculated end time.
पूर्णाङ्क
timer_check(u32 अन्तः)
{
    प्रत्यागम (s32)(timer_read() - अन्तः) > 0;
}

स्थिर शून्य
timer_delay(u32 अन्तः)
{
    यावत् (!timer_check(अन्तः))
        cpu_relax();
}

स्थिर शून्य
timer_sleep(u32 अन्तः)
{
    यावत् (!timer_check(अन्तः))
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

#परिभाषय PIT_TICK_INTERVAL 65536 // Default interval for 18.2Hz timer

// Return the number of milliseconds in 'ticks' number of timer irqs.
u32
ticks_गन्तव्यम्_ms(u32 ticks)
{
    u32 t = PIT_TICK_INTERVAL * 1000 * PMTIMER_गन्तव्यम्_PIT * ticks;
    प्रत्यागम DIV_आवर्तनम्_ऊर्ध्वम्(t, PMTIMER_HZ);
}

// Return the number of timer irqs in 'ms' number of milliseconds.
u32
ticks_from_ms(u32 ms)
{
    u32 t = DIV_आवर्तनम्_ऊर्ध्वम्((u64)ms * PMTIMER_HZ, PIT_TICK_INTERVAL);
    प्रत्यागम DIV_आवर्तनम्_ऊर्ध्वम्(t, 1000 * PMTIMER_गन्तव्यम्_PIT);
}

शून्य
pit_सज्जता(शून्य)
{
    यदि (!CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    // timer0: binary count, 16bit count, mode 2
    outb(PM_SEL_TIMER0|PM_ACCESS_WORD|PM_MODE2|PM_CNT_द्विमान, पत्तनम्_PIT_बहुलकम्);
    // maximum count of 0000H = 18.2Hz
    outb(0x0, पत्तनम्_PIT_COUNTER0);
    outb(0x0, पत्तनम्_PIT_COUNTER0);
}
