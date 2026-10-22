// 16bit code to handle mouse events.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_EBDA
#include "bregs.h" // struct bregs
#include "hw/ps2port.h" // ps2_mouse_command
#include "hw/usb-hid.h" // usb_mouse_command
#include "output.h" // dprintf
#include "stacks.h" // stack_hop_back
#include "util.h" // mouse_init

void
mouse_init(void)
{
    만약 (! CONFIG_MOUSE)
        반환;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

static 정수
mouse_command(정수 command, u8 *param)
{
    만약 (usb_mouse_active())
        반환 usb_mouse_command(command, param);
    반환 ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
static void
mouse_15c20000(struct bregs *regs)
{
    정수 ret = mouse_command(PSMOUSE_CMD_DISABLE, NULL);
    만약 (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    아니면
        set_code_success(regs);
}

// Enable Mouse
static void
mouse_15c20001(struct bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    만약 ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        반환;
    }

    정수 ret = mouse_command(PSMOUSE_CMD_ENABLE, NULL);
    만약 (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    아니면
        set_code_success(regs);
}

static void
mouse_15c200XX(struct bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
static void
mouse_15c200(struct bregs *regs)
{
    switch (regs->bh) {
    case 0x00: mouse_15c20000(regs); break;
    case 0x01: mouse_15c20001(regs); break;
    default:   mouse_15c200XX(regs); break;
    }
}

// Reset Mouse
static void
mouse_15c201(struct bregs *regs)
{
    u8 param[2];
    정수 ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    만약 (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        반환;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
static void
mouse_15c202(struct bregs *regs)
{
    static u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    만약 (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        반환;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    정수 ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    만약 (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    아니면
        set_code_success(regs);
}

// Set Resolution
static void
mouse_15c203(struct bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    만약 (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        반환;
    }
    u8 param = regs->bh;
    정수 ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    만약 (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    아니면
        set_code_success(regs);
}

// Get Device ID
static void
mouse_15c204(struct bregs *regs)
{
    u8 param[2];
    정수 ret = mouse_command(PSMOUSE_CMD_GETID, param);
    만약 (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        반환;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
static void
mouse_15c205(struct bregs *regs)
{
    만약 (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        반환;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
static void
mouse_15c20600(struct bregs *regs)
{
    u8 param[3];
    정수 ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    만약 (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        반환;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
static void
mouse_15c20601(struct bregs *regs)
{
    정수 ret = mouse_command(PSMOUSE_CMD_SETSCALE11, NULL);
    만약 (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    아니면
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
static void
mouse_15c20602(struct bregs *regs)
{
    정수 ret = mouse_command(PSMOUSE_CMD_SETSCALE21, NULL);
    만약 (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    아니면
        set_code_success(regs);
}

static void
mouse_15c206XX(struct bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
static void
mouse_15c206(struct bregs *regs)
{
    switch (regs->bh) {
    case 0x00: mouse_15c20600(regs); break;
    case 0x01: mouse_15c20601(regs); break;
    case 0x02: mouse_15c20602(regs); break;
    default:   mouse_15c206XX(regs); break;
    }
}

// Set Mouse Handler Address
static void
mouse_15c207(struct bregs *regs)
{
    struct segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    만약 (! farptr.segoff) {
        /* remove handler */
        만약 ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, NULL);
        }
    } 아니면 {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

static void
mouse_15c2XX(struct bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

void
handle_15c2(struct bregs *regs)
{
    //debug_stub(regs);

    만약 (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        반환;
    }

    switch (regs->al) {
    case 0x00: mouse_15c200(regs); break;
    case 0x01: mouse_15c201(regs); break;
    case 0x02: mouse_15c202(regs); break;
    case 0x03: mouse_15c203(regs); break;
    case 0x04: mouse_15c204(regs); break;
    case 0x05: mouse_15c205(regs); break;
    case 0x06: mouse_15c206(regs); break;
    case 0x07: mouse_15c207(regs); break;
    default:   mouse_15c2XX(regs); break;
    }
}

void VISIBLE16
invoke_mouse_handler(void)
{
    만약 (!CONFIG_MOUSE)
        반환;
    만약 (need_hop_back()) {
        stack_hop_back(invoke_mouse_handler, 0, 0);
        반환;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    struct segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm volatile(
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

void
process_mouse(u8 data)
{
    만약 (!CONFIG_MOUSE)
        반환;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    만약 (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        반환;

    u8 package_count = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    만약 ((index+1) < package_count) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        반환;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
