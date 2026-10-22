// 16bit code to handle keyboard requests.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_BDA
#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "hw/ps2port.h" // ps2_kbd_command
#ضمن "hw/usb-hid.h" // usb_kbd_command
#ضمن "output.h" // debug_enter
#ضمن "stacks.h" // yield
#ضمن "string.h" // memset
#ضمن "util.h" // kbd_init

فراغ
kbd_init(فراغ)
{
    dprintf(3, "init keyboard\n");
    u16 x = offsetof(بنية bios_معلومات_area_s, kbd_buf);
    سېټ_BDA(kbd_flag1, KF1_101KBD);
    سېټ_BDA(kbd_buf_سر, x);
    سېټ_BDA(kbd_buf_لکۍ, x);
    سېټ_BDA(kbd_buf_پیل_دننه_واټن, x);

    سېټ_BDA(kbd_buf_پای_ټکی_دننه_واټن
            , x + ډګر_SIZEOF(بنية bios_معلومات_area_s, kbd_buf));
}

u8
کتار_ته_ورزیاتول_کلی(u16 keycode)
{
    u16 بفر_پیل = GET_BDA(kbd_buf_پیل_دننه_واټن);
    u16 بفر_پای_ټکی   = GET_BDA(kbd_buf_پای_ټکی_دننه_واټن);

    u16 بفر_سر = GET_BDA(kbd_buf_سر);
    u16 بفر_لکۍ = GET_BDA(kbd_buf_لکۍ);

    u16 لنډمهاله_لکۍ = بفر_لکۍ;
    بفر_لکۍ += 2;
    إذا (بفر_لکۍ >= بفر_پای_ټکی)
        بفر_لکۍ = بفر_پیل;

    إذا (بفر_لکۍ == بفر_سر)
        أرجع 0;

    سېټ_FARVAR(SEG_BDA, *(u16*)(لنډمهاله_لکۍ+0), keycode);
    سېټ_BDA(kbd_buf_لکۍ, بفر_لکۍ);
    أرجع 1;
}

ساكن فراغ
له_کتار_ایستل_کلی(بنية bregs *regs, صحيح incr, صحيح extended)
{
    yield();
    u16 بفر_سر;
    u16 بفر_لکۍ;
    لكل (;;) {
        بفر_سر = GET_BDA(kbd_buf_سر);
        بفر_لکۍ = GET_BDA(kbd_buf_لکۍ);

        إذا (بفر_سر != بفر_لکۍ)
            اكسر;
        إذا (!incr) {
            regs->flags |= F_ZF;
            أرجع;
        }
        yield_toirq();
    }

    u16 keycode = GET_FARVAR(SEG_BDA, *(u16*)(بفر_سر+0));
    u8 ascii = keycode & 0xff;
    إذا (!extended) {
        // Translate extended keys
        إذا (ascii == 0xe0 && keycode & 0xff00)
            keycode &= 0xff00;
        وإلا إذا (keycode == 0xe00d || keycode == 0xe00a)
            // Extended enter key
            keycode = 0x1c00 | ascii;
        وإلا إذا (keycode == 0xe02f)
            // Extended '/' key
            keycode = 0x352f;
        // Technically, if the ascii value is 0xf0 or if the
        // 'scancode' is greater than 0x84 then the key should be
        // discarded.  However, there seems no harm in passing on the
        // extended values in these cases.
    }
    إذا (ascii == 0xf0 && keycode & 0xff00)
        keycode &= 0xff00;
    regs->ax = keycode;

    إذا (!incr) {
        regs->flags &= ~F_ZF;
        أرجع;
    }
    u16 بفر_پیل = GET_BDA(kbd_buf_پیل_دننه_واټن);
    u16 بفر_پای_ټکی   = GET_BDA(kbd_buf_پای_ټکی_دننه_واټن);

    بفر_سر += 2;
    إذا (بفر_سر >= بفر_پای_ټکی)
        بفر_سر = بفر_پیل;
    سېټ_BDA(kbd_buf_سر, بفر_سر);
}

ساكن صحيح
kbd_command(صحيح command, u8 *param)
{
    إذا (usb_kbd_فعال())
        أرجع usb_kbd_command(command, param);
    أرجع ps2_kbd_command(command, param);
}

// read keyboard input
ساكن فراغ
handle_1600(بنية bregs *regs)
{
    له_کتار_ایستل_کلی(regs, 1, 0);
}

// check keyboard status
ساكن فراغ
handle_1601(بنية bregs *regs)
{
    له_کتار_ایستل_کلی(regs, 0, 0);
}

// get shift flag status
ساكن فراغ
handle_1602(بنية bregs *regs)
{
    yield();
    regs->al = GET_BDA(kbd_flag0);
}

