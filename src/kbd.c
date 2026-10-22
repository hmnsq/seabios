// 16bit code to handle keyboard requests.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_BDA
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "hw/ps2port.h" // ps2_kbd_command
#অন্তর্ভাব "hw/usb-hid.h" // usb_kbd_command
#অন্তর্ভাব "output.h" // debug_enter
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // kbd_init

শূন্য
kbd_init(শূন্য)
{
    dprintf(3, "init keyboard\n");
    u16 x = offsetof(কাঠামো bios_তথ্য_area_s, kbd_buf);
    SET_BDA(kbd_flag1, KF1_101KBD);
    SET_BDA(kbd_buf_শীর্ষ, x);
    SET_BDA(kbd_buf_লেজ, x);
    SET_BDA(kbd_buf_start_offset, x);

    SET_BDA(kbd_buf_end_offset
            , x + FIELD_SIZEOF(কাঠামো bios_তথ্য_area_s, kbd_buf));
}

u8
enqueue_চাবি(u16 keycode)
{
    u16 বাফার_start = GET_BDA(kbd_buf_start_offset);
    u16 বাফার_end   = GET_BDA(kbd_buf_end_offset);

    u16 বাফার_শীর্ষ = GET_BDA(kbd_buf_শীর্ষ);
    u16 বাফার_লেজ = GET_BDA(kbd_buf_লেজ);

    u16 অস্থায়ী_লেজ = বাফার_লেজ;
    বাফার_লেজ += 2;
    যদি (বাফার_লেজ >= বাফার_end)
        বাফার_লেজ = বাফার_start;

    যদি (বাফার_লেজ == বাফার_শীর্ষ)
        ফেরত 0;

    SET_FARVAR(SEG_BDA, *(u16*)(অস্থায়ী_লেজ+0), keycode);
    SET_BDA(kbd_buf_লেজ, বাফার_লেজ);
    ফেরত 1;
}

স্থির শূন্য
dequeue_চাবি(কাঠামো bregs *regs, পূর্ণসংখ্যা incr, পূর্ণসংখ্যা extended)
{
    yield();
    u16 বাফার_শীর্ষ;
    u16 বাফার_লেজ;
    জন্য (;;) {
        বাফার_শীর্ষ = GET_BDA(kbd_buf_শীর্ষ);
        বাফার_লেজ = GET_BDA(kbd_buf_লেজ);

        যদি (বাফার_শীর্ষ != বাফার_লেজ)
            বিরতি;
        যদি (!incr) {
            regs->flags |= F_ZF;
            ফেরত;
        }
        yield_toirq();
    }

    u16 keycode = GET_FARVAR(SEG_BDA, *(u16*)(বাফার_শীর্ষ+0));
    u8 ascii = keycode & 0xff;
    যদি (!extended) {
        // Translate extended keys
        যদি (ascii == 0xe0 && keycode & 0xff00)
            keycode &= 0xff00;
        অন্যথা যদি (keycode == 0xe00d || keycode == 0xe00a)
            // Extended enter key
            keycode = 0x1c00 | ascii;
        অন্যথা যদি (keycode == 0xe02f)
            // Extended '/' key
            keycode = 0x352f;
        // Technically, if the ascii value is 0xf0 or if the
        // 'scancode' is greater than 0x84 then the key should be
        // discarded.  However, there seems no harm in passing on the
        // extended values in these cases.
    }
    যদি (ascii == 0xf0 && keycode & 0xff00)
        keycode &= 0xff00;
    regs->ax = keycode;

    যদি (!incr) {
        regs->flags &= ~F_ZF;
        ফেরত;
    }
    u16 বাফার_start = GET_BDA(kbd_buf_start_offset);
    u16 বাফার_end   = GET_BDA(kbd_buf_end_offset);

    বাফার_শীর্ষ += 2;
    যদি (বাফার_শীর্ষ >= বাফার_end)
        বাফার_শীর্ষ = বাফার_start;
    SET_BDA(kbd_buf_শীর্ষ, বাফার_শীর্ষ);
}

