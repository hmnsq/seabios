// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_EBDA
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "hw/ps2port.h" // ps2_mouse_command
#അന്തര്ഭാവ "hw/usb-hid.h" // usb_mouse_command
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // stack_hop_back
#അന്തര്ഭാവ "util.h" // mouse_init

ശൂന്യം
mouse_init(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_MOUSE)
        തിരികെ;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
mouse_command(പൂർണ്ണസംഖ്യ command, u8 *param)
{
    എങ്കിൽ (usb_mouse_active())
        തിരികെ usb_mouse_command(command, param);
    തിരികെ ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
സ്ഥിതം ശൂന്യം
mouse_15c20000(ഘടന bregs *regs)
{
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_DISABLE, ശൂന്യ);
    എങ്കിൽ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    അല്ലെങ്കിൽ
        set_code_success(regs);
}

// Enable Mouse
സ്ഥിതം ശൂന്യം
mouse_15c20001(ഘടന bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    എങ്കിൽ ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        തിരികെ;
    }

    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_ENABLE, ശൂന്യ);
    എങ്കിൽ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    അല്ലെങ്കിൽ
        set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
mouse_15c200XX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
സ്ഥിതം ശൂന്യം
mouse_15c200(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->bh) {
    അവസ്ഥ 0x00: mouse_15c20000(regs); നിർത്തുക;
    അവസ്ഥ 0x01: mouse_15c20001(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   mouse_15c200XX(regs); നിർത്തുക;
    }
}

// Reset Mouse
സ്ഥിതം ശൂന്യം
mouse_15c201(ഘടന bregs *regs)
{
    u8 param[2];
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    എങ്കിൽ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        തിരികെ;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
സ്ഥിതം ശൂന്യം
mouse_15c202(ഘടന bregs *regs)
{
    സ്ഥിതം u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    എങ്കിൽ (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        തിരികെ;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    എങ്കിൽ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    അല്ലെങ്കിൽ
        set_code_success(regs);
}

// Set Resolution
സ്ഥിതം ശൂന്യം
mouse_15c203(ഘടന bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    എങ്കിൽ (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        തിരികെ;
    }
    u8 param = regs->bh;
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    എങ്കിൽ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    അല്ലെങ്കിൽ
        set_code_success(regs);
}

// Get Device ID
സ്ഥിതം ശൂന്യം
mouse_15c204(ഘടന bregs *regs)
{
    u8 param[2];
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_GETID, param);
    എങ്കിൽ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        തിരികെ;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
സ്ഥിതം ശൂന്യം
mouse_15c205(ഘടന bregs *regs)
{
    എങ്കിൽ (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        തിരികെ;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
സ്ഥിതം ശൂന്യം
mouse_15c20600(ഘടന bregs *regs)
{
    u8 param[3];
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    എങ്കിൽ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        തിരികെ;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
സ്ഥിതം ശൂന്യം
mouse_15c20601(ഘടന bregs *regs)
{
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_SETSCALE11, ശൂന്യ);
    എങ്കിൽ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    അല്ലെങ്കിൽ
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
സ്ഥിതം ശൂന്യം
mouse_15c20602(ഘടന bregs *regs)
{
    പൂർണ്ണസംഖ്യ ret = mouse_command(PSMOUSE_CMD_SETSCALE21, ശൂന്യ);
    എങ്കിൽ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    അല്ലെങ്കിൽ
        set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
mouse_15c206XX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
സ്ഥിതം ശൂന്യം
mouse_15c206(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->bh) {
    അവസ്ഥ 0x00: mouse_15c20600(regs); നിർത്തുക;
    അവസ്ഥ 0x01: mouse_15c20601(regs); നിർത്തുക;
    അവസ്ഥ 0x02: mouse_15c20602(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   mouse_15c206XX(regs); നിർത്തുക;
    }
}

// Set Mouse Handler Address
സ്ഥിതം ശൂന്യം
mouse_15c207(ഘടന bregs *regs)
{
    ഘടന segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    എങ്കിൽ (! farptr.segoff) {
        /* remove handler */
        എങ്കിൽ ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, ശൂന്യ);
        }
    } അല്ലെങ്കിൽ {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

സ്ഥിതം ശൂന്യം
mouse_15c2XX(ഘടന bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

ശൂന്യം
handle_15c2(ഘടന bregs *regs)
{
    //debug_stub(regs);

    എങ്കിൽ (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        തിരികെ;
    }

    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 0x00: mouse_15c200(regs); നിർത്തുക;
    അവസ്ഥ 0x01: mouse_15c201(regs); നിർത്തുക;
    അവസ്ഥ 0x02: mouse_15c202(regs); നിർത്തുക;
    അവസ്ഥ 0x03: mouse_15c203(regs); നിർത്തുക;
    അവസ്ഥ 0x04: mouse_15c204(regs); നിർത്തുക;
    അവസ്ഥ 0x05: mouse_15c205(regs); നിർത്തുക;
    അവസ്ഥ 0x06: mouse_15c206(regs); നിർത്തുക;
    അവസ്ഥ 0x07: mouse_15c207(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   mouse_15c2XX(regs); നിർത്തുക;
    }
}

ശൂന്യം VISIBLE16
invoke_mouse_handler(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_MOUSE)
        തിരികെ;
    എങ്കിൽ (need_hop_back()) {
        അട്ടി_hop_back(invoke_mouse_handler, 0, 0);
        തിരികെ;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    ഘടന segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm അസ്ഥിര(
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

ശൂന്യം
process_mouse(u8 data)
{
    എങ്കിൽ (!CONFIG_MOUSE)
        തിരികെ;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    എങ്കിൽ (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        തിരികെ;

    u8 package_എണ്ണം = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    എങ്കിൽ ((index+1) < package_എണ്ണം) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        തിരികെ;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
