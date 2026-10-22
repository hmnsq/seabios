// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // SET_BDA
#ضمن "bregs.h" // struct bregs
#ضمن "stacks.h" // yield
#ضمن "output.h" // dprintf
#ضمن "util.h" // irqtimer_calc_ticks
#ضمن "string.h" // memcpy
#ضمن "romfile.h" // romfile_loadint
#ضمن "hw/serialio.h" // SEROFF_IER
#ضمن "cp437.h"

ساكن u8 video_صفاں(فراغ)
{
    أرجع GET_BDA(video_صفاں)+1;
}

ساكن u8 video_خانے_گنتی(فراغ)
{
    أرجع GET_BDA(video_خانے_گنتی);
}

ساكن u8 cursor_pos_خانہ(فراغ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    أرجع pos & 0xff;
}

ساكن u8 cursor_pos_صف(فراغ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    أرجع (pos >> 8) & 0xff;
}

ساكن فراغ cursor_pos_سیٹ(u8 صف, u8 خانہ)
{
    u16 pos = ((u16)صف << 8) | خانہ;
    سیٹ_BDA(cursor_pos[0], pos);
}

/****************************************************************
 * serial console output
 ****************************************************************/

VARLOW u16 sercon_پورٹ;
VARLOW u8 sercon_ونڈو;
VARLOW u8 sercon_enable;
VARFSEG بنية segoff_s sercon_real_vga_handler;

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
VARLOW u8 sercon_خانہ_آخری;
VARLOW u8 sercon_صف_آخری;
VARLOW u8 sercon_char;
VARLOW u8 sercon_attr = 0x07;

ساكن VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ساكن صحيح sercon_splitmode(فراغ)
{
    أرجع GET_کم(sercon_ونڈو);
}

ساكن فراغ sercon_putchar(u8 chr)
{
    u16 addr = GET_کم(sercon_پورٹ);
    u32 سرا = irqtimer_calc_ticks(0x0a);

#إذا 0
    /* for visual control sequence debugging */
    إذا (chr == '\x1b')
        chr = '*';
#جے_انت

    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_انکڑے);
            اكسر;
        }
        إذا (irqtimer_check(سرا)) {
            اكسر;
        }
        yield();
    }
}