স্থির পূর্ণসংখ্যা
kbd_command(পূর্ণসংখ্যা command, u8 *param)
{
    যদি (usb_kbd_active())
        ফেরত usb_kbd_command(command, param);
    ফেরত ps2_kbd_command(command, param);
}

// read keyboard input
স্থির শূন্য
handle_1600(কাঠামো bregs *regs)
{
    dequeue_চাবি(regs, 1, 0);
}

// check keyboard status
স্থির শূন্য
handle_1601(কাঠামো bregs *regs)
{
    dequeue_চাবি(regs, 0, 0);
}

// get shift flag status
স্থির শূন্য
handle_1602(কাঠামো bregs *regs)
{
    yield();
    regs->al = GET_BDA(kbd_flag0);
}

// store key-stroke into buffer
স্থির শূন্য
handle_1605(কাঠামো bregs *regs)
{
    regs->al = !enqueue_চাবি(regs->cx);
}

// GET KEYBOARD FUNCTIONALITY
স্থির শূন্য
handle_1609(কাঠামো bregs *regs)
{
    // bit Bochs Description
    //  7    0   reserved
    //  6    0   INT 16/AH=20h-22h supported (122-key keyboard support)
    //  5    1   INT 16/AH=10h-12h supported (enhanced keyboard support)
    //  4    1   INT 16/AH=0Ah supported
    //  3    0   INT 16/AX=0306h supported
    //  2    0   INT 16/AX=0305h supported
    //  1    0   INT 16/AX=0304h supported
    //  0    0   INT 16/AX=0300h supported
    //
    regs->al = 0x30;
}

// GET KEYBOARD ID
স্থির শূন্য noinline
handle_160a(কাঠামো bregs *regs)
{
    u8 param[2];
    পূর্ণসংখ্যা ret = kbd_command(ATKBD_CMD_GETID, param);
    যদি (ret) {
        regs->bx = 0;
        ফেরত;
    }
    regs->bx = (param[1] << 8) | param[0];
}

// read MF-II keyboard input
স্থির শূন্য
handle_1610(কাঠামো bregs *regs)
{
    dequeue_চাবি(regs, 1, 1);
}

// check MF-II keyboard status
স্থির শূন্য
handle_1611(কাঠামো bregs *regs)
{
    dequeue_চাবি(regs, 0, 1);
}

// get extended keyboard status
স্থির শূন্য
handle_1612(কাঠামো bregs *regs)
{
    yield();
    regs->ax = ((GET_BDA(kbd_flag0) & ~((KF1_RCTRL|KF1_RALT) << 8))
                | ((GET_BDA(kbd_flag1) & (KF1_RCTRL|KF1_RALT)) << 8));
    //BX_DEBUG_INT16("int16: func 12 sending %04x\n",AX);
}

স্থির শূন্য
handle_166f(কাঠামো bregs *regs)
{
    যদি (regs->al == 0x08)
        // unsupported, aka normal keyboard
        regs->ah = 2;
}

// keyboard capability check called by DOS 5.0+ keyb
স্থির শূন্য
handle_1692(কাঠামো bregs *regs)
{
    // function int16 ah=0x10-0x12 supported
    regs->ah = 0x80;
}

// 122 keys capability check called by DOS 5.0+ keyb
স্থির শূন্য
handle_16a2(কাঠামো bregs *regs)
{
    // don't change AH : function int16 ah=0x20-0x22 NOT supported
}

স্থির শূন্য
handle_16XX(কাঠামো bregs *regs)
{
    warn_unimplemented(regs);
}

