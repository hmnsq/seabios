// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // SET_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/serialio.h" // SEROFF_IER
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // debug_enter
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

ਸਥਿਰ u16
detect_serial(u16 ਪੋਰਟ, u8 timeout, u8 ਗਿਣਤੀ_2)
{
    ਜੇ (CONFIG_DEBUG_SERIAL && ਪੋਰਟ == CONFIG_DEBUG_SERIAL_ਪੋਰਟ
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        ਵਾਪਸ 0;
    outb(0x02, ਪੋਰਟ+SEROFF_IER);
    u8 ier = inb(ਪੋਰਟ+SEROFF_IER);
    ਜੇ (ier != 0x02)
        ਵਾਪਸ 0;
    u8 iir = inb(ਪੋਰਟ+SEROFF_IIR);
    ਜੇ ((iir & 0x3f) != 0x02)
        ਵਾਪਸ 0;

    outb(0x00, ਪੋਰਟ+SEROFF_IER);
    ਸੈੱਟ_BDA(ਪੋਰਟ_com[ਗਿਣਤੀ_2], ਪੋਰਟ);
    ਸੈੱਟ_BDA(com_timeout[ਗਿਣਤੀ_2], timeout);
    ਵਾਪਸ 1;
}

ਖਾਲੀ
serial_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_SERIAL)
        ਵਾਪਸ;
    dprintf(3, "init serial\n");

    u16 ਗਿਣਤੀ_2 = 0;
    ਗਿਣਤੀ_2 += detect_serial(ਪੋਰਟ_SERIAL1, 0x0a, ਗਿਣਤੀ_2);
    ਗਿਣਤੀ_2 += detect_serial(ਪੋਰਟ_SERIAL2, 0x0a, ਗਿਣਤੀ_2);
    ਗਿਣਤੀ_2 += detect_serial(ਪੋਰਟ_SERIAL3, 0x0a, ਗਿਣਤੀ_2);
    ਗਿਣਤੀ_2 += detect_serial(ਪੋਰਟ_SERIAL4, 0x0a, ਗਿਣਤੀ_2);
    dprintf(1, "Found %d serial ports\n", ਗਿਣਤੀ_2);

    // Equipment word bits 9..11 determing # serial ports
    ਸੈੱਟ_equipment_flags(0xe00, ਗਿਣਤੀ_2 << 9);
}

ਸਥਿਰ u16
getComAddr(ਬਣਤਰ bregs *regs)
{
    ਜੇ (regs->dx >= 4) {
        ਸੈੱਟ_invalid(regs);
        ਵਾਪਸ 0;
    }
    u16 addr = GET_BDA(ਪੋਰਟ_com[regs->dx]);
    ਜੇ (! addr)
        ਸੈੱਟ_invalid(regs);
    ਵਾਪਸ addr;
}

// SERIAL - INITIALIZE PORT
ਸਥਿਰ ਖਾਲੀ
handle_1400(ਬਣਤਰ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ਜੇ (!addr)
        ਵਾਪਸ;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    ਜੇ ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } ਨਹੀਂਤਾਂ {
        u16 val16 = 0x600 >> ((regs->al & 0xE0) >> 5);
        outb(val16 & 0xFF, addr+SEROFF_DLL);
        outb(val16 >> 8, addr+SEROFF_DLH);
    }
    outb(regs->al & 0x1F, addr+SEROFF_LCR);
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    ਸੈੱਟ_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
ਸਥਿਰ ਖਾਲੀ
handle_1401(ਬਣਤਰ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ਜੇ (!addr)
        ਵਾਪਸ;
    u32 ਸਿਰਾ = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    ਲਈ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ਜੇ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_ਅੰਕੜੇ);
            // XXX - reread lsr?
            regs->ah = lsr;
            ਰੋਕੋ;
        }
        ਜੇ (irqtimer_check(ਸਿਰਾ)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            ਰੋਕੋ;
        }
        yield();
    }
    ਸੈੱਟ_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ਸਥਿਰ ਖਾਲੀ
