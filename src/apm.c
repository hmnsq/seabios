// Basic support for apmbios callbacks.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2005 Struan Bartlett
// Copyright (C) 2004 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield_toirq
#ಅನ್ತರ್ಭಾವ "util.h" // apm_shutdown
#ಅನ್ತರ್ಭಾವ "x86.h" // outb

// APM installation check
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155300(ರಚನೆ bregs *regs)
{
    regs->ah = 1; // APM major version
    regs->al = 2; // APM minor version
    regs->bh = 'P';
    regs->bl = 'M';
    // bit 0 : 16 bit interface supported
    // bit 1 : 32 bit interface supported
    regs->cx = 0x03;
    ಗಣ_success(regs);
}

// APM real mode interface connect
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155301(ರಚನೆ bregs *regs)
{
    ಗಣ_success(regs);
}

// APM 16 bit protected mode interface connect
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155302(ರಚನೆ bregs *regs)
{
    ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_apm16(ಶೂನ್ಯ);
    regs->bx = (u32)ನಮೂದು_apm16;
    regs->ax = SEG_BIOS; // 16 bit code segment base
    regs->si = 0xfff0;   // 16 bit code segment size
    regs->cx = SEG_BIOS; // data segment address
    regs->di = 0xfff0;   // data segment length
    ಗಣ_success(regs);
}

// APM 32 bit protected mode interface connect
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155303(ರಚನೆ bregs *regs)
{
    ಬಾಹ್ಯ ಶೂನ್ಯ ನಮೂದು_apm32(ಶೂನ್ಯ);
    regs->ax = SEG_BIOS; // 32 bit code segment base
    regs->ebx = (u32)ನಮೂದು_apm32;
    regs->cx = SEG_BIOS; // 16 bit code segment base
    // 32 bit code segment size (low 16 bits)
    // 16 bit code segment size (high 16 bits)
    regs->esi = 0xfff0fff0;
    regs->dx = SEG_BIOS; // data segment address
    regs->di = 0xfff0; // data segment length
    ಗಣ_success(regs);
}

// APM interface disconnect
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155304(ರಚನೆ bregs *regs)
{
    ಗಣ_success(regs);
}

// APM cpu idle
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155305(ರಚನೆ bregs *regs)
{
    yield_toirq();
    ಗಣ_success(regs);
}

// APM cpu busy
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155306(ರಚನೆ bregs *regs)
{
    ಗಣ_success(regs);
}

ಶೂನ್ಯ
apm_shutdown(ಶೂನ್ಯ)
{
    u16 pm1a_cnt = GET_GLOBAL(acpi_pm1a_cnt);
    ಆದರೆ (pm1a_cnt)
        outw(0x2000, pm1a_cnt);

    irq_disable();
    ಗಾಗಿ (;;)
        hlt();
}

// APM Set Power State
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155307(ರಚನೆ bregs *regs)
{
    ಆದರೆ (regs->bx != 1) {
        ಗಣ_success(regs);
        ಹಿಂತಿರುಗಿ;
    }
    ಆಯ್ಕೆ (regs->cx) {
    ಸ್ಥಿತಿ 1:
        dprintf(1, "APM standby request\n");
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 2:
        dprintf(1, "APM suspend request\n");
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 3:
        apm_shutdown();
        ನಿಲ್ಲಿಸು;
    }
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_155308(ರಚನೆ bregs *regs)
{
    ಗಣ_success(regs);
}

// Get Power Status
ಸ್ಥಿರ ಶೂನ್ಯ
handle_15530a(ರಚನೆ bregs *regs)
{
    regs->bh = 0x01; // on line
    regs->bl = 0xff; // unknown battery status
    regs->ch = 0x80; // no system battery
    regs->cl = 0xff; // unknown remaining time
    regs->dx = 0xffff; // unknown remaining time
    regs->si = 0x00; // zero battery
    ಗಣ_success(regs);
}

#ವ್ಯಾಖ್ಯಾನಿಸು RET_ENOEVENT 0x80

// Get PM Event
ಸ್ಥಿರ ಶೂನ್ಯ
handle_15530b(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_invalid_silent(regs, RET_ENOEVENT);
}

// APM Driver Version
ಸ್ಥಿರ ಶೂನ್ಯ
handle_15530e(ರಚನೆ bregs *regs)
{
    regs->ah = 1;
    regs->al = 2;
    ಗಣ_success(regs);
}

// APM Engage / Disengage
ಸ್ಥಿರ ಶೂನ್ಯ
handle_15530f(ರಚನೆ bregs *regs)
{
    ಗಣ_success(regs);
}

// APM Get Capabilities
ಸ್ಥಿರ ಶೂನ್ಯ
handle_155310(ರಚನೆ bregs *regs)
{
    regs->bl = 0;
    regs->cx = 0;
    ಗಣ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_1553XX(ರಚನೆ bregs *regs)
{
    ಗಣ_unimplemented(regs);
}

ಶೂನ್ಯ
handle_1553(ರಚನೆ bregs *regs)
{
    ಆದರೆ (! CONFIG_APMBIOS) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EUNSUPPORTED);
        ಹಿಂತಿರುಗಿ;
    }

    //debug_stub(regs);
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x00: handle_155300(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: handle_155301(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: handle_155302(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: handle_155303(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x04: handle_155304(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x05: handle_155305(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x06: handle_155306(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x07: handle_155307(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x08: handle_155308(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0a: handle_15530a(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0b: handle_15530b(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0e: handle_15530e(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0f: handle_15530f(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x10: handle_155310(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   handle_1553XX(regs); ನಿಲ್ಲಿಸು;
    }
}

ಶೂನ್ಯ VISIBLE16 VISIBLE32SEG
handle_apm(ರಚನೆ bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_apm);
    handle_1553(regs);
}
