// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "stacks.h" // yield_toirq
#অন্তর্ভাব "util.h" // apm_shutdown
#অন্তর্ভাব "x86.h" // outb

// APM installation check
স্থির শূন্য
handle_155300(সংরচনা bregs *regs)
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
স্থির শূন্য
handle_155301(সংরচনা bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
স্থির শূন্য
handle_155302(সংরচনা bregs *regs)
{
    বাহ্য শূন্য entry_apm16(শূন্য);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
স্থির শূন্য
handle_155303(সংরচনা bregs *regs)
{
    বাহ্য শূন্য entry_apm32(শূন্য);
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
স্থির শূন্য
handle_155304(সংরচনা bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
স্থির শূন্য
handle_155305(সংরচনা bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
স্থির শূন্য
handle_155306(সংরচনা bregs *regs)
{
    set_success(regs);
}

শূন্য
apm_shutdown(শূন্য)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    যদি (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    প্রতি (;;)
        hlt();
}

// APM Set Power State
স্থির শূন্য
handle_155307(সংরচনা bregs *regs)
{
    যদি (regs->bx != 1) {
        set_success(regs);
        প্রত্যাগম;
    }
    বিকল্প (regs->cx) {
    পক্ষ 1:
        dprintf(1, "APM standby request\n");
        ভঙ্গ;
    পক্ষ 2:
        dprintf(1, "APM suspend request\n");
        ভঙ্গ;
    পক্ষ 3:
        apm_shutdown();
        ভঙ্গ;
    }
    set_success(regs);
}

স্থির শূন্য
handle_155308(সংরচনা bregs *regs)
{
    set_success(regs);
}

// Get Power Status
স্থির শূন্য
handle_15530a(সংরচনা bregs *regs)
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
স্থির শূন্য
handle_15530b(সংরচনা bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
স্থির শূন্য
handle_15530e(সংরচনা bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
স্থির শূন্য
handle_15530f(সংরচনা bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
স্থির শূন্য
handle_155310(সংরচনা bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

স্থির শূন্য
handle_1553XX(সংরচনা bregs *regs)
{
    set_unimplemented(regs);
}

শূন্য
handle_1553(সংরচনা bregs *regs)
{
    যদি (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        প্রত্যাগম;
    }

    //debug_stub(regs);
    বিকল্প (regs->al) {
    পক্ষ 0x00: handle_155300(regs); ভঙ্গ;
    পক্ষ 0x01: handle_155301(regs); ভঙ্গ;
    পক্ষ 0x02: handle_155302(regs); ভঙ্গ;
    পক্ষ 0x03: handle_155303(regs); ভঙ্গ;
    পক্ষ 0x04: handle_155304(regs); ভঙ্গ;
    পক্ষ 0x05: handle_155305(regs); ভঙ্গ;
    পক্ষ 0x06: handle_155306(regs); ভঙ্গ;
    পক্ষ 0x07: handle_155307(regs); ভঙ্গ;
    পক্ষ 0x08: handle_155308(regs); ভঙ্গ;
    পক্ষ 0x0a: handle_15530a(regs); ভঙ্গ;
    পক্ষ 0x0b: handle_15530b(regs); ভঙ্গ;
    পক্ষ 0x0e: handle_15530e(regs); ভঙ্গ;
    পক্ষ 0x0f: handle_15530f(regs); ভঙ্গ;
    পক্ষ 0x10: handle_155310(regs); ভঙ্গ;
    স্বভাব:   handle_1553XX(regs); ভঙ্গ;
    }
}

শূন্য VISIBLE16 VISIBLE32SEG
handle_apm(সংরচনা bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
