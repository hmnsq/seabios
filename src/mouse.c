// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_EBDA
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ps2port.h" // ps2_mouse_command
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/usb-hid.h" // usb_mouse_command
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // stack_hop_back
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // mouse_init

ਖਾਲੀ
mouse_init(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_MOUSE)
        ਵਾਪਸ;
    dprintf(3, "init mouse\n");
    // pointing device installed
    ਸੈੱਟ_equipment_flags(0x04, 0x04);
}

ਸਥਿਰ ਪੂਰਨ
mouse_command(ਪੂਰਨ command, u8 *param)
{
    ਜੇ (usb_mouse_ਸਰਗਰਮ())
        ਵਾਪਸ usb_mouse_command(command, param);
    ਵਾਪਸ ps2_mouse_command(command, param);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_SUCCESS      0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_EINVFUNCTION 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_EINVINPUT    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_EINTERFACE   0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_ENEEDRESEND  0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RET_ENOHANDLER   0x05

// Disable Mouse
ਸਥਿਰ ਖਾਲੀ
mouse_15c20000(ਬਣਤਰ bregs *regs)
{
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_DISABLE, ਨਿਰਾ);
    ਜੇ (ret)
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Enable Mouse
ਸਥਿਰ ਖਾਲੀ
mouse_15c20001(ਬਣਤਰ bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ਜੇ ((mouse_flags_2 & 0x80) == 0) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENOHANDLER);
        ਵਾਪਸ;
    }

    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_ENABLE, ਨਿਰਾ);
    ਜੇ (ret)
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
mouse_15c200XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
ਸਥਿਰ ਖਾਲੀ
mouse_15c200(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->bh) {
    ਹਾਲਤ 0x00: mouse_15c20000(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: mouse_15c20001(regs); ਰੋਕੋ;
    ਮੂਲ:   mouse_15c200XX(regs); ਰੋਕੋ;
    }
}

// Reset Mouse
ਸਥਿਰ ਖਾਲੀ
mouse_15c201(ਬਣਤਰ bregs *regs)
{
    u8 param[2];
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    ਜੇ (ret) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
        ਵਾਪਸ;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Set Sample Rate
ਸਥਿਰ ਖਾਲੀ
mouse_15c202(ਬਣਤਰ bregs *regs)
{
    ਸਥਿਰ u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    ਜੇ (regs->bh >= ARRAY_ਮਾਪ(sample_rates)) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EINVINPUT);
        ਵਾਪਸ;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    ਜੇ (ret)
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Set Resolution
ਸਥਿਰ ਖਾਲੀ
mouse_15c203(ਬਣਤਰ bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    ਜੇ (regs->bh >= 4) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EINVINPUT);
        ਵਾਪਸ;
    }
    u8 param = regs->bh;
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    ਜੇ (ret)
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Get Device ID
ਸਥਿਰ ਖਾਲੀ
mouse_15c204(ਬਣਤਰ bregs *regs)
{
    u8 param[2];
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_GETID, param);
    ਜੇ (ret) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
        ਵਾਪਸ;
    }
    regs->bh = param[0];
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Initialize Mouse
ਸਥਿਰ ਖਾਲੀ
mouse_15c205(ਬਣਤਰ bregs *regs)
{
    ਜੇ (regs->bh != 3) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EINTERFACE);
        ਵਾਪਸ;
    }
    u16 ebda_seg = get_ebda_seg();
    ਸੈੱਟ_EBDA(ebda_seg, mouse_flag1, 0x00);
    ਸੈੱਟ_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
ਸਥਿਰ ਖਾਲੀ
mouse_15c20600(ਬਣਤਰ bregs *regs)
{
    u8 param[3];
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    ਜੇ (ret) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
        ਵਾਪਸ;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Set Scaling Factor to 1:1
ਸਥਿਰ ਖਾਲੀ
mouse_15c20601(ਬਣਤਰ bregs *regs)
{
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_SETSCALE11, ਨਿਰਾ);
    ਜੇ (ret)
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਕੋਡ_success(regs);
}

