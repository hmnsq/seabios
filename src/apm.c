// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield_toirq
#ଅନ୍ତର୍ଭାଵ "util.h" // apm_shutdown
#ଅନ୍ତର୍ଭାଵ "x86.h" // outb

// APM installation check
ସ୍ଥିର ଶୂନ୍ୟ
handle_155300(ସଂରଚନା bregs *regs)
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_155301(ସଂରଚନା bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
ସ୍ଥିର ଶୂନ୍ୟ
handle_155302(ସଂରଚନା bregs *regs)
{
    ବାହ୍ୟ ଶୂନ୍ୟ entry_apm16(ଶୂନ୍ୟ);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
ସ୍ଥିର ଶୂନ୍ୟ
handle_155303(ସଂରଚନା bregs *regs)
{
    ବାହ୍ୟ ଶୂନ୍ୟ entry_apm32(ଶୂନ୍ୟ);
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_155304(ସଂରଚନା bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
ସ୍ଥିର ଶୂନ୍ୟ
handle_155305(ସଂରଚନା bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
ସ୍ଥିର ଶୂନ୍ୟ
handle_155306(ସଂରଚନା bregs *regs)
{
    set_success(regs);
}

ଶୂନ୍ୟ
apm_shutdown(ଶୂନ୍ୟ)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    ଯଦି (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    ପାଇଁ (;;)
        hlt();
}

// APM Set Power State
ସ୍ଥିର ଶୂନ୍ୟ
handle_155307(ସଂରଚନା bregs *regs)
{
    ଯଦି (regs->bx != 1) {
        set_success(regs);
        ଫେରନ୍ତୁ;
    }
    ବାଛ (regs->cx) {
    ସ୍ଥିତି 1:
        dprintf(1, "APM standby request\n");
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 2:
        dprintf(1, "APM suspend request\n");
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 3:
        apm_shutdown();
        ଭାଙ୍ଗ;
    }
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_155308(ସଂରଚନା bregs *regs)
{
    set_success(regs);
}

// Get Power Status
ସ୍ଥିର ଶୂନ୍ୟ
handle_15530a(ସଂରଚନା bregs *regs)
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
ସ୍ଥିର ଶୂନ୍ୟ
handle_15530b(ସଂରଚନା bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
ସ୍ଥିର ଶୂନ୍ୟ
handle_15530e(ସଂରଚନା bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
ସ୍ଥିର ଶୂନ୍ୟ
handle_15530f(ସଂରଚନା bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
ସ୍ଥିର ଶୂନ୍ୟ
handle_155310(ସଂରଚନା bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_1553XX(ସଂରଚନା bregs *regs)
{
    set_unimplemented(regs);
}

ଶୂନ୍ୟ
handle_1553(ସଂରଚନା bregs *regs)
{
    ଯଦି (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ଫେରନ୍ତୁ;
    }

    //debug_stub(regs);
    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x00: handle_155300(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: handle_155301(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: handle_155302(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: handle_155303(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x04: handle_155304(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x05: handle_155305(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x06: handle_155306(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x07: handle_155307(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x08: handle_155308(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0a: handle_15530a(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0b: handle_15530b(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0e: handle_15530e(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0f: handle_15530f(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x10: handle_155310(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   handle_1553XX(regs); ଭାଙ୍ଗ;
    }
}

ଶୂନ୍ୟ VISIBLE16 VISIBLE32SEG
handle_apm(ସଂରଚନା bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
