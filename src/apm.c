// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // yield_toirq
#अन्तर्भाव "util.h" // apm_shutdown
#अन्तर्भाव "x86.h" // outb

// APM installation check
स्थिर शून्य
handle_155300(संरचना bregs *regs)
{
    regs->ah = 1; // APM major version
    regs->al = 2; // APM minor version
    regs->bh = 'P';
    regs->bl = 'M';
    // bit 0 : 16 bit interface supported
    // bit 1 : 32 bit interface supported
    regs->cx = 0x03;
    समुच्चयः_success(regs);
}

// APM real mode interface connect
स्थिर शून्य
handle_155301(संरचना bregs *regs)
{
    समुच्चयः_success(regs);
}

// APM 16 bit protected mode interface connect
स्थिर शून्य
handle_155302(संरचना bregs *regs)
{
    बाह्य शून्य प्रविष्टिः_apm16(शून्य);
    regs->bx = (u32)प्रविष्टिः_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    समुच्चयः_success(regs);
}

// APM 32 bit protected mode interface connect
स्थिर शून्य
handle_155303(संरचना bregs *regs)
{
    बाह्य शून्य प्रविष्टिः_apm32(शून्य);
    regs->ax = SEG_BIOS; // 32 bit code segment base
    regs->ebx = (u32)प्रविष्टिः_apm32;
    regs->cx = SEG_BIOS; // 16 bit code segment base
    // 32 bit code segment size (low 16 bits)
    // 16 bit code segment size (high 16 bits)
    regs->esi = 0xfff0fff0;
    regs->dx = SEG_BIOS; // data segment address
    regs->di = 0xfff0; // data segment length
    समुच्चयः_success(regs);
}

// APM interface disconnect
स्थिर शून्य
handle_155304(संरचना bregs *regs)
{
    समुच्चयः_success(regs);
}

// APM cpu idle
स्थिर शून्य
handle_155305(संरचना bregs *regs)
{
    yield_toirq();
    समुच्चयः_success(regs);
}

// APM cpu busy
स्थिर शून्य
handle_155306(संरचना bregs *regs)
{
    समुच्चयः_success(regs);
}

शून्य
apm_shutdown(शून्य)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    यदि (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    प्रति (;;)
        hlt();
}

// APM Set Power State
स्थिर शून्य
handle_155307(संरचना bregs *regs)
{
    यदि (regs->bx != 1) {
        समुच्चयः_success(regs);
        प्रत्यागम;
    }
    विकल्प (regs->cx) {
    पक्ष 1:
        dprintf(1, "APM standby request\n");
        भङ्ग;
    पक्ष 2:
        dprintf(1, "APM suspend request\n");
        भङ्ग;
    पक्ष 3:
        apm_shutdown();
        भङ्ग;
    }
    समुच्चयः_success(regs);
}

स्थिर शून्य
handle_155308(संरचना bregs *regs)
{
    समुच्चयः_success(regs);
}

// Get Power Status
स्थिर शून्य
handle_15530a(संरचना bregs *regs)
{
    regs->bh = 0x01; // on line
    regs->bl = 0xff; // unknown battery status
    regs->ch = 0x80; // no system battery
    regs->cl = 0xff; // unknown remaining time
    regs->dx = 0xffff; // unknown remaining time
    regs->si = 0x00; // zero battery
    समुच्चयः_success(regs);
}

#परिभाषय RET_ENOEVENT 0x80

// Get PM Event
स्थिर शून्य
handle_15530b(संरचना bregs *regs)
{
    समुच्चयः_संहिता_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
स्थिर शून्य
handle_15530e(संरचना bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    समुच्चयः_success(regs);
}

// APM Engage / Disengage
स्थिर शून्य
handle_15530f(संरचना bregs *regs)
{
    समुच्चयः_success(regs);
}

// APM Get Capabilities
स्थिर शून्य
handle_155310(संरचना bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    समुच्चयः_success(regs);
}

स्थिर शून्य
handle_1553XX(संरचना bregs *regs)
{
    समुच्चयः_unimplemented(regs);
}

शून्य
handle_1553(संरचना bregs *regs)
{
    यदि (! CONFIG_APMBIOS) {
        समुच्चयः_संहिता_invalid(regs, RET_EUNSUPPORTED);
        प्रत्यागम;
    }

    //debug_stub(regs);
    विकल्प (regs->al) {
    पक्ष 0x00: handle_155300(regs); भङ्ग;
    पक्ष 0x01: handle_155301(regs); भङ्ग;
    पक्ष 0x02: handle_155302(regs); भङ्ग;
    पक्ष 0x03: handle_155303(regs); भङ्ग;
    पक्ष 0x04: handle_155304(regs); भङ्ग;
    पक्ष 0x05: handle_155305(regs); भङ्ग;
    पक्ष 0x06: handle_155306(regs); भङ्ग;
    पक्ष 0x07: handle_155307(regs); भङ्ग;
    पक्ष 0x08: handle_155308(regs); भङ्ग;
    पक्ष 0x0a: handle_15530a(regs); भङ्ग;
    पक्ष 0x0b: handle_15530b(regs); भङ्ग;
    पक्ष 0x0e: handle_15530e(regs); भङ्ग;
    पक्ष 0x0f: handle_15530f(regs); भङ्ग;
    पक्ष 0x10: handle_155310(regs); भङ्ग;
    स्वभाव:   handle_1553XX(regs); भङ्ग;
    }
}

शून्य VISIBLE16 VISIBLE32SEG
handle_apm(संरचना bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
