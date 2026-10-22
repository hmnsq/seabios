// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_EBDA
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "hw/ps2port.h" // ps2_mouse_command
#अन्तर्भाव "hw/usb-hid.h" // usb_mouse_command
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // stack_hop_back
#अन्तर्भाव "util.h" // mouse_init

शून्य
mouse_init(शून्य)
{
    यदि (! CONFIG_MOUSE)
        फर्काऊ;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

स्थिर पूर्णाङ्क
mouse_command(पूर्णाङ्क command, u8 *param)
{
    यदि (usb_mouse_active())
        फर्काऊ usb_mouse_command(command, param);
    फर्काऊ ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
स्थिर शून्य
mouse_15c20000(संरचना bregs *regs)
{
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_DISABLE, रिक्त);
    यदि (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Enable Mouse
स्थिर शून्य
mouse_15c20001(संरचना bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    यदि ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        फर्काऊ;
    }

    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_ENABLE, रिक्त);
    यदि (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

स्थिर शून्य
mouse_15c200XX(संरचना bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
स्थिर शून्य
mouse_15c200(संरचना bregs *regs)
{
    विकल्प (regs->bh) {
    पक्ष 0x00: mouse_15c20000(regs); रोक;
    पक्ष 0x01: mouse_15c20001(regs); रोक;
    स्वभाव:   mouse_15c200XX(regs); रोक;
    }
}

// Reset Mouse
स्थिर शून्य
mouse_15c201(संरचना bregs *regs)
{
    u8 param[2];
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    यदि (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        फर्काऊ;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
स्थिर शून्य
mouse_15c202(संरचना bregs *regs)
{
    स्थिर u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    यदि (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        फर्काऊ;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    यदि (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Set Resolution
स्थिर शून्य
mouse_15c203(संरचना bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    यदि (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        फर्काऊ;
    }
    u8 param = regs->bh;
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    यदि (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Get Device ID
स्थिर शून्य
mouse_15c204(संरचना bregs *regs)
{
    u8 param[2];
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_GETID, param);
    यदि (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        फर्काऊ;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
स्थिर शून्य
mouse_15c205(संरचना bregs *regs)
{
    यदि (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        फर्काऊ;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
स्थिर शून्य
mouse_15c20600(संरचना bregs *regs)
{
    u8 param[3];
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    यदि (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        फर्काऊ;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
स्थिर शून्य
mouse_15c20601(संरचना bregs *regs)
{
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_SETSCALE11, रिक्त);
    यदि (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
स्थिर शून्य
mouse_15c20602(संरचना bregs *regs)
{
    पूर्णाङ्क ret = mouse_command(PSMOUSE_CMD_SETSCALE21, रिक्त);
    यदि (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

स्थिर शून्य
mouse_15c206XX(संरचना bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
स्थिर शून्य
mouse_15c206(संरचना bregs *regs)
{
    विकल्प (regs->bh) {
    पक्ष 0x00: mouse_15c20600(regs); रोक;
    पक्ष 0x01: mouse_15c20601(regs); रोक;
    पक्ष 0x02: mouse_15c20602(regs); रोक;
    स्वभाव:   mouse_15c206XX(regs); रोक;
    }
}

// Set Mouse Handler Address
स्थिर शून्य
mouse_15c207(संरचना bregs *regs)
{
    संरचना segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    यदि (! farptr.segoff) {
        /* remove handler */
        यदि ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, रिक्त);
        }
    } अन्यथा {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

स्थिर शून्य
mouse_15c2XX(संरचना bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

शून्य
handle_15c2(संरचना bregs *regs)
{
    //debug_stub(regs);

    यदि (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        फर्काऊ;
    }

    विकल्प (regs->al) {
    पक्ष 0x00: mouse_15c200(regs); रोक;
    पक्ष 0x01: mouse_15c201(regs); रोक;
    पक्ष 0x02: mouse_15c202(regs); रोक;
    पक्ष 0x03: mouse_15c203(regs); रोक;
    पक्ष 0x04: mouse_15c204(regs); रोक;
    पक्ष 0x05: mouse_15c205(regs); रोक;
    पक्ष 0x06: mouse_15c206(regs); रोक;
    पक्ष 0x07: mouse_15c207(regs); रोक;
    स्वभाव:   mouse_15c2XX(regs); रोक;
    }
}

शून्य VISIBLE16
invoke_mouse_handler(शून्य)
{
    यदि (!CONFIG_MOUSE)
        फर्काऊ;
    यदि (need_hop_back()) {
        थाक_hop_back(invoke_mouse_handler, 0, 0);
        फर्काऊ;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    संरचना segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    यंत्र चञ्चल(
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

शून्य
process_mouse(u8 data)
{
    यदि (!CONFIG_MOUSE)
        फर्काऊ;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    यदि (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        फर्काऊ;

    u8 package_गन्ती = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    यदि ((index+1) < package_गन्ती) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        फर्काऊ;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