handle_1402(ਬਣਤਰ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ਜੇ (!addr)
        ਵਾਪਸ;
    u32 ਸਿਰਾ = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    ਲਈ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ਜੇ (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_ਅੰਕੜੇ);
            regs->ah = lsr;
            ਰੋਕੋ;
        }
        ਜੇ (irqtimer_check(ਸਿਰਾ)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            ਰੋਕੋ;
        }
        yield();
    }
    ਸੈੱਟ_success(regs);
}

// SERIAL - GET PORT STATUS
ਸਥਿਰ ਖਾਲੀ
handle_1403(ਬਣਤਰ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ਜੇ (!addr)
        ਵਾਪਸ;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_14XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
ਖਾਲੀ VISIBLE16
handle_14(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    ਜੇ (! CONFIG_SERIAL) {
        handle_14XX(regs);
        ਵਾਪਸ;
    }

    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x00: handle_1400(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: handle_1401(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: handle_1402(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x03: handle_1403(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_14XX(regs); ਰੋਕੋ;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

ਸਥਿਰ u16
detect_parport(u16 ਪੋਰਟ, u8 timeout, u8 ਗਿਣਤੀ_2)
{
    // clear input mode
    outb(inb(ਪੋਰਟ+2) & 0xdf, ਪੋਰਟ+2);

    outb(0xaa, ਪੋਰਟ);
    ਜੇ (inb(ਪੋਰਟ) != 0xaa)
        // Not present
        ਵਾਪਸ 0;
    ਸੈੱਟ_BDA(ਪੋਰਟ_lpt[ਗਿਣਤੀ_2], ਪੋਰਟ);
    ਸੈੱਟ_BDA(lpt_timeout[ਗਿਣਤੀ_2], timeout);
    ਵਾਪਸ 1;
}

ਖਾਲੀ
lpt_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_LPT)
        ਵਾਪਸ;
    dprintf(3, "init lpt\n");

    u16 ਗਿਣਤੀ_2 = 0;
    ਗਿਣਤੀ_2 += detect_parport(ਪੋਰਟ_LPT1, 0x14, ਗਿਣਤੀ_2);
    ਗਿਣਤੀ_2 += detect_parport(ਪੋਰਟ_LPT2, 0x14, ਗਿਣਤੀ_2);
    dprintf(1, "Found %d lpt ports\n", ਗਿਣਤੀ_2);

    // Equipment word bits 14..15 determing # parallel ports
    ਸੈੱਟ_equipment_flags(0xc000, ਗਿਣਤੀ_2 << 14);
}

ਸਥਿਰ u16
getLptAddr(ਬਣਤਰ bregs *regs)
{
    ਜੇ (regs->dx >= 3) {
        ਸੈੱਟ_invalid(regs);
        ਵਾਪਸ 0;
    }
    u16 addr = GET_BDA(ਪੋਰਟ_lpt[regs->dx]);
    ਜੇ (! addr)
        ਸੈੱਟ_invalid(regs);
    ਵਾਪਸ addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ਸਥਿਰ ਖਾਲੀ
handle_1700(ਬਣਤਰ bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ਜੇ (!addr)
        ਵਾਪਸ;

    u32 ਸਿਰਾ = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    ਲਈ (;;) {
        u8 v = inb(addr+1);
        ਜੇ (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            ਰੋਕੋ;
        }
        ਜੇ (irqtimer_check(ਸਿਰਾ)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            ਰੋਕੋ;
        }
        yield();
    }

    ਸੈੱਟ_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
ਸਥਿਰ ਖਾਲੀ
handle_1701(ਬਣਤਰ bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ਜੇ (!addr)
        ਵਾਪਸ;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    ਸੈੱਟ_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ਸਥਿਰ ਖਾਲੀ
handle_1702(ਬਣਤਰ bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ਜੇ (!addr)
        ਵਾਪਸ;
    regs->ah = inb(addr+1) ^ 0x48;
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_17XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
ਖਾਲੀ VISIBLE16
handle_17(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    ਜੇ (! CONFIG_LPT) {
        handle_17XX(regs);
        ਵਾਪਸ;
    }

    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x00: handle_1700(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: handle_1701(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: handle_1702(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_17XX(regs); ਰੋਕੋ;
    }
}
