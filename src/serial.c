// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // SET_BDA
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "hw/serialio.h" // SEROFF_IER
#அந்தர்பாவ "output.h" // debug_enter
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

நிலையான u16
detect_serial(u16 port, u8 timeout, u8 மொத்தம்)
{
    எனில் (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        திரும்பு 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    எனில் (ier != 0x02)
        திரும்பு 0;
    u8 iir = inb(port+SEROFF_IIR);
    எனில் ((iir & 0x3f) != 0x02)
        திரும்பு 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[மொத்தம்], port);
    SET_BDA(com_timeout[மொத்தம்], timeout);
    திரும்பு 1;
}

வெறுமை
serial_setup(வெறுமை)
{
    எனில் (! CONFIG_SERIAL)
        திரும்பு;
    dprintf(3, "init serial\n");

    u16 மொத்தம் = 0;
    மொத்தம் += detect_serial(PORT_SERIAL1, 0x0a, மொத்தம்);
    மொத்தம் += detect_serial(PORT_SERIAL2, 0x0a, மொத்தம்);
    மொத்தம் += detect_serial(PORT_SERIAL3, 0x0a, மொத்தம்);
    மொத்தம் += detect_serial(PORT_SERIAL4, 0x0a, மொத்தம்);
    dprintf(1, "Found %d serial ports\n", மொத்தம்);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, மொத்தம் << 9);
}

நிலையான u16
getComAddr(கட்டமைப்பு bregs *regs)
{
    எனில் (regs->dx >= 4) {
        set_invalid(regs);
        திரும்பு 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    எனில் (! addr)
        set_invalid(regs);
    திரும்பு addr;
}

// SERIAL - INITIALIZE PORT
நிலையான வெறுமை
handle_1400(கட்டமைப்பு bregs *regs)
{
    u16 addr = getComAddr(regs);
    எனில் (!addr)
        திரும்பு;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    எனில் ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } இல்லையெனில் {
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
நிலையான வெறுமை
handle_1401(கட்டமைப்பு bregs *regs)
{
    u16 addr = getComAddr(regs);
    எனில் (!addr)
        திரும்பு;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    க்கு (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        எனில் ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            நிறுத்து;
        }
        எனில் (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            நிறுத்து;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
நிலையான வெறுமை
handle_1402(கட்டமைப்பு bregs *regs)
{
    u16 addr = getComAddr(regs);
    எனில் (!addr)
        திரும்பு;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    க்கு (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        எனில் (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            நிறுத்து;
        }
        எனில் (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            நிறுத்து;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
நிலையான வெறுமை
handle_1403(கட்டமைப்பு bregs *regs)
{
    u16 addr = getComAddr(regs);
    எனில் (!addr)
        திரும்பு;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

நிலையான வெறுமை
handle_14XX(கட்டமைப்பு bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
வெறுமை VISIBLE16
handle_14(கட்டமைப்பு bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    எனில் (! CONFIG_SERIAL) {
        handle_14XX(regs);
        திரும்பு;
    }

    தேர்வு (regs->ah) {
    நிலை 0x00: handle_1400(regs); நிறுத்து;
    நிலை 0x01: handle_1401(regs); நிறுத்து;
    நிலை 0x02: handle_1402(regs); நிறுத்து;
    நிலை 0x03: handle_1403(regs); நிறுத்து;
    இயல்பு:   handle_14XX(regs); நிறுத்து;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

நிலையான u16
detect_parport(u16 port, u8 timeout, u8 மொத்தம்)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    எனில் (inb(port) != 0xaa)
        // Not present
        திரும்பு 0;
    SET_BDA(port_lpt[மொத்தம்], port);
    SET_BDA(lpt_timeout[மொத்தம்], timeout);
    திரும்பு 1;
}

வெறுமை
lpt_setup(வெறுமை)
{
    எனில் (! CONFIG_LPT)
        திரும்பு;
    dprintf(3, "init lpt\n");

    u16 மொத்தம் = 0;
    மொத்தம் += detect_parport(PORT_LPT1, 0x14, மொத்தம்);
    மொத்தம் += detect_parport(PORT_LPT2, 0x14, மொத்தம்);
    dprintf(1, "Found %d lpt ports\n", மொத்தம்);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, மொத்தம் << 14);
}

நிலையான u16
getLptAddr(கட்டமைப்பு bregs *regs)
{
    எனில் (regs->dx >= 3) {
        set_invalid(regs);
        திரும்பு 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    எனில் (! addr)
        set_invalid(regs);
    திரும்பு addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
நிலையான வெறுமை
handle_1700(கட்டமைப்பு bregs *regs)
{
    u16 addr = getLptAddr(regs);
    எனில் (!addr)
        திரும்பு;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    க்கு (;;) {
        u8 v = inb(addr+1);
        எனில் (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            நிறுத்து;
        }
        எனில் (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            நிறுத்து;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
நிலையான வெறுமை
handle_1701(கட்டமைப்பு bregs *regs)
{
    u16 addr = getLptAddr(regs);
    எனில் (!addr)
        திரும்பு;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
நிலையான வெறுமை
handle_1702(கட்டமைப்பு bregs *regs)
{
    u16 addr = getLptAddr(regs);
    எனில் (!addr)
        திரும்பு;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

நிலையான வெறுமை
handle_17XX(கட்டமைப்பு bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
வெறுமை VISIBLE16
handle_17(கட்டமைப்பு bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    எனில் (! CONFIG_LPT) {
        handle_17XX(regs);
        திரும்பு;
    }

    தேர்வு (regs->ah) {
    நிலை 0x00: handle_1700(regs); நிறுத்து;
    நிலை 0x01: handle_1701(regs); நிறுத்து;
    நிலை 0x02: handle_1702(regs); நிறுத்து;
    இயல்பு:   handle_17XX(regs); நிறுத்து;
    }
}
