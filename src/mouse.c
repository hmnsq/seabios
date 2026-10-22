// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_EBDA
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "hw/ps2port.h" // ps2_mouse_command
#অন্তর্ভাব "hw/usb-hid.h" // usb_mouse_command
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "stacks.h" // stack_hop_back
#অন্তর্ভাব "util.h" // mouse_init

শূন্য
mouse_init(শূন্য)
{
    যদি (! CONFIG_MOUSE)
        প্রত্যাগম;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

স্থির পূর্ণাঙ্ক
mouse_command(পূর্ণাঙ্ক command, u8 *param)
{
    যদি (usb_mouse_active())
        প্রত্যাগম usb_mouse_command(command, param);
    প্রত্যাগম ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
স্থির শূন্য
mouse_15c20000(সংরচনা bregs *regs)
{
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_DISABLE, রিক্ত);
    যদি (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    অন্যথা
        set_code_success(regs);
}

// Enable Mouse
স্থির শূন্য
mouse_15c20001(সংরচনা bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    যদি ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        প্রত্যাগম;
    }

    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_ENABLE, রিক্ত);
    যদি (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    অন্যথা
        set_code_success(regs);
}

স্থির শূন্য
mouse_15c200XX(সংরচনা bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
স্থির শূন্য
mouse_15c200(সংরচনা bregs *regs)
{
    বিকল্প (regs->bh) {
    পক্ষ 0x00: mouse_15c20000(regs); ভঙ্গ;
    পক্ষ 0x01: mouse_15c20001(regs); ভঙ্গ;
    স্বভাব:   mouse_15c200XX(regs); ভঙ্গ;
    }
}

// Reset Mouse
স্থির শূন্য
mouse_15c201(সংরচনা bregs *regs)
{
    u8 param[2];
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    যদি (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        প্রত্যাগম;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
স্থির শূন্য
mouse_15c202(সংরচনা bregs *regs)
{
    স্থির u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    যদি (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        প্রত্যাগম;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    যদি (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    অন্যথা
        set_code_success(regs);
}

// Set Resolution
স্থির শূন্য
mouse_15c203(সংরচনা bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    যদি (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        প্রত্যাগম;
    }
    u8 param = regs->bh;
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    যদি (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    অন্যথা
        set_code_success(regs);
}

// Get Device ID
স্থির শূন্য
mouse_15c204(সংরচনা bregs *regs)
{
    u8 param[2];
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_GETID, param);
    যদি (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        প্রত্যাগম;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
স্থির শূন্য
mouse_15c205(সংরচনা bregs *regs)
{
    যদি (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        প্রত্যাগম;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
স্থির শূন্য
mouse_15c20600(সংরচনা bregs *regs)
{
    u8 param[3];
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    যদি (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        প্রত্যাগম;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
স্থির শূন্য
mouse_15c20601(সংরচনা bregs *regs)
{
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_SETSCALE11, রিক্ত);
    যদি (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    অন্যথা
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
স্থির শূন্য
mouse_15c20602(সংরচনা bregs *regs)
{
    পূর্ণাঙ্ক ret = mouse_command(PSMOUSE_CMD_SETSCALE21, রিক্ত);
    যদি (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    অন্যথা
        set_code_success(regs);
}

স্থির শূন্য
mouse_15c206XX(সংরচনা bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
স্থির শূন্য
mouse_15c206(সংরচনা bregs *regs)
{
    বিকল্প (regs->bh) {
    পক্ষ 0x00: mouse_15c20600(regs); ভঙ্গ;
    পক্ষ 0x01: mouse_15c20601(regs); ভঙ্গ;
    পক্ষ 0x02: mouse_15c20602(regs); ভঙ্গ;
    স্বভাব:   mouse_15c206XX(regs); ভঙ্গ;
    }
}

// Set Mouse Handler Address
স্থির শূন্য
mouse_15c207(সংরচনা bregs *regs)
{
    সংরচনা segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    যদি (! farptr.segoff) {
        /* remove handler */
        যদি ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, রিক্ত);
        }
    } অন্যথা {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

স্থির শূন্য
mouse_15c2XX(সংরচনা bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

শূন্য
handle_15c2(সংরচনা bregs *regs)
{
    //debug_stub(regs);

    যদি (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        প্রত্যাগম;
    }

    বিকল্প (regs->al) {
    পক্ষ 0x00: mouse_15c200(regs); ভঙ্গ;
    পক্ষ 0x01: mouse_15c201(regs); ভঙ্গ;
    পক্ষ 0x02: mouse_15c202(regs); ভঙ্গ;
    পক্ষ 0x03: mouse_15c203(regs); ভঙ্গ;
    পক্ষ 0x04: mouse_15c204(regs); ভঙ্গ;
    পক্ষ 0x05: mouse_15c205(regs); ভঙ্গ;
    পক্ষ 0x06: mouse_15c206(regs); ভঙ্গ;
    পক্ষ 0x07: mouse_15c207(regs); ভঙ্গ;
    স্বভাব:   mouse_15c2XX(regs); ভঙ্গ;
    }
}

শূন্য VISIBLE16
invoke_mouse_handler(শূন্য)
{
    যদি (!CONFIG_MOUSE)
        প্রত্যাগম;
    যদি (need_hop_back()) {
        স্তূপ_hop_back(invoke_mouse_handler, 0, 0);
        প্রত্যাগম;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    সংরচনা segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm চঞ্চল(
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

শূন্য
process_mouse(u8 data)
{
    যদি (!CONFIG_MOUSE)
        প্রত্যাগম;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    যদি (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        প্রত্যাগম;

    u8 package_গুনতি = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    যদি ((index+1) < package_গুনতি) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        প্রত্যাগম;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
