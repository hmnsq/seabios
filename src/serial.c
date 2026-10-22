// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // SET_BDA
#ضمن "bregs.h" // struct bregs
#ضمن "hw/serialio.h" // SEROFF_IER
#ضمن "output.h" // debug_enter
#ضمن "romfile.h" // romfile_loadint
#ضمن "stacks.h" // yield
#ضمن "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

ساكن u16
detect_serial(u16 port, u8 timeout, u8 ژماردن)
{
    إذا (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        أرجع 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    إذا (ier != 0x02)
        أرجع 0;
    u8 iir = inb(port+SEROFF_IIR);
    إذا ((iir & 0x3f) != 0x02)
        أرجع 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[ژماردن], port);
    SET_BDA(com_timeout[ژماردن], timeout);
    أرجع 1;
}

فراغ
serial_setup(فراغ)
{
    إذا (! CONFIG_SERIAL)
        أرجع;
    dprintf(3, "init serial\n");

    u16 ژماردن = 0;
    ژماردن += detect_serial(PORT_SERIAL1, 0x0a, ژماردن);
    ژماردن += detect_serial(PORT_SERIAL2, 0x0a, ژماردن);
    ژماردن += detect_serial(PORT_SERIAL3, 0x0a, ژماردن);
    ژماردن += detect_serial(PORT_SERIAL4, 0x0a, ژماردن);
    dprintf(1, "Found %d serial ports\n", ژماردن);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, ژماردن << 9);
}

ساكن u16
getComAddr(بنية bregs *regs)
{
    إذا (regs->dx >= 4) {
        set_invalid(regs);
        أرجع 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    إذا (! addr)
        set_invalid(regs);
    أرجع addr;
}

// SERIAL - INITIALIZE PORT
ساكن فراغ
handle_1400(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    إذا ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } وإلا {
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
ساكن فراغ
handle_1401(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            اكسر;
        }
        إذا (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            اكسر;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ساكن فراغ
handle_1402(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            اكسر;
        }
        إذا (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            اكسر;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
ساكن فراغ
handle_1403(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

ساكن فراغ
handle_14XX(بنية bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
فراغ VISIBLE16
handle_14(بنية bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    إذا (! CONFIG_SERIAL) {
        handle_14XX(regs);
        أرجع;
    }

    بدل (regs->ah) {
    حالة 0x00: handle_1400(regs); اكسر;
    حالة 0x01: handle_1401(regs); اكسر;
    حالة 0x02: handle_1402(regs); اكسر;
    حالة 0x03: handle_1403(regs); اكسر;
    افتراضي:   handle_14XX(regs); اكسر;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

ساكن u16
detect_parport(u16 port, u8 timeout, u8 ژماردن)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    إذا (inb(port) != 0xaa)
        // Not present
        أرجع 0;
    SET_BDA(port_lpt[ژماردن], port);
    SET_BDA(lpt_timeout[ژماردن], timeout);
    أرجع 1;
}

فراغ
lpt_setup(فراغ)
{
    إذا (! CONFIG_LPT)
        أرجع;
    dprintf(3, "init lpt\n");

    u16 ژماردن = 0;
    ژماردن += detect_parport(PORT_LPT1, 0x14, ژماردن);
    ژماردن += detect_parport(PORT_LPT2, 0x14, ژماردن);
    dprintf(1, "Found %d lpt ports\n", ژماردن);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, ژماردن << 14);
}

ساكن u16
getLptAddr(بنية bregs *regs)
{
    إذا (regs->dx >= 3) {
        set_invalid(regs);
        أرجع 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    إذا (! addr)
        set_invalid(regs);
    أرجع addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ساكن فراغ
handle_1700(بنية bregs *regs)
{
    u16 addr = getLptAddr(regs);
    إذا (!addr)
        أرجع;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    لكل (;;) {
        u8 v = inb(addr+1);
        إذا (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            اكسر;
        }
        إذا (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            اكسر;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
ساكن فراغ
handle_1701(بنية bregs *regs)
{
    u16 addr = getLptAddr(regs);
    إذا (!addr)
        أرجع;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ساكن فراغ
handle_1702(بنية bregs *regs)
{
    u16 addr = getLptAddr(regs);
    إذا (!addr)
        أرجع;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

ساكن فراغ
handle_17XX(بنية bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
فراغ VISIBLE16
handle_17(بنية bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    إذا (! CONFIG_LPT) {
        handle_17XX(regs);
        أرجع;
    }

    بدل (regs->ah) {
    حالة 0x00: handle_1700(regs); اكسر;
    حالة 0x01: handle_1701(regs); اكسر;
    حالة 0x02: handle_1702(regs); اكسر;
    افتراضي:   handle_17XX(regs); اكسر;
    }
}
