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

ایستا u8 video_rows(تهی)
{
    بازگشت GET_BDA(video_rows)+1;
}

ایستا u8 video_cols(تهی)
{
    بازگشت GET_BDA(video_cols);
}

ایستا u8 cursor_pos_col(تهی)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    بازگشت pos & 0xff;
}

ایستا u8 cursor_pos_row(تهی)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    بازگشت (pos >> 8) & 0xff;
}

ایستا تهی cursor_pos_set(u8 row, u8 col)
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
VARFSEG ساختار segoff_s sercon_real_vga_handler;

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

ایستا VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ایستا صحیح sercon_splitmode(تهی)
{
    بازگشت GET_کم(sercon_split);
}

ایستا تهی sercon_putchar(u8 chr)
{
    u16 addr = GET_کم(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#اگر 0
    /* for visual control sequence debugging */
    اگر (chr == '\x1b')
        chr = '*';
#endif

    برای (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        اگر ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            بشکن;
        }
        اگر (irqtimer_check(end)) {
            بشکن;
        }
        yield();
    }
}

ایستا تهی sercon_term_reset(تهی)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ایستا تهی sercon_term_clear_screen(تهی)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ایستا تهی sercon_term_no_linewrap(تهی)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ایستا تهی sercon_term_cursor_goto(u8 row, u8 col)
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

ایستا تهی sercon_term_set_color(u8 fg, u8 bg, u8 bold)
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