// store key-stroke into buffer
ساكن فراغ
handle_1605(بنية bregs *regs)
{
    regs->al = !کتار_ته_ورزیاتول_کلی(regs->cx);
}

// GET KEYBOARD FUNCTIONALITY
ساكن فراغ
handle_1609(بنية bregs *regs)
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
ساكن فراغ noinline
handle_160a(بنية bregs *regs)
{
    u8 param[2];
    صحيح ret = kbd_command(ATKBD_CMD_GETID, param);
    إذا (ret) {
        regs->bx = 0;
        أرجع;
    }
    regs->bx = (param[1] << 8) | param[0];
}

// read MF-II keyboard input
ساكن فراغ
handle_1610(بنية bregs *regs)
{
    له_کتار_ایستل_کلی(regs, 1, 1);
}

// check MF-II keyboard status
ساكن فراغ
handle_1611(بنية bregs *regs)
{
    له_کتار_ایستل_کلی(regs, 0, 1);
}

// get extended keyboard status
ساكن فراغ
handle_1612(بنية bregs *regs)
{
    yield();
    regs->ax = ((GET_BDA(kbd_flag0) & ~((KF1_RCTRL|KF1_RALT) << 8))
                | ((GET_BDA(kbd_flag1) & (KF1_RCTRL|KF1_RALT)) << 8));
    //BX_DEBUG_INT16("int16: func 12 sending %04x\n",AX);
}

ساكن فراغ
handle_166f(بنية bregs *regs)
{
    إذا (regs->al == 0x08)
        // unsupported, aka normal keyboard
        regs->ah = 2;
}

// keyboard capability check called by DOS 5.0+ keyb
ساكن فراغ
handle_1692(بنية bregs *regs)
{
    // function int16 ah=0x10-0x12 supported
    regs->ah = 0x80;
}

// 122 keys capability check called by DOS 5.0+ keyb
ساكن فراغ
handle_16a2(بنية bregs *regs)
{
    // don't change AH : function int16 ah=0x20-0x22 NOT supported
}

ساكن فراغ
handle_16XX(بنية bregs *regs)
{
    warn_unimplemented(regs);
}

ساكن فراغ noinline
سېټ_leds(فراغ)
{
    u8 ښويدنه_flags = (GET_BDA(kbd_flag0) >> 4) & 0x07;
    u8 kbd_led = GET_BDA(kbd_led);
    u8 led_flags = kbd_led & 0x07;
    إذا (ښويدنه_flags == led_flags)
        أرجع;

    صحيح ret = kbd_command(ATKBD_CMD_SETLEDS, &ښويدنه_flags);
    إذا (ret)
        // Error
        أرجع;
    kbd_led = (kbd_led & ~0x07) | ښويدنه_flags;
    سېټ_BDA(kbd_led, kbd_led);
}

// INT 16h Keyboard Service Entry Point
فراغ VISIBLE16
handle_16(بنية bregs *regs)
{
    debug_enter(regs, DEBUG_HDL_16);
    إذا (! CONFIG_KEYBOARD)
        أرجع;

    // XXX - set_leds should be called from irq handler
    سېټ_leds();

    بدل (regs->ah) {
    حالة 0x00: handle_1600(regs); اكسر;
    حالة 0x01: handle_1601(regs); اكسر;
    حالة 0x02: handle_1602(regs); اكسر;
    حالة 0x05: handle_1605(regs); اكسر;
    حالة 0x09: handle_1609(regs); اكسر;
    حالة 0x0a: handle_160a(regs); اكسر;
    حالة 0x10: handle_1610(regs); اكسر;
    حالة 0x11: handle_1611(regs); اكسر;
    حالة 0x12: handle_1612(regs); اكسر;
    حالة 0x92: handle_1692(regs); اكسر;
    حالة 0xa2: handle_16a2(regs); اكسر;
    حالة 0x6f: handle_166f(regs); اكسر;
    افتراضي:   handle_16XX(regs); اكسر;
    }
}

#تعریف_کړه none 0

