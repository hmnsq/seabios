// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // SET_BDA
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "hw/serialio.h" // SEROFF_IER
#অন্তর্ভাব "output.h" // debug_enter
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

স্থির u16
detect_serial(u16 port, u8 timeout, u8 গুনতি)
{
    যদি (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        ফেরত 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    যদি (ier != 0x02)
        ফেরত 0;
    u8 iir = inb(port+SEROFF_IIR);
    যদি ((iir & 0x3f) != 0x02)
        ফেরত 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[গুনতি], port);
    SET_BDA(com_timeout[গুনতি], timeout);
    ফেরত 1;
}

শূন্য
serial_setup(শূন্য)
{
    যদি (! CONFIG_SERIAL)
        ফেরত;
    dprintf(3, "init serial\n");

    u16 গুনতি = 0;
    গুনতি += detect_serial(PORT_SERIAL1, 0x0a, গুনতি);
    গুনতি += detect_serial(PORT_SERIAL2, 0x0a, গুনতি);
    গুনতি += detect_serial(PORT_SERIAL3, 0x0a, গুনতি);
    গুনতি += detect_serial(PORT_SERIAL4, 0x0a, গুনতি);
    dprintf(1, "Found %d serial ports\n", গুনতি);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, গুনতি << 9);
}

স্থির u16
getComAddr(কাঠামো bregs *regs)
{
    যদি (regs->dx >= 4) {
        set_invalid(regs);
        ফেরত 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    যদি (! addr)
        set_invalid(regs);
    ফেরত addr;
}

// SERIAL - INITIALIZE PORT
স্থির শূন্য
handle_1400(কাঠামো bregs *regs)
{
    u16 addr = getComAddr(regs);
    যদি (!addr)
        ফেরত;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    যদি ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } অন্যথা {
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
স্থির শূন্য
handle_1401(কাঠামো bregs *regs)
{
    u16 addr = getComAddr(regs);
    যদি (!addr)
        ফেরত;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    জন্য (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        যদি ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_তথ্য);
            // XXX - reread lsr?
            regs->ah = lsr;
            বিরতি;
        }
        যদি (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            বিরতি;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
স্থির শূন্য
handle_1402(কাঠামো bregs *regs)
{
    u16 addr = getComAddr(regs);
    যদি (!addr)
        ফেরত;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    জন্য (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        যদি (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_তথ্য);
            regs->ah = lsr;
            বিরতি;
        }
        যদি (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            বিরতি;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
স্থির শূন্য
handle_1403(কাঠামো bregs *regs)
{
    u16 addr = getComAddr(regs);
    যদি (!addr)
        ফেরত;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

স্থির শূন্য
handle_14XX(কাঠামো bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
শূন্য VISIBLE16
handle_14(কাঠামো bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    যদি (! CONFIG_SERIAL) {
        handle_14XX(regs);
        ফেরত;
    }

    নির্বাচন (regs->ah) {
    ক্ষেত্র 0x00: handle_1400(regs); বিরতি;
    ক্ষেত্র 0x01: handle_1401(regs); বিরতি;
    ক্ষেত্র 0x02: handle_1402(regs); বিরতি;
    ক্ষেত্র 0x03: handle_1403(regs); বিরতি;
    পূর্বনির্ধারিত:   handle_14XX(regs); বিরতি;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

স্থির u16
detect_parport(u16 port, u8 timeout, u8 গুনতি)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    যদি (inb(port) != 0xaa)
        // Not present
        ফেরত 0;
    SET_BDA(port_lpt[গুনতি], port);
    SET_BDA(lpt_timeout[গুনতি], timeout);
    ফেরত 1;
}

শূন্য
lpt_setup(শূন্য)
{
    যদি (! CONFIG_LPT)
        ফেরত;
    dprintf(3, "init lpt\n");

    u16 গুনতি = 0;
    গুনতি += detect_parport(PORT_LPT1, 0x14, গুনতি);
    গুনতি += detect_parport(PORT_LPT2, 0x14, গুনতি);
    dprintf(1, "Found %d lpt ports\n", গুনতি);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, গুনতি << 14);
}

স্থির u16
getLptAddr(কাঠামো bregs *regs)
{
    যদি (regs->dx >= 3) {
        set_invalid(regs);
        ফেরত 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    যদি (! addr)
        set_invalid(regs);
    ফেরত addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
স্থির শূন্য
handle_1700(কাঠামো bregs *regs)
{
    u16 addr = getLptAddr(regs);
    যদি (!addr)
        ফেরত;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    জন্য (;;) {
        u8 v = inb(addr+1);
        যদি (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            বিরতি;
        }
        যদি (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            বিরতি;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
স্থির শূন্য
handle_1701(কাঠামো bregs *regs)
{
    u16 addr = getLptAddr(regs);
    যদি (!addr)
        ফেরত;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
স্থির শূন্য
handle_1702(কাঠামো bregs *regs)
{
    u16 addr = getLptAddr(regs);
    যদি (!addr)
        ফেরত;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

স্থির শূন্য
handle_17XX(কাঠামো bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
শূন্য VISIBLE16
handle_17(কাঠামো bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    যদি (! CONFIG_LPT) {
        handle_17XX(regs);
        ফেরত;
    }

    নির্বাচন (regs->ah) {
    ক্ষেত্র 0x00: handle_1700(regs); বিরতি;
    ক্ষেত্র 0x01: handle_1701(regs); বিরতি;
    ক্ষেত্র 0x02: handle_1702(regs); বিরতি;
    পূর্বনির্ধারিত:   handle_17XX(regs); বিরতি;
    }
}
