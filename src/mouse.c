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
    ប្រសិនបើ (! CONFIG_MOUSE)
        ត្រឡប់;
    dprintf(3, "init mouse\n");
    // pointing device installed
    set_equipment_flags(0x04, 0x04);
}

static ចំនួនគត់
mouse_command(ចំនួនគត់ command, u8 *param)
{
    ប្រសិនបើ (usb_mouse_active())
        ត្រឡប់ usb_mouse_command(command, param);
    ត្រឡប់ ps2_mouse_command(command, param);
}

#define RET_SUCCESS      0x00
#define RET_EINVFUNCTION 0x01
#define RET_EINVINPUT    0x02
#define RET_EINTERFACE   0x03
#define RET_ENEEDRESEND  0x04
#define RET_ENOHANDLER   0x05

// Disable Mouse
static void
mouse_15c20000(sa_nracanaa bregs *regs)
{
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_DISABLE, NULL);
    ប្រសិនបើ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    បើមិនដូច្នេះ
        set_code_success(regs);
}

// Enable Mouse
static void
mouse_15c20001(sa_nracanaa bregs *regs)
{
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ប្រសិនបើ ((mouse_flags_2 & 0x80) == 0) {
        set_code_invalid(regs, RET_ENOHANDLER);
        ត្រឡប់;
    }

    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_ENABLE, NULL);
    ប្រសិនបើ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    បើមិនដូច្នេះ
        set_code_success(regs);
}

static void
mouse_15c200XX(sa_nracanaa bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Disable/Enable Mouse
static void
mouse_15c200(sa_nracanaa bregs *regs)
{
    switch (regs->bh) {
    pak_sa 0x00: mouse_15c20000(regs); bha_nga;
    pak_sa 0x01: mouse_15c20001(regs); bha_nga;
    default:   mouse_15c200XX(regs); bha_nga;
    }
}

// Reset Mouse
static void
mouse_15c201(sa_nracanaa bregs *regs)
{
    u8 param[2];
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_RESET_BAT, param);
    ប្រសិនបើ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ត្រឡប់;
    }
    regs->bl = param[0];
    regs->bh = param[1];
    set_code_success(regs);
}

// Set Sample Rate
static void
mouse_15c202(sa_nracanaa bregs *regs)
{
    static u8 VAR16 sample_rates[7] = {10, 20, 40, 60, 80, 100, 200};
    ប្រសិនបើ (regs->bh >= ARRAY_SIZE(sample_rates)) {
        set_code_invalid(regs, RET_EINVINPUT);
        ត្រឡប់;
    }
    u8 mouse_data1 = GET_GLOBAL(sample_rates[regs->bh]);
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_SETRATE, &mouse_data1);
    ប្រសិនបើ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    បើមិនដូច្នេះ
        set_code_success(regs);
}

// Set Resolution
static void
mouse_15c203(sa_nracanaa bregs *regs)
{
    // BH:
    //      0 =  25 dpi, 1 count  per millimeter
    //      1 =  50 dpi, 2 counts per millimeter
    //      2 = 100 dpi, 4 counts per millimeter
    //      3 = 200 dpi, 8 counts per millimeter
    ប្រសិនបើ (regs->bh >= 4) {
        set_code_invalid(regs, RET_EINVINPUT);
        ត្រឡប់;
    }
    u8 param = regs->bh;
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_SETRES, &param);
    ប្រសិនបើ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    បើមិនដូច្នេះ
        set_code_success(regs);
}

// Get Device ID
static void
mouse_15c204(sa_nracanaa bregs *regs)
{
    u8 param[2];
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_GETID, param);
    ប្រសិនបើ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ត្រឡប់;
    }
    regs->bh = param[0];
    set_code_success(regs);
}

// Initialize Mouse
static void
mouse_15c205(sa_nracanaa bregs *regs)
{
    ប្រសិនបើ (regs->bh != 3) {
        set_code_invalid(regs, RET_EINTERFACE);
        ត្រឡប់;
    }
    u16 ebda_seg = get_ebda_seg();
    SET_EBDA(ebda_seg, mouse_flag1, 0x00);
    SET_EBDA(ebda_seg, mouse_flag2, regs->bh);

    // Reset Mouse
    mouse_15c201(regs);
}

// Return Status
static void
mouse_15c20600(sa_nracanaa bregs *regs)
{
    u8 param[3];
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_GETINFO, param);
    ប្រសិនបើ (ret) {
        set_code_invalid(regs, RET_ENEEDRESEND);
        ត្រឡប់;
    }
    regs->bl = param[0];
    regs->cl = param[1];
    regs->dl = param[2];
    set_code_success(regs);
}

// Set Scaling Factor to 1:1
static void
mouse_15c20601(sa_nracanaa bregs *regs)
{
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_SETSCALE11, NULL);
    ប្រសិនបើ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    បើមិនដូច្នេះ
        set_code_success(regs);
}

