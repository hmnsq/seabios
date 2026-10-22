// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // SET_BDA
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "hw/serialio.h" // SEROFF_IER
#ಅನ್ತರ್ಭಾವ "output.h" // debug_enter
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

ಸ್ಥಿರ u16
detect_serial(u16 ಪೋರ್ಟ್, u8 timeout, u8 ಎಣಿಕೆ_2)
{
    ಆದರೆ (CONFIG_DEBUG_SERIAL && ಪೋರ್ಟ್ == CONFIG_DEBUG_SERIAL_ಪೋರ್ಟ್
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        ಹಿಂತಿರುಗಿ 0;
    outb(0x02, ಪೋರ್ಟ್+SEROFF_IER);
    u8 ier = inb(ಪೋರ್ಟ್+SEROFF_IER);
    ಆದರೆ (ier != 0x02)
        ಹಿಂತಿರುಗಿ 0;
    u8 iir = inb(ಪೋರ್ಟ್+SEROFF_IIR);
    ಆದರೆ ((iir & 0x3f) != 0x02)
        ಹಿಂತಿರುಗಿ 0;

    outb(0x00, ಪೋರ್ಟ್+SEROFF_IER);
    ಗಣ_BDA(ಪೋರ್ಟ್_com[ಎಣಿಕೆ_2], ಪೋರ್ಟ್);
    ಗಣ_BDA(com_timeout[ಎಣಿಕೆ_2], timeout);
    ಹಿಂತಿರುಗಿ 1;
}

ಶೂನ್ಯ
serial_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_SERIAL)
        ಹಿಂತಿರುಗಿ;
    dprintf(3, "init serial\n");

    u16 ಎಣಿಕೆ_2 = 0;
    ಎಣಿಕೆ_2 += detect_serial(ಪೋರ್ಟ್_SERIAL1, 0x0a, ಎಣಿಕೆ_2);
    ಎಣಿಕೆ_2 += detect_serial(ಪೋರ್ಟ್_SERIAL2, 0x0a, ಎಣಿಕೆ_2);
    ಎಣಿಕೆ_2 += detect_serial(ಪೋರ್ಟ್_SERIAL3, 0x0a, ಎಣಿಕೆ_2);
    ಎಣಿಕೆ_2 += detect_serial(ಪೋರ್ಟ್_SERIAL4, 0x0a, ಎಣಿಕೆ_2);
    dprintf(1, "Found %d serial ports\n", ಎಣಿಕೆ_2);

    // Equipment word bits 9..11 determing # serial ports
    ಗಣ_equipment_flags(0xe00, ಎಣಿಕೆ_2 << 9);
}

ಸ್ಥಿರ u16
getComAddr(ರಚನೆ bregs *regs)
{
    ಆದರೆ (regs->dx >= 4) {
        ಗಣ_invalid(regs);
        ಹಿಂತಿರುಗಿ 0;
    }
    u16 addr = GET_BDA(ಪೋರ್ಟ್_com[regs->dx]);
    ಆದರೆ (! addr)
        ಗಣ_invalid(regs);
    ಹಿಂತಿರುಗಿ addr;
}

// SERIAL - INITIALIZE PORT
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1400(ರಚನೆ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    ಆದರೆ ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } ಇಲ್ಲದಿದ್ದರೆ {
        u16 val16 = 0x600 >> ((regs->al & 0xE0) >> 5);
        outb(val16 & 0xFF, addr+SEROFF_DLL);
        outb(val16 >> 8, addr+SEROFF_DLH);
    }
    outb(regs->al & 0x1F, addr+SEROFF_LCR);
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    ಗಣ_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1401(ರಚನೆ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;
    u32 ಕೊನೆ = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    ಗಾಗಿ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ಆದರೆ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_ದತ್ತಾಂಶ);
            // XXX - reread lsr?
            regs->ah = lsr;
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (irqtimer_check(ಕೊನೆ)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            ನಿಲ್ಲಿಸು;
        }
        yield();
    }
    ಗಣ_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1402(ರಚನೆ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;
    u32 ಕೊನೆ = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    ಗಾಗಿ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ಆದರೆ (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_ದತ್ತಾಂಶ);
            regs->ah = lsr;
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (irqtimer_check(ಕೊನೆ)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            ನಿಲ್ಲಿಸು;
        }
        yield();
    }
    ಗಣ_success(regs);
}

