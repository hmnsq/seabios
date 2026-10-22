// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // yield_toirq
#ضمن "util.h" // apm_shutdown
#ضمن "x86.h" // outb

// APM installation check
ساكن فراغ
handle_155300(بنية bregs *regs)
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
ساكن فراغ
handle_155301(بنية bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
ساكن فراغ
handle_155302(بنية bregs *regs)
{
    خارجي فراغ entry_apm16(فراغ);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
ساكن فراغ
handle_155303(بنية bregs *regs)
{
    خارجي فراغ entry_apm32(فراغ);
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
ساكن فراغ
handle_155304(بنية bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
ساكن فراغ
handle_155305(بنية bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
ساكن فراغ
handle_155306(بنية bregs *regs)
{
    set_success(regs);
}

فراغ
apm_shutdown(فراغ)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    إذا (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    لكل (;;)
        hlt();
}

// APM Set Power State
ساكن فراغ
handle_155307(بنية bregs *regs)
{
    إذا (regs->bx != 1) {
        set_success(regs);
        أرجع;
    }
    بدل (regs->cx) {
    حالة 1:
        dprintf(1, "APM standby request\n");
        اكسر;
    حالة 2:
        dprintf(1, "APM suspend request\n");
        اكسر;
    حالة 3:
        apm_shutdown();
        اكسر;
    }
    set_success(regs);
}

ساكن فراغ
handle_155308(بنية bregs *regs)
{
    set_success(regs);
}

// Get Power Status
ساكن فراغ
handle_15530a(بنية bregs *regs)
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
ساكن فراغ
handle_15530b(بنية bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
ساكن فراغ
handle_15530e(بنية bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
ساكن فراغ
handle_15530f(بنية bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
ساكن فراغ
handle_155310(بنية bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

ساكن فراغ
handle_1553XX(بنية bregs *regs)
{
    set_unimplemented(regs);
}

فراغ
handle_1553(بنية bregs *regs)
{
    إذا (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        أرجع;
    }

    //debug_stub(regs);
    بدل (regs->al) {
    حالة 0x00: handle_155300(regs); اكسر;
    حالة 0x01: handle_155301(regs); اكسر;
    حالة 0x02: handle_155302(regs); اكسر;
    حالة 0x03: handle_155303(regs); اكسر;
    حالة 0x04: handle_155304(regs); اكسر;
    حالة 0x05: handle_155305(regs); اكسر;
    حالة 0x06: handle_155306(regs); اكسر;
    حالة 0x07: handle_155307(regs); اكسر;
    حالة 0x08: handle_155308(regs); اكسر;
    حالة 0x0a: handle_15530a(regs); اكسر;
    حالة 0x0b: handle_15530b(regs); اكسر;
    حالة 0x0e: handle_15530e(regs); اكسر;
    حالة 0x0f: handle_15530f(regs); اكسر;
    حالة 0x10: handle_155310(regs); اكسر;
    افتراضي:   handle_1553XX(regs); اكسر;
    }
}

فراغ VISIBLE16 VISIBLE32SEG
handle_apm(بنية bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
