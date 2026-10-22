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

خالی
mouse_init(خالی)
{
    اگر (! CONFIG_MOUSE)
        واپس;
    dprintf(3, "init mouse\n");
    // pointing device installed
    سیٹ_equipment_flags(0x04, 0x04);
}

ساکن صحیح
mouse_command(صحیح command, u8 *param)
{
    اگر (usb_mouse_فعال())
        واپس usb_mouse_command(command, param);
    واپس ps2_mouse_command(command, param);
}

#تعریف_کرو RET_SUCCESS      0x00
#تعریف_کرو RET_EINVFUNCTION 0x01
#تعریف_کرو RET_EINVINPUT    0x02
#تعریف_کرو RET_EINTERFACE   0x03
#تعریف_کرو RET_ENEEDRESEND  0x04
#تعریف_کرو RET_ENOHANDLER   0x05

// Disable Mouse
ساکن خالی
mouse_15c20000(ساخت bregs *regs)
{
    صحیح ret = mouse_command(PSMOUSE_CMD_DISABLE, تہی);
    اگر (ret)
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
    ورنہ
        سیٹ_رمز_success(regs);
}

// Enable Mouse
ساکن خالی
mouse_15c20001(ساخت bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    اگر ((mouse_flags_2 & 0x80) == 0) {
        سیٹ_رمز_invalid(regs, RET_ENOHANDLER);
        واپس;
    }

    صحیح ret = mouse_command(PSMOUSE_CMD_ENABLE, تہی);
    اگر (ret)
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
    ورنہ
        سیٹ_رمز_success(regs);
}

ساکن خالی
mouse_15c200XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
ساکن خالی
mouse_15c200(ساخت bregs *regs)
{
    انتخاب (regs->bh) {
    حالت 0x00: mouse_15c20000(regs); توڑو;
    حالت 0x01: mouse_15c20001(regs); توڑو;
    طےشدہ:   mouse_15c200XX(regs); توڑو;
    }
}