স্থির শূন্য noinline
set_leds(শূন্য)
{
    u8 shift_flags = (GET_BDA(kbd_flag0) >> 4) & 0x07;
    u8 kbd_led = GET_BDA(kbd_led);
    u8 led_flags = kbd_led & 0x07;
    যদি (shift_flags == led_flags)
        ফেরত;

    পূর্ণসংখ্যা ret = kbd_command(ATKBD_CMD_SETLEDS, &shift_flags);
    যদি (ret)
        // Error
        ফেরত;
    kbd_led = (kbd_led & ~0x07) | shift_flags;
    SET_BDA(kbd_led, kbd_led);
}

// INT 16h Keyboard Service Entry Point
শূন্য VISIBLE16
handle_16(কাঠামো bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_16);
    যদি (! CONFIG_KEYBOARD)
        ফেরত;

    // XXX - set_leds should be called from irq handler
    set_leds();

    নির্বাচন (regs->ah) {
    ক্ষেত্র 0x00: handle_1600(regs); বিরতি;
    ক্ষেত্র 0x01: handle_1601(regs); বিরতি;
    ক্ষেত্র 0x02: handle_1602(regs); বিরতি;
    ক্ষেত্র 0x05: handle_1605(regs); বিরতি;
    ক্ষেত্র 0x09: handle_1609(regs); বিরতি;
    ক্ষেত্র 0x0a: handle_160a(regs); বিরতি;
    ক্ষেত্র 0x10: handle_1610(regs); বিরতি;
    ক্ষেত্র 0x11: handle_1611(regs); বিরতি;
    ক্ষেত্র 0x12: handle_1612(regs); বিরতি;
    ক্ষেত্র 0x92: handle_1692(regs); বিরতি;
    ক্ষেত্র 0xa2: handle_16a2(regs); বিরতি;
    ক্ষেত্র 0x6f: handle_166f(regs); বিরতি;
    পূর্বনির্ধারিত:   handle_16XX(regs); বিরতি;
    }
}

#define none 0

