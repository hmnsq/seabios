// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_EBDA
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "hw/ps2port.h" // ps2_mouse_command
#ಅನ್ತರ್ಭಾವ "hw/usb-hid.h" // usb_mouse_command
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // stack_hop_back
#ಅನ್ತರ್ಭಾವ "util.h" // mouse_init

ಶೂನ್ಯ
mouse_init(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_MOUSE)
        ಹಿಂತಿರುಗಿ;
    dprintf(3, "init mouse\n");
    // pointing device installed
    ಗಣ_equipment_flags(0x04, 0x04);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
mouse_command(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    ಆದರೆ (usb_mouse_ಸಕ್ರಿಯ())
        ಹಿಂತಿರುಗಿ usb_mouse_command(command, param);
    ಹಿಂತಿರುಗಿ ps2_mouse_command(command, param);
}

#ವ್ಯಾಖ್ಯಾನಿಸು RET_SUCCESS      0x00
#ವ್ಯಾಖ್ಯಾನಿಸು RET_EINVFUNCTION 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು RET_EINVINPUT    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು RET_EINTERFACE   0x03
#ವ್ಯಾಖ್ಯಾನಿಸು RET_ENEEDRESEND  0x04
#ವ್ಯಾಖ್ಯಾನಿಸು RET_ENOHANDLER   0x05

// Disable Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20000(ರಚನೆ bregs *regs)
{
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_DISABLE, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಸಂಕೇತ_success(regs);
}

// Enable Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20001(ರಚನೆ bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ಆದರೆ ((mouse_flags_2 & 0x80) == 0) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENOHANDLER);
        ಹಿಂತಿರುಗಿ;
    }

    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_ENABLE, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c200XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c200(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->bh) {
    ಸ್ಥಿತಿ 0x00: mouse_15c20000(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: mouse_15c20001(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   mouse_15c200XX(regs); ನಿಲ್ಲಿಸು;
    }
}

// Reset Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c201(ರಚನೆ bregs *regs)
{
    u8 param[2];
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    ಆದರೆ (ret) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
        ಹಿಂತಿರುಗಿ;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// Set Sample Rate
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c202(ರಚನೆ bregs *regs)
{
    ಸ್ಥಿರ u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    ಆದರೆ (regs->bh >= ARRAY_ಗಾತ್ರ(sample_rates)) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EINVINPUT);
        ಹಿಂತಿರುಗಿ;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    ಆದರೆ (ret)
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಸಂಕೇತ_success(regs);
}

// Set Resolution
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c203(ರಚನೆ bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    ಆದರೆ (regs->bh >= 4) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EINVINPUT);
        ಹಿಂತಿರುಗಿ;
    }
    u8 param = regs->bh;
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    ಆದರೆ (ret)
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಸಂಕೇತ_success(regs);
}

// Get Device ID
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c204(ರಚನೆ bregs *regs)
{
    u8 param[2];
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_GETID, param);
    ಆದರೆ (ret) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
        ಹಿಂತಿರುಗಿ;
    }
    regs->bh = param[0];
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// Initialize Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c205(ರಚನೆ bregs *regs)
{
    ಆದರೆ (regs->bh != 3) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EINTERFACE);
        ಹಿಂತಿರುಗಿ;
    }
    u16 ebda_seg = get_ebda_seg();
    ಗಣ_EBDA(ebda_seg, mouse_flag1, 0x00);
    ಗಣ_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20600(ರಚನೆ bregs *regs)
{
    u8 param[3];
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    ಆದರೆ (ret) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
        ಹಿಂತಿರುಗಿ;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    ಗಣ_ಸಂಕೇತ_success(regs);
}

// Set Scaling Factor to 1:1
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20601(ರಚನೆ bregs *regs)
{
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_SETSCALE11, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಸಂಕೇತ_success(regs);
}

