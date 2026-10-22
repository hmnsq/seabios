// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_EBDA
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "hw/ps2port.h" // ps2_mouse_command
#அந்தர்பாவ "hw/usb-hid.h" // usb_mouse_command
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "stacks.h" // stack_hop_back
#அந்தர்பாவ "util.h" // mouse_init

வெறுமை
mouse_init(வெறுமை)
{
    எனில் (! CONFIG_MOUSE)
        திரும்பு;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

நிலையான முழு
mouse_command(முழு command, u8 *param)
{
    எனில் (usb_mouse_active())
        திரும்பு usb_mouse_command(command, param);
    திரும்பு ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
நிலையான வெறுமை
mouse_15c20000(கட்டமைப்பு bregs *regs)
{
    முழு ret = mouse_command(PSMOUSE_CMD_DISABLE, வெற்று);
    எனில் (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    இல்லையெனில்
        set_code_success(regs);
}

// Enable Mouse
நிலையான வெறுமை
mouse_15c20001(கட்டமைப்பு bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    எனில் ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        திரும்பு;
    }

    முழு ret = mouse_command(PSMOUSE_CMD_ENABLE, வெற்று);
    எனில் (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    இல்லையெனில்
        set_code_success(regs);
}

நிலையான வெறுமை
mouse_15c200XX(கட்டமைப்பு bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
நிலையான வெறுமை
mouse_15c200(கட்டமைப்பு bregs *regs)
{
    தேர்வு (regs->bh) {
    நிலை 0x00: mouse_15c20000(regs); நிறுத்து;
    நிலை 0x01: mouse_15c20001(regs); நிறுத்து;
    இயல்பு:   mouse_15c200XX(regs); நிறுத்து;
    }
}

// Reset Mouse
நிலையான வெறுமை
mouse_15c201(கட்டமைப்பு bregs *regs)
{
    u8 param[2];
    முழு ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    எனில் (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        திரும்பு;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
நிலையான வெறுமை
mouse_15c202(கட்டமைப்பு bregs *regs)
{
    நிலையான u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    எனில் (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        திரும்பு;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    முழு ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    எனில் (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    இல்லையெனில்
        set_code_success(regs);
}

// Set Resolution
நிலையான வெறுமை
mouse_15c203(கட்டமைப்பு bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    எனில் (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        திரும்பு;
    }
    u8 param = regs->bh;
    முழு ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    எனில் (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    இல்லையெனில்
        set_code_success(regs);
}

// Get Device ID
நிலையான வெறுமை
mouse_15c204(கட்டமைப்பு bregs *regs)
{
    u8 param[2];
    முழு ret = mouse_command(PSMOUSE_CMD_GETID, param);
    எனில் (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        திரும்பு;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
நிலையான வெறுமை
mouse_15c205(கட்டமைப்பு bregs *regs)
{
    எனில் (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        திரும்பு;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
நிலையான வெறுமை
mouse_15c20600(கட்டமைப்பு bregs *regs)
{
    u8 param[3];
    முழு ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    எனில் (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        திரும்பு;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
நிலையான வெறுமை
mouse_15c20601(கட்டமைப்பு bregs *regs)
{
    முழு ret = mouse_command(PSMOUSE_CMD_SETSCALE11, வெற்று);
    எனில் (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    இல்லையெனில்
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
நிலையான வெறுமை
mouse_15c20602(கட்டமைப்பு bregs *regs)
{
    முழு ret = mouse_command(PSMOUSE_CMD_SETSCALE21, வெற்று);
    எனில் (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    இல்லையெனில்
        set_code_success(regs);
}

நிலையான வெறுமை
mouse_15c206XX(கட்டமைப்பு bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
நிலையான வெறுமை
mouse_15c206(கட்டமைப்பு bregs *regs)
{
    தேர்வு (regs->bh) {
    நிலை 0x00: mouse_15c20600(regs); நிறுத்து;
    நிலை 0x01: mouse_15c20601(regs); நிறுத்து;
    நிலை 0x02: mouse_15c20602(regs); நிறுத்து;
    இயல்பு:   mouse_15c206XX(regs); நிறுத்து;
    }
}

// Set Mouse Handler Address
நிலையான வெறுமை
mouse_15c207(கட்டமைப்பு bregs *regs)
{
    கட்டமைப்பு segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    எனில் (! farptr.segoff) {
        /* remove handler */
        எனில் ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, வெற்று);
        }
    } இல்லையெனில் {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

நிலையான வெறுமை
mouse_15c2XX(கட்டமைப்பு bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

வெறுமை
handle_15c2(கட்டமைப்பு bregs *regs)
{
    //debug_stub(regs);

    எனில் (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        திரும்பு;
    }

    தேர்வு (regs->al) {
    நிலை 0x00: mouse_15c200(regs); நிறுத்து;
    நிலை 0x01: mouse_15c201(regs); நிறுத்து;
    நிலை 0x02: mouse_15c202(regs); நிறுத்து;
    நிலை 0x03: mouse_15c203(regs); நிறுத்து;
    நிலை 0x04: mouse_15c204(regs); நிறுத்து;
    நிலை 0x05: mouse_15c205(regs); நிறுத்து;
    நிலை 0x06: mouse_15c206(regs); நிறுத்து;
    நிலை 0x07: mouse_15c207(regs); நிறுத்து;
    இயல்பு:   mouse_15c2XX(regs); நிறுத்து;
    }
}

வெறுமை VISIBLE16
invoke_mouse_handler(வெறுமை)
{
    எனில் (!CONFIG_MOUSE)
        திரும்பு;
    எனில் (need_hop_back()) {
        அடுக்கு_hop_back(invoke_mouse_handler, 0, 0);
        திரும்பு;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    கட்டமைப்பு segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm நிலையற்ற(
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

வெறுமை
process_mouse(u8 data)
{
    எனில் (!CONFIG_MOUSE)
        திரும்பு;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    எனில் (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        திரும்பு;

    u8 package_மொத்தம் = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    எனில் ((index+1) < package_மொத்தம்) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        திரும்பு;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