ساكن فراغ sercon_رکن_reset(فراغ)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ساكن فراغ sercon_رکن_clear_screen(فراغ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ساكن فراغ sercon_رکن_no_linewrap(فراغ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ساكن فراغ sercon_رکن_cursor_goto(u8 صف, u8 خانہ)
{
    صف++; خانہ++;
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0' + صف / 10);
    sercon_putchar('0' + صف % 10);
    sercon_putchar(';');
    sercon_putchar('0' + خانہ / 10);
    sercon_putchar('0' + خانہ % 10);
    sercon_putchar('H');
}

ساكن فراغ sercon_رکن_سیٹ_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    إذا (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    إذا (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    إذا (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

ساكن فراغ sercon_سیٹ_attr(u8 attr)
{
    إذا (attr == GET_کم(sercon_attr_آخری))
        أرجع;

    سیٹ_کم(sercon_attr_آخری, attr);
    sercon_رکن_سیٹ_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ساكن فراغ sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_منزل_unicode(chr);

    إذا (unicode < 0x7f) {
        sercon_putchar(unicode);
    } وإلا إذا (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } وإلا {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

ساكن فراغ sercon_cursor_pos_سیٹ(u8 صف, u8 خانہ)
{
    إذا (!sercon_splitmode()) {
        cursor_pos_سیٹ(صف, خانہ);
    } وإلا {
        /* let vgabios update cursor */
    }
}

ساكن فراغ sercon_lazy_cursor_sync(فراغ)
{
    u8 صف = cursor_pos_صف();
    u8 خانہ = cursor_pos_خانہ();

    إذا (GET_کم(sercon_صف_آخری) == صف &&
        GET_کم(sercon_خانہ_آخری) == خانہ)
        أرجع;

    إذا (خانہ == 0 && GET_کم(sercon_صف_آخری) <= صف) {
        إذا (GET_کم(sercon_خانہ_آخری) != 0) {
            sercon_putchar('\r');
            سیٹ_کم(sercon_خانہ_آخری, 0);
        }
        طالما (GET_کم(sercon_صف_آخری) < صف) {
            sercon_putchar('\n');
            سیٹ_کم(sercon_صف_آخری, GET_کم(sercon_صف_آخری)+1);
        }
        إذا (GET_کم(sercon_صف_آخری) == صف &&
            GET_کم(sercon_خانہ_آخری) == خانہ)
            أرجع;
    }

    sercon_رکن_cursor_goto(صف, خانہ);
    سیٹ_کم(sercon_صف_آخری, صف);
    سیٹ_کم(sercon_خانہ_آخری, خانہ);
}

ساكن فراغ sercon_lazy_flush(فراغ)
{
    u8 chr, attr;

    chr = GET_کم(sercon_char);
    attr = GET_کم(sercon_attr);
    إذا (chr) {
        sercon_سیٹ_attr(attr);
        sercon_print_utf8(chr);
        سیٹ_کم(sercon_خانہ_آخری, GET_کم(sercon_خانہ_آخری) + 1);
    }

    sercon_lazy_cursor_sync();

    سیٹ_کم(sercon_attr, 0x07);
    سیٹ_کم(sercon_char, 0x00);
}

ساكن فراغ sercon_lazy_cursor_update(u8 صف, u8 خانہ)
{
    sercon_cursor_pos_سیٹ(صف, خانہ);
    سیٹ_کم(sercon_صف_آخری, صف);
    سیٹ_کم(sercon_خانہ_آخری, خانہ);
}

ساكن فراغ sercon_lazy_backspace(فراغ)
{
    u8 خانہ;

    sercon_lazy_flush();
    خانہ = cursor_pos_خانہ();
    إذا (خانہ > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_صف(), خانہ-1);
    }
}

ساكن فراغ sercon_lazy_cr(فراغ)
{
    sercon_cursor_pos_سیٹ(cursor_pos_صف(), 0);
}

ساكن فراغ sercon_lazy_lf(فراغ)
{
    u8 صف;

    صف = cursor_pos_صف() + 1;
    إذا (صف >= video_صفاں()) {
        /* scrolling up */
        صف = video_صفاں()-1;
        إذا (GET_کم(sercon_صف_آخری) > 0) {
            سیٹ_کم(sercon_صف_آخری, GET_کم(sercon_صف_آخری) - 1);
        }
    }
    sercon_cursor_pos_سیٹ(صف, cursor_pos_خانہ());
}

ساكن فراغ sercon_lazy_move_cursor(فراغ)
{
    u8 خانہ;

    خانہ = cursor_pos_خانہ() + 1;
    إذا (خانہ >= video_خانے_گنتی()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } وإلا {
        sercon_cursor_pos_سیٹ(cursor_pos_صف(), خانہ);
    }
}

ساكن فراغ sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    إذا (cursor_pos_صف() != GET_کم(sercon_صف_آخری) ||
        cursor_pos_خانہ() != GET_کم(sercon_خانہ_آخری)) {
        sercon_lazy_flush();
    }

    سیٹ_کم(sercon_char, chr);
    إذا (teletype)
        sercon_lazy_move_cursor();
    وإلا
        سیٹ_کم(sercon_attr, attr);
}

/* Set video mode */
ساكن فراغ sercon_1000(بنية bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 منوال = regs->al & 0x7f;
    u8 صفاں, خانے_گنتی;

    إذا (!sercon_splitmode()) {
        بدل (منوال) {
        حالة 0x00:
        حالة 0x01:
        حالة 0x04: /* 320x200 */
        حالة 0x05: /* 320x200 */
            خانے_گنتی = 40;
            صفاں = 25;
            regs->al = 0x30;
            اكسر;
        حالة 0x02:
        حالة 0x03:
        حالة 0x06: /* 640x200 */
        حالة 0x07:
        افتراضي:
            خانے_گنتی = 80;
            صفاں = 25;
            regs->al = 0x30;
            اكسر;
        }
        cursor_pos_سیٹ(0, 0);
        سیٹ_BDA(video_منوال, منوال);
        سیٹ_BDA(video_خانے_گنتی, خانے_گنتی);
        سیٹ_BDA(video_صفاں, صفاں-1);
        سیٹ_BDA(cursor_type, 0x0007);
    } وإلا {
        /* let vgabios handle mode init */
    }

    سیٹ_کم(sercon_enable, منوال <= 0x07);
    سیٹ_کم(sercon_خانہ_آخری, 0);
    سیٹ_کم(sercon_صف_آخری, 0);
    سیٹ_کم(sercon_attr_آخری, 0);

    sercon_رکن_reset();
    sercon_رکن_no_linewrap();
    إذا (clearscreen)
        sercon_رکن_clear_screen();
}

/* Set text-mode cursor shape */
ساكن فراغ sercon_1001(بنية bregs *regs)
{
    /* show/hide cursor? */
    سیٹ_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ساكن فراغ sercon_1002(بنية bregs *regs)
{
    sercon_cursor_pos_سیٹ(regs->dh, regs->dl);
}

/* Get cursor position */
ساكن فراغ sercon_1003(بنية bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_صف();
    regs->dl = cursor_pos_خانہ();
}

/* Scroll up window */
ساكن فراغ sercon_1006(بنية bregs *regs)
{
    sercon_lazy_flush();
    إذا (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        إذا (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_صفاں()-1 &&
            regs->dl == video_خانے_گنتی()-1) {
            sercon_سیٹ_attr(regs->bh);
            sercon_رکن_clear_screen();
        }
    } وإلا {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
ساكن فراغ sercon_1008(بنية bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
ساكن فراغ sercon_1009(بنية bregs *regs)
{
    u16 گنتی = regs->cx;

    إذا (گنتی == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } وإلا إذا (regs->al == 0x20 &&
               video_صفاں() * video_خانے_گنتی() == گنتی &&
               cursor_pos_صف() == 0 &&
               cursor_pos_خانہ() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_سیٹ_attr(regs->bl);
        sercon_رکن_clear_screen();

    } وإلا {
        sercon_lazy_flush();
        sercon_سیٹ_attr(regs->bl);
        طالما (گنتی) {
            sercon_print_utf8(regs->al);
            گنتی--;
        }
        sercon_رکن_cursor_goto(cursor_pos_صف(),
                                cursor_pos_خانہ());
    }
}

/* Teletype output */
ساكن فراغ sercon_100e(بنية bregs *regs)
{
    بدل (regs->al) {
    حالة 7:
        sercon_putchar(0x07);
        اكسر;
    حالة 8:
        sercon_lazy_backspace();
        اكسر;
    حالة '\r':
        sercon_lazy_cr();
        اكسر;
    حالة '\n':
        sercon_lazy_lf();
        اكسر;
    افتراضي:
        sercon_lazy_putchar(regs->al, 0, 1);
        اكسر;
    }
}

/* Get current video mode */
ساكن فراغ sercon_100f(بنية bregs *regs)
{
    regs->al = GET_BDA(video_منوال);
    regs->ah = GET_BDA(video_خانے_گنتی);
}

/* VBE 2.0 */
ساكن فراغ sercon_104f(بنية bregs *regs)
{
    إذا (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } وإلا {
        // Disable sercon entry point on any vesa modeset
        إذا (regs->al == 0x02)
            سیٹ_کم(sercon_enable, 0);
    }
}

ساكن فراغ sercon_10XX(بنية bregs *regs)
{
    warn_unimplemented(regs);
}

فراغ VISIBLE16
handle_sercon(بنية bregs *regs)
{
    إذا (!CONFIG_SERCON)
        أرجع;
    إذا (!GET_کم(sercon_پورٹ))
        أرجع;

    بدل (regs->ah) {
    حالة 0x01:
    حالة 0x02:
    حالة 0x03:
    حالة 0x08:
    حالة 0x0f:
        إذا (sercon_splitmode())
            /* nothing, vgabios handles it */
            أرجع;
    }

    بدل (regs->ah) {
    حالة 0x00: sercon_1000(regs); اكسر;
    حالة 0x01: sercon_1001(regs); اكسر;
    حالة 0x02: sercon_1002(regs); اكسر;
    حالة 0x03: sercon_1003(regs); اكسر;
    حالة 0x06: sercon_1006(regs); اكسر;
    حالة 0x08: sercon_1008(regs); اكسر;
    حالة 0x09: sercon_1009(regs); اكسر;
    حالة 0x0e: sercon_100e(regs); اكسر;
    حالة 0x0f: sercon_100f(regs); اكسر;
    حالة 0x4f: sercon_104f(regs); اكسر;
    افتراضي:   sercon_10XX(regs); اكسر;
    }
}

فراغ sercon_تیاری(فراغ)
{
    إذا (!CONFIG_SERCON)
        أرجع;

    بنية segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    إذا (!addr)
        أرجع;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    إذا (CONFIG_DEBUG_SERIAL)
        إذا (addr == CONFIG_DEBUG_SERIAL_پورٹ)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(اندراج_10);
    إذا (vgabios.seg != seabios.seg ||
        vgabios.وتھ_2 != seabios.وتھ_2) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.وتھ_2);
        sercon_real_vga_handler = vgabios;
        سیٹ_کم(sercon_ونڈو, 1);
    } وإلا {
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
VARLOW u8 rx_بائٹاں;

ساكن VAR16 بنية {
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

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    //اتے
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    //تھلے
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //دایاں
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //بایاں

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    //سرا
};

ساكن فراغ shiftbuf(صحيح remove)
{
    صحيح i, بچیا;

    بچیا = GET_کم(rx_بائٹاں) - remove;
    سیٹ_کم(rx_بائٹاں, بچیا);
    لكل (i = 0; i < بچیا; i++)
        سیٹ_کم(rx_buf[i], GET_کم(rx_buf[i + remove]));
}

ساكن صحيح cmpbuf(صحيح seq)
{
    صحيح chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    إذا (GET_کم(rx_بائٹاں) < len + 1)
        أرجع 0;
    لكل (chr = 0; chr < len; chr++)
        إذا (GET_GLOBAL(termseq[seq].seq[chr]) != GET_کم(rx_buf[chr + 1]))
            أرجع 0;
    أرجع 1;
}

ساكن صحيح findseq(فراغ)
{
    صحيح seq;

    لكل (seq = 0; seq < ARRAY_ناپ(termseq); seq++)
        إذا (cmpbuf(seq))
            أرجع seq;
    أرجع -1;
}

فراغ
sercon_check_event(فراغ)
{
    إذا (!CONFIG_SERCON)
        أرجع;

    u16 addr = GET_کم(sercon_پورٹ);
    u16 keycode;
    u8 byte, گنتی = 0;
    صحيح seq;

    // check to see if there is a active serial port
    إذا (!addr)
        أرجع;
    إذا (inb(addr + SEROFF_LSR) == 0xFF)
        أرجع;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    طالما (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_انکڑے);
        u8 rb = GET_کم(rx_بائٹاں);
        إذا (rb < حجم(rx_buf)) {
            سیٹ_کم(rx_buf[rb], byte);
            سیٹ_کم(rx_بائٹاں, rb + 1);
            گنتی++;
        }
    }

    لكل (;;) {
        // no (more) input data
        u8 rb = GET_کم(rx_بائٹاں);
        إذا (!rb)
            أرجع;

        // lookup escape sequences
        u8 اگلا_char = GET_کم(rx_buf[0]);
        إذا (rb > 1 && اگلا_char == 0x1b) {
            seq = findseq();
            إذا (seq >= 0) {
                قطار_وچ_پاؤ_کلید(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                استمر;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        إذا (اگلا_char == 0x1b && گنتی)
            أرجع;

        // Handle input as individual char.
        keycode = ascii_منزل_keycode(اگلا_char);
        إذا (keycode)
            قطار_وچ_پاؤ_کلید(keycode);
        shiftbuf(1);
    }
}
