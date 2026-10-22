// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // yield_toirq
#અન્તર્ભાવ "util.h" // apm_shutdown
#અન્તર્ભાવ "x86.h" // outb

// APM installation check
સ્થિર શૂન્ય
handle_155300(સંરચના bregs *regs)
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
સ્થિર શૂન્ય
handle_155301(સંરચના bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
સ્થિર શૂન્ય
handle_155302(સંરચના bregs *regs)
{
    બાહ્ય શૂન્ય entry_apm16(શૂન્ય);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
સ્થિર શૂન્ય
handle_155303(સંરચના bregs *regs)
{
    બાહ્ય શૂન્ય entry_apm32(શૂન્ય);
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
સ્થિર શૂન્ય
handle_155304(સંરચના bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
સ્થિર શૂન્ય
handle_155305(સંરચના bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
સ્થિર શૂન્ય
handle_155306(સંરચના bregs *regs)
{
    set_success(regs);
}

શૂન્ય
apm_shutdown(શૂન્ય)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    જો (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    માટે (;;)
        hlt();
}

// APM Set Power State
સ્થિર શૂન્ય
handle_155307(સંરચના bregs *regs)
{
    જો (regs->bx != 1) {
        set_success(regs);
        પાછા;
    }
    પસંદગી (regs->cx) {
    સ્થિતિ 1:
        dprintf(1, "APM standby request\n");
        તોડો;
    સ્થિતિ 2:
        dprintf(1, "APM suspend request\n");
        તોડો;
    સ્થિતિ 3:
        apm_shutdown();
        તોડો;
    }
    set_success(regs);
}

સ્થિર શૂન્ય
handle_155308(સંરચના bregs *regs)
{
    set_success(regs);
}

// Get Power Status
સ્થિર શૂન્ય
handle_15530a(સંરચના bregs *regs)
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
સ્થિર શૂન્ય
handle_15530b(સંરચના bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
સ્થિર શૂન્ય
handle_15530e(સંરચના bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
સ્થિર શૂન્ય
handle_15530f(સંરચના bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
સ્થિર શૂન્ય
handle_155310(સંરચના bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

સ્થિર શૂન્ય
handle_1553XX(સંરચના bregs *regs)
{
    set_unimplemented(regs);
}

શૂન્ય
handle_1553(સંરચના bregs *regs)
{
    જો (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        પાછા;
    }

    //debug_stub(regs);
    પસંદગી (regs->al) {
    સ્થિતિ 0x00: handle_155300(regs); તોડો;
    સ્થિતિ 0x01: handle_155301(regs); તોડો;
    સ્થિતિ 0x02: handle_155302(regs); તોડો;
    સ્થિતિ 0x03: handle_155303(regs); તોડો;
    સ્થિતિ 0x04: handle_155304(regs); તોડો;
    સ્થિતિ 0x05: handle_155305(regs); તોડો;
    સ્થિતિ 0x06: handle_155306(regs); તોડો;
    સ્થિતિ 0x07: handle_155307(regs); તોડો;
    સ્થિતિ 0x08: handle_155308(regs); તોડો;
    સ્થિતિ 0x0a: handle_15530a(regs); તોડો;
    સ્થિતિ 0x0b: handle_15530b(regs); તોડો;
    સ્થિતિ 0x0e: handle_15530e(regs); તોડો;
    સ્થિતિ 0x0f: handle_15530f(regs); તોડો;
    સ્થિતિ 0x10: handle_155310(regs); તોડો;
    મૂળભૂત:   handle_1553XX(regs); તોડો;
    }
}

શૂન્ય VISIBLE16 VISIBLE32SEG
handle_apm(સંરચના bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
