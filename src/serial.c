// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // SET_BDA
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "hw/serialio.h" // SEROFF_IER
#अन्तर्भाव "output.h" // debug_enter
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

स्थिर u16
detect_serial(u16 port, u8 timeout, u8 मोजणी)
{
    जर (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        परत 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    जर (ier != 0x02)
        परत 0;
    u8 iir = inb(port+SEROFF_IIR);
    जर ((iir & 0x3f) != 0x02)
        परत 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[मोजणी], port);
    SET_BDA(com_timeout[मोजणी], timeout);
    परत 1;
}

शून्य
serial_setup(शून्य)
{
    जर (! CONFIG_SERIAL)
        परत;
    dprintf(3, "init serial\n");

    u16 मोजणी = 0;
    मोजणी += detect_serial(PORT_SERIAL1, 0x0a, मोजणी);
    मोजणी += detect_serial(PORT_SERIAL2, 0x0a, मोजणी);
    मोजणी += detect_serial(PORT_SERIAL3, 0x0a, मोजणी);
    मोजणी += detect_serial(PORT_SERIAL4, 0x0a, मोजणी);
    dprintf(1, "Found %d serial ports\n", मोजणी);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, मोजणी << 9);
}

स्थिर u16
getComAddr(संरचना bregs *regs)
{
    जर (regs->dx >= 4) {
        set_invalid(regs);
        परत 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    जर (! addr)
        set_invalid(regs);
    परत addr;
}

// SERIAL - INITIALIZE PORT
स्थिर शून्य
handle_1400(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    जर (!addr)
        परत;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    जर ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } अन्यथा {
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
स्थिर शून्य
handle_1401(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    जर (!addr)
        परत;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    साठी (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        जर ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            थांबा;
        }
        जर (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            थांबा;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
स्थिर शून्य
handle_1402(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    जर (!addr)
        परत;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    साठी (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        जर (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            थांबा;
        }
        जर (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            थांबा;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
स्थिर शून्य
handle_1403(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    जर (!addr)
        परत;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

स्थिर शून्य
handle_14XX(संरचना bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
शून्य VISIBLE16
handle_14(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    जर (! CONFIG_SERIAL) {
        handle_14XX(regs);
        परत;
    }

    निवड (regs->ah) {
    स्थिती 0x00: handle_1400(regs); थांबा;
    स्थिती 0x01: handle_1401(regs); थांबा;
    स्थिती 0x02: handle_1402(regs); थांबा;
    स्थिती 0x03: handle_1403(regs); थांबा;
    पूर्वनिर्धारित:   handle_14XX(regs); थांबा;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

स्थिर u16
detect_parport(u16 port, u8 timeout, u8 मोजणी)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    जर (inb(port) != 0xaa)
        // Not present
        परत 0;
    SET_BDA(port_lpt[मोजणी], port);
    SET_BDA(lpt_timeout[मोजणी], timeout);
    परत 1;
}

शून्य
lpt_setup(शून्य)
{
    जर (! CONFIG_LPT)
        परत;
    dprintf(3, "init lpt\n");

    u16 मोजणी = 0;
    मोजणी += detect_parport(PORT_LPT1, 0x14, मोजणी);
    मोजणी += detect_parport(PORT_LPT2, 0x14, मोजणी);
    dprintf(1, "Found %d lpt ports\n", मोजणी);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, मोजणी << 14);
}

स्थिर u16
getLptAddr(संरचना bregs *regs)
{
    जर (regs->dx >= 3) {
        set_invalid(regs);
        परत 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    जर (! addr)
        set_invalid(regs);
    परत addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
स्थिर शून्य
handle_1700(संरचना bregs *regs)
{
    u16 addr = getLptAddr(regs);
    जर (!addr)
        परत;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    साठी (;;) {
        u8 v = inb(addr+1);
        जर (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            थांबा;
        }
        जर (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            थांबा;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
स्थिर शून्य
handle_1701(संरचना bregs *regs)
{
    u16 addr = getLptAddr(regs);
    जर (!addr)
        परत;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
स्थिर शून्य
handle_1702(संरचना bregs *regs)
{
    u16 addr = getLptAddr(regs);
    जर (!addr)
        परत;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

स्थिर शून्य
handle_17XX(संरचना bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
शून्य VISIBLE16
handle_17(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    जर (! CONFIG_LPT) {
        handle_17XX(regs);
        परत;
    }

    निवड (regs->ah) {
    स्थिती 0x00: handle_1700(regs); थांबा;
    स्थिती 0x01: handle_1701(regs); थांबा;
    स्थिती 0x02: handle_1702(regs); थांबा;
    पूर्वनिर्धारित:   handle_17XX(regs); थांबा;
    }
}
