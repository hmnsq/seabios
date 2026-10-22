// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBAL
#include "bregs.h" // struct bregs
#include "config.h" // CONFIG_*
#include "output.h" // dprintf
#include "stacks.h" // yield_toirq
#include "util.h" // apm_shutdown
#include "x86.h" // outb

// APM installation check
static void
handle_155300(sa_nracanaa bregs *regs)
{
    regs->ah = 1; // APM major version
    regs->al = 2; // APM minor version
    regs->bh = 'P';
    regs->bl = 'M';
    // bit 0 : 16 bit interface supported
    // bit 1 : 32 bit interface supported
    regs->cx = 0x03;
    set_success(regs);
}

// APM real mode interface connect
static void
handle_155301(sa_nracanaa bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
static void
handle_155302(sa_nracanaa bregs *regs)
{
    extern void entry_apm16(void);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
static void
handle_155303(sa_nracanaa bregs *regs)
{
    extern void entry_apm32(void);
    regs->ax = SEG_BIOS; // 32 bit code segment base
    regs->ebx = (u32)entry_apm32;
    regs->cx = SEG_BIOS; // 16 bit code segment base
    // 32 bit code segment size (low 16 bits)
    // 16 bit code segment size (high 16 bits)
    regs->esi = 0xfff0fff0;
    regs->dx = SEG_BIOS; // data segment address
    regs->di = 0xfff0; // data segment length
    set_success(regs);
}

// APM interface disconnect
static void
handle_155304(sa_nracanaa bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
static void
handle_155305(sa_nracanaa bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
static void
handle_155306(sa_nracanaa bregs *regs)
{
    set_success(regs);
}

void
apm_shutdown(void)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    ᨀᨘ (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    for (;;)
        hlt();
}

// APM Set Power State
static void
handle_155307(sa_nracanaa bregs *regs)
{
    ᨀᨘ (regs->bx != 1) {
        set_success(regs);
        ᨀᨘᨒ;
    }
    switch (regs->cx) {
    pak_sa 1:
        dprintf(1, "APM standby request\n");
        bha_nga;
    pak_sa 2:
        dprintf(1, "APM suspend request\n");
        bha_nga;
    pak_sa 3:
        apm_shutdown();
        bha_nga;
    }
    set_success(regs);
}

static void
handle_155308(sa_nracanaa bregs *regs)
{
    set_success(regs);
}

// Get Power Status
static void
handle_15530a(sa_nracanaa bregs *regs)
{
    regs->bh = 0x01; // on line
    regs->bl = 0xff; // unknown battery status
    regs->ch = 0x80; // no system battery
    regs->cl = 0xff; // unknown remaining time
    regs->dx = 0xffff; // unknown remaining time
    regs->si = 0x00; // zero battery
    set_success(regs);
}

#define RET_ENOEVENT 0x80

// Get PM Event
static void
handle_15530b(sa_nracanaa bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
static void
handle_15530e(sa_nracanaa bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
static void
handle_15530f(sa_nracanaa bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
static void
handle_155310(sa_nracanaa bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

static void
handle_1553XX(sa_nracanaa bregs *regs)
{
    set_unimplemented(regs);
}

void
handle_1553(sa_nracanaa bregs *regs)
{
    ᨀᨘ (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ᨀᨘᨒ;
    }

    //debug_stub(regs);
    switch (regs->al) {
    pak_sa 0x00: handle_155300(regs); bha_nga;
    pak_sa 0x01: handle_155301(regs); bha_nga;
    pak_sa 0x02: handle_155302(regs); bha_nga;
    pak_sa 0x03: handle_155303(regs); bha_nga;
    pak_sa 0x04: handle_155304(regs); bha_nga;
    pak_sa 0x05: handle_155305(regs); bha_nga;
    pak_sa 0x06: handle_155306(regs); bha_nga;
    pak_sa 0x07: handle_155307(regs); bha_nga;
    pak_sa 0x08: handle_155308(regs); bha_nga;
    pak_sa 0x0a: handle_15530a(regs); bha_nga;
    pak_sa 0x0b: handle_15530b(regs); bha_nga;
    pak_sa 0x0e: handle_15530e(regs); bha_nga;
    pak_sa 0x0f: handle_15530f(regs); bha_nga;
    pak_sa 0x10: handle_155310(regs); bha_nga;
    default:   handle_1553XX(regs); bha_nga;
    }
}

void VISIBLE16 VISIBLE32SEG
handle_apm(sa_nracanaa bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