স্থির কাঠামো scaninfo {
    u16 normal;
    u16 shift;
    u16 control;
    u16 alt;
} scan_to_keycode[] VAR16 = {
    {   none,   none,   none,   none },
    { 0x011b, 0x011b, 0x011b, 0x01f0 }, /* escape */
    { 0x0231, 0x0221,   none, 0x7800 }, /* 1! */
    { 0x0332, 0x0340, 0x0300, 0x7900 }, /* 2@ */
    { 0x0433, 0x0423,   none, 0x7a00 }, /* 3# */
    { 0x0534, 0x0524,   none, 0x7b00 }, /* 4$ */
    { 0x0635, 0x0625,   none, 0x7c00 }, /* 5% */
    { 0x0736, 0x075e, 0x071e, 0x7d00 }, /* 6^ */
    { 0x0837, 0x0826,   none, 0x7e00 }, /* 7& */
    { 0x0938, 0x092a,   none, 0x7f00 }, /* 8* */
    { 0x0a39, 0x0a28,   none, 0x8000 }, /* 9( */
    { 0x0b30, 0x0b29,   none, 0x8100 }, /* 0) */
    { 0x0c2d, 0x0c5f, 0x0c1f, 0x8200 }, /* -_ */
    { 0x0d3d, 0x0d2b,   none, 0x8300 }, /* =+ */
    { 0x0e08, 0x0e08, 0x0e7f, 0x0ef0 }, /* backspace */
    { 0x0f09, 0x0f00, 0x9400, 0xa5f0 }, /* tab */
    { 0x1071, 0x1051, 0x1011, 0x1000 }, /* Q */
    { 0x1177, 0x1157, 0x1117, 0x1100 }, /* W */
    { 0x1265, 0x1245, 0x1205, 0x1200 }, /* E */
    { 0x1372, 0x1352, 0x1312, 0x1300 }, /* R */
    { 0x1474, 0x1454, 0x1414, 0x1400 }, /* T */
    { 0x1579, 0x1559, 0x1519, 0x1500 }, /* Y */
    { 0x1675, 0x1655, 0x1615, 0x1600 }, /* U */
    { 0x1769, 0x1749, 0x1709, 0x1700 }, /* I */
    { 0x186f, 0x184f, 0x180f, 0x1800 }, /* O */
    { 0x1970, 0x1950, 0x1910, 0x1900 }, /* P */
    { 0x1a5b, 0x1a7b, 0x1a1b, 0x1af0 }, /* [{ */
    { 0x1b5d, 0x1b7d, 0x1b1d, 0x1bf0 }, /* ]} */
    { 0x1c0d, 0x1c0d, 0x1c0a, 0x1cf0 }, /* Enter */
    {   none,   none,   none,   none }, /* L Ctrl */
    { 0x1e61, 0x1e41, 0x1e01, 0x1e00 }, /* A */
    { 0x1f73, 0x1f53, 0x1f13, 0x1f00 }, /* S */
    { 0x2064, 0x2044, 0x2004, 0x2000 }, /* D */
    { 0x2166, 0x2146, 0x2106, 0x2100 }, /* F */
    { 0x2267, 0x2247, 0x2207, 0x2200 }, /* G */
    { 0x2368, 0x2348, 0x2308, 0x2300 }, /* H */
    { 0x246a, 0x244a, 0x240a, 0x2400 }, /* J */
    { 0x256b, 0x254b, 0x250b, 0x2500 }, /* K */
    { 0x266c, 0x264c, 0x260c, 0x2600 }, /* L */
    { 0x273b, 0x273a,   none, 0x27f0 }, /* ;: */
    { 0x2827, 0x2822,   none, 0x28f0 }, /* '" */
    { 0x2960, 0x297e,   none, 0x29f0 }, /* `~ */
    {   none,   none,   none,   none }, /* L shift */
    { 0x2b5c, 0x2b7c, 0x2b1c, 0x2bf0 }, /* |\ */
    { 0x2c7a, 0x2c5a, 0x2c1a, 0x2c00 }, /* Z */
    { 0x2d78, 0x2d58, 0x2d18, 0x2d00 }, /* X */
    { 0x2e63, 0x2e43, 0x2e03, 0x2e00 }, /* C */
    { 0x2f76, 0x2f56, 0x2f16, 0x2f00 }, /* V */
    { 0x3062, 0x3042, 0x3002, 0x3000 }, /* B */
    { 0x316e, 0x314e, 0x310e, 0x3100 }, /* N */
    { 0x326d, 0x324d, 0x320d, 0x3200 }, /* M */
    { 0x332c, 0x333c,   none, 0x33f0 }, /* ,< */
    { 0x342e, 0x343e,   none, 0x34f0 }, /* .> */
    { 0x352f, 0x353f,   none, 0x35f0 }, /* /? */
    {   none,   none,   none,   none }, /* R Shift */
    { 0x372a, 0x372a, 0x9600, 0x37f0 }, /* * */
    {   none,   none,   none,   none }, /* L Alt */
    { 0x3920, 0x3920, 0x3920, 0x3920 }, /* space */
    {   none,   none,   none,   none }, /* caps lock */
    { 0x3b00, 0x5400, 0x5e00, 0x6800 }, /* F1 */
    { 0x3c00, 0x5500, 0x5f00, 0x6900 }, /* F2 */
    { 0x3d00, 0x5600, 0x6000, 0x6a00 }, /* F3 */
    { 0x3e00, 0x5700, 0x6100, 0x6b00 }, /* F4 */
    { 0x3f00, 0x5800, 0x6200, 0x6c00 }, /* F5 */
    { 0x4000, 0x5900, 0x6300, 0x6d00 }, /* F6 */
    { 0x4100, 0x5a00, 0x6400, 0x6e00 }, /* F7 */
    { 0x4200, 0x5b00, 0x6500, 0x6f00 }, /* F8 */
    { 0x4300, 0x5c00, 0x6600, 0x7000 }, /* F9 */
    { 0x4400, 0x5d00, 0x6700, 0x7100 }, /* F10 */
    {   none,   none,   none,   none }, /* Num Lock */
    {   none,   none,   none,   none }, /* Scroll Lock */
    { 0x4700, 0x4737, 0x7700,   none }, /* 7 Home */
    { 0x4800, 0x4838, 0x8d00,   none }, /* 8 UP */
    { 0x4900, 0x4939, 0x8400,   none }, /* 9 PgUp */
    { 0x4a2d, 0x4a2d, 0x8e00, 0x4af0 }, /* - */
    { 0x4b00, 0x4b34, 0x7300,   none }, /* 4 Left */
    { 0x4c00, 0x4c35, 0x8f00,   none }, /* 5 */
    { 0x4d00, 0x4d36, 0x7400,   none }, /* 6 Right */
    { 0x4e2b, 0x4e2b, 0x9000, 0x4ef0 }, /* + */
    { 0x4f00, 0x4f31, 0x7500,   none }, /* 1 End */
    { 0x5000, 0x5032, 0x9100,   none }, /* 2 Down */
    { 0x5100, 0x5133, 0x7600,   none }, /* 3 PgDn */
    { 0x5200, 0x5230, 0x9200,   none }, /* 0 Ins */
    { 0x5300, 0x532e, 0x9300,   none }, /* Del */
    {   none,   none,   none,   none }, /* SysReq */
    {   none,   none,   none,   none },
    { 0x565c, 0x567c,   none,   none }, /* \| */
    { 0x8500, 0x8700, 0x8900, 0x8b00 }, /* F11 */
    { 0x8600, 0x8800, 0x8a00, 0x8c00 }, /* F12 */
};

