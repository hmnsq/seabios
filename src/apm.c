// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield_toirq
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // apm_shutdown
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outb

// APM installation check
ਸਥਿਰ ਖਾਲੀ
handle_155300(ਬਣਤਰ bregs *regs)
{
    regs->ah = 1; // APM major version
    regs->al = 2; // APM minor version
    regs->bh = 'P';
    regs->bl = 'M';
    // bit 0 : 16 bit interface supported
    // bit 1 : 32 bit interface supported
    regs->cx = 0x03;
    ਸੈੱਟ_success(regs);
}

// APM real mode interface connect
ਸਥਿਰ ਖਾਲੀ
handle_155301(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_success(regs);
}

// APM 16 bit protected mode interface connect
ਸਥਿਰ ਖਾਲੀ
handle_155302(ਬਣਤਰ bregs *regs)
{
    ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_apm16(ਖਾਲੀ);
    regs->bx = (u32)ਇੰਦਰਾਜ_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    ਸੈੱਟ_success(regs);
}

// APM 32 bit protected mode interface connect
ਸਥਿਰ ਖਾਲੀ
handle_155303(ਬਣਤਰ bregs *regs)
{
    ਬਾਹਰੀ ਖਾਲੀ ਇੰਦਰਾਜ_apm32(ਖਾਲੀ);
    regs->ax = SEG_BIOS; // 32 bit code segment base
    regs->ebx = (u32)ਇੰਦਰਾਜ_apm32;
    regs->cx = SEG_BIOS; // 16 bit code segment base
    // 32 bit code segment size (low 16 bits)
    // 16 bit code segment size (high 16 bits)
    regs->esi = 0xfff0fff0;
    regs->dx = SEG_BIOS; // data segment address
    regs->di = 0xfff0; // data segment length
    ਸੈੱਟ_success(regs);
}

// APM interface disconnect
ਸਥਿਰ ਖਾਲੀ
handle_155304(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_success(regs);
}

// APM cpu idle
ਸਥਿਰ ਖਾਲੀ
handle_155305(ਬਣਤਰ bregs *regs)
{
    yield_toirq();
    ਸੈੱਟ_success(regs);
}

// APM cpu busy
ਸਥਿਰ ਖਾਲੀ
handle_155306(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_success(regs);
}

ਖਾਲੀ
apm_shutdown(ਖਾਲੀ)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    ਜੇ (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    ਲਈ (;;)
        hlt();
}

// APM Set Power State
ਸਥਿਰ ਖਾਲੀ
handle_155307(ਬਣਤਰ bregs *regs)
{
    ਜੇ (regs->bx != 1) {
        ਸੈੱਟ_success(regs);
        ਵਾਪਸ;
    }
    ਚੋਣ (regs->cx) {
    ਹਾਲਤ 1:
        dprintf(1, "APM standby request\n");
        ਰੋਕੋ;
    ਹਾਲਤ 2:
        dprintf(1, "APM suspend request\n");
        ਰੋਕੋ;
    ਹਾਲਤ 3:
        apm_shutdown();
        ਰੋਕੋ;
    }
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_155308(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_success(regs);
}

// Get Power Status
ਸਥਿਰ ਖਾਲੀ
handle_15530a(ਬਣਤਰ bregs *regs)
{
    regs->bh = 0x01; // on line
    regs->bl = 0xff; // unknown battery status
    regs->ch = 0x80; // no system battery
    regs->cl = 0xff; // unknown remaining time
    regs->dx = 0xffff; // unknown remaining time
    regs->si = 0x00; // zero battery
    ਸੈੱਟ_success(regs);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_ENOEVENT 0x80

// Get PM Event
ਸਥਿਰ ਖਾਲੀ
handle_15530b(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
ਸਥਿਰ ਖਾਲੀ
handle_15530e(ਬਣਤਰ bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    ਸੈੱਟ_success(regs);
}

// APM Engage / Disengage
ਸਥਿਰ ਖਾਲੀ
handle_15530f(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_success(regs);
}

// APM Get Capabilities
ਸਥਿਰ ਖਾਲੀ
handle_155310(ਬਣਤਰ bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    ਸੈੱਟ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
handle_1553XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_unimplemented(regs);
}

ਖਾਲੀ
handle_1553(ਬਣਤਰ bregs *regs)
{
    ਜੇ (! CONFIG_APMBIOS) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EUNSUPPORTED);
        ਵਾਪਸ;
    }

    //debug_stub(regs);
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x00: handle_155300(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: handle_155301(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: handle_155302(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x03: handle_155303(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x04: handle_155304(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x05: handle_155305(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x06: handle_155306(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x07: handle_155307(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x08: handle_155308(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0a: handle_15530a(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0b: handle_15530b(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0e: handle_15530e(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0f: handle_15530f(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x10: handle_155310(regs); ਰੋਕੋ;
    ਮੂਲ:   handle_1553XX(regs); ਰੋਕੋ;
    }
}

ਖਾਲੀ VISIBLE16 VISIBLE32SEG
handle_apm(ਬਣਤਰ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
