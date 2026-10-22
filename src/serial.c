// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // SET_BDA
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "hw/serialio.h" // SEROFF_IER
#અન્તર્ભાવ "output.h" // debug_enter
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

સ્થિર u16
detect_serial(u16 port, u8 timeout, u8 ગણતરી)
{
    જો (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        પાછા 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    જો (ier != 0x02)
        પાછા 0;
    u8 iir = inb(port+SEROFF_IIR);
    જો ((iir & 0x3f) != 0x02)
        પાછા 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[ગણતરી], port);
    SET_BDA(com_timeout[ગણતરી], timeout);
    પાછા 1;
}

શૂન્ય
serial_setup(શૂન્ય)
{
    જો (! CONFIG_SERIAL)
        પાછા;
    dprintf(3, "init serial\n");

    u16 ગણતરી = 0;
    ગણતરી += detect_serial(PORT_SERIAL1, 0x0a, ગણતરી);
    ગણતરી += detect_serial(PORT_SERIAL2, 0x0a, ગણતરી);
    ગણતરી += detect_serial(PORT_SERIAL3, 0x0a, ગણતરી);
    ગણતરી += detect_serial(PORT_SERIAL4, 0x0a, ગણતરી);
    dprintf(1, "Found %d serial ports\n", ગણતરી);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, ગણતરી << 9);
}

સ્થિર u16
getComAddr(સંરચના bregs *regs)
{
    જો (regs->dx >= 4) {
        set_invalid(regs);
        પાછા 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    જો (! addr)
        set_invalid(regs);
    પાછા addr;
}

// SERIAL - INITIALIZE PORT
સ્થિર શૂન્ય
handle_1400(સંરચના bregs *regs)
{
    u16 addr = getComAddr(regs);
    જો (!addr)
        પાછા;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    જો ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } નહીંતર {
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
સ્થિર શૂન્ય
handle_1401(સંરચના bregs *regs)
{
    u16 addr = getComAddr(regs);
    જો (!addr)
        પાછા;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    માટે (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        જો ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            તોડો;
        }
        જો (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            તોડો;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
સ્થિર શૂન્ય
handle_1402(સંરચના bregs *regs)
{
    u16 addr = getComAddr(regs);
    જો (!addr)
        પાછા;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    માટે (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        જો (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            તોડો;
        }
        જો (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            તોડો;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
સ્થિર શૂન્ય
handle_1403(સંરચના bregs *regs)
{
    u16 addr = getComAddr(regs);
    જો (!addr)
        પાછા;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

સ્થિર શૂન્ય
handle_14XX(સંરચના bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
શૂન્ય VISIBLE16
handle_14(સંરચના bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    જો (! CONFIG_SERIAL) {
        handle_14XX(regs);
        પાછા;
    }

    પસંદગી (regs->ah) {
    સ્થિતિ 0x00: handle_1400(regs); તોડો;
    સ્થિતિ 0x01: handle_1401(regs); તોડો;
    સ્થિતિ 0x02: handle_1402(regs); તોડો;
    સ્થિતિ 0x03: handle_1403(regs); તોડો;
    મૂળભૂત:   handle_14XX(regs); તોડો;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

સ્થિર u16
detect_parport(u16 port, u8 timeout, u8 ગણતરી)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    જો (inb(port) != 0xaa)
        // Not present
        પાછા 0;
    SET_BDA(port_lpt[ગણતરી], port);
    SET_BDA(lpt_timeout[ગણતરી], timeout);
    પાછા 1;
}

શૂન્ય
lpt_setup(શૂન્ય)
{
    જો (! CONFIG_LPT)
        પાછા;
    dprintf(3, "init lpt\n");

    u16 ગણતરી = 0;
    ગણતરી += detect_parport(PORT_LPT1, 0x14, ગણતરી);
    ગણતરી += detect_parport(PORT_LPT2, 0x14, ગણતરી);
    dprintf(1, "Found %d lpt ports\n", ગણતરી);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, ગણતરી << 14);
}

સ્થિર u16
getLptAddr(સંરચના bregs *regs)
{
    જો (regs->dx >= 3) {
        set_invalid(regs);
        પાછા 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    જો (! addr)
        set_invalid(regs);
    પાછા addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
સ્થિર શૂન્ય
handle_1700(સંરચના bregs *regs)
{
    u16 addr = getLptAddr(regs);
    જો (!addr)
        પાછા;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    માટે (;;) {
        u8 v = inb(addr+1);
        જો (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            તોડો;
        }
        જો (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            તોડો;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
સ્થિર શૂન્ય
handle_1701(સંરચના bregs *regs)
{
    u16 addr = getLptAddr(regs);
    જો (!addr)
        પાછા;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
સ્થિર શૂન્ય
handle_1702(સંરચના bregs *regs)
{
    u16 addr = getLptAddr(regs);
    જો (!addr)
        પાછા;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

સ્થિર શૂન્ય
handle_17XX(સંરચના bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
શૂન્ય VISIBLE16
handle_17(સંરચના bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    જો (! CONFIG_LPT) {
        handle_17XX(regs);
        પાછા;
    }

    પસંદગી (regs->ah) {
    સ્થિતિ 0x00: handle_1700(regs); તોડો;
    સ્થિતિ 0x01: handle_1701(regs); તોડો;
    સ્થિતિ 0x02: handle_1702(regs); તોડો;
    મૂળભૂત:   handle_17XX(regs); તોડો;
    }
}
