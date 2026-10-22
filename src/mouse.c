// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_EBDA
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "hw/ps2port.h" // ps2_mouse_command
#ଅନ୍ତର୍ଭାଵ "hw/usb-hid.h" // usb_mouse_command
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "stacks.h" // stack_hop_back
#ଅନ୍ତର୍ଭାଵ "util.h" // mouse_init

ଶୂନ୍ୟ
mouse_init(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_MOUSE)
        ଫେରନ୍ତୁ;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
mouse_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    ଯଦି (usb_mouse_active())
        ଫେରନ୍ତୁ usb_mouse_command(command, param);
    ଫେରନ୍ତୁ ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c20000(ସଂରଚନା bregs *regs)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_DISABLE, ଖାଲି);
    ଯଦି (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ଅନ୍ୟଥା
        set_code_success(regs);
}

// Enable Mouse
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c20001(ସଂରଚନା bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ଯଦି ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        ଫେରନ୍ତୁ;
    }

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_ENABLE, ଖାଲି);
    ଯଦି (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ଅନ୍ୟଥା
        set_code_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c200XX(ସଂରଚନା bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c200(ସଂରଚନା bregs *regs)
{
    ବାଛ (regs->bh) {
    ସ୍ଥିତି 0x00: mouse_15c20000(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: mouse_15c20001(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   mouse_15c200XX(regs); ଭାଙ୍ଗ;
    }
}

// Reset Mouse
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c201(ସଂରଚନା bregs *regs)
{
    u8 param[2];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    ଯଦି (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ଫେରନ୍ତୁ;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c202(ସଂରଚନା bregs *regs)
{
    ସ୍ଥିର u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    ଯଦି (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        ଫେରନ୍ତୁ;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    ଯଦି (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ଅନ୍ୟଥା
        set_code_success(regs);
}

// Set Resolution
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c203(ସଂରଚନା bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    ଯଦି (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        ଫେରନ୍ତୁ;
    }
    u8 param = regs->bh;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    ଯଦି (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ଅନ୍ୟଥା
        set_code_success(regs);
}

// Get Device ID
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c204(ସଂରଚନା bregs *regs)
{
    u8 param[2];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_GETID, param);
    ଯଦି (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ଫେରନ୍ତୁ;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c205(ସଂରଚନା bregs *regs)
{
    ଯଦି (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        ଫେରନ୍ତୁ;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c20600(ସଂରଚନା bregs *regs)
{
    u8 param[3];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    ଯଦି (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ଫେରନ୍ତୁ;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c20601(ସଂରଚନା bregs *regs)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_SETSCALE11, ଖାଲି);
    ଯଦି (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ଅନ୍ୟଥା
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c20602(ସଂରଚନା bregs *regs)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = mouse_command(PSMOUSE_CMD_SETSCALE21, ଖାଲି);
    ଯଦି (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    ଅନ୍ୟଥା
        set_code_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c206XX(ସଂରଚନା bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c206(ସଂରଚନା bregs *regs)
{
    ବାଛ (regs->bh) {
    ସ୍ଥିତି 0x00: mouse_15c20600(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: mouse_15c20601(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: mouse_15c20602(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   mouse_15c206XX(regs); ଭାଙ୍ଗ;
    }
}

// Set Mouse Handler Address
ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c207(ସଂରଚନା bregs *regs)
{
    ସଂରଚନା segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ଯଦି (! farptr.segoff) {
        /* remove handler */
        ଯଦି ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, ଖାଲି);
        }
    } ଅନ୍ୟଥା {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

ସ୍ଥିର ଶୂନ୍ୟ
mouse_15c2XX(ସଂରଚନା bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

ଶୂନ୍ୟ
handle_15c2(ସଂରଚନା bregs *regs)
{
    //debug_stub(regs);

    ଯଦି (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ଫେରନ୍ତୁ;
    }

    ବାଛ (regs->al) {
    ସ୍ଥିତି 0x00: mouse_15c200(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: mouse_15c201(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: mouse_15c202(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: mouse_15c203(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x04: mouse_15c204(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x05: mouse_15c205(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x06: mouse_15c206(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x07: mouse_15c207(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   mouse_15c2XX(regs); ଭାଙ୍ଗ;
    }
}

ଶୂନ୍ୟ VISIBLE16
invoke_mouse_handler(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_MOUSE)
        ଫେରନ୍ତୁ;
    ଯଦି (need_hop_back()) {
        ଗଦା_hop_back(invoke_mouse_handler, 0, 0);
        ଫେରନ୍ତୁ;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    ସଂରଚନା segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm ଅସ୍ଥିର(
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

ଶୂନ୍ୟ
process_mouse(u8 data)
{
    ଯଦି (!CONFIG_MOUSE)
        ଫେରନ୍ତୁ;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    ଯଦି (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        ଫେରନ୍ତୁ;

    u8 package_ଗଣନା = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    ଯଦି ((index+1) < package_ଗଣନା) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        ଫେରନ୍ତୁ;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
