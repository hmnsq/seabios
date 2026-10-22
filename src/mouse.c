// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_EBDA
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "hw/ps2port.h" // ps2_mouse_command
#અન્તર્ભાવ "hw/usb-hid.h" // usb_mouse_command
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // stack_hop_back
#અન્તર્ભાવ "util.h" // mouse_init

શૂન્ય
mouse_init(શૂન્ય)
{
    જો (! CONFIG_MOUSE)
        પાછા;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

સ્થિર પૂર્ણાંક
mouse_command(પૂર્ણાંક command, u8 *param)
{
    જો (usb_mouse_active())
        પાછા usb_mouse_command(command, param);
    પાછા ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
સ્થિર શૂન્ય
mouse_15c20000(સંરચના bregs *regs)
{
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_DISABLE, ખાલી);
    જો (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    નહીંતર
        set_code_success(regs);
}

// Enable Mouse
સ્થિર શૂન્ય
mouse_15c20001(સંરચના bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    જો ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        પાછા;
    }

    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_ENABLE, ખાલી);
    જો (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    નહીંતર
        set_code_success(regs);
}

સ્થિર શૂન્ય
mouse_15c200XX(સંરચના bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
સ્થિર શૂન્ય
mouse_15c200(સંરચના bregs *regs)
{
    પસંદગી (regs->bh) {
    સ્થિતિ 0x00: mouse_15c20000(regs); તોડો;
    સ્થિતિ 0x01: mouse_15c20001(regs); તોડો;
    મૂળભૂત:   mouse_15c200XX(regs); તોડો;
    }
}

// Reset Mouse
સ્થિર શૂન્ય
mouse_15c201(સંરચના bregs *regs)
{
    u8 param[2];
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    જો (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        પાછા;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
સ્થિર શૂન્ય
mouse_15c202(સંરચના bregs *regs)
{
    સ્થિર u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    જો (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        પાછા;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    જો (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    નહીંતર
        set_code_success(regs);
}

// Set Resolution
સ્થિર શૂન્ય
mouse_15c203(સંરચના bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    જો (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        પાછા;
    }
    u8 param = regs->bh;
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    જો (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    નહીંતર
        set_code_success(regs);
}

// Get Device ID
સ્થિર શૂન્ય
mouse_15c204(સંરચના bregs *regs)
{
    u8 param[2];
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_GETID, param);
    જો (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        પાછા;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
સ્થિર શૂન્ય
mouse_15c205(સંરચના bregs *regs)
{
    જો (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        પાછા;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
સ્થિર શૂન્ય
mouse_15c20600(સંરચના bregs *regs)
{
    u8 param[3];
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    જો (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        પાછા;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
સ્થિર શૂન્ય
mouse_15c20601(સંરચના bregs *regs)
{
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_SETSCALE11, ખાલી);
    જો (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    નહીંતર
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
સ્થિર શૂન્ય
mouse_15c20602(સંરચના bregs *regs)
{
    પૂર્ણાંક ret = mouse_command(PSMOUSE_CMD_SETSCALE21, ખાલી);
    જો (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    નહીંતર
        set_code_success(regs);
}

સ્થિર શૂન્ય
mouse_15c206XX(સંરચના bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
સ્થિર શૂન્ય
mouse_15c206(સંરચના bregs *regs)
{
    પસંદગી (regs->bh) {
    સ્થિતિ 0x00: mouse_15c20600(regs); તોડો;
    સ્થિતિ 0x01: mouse_15c20601(regs); તોડો;
    સ્થિતિ 0x02: mouse_15c20602(regs); તોડો;
    મૂળભૂત:   mouse_15c206XX(regs); તોડો;
    }
}

// Set Mouse Handler Address
સ્થિર શૂન્ય
mouse_15c207(સંરચના bregs *regs)
{
    સંરચના segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    જો (! farptr.segoff) {
        /* remove handler */
        જો ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, ખાલી);
        }
    } નહીંતર {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

સ્થિર શૂન્ય
mouse_15c2XX(સંરચના bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

શૂન્ય
handle_15c2(સંરચના bregs *regs)
{
    //debug_stub(regs);

    જો (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        પાછા;
    }

    પસંદગી (regs->al) {
    સ્થિતિ 0x00: mouse_15c200(regs); તોડો;
    સ્થિતિ 0x01: mouse_15c201(regs); તોડો;
    સ્થિતિ 0x02: mouse_15c202(regs); તોડો;
    સ્થિતિ 0x03: mouse_15c203(regs); તોડો;
    સ્થિતિ 0x04: mouse_15c204(regs); તોડો;
    સ્થિતિ 0x05: mouse_15c205(regs); તોડો;
    સ્થિતિ 0x06: mouse_15c206(regs); તોડો;
    સ્થિતિ 0x07: mouse_15c207(regs); તોડો;
    મૂળભૂત:   mouse_15c2XX(regs); તોડો;
    }
}

શૂન્ય VISIBLE16
invoke_mouse_handler(શૂન્ય)
{
    જો (!CONFIG_MOUSE)
        પાછા;
    જો (need_hop_back()) {
        ઢગલો_hop_back(invoke_mouse_handler, 0, 0);
        પાછા;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    સંરચના segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm અસ્થિર(
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

શૂન્ય
process_mouse(u8 data)
{
    જો (!CONFIG_MOUSE)
        પાછા;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    જો (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        પાછા;

    u8 package_ગણતરી = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    જો ((index+1) < package_ગણતરી) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        પાછા;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
