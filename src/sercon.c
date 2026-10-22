// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // SET_BDA
#شامل "bregs.h" // struct bregs
#شامل "stacks.h" // yield
#شامل "output.h" // dprintf
#شامل "util.h" // irqtimer_calc_ticks
#شامل "string.h" // memcpy
#شامل "romfile.h" // romfile_loadint
#شامل "hw/serialio.h" // SEROFF_IER
#شامل "cp437.h"

ساکن u8 video_صفیں(خالی)
{
    واپس GET_BDA(video_صفیں)+1;
}

ساکن u8 video_ستون_تعداد(خالی)
{
    واپس GET_BDA(video_ستون_تعداد);
}

ساکن u8 cursor_pos_ستون(خالی)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    واپس pos & 0xff;
}

ساکن u8 cursor_pos_صف(خالی)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    واپس (pos >> 8) & 0xff;
}

ساکن خالی cursor_pos_سیٹ(u8 صف, u8 ستون)
{
    u16 pos = ((u16)صف << 8) | ستون;
    سیٹ_BDA(cursor_pos[0], pos);
}

/****************************************************************
 * serial console output
 ****************************************************************/

VARLOW u16 sercon_پورٹ;
VARLOW u8 sercon_بانٹو;
VARLOW u8 sercon_enable;
VARFSEG ساخت segoff_s sercon_real_vga_handler;

/*
 * We have a small output buffer here, for lazy output.  That allows
 * to avoid a whole bunch of control sequences for pointless cursor
 * moves, so when logging the output it'll be *alot* less cluttered.
 *
 * sercon_char/attr  is the actual output buffer.
 * sercon_attr_last  is the most recent attribute sent to the terminal.
 * sercon_col_last   is the most recent column sent to the terminal.
 * sercon_row_last   is the most recent row sent to the terminal.
 */
VARLOW u8 sercon_attr_آخری;
VARLOW u8 sercon_ستون_آخری;
VARLOW u8 sercon_صف_آخری;
VARLOW u8 sercon_char;
VARLOW u8 sercon_attr = 0x07;

ساکن VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ساکن صحیح sercon_splitmode(خالی)
{
    واپس GET_کم(sercon_بانٹو);
}

ساکن خالی sercon_putchar(u8 chr)
{
    u16 addr = GET_کم(sercon_پورٹ);
    u32 انتہا = irqtimer_calc_ticks(0x0a);

#اگر 0
    /* for visual control sequence debugging */
    اگر (chr == '\x1b')
        chr = '*';
#اگر_ختم

    برائے (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        اگر ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_ڈیٹا);
            توڑو;
        }
        اگر (irqtimer_check(انتہا)) {
            توڑو;
        }
        yield();
    }
}

ساکن خالی sercon_رکن_reset(خالی)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ساکن خالی sercon_رکن_clear_screen(خالی)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ساکن خالی sercon_رکن_no_linewrap(خالی)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ساکن خالی sercon_رکن_cursor_goto(u8 صف, u8 ستون)
{
    صف++; ستون++;
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0' + صف / 10);
    sercon_putchar('0' + صف % 10);
    sercon_putchar(';');
    sercon_putchar('0' + ستون / 10);
    sercon_putchar('0' + ستون % 10);
    sercon_putchar('H');
}

