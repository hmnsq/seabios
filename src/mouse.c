// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_EBDA
#ضمن "bregs.h" // struct bregs
#ضمن "hw/ps2port.h" // ps2_mouse_command
#ضمن "hw/usb-hid.h" // usb_mouse_command
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // stack_hop_back
#ضمن "util.h" // mouse_init

فراغ
mouse_init(فراغ)
{
    إذا (! CONFIG_MOUSE)
        أرجع;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

ساكن صحيح
mouse_command(صحيح command, u8 *param)
{
    إذا (usb_mouse_active())
        أرجع usb_mouse_command(command, param);
    أرجع ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
ساكن فراغ
mouse_15c20000(بنية bregs *regs)
{
    صحيح ret = mouse_command(PSMOUSE_CMD_DISABLE, عدم);
    إذا (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وإلا
        set_code_success(regs);
}

// Enable Mouse
ساكن فراغ
mouse_15c20001(بنية bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    إذا ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        أرجع;
    }

    صحيح ret = mouse_command(PSMOUSE_CMD_ENABLE, عدم);
    إذا (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وإلا
        set_code_success(regs);
}

ساكن فراغ
mouse_15c200XX(بنية bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
ساكن فراغ
mouse_15c200(بنية bregs *regs)
{
    بدل (regs->bh) {
    حالة 0x00: mouse_15c20000(regs); اكسر;
    حالة 0x01: mouse_15c20001(regs); اكسر;
    افتراضي:   mouse_15c200XX(regs); اكسر;
    }
}

// Reset Mouse
ساكن فراغ
mouse_15c201(بنية bregs *regs)
{
    u8 param[2];
    صحيح ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    إذا (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        أرجع;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
ساكن فراغ
mouse_15c202(بنية bregs *regs)
{
    ساكن u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    إذا (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        أرجع;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    صحيح ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    إذا (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وإلا
        set_code_success(regs);
}

// Set Resolution
ساكن فراغ
mouse_15c203(بنية bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    إذا (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        أرجع;
    }
    u8 param = regs->bh;
    صحيح ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    إذا (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وإلا
        set_code_success(regs);
}

// Get Device ID
ساكن فراغ
mouse_15c204(بنية bregs *regs)
{
    u8 param[2];
    صحيح ret = mouse_command(PSMOUSE_CMD_GETID, param);
    إذا (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        أرجع;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
ساكن فراغ
mouse_15c205(بنية bregs *regs)
{
    إذا (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        أرجع;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
ساكن فراغ
mouse_15c20600(بنية bregs *regs)
{
    u8 param[3];
    صحيح ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    إذا (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        أرجع;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
ساكن فراغ
mouse_15c20601(بنية bregs *regs)
{
    صحيح ret = mouse_command(PSMOUSE_CMD_SETSCALE11, عدم);
    إذا (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وإلا
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
ساكن فراغ
mouse_15c20602(بنية bregs *regs)
{
    صحيح ret = mouse_command(PSMOUSE_CMD_SETSCALE21, عدم);
    إذا (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    وإلا
        set_code_success(regs);
}

ساكن فراغ
mouse_15c206XX(بنية bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
ساكن فراغ
mouse_15c206(بنية bregs *regs)
{
    بدل (regs->bh) {
    حالة 0x00: mouse_15c20600(regs); اكسر;
    حالة 0x01: mouse_15c20601(regs); اكسر;
    حالة 0x02: mouse_15c20602(regs); اكسر;
    افتراضي:   mouse_15c206XX(regs); اكسر;
    }
}

// Set Mouse Handler Address
ساكن فراغ
mouse_15c207(بنية bregs *regs)
{
    بنية segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    إذا (! farptr.segoff) {
        /* remove handler */
        إذا ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, عدم);
        }
    } وإلا {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

ساكن فراغ
mouse_15c2XX(بنية bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

فراغ
handle_15c2(بنية bregs *regs)
{
    //debug_stub(regs);

    إذا (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        أرجع;
    }

    بدل (regs->al) {
    حالة 0x00: mouse_15c200(regs); اكسر;
    حالة 0x01: mouse_15c201(regs); اكسر;
    حالة 0x02: mouse_15c202(regs); اكسر;
    حالة 0x03: mouse_15c203(regs); اكسر;
    حالة 0x04: mouse_15c204(regs); اكسر;
    حالة 0x05: mouse_15c205(regs); اكسر;
    حالة 0x06: mouse_15c206(regs); اكسر;
    حالة 0x07: mouse_15c207(regs); اكسر;
    افتراضي:   mouse_15c2XX(regs); اكسر;
    }
}

فراغ VISIBLE16
invoke_mouse_handler(فراغ)
{
    إذا (!CONFIG_MOUSE)
        أرجع;
    إذا (need_hop_back()) {
        مكدس_hop_back(invoke_mouse_handler, 0, 0);
        أرجع;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    بنية segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm متطاير(
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

فراغ
process_mouse(u8 data)
{
    إذا (!CONFIG_MOUSE)
        أرجع;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    إذا (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        أرجع;

    u8 package_عدد = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    إذا ((index+1) < package_عدد) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        أرجع;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
