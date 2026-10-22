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
ساکن خالی
handle_155300(ساخت bregs *regs)
{
    regs->ah = 1; // APM major version
    regs->al = 2; // APM minor version
    regs->bh = 'P';
    regs->bl = 'M';
    // bit 0 : 16 bit interface supported
    // bit 1 : 32 bit interface supported
    regs->cx = 0x03;
    سیٹ_success(regs);
}

// APM real mode interface connect
ساکن خالی
handle_155301(ساخت bregs *regs)
{
    سیٹ_success(regs);
}

// APM 16 bit protected mode interface connect
ساکن خالی
handle_155302(ساخت bregs *regs)
{
    خارجی خالی اندراج_apm16(خالی);
    regs->bx = (u32)اندراج_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    سیٹ_success(regs);
}

// APM 32 bit protected mode interface connect
ساکن خالی
handle_155303(ساخت bregs *regs)
{
    خارجی خالی اندراج_apm32(خالی);
    regs->ax = SEG_BIOS; // 32 bit code segment base
    regs->ebx = (u32)اندراج_apm32;
    regs->cx = SEG_BIOS; // 16 bit code segment base
    // 32 bit code segment size (low 16 bits)
    // 16 bit code segment size (high 16 bits)
    regs->esi = 0xfff0fff0;
    regs->dx = SEG_BIOS; // data segment address
    regs->di = 0xfff0; // data segment length
    سیٹ_success(regs);
}

// APM interface disconnect
ساکن خالی
handle_155304(ساخت bregs *regs)
{
    سیٹ_success(regs);
}

// APM cpu idle
ساکن خالی
handle_155305(ساخت bregs *regs)
{
    yield_toirq();
    سیٹ_success(regs);
}

// APM cpu busy
ساکن خالی
handle_155306(ساخت bregs *regs)
{
    سیٹ_success(regs);
}

خالی
apm_shutdown(خالی)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    اگر (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    برائے (;;)
        hlt();
}

// APM Set Power State
ساکن خالی
handle_155307(ساخت bregs *regs)
{
    اگر (regs->bx != 1) {
        سیٹ_success(regs);
        واپس;
    }
    انتخاب (regs->cx) {
    حالت 1:
        dprintf(1, "APM standby request\n");
        توڑو;
    حالت 2:
        dprintf(1, "APM suspend request\n");
        توڑو;
    حالت 3:
        apm_shutdown();
        توڑو;
    }
    سیٹ_success(regs);
}

ساکن خالی
handle_155308(ساخت bregs *regs)
{
    سیٹ_success(regs);
}

// Get Power Status
ساکن خالی
handle_15530a(ساخت bregs *regs)
{
    regs->bh = 0x01; // on line
    regs->bl = 0xff; // unknown battery status
    regs->ch = 0x80; // no system battery
    regs->cl = 0xff; // unknown remaining time
    regs->dx = 0xffff; // unknown remaining time
    regs->si = 0x00; // zero battery
    سیٹ_success(regs);
}

#تعریف_کرو RET_ENOEVENT 0x80

// Get PM Event
ساکن خالی
handle_15530b(ساخت bregs *regs)
{
    سیٹ_رمز_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
ساکن خالی
handle_15530e(ساخت bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    سیٹ_success(regs);
}

// APM Engage / Disengage
ساکن خالی
handle_15530f(ساخت bregs *regs)
{
    سیٹ_success(regs);
}

// APM Get Capabilities
ساکن خالی
handle_155310(ساخت bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    سیٹ_success(regs);
}

ساکن خالی
handle_1553XX(ساخت bregs *regs)
{
    سیٹ_unimplemented(regs);
}

خالی
handle_1553(ساخت bregs *regs)
{
    اگر (! CONFIG_APMBIOS) {
        سیٹ_رمز_invalid(regs, RET_EUNSUPPORTED);
        واپس;
    }

    //debug_stub(regs);
    انتخاب (regs->al) {
    حالت 0x00: handle_155300(regs); توڑو;
    حالت 0x01: handle_155301(regs); توڑو;
    حالت 0x02: handle_155302(regs); توڑو;
    حالت 0x03: handle_155303(regs); توڑو;
    حالت 0x04: handle_155304(regs); توڑو;
    حالت 0x05: handle_155305(regs); توڑو;
    حالت 0x06: handle_155306(regs); توڑو;
    حالت 0x07: handle_155307(regs); توڑو;
    حالت 0x08: handle_155308(regs); توڑو;
    حالت 0x0a: handle_15530a(regs); توڑو;
    حالت 0x0b: handle_15530b(regs); توڑو;
    حالت 0x0e: handle_15530e(regs); توڑو;
    حالت 0x0f: handle_15530f(regs); توڑو;
    حالت 0x10: handle_155310(regs); توڑو;
    طےشدہ:   handle_1553XX(regs); توڑو;
    }
}

خالی VISIBLE16 VISIBLE32SEG
handle_apm(ساخت bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