ساکن خالی sercon_رکن_سیٹ_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    اگر (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    اگر (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    اگر (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

ساکن خالی sercon_سیٹ_attr(u8 attr)
{
    اگر (attr == GET_کم(sercon_attr_آخری))
        واپس;

    سیٹ_کم(sercon_attr_آخری, attr);
    sercon_رکن_سیٹ_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ساکن خالی sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_مقصد_unicode(chr);

    اگر (unicode < 0x7f) {
        sercon_putchar(unicode);
    } ورنہ اگر (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } ورنہ {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

ساکن خالی sercon_cursor_pos_سیٹ(u8 صف, u8 ستون)
{
    اگر (!sercon_splitmode()) {
        cursor_pos_سیٹ(صف, ستون);
    } ورنہ {
        /* let vgabios update cursor */
    }
}

ساکن خالی sercon_lazy_cursor_sync(خالی)
{
    u8 صف = cursor_pos_صف();
    u8 ستون = cursor_pos_ستون();

    اگر (GET_کم(sercon_صف_آخری) == صف &&
        GET_کم(sercon_ستون_آخری) == ستون)
        واپس;

    اگر (ستون == 0 && GET_کم(sercon_صف_آخری) <= صف) {
        اگر (GET_کم(sercon_ستون_آخری) != 0) {
            sercon_putchar('\r');
            سیٹ_کم(sercon_ستون_آخری, 0);
        }
        جب (GET_کم(sercon_صف_آخری) < صف) {
            sercon_putchar('\n');
            سیٹ_کم(sercon_صف_آخری, GET_کم(sercon_صف_آخری)+1);
        }
        اگر (GET_کم(sercon_صف_آخری) == صف &&
            GET_کم(sercon_ستون_آخری) == ستون)
            واپس;
    }

    sercon_رکن_cursor_goto(صف, ستون);
    سیٹ_کم(sercon_صف_آخری, صف);
    سیٹ_کم(sercon_ستون_آخری, ستون);
}

ساکن خالی sercon_lazy_flush(خالی)
{
    u8 chr, attr;

    chr = GET_کم(sercon_char);
    attr = GET_کم(sercon_attr);
    اگر (chr) {
        sercon_سیٹ_attr(attr);
        sercon_print_utf8(chr);
        سیٹ_کم(sercon_ستون_آخری, GET_کم(sercon_ستون_آخری) + 1);
    }

    sercon_lazy_cursor_sync();

    سیٹ_کم(sercon_attr, 0x07);
    سیٹ_کم(sercon_char, 0x00);
}

ساکن خالی sercon_lazy_cursor_update(u8 صف, u8 ستون)
{
    sercon_cursor_pos_سیٹ(صف, ستون);
    سیٹ_کم(sercon_صف_آخری, صف);
    سیٹ_کم(sercon_ستون_آخری, ستون);
}

ساکن خالی sercon_lazy_backspace(خالی)
{
    u8 ستون;

    sercon_lazy_flush();
    ستون = cursor_pos_ستون();
    اگر (ستون > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_صف(), ستون-1);
    }
}

ساکن خالی sercon_lazy_cr(خالی)
{
    sercon_cursor_pos_سیٹ(cursor_pos_صف(), 0);
}

ساکن خالی sercon_lazy_lf(خالی)
{
    u8 صف;

    صف = cursor_pos_صف() + 1;
    اگر (صف >= video_صفیں()) {
        /* scrolling up */
        صف = video_صفیں()-1;
        اگر (GET_کم(sercon_صف_آخری) > 0) {
            سیٹ_کم(sercon_صف_آخری, GET_کم(sercon_صف_آخری) - 1);
        }
    }
    sercon_cursor_pos_سیٹ(صف, cursor_pos_ستون());
}

ساکن خالی sercon_lazy_move_cursor(خالی)
{
    u8 ستون;

    ستون = cursor_pos_ستون() + 1;
    اگر (ستون >= video_ستون_تعداد()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } ورنہ {
        sercon_cursor_pos_سیٹ(cursor_pos_صف(), ستون);
    }
}

ساکن خالی sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    اگر (cursor_pos_صف() != GET_کم(sercon_صف_آخری) ||
        cursor_pos_ستون() != GET_کم(sercon_ستون_آخری)) {
        sercon_lazy_flush();
    }

    سیٹ_کم(sercon_char, chr);
    اگر (teletype)
        sercon_lazy_move_cursor();
    ورنہ
        سیٹ_کم(sercon_attr, attr);
}

/* Set video mode */
ساکن خالی sercon_1000(ساخت bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 منوال = regs->al & 0x7f;
    u8 صفیں, ستون_تعداد;

    اگر (!sercon_splitmode()) {
        انتخاب (منوال) {
        حالت 0x00:
        حالت 0x01:
        حالت 0x04: /* 320x200 */
        حالت 0x05: /* 320x200 */
            ستون_تعداد = 40;
            صفیں = 25;
            regs->al = 0x30;
            توڑو;
        حالت 0x02:
        حالت 0x03:
        حالت 0x06: /* 640x200 */
        حالت 0x07:
        طےشدہ:
            ستون_تعداد = 80;
            صفیں = 25;
            regs->al = 0x30;
            توڑو;
        }
        cursor_pos_سیٹ(0, 0);
        سیٹ_BDA(video_منوال, منوال);
        سیٹ_BDA(video_ستون_تعداد, ستون_تعداد);
        سیٹ_BDA(video_صفیں, صفیں-1);
        سیٹ_BDA(cursor_type, 0x0007);
    } ورنہ {
        /* let vgabios handle mode init */
    }

    سیٹ_کم(sercon_enable, منوال <= 0x07);
    سیٹ_کم(sercon_ستون_آخری, 0);
    سیٹ_کم(sercon_صف_آخری, 0);
    سیٹ_کم(sercon_attr_آخری, 0);

    sercon_رکن_reset();
    sercon_رکن_no_linewrap();
    اگر (clearscreen)
        sercon_رکن_clear_screen();
}

/* Set text-mode cursor shape */
ساکن خالی sercon_1001(ساخت bregs *regs)
{
    /* show/hide cursor? */
    سیٹ_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ساکن خالی sercon_1002(ساخت bregs *regs)
{
    sercon_cursor_pos_سیٹ(regs->dh, regs->dl);
}

/* Get cursor position */
ساکن خالی sercon_1003(ساخت bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_صف();
    regs->dl = cursor_pos_ستون();
}

/* Scroll up window */
ساکن خالی sercon_1006(ساخت bregs *regs)
{
    sercon_lazy_flush();
    اگر (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        اگر (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_صفیں()-1 &&
            regs->dl == video_ستون_تعداد()-1) {
            sercon_سیٹ_attr(regs->bh);
            sercon_رکن_clear_screen();
        }
    } ورنہ {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
ساکن خالی sercon_1008(ساخت bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
ساکن خالی sercon_1009(ساخت bregs *regs)
{
    u16 گنتی = regs->cx;

    اگر (گنتی == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } ورنہ اگر (regs->al == 0x20 &&
               video_صفیں() * video_ستون_تعداد() == گنتی &&
               cursor_pos_صف() == 0 &&
               cursor_pos_ستون() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_سیٹ_attr(regs->bl);
        sercon_رکن_clear_screen();

    } ورنہ {
        sercon_lazy_flush();
        sercon_سیٹ_attr(regs->bl);
        جب (گنتی) {
            sercon_print_utf8(regs->al);
            گنتی--;
        }
        sercon_رکن_cursor_goto(cursor_pos_صف(),
                                cursor_pos_ستون());
    }
}

/* Teletype output */
ساکن خالی sercon_100e(ساخت bregs *regs)
{
    انتخاب (regs->al) {
    حالت 7:
        sercon_putchar(0x07);
        توڑو;
    حالت 8:
        sercon_lazy_backspace();
        توڑو;
    حالت '\r':
        sercon_lazy_cr();
        توڑو;
    حالت '\n':
        sercon_lazy_lf();
        توڑو;
    طےشدہ:
        sercon_lazy_putchar(regs->al, 0, 1);
        توڑو;
    }
}

/* Get current video mode */
ساکن خالی sercon_100f(ساخت bregs *regs)
{
    regs->al = GET_BDA(video_منوال);
    regs->ah = GET_BDA(video_ستون_تعداد);
}

/* VBE 2.0 */
ساکن خالی sercon_104f(ساخت bregs *regs)
{
    اگر (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } ورنہ {
        // Disable sercon entry point on any vesa modeset
        اگر (regs->al == 0x02)
            سیٹ_کم(sercon_enable, 0);
    }
}

ساکن خالی sercon_10XX(ساخت bregs *regs)
{
    warn_unimplemented(regs);
}

خالی VISIBLE16
handle_sercon(ساخت bregs *regs)
{
    اگر (!CONFIG_SERCON)
        واپس;
    اگر (!GET_کم(sercon_پورٹ))
        واپس;

    انتخاب (regs->ah) {
    حالت 0x01:
    حالت 0x02:
    حالت 0x03:
    حالت 0x08:
    حالت 0x0f:
        اگر (sercon_splitmode())
            /* nothing, vgabios handles it */
            واپس;
    }

    انتخاب (regs->ah) {
    حالت 0x00: sercon_1000(regs); توڑو;
    حالت 0x01: sercon_1001(regs); توڑو;
    حالت 0x02: sercon_1002(regs); توڑو;
    حالت 0x03: sercon_1003(regs); توڑو;
    حالت 0x06: sercon_1006(regs); توڑو;
    حالت 0x08: sercon_1008(regs); توڑو;
    حالت 0x09: sercon_1009(regs); توڑو;
    حالت 0x0e: sercon_100e(regs); توڑو;
    حالت 0x0f: sercon_100f(regs); توڑو;
    حالت 0x4f: sercon_104f(regs); توڑو;
    طےشدہ:   sercon_10XX(regs); توڑو;
    }
}

خالی sercon_تیاری(خالی)
{
    اگر (!CONFIG_SERCON)
        واپس;

    ساخت segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    اگر (!addr)
        واپس;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    اگر (CONFIG_DEBUG_SERIAL)
        اگر (addr == CONFIG_DEBUG_SERIAL_پورٹ)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(اندراج_10);
    اگر (vgabios.seg != seabios.seg ||
        vgabios.انحراف_2 != seabios.انحراف_2) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.انحراف_2);
        sercon_real_vga_handler = vgabios;
        سیٹ_کم(sercon_بانٹو, 1);
    } ورنہ {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    سیٹ_IVT(0x10, FUNC16(اندراج_sercon));
    سیٹ_کم(sercon_پورٹ, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_بائٹیں;

ساکن VAR16 ساخت {
    حرف seq[4];
    u8   len;
    u16  keycode;
} termseq[] = {
    { .seq = "OP",   .len = 2, .keycode = 0x3b00 },    // F1
    { .seq = "OQ",   .len = 2, .keycode = 0x3c00 },    // F2
    { .seq = "OR",   .len = 2, .keycode = 0x3d00 },    // F3
    { .seq = "OS",   .len = 2, .keycode = 0x3e00 },    // F4

    { .seq = "[15~", .len = 4, .keycode = 0x3f00 },    // F5
    { .seq = "[17~", .len = 4, .keycode = 0x4000 },    // F6
    { .seq = "[18~", .len = 4, .keycode = 0x4100 },    // F7
    { .seq = "[19~", .len = 4, .keycode = 0x4200 },    // F8
    { .seq = "[20~", .len = 4, .keycode = 0x4300 },    // F9
    { .seq = "[21~", .len = 4, .keycode = 0x4400 },    // F10
    { .seq = "[23~", .len = 4, .keycode = 0x8500 },    // F11
    { .seq = "[24~", .len = 4, .keycode = 0x8600 },    // F12

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //داخل کرو
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    //اوپر
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    //نیچے_کو
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //دایاں
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //بایاں

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    //انتہا
};

ساکن خالی shiftbuf(صحیح remove)
{
    صحیح i, بقایا;

    بقایا = GET_کم(rx_بائٹیں) - remove;
    سیٹ_کم(rx_بائٹیں, بقایا);
    برائے (i = 0; i < بقایا; i++)
        سیٹ_کم(rx_buf[i], GET_کم(rx_buf[i + remove]));
}

ساکن صحیح cmpbuf(صحیح seq)
{
    صحیح chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    اگر (GET_کم(rx_بائٹیں) < len + 1)
        واپس 0;
    برائے (chr = 0; chr < len; chr++)
        اگر (GET_GLOBAL(termseq[seq].seq[chr]) != GET_کم(rx_buf[chr + 1]))
            واپس 0;
    واپس 1;
}

ساکن صحیح findseq(خالی)
{
    صحیح seq;

    برائے (seq = 0; seq < ARRAY_سائز(termseq); seq++)
        اگر (cmpbuf(seq))
            واپس seq;
    واپس -1;
}

خالی
sercon_check_event(خالی)
{
    اگر (!CONFIG_SERCON)
        واپس;

    u16 addr = GET_کم(sercon_پورٹ);
    u16 keycode;
    u8 byte, گنتی = 0;
    صحیح seq;

    // check to see if there is a active serial port
    اگر (!addr)
        واپس;
    اگر (inb(addr + SEROFF_LSR) == 0xFF)
        واپس;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    جب (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_ڈیٹا);
        u8 rb = GET_کم(rx_بائٹیں);
        اگر (rb < حجم(rx_buf)) {
            سیٹ_کم(rx_buf[rb], byte);
            سیٹ_کم(rx_بائٹیں, rb + 1);
            گنتی++;
        }
    }

    برائے (;;) {
        // no (more) input data
        u8 rb = GET_کم(rx_بائٹیں);
        اگر (!rb)
            واپس;

        // lookup escape sequences
        u8 اگلا_char = GET_کم(rx_buf[0]);
        اگر (rb > 1 && اگلا_char == 0x1b) {
            seq = findseq();
            اگر (seq >= 0) {
                قطار_میں_ڈالو_کلید(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                جاری;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        اگر (اگلا_char == 0x1b && گنتی)
            واپس;

        // Handle input as individual char.
        keycode = ascii_مقصد_keycode(اگلا_char);
        اگر (keycode)
            قطار_میں_ڈالو_کلید(keycode);
        shiftbuf(1);
    }
}