কাঠামো scaninfo চাবি_ext_enter VAR16 = {
    0xe00d, 0xe00d, 0xe00a, 0xa600
};
কাঠামো scaninfo চাবি_ext_slash VAR16 = {
    0xe02f, 0xe02f, 0x9500, 0xa400
};

u16 ascii_to_keycode(u8 ascii)
{
    পূর্ণসংখ্যা i;

    জন্য (i = 0; i < ARRAY_আকার(scan_to_keycode); i++) {
        যদি ((GET_GLOBAL(scan_to_keycode[i].normal) & 0xff) == ascii)
            ফেরত GET_GLOBAL(scan_to_keycode[i].normal);
        যদি ((GET_GLOBAL(scan_to_keycode[i].shift) & 0xff) == ascii)
            ফেরত GET_GLOBAL(scan_to_keycode[i].shift);
        যদি ((GET_GLOBAL(scan_to_keycode[i].control) & 0xff) == ascii)
            ফেরত GET_GLOBAL(scan_to_keycode[i].control);
    }
    ফেরত 0;
}

// Handle a ps2 style scancode read from the keyboard.
স্থির শূন্য
kbd_set_flag(পূর্ণসংখ্যা চাবি_release, u16 set_bit0, u8 set_bit1, u16 toggle_bit)
{
    u16 flags0 = GET_BDA(kbd_flag0);
    u8 flags1 = GET_BDA(kbd_flag1);
    যদি (চাবি_release) {
        flags0 &= ~set_bit0;
        flags1 &= ~set_bit1;
    } অন্যথা {
        flags0 ^= toggle_bit;
        flags0 |= set_bit0;
        flags1 |= set_bit1;
    }
    SET_BDA(kbd_flag0, flags0);
    SET_BDA(kbd_flag1, flags1);
}

