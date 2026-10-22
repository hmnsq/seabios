// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // yield_toirq
#അന്തര്ഭാവ "util.h" // apm_shutdown
#അന്തര്ഭാവ "x86.h" // outb

// APM installation check
സ്ഥിതം ശൂന്യം
handle_155300(ഘടന bregs *regs)
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
സ്ഥിതം ശൂന്യം
handle_155301(ഘടന bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
സ്ഥിതം ശൂന്യം
handle_155302(ഘടന bregs *regs)
{
    ബാഹ്യ ശൂന്യം entry_apm16(ശൂന്യം);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
സ്ഥിതം ശൂന്യം
handle_155303(ഘടന bregs *regs)
{
    ബാഹ്യ ശൂന്യം entry_apm32(ശൂന്യം);
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
സ്ഥിതം ശൂന്യം
handle_155304(ഘടന bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
സ്ഥിതം ശൂന്യം
handle_155305(ഘടന bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
സ്ഥിതം ശൂന്യം
handle_155306(ഘടന bregs *regs)
{
    set_success(regs);
}

ശൂന്യം
apm_shutdown(ശൂന്യം)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    എങ്കിൽ (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    വേണ്ടി (;;)
        hlt();
}

// APM Set Power State
സ്ഥിതം ശൂന്യം
handle_155307(ഘടന bregs *regs)
{
    എങ്കിൽ (regs->bx != 1) {
        set_success(regs);
        തിരികെ;
    }
    തിരഞ്ഞെടുപ്പ് (regs->cx) {
    അവസ്ഥ 1:
        dprintf(1, "APM standby request\n");
        നിർത്തുക;
    അവസ്ഥ 2:
        dprintf(1, "APM suspend request\n");
        നിർത്തുക;
    അവസ്ഥ 3:
        apm_shutdown();
        നിർത്തുക;
    }
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_155308(ഘടന bregs *regs)
{
    set_success(regs);
}

// Get Power Status
സ്ഥിതം ശൂന്യം
handle_15530a(ഘടന bregs *regs)
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
സ്ഥിതം ശൂന്യം
handle_15530b(ഘടന bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
സ്ഥിതം ശൂന്യം
handle_15530e(ഘടന bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
സ്ഥിതം ശൂന്യം
handle_15530f(ഘടന bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
സ്ഥിതം ശൂന്യം
handle_155310(ഘടന bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

സ്ഥിതം ശൂന്യം
handle_1553XX(ഘടന bregs *regs)
{
    set_unimplemented(regs);
}

ശൂന്യം
handle_1553(ഘടന bregs *regs)
{
    എങ്കിൽ (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        തിരികെ;
    }

    //debug_stub(regs);
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x00: handle_155300(regs); നിർത്തുക;
    അവസ്ഥ 0x01: handle_155301(regs); നിർത്തുക;
    അവസ്ഥ 0x02: handle_155302(regs); നിർത്തുക;
    അവസ്ഥ 0x03: handle_155303(regs); നിർത്തുക;
    അവസ്ഥ 0x04: handle_155304(regs); നിർത്തുക;
    അവസ്ഥ 0x05: handle_155305(regs); നിർത്തുക;
    അവസ്ഥ 0x06: handle_155306(regs); നിർത്തുക;
    അവസ്ഥ 0x07: handle_155307(regs); നിർത്തുക;
    അവസ്ഥ 0x08: handle_155308(regs); നിർത്തുക;
    അവസ്ഥ 0x0a: handle_15530a(regs); നിർത്തുക;
    അവസ്ഥ 0x0b: handle_15530b(regs); നിർത്തുക;
    അവസ്ഥ 0x0e: handle_15530e(regs); നിർത്തുക;
    അവസ്ഥ 0x0f: handle_15530f(regs); നിർത്തുക;
    അവസ്ഥ 0x10: handle_155310(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   handle_1553XX(regs); നിർത്തുക;
    }
}

ശൂന്യം VISIBLE16 VISIBLE32SEG
handle_apm(ഘടന bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
