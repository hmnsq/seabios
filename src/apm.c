// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // yield_toirq
#అన్తర్భావ "util.h" // apm_shutdown
#అన్తర్భావ "x86.h" // outb

// APM installation check
స్థిర శూన్య
handle_155300(నిర్మాణం bregs *regs)
{
    regs->ah = 1; // APM major version
    regs->al = 2; // APM minor version
    regs->bh = 'P';
    regs->bl = 'M';
    // bit 0 : 16 bit interface supported
    // bit 1 : 32 bit interface supported
    regs->cx = 0x03;
    సమితి_success(regs);
}

// APM real mode interface connect
స్థిర శూన్య
handle_155301(నిర్మాణం bregs *regs)
{
    సమితి_success(regs);
}

// APM 16 bit protected mode interface connect
స్థిర శూన్య
handle_155302(నిర్మాణం bregs *regs)
{
    బాహ్య శూన్య నమోదు_apm16(శూన్య);
    regs->bx = (u32)నమోదు_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    సమితి_success(regs);
}

// APM 32 bit protected mode interface connect
స్థిర శూన్య
handle_155303(నిర్మాణం bregs *regs)
{
    బాహ్య శూన్య నమోదు_apm32(శూన్య);
    regs->ax = SEG_BIOS; // 32 bit code segment base
    regs->ebx = (u32)నమోదు_apm32;
    regs->cx = SEG_BIOS; // 16 bit code segment base
    // 32 bit code segment size (low 16 bits)
    // 16 bit code segment size (high 16 bits)
    regs->esi = 0xfff0fff0;
    regs->dx = SEG_BIOS; // data segment address
    regs->di = 0xfff0; // data segment length
    సమితి_success(regs);
}

// APM interface disconnect
స్థిర శూన్య
handle_155304(నిర్మాణం bregs *regs)
{
    సమితి_success(regs);
}

// APM cpu idle
స్థిర శూన్య
handle_155305(నిర్మాణం bregs *regs)
{
    yield_toirq();
    సమితి_success(regs);
}

// APM cpu busy
స్థిర శూన్య
handle_155306(నిర్మాణం bregs *regs)
{
    సమితి_success(regs);
}

శూన్య
apm_shutdown(శూన్య)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    ఒకవేళ (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    కొరకు (;;)
        hlt();
}

// APM Set Power State
స్థిర శూన్య
handle_155307(నిర్మాణం bregs *regs)
{
    ఒకవేళ (regs->bx != 1) {
        సమితి_success(regs);
        తిరిగి;
    }
    ఎంపిక (regs->cx) {
    స్థితి 1:
        dprintf(1, "APM standby request\n");
        ఆపు;
    స్థితి 2:
        dprintf(1, "APM suspend request\n");
        ఆపు;
    స్థితి 3:
        apm_shutdown();
        ఆపు;
    }
    సమితి_success(regs);
}

స్థిర శూన్య
handle_155308(నిర్మాణం bregs *regs)
{
    సమితి_success(regs);
}

// Get Power Status
స్థిర శూన్య
handle_15530a(నిర్మాణం bregs *regs)
{
    regs->bh = 0x01; // on line
    regs->bl = 0xff; // unknown battery status
    regs->ch = 0x80; // no system battery
    regs->cl = 0xff; // unknown remaining time
    regs->dx = 0xffff; // unknown remaining time
    regs->si = 0x00; // zero battery
    సమితి_success(regs);
}

#నిర్వచించు RET_ENOEVENT 0x80

// Get PM Event
స్థిర శూన్య
handle_15530b(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
స్థిర శూన్య
handle_15530e(నిర్మాణం bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    సమితి_success(regs);
}

// APM Engage / Disengage
స్థిర శూన్య
handle_15530f(నిర్మాణం bregs *regs)
{
    సమితి_success(regs);
}

// APM Get Capabilities
స్థిర శూన్య
handle_155310(నిర్మాణం bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    సమితి_success(regs);
}

స్థిర శూన్య
handle_1553XX(నిర్మాణం bregs *regs)
{
    సమితి_unimplemented(regs);
}

శూన్య
handle_1553(నిర్మాణం bregs *regs)
{
    ఒకవేళ (! CONFIG_APMBIOS) {
        సమితి_సంకేతం_invalid(regs, RET_EUNSUPPORTED);
        తిరిగి;
    }

    //debug_stub(regs);
    ఎంపిక (regs->al) {
    స్థితి 0x00: handle_155300(regs); ఆపు;
    స్థితి 0x01: handle_155301(regs); ఆపు;
    స్థితి 0x02: handle_155302(regs); ఆపు;
    స్థితి 0x03: handle_155303(regs); ఆపు;
    స్థితి 0x04: handle_155304(regs); ఆపు;
    స్థితి 0x05: handle_155305(regs); ఆపు;
    స్థితి 0x06: handle_155306(regs); ఆపు;
    స్థితి 0x07: handle_155307(regs); ఆపు;
    స్థితి 0x08: handle_155308(regs); ఆపు;
    స్థితి 0x0a: handle_15530a(regs); ఆపు;
    స్థితి 0x0b: handle_15530b(regs); ఆపు;
    స్థితి 0x0e: handle_15530e(regs); ఆపు;
    స్థితి 0x0f: handle_15530f(regs); ఆపు;
    స్థితి 0x10: handle_155310(regs); ఆపు;
    అప్రమేయ:   handle_1553XX(regs); ఆపు;
    }
}

శూన్య VISIBLE16 VISIBLE32SEG
handle_apm(నిర్మాణం bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
