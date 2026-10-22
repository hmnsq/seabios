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
detect_serial(u16 پورٽ, u8 timeout, u8 ڳڻپ)
{
    إذا (CONFIG_DEBUG_SERIAL && پورٽ == CONFIG_DEBUG_SERIAL_پورٽ
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        أرجع 0;
    outb(0x02, پورٽ+SEROFF_IER);
    u8 ier = inb(پورٽ+SEROFF_IER);
    إذا (ier != 0x02)
        أرجع 0;
    u8 iir = inb(پورٽ+SEROFF_IIR);
    إذا ((iir & 0x3f) != 0x02)
        أرجع 0;

    outb(0x00, پورٽ+SEROFF_IER);
    سيٽ_BDA(پورٽ_com[ڳڻپ], پورٽ);
    سيٽ_BDA(com_timeout[ڳڻپ], timeout);
    أرجع 1;
}

فراغ
serial_تياري(فراغ)
{
    إذا (! CONFIG_SERIAL)
        أرجع;
    dprintf(3, "init serial\n");

    u16 ڳڻپ = 0;
    ڳڻپ += detect_serial(پورٽ_SERIAL1, 0x0a, ڳڻپ);
    ڳڻپ += detect_serial(پورٽ_SERIAL2, 0x0a, ڳڻپ);
    ڳڻپ += detect_serial(پورٽ_SERIAL3, 0x0a, ڳڻپ);
    ڳڻپ += detect_serial(پورٽ_SERIAL4, 0x0a, ڳڻپ);
    dprintf(1, "Found %d serial ports\n", ڳڻپ);

    // Equipment word bits 9..11 determing # serial ports
    سيٽ_equipment_flags(0xe00, ڳڻپ << 9);
}

ساكن u16
getComAddr(بنية bregs *regs)
{
    إذا (regs->dx >= 4) {
        سيٽ_invalid(regs);
        أرجع 0;
    }
    u16 addr = GET_BDA(پورٽ_com[regs->dx]);
    إذا (! addr)
        سيٽ_invalid(regs);
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
    سيٽ_success(regs);
}

// SERIAL - WRITE CHARACTER TO PORT
ساكن فراغ
handle_1401(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    u32 پڇاڙي = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_معلومات);
            // XXX - reread lsr?
            regs->ah = lsr;
            اكسر;
        }
        إذا (irqtimer_check(پڇاڙي)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            اكسر;
        }
        yield();
    }
    سيٽ_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
ساكن فراغ
handle_1402(بنية bregs *regs)
{
    u16 addr = getComAddr(regs);
    إذا (!addr)
        أرجع;
    u32 پڇاڙي = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_معلومات);
            regs->ah = lsr;
            اكسر;
        }
        إذا (irqtimer_check(پڇاڙي)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            اكسر;
        }
        yield();
    }
    سيٽ_success(regs);
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
    سيٽ_success(regs);
}

ساكن فراغ
handle_14XX(بنية bregs *regs)
{
    سيٽ_unimplemented(regs);
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
detect_parport(u16 پورٽ, u8 timeout, u8 ڳڻپ)
{
    // clear input mode
    outb(inb(پورٽ+2) & 0xdf, پورٽ+2);

    outb(0xaa, پورٽ);
    إذا (inb(پورٽ) != 0xaa)
        // Not present
        أرجع 0;
    سيٽ_BDA(پورٽ_lpt[ڳڻپ], پورٽ);
    سيٽ_BDA(lpt_timeout[ڳڻپ], timeout);
    أرجع 1;
}

فراغ
lpt_تياري(فراغ)
{
    إذا (! CONFIG_LPT)
        أرجع;
    dprintf(3, "init lpt\n");

    u16 ڳڻپ = 0;
    ڳڻپ += detect_parport(پورٽ_LPT1, 0x14, ڳڻپ);
    ڳڻپ += detect_parport(پورٽ_LPT2, 0x14, ڳڻپ);
    dprintf(1, "Found %d lpt ports\n", ڳڻپ);

    // Equipment word bits 14..15 determing # parallel ports
    سيٽ_equipment_flags(0xc000, ڳڻپ << 14);
}

ساكن u16
getLptAddr(بنية bregs *regs)
{
    إذا (regs->dx >= 3) {
        سيٽ_invalid(regs);
        أرجع 0;
    }
    u16 addr = GET_BDA(پورٽ_lpt[regs->dx]);
    إذا (! addr)
        سيٽ_invalid(regs);
    أرجع addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
ساكن فراغ
handle_1700(بنية bregs *regs)
{
    u16 addr = getLptAddr(regs);
    إذا (!addr)
        أرجع;

    u32 پڇاڙي = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

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
        إذا (irqtimer_check(پڇاڙي)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            اكسر;
        }
        yield();
    }

    سيٽ_success(regs);
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
    سيٽ_success(regs);
}

// INT 17 - PRINTER - GET STATUS
ساكن فراغ
handle_1702(بنية bregs *regs)
{
    u16 addr = getLptAddr(regs);
    إذا (!addr)
        أرجع;
    regs->ah = inb(addr+1) ^ 0x48;
    سيٽ_success(regs);
}

ساكن فراغ
handle_17XX(بنية bregs *regs)
{
    سيٽ_unimplemented(regs);
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
