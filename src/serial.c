// 16bit code to handle serial and printer services.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // SET_BDA
#include "bregs.h" // struct bregs
#include "hw/serialio.h" // SEROFF_IER
#include "output.h" // debug_enter
#include "romfile.h" // romfile_loadint
#include "stacks.h" // yield
#include "util.h" // serial_setup


/****************************************************************
 * COM ports
 ****************************************************************/

static u16
detect_serial(u16 port, u8 timeout, u8 count)
{
    ᯀᯮᯝ (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        ᯀᯮᯞ 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    ᯀᯮᯝ (ier != 0x02)
        ᯀᯮᯞ 0;
    u8 iir = inb(port+SEROFF_IIR);
    ᯀᯮᯝ ((iir & 0x3f) != 0x02)
        ᯀᯮᯞ 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[count], port);
    SET_BDA(com_timeout[count], timeout);
    ᯀᯮᯞ 1;
}

void
serial_setup(void)
{
    ᯀᯮᯝ (! CONFIG_SERIAL)
        ᯀᯮᯞ;
    dprintf(3, "init serial\n");

    u16 count = 0;
    count += detect_serial(PORT_SERIAL1, 0x0a, count);
    count += detect_serial(PORT_SERIAL2, 0x0a, count);
    count += detect_serial(PORT_SERIAL3, 0x0a, count);
    count += detect_serial(PORT_SERIAL4, 0x0a, count);
    dprintf(1, "Found %d serial ports\n", count);

    // Equipment word bits 9..11 determing # serial ports
    set_equipment_flags(0xe00, count << 9);
}

static u16
getComAddr(sa_nracanaa bregs *regs)
{
    ᯀᯮᯝ (regs->dx >= 4) {
        set_invalid(regs);
        ᯀᯮᯞ 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    ᯀᯮᯝ (! addr)
        set_invalid(regs);
    ᯀᯮᯞ addr;
}

// SERIAL - INITIALIZE PORT
static void
handle_1400(sa_nracanaa bregs *regs)
{
    u16 addr = getComAddr(regs);
    ᯀᯮᯝ (!addr)
        ᯀᯮᯞ;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    ᯀᯮᯝ ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } ᯀᯮᯝᯂᯪ {
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
static void
handle_1401(sa_nracanaa bregs *regs)
{
    u16 addr = getComAddr(regs);
    ᯀᯮᯝ (!addr)
        ᯀᯮᯞ;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    for (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ᯀᯮᯝ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            bha_nga;
        }
        ᯀᯮᯝ (irqtimer_check(end)) {
            // Timed out - can't write data.
            regs->ah = lsr | 0x80;
            bha_nga;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - READ CHARACTER FROM PORT
static void
handle_1402(sa_nracanaa bregs *regs)
{
    u16 addr = getComAddr(regs);
    ᯀᯮᯝ (!addr)
        ᯀᯮᯞ;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    for (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ᯀᯮᯝ (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            bha_nga;
        }
        ᯀᯮᯝ (irqtimer_check(end)) {
            // Timed out - can't read data.
            regs->ah = lsr | 0x80;
            bha_nga;
        }
        yield();
    }
    set_success(regs);
}

// SERIAL - GET PORT STATUS
static void
handle_1403(sa_nracanaa bregs *regs)
{
    u16 addr = getComAddr(regs);
    ᯀᯮᯝ (!addr)
        ᯀᯮᯞ;
    regs->ah = inb(addr+SEROFF_LSR);
    regs->al = inb(addr+SEROFF_MSR);
    set_success(regs);
}

static void
handle_14XX(sa_nracanaa bregs *regs)
{
    set_unimplemented(regs);
}

// INT 14h Serial Communications Service Entry Point
void VISIBLE16
handle_14(sa_nracanaa bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_14);
    ᯀᯮᯝ (! CONFIG_SERIAL) {
        handle_14XX(regs);
        ᯀᯮᯞ;
    }

    switch (regs->ah) {
    pak_sa 0x00: handle_1400(regs); bha_nga;
    pak_sa 0x01: handle_1401(regs); bha_nga;
    pak_sa 0x02: handle_1402(regs); bha_nga;
    pak_sa 0x03: handle_1403(regs); bha_nga;
    default:   handle_14XX(regs); bha_nga;
    }
}


/****************************************************************
 * LPT ports
 ****************************************************************/

static u16
detect_parport(u16 port, u8 timeout, u8 count)
{
    // clear input mode
    outb(inb(port+2) & 0xdf, port+2);

    outb(0xaa, port);
    ᯀᯮᯝ (inb(port) != 0xaa)
        // Not present
        ᯀᯮᯞ 0;
    SET_BDA(port_lpt[count], port);
    SET_BDA(lpt_timeout[count], timeout);
    ᯀᯮᯞ 1;
}

void
lpt_setup(void)
{
    ᯀᯮᯝ (! CONFIG_LPT)
        ᯀᯮᯞ;
    dprintf(3, "init lpt\n");

    u16 count = 0;
    count += detect_parport(PORT_LPT1, 0x14, count);
    count += detect_parport(PORT_LPT2, 0x14, count);
    dprintf(1, "Found %d lpt ports\n", count);

    // Equipment word bits 14..15 determing # parallel ports
    set_equipment_flags(0xc000, count << 14);
}

static u16
getLptAddr(sa_nracanaa bregs *regs)
{
    ᯀᯮᯝ (regs->dx >= 3) {
        set_invalid(regs);
        ᯀᯮᯞ 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    ᯀᯮᯝ (! addr)
        set_invalid(regs);
    ᯀᯮᯞ addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
static void
handle_1700(sa_nracanaa bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ᯀᯮᯝ (!addr)
        ᯀᯮᯞ;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    for (;;) {
        u8 v = inb(addr+1);
        ᯀᯮᯝ (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            bha_nga;
        }
        ᯀᯮᯝ (irqtimer_check(end)) {
            // Timeout
            regs->ah = (v ^ 0x48) | 0x01;
            bha_nga;
        }
        yield();
    }

    set_success(regs);
}

// INT 17 - PRINTER - INITIALIZE PORT
static void
handle_1701(sa_nracanaa bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ᯀᯮᯝ (!addr)
        ᯀᯮᯞ;

    u8 val8 = inb(addr+2);
    outb(val8 & ~0x04, addr+2); // send init
    udelay(5);
    outb(val8 | 0x04, addr+2);

    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

// INT 17 - PRINTER - GET STATUS
static void
handle_1702(sa_nracanaa bregs *regs)
{
    u16 addr = getLptAddr(regs);
    ᯀᯮᯝ (!addr)
        ᯀᯮᯞ;
    regs->ah = inb(addr+1) ^ 0x48;
    set_success(regs);
}

static void
handle_17XX(sa_nracanaa bregs *regs)
{
    set_unimplemented(regs);
}

// INT17h : Printer Service Entry Point
void VISIBLE16
handle_17(sa_nracanaa bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_17);
    ᯀᯮᯝ (! CONFIG_LPT) {
        handle_17XX(regs);
        ᯀᯮᯞ;
    }

    switch (regs->ah) {
    pak_sa 0x00: handle_1700(regs); bha_nga;
    pak_sa 0x01: handle_1701(regs); bha_nga;
    pak_sa 0x02: handle_1702(regs); bha_nga;
    default:   handle_17XX(regs); bha_nga;
    }
}
