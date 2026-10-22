// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // SET_BDA
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "hw/serialio.h" // SEROFF_IER
#അന്തര്ഭാവ "output.h" // debug_enter
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

സ്ഥിതം u16
detect_serial(u16 port, u8 timeout, u8 എണ്ണം)
{
    എങ്കിൽ (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        തിരികെ 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    എങ്കിൽ (ier != 0x02)
        തിരികെ 0;
    u8 iir = inb(port+SEROFF_IIR);
    എങ്കിൽ ((iir & 0x3f) != 0x02)
        തിരികെ 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[എണ്ണം], port);
    SET_BDA(com_timeout[എണ്ണം], timeout);
    തിരികെ 1;
}

ശൂന്യം
serial_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_SERIAL)
        തിരികെ;
    dprintf(3, "init serial\n");

    u16 എണ്ണം = 0;
    എണ്ണം += detect_serial(PORT_SERIAL1, 0x0a, എണ്ണം);
    എണ്ണം += detect_serial(PORT_SERIAL2, 0x0a, എണ്ണം);
    എണ്ണം += detect_serial(PORT_SERIAL3, 0x0a, എണ്ണം);
    എണ്ണം += detect_serial(PORT_SERIAL4, 0x0a, എണ്ണം);
    dprintf(1, "Found %d serial ports\n", എണ്ണം);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, എണ്ണം << 9);
}

സ്ഥിതം u16
getComAddr(ഘടന bregs *regs)
{
    എങ്കിൽ (regs->dx >= 4) {
        set_invalid(regs);
        തിരികെ 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    എങ്കിൽ (! addr)
        set_invalid(regs);
    തിരികെ addr;
}

// SERIAL - INITIALIZE PORT
സ്ഥിതം ശൂന്യം
handle_1400(ഘടന bregs *regs)
{
    u16 addr = getComAddr(regs);
    എങ്കിൽ (!addr)
        തിരികെ;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    എങ്കിൽ ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } അല്ലെങ്കിൽ {
        u16 val16 = 0x600 >> ((regs->al & 0xE0) >> 5);
        outb(val16 & 0xFF, addr+SEROFF_DLL);
        outb(val16 >> 8, addr+SEROFF_DLH);
    }
    outb(regs->al & 0x1F, addr+SEROFF_LCR);
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
സ്ഥിതം ശൂന്യം
handle_1401(ഘടന bregs *regs)
{
    u16 addr = getComAddr(regs);
    എങ്കിൽ (!addr)
        തിരികെ;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    വേണ്ടി (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        എങ്കിൽ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            നിർത്തുക;
        }
        എങ്കിൽ (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            നിർത്തുക;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
സ്ഥിതം ശൂന്യം
handle_1402(ഘടന bregs *regs)
{
    u16 addr = getComAddr(regs);
    എങ്കിൽ (!addr)
        തിരികെ;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    വേണ്ടി (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        എങ്കിൽ (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            നിർത്തുക;
        }
        എങ്കിൽ (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            നിർത്തുക;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
സ്ഥിതം ശൂന്യം
handle_1403(ഘടന bregs *regs)
{
    u16 addr = getComAddr(regs);
    എങ്കിൽ (!addr)
        തിരികെ;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_14XX(ഘടന bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
ശൂന്യം VISIBLE16
handle_14(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    എങ്കിൽ (! CONFIG_SERIAL) {
        handle_14XX(regs);
        തിരികെ;
    }

    തിരഞ്ഞെടുപ്പ് (regs->ah) {
    അവസ്ഥ 0x00: handle_1400(regs); നിർത്തുക;
    അവസ്ഥ 0x01: handle_1401(regs); നിർത്തുക;
    അവസ്ഥ 0x02: handle_1402(regs); നിർത്തുക;
    അവസ്ഥ 0x03: handle_1403(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_14XX(regs); നിർത്തുക;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

സ്ഥിതം u16
detect_parport(u16 port, u8 timeout, u8 എണ്ണം)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    എങ്കിൽ (inb(port) != 0xaa)
        // Not present
        തിരികെ 0;
    SET_BDA(port_lpt[എണ്ണം], port);
    SET_BDA(lpt_timeout[എണ്ണം], timeout);
    തിരികെ 1;
}

ശൂന്യം
lpt_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_LPT)
        തിരികെ;
    dprintf(3, "init lpt\n");

    u16 എണ്ണം = 0;
    എണ്ണം += detect_parport(PORT_LPT1, 0x14, എണ്ണം);
    എണ്ണം += detect_parport(PORT_LPT2, 0x14, എണ്ണം);
    dprintf(1, "Found %d lpt ports\n", എണ്ണം);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, എണ്ണം << 14);
}

സ്ഥിതം u16
getLptAddr(ഘടന bregs *regs)
{
    എങ്കിൽ (regs->dx >= 3) {
        set_invalid(regs);
        തിരികെ 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    എങ്കിൽ (! addr)
        set_invalid(regs);
    തിരികെ addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
സ്ഥിതം ശൂന്യം
handle_1700(ഘടന bregs *regs)
{
    u16 addr = getLptAddr(regs);
    എങ്കിൽ (!addr)
        തിരികെ;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    വേണ്ടി (;;) {
        u8 v = inb(addr+1);
        എങ്കിൽ (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            നിർത്തുക;
        }
        എങ്കിൽ (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            നിർത്തുക;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
സ്ഥിതം ശൂന്യം
handle_1701(ഘടന bregs *regs)
{
    u16 addr = getLptAddr(regs);
    എങ്കിൽ (!addr)
        തിരികെ;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
സ്ഥിതം ശൂന്യം
handle_1702(ഘടന bregs *regs)
{
    u16 addr = getLptAddr(regs);
    എങ്കിൽ (!addr)
        തിരികെ;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_17XX(ഘടന bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
ശൂന്യം VISIBLE16
handle_17(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    എങ്കിൽ (! CONFIG_LPT) {
        handle_17XX(regs);
        തിരികെ;
    }

    തിരഞ്ഞെടുപ്പ് (regs->ah) {
    അവസ്ഥ 0x00: handle_1700(regs); നിർത്തുക;
    അവസ്ഥ 0x01: handle_1701(regs); നിർത്തുക;
    അവസ്ഥ 0x02: handle_1702(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_17XX(regs); നിർത്തുക;
    }
}