ساكن بنية scaninfo {
    u16 نورمال;
    u16 ښويدنه;
    u16 control;
    u16 alt;
} لټون_منزل_keycode[] VAR16 = {
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

بنية scaninfo کلی_ext_enter VAR16 = {
    0xe00d, 0xe00d, 0xe00a, 0xa600
};
بنية scaninfo کلی_ext_slash VAR16 = {
    0xe02f, 0xe02f, 0x9500, 0xa400
};

u16 ascii_منزل_keycode(u8 ascii)
{
    صحيح i;

    لكل (i = 0; i < ARRAY_اندازه(لټون_منزل_keycode); i++) {
        إذا ((GET_GLOBAL(لټون_منزل_keycode[i].نورمال) & 0xff) == ascii)
            أرجع GET_GLOBAL(لټون_منزل_keycode[i].نورمال);
        إذا ((GET_GLOBAL(لټون_منزل_keycode[i].ښويدنه) & 0xff) == ascii)
            أرجع GET_GLOBAL(لټون_منزل_keycode[i].ښويدنه);
        إذا ((GET_GLOBAL(لټون_منزل_keycode[i].control) & 0xff) == ascii)
            أرجع GET_GLOBAL(لټون_منزل_keycode[i].control);
    }
    أرجع 0;
}

// Handle a ps2 style scancode read from the keyboard.
ساكن فراغ
kbd_سېټ_flag(صحيح کلی_release, u16 سېټ_bit0, u8 سېټ_bit1, u16 toggle_بټ)
{
    u16 flags0 = GET_BDA(kbd_flag0);
    u8 flags1 = GET_BDA(kbd_flag1);
    إذا (کلی_release) {
        flags0 &= ~سېټ_bit0;
        flags1 &= ~سېټ_bit1;
    } وإلا {
        flags0 ^= toggle_بټ;
        flags0 |= سېټ_bit0;
        flags1 |= سېټ_bit1;
    }
    سېټ_BDA(kbd_flag0, flags0);
    سېټ_BDA(kbd_flag1, flags1);
}

ساكن فراغ
kbd_ctrl_break(صحيح کلی_release)
{
    إذا (!کلی_release)
        أرجع;
    // Clear keyboard buffer and place 0x0000 in buffer
    u16 بفر_پیل = GET_BDA(kbd_buf_پیل_دننه_واټن);
    سېټ_BDA(kbd_buf_سر, بفر_پیل);
    سېټ_BDA(kbd_buf_لکۍ, بفر_پیل+2);
    سېټ_FARVAR(SEG_BDA, *(u16*)(بفر_پیل+0), 0x0000);
    // Set break flag
    سېټ_BDA(break_flag, 0x80);
    // Generate int 0x1b
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x1b, &br);
}

ساكن فراغ
kbd_sysreq(صحيح کلی_release)
{
    // SysReq generates int 0x15/0x85
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.ah = 0x85;
    br.al = کلی_release ? 0x01 : 0x00;
    br.flags = F_IF;
    call16_int(0x15, &br);
}

ساكن فراغ
kbd_prtscr(صحيح کلی_release)
{
    إذا (کلی_release)
        أرجع;
    // PrtScr generates int 0x05 (ctrl-prtscr has keycode 0x7200?)
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    call16_int(0x05, &br);
}