স্থির শূন্য
kbd_ctrl_break(পূর্ণসংখ্যা চাবি_release)
{
    যদি (!চাবি_release)
        ফেরত;
    // Clear keyboard buffer and place 0x0000 in buffer
    u16 বাফার_start = GET_BDA(kbd_buf_start_offset);
    SET_BDA(kbd_buf_শীর্ষ, বাফার_start);
    SET_BDA(kbd_buf_লেজ, বাফার_start+2);
    SET_FARVAR(SEG_BDA, *(u16*)(বাফার_start+0), 0x0000);
    // Set break flag
    SET_BDA(break_flag, 0x80);
    // Generate int 0x1b
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF;
    call16_int(0x1b, &br);
}

স্থির শূন্য
kbd_sysreq(পূর্ণসংখ্যা চাবি_release)
{
    // SysReq generates int 0x15/0x85
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.ah = 0x85;
    br.al = চাবি_release ? 0x01 : 0x00;
    br.flags = F_IF;
    call16_int(0x15, &br);
}

স্থির শূন্য
kbd_prtscr(পূর্ণসংখ্যা চাবি_release)
{
    যদি (চাবি_release)
        ফেরত;
    // PrtScr generates int 0x05 (ctrl-prtscr has keycode 0x7200?)
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF;
    call16_int(0x05, &br);
}

