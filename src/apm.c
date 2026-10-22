// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "bregs.h" // struct bregs
#समावेश "config.h" // CONFIG_*
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // yield_toirq
#समावेश "util.h" // apm_shutdown
#समावेश "x86.h" // outb

// APM installation check
अटल व्योम
handle_155300(काष्ठा bregs *regs)
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
अटल व्योम
handle_155301(काष्ठा bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
अटल व्योम
handle_155302(काष्ठा bregs *regs)
{
    बाह्य व्योम entry_apm16(व्योम);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
अटल व्योम
handle_155303(काष्ठा bregs *regs)
{
    बाह्य व्योम entry_apm32(व्योम);
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
अटल व्योम
handle_155304(काष्ठा bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
अटल व्योम
handle_155305(काष्ठा bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
अटल व्योम
handle_155306(काष्ठा bregs *regs)
{
    set_success(regs);
}

व्योम
apm_shutdown(व्योम)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    अगर (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    क्रम (;;)
        hlt();
}

// APM Set Power State
अटल व्योम
handle_155307(काष्ठा bregs *regs)
{
    अगर (regs->bx != 1) {
        set_success(regs);
        वापस;
    }
    चयन (regs->cx) {
    हाल 1:
        dprintf(1, "APM standby request\n");
        अवरोध;
    हाल 2:
        dprintf(1, "APM suspend request\n");
        अवरोध;
    हाल 3:
        apm_shutdown();
        अवरोध;
    }
    set_success(regs);
}

अटल व्योम
handle_155308(काष्ठा bregs *regs)
{
    set_success(regs);
}

// Get Power Status
अटल व्योम
handle_15530a(काष्ठा bregs *regs)
{
    regs->bh = 0x01; // on line
    regs->bl = 0xff; // unknown battery status
    regs->ch = 0x80; // no system battery
    regs->cl = 0xff; // unknown remaining time
    regs->dx = 0xffff; // unknown remaining time
    regs->si = 0x00; // zero battery
    set_success(regs);
}

#घोषणा RET_ENOEVENT 0x80

// Get PM Event
अटल व्योम
handle_15530b(काष्ठा bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
अटल व्योम
handle_15530e(काष्ठा bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
अटल व्योम
handle_15530f(काष्ठा bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
अटल व्योम
handle_155310(काष्ठा bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

अटल व्योम
handle_1553XX(काष्ठा bregs *regs)
{
    set_unimplemented(regs);
}

व्योम
handle_1553(काष्ठा bregs *regs)
{
    अगर (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        वापस;
    }

    //debug_stub(regs);
    चयन (regs->al) {
    हाल 0x00: handle_155300(regs); अवरोध;
    हाल 0x01: handle_155301(regs); अवरोध;
    हाल 0x02: handle_155302(regs); अवरोध;
    हाल 0x03: handle_155303(regs); अवरोध;
    हाल 0x04: handle_155304(regs); अवरोध;
    हाल 0x05: handle_155305(regs); अवरोध;
    हाल 0x06: handle_155306(regs); अवरोध;
    हाल 0x07: handle_155307(regs); अवरोध;
    हाल 0x08: handle_155308(regs); अवरोध;
    हाल 0x0a: handle_15530a(regs); अवरोध;
    हाल 0x0b: handle_15530b(regs); अवरोध;
    हाल 0x0e: handle_15530e(regs); अवरोध;
    हाल 0x0f: handle_15530f(regs); अवरोध;
    हाल 0x10: handle_155310(regs); अवरोध;
    शेष:   handle_1553XX(regs); अवरोध;
    }
}

व्योम VISIBLE16 VISIBLE32SEG
handle_apm(काष्ठा bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