// Handle a ps2 style scancode read from the keyboard.
ساكن فراغ
__process_key(u8 scancode)
{
    // Check for multi-scancode key sequences
    u8 flags1 = GET_BDA(kbd_flag1);
    إذا (scancode == 0xe0 || scancode == 0xe1) {
        // Start of two byte extended (e0) or three byte pause key (e1) sequence
        u8 eflag = scancode == 0xe0 ? KF1_وروستی_E0 : KF1_وروستی_E1;
        سېټ_BDA(kbd_flag1, flags1 | eflag);
        أرجع;
    }
    صحيح کلی_release = scancode & 0x80;
    scancode &= ~0x80;
    إذا (flags1 & (KF1_وروستی_E0|KF1_وروستی_E1)) {
        إذا (flags1 & KF1_وروستی_E1 && scancode == 0x1d)
            // Ignore second byte of pause key (e1 1d 45 / e1 9d c5)
            أرجع;
        // Clear E0/E1 flag in memory for next key event
        سېټ_BDA(kbd_flag1, flags1 & ~(KF1_وروستی_E0|KF1_وروستی_E1));
    }

    // Check for special keys
    بدل (scancode) {
    حالة 0x3a: /* Caps Lock */
        kbd_سېټ_flag(کلی_release, KF0_CAPS, 0, KF0_CAPSACTIVE);
        أرجع;
    حالة 0x2a: /* L Shift */
        إذا (flags1 & KF1_وروستی_E0)
            // Ignore fake shifts
            أرجع;
        kbd_سېټ_flag(کلی_release, KF0_LSHIFT, 0, 0);
        أرجع;
    حالة 0x36: /* R Shift */
        إذا (flags1 & KF1_وروستی_E0)
            // Ignore fake shifts
            أرجع;
        kbd_سېټ_flag(کلی_release, KF0_RSHIFT, 0, 0);
        أرجع;
    حالة 0x1d: /* Ctrl */
        إذا (flags1 & KF1_وروستی_E0)
            kbd_سېټ_flag(کلی_release, KF0_CTRLACTIVE, KF1_RCTRL, 0);
        وإلا
            kbd_سېټ_flag(کلی_release, KF0_CTRLACTIVE | KF0_LCTRL, 0, 0);
        أرجع;
    حالة 0x38: /* Alt */
        إذا (flags1 & KF1_وروستی_E0)
            kbd_سېټ_flag(کلی_release, KF0_ALTACTIVE, KF1_RALT, 0);
        وإلا
            kbd_سېټ_flag(کلی_release, KF0_ALTACTIVE | KF0_LALT, 0, 0);
        أرجع;
    حالة 0x45: /* Num Lock */
        إذا (flags1 & KF1_وروستی_E1)
            // XXX - pause key.
            أرجع;
        kbd_سېټ_flag(کلی_release, KF0_عدد, 0, KF0_NUMACTIVE);
        أرجع;
    حالة 0x46: /* Scroll Lock */
        إذا (flags1 & KF1_وروستی_E0) {
            kbd_ctrl_break(کلی_release);
            أرجع;
        }
        kbd_سېټ_flag(کلی_release, KF0_SCROLL, 0, KF0_SCROLLACTIVE);
        أرجع;

    حالة 0x37: /* * */
        إذا (flags1 & KF1_وروستی_E0) {
            kbd_prtscr(کلی_release);
            أرجع;
        }
        اكسر;
    حالة 0x54: /* SysReq */
        kbd_sysreq(کلی_release);
        أرجع;
    حالة 0x53: /* Del */
        إذا ((GET_BDA(kbd_flag0) & (KF0_CTRLACTIVE|KF0_ALTACTIVE))
            == (KF0_CTRLACTIVE|KF0_ALTACTIVE) && !کلی_release) {
            // Ctrl+alt+del - reset machine.
            سېټ_BDA(soft_reset_flag, 0x1234);
            reset();
        }
        اكسر;

    افتراضي:
        اكسر;
    }

    // Handle generic keys
    إذا (کلی_release)
        // ignore key releases
        أرجع;
    إذا (!scancode || scancode >= ARRAY_اندازه(لټون_منزل_keycode)) {
        dprintf(1, "__process_key unknown scancode read: 0x%02x!\n", scancode);
        أرجع;
    }
    بنية scaninfo *info = &لټون_منزل_keycode[scancode];
    إذا (flags1 & KF1_وروستی_E0 && (scancode == 0x1c || scancode == 0x35))
        info = (scancode == 0x1c ? &کلی_ext_enter : &کلی_ext_slash);
    u16 flags0 = GET_BDA(kbd_flag0);
    u16 keycode;
    إذا (flags0 & KF0_ALTACTIVE) {
        keycode = GET_GLOBAL(info->alt);
    } وإلا إذا (flags0 & KF0_CTRLACTIVE) {
        keycode = GET_GLOBAL(info->control);
    } وإلا {
        u8 useshift = flags0 & (KF0_RSHIFT|KF0_LSHIFT) ? 1 : 0;
        u8 ascii = GET_GLOBAL(info->نورمال) & 0xff;
        إذا ((flags0 & KF0_NUMACTIVE && scancode >= 0x47 && scancode <= 0x53)
            || (flags0 & KF0_CAPSACTIVE && ascii >= 'a' && ascii <= 'z'))
            // Numlock/capslock toggles shift on certain keys
            useshift ^= 1;
        إذا (useshift)
            keycode = GET_GLOBAL(info->ښويدنه);
        وإلا
            keycode = GET_GLOBAL(info->نورمال);
    }
    إذا (flags1 & KF1_وروستی_E0 && scancode >= 0x47 && scancode <= 0x53) {
        /* extended keys handling */
        إذا (flags0 & KF0_ALTACTIVE)
            keycode = (scancode + 0x50) << 8;
        وإلا
            keycode = (keycode & 0xff00) | 0xe0;
    }
    إذا (keycode)
        کتار_ته_ورزیاتول_کلی(keycode);
}

فراغ
process_کلی(u8 کلی)
{
    إذا (!CONFIG_KEYBOARD)
        أرجع;

    إذا (CONFIG_KBD_CALL_INT15_4F) {
        // allow for keyboard intercept
        بنية bregs br;
        memset(&br, 0, حجم(br));
        br.eax = (0x4f << 8) | کلی;
        br.flags = F_IF|F_CF;
        call16_int(0x15, &br);
        إذا (!(br.flags & F_CF))
            أرجع;
        کلی = br.eax;
    }
    __process_key(کلی);
}
