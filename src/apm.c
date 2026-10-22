// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "output.h" // dprintf
#شامل "stacks.h" // yield_toirq
#شامل "util.h" // apm_shutdown
#شامل "x86.h" // outb

// APM installation check
ایستا تهی
handle_155300(ساختار bregs *regs)
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
ایستا تهی
handle_155301(ساختار bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
ایستا تهی
handle_155302(ساختار bregs *regs)
{
    بیرونی تهی entry_apm16(تهی);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
ایستا تهی
handle_155303(ساختار bregs *regs)
{
    بیرونی تهی entry_apm32(تهی);
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
ایستا تهی
handle_155304(ساختار bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
ایستا تهی
handle_155305(ساختار bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
ایستا تهی
handle_155306(ساختار bregs *regs)
{
    set_success(regs);
}

تهی
apm_shutdown(تهی)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    اگر (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    برای (;;)
        hlt();
}

// APM Set Power State
ایستا تهی
handle_155307(ساختار bregs *regs)
{
    اگر (regs->bx != 1) {
        set_success(regs);
        بازگشت;
    }
    انتخاب (regs->cx) {
    حالت 1:
        dprintf(1, "APM standby request\n");
        بشکن;
    حالت 2:
        dprintf(1, "APM suspend request\n");
        بشکن;
    حالت 3:
        apm_shutdown();
        بشکن;
    }
    set_success(regs);
}

ایستا تهی
handle_155308(ساختار bregs *regs)
{
    set_success(regs);
}

// Get Power Status
ایستا تهی
handle_15530a(ساختار bregs *regs)
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
ایستا تهی
handle_15530b(ساختار bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
ایستا تهی
handle_15530e(ساختار bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
ایستا تهی
handle_15530f(ساختار bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
ایستا تهی
handle_155310(ساختار bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

ایستا تهی
handle_1553XX(ساختار bregs *regs)
{
    set_unimplemented(regs);
}

تهی
handle_1553(ساختار bregs *regs)
{
    اگر (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        بازگشت;
    }

    //debug_stub(regs);
    انتخاب (regs->al) {
    حالت 0x00: handle_155300(regs); بشکن;
    حالت 0x01: handle_155301(regs); بشکن;
    حالت 0x02: handle_155302(regs); بشکن;
    حالت 0x03: handle_155303(regs); بشکن;
    حالت 0x04: handle_155304(regs); بشکن;
    حالت 0x05: handle_155305(regs); بشکن;
    حالت 0x06: handle_155306(regs); بشکن;
    حالت 0x07: handle_155307(regs); بشکن;
    حالت 0x08: handle_155308(regs); بشکن;
    حالت 0x0a: handle_15530a(regs); بشکن;
    حالت 0x0b: handle_15530b(regs); بشکن;
    حالت 0x0e: handle_15530e(regs); بشکن;
    حالت 0x0f: handle_15530f(regs); بشکن;
    حالت 0x10: handle_155310(regs); بشکن;
    پیشفرض:   handle_1553XX(regs); بشکن;
    }
}

تهی VISIBLE16 VISIBLE32SEG
handle_apm(ساختار bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
