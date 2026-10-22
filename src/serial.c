// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // SET_BDA
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "hw/serialio.h" // SEROFF_IER
#అన్తర్భావ "output.h" // debug_enter
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

స్థిర u16
detect_serial(u16 పోర్టు, u8 timeout, u8 లెక్క)
{
    ఒకవేళ (CONFIG_DEBUG_SERIAL && పోర్టు == CONFIG_DEBUG_SERIAL_పోర్టు
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        తిరిగి 0;
    outb(0x02, పోర్టు+SEROFF_IER);
    u8 ier = inb(పోర్టు+SEROFF_IER);
    ఒకవేళ (ier != 0x02)
        తిరిగి 0;
    u8 iir = inb(పోర్టు+SEROFF_IIR);
    ఒకవేళ ((iir & 0x3f) != 0x02)
        తిరిగి 0;

    outb(0x00, పోర్టు+SEROFF_IER);
    సమితి_BDA(పోర్టు_com[లెక్క], పోర్టు);
    సమితి_BDA(com_timeout[లెక్క], timeout);
    తిరిగి 1;
}

శూన్య
serial_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_SERIAL)
        తిరిగి;
    dprintf(3, "init serial\n");

    u16 లెక్క = 0;
    లెక్క += detect_serial(పోర్టు_SERIAL1, 0x0a, లెక్క);
    లెక్క += detect_serial(పోర్టు_SERIAL2, 0x0a, లెక్క);
    లెక్క += detect_serial(పోర్టు_SERIAL3, 0x0a, లెక్క);
    లెక్క += detect_serial(పోర్టు_SERIAL4, 0x0a, లెక్క);
    dprintf(1, "Found %d serial ports\n", లెక్క);

    // Equipment word bits 9..11 determing # serial ports
    సమితి_equipment_flags(0xe00, లెక్క << 9);
}

స్థిర u16
getComAddr(నిర్మాణం bregs *regs)
{
    ఒకవేళ (regs->dx >= 4) {
        సమితి_invalid(regs);
        తిరిగి 0;
    }
    u16 addr = GET_BDA(పోర్టు_com[regs->dx]);
    ఒకవేళ (! addr)
        సమితి_invalid(regs);
    తిరిగి addr;
}

// SERIAL - INITIALIZE PORT
స్థిర శూన్య
handle_1400(నిర్మాణం bregs *regs)
{
    u16 addr = getComAddr(regs);
    ఒకవేళ (!addr)
        తిరిగి;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    ఒకవేళ ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } లేకపోతే {
        u16 val16 = 0x600 >> ((regs->al & 0xE0) >> 5);
        outb(val16 & 0xFF, addr+SEROFF_DLL);
        outb(val16 >> 8, addr+SEROFF_DLH);
    }
    outb(regs->al & 0x1F, addr+SEROFF_LCR);
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    సమితి_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
స్థిర శూన్య
handle_1401(నిర్మాణం bregs *regs)
{
    u16 addr = getComAddr(regs);
    ఒకవేళ (!addr)
        తిరిగి;
    u32 చివర = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    కొరకు (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ఒకవేళ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_దత్తాంశం);
            // XXX - reread lsr?
            regs->ah = lsr;
            ఆపు;
        }
        ఒకవేళ (irqtimer_check(చివర)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            ఆపు;
        }
        yield();
    }
    సమితి_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
స్థిర శూన్య
handle_1402(నిర్మాణం bregs *regs)
{
    u16 addr = getComAddr(regs);
    ఒకవేళ (!addr)
        తిరిగి;
    u32 చివర = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    కొరకు (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ఒకవేళ (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_దత్తాంశం);
            regs->ah = lsr;
            ఆపు;
        }
        ఒకవేళ (irqtimer_check(చివర)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            ఆపు;
        }
        yield();
    }
    సమితి_success(regs);
}

// SERIAL - GET PORT STATUS
స్థిర శూన్య
handle_1403(నిర్మాణం bregs *regs)
{
    u16 addr = getComAddr(regs);
    ఒకవేళ (!addr)
        తిరిగి;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    సమితి_success(regs);
}

స్థిర శూన్య
handle_14XX(నిర్మాణం bregs *regs)
{
    సమితి_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
శూన్య VISIBLE16
handle_14(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    ఒకవేళ (! CONFIG_SERIAL) {
        handle_14XX(regs);
        తిరిగి;
    }

    ఎంపిక (regs->ah) {
    స్థితి 0x00: handle_1400(regs); ఆపు;
    స్థితి 0x01: handle_1401(regs); ఆపు;
    స్థితి 0x02: handle_1402(regs); ఆపు;
    స్థితి 0x03: handle_1403(regs); ఆపు;
    అప్రమేయ:   handle_14XX(regs); ఆపు;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

స్థిర u16
detect_parport(u16 పోర్టు, u8 timeout, u8 లెక్క)
{
    // clear input mode
    outb(inb(పోర్టు+2) & 0xdf, పోర్టు+2);

    outb(0xaa, పోర్టు);
    ఒకవేళ (inb(పోర్టు) != 0xaa)
        // Not present
        తిరిగి 0;
    సమితి_BDA(పోర్టు_lpt[లెక్క], పోర్టు);
    సమితి_BDA(lpt_timeout[లెక్క], timeout);
    తిరిగి 1;
}

శూన్య
lpt_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_LPT)
        తిరిగి;
    dprintf(3, "init lpt\n");

    u16 లెక్క = 0;
    లెక్క += detect_parport(పోర్టు_LPT1, 0x14, లెక్క);
    లెక్క += detect_parport(పోర్టు_LPT2, 0x14, లెక్క);
    dprintf(1, "Found %d lpt ports\n", లెక్క);

    // Equipment word bits 14..15 determing # parallel ports
    సమితి_equipment_flags(0xc000, లెక్క << 14);
}

స్థిర u16
getLptAddr(నిర్మాణం bregs *regs)
{
    ఒకవేళ (regs->dx >= 3) {
        సమితి_invalid(regs);
        తిరిగి 0;
    }
    u16 addr = GET_BDA(పోర్టు_lpt[regs->dx]);
    ఒకవేళ (! addr)
        సమితి_invalid(regs);
    తిరిగి addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
స్థిర శూన్య
handle_1700(నిర్మాణం bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ఒకవేళ (!addr)
        తిరిగి;

    u32 చివర = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    కొరకు (;;) {
        u8 v = inb(addr+1);
        ఒకవేళ (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            ఆపు;
        }
        ఒకవేళ (irqtimer_check(చివర)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            ఆపు;
        }
        yield();
    }

    సమితి_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
స్థిర శూన్య
handle_1701(నిర్మాణం bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ఒకవేళ (!addr)
        తిరిగి;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    సమితి_success(regs);
}

// INT 17 - PRINTER - GET STATUS
స్థిర శూన్య
handle_1702(నిర్మాణం bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ఒకవేళ (!addr)
        తిరిగి;
    regs->ah = inb(addr+1) ^ 0x48;
    సమితి_success(regs);
}

స్థిర శూన్య
handle_17XX(నిర్మాణం bregs *regs)
{
    సమితి_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
శూన్య VISIBLE16
handle_17(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    ఒకవేళ (! CONFIG_LPT) {
        handle_17XX(regs);
        తిరిగి;
    }

    ఎంపిక (regs->ah) {
    స్థితి 0x00: handle_1700(regs); ఆపు;
    స్థితి 0x01: handle_1701(regs); ఆపు;
    స్థితి 0x02: handle_1702(regs); ఆపు;
    అప్రమేయ:   handle_17XX(regs); ఆపు;
    }
}