// SERIAL - GET PORT STATUS
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1403(ರಚನೆ bregs *regs)
{
    u16 addr = getComAddr(regs);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_14XX(ರಚನೆ bregs *regs)
{
    ಗಣ_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
ಶೂನ್ಯ VISIBLE16
handle_14(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    ಆದರೆ (! CONFIG_SERIAL) {
        handle_14XX(regs);
        ಹಿಂತಿರುಗಿ;
    }

    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x00: handle_1400(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: handle_1401(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: handle_1402(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: handle_1403(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_14XX(regs); ನಿಲ್ಲಿಸು;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

ಸ್ಥಿರ u16
detect_parport(u16 ಪೋರ್ಟ್, u8 timeout, u8 ಎಣಿಕೆ_2)
{
    // clear input mode
    outb(inb(ಪೋರ್ಟ್+2) & 0xdf, ಪೋರ್ಟ್+2);

    outb(0xaa, ಪೋರ್ಟ್);
    ಆದರೆ (inb(ಪೋರ್ಟ್) != 0xaa)
        // Not present
        ಹಿಂತಿರುಗಿ 0;
    ಗಣ_BDA(ಪೋರ್ಟ್_lpt[ಎಣಿಕೆ_2], ಪೋರ್ಟ್);
    ಗಣ_BDA(lpt_timeout[ಎಣಿಕೆ_2], timeout);
    ಹಿಂತಿರುಗಿ 1;
}

ಶೂನ್ಯ
lpt_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_LPT)
        ಹಿಂತಿರುಗಿ;
    dprintf(3, "init lpt\n");

    u16 ಎಣಿಕೆ_2 = 0;
    ಎಣಿಕೆ_2 += detect_parport(ಪೋರ್ಟ್_LPT1, 0x14, ಎಣಿಕೆ_2);
    ಎಣಿಕೆ_2 += detect_parport(ಪೋರ್ಟ್_LPT2, 0x14, ಎಣಿಕೆ_2);
    dprintf(1, "Found %d lpt ports\n", ಎಣಿಕೆ_2);

    // Equipment word bits 14..15 determing # parallel ports
    ಗಣ_equipment_flags(0xc000, ಎಣಿಕೆ_2 << 14);
}

ಸ್ಥಿರ u16
getLptAddr(ರಚನೆ bregs *regs)
{
    ಆದರೆ (regs->dx >= 3) {
        ಗಣ_invalid(regs);
        ಹಿಂತಿರುಗಿ 0;
    }
    u16 addr = GET_BDA(ಪೋರ್ಟ್_lpt[regs->dx]);
    ಆದರೆ (! addr)
        ಗಣ_invalid(regs);
    ಹಿಂತಿರುಗಿ addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1700(ರಚನೆ bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;

    u32 ಕೊನೆ = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    ಗಾಗಿ (;;) {
        u8 v = inb(addr+1);
        ಆದರೆ (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (irqtimer_check(ಕೊನೆ)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            ನಿಲ್ಲಿಸು;
        }
        yield();
    }

    ಗಣ_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1701(ರಚನೆ bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    ಗಣ_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ಸ್ಥಿರ ಶೂನ್ಯ
handle_1702(ರಚನೆ bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;
    regs->ah = inb(addr+1) ^ 0x48;
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_17XX(ರಚನೆ bregs *regs)
{
    ಗಣ_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
ಶೂನ್ಯ VISIBLE16
handle_17(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    ಆದರೆ (! CONFIG_LPT) {
        handle_17XX(regs);
        ಹಿಂತಿರುಗಿ;
    }

    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x00: handle_1700(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: handle_1701(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: handle_1702(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_17XX(regs); ನಿಲ್ಲಿಸು;
    }
}