// Set Scaling Factor to 2:1
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20602(ರಚನೆ bregs *regs)
{
    ಪೂರ್ಣಾಂಕ ret = mouse_command(PSMOUSE_CMD_SETSCALE21, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_ENEEDRESEND);
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c206XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c206(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->bh) {
    ಸ್ಥಿತಿ 0x00: mouse_15c20600(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: mouse_15c20601(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: mouse_15c20602(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   mouse_15c206XX(regs); ನಿಲ್ಲಿಸು;
    }
}

// Set Mouse Handler Address
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c207(ರಚನೆ bregs *regs)
{
    ರಚನೆ segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ಆದರೆ (! farptr.segoff) {
        /* remove handler */
        ಆದರೆ ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, ಖಾಲಿ);
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    ಗಣ_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    ಗಣ_EBDA(ebda_seg, far_call_ಸೂಚಕ, farptr);
    ಗಣ_ಸಂಕೇತ_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c2XX(ರಚನೆ bregs *regs)
{
    ಗಣ_ಸಂಕೇತ_unimplemented(regs, RET_EINVFUNCTION);
}

ಶೂನ್ಯ
handle_15c2(ರಚನೆ bregs *regs)
{
    //debug_stub(regs);

    ಆದರೆ (! CONFIG_MOUSE) {
        ಗಣ_ಸಂಕೇತ_invalid(regs, RET_EUNSUPPORTED);
        ಹಿಂತಿರುಗಿ;
    }

    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 0x00: mouse_15c200(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: mouse_15c201(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: mouse_15c202(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: mouse_15c203(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x04: mouse_15c204(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x05: mouse_15c205(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x06: mouse_15c206(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x07: mouse_15c207(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   mouse_15c2XX(regs); ನಿಲ್ಲಿಸು;
    }
}

ಶೂನ್ಯ VISIBLE16
invoke_mouse_handler(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_MOUSE)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (need_hop_ಹಿಂಭಾಗ()) {
        ರಾಶಿ_hop_ಹಿಂಭಾಗ(invoke_mouse_handler, 0, 0);
        ಹಿಂತಿರುಗಿ;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 ಸ್ಥಿತಿ_2 = GET_EBDA(ebda_seg, mouse_ದತ್ತಾಂಶ[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_ದತ್ತಾಂಶ[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_ದತ್ತಾಂಶ[2]);

    ರಚನೆ segoff_s func = GET_EBDA(ebda_seg, far_call_ಸೂಚಕ);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , ಸ್ಥಿತಿ_2, X, Y, func.seg, func.ಸ್ಥಾನಾಂತರ_2);

    asm ಅಸ್ಥಿರ(
        "pushl %%ebp\n"
        "sti\n"

        "pushl %0\n"
        "pushw %w1\n"  //ಸ್ಥಿತಿ
        "pushw %w2\n"  // X
        "pushw %w3\n"  // Y
        "pushw $0\n"   // Z
        "lcallw *8(%%esp)\n"
        "addl $12, %%esp\n"

        "cli\n"
        "cld\n"
        "popl %%ebp"
        : "+a"(func.segoff), "+c"(ಸ್ಥಿತಿ_2), "+d"(X), "+b"(Y)
        :
        : "edi", "esi", "cc", "memory");
}

ಶೂನ್ಯ
process_mouse(u8 ದತ್ತಾಂಶ)
{
    ಆದರೆ (!CONFIG_MOUSE)
        ಹಿಂತಿರುಗಿ;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    ಆದರೆ (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        ಹಿಂತಿರುಗಿ;

    u8 package_ಎಣಿಕೆ = mouse_flags_2 & 0x07;
    u8 ಸೂಚ್ಯಂಕ = mouse_flags_1 & 0x07;
    ಗಣ_EBDA(ebda_seg, mouse_ದತ್ತಾಂಶ[ಸೂಚ್ಯಂಕ], ದತ್ತಾಂಶ);

    ಆದರೆ ((ಸೂಚ್ಯಂಕ+1) < package_ಎಣಿಕೆ) {
        mouse_flags_1++;
        ಗಣ_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        ಹಿಂತಿರುಗಿ;
    }

    ಗಣ_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