// Set Scaling Factor to 2:1
static void
mouse_15c20602(sa_nracanaa bregs *regs)
{
    ចំនួនគត់ ret = mouse_command(PSMOUSE_CMD_SETSCALE21, NULL);
    ប្រសិនបើ (ret)
        set_code_invalid(regs, RET_ENEEDRESEND);
    បើមិនដូច្នេះ
        set_code_success(regs);
}

static void
mouse_15c206XX(sa_nracanaa bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

// Return Status & Set Scaling Factor...
static void
mouse_15c206(sa_nracanaa bregs *regs)
{
    switch (regs->bh) {
    pak_sa 0x00: mouse_15c20600(regs); bha_nga;
    pak_sa 0x01: mouse_15c20601(regs); bha_nga;
    pak_sa 0x02: mouse_15c20602(regs); bha_nga;
    default:   mouse_15c206XX(regs); bha_nga;
    }
}

// Set Mouse Handler Address
static void
mouse_15c207(sa_nracanaa bregs *regs)
{
    sa_nracanaa segoff_s farptr = SEGOFF(regs->es, regs->bx);
    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);
    ប្រសិនបើ (! farptr.segoff) {
        /* remove handler */
        ប្រសិនបើ ((mouse_flags_2 & 0x80) != 0) {
            mouse_flags_2 &= ~0x80;
            mouse_command(PSMOUSE_CMD_DISABLE, NULL);
        }
    } បើមិនដូច្នេះ {
        /* install handler */
        mouse_flags_2 |= 0x80;
    }
    SET_EBDA(ebda_seg, mouse_flag2, mouse_flags_2);
    SET_EBDA(ebda_seg, far_call_pointer, farptr);
    set_code_success(regs);
}

static void
mouse_15c2XX(sa_nracanaa bregs *regs)
{
    set_code_unimplemented(regs, RET_EINVFUNCTION);
}

void
handle_15c2(sa_nracanaa bregs *regs)
{
    //debug_stub(regs);

    ប្រសិនបើ (! CONFIG_MOUSE) {
        set_code_invalid(regs, RET_EUNSUPPORTED);
        ត្រឡប់;
    }

    switch (regs->al) {
    pak_sa 0x00: mouse_15c200(regs); bha_nga;
    pak_sa 0x01: mouse_15c201(regs); bha_nga;
    pak_sa 0x02: mouse_15c202(regs); bha_nga;
    pak_sa 0x03: mouse_15c203(regs); bha_nga;
    pak_sa 0x04: mouse_15c204(regs); bha_nga;
    pak_sa 0x05: mouse_15c205(regs); bha_nga;
    pak_sa 0x06: mouse_15c206(regs); bha_nga;
    pak_sa 0x07: mouse_15c207(regs); bha_nga;
    default:   mouse_15c2XX(regs); bha_nga;
    }
}

void VISIBLE16
invoke_mouse_handler(void)
{
    ប្រសិនបើ (!CONFIG_MOUSE)
        ត្រឡប់;
    ប្រសិនបើ (need_hop_back()) {
        stack_hop_back(invoke_mouse_handler, 0, 0);
        ត្រឡប់;
    }
    ASSERT16();
    u16 ebda_seg = get_ebda_seg();
    u16 status = GET_EBDA(ebda_seg, mouse_data[0]);
    u16 X      = GET_EBDA(ebda_seg, mouse_data[1]);
    u16 Y      = GET_EBDA(ebda_seg, mouse_data[2]);

    sa_nracanaa segoff_s func = GET_EBDA(ebda_seg, far_call_pointer);
    dprintf(16, "mouse farcall s=%04x x=%04x y=%04x func=%04x:%04x\n"
            , status, X, Y, func.seg, func.offset);

    asm ca_ncala(
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
    ប្រសិនបើ (!CONFIG_MOUSE)
        ត្រឡប់;

    u16 ebda_seg = get_ebda_seg();
    u8 mouse_flags_1 = GET_EBDA(ebda_seg, mouse_flag1);
    u8 mouse_flags_2 = GET_EBDA(ebda_seg, mouse_flag2);

    ប្រសិនបើ (! (mouse_flags_2 & 0x80))
        // far call handler not installed
        ត្រឡប់;

    u8 package_count = mouse_flags_2 & 0x07;
    u8 index = mouse_flags_1 & 0x07;
    SET_EBDA(ebda_seg, mouse_data[index], data);

    ប្រសិនបើ ((index+1) < package_count) {
        mouse_flags_1++;
        SET_EBDA(ebda_seg, mouse_flag1, mouse_flags_1);
        ត្រឡប់;
    }

    SET_EBDA(ebda_seg, mouse_flag1, 0);
    invoke_mouse_handler();
}
