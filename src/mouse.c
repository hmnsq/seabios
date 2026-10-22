// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_EBDA
#شامل "bregs.h" // struct bregs
#شامل "hw/ps2port.h" // ps2_mouse_command
#شامل "hw/usb-hid.h" // usb_mouse_command
#شامل "output.h" // dprintf
#شامل "stacks.h" // stack_hop_back
#شامل "util.h" // mouse_init

تهی
mouse_init(تهی)
{
    اگر (! CONFIG_MOUSE)
        بازگشت;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

ایستا صحیح
mouse_command(صحیح command, u8 *param)
{
    اگر (usb_mouse_active())
        بازگشت usb_mouse_command(command, param);
    بازگشت ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
ایستا تهی
mouse_15c20000(ساختار bregs *regs)
{
    صحیح ret = mouse_command(PSMOUSE_CMD_DISABLE, پوچ);
    اگر (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وگرنه
        set_code_success(regs);
}

// Enable Mouse
ایستا تهی
mouse_15c20001(ساختار bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    اگر ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        بازگشت;
    }

    صحیح ret = mouse_command(PSMOUSE_CMD_ENABLE, پوچ);
    اگر (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وگرنه
        set_code_success(regs);
}

ایستا تهی
mouse_15c200XX(ساختار bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
ایستا تهی
mouse_15c200(ساختار bregs *regs)
{
    انتخاب (regs->bh) {
    حالت 0x00: mouse_15c20000(regs); بشکن;
    حالت 0x01: mouse_15c20001(regs); بشکن;
    پیشفرض:   mouse_15c200XX(regs); بشکن;
    }
}

// Reset Mouse
ایستا تهی
mouse_15c201(ساختار bregs *regs)
{
    u8 param[2];
    صحیح ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    اگر (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        بازگشت;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
ایستا تهی
mouse_15c202(ساختار bregs *regs)
{
    ایستا u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    اگر (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        بازگشت;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    صحیح ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    اگر (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وگرنه
        set_code_success(regs);
}

// Set Resolution
ایستا تهی
mouse_15c203(ساختار bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    اگر (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        بازگشت;
    }
    u8 param = regs->bh;
    صحیح ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    اگر (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وگرنه
        set_code_success(regs);
}

// Get Device ID
ایستا تهی
mouse_15c204(ساختار bregs *regs)
{
    u8 param[2];
    صحیح ret = mouse_command(PSMOUSE_CMD_GETID, param);
    اگر (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        بازگشت;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
ایستا تهی
mouse_15c205(ساختار bregs *regs)
{
    اگر (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        بازگشت;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
ایستا تهی
mouse_15c20600(ساختار bregs *regs)
{
    u8 param[3];
    صحیح ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    اگر (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        بازگشت;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
ایستا تهی
mouse_15c20601(ساختار bregs *regs)
{
    صحیح ret = mouse_command(PSMOUSE_CMD_SETSCALE11, پوچ);
    اگر (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وگرنه
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
ایستا تهی
mouse_15c20602(ساختار bregs *regs)
{
    صحیح ret = mouse_command(PSMOUSE_CMD_SETSCALE21, پوچ);
    اگر (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وگرنه
        set_code_success(regs);
}

ایستا تهی
mouse_15c206XX(ساختار bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
ایستا تهی
mouse_15c206(ساختار bregs *regs)
{
    انتخاب (regs->bh) {
    حالت 0x00: mouse_15c20600(regs); بشکن;
    حالت 0x01: mouse_15c20601(regs); بشکن;
    حالت 0x02: mouse_15c20602(regs); بشکن;
    پیشفرض:   mouse_15c206XX(regs); بشکن;
    }
}

// Set Mouse Handler Address
ایستا تهی
mouse_15c207(ساختار bregs *regs)
{
    ساختار segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    اگر (! farptr.segoff) {
        /* remove handler */
        اگر ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, پوچ);
        }
    } وگرنه {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

ایستا تهی
mouse_15c2XX(ساختار bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

تهی
handle_15c2(ساختار bregs *regs)
{
    //debug_stub(regs);

    اگر (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        بازگشت;
    }

    انتخاب (regs->al) {
    حالت 0x00: mouse_15c200(regs); بشکن;
    حالت 0x01: mouse_15c201(regs); بشکن;
    حالت 0x02: mouse_15c202(regs); بشکن;
    حالت 0x03: mouse_15c203(regs); بشکن;
    حالت 0x04: mouse_15c204(regs); بشکن;
    حالت 0x05: mouse_15c205(regs); بشکن;
    حالت 0x06: mouse_15c206(regs); بشکن;
    حالت 0x07: mouse_15c207(regs); بشکن;
    پیشفرض:   mouse_15c2XX(regs); بشکن;
    }
}

تهی VISIBLE16
invoke_mouse_handler(تهی)
{
    اگر (!CONFIG_MOUSE)
        بازگشت;
    اگر (need_hop_back()) {
        پشته_hop_back(invoke_mouse_handler, 0, 0);
        بازگشت;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    ساختار segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm فرار(
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

تهی
process_mouse(u8 data)
{
    اگر (!CONFIG_MOUSE)
        بازگشت;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    اگر (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        بازگشت;

    u8 package_شمار = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    اگر ((index+1) < package_شمار) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        بازگشت;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
