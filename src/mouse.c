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
    ಯದಿ (! CONFIG_MOUSE)
        ಪ್ರತ್ಯಾಗಮ;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
mouse_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    ಯದಿ (usb_mouse_active())
        ಪ್ರತ್ಯಾಗಮ usb_mouse_command(command, param);
    ಪ್ರತ್ಯಾಗಮ ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20000(ಸಂರಚನಾ bregs *regs)
{
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_DISABLE, ರಿಕ್ತ);
    ಯದಿ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ಅನ್ಯಥಾ
        set_code_success(regs);
}

// Enable Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20001(ಸಂರಚನಾ bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ಯದಿ ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_ENABLE, ರಿಕ್ತ);
    ಯದಿ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ಅನ್ಯಥಾ
        set_code_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c200XX(ಸಂರಚನಾ bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c200(ಸಂರಚನಾ bregs *regs)
{
    ವಿಕಲ್ಪ (regs->bh) {
    ಪಕ್ಷ 0x00: mouse_15c20000(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: mouse_15c20001(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   mouse_15c200XX(regs); ಭಙ್ಗ;
    }
}

// Reset Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c201(ಸಂರಚನಾ bregs *regs)
{
    u8 param[2];
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    ಯದಿ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ಪ್ರತ್ಯಾಗಮ;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c202(ಸಂರಚನಾ bregs *regs)
{
    ಸ್ಥಿರ u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    ಯದಿ (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        ಪ್ರತ್ಯಾಗಮ;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    ಯದಿ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ಅನ್ಯಥಾ
        set_code_success(regs);
}

// Set Resolution
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c203(ಸಂರಚನಾ bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    ಯದಿ (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        ಪ್ರತ್ಯಾಗಮ;
    }
    u8 param = regs->bh;
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    ಯದಿ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ಅನ್ಯಥಾ
        set_code_success(regs);
}

// Get Device ID
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c204(ಸಂರಚನಾ bregs *regs)
{
    u8 param[2];
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_GETID, param);
    ಯದಿ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ಪ್ರತ್ಯಾಗಮ;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c205(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        ಪ್ರತ್ಯಾಗಮ;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20600(ಸಂರಚನಾ bregs *regs)
{
    u8 param[3];
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    ಯದಿ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ಪ್ರತ್ಯಾಗಮ;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20601(ಸಂರಚನಾ bregs *regs)
{
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_SETSCALE11, ರಿಕ್ತ);
    ಯದಿ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ಅನ್ಯಥಾ
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c20602(ಸಂರಚನಾ bregs *regs)
{
    ಪೂರ್ಣಾಙ್ಕ ret = mouse_command(PSMOUSE_CMD_SETSCALE21, ರಿಕ್ತ);
    ಯದಿ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ಅನ್ಯಥಾ
        set_code_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c206XX(ಸಂರಚನಾ bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c206(ಸಂರಚನಾ bregs *regs)
{
    ವಿಕಲ್ಪ (regs->bh) {
    ಪಕ್ಷ 0x00: mouse_15c20600(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: mouse_15c20601(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: mouse_15c20602(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   mouse_15c206XX(regs); ಭಙ್ಗ;
    }
}

// Set Mouse Handler Address
ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c207(ಸಂರಚನಾ bregs *regs)
{
    ಸಂರಚನಾ segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ಯದಿ (! farptr.segoff) {
        /* remove handler */
        ಯದಿ ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, ರಿಕ್ತ);
        }
    } ಅನ್ಯಥಾ {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mouse_15c2XX(ಸಂರಚನಾ bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

ಶೂನ್ಯ
handle_15c2(ಸಂರಚನಾ bregs *regs)
{
    //debug_stub(regs);

    ಯದಿ (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 0x00: mouse_15c200(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: mouse_15c201(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: mouse_15c202(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x03: mouse_15c203(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x04: mouse_15c204(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x05: mouse_15c205(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x06: mouse_15c206(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x07: mouse_15c207(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   mouse_15c2XX(regs); ಭಙ್ಗ;
    }
}

ಶೂನ್ಯ VISIBLE16
invoke_mouse_handler(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_MOUSE)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (need_hop_back()) {
        ರಾಶಿ_hop_back(invoke_mouse_handler, 0, 0);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    ಸಂರಚನಾ segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm ಚಞ್ಚಲ(
        "pushl %%ebp\n"
        "sti\n"

        "pushl %0\n"
        "pushw %w1\n"  // status
        "pushw %w2\n"  // X
        "pushw %w3\n"  // Y
        "pushw $0\n"   // Z
        "lcallw *8(%%esp)\n"
        "addl $12, %%esp\n"

        "cli\n"
        "cld\n"
        "popl %%ebp"
        : "+a"(func.segoff), "+c"(status), "+d"(X), "+b"(Y)
        :
        : "edi", "esi", "cc", "memory");
}

ಶೂನ್ಯ
process_mouse(u8 data)
{
    ಯದಿ (!CONFIG_MOUSE)
        ಪ್ರತ್ಯಾಗಮ;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    ಯದಿ (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        ಪ್ರತ್ಯಾಗಮ;

    u8 package_ಎಣಿಕೆ = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    ಯದಿ ((index+1) < package_ಎಣಿಕೆ) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        ಪ್ರತ್ಯಾಗಮ;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
