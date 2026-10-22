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

ساکن u16
detect_serial(u16 پورٹ, u8 timeout, u8 گنتی)
{
    اگر (CONFIG_DEBUG_SERIAL && پورٹ == CONFIG_DEBUG_SERIAL_پورٹ
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        واپس 0;
    outb(0x02, پورٹ+SEROFF_IER);
    u8 ier = inb(پورٹ+SEROFF_IER);
    اگر (ier != 0x02)
        واپس 0;
    u8 iir = inb(پورٹ+SEROFF_IIR);
    اگر ((iir & 0x3f) != 0x02)
        واپس 0;

    outb(0x00, پورٹ+SEROFF_IER);
    سیٹ_BDA(پورٹ_com[گنتی], پورٹ);
    سیٹ_BDA(com_timeout[گنتی], timeout);
    واپس 1;
}

خالی
serial_تیاری(خالی)
{
    اگر (! CONFIG_SERIAL)
        واپس;
    dprintf(3, "init serial\n");

    u16 گنتی = 0;
    گنتی += detect_serial(پورٹ_SERIAL1, 0x0a, گنتی);
    گنتی += detect_serial(پورٹ_SERIAL2, 0x0a, گنتی);
    گنتی += detect_serial(پورٹ_SERIAL3, 0x0a, گنتی);
    گنتی += detect_serial(پورٹ_SERIAL4, 0x0a, گنتی);
    dprintf(1, "Found %d serial ports\n", گنتی);

    // Equipment word bits 9..11 determing # serial ports
    سیٹ_equipment_flags(0xe00, گنتی << 9);
}

ساکن u16
getComAddr(ساخت bregs *regs)
{
    اگر (regs->dx >= 4) {
        سیٹ_invalid(regs);
        واپس 0;
    }
    u16 addr = GET_BDA(پورٹ_com[regs->dx]);
    اگر (! addr)
        سیٹ_invalid(regs);
    واپس addr;
}

// SERIAL - INITIALIZE PORT
ساکن خالی
handle_1400(ساخت bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        واپس;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    اگر ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } ورنہ {
        u16 val16 = 0x600 >> ((regs->al & 0xE0) >> 5);
        outb(val16 & 0xFF, addr+SEROFF_DLL);
        outb(val16 >> 8, addr+SEROFF_DLH);
    }
    outb(regs->al & 0x1F, addr+SEROFF_LCR);
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    سیٹ_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
ساکن خالی
handle_1401(ساخت bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        واپس;
    u32 انتہا = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    برائے (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        اگر ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_ڈیٹا);
            // XXX - reread lsr?
            regs->ah = lsr;
            توڑو;
        }
        اگر (irqtimer_check(انتہا)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            توڑو;
        }
        yield();
    }
    سیٹ_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ساکن خالی
handle_1402(ساخت bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        واپس;
    u32 انتہا = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    برائے (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        اگر (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_ڈیٹا);
            regs->ah = lsr;
            توڑو;
        }
        اگر (irqtimer_check(انتہا)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            توڑو;
        }
        yield();
    }
    سیٹ_success(regs);
}

// SERIAL - GET PORT STATUS
ساکن خالی
handle_1403(ساخت bregs *regs)
{
    u16 addr = getComAddr(regs);
    اگر (!addr)
        واپس;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    سیٹ_success(regs);
}

ساکن خالی
handle_14XX(ساخت bregs *regs)
{
    سیٹ_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
خالی VISIBLE16
handle_14(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    اگر (! CONFIG_SERIAL) {
        handle_14XX(regs);
        واپس;
    }

    انتخاب (regs->ah) {
    حالت 0x00: handle_1400(regs); توڑو;
    حالت 0x01: handle_1401(regs); توڑو;
    حالت 0x02: handle_1402(regs); توڑو;
    حالت 0x03: handle_1403(regs); توڑو;
    طےشدہ:   handle_14XX(regs); توڑو;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

ساکن u16
detect_parport(u16 پورٹ, u8 timeout, u8 گنتی)
{
    // clear input mode
    outb(inb(پورٹ+2) & 0xdf, پورٹ+2);

    outb(0xaa, پورٹ);
    اگر (inb(پورٹ) != 0xaa)
        // Not present
        واپس 0;
    سیٹ_BDA(پورٹ_lpt[گنتی], پورٹ);
    سیٹ_BDA(lpt_timeout[گنتی], timeout);
    واپس 1;
}

خالی
lpt_تیاری(خالی)
{
    اگر (! CONFIG_LPT)
        واپس;
    dprintf(3, "init lpt\n");

    u16 گنتی = 0;
    گنتی += detect_parport(پورٹ_LPT1, 0x14, گنتی);
    گنتی += detect_parport(پورٹ_LPT2, 0x14, گنتی);
    dprintf(1, "Found %d lpt ports\n", گنتی);

    // Equipment word bits 14..15 determing # parallel ports
    سیٹ_equipment_flags(0xc000, گنتی << 14);
}

ساکن u16
getLptAddr(ساخت bregs *regs)
{
    اگر (regs->dx >= 3) {
        سیٹ_invalid(regs);
        واپس 0;
    }
    u16 addr = GET_BDA(پورٹ_lpt[regs->dx]);
    اگر (! addr)
        سیٹ_invalid(regs);
    واپس addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ساکن خالی
handle_1700(ساخت bregs *regs)
{
    u16 addr = getLptAddr(regs);
    اگر (!addr)
        واپس;

    u32 انتہا = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    برائے (;;) {
        u8 v = inb(addr+1);
        اگر (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            توڑو;
        }
        اگر (irqtimer_check(انتہا)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            توڑو;
        }
        yield();
    }

    سیٹ_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
ساکن خالی
handle_1701(ساخت bregs *regs)
{
    u16 addr = getLptAddr(regs);
    اگر (!addr)
        واپس;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    سیٹ_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ساکن خالی
handle_1702(ساخت bregs *regs)
{
    u16 addr = getLptAddr(regs);
    اگر (!addr)
        واپس;
    regs->ah = inb(addr+1) ^ 0x48;
    سیٹ_success(regs);
}

ساکن خالی
handle_17XX(ساخت bregs *regs)
{
    سیٹ_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
خالی VISIBLE16
handle_17(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    اگر (! CONFIG_LPT) {
        handle_17XX(regs);
        واپس;
    }

    انتخاب (regs->ah) {
    حالت 0x00: handle_1700(regs); توڑو;
    حالت 0x01: handle_1701(regs); توڑو;
    حالت 0x02: handle_1702(regs); توڑو;
    طےشدہ:   handle_17XX(regs); توڑو;
    }
}
