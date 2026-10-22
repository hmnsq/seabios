// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "stacks.h" // yield_toirq
#அந்தர்பாவ "util.h" // apm_shutdown
#அந்தர்பாவ "x86.h" // outb

// APM installation check
நிலையான வெறுமை
handle_155300(கட்டமைப்பு bregs *regs)
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
நிலையான வெறுமை
handle_155301(கட்டமைப்பு bregs *regs)
{
    set_success(regs);
}

// APM 16 bit protected mode interface connect
நிலையான வெறுமை
handle_155302(கட்டமைப்பு bregs *regs)
{
    வெளி வெறுமை entry_apm16(வெறுமை);
    regs->bx = (u32)entry_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    set_success(regs);
}

// APM 32 bit protected mode interface connect
நிலையான வெறுமை
handle_155303(கட்டமைப்பு bregs *regs)
{
    வெளி வெறுமை entry_apm32(வெறுமை);
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
நிலையான வெறுமை
handle_155304(கட்டமைப்பு bregs *regs)
{
    set_success(regs);
}

// APM cpu idle
நிலையான வெறுமை
handle_155305(கட்டமைப்பு bregs *regs)
{
    yield_toirq();
    set_success(regs);
}

// APM cpu busy
நிலையான வெறுமை
handle_155306(கட்டமைப்பு bregs *regs)
{
    set_success(regs);
}

வெறுமை
apm_shutdown(வெறுமை)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    எனில் (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    க்கு (;;)
        hlt();
}

// APM Set Power State
நிலையான வெறுமை
handle_155307(கட்டமைப்பு bregs *regs)
{
    எனில் (regs->bx != 1) {
        set_success(regs);
        திரும்பு;
    }
    தேர்வு (regs->cx) {
    நிலை 1:
        dprintf(1, "APM standby request\n");
        நிறுத்து;
    நிலை 2:
        dprintf(1, "APM suspend request\n");
        நிறுத்து;
    நிலை 3:
        apm_shutdown();
        நிறுத்து;
    }
    set_success(regs);
}

நிலையான வெறுமை
handle_155308(கட்டமைப்பு bregs *regs)
{
    set_success(regs);
}

// Get Power Status
நிலையான வெறுமை
handle_15530a(கட்டமைப்பு bregs *regs)
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
நிலையான வெறுமை
handle_15530b(கட்டமைப்பு bregs *regs)
{
    set_code_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
நிலையான வெறுமை
handle_15530e(கட்டமைப்பு bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    set_success(regs);
}

// APM Engage / Disengage
நிலையான வெறுமை
handle_15530f(கட்டமைப்பு bregs *regs)
{
    set_success(regs);
}

// APM Get Capabilities
நிலையான வெறுமை
handle_155310(கட்டமைப்பு bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    set_success(regs);
}

நிலையான வெறுமை
handle_1553XX(கட்டமைப்பு bregs *regs)
{
    set_unimplemented(regs);
}

வெறுமை
handle_1553(கட்டமைப்பு bregs *regs)
{
    எனில் (! CONFIG_APMBIOS) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        திரும்பு;
    }

    //debug_stub(regs);
    தேர்வு (regs->al) {
    நிலை 0x00: handle_155300(regs); நிறுத்து;
    நிலை 0x01: handle_155301(regs); நிறுத்து;
    நிலை 0x02: handle_155302(regs); நிறுத்து;
    நிலை 0x03: handle_155303(regs); நிறுத்து;
    நிலை 0x04: handle_155304(regs); நிறுத்து;
    நிலை 0x05: handle_155305(regs); நிறுத்து;
    நிலை 0x06: handle_155306(regs); நிறுத்து;
    நிலை 0x07: handle_155307(regs); நிறுத்து;
    நிலை 0x08: handle_155308(regs); நிறுத்து;
    நிலை 0x0a: handle_15530a(regs); நிறுத்து;
    நிலை 0x0b: handle_15530b(regs); நிறுத்து;
    நிலை 0x0e: handle_15530e(regs); நிறுத்து;
    நிலை 0x0f: handle_15530f(regs); நிறுத்து;
    நிலை 0x10: handle_155310(regs); நிறுத்து;
    இயல்பு:   handle_1553XX(regs); நிறுத்து;
    }
}

வெறுமை VISIBLE16 VISIBLE32SEG
handle_apm(கட்டமைப்பு bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
