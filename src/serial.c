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
    གལ་སྲིད (CONFIG_DEBUG_SERIAL && port == CONFIG_DEBUG_SERIAL_PORT
        && !romfile_loadint("etc/advertise-serial-debug-port", 1))
        ལོག 0;
    outb(0x02, port+SEROFF_IER);
    u8 ier = inb(port+SEROFF_IER);
    གལ་སྲིད (ier != 0x02)
        ལོག 0;
    u8 iir = inb(port+SEROFF_IIR);
    གལ་སྲིད ((iir & 0x3f) != 0x02)
        ལོག 0;

    outb(0x00, port+SEROFF_IER);
    SET_BDA(port_com[count], port);
    SET_BDA(com_timeout[count], timeout);
    ལོག 1;
}

void
serial_setup(void)
{
    གལ་སྲིད (! CONFIG_SERIAL)
        ལོག;
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
    གལ་སྲིད (regs->dx >= 4) {
        set_invalid(regs);
        ལོག 0;
    }
    u16 addr = GET_BDA(port_com[regs->dx]);
    གལ་སྲིད (! addr)
        set_invalid(regs);
    ལོག addr;
}

// SERIAL - INITIALIZE PORT
static void
handle_1400(sa_nracanaa bregs *regs)
{
    u16 addr = getComAddr(regs);
    གལ་སྲིད (!addr)
        ལོག;
    outb(inb(addr+SEROFF_LCR) | 0x80, addr+SEROFF_LCR);
    གལ་སྲིད ((regs->al & 0xE0) == 0) {
        outb(0x17, addr+SEROFF_DLL);
        outb(0x04, addr+SEROFF_DLH);
    } དེ་མིན {
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
    གལ་སྲིད (!addr)
        ལོག;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    དོན་ལུ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        གལ་སྲིད ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(regs->al, addr+SEROFF_DATA);
            // XXX - reread lsr?
            regs->ah = lsr;
            bha_nga;
        }
        གལ་སྲིད (irqtimer_check(end)) {
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
    གལ་སྲིད (!addr)
        ལོག;
    u32 end = irqtimer_calc_ticks(GET_BDA(com_timeout[regs->dx]));
    དོན་ལུ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        གལ་སྲིད (lsr & 0x01) {
            // Success - can read data
            regs->al = inb(addr+SEROFF_DATA);
            regs->ah = lsr;
            bha_nga;
        }
        གལ་སྲིད (irqtimer_check(end)) {
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
    གལ་སྲིད (!addr)
        ལོག;
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
    གལ་སྲིད (! CONFIG_SERIAL) {
        handle_14XX(regs);
        ལོག;
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
    གལ་སྲིད (inb(port) != 0xaa)
        // Not present
        ལོག 0;
    SET_BDA(port_lpt[count], port);
    SET_BDA(lpt_timeout[count], timeout);
    ལོག 1;
}

void
lpt_setup(void)
{
    གལ་སྲིད (! CONFIG_LPT)
        ལོག;
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
    གལ་སྲིད (regs->dx >= 3) {
        set_invalid(regs);
        ལོག 0;
    }
    u16 addr = GET_BDA(port_lpt[regs->dx]);
    གལ་སྲིད (! addr)
        set_invalid(regs);
    ལོག addr;
}

// INT 17 - PRINTER - WRITE CHARACTER
static void
handle_1700(sa_nracanaa bregs *regs)
{
    u16 addr = getLptAddr(regs);
    གལ་སྲིད (!addr)
        ལོག;

    u32 end = irqtimer_calc_ticks(GET_BDA(lpt_timeout[regs->dx]));

    outb(regs->al, addr);
    u8 val8 = inb(addr+2);
    outb(val8 | 0x01, addr+2); // send strobe
    udelay(5);
    outb(val8 & ~0x01, addr+2);

    དོན་ལུ (;;) {
        u8 v = inb(addr+1);
        གལ་སྲིད (!(v & 0x40)) {
            // Success
            regs->ah = v ^ 0x48;
            bha_nga;
        }
        གལ་སྲིད (irqtimer_check(end)) {
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
    གལ་སྲིད (!addr)
        ལོག;

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
    གལ་སྲིད (!addr)
        ལོག;
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
    གལ་སྲིད (! CONFIG_LPT) {
        handle_17XX(regs);
        ལོག;
    }

    switch (regs->ah) {
    pak_sa 0x00: handle_1700(regs); bha_nga;
    pak_sa 0x01: handle_1701(regs); bha_nga;
    pak_sa 0x02: handle_1702(regs); bha_nga;
    default:   handle_17XX(regs); bha_nga;
    }
}