// Reset Mouse
ساکن خالی
mouse_15c201(ساخت bregs *regs)
{
    u8 param[2];
    صحیح ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    اگر (ret) {
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
        واپس;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    سیٹ_رمز_success(regs);
}

// Set Sample Rate
ساکن خالی
mouse_15c202(ساخت bregs *regs)
{
    ساکن u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    اگر (regs->bh >= ARRAY_سائز(sample_rates)) {
        سیٹ_رمز_invalid(regs, RET_EINVINPUT);
        واپس;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    صحیح ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    اگر (ret)
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
    ورنہ
        سیٹ_رمز_success(regs);
}

// Set Resolution
ساکن خالی
mouse_15c203(ساخت bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    اگر (regs->bh >= 4) {
        سیٹ_رمز_invalid(regs, RET_EINVINPUT);
        واپس;
    }
    u8 param = regs->bh;
    صحیح ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    اگر (ret)
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
    ورنہ
        سیٹ_رمز_success(regs);
}

// Get Device ID
ساکن خالی
mouse_15c204(ساخت bregs *regs)
{
    u8 param[2];
    صحیح ret = mouse_command(PSMOUSE_CMD_GETID, param);
    اگر (ret) {
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
        واپس;
    }
    regs->bh = param[0];
    سیٹ_رمز_success(regs);
}

// Initialize Mouse
ساکن خالی
mouse_15c205(ساخت bregs *regs)
{
    اگر (regs->bh != 3) {
        سیٹ_رمز_invalid(regs, RET_EINTERFACE);
        واپس;
    }
    u16 ebda_seg = get_ebda_seg();
    سیٹ_EBDA(ebda_seg, mouse_flag1, 0x00);
    سیٹ_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
ساکن خالی
mouse_15c20600(ساخت bregs *regs)
{
    u8 param[3];
    صحیح ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    اگر (ret) {
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
        واپس;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    سیٹ_رمز_success(regs);
}

// Set Scaling Factor to 1:1
ساکن خالی
mouse_15c20601(ساخت bregs *regs)
{
    صحیح ret = mouse_command(PSMOUSE_CMD_SETSCALE11, تہی);
    اگر (ret)
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
    ورنہ
        سیٹ_رمز_success(regs);
}

// Set Scaling Factor to 2:1
ساکن خالی
mouse_15c20602(ساخت bregs *regs)
{
    صحیح ret = mouse_command(PSMOUSE_CMD_SETSCALE21, تہی);
    اگر (ret)
        سیٹ_رمز_invalid(regs, RET_ENEEDRESEND);
    ورنہ
        سیٹ_رمز_success(regs);
}

ساکن خالی
mouse_15c206XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
ساکن خالی
mouse_15c206(ساخت bregs *regs)
{
    انتخاب (regs->bh) {
    حالت 0x00: mouse_15c20600(regs); توڑو;
    حالت 0x01: mouse_15c20601(regs); توڑو;
    حالت 0x02: mouse_15c20602(regs); توڑو;
    طےشدہ:   mouse_15c206XX(regs); توڑو;
    }
}

// Set Mouse Handler Address
ساکن خالی
mouse_15c207(ساخت bregs *regs)
{
    ساخت segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    اگر (! farptr.segoff) {
        /* remove handler */
        اگر ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, تہی);
        }
    } ورنہ {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    سیٹ_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    سیٹ_EBDA(ebda_seg, far_call_اشارہ_گر, farptr);
    سیٹ_رمز_success(regs);
}

ساکن خالی
mouse_15c2XX(ساخت bregs *regs)
{
    سیٹ_رمز_unimplemented(regs, RET_EINVFUNCTION);
}

خالی
handle_15c2(ساخت bregs *regs)
{
    //debug_stub(regs);

    اگر (! CONFIG_MOUSE) {
        سیٹ_رمز_invalid(regs, RET_EUNSUPPORTED);
        واپس;
    }

    انتخاب (regs->al) {
    حالت 0x00: mouse_15c200(regs); توڑو;
    حالت 0x01: mouse_15c201(regs); توڑو;
    حالت 0x02: mouse_15c202(regs); توڑو;
    حالت 0x03: mouse_15c203(regs); توڑو;
    حالت 0x04: mouse_15c204(regs); توڑو;
    حالت 0x05: mouse_15c205(regs); توڑو;
    حالت 0x06: mouse_15c206(regs); توڑو;
    حالت 0x07: mouse_15c207(regs); توڑو;
    طےشدہ:   mouse_15c2XX(regs); توڑو;
    }
}

خالی VISIBLE16
invoke_mouse_handler(خالی)
{
    اگر (!CONFIG_MOUSE)
        واپس;
    اگر (need_hop_عقب()) {
        ڈھیر_hop_عقب(invoke_mouse_handler, 0, 0);
        واپس;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 حالت_2 = GET_EBDA(ebda_seg, mouse_ڈیٹا[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_ڈیٹا[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_ڈیٹا[2]);

    ساخت segoff_s func = GET_EBDA(ebda_seg, far_call_اشارہ_گر);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , حالت_2, X, Y, func.seg, func.انحراف_2);

    asm متلون(
        "pushl %%ebp\n"
        "sti\n"

        "pushl %0\n"
        "pushw %w1\n"  //حالت
        "pushw %w2\n"  // X
        "pushw %w3\n"  // Y
        "pushw $0\n"   // Z
        "lcallw *8(%%esp)\n"
        "addl $12, %%esp\n"

        "cli\n"
        "cld\n"
        "popl %%ebp"
        : "+a"(func.segoff), "+c"(حالت_2), "+d"(X), "+b"(Y)
        :
        : "edi", "esi", "cc", "memory");
}

خالی
process_mouse(u8 ڈیٹا)
{
    اگر (!CONFIG_MOUSE)
        واپس;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    اگر (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        واپس;

    u8 package_گنتی = mouse_flags_2 & 0x07;
    u8 اشاریہ = mouse_flags_1 & 0x07;
    سیٹ_EBDA(ebda_seg, mouse_ڈیٹا[اشاریہ], ڈیٹا);

    اگر ((اشاریہ+1) < package_گنتی) {
        mouse_flags_1++;
        سیٹ_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        واپس;
    }

    سیٹ_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