// Handle a ps2 style scancode read from the keyboard.
স্থির শূন্য
__process_key(u8 scancode)
{
    // Check for multi-scancode key sequences
    u8 flags1 = GET_BDA(kbd_flag1);
    যদি (scancode == 0xe0 || scancode == 0xe1) {
        // Start of two byte extended (e0) or three byte pause key (e1) sequence
        u8 eflag = scancode == 0xe0 ? KF1_LAST_E0 : KF1_LAST_E1;
        SET_BDA(kbd_flag1, flags1 | eflag);
        ফেরত;
    }
    পূর্ণসংখ্যা চাবি_release = scancode & 0x80;
    scancode &= ~0x80;
    যদি (flags1 & (KF1_LAST_E0|KF1_LAST_E1)) {
        যদি (flags1 & KF1_LAST_E1 && scancode == 0x1d)
            // Ignore second byte of pause key (e1 1d 45 / e1 9d c5)
            ফেরত;
        // Clear E0/E1 flag in memory for next key event
        SET_BDA(kbd_flag1, flags1 & ~(KF1_LAST_E0|KF1_LAST_E1));
    }

    // Check for special keys
    নির্বাচন (scancode) {
    ক্ষেত্র 0x3a: /* Caps Lock */
        kbd_set_flag(চাবি_release, KF0_CAPS, 0, KF0_CAPSACTIVE);
        ফেরত;
    ক্ষেত্র 0x2a: /* L Shift */
        যদি (flags1 & KF1_LAST_E0)
            // Ignore fake shifts
            ফেরত;
        kbd_set_flag(চাবি_release, KF0_LSHIFT, 0, 0);
        ফেরত;
    ক্ষেত্র 0x36: /* R Shift */
        যদি (flags1 & KF1_LAST_E0)
            // Ignore fake shifts
            ফেরত;
        kbd_set_flag(চাবি_release, KF0_RSHIFT, 0, 0);
        ফেরত;
    ক্ষেত্র 0x1d: /* Ctrl */
        যদি (flags1 & KF1_LAST_E0)
            kbd_set_flag(চাবি_release, KF0_CTRLACTIVE, KF1_RCTRL, 0);
        অন্যথা
            kbd_set_flag(চাবি_release, KF0_CTRLACTIVE | KF0_LCTRL, 0, 0);
        ফেরত;
    ক্ষেত্র 0x38: /* Alt */
        যদি (flags1 & KF1_LAST_E0)
            kbd_set_flag(চাবি_release, KF0_ALTACTIVE, KF1_RALT, 0);
        অন্যথা
            kbd_set_flag(চাবি_release, KF0_ALTACTIVE | KF0_LALT, 0, 0);
        ফেরত;
    ক্ষেত্র 0x45: /* Num Lock */
        যদি (flags1 & KF1_LAST_E1)
            // XXX - pause key.
            ফেরত;
        kbd_set_flag(চাবি_release, KF0_অঙ্ক, 0, KF0_NUMACTIVE);
        ফেরত;
    ক্ষেত্র 0x46: /* Scroll Lock */
        যদি (flags1 & KF1_LAST_E0) {
            kbd_ctrl_break(চাবি_release);
            ফেরত;
        }
        kbd_set_flag(চাবি_release, KF0_SCROLL, 0, KF0_SCROLLACTIVE);
        ফেরত;

    ক্ষেত্র 0x37: /* * */
        যদি (flags1 & KF1_LAST_E0) {
            kbd_prtscr(চাবি_release);
            ফেরত;
        }
        বিরতি;
    ক্ষেত্র 0x54: /* SysReq */
        kbd_sysreq(চাবি_release);
        ফেরত;
    ক্ষেত্র 0x53: /* Del */
        যদি ((GET_BDA(kbd_flag0) & (KF0_CTRLACTIVE|KF0_ALTACTIVE))
            == (KF0_CTRLACTIVE|KF0_ALTACTIVE) && !চাবি_release) {
            // Ctrl+alt+del - reset machine.
            SET_BDA(soft_reset_flag, 0x1234);
            reset();
        }
        বিরতি;

    পূর্বনির্ধারিত:
        বিরতি;
    }

    // Handle generic keys
    যদি (চাবি_release)
        // ignore key releases
        ফেরত;
    যদি (!scancode || scancode >= ARRAY_আকার(scan_to_keycode)) {
        dprintf(1, "__process_key unknown scancode read: 0x%02x!\n", scancode);
        ফেরত;
    }
    কাঠামো scaninfo *info = &scan_to_keycode[scancode];
    যদি (flags1 & KF1_LAST_E0 && (scancode == 0x1c || scancode == 0x35))
        info = (scancode == 0x1c ? &চাবি_ext_enter : &চাবি_ext_slash);
    u16 flags0 = GET_BDA(kbd_flag0);
    u16 keycode;
    যদি (flags0 & KF0_ALTACTIVE) {
        keycode = GET_GLOBAL(info->alt);
    } অন্যথা যদি (flags0 & KF0_CTRLACTIVE) {
        keycode = GET_GLOBAL(info->control);
    } অন্যথা {
        u8 useshift = flags0 & (KF0_RSHIFT|KF0_LSHIFT) ? 1 : 0;
        u8 ascii = GET_GLOBAL(info->normal) & 0xff;
        যদি ((flags0 & KF0_NUMACTIVE && scancode >= 0x47 && scancode <= 0x53)
            || (flags0 & KF0_CAPSACTIVE && ascii >= 'a' && ascii <= 'z'))
            // Numlock/capslock toggles shift on certain keys
            useshift ^= 1;
        যদি (useshift)
            keycode = GET_GLOBAL(info->shift);
        অন্যথা
            keycode = GET_GLOBAL(info->normal);
    }
    যদি (flags1 & KF1_LAST_E0 && scancode >= 0x47 && scancode <= 0x53) {
        /* extended keys handling */
        যদি (flags0 & KF0_ALTACTIVE)
            keycode = (scancode + 0x50) << 8;
        অন্যথা
            keycode = (keycode & 0xff00) | 0xe0;
    }
    যদি (keycode)
        enqueue_চাবি(keycode);
}

শূন্য
process_চাবি(u8 চাবি)
{
    যদি (!CONFIG_KEYBOARD)
        ফেরত;

    যদি (CONFIG_KBD_CALL_INT15_4F) {
        // allow for keyboard intercept
        কাঠামো bregs br;
        memset(&br, 0, আকার(br));
        br.eax = (0x4f << 8) | চাবি;
        br.flags = F_IF|F_CF;
        call16_int(0x15, &br);
        যদি (!(br.flags & F_CF))
            ফেরত;
        চাবি = br.eax;
    }
    __process_key(চাবি);
}