// Set Scaling Factor to 2:1
ਸਥਿਰ ਖਾਲੀ
mouse_15c20602(ਬਣਤਰ bregs *regs)
{
    ਪੂਰਨ ret = mouse_command(PSMOUSE_CMD_SETSCALE21, ਨਿਰਾ);
    ਜੇ (ret)
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_ENEEDRESEND);
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
mouse_15c206XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
ਸਥਿਰ ਖਾਲੀ
mouse_15c206(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->bh) {
    ਹਾਲਤ 0x00: mouse_15c20600(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: mouse_15c20601(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: mouse_15c20602(regs); ਰੋਕੋ;
    ਮੂਲ:   mouse_15c206XX(regs); ਰੋਕੋ;
    }
}

// Set Mouse Handler Address
ਸਥਿਰ ਖਾਲੀ
mouse_15c207(ਬਣਤਰ bregs *regs)
{
    ਬਣਤਰ segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ਜੇ (! farptr.segoff) {
        /* remove handler */
        ਜੇ ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, ਨਿਰਾ);
        }
    } ਨਹੀਂਤਾਂ {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    ਸੈੱਟ_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    ਸੈੱਟ_EBDA(ebda_seg, far_call_ਸੂਚਕ, farptr);
    ਸੈੱਟ_ਕੋਡ_success(regs);
}

ਸਥਿਰ ਖਾਲੀ
mouse_15c2XX(ਬਣਤਰ bregs *regs)
{
    ਸੈੱਟ_ਕੋਡ_unimplemented(regs, RET_EINVFUNCTION);
}

ਖਾਲੀ
handle_15c2(ਬਣਤਰ bregs *regs)
{
    //debug_stub(regs);

    ਜੇ (! CONFIG_MOUSE) {
        ਸੈੱਟ_ਕੋਡ_invalid(regs, RET_EUNSUPPORTED);
        ਵਾਪਸ;
    }

    ਚੋਣ (regs->al) {
    ਹਾਲਤ 0x00: mouse_15c200(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: mouse_15c201(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: mouse_15c202(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x03: mouse_15c203(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x04: mouse_15c204(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x05: mouse_15c205(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x06: mouse_15c206(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x07: mouse_15c207(regs); ਰੋਕੋ;
    ਮੂਲ:   mouse_15c2XX(regs); ਰੋਕੋ;
    }
}

ਖਾਲੀ VISIBLE16
invoke_mouse_handler(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_MOUSE)
        ਵਾਪਸ;
    ਜੇ (need_hop_ਪਿੱਠ()) {
        ਢੇਰ_hop_ਪਿੱਠ(invoke_mouse_handler, 0, 0);
        ਵਾਪਸ;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 ਸਥਿਤੀ = GET_EBDA(ebda_seg, mouse_ਅੰਕੜੇ[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_ਅੰਕੜੇ[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_ਅੰਕੜੇ[2]);

    ਬਣਤਰ segoff_s func = GET_EBDA(ebda_seg, far_call_ਸੂਚਕ);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , ਸਥਿਤੀ, X, Y, func.seg, func.ਵਿੱਥ_2);

    asm ਅਸਥਿਰ(
        "pushl %%ebp\n"
        "sti\n"

        "pushl %0\n"
        "pushw %w1\n"  //ਸਥਿਤੀ
        "pushw %w2\n"  // X
        "pushw %w3\n"  // Y
        "pushw $0\n"   // Z
        "lcallw *8(%%esp)\n"
        "addl $12, %%esp\n"

        "cli\n"
        "cld\n"
        "popl %%ebp"
        : "+a"(func.segoff), "+c"(ਸਥਿਤੀ), "+d"(X), "+b"(Y)
        :
        : "edi", "esi", "cc", "memory");
}

ਖਾਲੀ
process_mouse(u8 ਅੰਕੜੇ)
{
    ਜੇ (!CONFIG_MOUSE)
        ਵਾਪਸ;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    ਜੇ (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        ਵਾਪਸ;

    u8 package_ਗਿਣਤੀ = mouse_flags_2 & 0x07;
    u8 ਸੂਚਕਾਂਕ = mouse_flags_1 & 0x07;
    ਸੈੱਟ_EBDA(ebda_seg, mouse_ਅੰਕੜੇ[ਸੂਚਕਾਂਕ], ਅੰਕੜੇ);

    ਜੇ ((ਸੂਚਕਾਂਕ+1) < package_ਗਿਣਤੀ) {
        mouse_flags_1++;
        ਸੈੱਟ_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        ਵਾਪਸ;
    }

    ਸੈੱਟ_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
