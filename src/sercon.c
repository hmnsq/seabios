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

ساكن u8 video_rows(فراغ)
{
    أرجع GET_BDA(video_rows)+1;
}

ساكن u8 video_cols(فراغ)
{
    أرجع GET_BDA(video_cols);
}

ساكن u8 cursor_pos_col(فراغ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    أرجع pos & 0xff;
}

ساكن u8 cursor_pos_row(فراغ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    أرجع (pos >> 8) & 0xff;
}

ساكن فراغ cursor_pos_set(u8 row, u8 col)
{
    u16 pos = ((u16)row << 8) | col;
    SET_BDA(cursor_pos[0], pos);
}

/****************************************************************
 * serial console output
 ****************************************************************/

VARLOW u16 sercon_port;
VARLOW u8 sercon_split;
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
VARLOW u8 sercon_attr_last;
VARLOW u8 sercon_col_last;
VARLOW u8 sercon_row_last;
VARLOW u8 sercon_char;
VARLOW u8 sercon_attr = 0x07;

ساكن VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ساكن صحيح sercon_splitmode(فراغ)
{
    أرجع GET_نزم(sercon_split);
}

ساكن فراغ sercon_putchar(u8 chr)
{
    u16 addr = GET_نزم(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#إذا 0
    /* for visual control sequence debugging */
    إذا (chr == '\x1b')
        chr = '*';
#endif

    لكل (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        إذا ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            اكسر;
        }
        إذا (irqtimer_check(end)) {
            اكسر;
        }
        yield();
    }
}

