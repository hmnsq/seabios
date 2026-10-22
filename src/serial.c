// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // SET_BDA
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "hw/serialio.h" // SEROFF_IER
#ଅନ୍ତର୍ଭାଵ "output.h" // debug_enter
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

ସ୍ଥିର u16
detect_serial(u16 port, u8 timeout, u8 ଗଣନା_2)
{
    ଯଦି (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        ଫେରନ୍ତୁ 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    ଯଦି (ier != 0x02)
        ଫେରନ୍ତୁ 0;
    u8 iir = inb(port+SEROFF_IIR);
    ଯଦି ((iir & 0x3f) != 0x02)
        ଫେରନ୍ତୁ 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[ଗଣନା_2], port);
    SET_BDA(com_timeout[ଗଣନା_2], timeout);
    ଫେରନ୍ତୁ 1;
}

ଶୂନ୍ୟ
serial_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_SERIAL)
        ଫେରନ୍ତୁ;
    dprintf(3, "init serial\n");

    u16 ଗଣନା_2 = 0;
    ଗଣନା_2 += detect_serial(PORT_SERIAL1, 0x0a, ଗଣନା_2);
    ଗଣନା_2 += detect_serial(PORT_SERIAL2, 0x0a, ଗଣନା_2);
    ଗଣନା_2 += detect_serial(PORT_SERIAL3, 0x0a, ଗଣନା_2);
    ଗଣନା_2 += detect_serial(PORT_SERIAL4, 0x0a, ଗଣନା_2);
    dprintf(1, "Found %d serial ports\n", ଗଣନା_2);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, ଗଣନା_2 << 9);
}

ସ୍ଥିର u16
getComAddr(ସଂରଚନା bregs *regs)
{
    ଯଦି (regs->dx >= 4) {
        set_invalid(regs);
        ଫେରନ୍ତୁ 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    ଯଦି (! addr)
        set_invalid(regs);
    ଫେରନ୍ତୁ addr;
}

// SERIAL - INITIALIZE PORT
ସ୍ଥିର ଶୂନ୍ୟ
handle_1400(ସଂରଚନା bregs *regs)
{
    u16 addr = getComAddr(regs);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    ଯଦି ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } ଅନ୍ୟଥା {
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_1401(ସଂରଚନା bregs *regs)
{
    u16 addr = getComAddr(regs);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    ପାଇଁ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ଯଦି ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            ଭାଙ୍ଗ;
        }
        ଯଦି (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            ଭାଙ୍ଗ;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ସ୍ଥିର ଶୂନ୍ୟ
handle_1402(ସଂରଚନା bregs *regs)
{
    u16 addr = getComAddr(regs);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    ପାଇଁ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ଯଦି (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            ଭାଙ୍ଗ;
        }
        ଯଦି (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            ଭାଙ୍ଗ;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
ସ୍ଥିର ଶୂନ୍ୟ
handle_1403(ସଂରଚନା bregs *regs)
{
    u16 addr = getComAddr(regs);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_14XX(ସଂରଚନା bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
ଶୂନ୍ୟ VISIBLE16
handle_14(ସଂରଚନା bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    ଯଦି (! CONFIG_SERIAL) {
        handle_14XX(regs);
        ଫେରନ୍ତୁ;
    }

    ବାଛ (regs->ah) {
    ସ୍ଥିତି 0x00: handle_1400(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: handle_1401(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: handle_1402(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: handle_1403(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_14XX(regs); ଭାଙ୍ଗ;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

ସ୍ଥିର u16
detect_parport(u16 port, u8 timeout, u8 ଗଣନା_2)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    ଯଦି (inb(port) != 0xaa)
        // Not present
        ଫେରନ୍ତୁ 0;
    SET_BDA(port_lpt[ଗଣନା_2], port);
    SET_BDA(lpt_timeout[ଗଣନା_2], timeout);
    ଫେରନ୍ତୁ 1;
}

ଶୂନ୍ୟ
lpt_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_LPT)
        ଫେରନ୍ତୁ;
    dprintf(3, "init lpt\n");

    u16 ଗଣନା_2 = 0;
    ଗଣନା_2 += detect_parport(PORT_LPT1, 0x14, ଗଣନା_2);
    ଗଣନା_2 += detect_parport(PORT_LPT2, 0x14, ଗଣନା_2);
    dprintf(1, "Found %d lpt ports\n", ଗଣନା_2);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, ଗଣନା_2 << 14);
}

ସ୍ଥିର u16
getLptAddr(ସଂରଚନା bregs *regs)
{
    ଯଦି (regs->dx >= 3) {
        set_invalid(regs);
        ଫେରନ୍ତୁ 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    ଯଦି (! addr)
        set_invalid(regs);
    ଫେରନ୍ତୁ addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ସ୍ଥିର ଶୂନ୍ୟ
handle_1700(ସଂରଚନା bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    ପାଇଁ (;;) {
        u8 v = inb(addr+1);
        ଯଦି (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            ଭାଙ୍ଗ;
        }
        ଯଦି (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            ଭାଙ୍ଗ;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
ସ୍ଥିର ଶୂନ୍ୟ
handle_1701(ସଂରଚନା bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ସ୍ଥିର ଶୂନ୍ୟ
handle_1702(ସଂରଚନା bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_17XX(ସଂରଚନା bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
ଶୂନ୍ୟ VISIBLE16
handle_17(ସଂରଚନା bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    ଯଦି (! CONFIG_LPT) {
        handle_17XX(regs);
        ଫେରନ୍ତୁ;
    }

    ବାଛ (regs->ah) {
    ସ୍ଥିତି 0x00: handle_1700(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: handle_1701(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: handle_1702(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_17XX(regs); ଭାଙ୍ଗ;
    }
}
