// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_EBDA
#समावेश "bregs.h" // struct bregs
#समावेश "hw/ps2port.h" // ps2_mouse_command
#समावेश "hw/usb-hid.h" // usb_mouse_command
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // stack_hop_back
#समावेश "util.h" // mouse_init

व्योम
mouse_init(व्योम)
{
    अगर (! CONFIG_MOUSE)
        वापस;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

अटल पूर्णांक
mouse_command(पूर्णांक command, u8 *param)
{
    अगर (usb_mouse_active())
        वापस usb_mouse_command(command, param);
    वापस ps2_mouse_command(command, param);
}

#घोषणा RET_SUCCESS      0x00
#घोषणा RET_EINVFUNCTION 0x01
#घोषणा RET_EINVINPUT    0x02
#घोषणा RET_EINTERFACE   0x03
#घोषणा RET_ENEEDRESEND  0x04
#घोषणा RET_ENOHANDLER   0x05

// Disable Mouse
अटल व्योम
mouse_15c20000(काष्ठा bregs *regs)
{
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_DISABLE, शून्य);
    अगर (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Enable Mouse
अटल व्योम
mouse_15c20001(काष्ठा bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    अगर ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        वापस;
    }

    पूर्णांक ret = mouse_command(PSMOUSE_CMD_ENABLE, शून्य);
    अगर (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

अटल व्योम
mouse_15c200XX(काष्ठा bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
अटल व्योम
mouse_15c200(काष्ठा bregs *regs)
{
    चयन (regs->bh) {
    हाल 0x00: mouse_15c20000(regs); अवरोध;
    हाल 0x01: mouse_15c20001(regs); अवरोध;
    शेष:   mouse_15c200XX(regs); अवरोध;
    }
}

// Reset Mouse
अटल व्योम
mouse_15c201(काष्ठा bregs *regs)
{
    u8 param[2];
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    अगर (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        वापस;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
अटल व्योम
mouse_15c202(काष्ठा bregs *regs)
{
    अटल u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    अगर (regs->bh >= ARRAY_आकार(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        वापस;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    अगर (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Set Resolution
अटल व्योम
mouse_15c203(काष्ठा bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    अगर (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        वापस;
    }
    u8 param = regs->bh;
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    अगर (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Get Device ID
अटल व्योम
mouse_15c204(काष्ठा bregs *regs)
{
    u8 param[2];
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_GETID, param);
    अगर (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        वापस;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
अटल व्योम
mouse_15c205(काष्ठा bregs *regs)
{
    अगर (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        वापस;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
अटल व्योम
mouse_15c20600(काष्ठा bregs *regs)
{
    u8 param[3];
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    अगर (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        वापस;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
अटल व्योम
mouse_15c20601(काष्ठा bregs *regs)
{
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_SETSCALE11, शून्य);
    अगर (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
अटल व्योम
mouse_15c20602(काष्ठा bregs *regs)
{
    पूर्णांक ret = mouse_command(PSMOUSE_CMD_SETSCALE21, शून्य);
    अगर (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    अन्यथा
        set_code_success(regs);
}

अटल व्योम
mouse_15c206XX(काष्ठा bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
अटल व्योम
mouse_15c206(काष्ठा bregs *regs)
{
    चयन (regs->bh) {
    हाल 0x00: mouse_15c20600(regs); अवरोध;
    हाल 0x01: mouse_15c20601(regs); अवरोध;
    हाल 0x02: mouse_15c20602(regs); अवरोध;
    शेष:   mouse_15c206XX(regs); अवरोध;
    }
}

// Set Mouse Handler Address
अटल व्योम
mouse_15c207(काष्ठा bregs *regs)
{
    काष्ठा segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    अगर (! farptr.segoff) {
        /* remove handler */
        अगर ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, शून्य);
        }
    } अन्यथा {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

अटल व्योम
mouse_15c2XX(काष्ठा bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

व्योम
handle_15c2(काष्ठा bregs *regs)
{
    //debug_stub(regs);

    अगर (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        वापस;
    }

    चयन (regs->al) {
    हाल 0x00: mouse_15c200(regs); अवरोध;
    हाल 0x01: mouse_15c201(regs); अवरोध;
    हाल 0x02: mouse_15c202(regs); अवरोध;
    हाल 0x03: mouse_15c203(regs); अवरोध;
    हाल 0x04: mouse_15c204(regs); अवरोध;
    हाल 0x05: mouse_15c205(regs); अवरोध;
    हाल 0x06: mouse_15c206(regs); अवरोध;
    हाल 0x07: mouse_15c207(regs); अवरोध;
    शेष:   mouse_15c2XX(regs); अवरोध;
    }
}

व्योम VISIBLE16
invoke_mouse_handler(व्योम)
{
    अगर (!CONFIG_MOUSE)
        वापस;
    अगर (need_hop_back()) {
        ढेर_hop_back(invoke_mouse_handler, 0, 0);
        वापस;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_तथ्य[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_तथ्य[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_तथ्य[2]);

    काष्ठा segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    यंत्र अस्थिर(
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

व्योम
process_mouse(u8 तथ्य)
{
    अगर (!CONFIG_MOUSE)
        वापस;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    अगर (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        वापस;

    u8 package_गिनती = mouse_flags_2 & 0x07;
    u8 सूचकांक = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_तथ्य[सूचकांक], तथ्य);

    अगर ((सूचकांक+1) < package_गिनती) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        वापस;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