ساكن فراغ sercon_term_reset(فراغ)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ساكن فراغ sercon_term_clear_screen(فراغ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ساكن فراغ sercon_term_no_linewrap(فراغ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ساكن فراغ sercon_term_cursor_goto(u8 row, u8 col)
{
    row++; col++;
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0' + row / 10);
    sercon_putchar('0' + row % 10);
    sercon_putchar(';');
    sercon_putchar('0' + col / 10);
    sercon_putchar('0' + col % 10);
    sercon_putchar('H');
}

ساكن فراغ sercon_term_set_color(u8 fg, u8 bg, u8 bold)
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

ساكن فراغ sercon_set_attr(u8 attr)
{
    إذا (attr == GET_نزم(sercon_attr_last))
        أرجع;

    SET_نزم(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ساكن فراغ sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

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

ساكن فراغ sercon_cursor_pos_set(u8 row, u8 col)
{
    إذا (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } وإلا {
        /* let vgabios update cursor */
    }
}

ساكن فراغ sercon_lazy_cursor_sync(فراغ)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    إذا (GET_نزم(sercon_row_last) == row &&
        GET_نزم(sercon_col_last) == col)
        أرجع;

    إذا (col == 0 && GET_نزم(sercon_row_last) <= row) {
        إذا (GET_نزم(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_نزم(sercon_col_last, 0);
        }
        طالما (GET_نزم(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_نزم(sercon_row_last, GET_نزم(sercon_row_last)+1);
        }
        إذا (GET_نزم(sercon_row_last) == row &&
            GET_نزم(sercon_col_last) == col)
            أرجع;
    }

    sercon_term_cursor_goto(row, col);
    SET_نزم(sercon_row_last, row);
    SET_نزم(sercon_col_last, col);
}

ساكن فراغ sercon_lazy_flush(فراغ)
{
    u8 chr, attr;

    chr = GET_نزم(sercon_char);
    attr = GET_نزم(sercon_attr);
    إذا (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_نزم(sercon_col_last, GET_نزم(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_نزم(sercon_attr, 0x07);
    SET_نزم(sercon_char, 0x00);
}

ساكن فراغ sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_نزم(sercon_row_last, row);
    SET_نزم(sercon_col_last, col);
}

ساكن فراغ sercon_lazy_backspace(فراغ)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    إذا (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

ساكن فراغ sercon_lazy_cr(فراغ)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

ساكن فراغ sercon_lazy_lf(فراغ)
{
    u8 row;

    row = cursor_pos_row() + 1;
    إذا (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        إذا (GET_نزم(sercon_row_last) > 0) {
            SET_نزم(sercon_row_last, GET_نزم(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

ساكن فراغ sercon_lazy_move_cursor(فراغ)
{
    u8 col;

    col = cursor_pos_col() + 1;
    إذا (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } وإلا {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

ساكن فراغ sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    إذا (cursor_pos_row() != GET_نزم(sercon_row_last) ||
        cursor_pos_col() != GET_نزم(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_نزم(sercon_char, chr);
    إذا (teletype)
        sercon_lazy_move_cursor();
    وإلا
        SET_نزم(sercon_attr, attr);
}

/* Set video mode */
ساكن فراغ sercon_1000(بنية bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    إذا (!sercon_splitmode()) {
        بدل (mode) {
        حالة 0x00:
        حالة 0x01:
        حالة 0x04: /* 320x200 */
        حالة 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            اكسر;
        حالة 0x02:
        حالة 0x03:
        حالة 0x06: /* 640x200 */
        حالة 0x07:
        افتراضي:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            اكسر;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } وإلا {
        /* let vgabios handle mode init */
    }

    SET_نزم(sercon_enable, mode <= 0x07);
    SET_نزم(sercon_col_last, 0);
    SET_نزم(sercon_row_last, 0);
    SET_نزم(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    إذا (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
ساكن فراغ sercon_1001(بنية bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ساكن فراغ sercon_1002(بنية bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
ساكن فراغ sercon_1003(بنية bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
ساكن فراغ sercon_1006(بنية bregs *regs)
{
    sercon_lazy_flush();
    إذا (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        إذا (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
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
    u16 ژماردن = regs->cx;

    إذا (ژماردن == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } وإلا إذا (regs->al == 0x20 &&
               video_rows() * video_cols() == ژماردن &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } وإلا {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        طالما (ژماردن) {
            sercon_print_utf8(regs->al);
            ژماردن--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
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
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
ساكن فراغ sercon_104f(بنية bregs *regs)
{
    إذا (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } وإلا {
        // Disable sercon entry point on any vesa modeset
        إذا (regs->al == 0x02)
            SET_نزم(sercon_enable, 0);
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
    إذا (!GET_نزم(sercon_port))
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

فراغ sercon_setup(فراغ)
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
        إذا (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    إذا (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_نزم(sercon_split, 1);
    } وإلا {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_نزم(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //زیادکردن
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //ڕاست
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //چەپ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

ساكن فراغ shiftbuf(صحيح remove)
{
    صحيح i, remaining;

    remaining = GET_نزم(rx_bytes) - remove;
    SET_نزم(rx_bytes, remaining);
    لكل (i = 0; i < remaining; i++)
        SET_نزم(rx_buf[i], GET_نزم(rx_buf[i + remove]));
}

ساكن صحيح cmpbuf(صحيح seq)
{
    صحيح chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    إذا (GET_نزم(rx_bytes) < len + 1)
        أرجع 0;
    لكل (chr = 0; chr < len; chr++)
        إذا (GET_GLOBAL(termseq[seq].seq[chr]) != GET_نزم(rx_buf[chr + 1]))
            أرجع 0;
    أرجع 1;
}

ساكن صحيح findseq(فراغ)
{
    صحيح seq;

    لكل (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        إذا (cmpbuf(seq))
            أرجع seq;
    أرجع -1;
}

فراغ
sercon_check_event(فراغ)
{
    إذا (!CONFIG_SERCON)
        أرجع;

    u16 addr = GET_نزم(sercon_port);
    u16 keycode;
    u8 byte, ژماردن = 0;
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
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_نزم(rx_bytes);
        إذا (rb < حجم(rx_buf)) {
            SET_نزم(rx_buf[rb], byte);
            SET_نزم(rx_bytes, rb + 1);
            ژماردن++;
        }
    }

    لكل (;;) {
        // no (more) input data
        u8 rb = GET_نزم(rx_bytes);
        إذا (!rb)
            أرجع;

        // lookup escape sequences
        u8 next_char = GET_نزم(rx_buf[0]);
        إذا (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            إذا (seq >= 0) {
                enqueue_کلیل(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                استمر;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        إذا (next_char == 0x1b && ژماردن)
            أرجع;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        إذا (keycode)
            enqueue_کلیل(keycode);
        shiftbuf(1);
    }
}
