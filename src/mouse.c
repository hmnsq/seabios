// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_EBDA
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "hw/ps2port.h" // ps2_mouse_command
#అన్తర్భావ "hw/usb-hid.h" // usb_mouse_command
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // stack_hop_back
#అన్తర్భావ "util.h" // mouse_init

శూన్య
mouse_init(శూన్య)
{
    ఒకవేళ (! CONFIG_MOUSE)
        తిరిగి;
    dprintf(3, "init mouse\n");
    // pointing device installed
    సమితి_equipment_flags(0x04, 0x04);
}

స్థిర పూర్ణాంకం
mouse_command(పూర్ణాంకం command, u8 *param)
{
    ఒకవేళ (usb_mouse_చురుకైన())
        తిరిగి usb_mouse_command(command, param);
    తిరిగి ps2_mouse_command(command, param);
}

#నిర్వచించు RET_SUCCESS      0x00
#నిర్వచించు RET_EINVFUNCTION 0x01
#నిర్వచించు RET_EINVINPUT    0x02
#నిర్వచించు RET_EINTERFACE   0x03
#నిర్వచించు RET_ENEEDRESEND  0x04
#నిర్వచించు RET_ENOHANDLER   0x05

// Disable Mouse
స్థిర శూన్య
mouse_15c20000(నిర్మాణం bregs *regs)
{
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_DISABLE, ఖాళీ);
    ఒకవేళ (ret)
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
    లేకపోతే
        సమితి_సంకేతం_success(regs);
}

// Enable Mouse
స్థిర శూన్య
mouse_15c20001(నిర్మాణం bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ఒకవేళ ((mouse_flags_2 & 0x80) == 0) {
        సమితి_సంకేతం_invalid(regs, RET_ENOHANDLER);
        తిరిగి;
    }

    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_ENABLE, ఖాళీ);
    ఒకవేళ (ret)
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
    లేకపోతే
        సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
mouse_15c200XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
స్థిర శూన్య
mouse_15c200(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->bh) {
    స్థితి 0x00: mouse_15c20000(regs); ఆపు;
    స్థితి 0x01: mouse_15c20001(regs); ఆపు;
    అప్రమేయ:   mouse_15c200XX(regs); ఆపు;
    }
}

