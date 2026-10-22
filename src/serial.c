// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // SET_BDA
#समावेश "bregs.h" // struct bregs
#समावेश "hw/serialio.h" // SEROFF_IER
#समावेश "output.h" // debug_enter
#समावेश "romfile.h" // romfile_loadint
#समावेश "stacks.h" // yield
#समावेश "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

अटल u16
detect_serial(u16 port, u8 timeout, u8 गिनती)
{
    अगर (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        वापस 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    अगर (ier != 0x02)
        वापस 0;
    u8 iir = inb(port+SEROFF_IIR);
    अगर ((iir & 0x3f) != 0x02)
        वापस 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[गिनती], port);
    SET_BDA(com_timeout[गिनती], timeout);
    वापस 1;
}

व्योम
serial_setup(व्योम)
{
    अगर (! CONFIG_SERIAL)
        वापस;
    dprintf(3, "init serial\n");

    u16 गिनती = 0;
    गिनती += detect_serial(PORT_SERIAL1, 0x0a, गिनती);
    गिनती += detect_serial(PORT_SERIAL2, 0x0a, गिनती);
    गिनती += detect_serial(PORT_SERIAL3, 0x0a, गिनती);
    गिनती += detect_serial(PORT_SERIAL4, 0x0a, गिनती);
    dprintf(1, "Found %d serial ports\n", गिनती);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, गिनती << 9);
}

अटल u16
getComAddr(काष्ठा bregs *regs)
{
    अगर (regs->dx >= 4) {
        set_invalid(regs);
        वापस 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    अगर (! addr)
        set_invalid(regs);
    वापस addr;
}

// SERIAL - INITIALIZE PORT
अटल व्योम
handle_1400(काष्ठा bregs *regs)
{
    u16 addr = getComAddr(regs);
    अगर (!addr)
        वापस;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    अगर ((regs->al & 0xE0) == 0) {
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
अटल व्योम
handle_1401(काष्ठा bregs *regs)
{
    u16 addr = getComAddr(regs);
    अगर (!addr)
        वापस;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    क्रम (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        अगर ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_तथ्य);
            // XXX - reread lsr?
            regs->ah = lsr;
            अवरोध;
        }
        अगर (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            अवरोध;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
अटल व्योम
handle_1402(काष्ठा bregs *regs)
{
    u16 addr = getComAddr(regs);
    अगर (!addr)
        वापस;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    क्रम (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        अगर (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_तथ्य);
            regs->ah = lsr;
            अवरोध;
        }
        अगर (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            अवरोध;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
अटल व्योम
handle_1403(काष्ठा bregs *regs)
{
    u16 addr = getComAddr(regs);
    अगर (!addr)
        वापस;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

अटल व्योम
handle_14XX(काष्ठा bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
व्योम VISIBLE16
handle_14(काष्ठा bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    अगर (! CONFIG_SERIAL) {
        handle_14XX(regs);
        वापस;
    }

    चयन (regs->ah) {
    हाल 0x00: handle_1400(regs); अवरोध;
    हाल 0x01: handle_1401(regs); अवरोध;
    हाल 0x02: handle_1402(regs); अवरोध;
    हाल 0x03: handle_1403(regs); अवरोध;
    शेष:   handle_14XX(regs); अवरोध;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

अटल u16
detect_parport(u16 port, u8 timeout, u8 गिनती)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    अगर (inb(port) != 0xaa)
        // Not present
        वापस 0;
    SET_BDA(port_lpt[गिनती], port);
    SET_BDA(lpt_timeout[गिनती], timeout);
    वापस 1;
}

व्योम
lpt_setup(व्योम)
{
    अगर (! CONFIG_LPT)
        वापस;
    dprintf(3, "init lpt\n");

    u16 गिनती = 0;
    गिनती += detect_parport(PORT_LPT1, 0x14, गिनती);
    गिनती += detect_parport(PORT_LPT2, 0x14, गिनती);
    dprintf(1, "Found %d lpt ports\n", गिनती);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, गिनती << 14);
}

अटल u16
getLptAddr(काष्ठा bregs *regs)
{
    अगर (regs->dx >= 3) {
        set_invalid(regs);
        वापस 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    अगर (! addr)
        set_invalid(regs);
    वापस addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
अटल व्योम
handle_1700(काष्ठा bregs *regs)
{
    u16 addr = getLptAddr(regs);
    अगर (!addr)
        वापस;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    क्रम (;;) {
        u8 v = inb(addr+1);
        अगर (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            अवरोध;
        }
        अगर (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            अवरोध;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
अटल व्योम
handle_1701(काष्ठा bregs *regs)
{
    u16 addr = getLptAddr(regs);
    अगर (!addr)
        वापस;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
अटल व्योम
handle_1702(काष्ठा bregs *regs)
{
    u16 addr = getLptAddr(regs);
    अगर (!addr)
        वापस;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

अटल व्योम
handle_17XX(काष्ठा bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
व्योम VISIBLE16
handle_17(काष्ठा bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    अगर (! CONFIG_LPT) {
        handle_17XX(regs);
        वापस;
    }

    चयन (regs->ah) {
    हाल 0x00: handle_1700(regs); अवरोध;
    हाल 0x01: handle_1701(regs); अवरोध;
    हाल 0x02: handle_1702(regs); अवरोध;
    शेष:   handle_17XX(regs); अवरोध;
    }
}
