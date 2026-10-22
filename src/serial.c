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
detect_serial(u16 پورټ, u8 timeout, u8 شمېر)
{
    إذا (CONFIG_DEBUG_SERIAL && پورټ == CONFIG_DEBUG_SERIAL_پورټ
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        أرجع 0;
    outb(0x02, پورټ+SEROFF_IER);
    u8 ier = inb(پورټ+SEROFF_IER);
    إذا (ier != 0x02)
        أرجع 0;
    u8 iir = inb(پورټ+SEROFF_IIR);
    إذا ((iir & 0x3f) != 0x02)
        أرجع 0;

    outb(0x00, پورټ+SEROFF_IER);
    سېټ_BDA(پورټ_com[شمېر], پورټ);
    سېټ_BDA(com_timeout[شمېر], timeout);
    أرجع 1;
}

فراغ
serial_تیاری(فراغ)
{
    إذا (! CONFIG_SERIAL)
        أرجع;
    dprintf(3, "init serial\n");

    u16 شمېر = 0;
    شمېر += detect_serial(پورټ_SERIAL1, 0x0a, شمېر);
    شمېر += detect_serial(پورټ_SERIAL2, 0x0a, شمېر);
    شمېر += detect_serial(پورټ_SERIAL3, 0x0a, شمېر);
    شمېر += detect_serial(پورټ_SERIAL4, 0x0a, شمېر);
    dprintf(1, "Found %d serial ports\n", شمېر);

    // Equipment word bits 9..11 determing # serial ports
    سېټ_equipment_flags(0xe00, شمېر << 9);
}

ساكن u16
getComAddr(بنية bregs *regs)
{
    إذا (regs->dx >= 4) {
        سېټ_invalid(regs);
        أرجع 0;
    }
    u16 addr = GET_BDA(پورټ_com[regs->dx]);
    إذا (! addr)
        سېټ_invalid(regs);
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
    سېټ_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
ساكن فراغ
handle_1401(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    u32 پای_ټکی = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_معلومات);
            // XXX - reread lsr?
            regs->ah = lsr;
            اكسر;
        }
        إذا (irqtimer_check(پای_ټکی)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            اكسر;
        }
        yield();
    }
    سېټ_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ساكن فراغ
handle_1402(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    u32 پای_ټکی = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_معلومات);
            regs->ah = lsr;
            اكسر;
        }
        إذا (irqtimer_check(پای_ټکی)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            اكسر;
        }
        yield();
    }
    سېټ_success(regs);
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
    سېټ_success(regs);
}

ساكن فراغ
handle_14XX(بنية bregs *regs)
{
    سېټ_unimplemented(regs);
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
detect_parport(u16 پورټ, u8 timeout, u8 شمېر)
{
    // clear input mode
    outb(inb(پورټ+2) & 0xdf, پورټ+2);

    outb(0xaa, پورټ);
    إذا (inb(پورټ) != 0xaa)
        // Not present
        أرجع 0;
    سېټ_BDA(پورټ_lpt[شمېر], پورټ);
    سېټ_BDA(lpt_timeout[شمېر], timeout);
    أرجع 1;
}

فراغ
lpt_تیاری(فراغ)
{
    إذا (! CONFIG_LPT)
        أرجع;
    dprintf(3, "init lpt\n");

    u16 شمېر = 0;
    شمېر += detect_parport(پورټ_LPT1, 0x14, شمېر);
    شمېر += detect_parport(پورټ_LPT2, 0x14, شمېر);
    dprintf(1, "Found %d lpt ports\n", شمېر);

    // Equipment word bits 14..15 determing # parallel ports
    سېټ_equipment_flags(0xc000, شمېر << 14);
}

ساكن u16
getLptAddr(بنية bregs *regs)
{
    إذا (regs->dx >= 3) {
        سېټ_invalid(regs);
        أرجع 0;
    }
    u16 addr = GET_BDA(پورټ_lpt[regs->dx]);
    إذا (! addr)
        سېټ_invalid(regs);
    أرجع addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ساكن فراغ
handle_1700(بنية bregs *regs)
{
    u16 addr = getLptAddr(regs);
    إذا (!addr)
        أرجع;

    u32 پای_ټکی = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

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
        إذا (irqtimer_check(پای_ټکی)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            اكسر;
        }
        yield();
    }

    سېټ_success(regs);
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
    سېټ_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ساكن فراغ
handle_1702(بنية bregs *regs)
{
    u16 addr = getLptAddr(regs);
    إذا (!addr)
        أرجع;
    regs->ah = inb(addr+1) ^ 0x48;
    سېټ_success(regs);
}

ساكن فراغ
handle_17XX(بنية bregs *regs)
{
    سېټ_unimplemented(regs);
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