// Reset Mouse
స్థిర శూన్య
mouse_15c201(నిర్మాణం bregs *regs)
{
    u8 param[2];
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    ఒకవేళ (ret) {
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
        తిరిగి;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    సమితి_సంకేతం_success(regs);
}

// Set Sample Rate
స్థిర శూన్య
mouse_15c202(నిర్మాణం bregs *regs)
{
    స్థిర u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    ఒకవేళ (regs->bh >= ARRAY_కొలత(sample_rates)) {
        సమితి_సంకేతం_invalid(regs, RET_EINVINPUT);
        తిరిగి;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    ఒకవేళ (ret)
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
    లేకపోతే
        సమితి_సంకేతం_success(regs);
}

// Set Resolution
స్థిర శూన్య
mouse_15c203(నిర్మాణం bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    ఒకవేళ (regs->bh >= 4) {
        సమితి_సంకేతం_invalid(regs, RET_EINVINPUT);
        తిరిగి;
    }
    u8 param = regs->bh;
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    ఒకవేళ (ret)
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
    లేకపోతే
        సమితి_సంకేతం_success(regs);
}

// Get Device ID
స్థిర శూన్య
mouse_15c204(నిర్మాణం bregs *regs)
{
    u8 param[2];
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_GETID, param);
    ఒకవేళ (ret) {
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
        తిరిగి;
    }
    regs->bh = param[0];
    సమితి_సంకేతం_success(regs);
}

// Initialize Mouse
స్థిర శూన్య
mouse_15c205(నిర్మాణం bregs *regs)
{
    ఒకవేళ (regs->bh != 3) {
        సమితి_సంకేతం_invalid(regs, RET_EINTERFACE);
        తిరిగి;
    }
    u16 ebda_seg = get_ebda_seg();
    సమితి_EBDA(ebda_seg, mouse_flag1, 0x00);
    సమితి_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
స్థిర శూన్య
mouse_15c20600(నిర్మాణం bregs *regs)
{
    u8 param[3];
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    ఒకవేళ (ret) {
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
        తిరిగి;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    సమితి_సంకేతం_success(regs);
}

// Set Scaling Factor to 1:1
స్థిర శూన్య
mouse_15c20601(నిర్మాణం bregs *regs)
{
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_SETSCALE11, ఖాళీ);
    ఒకవేళ (ret)
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
    లేకపోతే
        సమితి_సంకేతం_success(regs);
}

// Set Scaling Factor to 2:1
స్థిర శూన్య
mouse_15c20602(నిర్మాణం bregs *regs)
{
    పూర్ణాంకం ret = mouse_command(PSMOUSE_CMD_SETSCALE21, ఖాళీ);
    ఒకవేళ (ret)
        సమితి_సంకేతం_invalid(regs, RET_ENEEDRESEND);
    లేకపోతే
        సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
mouse_15c206XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
స్థిర శూన్య
mouse_15c206(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->bh) {
    స్థితి 0x00: mouse_15c20600(regs); ఆపు;
    స్థితి 0x01: mouse_15c20601(regs); ఆపు;
    స్థితి 0x02: mouse_15c20602(regs); ఆపు;
    అప్రమేయ:   mouse_15c206XX(regs); ఆపు;
    }
}

// Set Mouse Handler Address
స్థిర శూన్య
mouse_15c207(నిర్మాణం bregs *regs)
{
    నిర్మాణం segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ఒకవేళ (! farptr.segoff) {
        /* remove handler */
        ఒకవేళ ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, ఖాళీ);
        }
    } లేకపోతే {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    సమితి_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    సమితి_EBDA(ebda_seg, far_call_సూచిక, farptr);
    సమితి_సంకేతం_success(regs);
}

స్థిర శూన్య
mouse_15c2XX(నిర్మాణం bregs *regs)
{
    సమితి_సంకేతం_unimplemented(regs, RET_EINVFUNCTION);
}

శూన్య
handle_15c2(నిర్మాణం bregs *regs)
{
    //debug_stub(regs);

    ఒకవేళ (! CONFIG_MOUSE) {
        సమితి_సంకేతం_invalid(regs, RET_EUNSUPPORTED);
        తిరిగి;
    }

    ఎంపిక (regs->al) {
    స్థితి 0x00: mouse_15c200(regs); ఆపు;
    స్థితి 0x01: mouse_15c201(regs); ఆపు;
    స్థితి 0x02: mouse_15c202(regs); ఆపు;
    స్థితి 0x03: mouse_15c203(regs); ఆపు;
    స్థితి 0x04: mouse_15c204(regs); ఆపు;
    స్థితి 0x05: mouse_15c205(regs); ఆపు;
    స్థితి 0x06: mouse_15c206(regs); ఆపు;
    స్థితి 0x07: mouse_15c207(regs); ఆపు;
    అప్రమేయ:   mouse_15c2XX(regs); ఆపు;
    }
}

శూన్య VISIBLE16
invoke_mouse_handler(శూన్య)
{
    ఒకవేళ (!CONFIG_MOUSE)
        తిరిగి;
    ఒకవేళ (need_hop_వెనుక()) {
        దొంతర_hop_వెనుక(invoke_mouse_handler, 0, 0);
        తిరిగి;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 స్థితి_2 = GET_EBDA(ebda_seg, mouse_దత్తాంశం[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_దత్తాంశం[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_దత్తాంశం[2]);

    నిర్మాణం segoff_s func = GET_EBDA(ebda_seg, far_call_సూచిక);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , స్థితి_2, X, Y, func.seg, func.స్థానభ్రంశం_2);

    asm అస్థిర(
        "pushl %%ebp\n"
        "sti\n"

        "pushl %0\n"
        "pushw %w1\n"  //స్థితి
        "pushw %w2\n"  // X
        "pushw %w3\n"  // Y
        "pushw $0\n"   // Z
        "lcallw *8(%%esp)\n"
        "addl $12, %%esp\n"

        "cli\n"
        "cld\n"
        "popl %%ebp"
        : "+a"(func.segoff), "+c"(స్థితి_2), "+d"(X), "+b"(Y)
        :
        : "edi", "esi", "cc", "memory");
}

శూన్య
process_mouse(u8 దత్తాంశం)
{
    ఒకవేళ (!CONFIG_MOUSE)
        తిరిగి;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    ఒకవేళ (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        తిరిగి;

    u8 package_లెక్క = mouse_flags_2 & 0x07;
    u8 సూచీ = mouse_flags_1 & 0x07;
    సమితి_EBDA(ebda_seg, mouse_దత్తాంశం[సూచీ], దత్తాంశం);

    ఒకవేళ ((సూచీ+1) < package_లెక్క) {
        mouse_flags_1++;
        సమితి_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        తిరిగి;
    }

    సమితి_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