ایستا تهی sercon_set_attr(u8 attr)
{
    اگر (attr == GET_کم(sercon_attr_last))
        بازگشت;

    SET_کم(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ایستا تهی sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    اگر (unicode < 0x7f) {
        sercon_putchar(unicode);
    } وگرنه اگر (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } وگرنه {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

ایستا تهی sercon_cursor_pos_set(u8 row, u8 col)
{
    اگر (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } وگرنه {
        /* let vgabios update cursor */
    }
}

ایستا تهی sercon_lazy_cursor_sync(تهی)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    اگر (GET_کم(sercon_row_last) == row &&
        GET_کم(sercon_col_last) == col)
        بازگشت;

    اگر (col == 0 && GET_کم(sercon_row_last) <= row) {
        اگر (GET_کم(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_کم(sercon_col_last, 0);
        }
        وقتی (GET_کم(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_کم(sercon_row_last, GET_کم(sercon_row_last)+1);
        }
        اگر (GET_کم(sercon_row_last) == row &&
            GET_کم(sercon_col_last) == col)
            بازگشت;
    }

    sercon_term_cursor_goto(row, col);
    SET_کم(sercon_row_last, row);
    SET_کم(sercon_col_last, col);
}

ایستا تهی sercon_lazy_flush(تهی)
{
    u8 chr, attr;

    chr = GET_کم(sercon_char);
    attr = GET_کم(sercon_attr);
    اگر (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_کم(sercon_col_last, GET_کم(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_کم(sercon_attr, 0x07);
    SET_کم(sercon_char, 0x00);
}

ایستا تهی sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_کم(sercon_row_last, row);
    SET_کم(sercon_col_last, col);
}

ایستا تهی sercon_lazy_backspace(تهی)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    اگر (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

ایستا تهی sercon_lazy_cr(تهی)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

ایستا تهی sercon_lazy_lf(تهی)
{
    u8 row;

    row = cursor_pos_row() + 1;
    اگر (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        اگر (GET_کم(sercon_row_last) > 0) {
            SET_کم(sercon_row_last, GET_کم(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

ایستا تهی sercon_lazy_move_cursor(تهی)
{
    u8 col;

    col = cursor_pos_col() + 1;
    اگر (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } وگرنه {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

ایستا تهی sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    اگر (cursor_pos_row() != GET_کم(sercon_row_last) ||
        cursor_pos_col() != GET_کم(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_کم(sercon_char, chr);
    اگر (teletype)
        sercon_lazy_move_cursor();
    وگرنه
        SET_کم(sercon_attr, attr);
}

/* Set video mode */
ایستا تهی sercon_1000(ساختار bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    اگر (!sercon_splitmode()) {
        انتخاب (mode) {
        حالت 0x00:
        حالت 0x01:
        حالت 0x04: /* 320x200 */
        حالت 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            بشکن;
        حالت 0x02:
        حالت 0x03:
        حالت 0x06: /* 640x200 */
        حالت 0x07:
        پیشفرض:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            بشکن;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } وگرنه {
        /* let vgabios handle mode init */
    }

    SET_کم(sercon_enable, mode <= 0x07);
    SET_کم(sercon_col_last, 0);
    SET_کم(sercon_row_last, 0);
    SET_کم(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    اگر (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
ایستا تهی sercon_1001(ساختار bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ایستا تهی sercon_1002(ساختار bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
ایستا تهی sercon_1003(ساختار bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
ایستا تهی sercon_1006(ساختار bregs *regs)
{
    sercon_lazy_flush();
    اگر (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        اگر (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } وگرنه {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
ایستا تهی sercon_1008(ساختار bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
ایستا تهی sercon_1009(ساختار bregs *regs)
{
    u16 شمار = regs->cx;

    اگر (شمار == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } وگرنه اگر (regs->al == 0x20 &&
               video_rows() * video_cols() == شمار &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } وگرنه {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        وقتی (شمار) {
            sercon_print_utf8(regs->al);
            شمار--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
ایستا تهی sercon_100e(ساختار bregs *regs)
{
    انتخاب (regs->al) {
    حالت 7:
        sercon_putchar(0x07);
        بشکن;
    حالت 8:
        sercon_lazy_backspace();
        بشکن;
    حالت '\r':
        sercon_lazy_cr();
        بشکن;
    حالت '\n':
        sercon_lazy_lf();
        بشکن;
    پیشفرض:
        sercon_lazy_putchar(regs->al, 0, 1);
        بشکن;
    }
}

/* Get current video mode */
ایستا تهی sercon_100f(ساختار bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
ایستا تهی sercon_104f(ساختار bregs *regs)
{
    اگر (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } وگرنه {
        // Disable sercon entry point on any vesa modeset
        اگر (regs->al == 0x02)
            SET_کم(sercon_enable, 0);
    }
}

ایستا تهی sercon_10XX(ساختار bregs *regs)
{
    warn_unimplemented(regs);
}

تهی VISIBLE16
handle_sercon(ساختار bregs *regs)
{
    اگر (!CONFIG_SERCON)
        بازگشت;
    اگر (!GET_کم(sercon_port))
        بازگشت;

    انتخاب (regs->ah) {
    حالت 0x01:
    حالت 0x02:
    حالت 0x03:
    حالت 0x08:
    حالت 0x0f:
        اگر (sercon_splitmode())
            /* nothing, vgabios handles it */
            بازگشت;
    }

    انتخاب (regs->ah) {
    حالت 0x00: sercon_1000(regs); بشکن;
    حالت 0x01: sercon_1001(regs); بشکن;
    حالت 0x02: sercon_1002(regs); بشکن;
    حالت 0x03: sercon_1003(regs); بشکن;
    حالت 0x06: sercon_1006(regs); بشکن;
    حالت 0x08: sercon_1008(regs); بشکن;
    حالت 0x09: sercon_1009(regs); بشکن;
    حالت 0x0e: sercon_100e(regs); بشکن;
    حالت 0x0f: sercon_100f(regs); بشکن;
    حالت 0x4f: sercon_104f(regs); بشکن;
    پیشفرض:   sercon_10XX(regs); بشکن;
    }
}

تهی sercon_setup(تهی)
{
    اگر (!CONFIG_SERCON)
        بازگشت;

    ساختار segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    اگر (!addr)
        بازگشت;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    اگر (CONFIG_DEBUG_SERIAL)
        اگر (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    اگر (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_کم(sercon_split, 1);
    } وگرنه {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_کم(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

ایستا VAR16 ساختار {
    نویسه seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //درج
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //راست
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //چپ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

ایستا تهی shiftbuf(صحیح remove)
{
    صحیح i, remaining;

    remaining = GET_کم(rx_bytes) - remove;
    SET_کم(rx_bytes, remaining);
    برای (i = 0; i < remaining; i++)
        SET_کم(rx_buf[i], GET_کم(rx_buf[i + remove]));
}

ایستا صحیح cmpbuf(صحیح seq)
{
    صحیح chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    اگر (GET_کم(rx_bytes) < len + 1)
        بازگشت 0;
    برای (chr = 0; chr < len; chr++)
        اگر (GET_GLOBAL(termseq[seq].seq[chr]) != GET_کم(rx_buf[chr + 1]))
            بازگشت 0;
    بازگشت 1;
}

ایستا صحیح findseq(تهی)
{
    صحیح seq;

    برای (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        اگر (cmpbuf(seq))
            بازگشت seq;
    بازگشت -1;
}

تهی
sercon_check_event(تهی)
{
    اگر (!CONFIG_SERCON)
        بازگشت;

    u16 addr = GET_کم(sercon_port);
    u16 keycode;
    u8 byte, شمار = 0;
    صحیح seq;

    // check to see if there is a active serial port
    اگر (!addr)
        بازگشت;
    اگر (inb(addr + SEROFF_LSR) == 0xFF)
        بازگشت;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    وقتی (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_کم(rx_bytes);
        اگر (rb < اندازه(rx_buf)) {
            SET_کم(rx_buf[rb], byte);
            SET_کم(rx_bytes, rb + 1);
            شمار++;
        }
    }

    برای (;;) {
        // no (more) input data
        u8 rb = GET_کم(rx_bytes);
        اگر (!rb)
            بازگشت;

        // lookup escape sequences
        u8 next_char = GET_کم(rx_buf[0]);
        اگر (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            اگر (seq >= 0) {
                enqueue_کلید(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                ادامه;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        اگر (next_char == 0x1b && شمار)
            بازگشت;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        اگر (keycode)
            enqueue_کلید(keycode);
        shiftbuf(1);
    }
}
