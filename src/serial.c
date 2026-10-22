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
detect_serial(u16 पत्तनम्, u8 timeout, u8 गणनम्)
{
    यदि (CONFIG_DEBUG_SERIAL && पत्तनम् == CONFIG_DEBUG_SERIAL_पत्तनम्
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        प्रत्यागम 0;
    outb(0x02, पत्तनम्+SEROFF_IER);
    u8 ier = inb(पत्तनम्+SEROFF_IER);
    यदि (ier != 0x02)
        प्रत्यागम 0;
    u8 iir = inb(पत्तनम्+SEROFF_IIR);
    यदि ((iir & 0x3f) != 0x02)
        प्रत्यागम 0;

    outb(0x00, पत्तनम्+SEROFF_IER);
    समुच्चयः_BDA(पत्तनम्_com[गणनम्], पत्तनम्);
    समुच्चयः_BDA(com_timeout[गणनम्], timeout);
    प्रत्यागम 1;
}

शून्य
serial_सज्जता(शून्य)
{
    यदि (! CONFIG_SERIAL)
        प्रत्यागम;
    dprintf(3, "init serial\n");

    u16 गणनम् = 0;
    गणनम् += detect_serial(पत्तनम्_SERIAL1, 0x0a, गणनम्);
    गणनम् += detect_serial(पत्तनम्_SERIAL2, 0x0a, गणनम्);
    गणनम् += detect_serial(पत्तनम्_SERIAL3, 0x0a, गणनम्);
    गणनम् += detect_serial(पत्तनम्_SERIAL4, 0x0a, गणनम्);
    dprintf(1, "Found %d serial ports\n", गणनम्);

    // Equipment word bits 9..11 determing # serial ports
    समुच्चयः_equipment_flags(0xe00, गणनम् << 9);
}

स्थिर u16
getComAddr(संरचना bregs *regs)
{
    यदि (regs->dx >= 4) {
        समुच्चयः_invalid(regs);
        प्रत्यागम 0;
    }
    u16 addr = GET_BDA(पत्तनम्_com[regs->dx]);
    यदि (! addr)
        समुच्चयः_invalid(regs);
    प्रत्यागम addr;
}

// SERIAL - INITIALIZE PORT
स्थिर शून्य
handle_1400(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    यदि (!addr)
        प्रत्यागम;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    यदि ((regs->al & 0xE0) == 0) {
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
    समुच्चयः_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
स्थिर शून्य
handle_1401(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    यदि (!addr)
        प्रत्यागम;
    u32 अन्तः = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    प्रति (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        यदि ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_दत्त);
            // XXX - reread lsr?
            regs->ah = lsr;
            भङ्ग;
        }
        यदि (irqtimer_check(अन्तः)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            भङ्ग;
        }
        yield();
    }
    समुच्चयः_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
स्थिर शून्य
handle_1402(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    यदि (!addr)
        प्रत्यागम;
    u32 अन्तः = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    प्रति (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        यदि (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_दत्त);
            regs->ah = lsr;
            भङ्ग;
        }
        यदि (irqtimer_check(अन्तः)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            भङ्ग;
        }
        yield();
    }
    समुच्चयः_success(regs);
}

// SERIAL - GET PORT STATUS
स्थिर शून्य
handle_1403(संरचना bregs *regs)
{
    u16 addr = getComAddr(regs);
    यदि (!addr)
        प्रत्यागम;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    समुच्चयः_success(regs);
}

स्थिर शून्य
handle_14XX(संरचना bregs *regs)
{
    समुच्चयः_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
शून्य VISIBLE16
handle_14(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    यदि (! CONFIG_SERIAL) {
        handle_14XX(regs);
        प्रत्यागम;
    }

    विकल्प (regs->ah) {
    पक्ष 0x00: handle_1400(regs); भङ्ग;
    पक्ष 0x01: handle_1401(regs); भङ्ग;
    पक्ष 0x02: handle_1402(regs); भङ्ग;
    पक्ष 0x03: handle_1403(regs); भङ्ग;
    स्वभाव:   handle_14XX(regs); भङ्ग;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

स्थिर u16
detect_parport(u16 पत्तनम्, u8 timeout, u8 गणनम्)
{
    // clear input mode
    outb(inb(पत्तनम्+2) & 0xdf, पत्तनम्+2);

    outb(0xaa, पत्तनम्);
    यदि (inb(पत्तनम्) != 0xaa)
        // Not present
        प्रत्यागम 0;
    समुच्चयः_BDA(पत्तनम्_lpt[गणनम्], पत्तनम्);
    समुच्चयः_BDA(lpt_timeout[गणनम्], timeout);
    प्रत्यागम 1;
}

शून्य
lpt_सज्जता(शून्य)
{
    यदि (! CONFIG_LPT)
        प्रत्यागम;
    dprintf(3, "init lpt\n");

    u16 गणनम् = 0;
    गणनम् += detect_parport(पत्तनम्_LPT1, 0x14, गणनम्);
    गणनम् += detect_parport(पत्तनम्_LPT2, 0x14, गणनम्);
    dprintf(1, "Found %d lpt ports\n", गणनम्);

    // Equipment word bits 14..15 determing # parallel ports
    समुच्चयः_equipment_flags(0xc000, गणनम् << 14);
}

स्थिर u16
getLptAddr(संरचना bregs *regs)
{
    यदि (regs->dx >= 3) {
        समुच्चयः_invalid(regs);
        प्रत्यागम 0;
    }
    u16 addr = GET_BDA(पत्तनम्_lpt[regs->dx]);
    यदि (! addr)
        समुच्चयः_invalid(regs);
    प्रत्यागम addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
स्थिर शून्य
handle_1700(संरचना bregs *regs)
{
    u16 addr = getLptAddr(regs);
    यदि (!addr)
        प्रत्यागम;

    u32 अन्तः = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    प्रति (;;) {
        u8 v = inb(addr+1);
        यदि (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            भङ्ग;
        }
        यदि (irqtimer_check(अन्तः)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            भङ्ग;
        }
        yield();
    }

    समुच्चयः_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
स्थिर शून्य
handle_1701(संरचना bregs *regs)
{
    u16 addr = getLptAddr(regs);
    यदि (!addr)
        प्रत्यागम;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    समुच्चयः_success(regs);
}

// INT 17 - PRINTER - GET STATUS
स्थिर शून्य
handle_1702(संरचना bregs *regs)
{
    u16 addr = getLptAddr(regs);
    यदि (!addr)
        प्रत्यागम;
    regs->ah = inb(addr+1) ^ 0x48;
    समुच्चयः_success(regs);
}

स्थिर शून्य
handle_17XX(संरचना bregs *regs)
{
    समुच्चयः_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
शून्य VISIBLE16
handle_17(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    यदि (! CONFIG_LPT) {
        handle_17XX(regs);
        प्रत्यागम;
    }

    विकल्प (regs->ah) {
    पक्ष 0x00: handle_1700(regs); भङ्ग;
    पक्ष 0x01: handle_1701(regs); भङ्ग;
    पक्ष 0x02: handle_1702(regs); भङ्ग;
    स्वभाव:   handle_17XX(regs); भङ्ग;
    }
}
