// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // SET_BDA
#شامل "bregs.h" // struct bregs
#شامل "hw/serialio.h" // SEROFF_IER
#شامل "output.h" // debug_enter
#شامل "romfile.h" // romfile_loadint
#شامل "stacks.h" // yield
#شامل "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

ایستا u16
detect_serial(u16 port, u8 timeout, u8 شمار)
{
    اگر (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        بازگشت 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    اگر (ier != 0x02)
        بازگشت 0;
    u8 iir = inb(port+SEROFF_IIR);
    اگر ((iir & 0x3f) != 0x02)
        بازگشت 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[شمار], port);
    SET_BDA(com_timeout[شمار], timeout);
    بازگشت 1;
}

تهی
serial_setup(تهی)
{
    اگر (! CONFIG_SERIAL)
        بازگشت;
    dprintf(3, "init serial\n");

    u16 شمار = 0;
    شمار += detect_serial(PORT_SERIAL1, 0x0a, شمار);
    شمار += detect_serial(PORT_SERIAL2, 0x0a, شمار);
    شمار += detect_serial(PORT_SERIAL3, 0x0a, شمار);
    شمار += detect_serial(PORT_SERIAL4, 0x0a, شمار);
    dprintf(1, "Found %d serial ports\n", شمار);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, شمار << 9);
}

ایستا u16
getComAddr(ساختار bregs *regs)
{
    اگر (regs->dx >= 4) {
        set_invalid(regs);
        بازگشت 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    اگر (! addr)
        set_invalid(regs);
    بازگشت addr;
}

// SERIAL - INITIALIZE PORT
ایستا تهی
handle_1400(ساختار bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        بازگشت;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    اگر ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } وگرنه {
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
ایستا تهی
handle_1401(ساختار bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        بازگشت;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    برای (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        اگر ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            بشکن;
        }
        اگر (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            بشکن;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ایستا تهی
handle_1402(ساختار bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        بازگشت;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    برای (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        اگر (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            بشکن;
        }
        اگر (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            بشکن;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
ایستا تهی
handle_1403(ساختار bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        بازگشت;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

ایستا تهی
handle_14XX(ساختار bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
تهی VISIBLE16
handle_14(ساختار bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    اگر (! CONFIG_SERIAL) {
        handle_14XX(regs);
        بازگشت;
    }

    انتخاب (regs->ah) {
    حالت 0x00: handle_1400(regs); بشکن;
    حالت 0x01: handle_1401(regs); بشکن;
    حالت 0x02: handle_1402(regs); بشکن;
    حالت 0x03: handle_1403(regs); بشکن;
    پیشفرض:   handle_14XX(regs); بشکن;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

ایستا u16
detect_parport(u16 port, u8 timeout, u8 شمار)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    اگر (inb(port) != 0xaa)
        // Not present
        بازگشت 0;
    SET_BDA(port_lpt[شمار], port);
    SET_BDA(lpt_timeout[شمار], timeout);
    بازگشت 1;
}

تهی
lpt_setup(تهی)
{
    اگر (! CONFIG_LPT)
        بازگشت;
    dprintf(3, "init lpt\n");

    u16 شمار = 0;
    شمار += detect_parport(PORT_LPT1, 0x14, شمار);
    شمار += detect_parport(PORT_LPT2, 0x14, شمار);
    dprintf(1, "Found %d lpt ports\n", شمار);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, شمار << 14);
}

ایستا u16
getLptAddr(ساختار bregs *regs)
{
    اگر (regs->dx >= 3) {
        set_invalid(regs);
        بازگشت 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    اگر (! addr)
        set_invalid(regs);
    بازگشت addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ایستا تهی
handle_1700(ساختار bregs *regs)
{
    u16 addr = getLptAddr(regs);
    اگر (!addr)
        بازگشت;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    برای (;;) {
        u8 v = inb(addr+1);
        اگر (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            بشکن;
        }
        اگر (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            بشکن;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
ایستا تهی
handle_1701(ساختار bregs *regs)
{
    u16 addr = getLptAddr(regs);
    اگر (!addr)
        بازگشت;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ایستا تهی
handle_1702(ساختار bregs *regs)
{
    u16 addr = getLptAddr(regs);
    اگر (!addr)
        بازگشت;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

ایستا تهی
handle_17XX(ساختار bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
تهی VISIBLE16
handle_17(ساختار bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    اگر (! CONFIG_LPT) {
        handle_17XX(regs);
        بازگشت;
    }

    انتخاب (regs->ah) {
    حالت 0x00: handle_1700(regs); بشکن;
    حالت 0x01: handle_1701(regs); بشکن;
    حالت 0x02: handle_1702(regs); بشکن;
    پیشفرض:   handle_17XX(regs); بشکن;
    }
}
