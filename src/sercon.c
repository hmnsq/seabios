// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // SET_BDA
#অন্তর্ভাৱ "bregs.h" // struct bregs
#অন্তর্ভাৱ "stacks.h" // yield
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "util.h" // irqtimer_calc_ticks
#অন্তর্ভাৱ "string.h" // memcpy
#অন্তর্ভাৱ "romfile.h" // romfile_loadint
#অন্তর্ভাৱ "hw/serialio.h" // SEROFF_IER
#অন্তর্ভাৱ "cp437.h"

স্থির u8 video_rows(শূন্য)
{
    ঘূৰাই GET_BDA(video_rows)+1;
}

স্থির u8 video_cols(শূন্য)
{
    ঘূৰাই GET_BDA(video_cols);
}

স্থির u8 cursor_pos_col(শূন্য)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ঘূৰাই pos & 0xff;
}

স্থির u8 cursor_pos_row(শূন্য)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ঘূৰাই (pos >> 8) & 0xff;
}

স্থির শূন্য cursor_pos_set(u8 row, u8 col)
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
VARFSEG সংরচনা segoff_s sercon_real_vga_handler;

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

স্থির VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

স্থির পূর্ণাঙ্ক sercon_splitmode(শূন্য)
{
    ঘূৰাই GET_নিচেৰ(sercon_split);
}

স্থির শূন্য sercon_putchar(u8 chr)
{
    u16 addr = GET_নিচেৰ(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#যদি 0
    /* for visual control sequence debugging */
    যদি (chr == '\x1b')
        chr = '*';
#endif

    বাবে (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        যদি ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            ভঙ্গ;
        }
        যদি (irqtimer_check(end)) {
            ভঙ্গ;
        }
        yield();
    }
}

স্থির শূন্য sercon_term_reset(শূন্য)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

স্থির শূন্য sercon_term_clear_screen(শূন্য)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

স্থির শূন্য sercon_term_no_linewrap(শূন্য)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

স্থির শূন্য sercon_term_cursor_goto(u8 row, u8 col)
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

স্থির শূন্য sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    যদি (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    যদি (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    যদি (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

স্থির শূন্য sercon_set_attr(u8 attr)
{
    যদি (attr == GET_নিচেৰ(sercon_attr_last))
        ঘূৰাই;

    SET_নিচেৰ(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

স্থির শূন্য sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    যদি (unicode < 0x7f) {
        sercon_putchar(unicode);
    } অন্যথা যদি (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } অন্যথা {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

স্থির শূন্য sercon_cursor_pos_set(u8 row, u8 col)
{
    যদি (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } অন্যথা {
        /* let vgabios update cursor */
    }
}

স্থির শূন্য sercon_lazy_cursor_sync(শূন্য)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    যদি (GET_নিচেৰ(sercon_row_last) == row &&
        GET_নিচেৰ(sercon_col_last) == col)
        ঘূৰাই;

    যদি (col == 0 && GET_নিচেৰ(sercon_row_last) <= row) {
        যদি (GET_নিচেৰ(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_নিচেৰ(sercon_col_last, 0);
        }
        যেতিয়ালৈ (GET_নিচেৰ(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_নিচেৰ(sercon_row_last, GET_নিচেৰ(sercon_row_last)+1);
        }
        যদি (GET_নিচেৰ(sercon_row_last) == row &&
            GET_নিচেৰ(sercon_col_last) == col)
            ঘূৰাই;
    }

    sercon_term_cursor_goto(row, col);
    SET_নিচেৰ(sercon_row_last, row);
    SET_নিচেৰ(sercon_col_last, col);
}

স্থির শূন্য sercon_lazy_flush(শূন্য)
{
    u8 chr, attr;

    chr = GET_নিচেৰ(sercon_char);
    attr = GET_নিচেৰ(sercon_attr);
    যদি (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_নিচেৰ(sercon_col_last, GET_নিচেৰ(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_নিচেৰ(sercon_attr, 0x07);
    SET_নিচেৰ(sercon_char, 0x00);
}

স্থির শূন্য sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_নিচেৰ(sercon_row_last, row);
    SET_নিচেৰ(sercon_col_last, col);
}

স্থির শূন্য sercon_lazy_backspace(শূন্য)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    যদি (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

স্থির শূন্য sercon_lazy_cr(শূন্য)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

স্থির শূন্য sercon_lazy_lf(শূন্য)
{
    u8 row;

    row = cursor_pos_row() + 1;
    যদি (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        যদি (GET_নিচেৰ(sercon_row_last) > 0) {
            SET_নিচেৰ(sercon_row_last, GET_নিচেৰ(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

স্থির শূন্য sercon_lazy_move_cursor(শূন্য)
{
    u8 col;

    col = cursor_pos_col() + 1;
    যদি (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } অন্যথা {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

স্থির শূন্য sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    যদি (cursor_pos_row() != GET_নিচেৰ(sercon_row_last) ||
        cursor_pos_col() != GET_নিচেৰ(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_নিচেৰ(sercon_char, chr);
    যদি (teletype)
        sercon_lazy_move_cursor();
    অন্যথা
        SET_নিচেৰ(sercon_attr, attr);
}

/* Set video mode */
স্থির শূন্য sercon_1000(সংরচনা bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    যদি (!sercon_splitmode()) {
        ৱিকল্প (mode) {
        পক্ষ 0x00:
        পক্ষ 0x01:
        পক্ষ 0x04: /* 320x200 */
        পক্ষ 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            ভঙ্গ;
        পক্ষ 0x02:
        পক্ষ 0x03:
        পক্ষ 0x06: /* 640x200 */
        পক্ষ 0x07:
        স্ৱভাৱ:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            ভঙ্গ;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } অন্যথা {
        /* let vgabios handle mode init */
    }

    SET_নিচেৰ(sercon_enable, mode <= 0x07);
    SET_নিচেৰ(sercon_col_last, 0);
    SET_নিচেৰ(sercon_row_last, 0);
    SET_নিচেৰ(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    যদি (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
স্থির শূন্য sercon_1001(সংরচনা bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
স্থির শূন্য sercon_1002(সংরচনা bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
স্থির শূন্য sercon_1003(সংরচনা bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
স্থির শূন্য sercon_1006(সংরচনা bregs *regs)
{
    sercon_lazy_flush();
    যদি (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        যদি (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } অন্যথা {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
স্থির শূন্য sercon_1008(সংরচনা bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
স্থির শূন্য sercon_1009(সংরচনা bregs *regs)
{
    u16 গুনতি = regs->cx;

    যদি (গুনতি == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } অন্যথা যদি (regs->al == 0x20 &&
               video_rows() * video_cols() == গুনতি &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } অন্যথা {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        যেতিয়ালৈ (গুনতি) {
            sercon_print_utf8(regs->al);
            গুনতি--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
স্থির শূন্য sercon_100e(সংরচনা bregs *regs)
{
    ৱিকল্প (regs->al) {
    পক্ষ 7:
        sercon_putchar(0x07);
        ভঙ্গ;
    পক্ষ 8:
        sercon_lazy_backspace();
        ভঙ্গ;
    পক্ষ '\r':
        sercon_lazy_cr();
        ভঙ্গ;
    পক্ষ '\n':
        sercon_lazy_lf();
        ভঙ্গ;
    স্ৱভাৱ:
        sercon_lazy_putchar(regs->al, 0, 1);
        ভঙ্গ;
    }
}

/* Get current video mode */
স্থির শূন্য sercon_100f(সংরচনা bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
স্থির শূন্য sercon_104f(সংরচনা bregs *regs)
{
    যদি (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } অন্যথা {
        // Disable sercon entry point on any vesa modeset
        যদি (regs->al == 0x02)
            SET_নিচেৰ(sercon_enable, 0);
    }
}

স্থির শূন্য sercon_10XX(সংরচনা bregs *regs)
{
    warn_unimplemented(regs);
}

শূন্য VISIBLE16
handle_sercon(সংরচনা bregs *regs)
{
    যদি (!CONFIG_SERCON)
        ঘূৰাই;
    যদি (!GET_নিচেৰ(sercon_port))
        ঘূৰাই;

    ৱিকল্প (regs->ah) {
    পক্ষ 0x01:
    পক্ষ 0x02:
    পক্ষ 0x03:
    পক্ষ 0x08:
    পক্ষ 0x0f:
        যদি (sercon_splitmode())
            /* nothing, vgabios handles it */
            ঘূৰাই;
    }

    ৱিকল্প (regs->ah) {
    পক্ষ 0x00: sercon_1000(regs); ভঙ্গ;
    পক্ষ 0x01: sercon_1001(regs); ভঙ্গ;
    পক্ষ 0x02: sercon_1002(regs); ভঙ্গ;
    পক্ষ 0x03: sercon_1003(regs); ভঙ্গ;
    পক্ষ 0x06: sercon_1006(regs); ভঙ্গ;
    পক্ষ 0x08: sercon_1008(regs); ভঙ্গ;
    পক্ষ 0x09: sercon_1009(regs); ভঙ্গ;
    পক্ষ 0x0e: sercon_100e(regs); ভঙ্গ;
    পক্ষ 0x0f: sercon_100f(regs); ভঙ্গ;
    পক্ষ 0x4f: sercon_104f(regs); ভঙ্গ;
    স্ৱভাৱ:   sercon_10XX(regs); ভঙ্গ;
    }
}

শূন্য sercon_setup(শূন্য)
{
    যদি (!CONFIG_SERCON)
        ঘূৰাই;

    সংরচনা segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    যদি (!addr)
        ঘূৰাই;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    যদি (CONFIG_DEBUG_SERIAL)
        যদি (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    যদি (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_নিচেৰ(sercon_split, 1);
    } অন্যথা {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_নিচেৰ(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

স্থির VAR16 সংরচনা {
    ৱর্ণ seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //ঢোকাও
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //ডান
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //বাম

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

স্থির শূন্য shiftbuf(পূর্ণাঙ্ক remove)
{
    পূর্ণাঙ্ক i, remaining;

    remaining = GET_নিচেৰ(rx_bytes) - remove;
    SET_নিচেৰ(rx_bytes, remaining);
    বাবে (i = 0; i < remaining; i++)
        SET_নিচেৰ(rx_buf[i], GET_নিচেৰ(rx_buf[i + remove]));
}

স্থির পূর্ণাঙ্ক cmpbuf(পূর্ণাঙ্ক seq)
{
    পূর্ণাঙ্ক chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    যদি (GET_নিচেৰ(rx_bytes) < len + 1)
        ঘূৰাই 0;
    বাবে (chr = 0; chr < len; chr++)
        যদি (GET_GLOBAL(termseq[seq].seq[chr]) != GET_নিচেৰ(rx_buf[chr + 1]))
            ঘূৰাই 0;
    ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক findseq(শূন্য)
{
    পূর্ণাঙ্ক seq;

    বাবে (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        যদি (cmpbuf(seq))
            ঘূৰাই seq;
    ঘূৰাই -1;
}

শূন্য
sercon_check_event(শূন্য)
{
    যদি (!CONFIG_SERCON)
        ঘূৰাই;

    u16 addr = GET_নিচেৰ(sercon_port);
    u16 keycode;
    u8 byte, গুনতি = 0;
    পূর্ণাঙ্ক seq;

    // check to see if there is a active serial port
    যদি (!addr)
        ঘূৰাই;
    যদি (inb(addr + SEROFF_LSR) == 0xFF)
        ঘূৰাই;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    যেতিয়ালৈ (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_নিচেৰ(rx_bytes);
        যদি (rb < পরিমাণ(rx_buf)) {
            SET_নিচেৰ(rx_buf[rb], byte);
            SET_নিচেৰ(rx_bytes, rb + 1);
            গুনতি++;
        }
    }

    বাবে (;;) {
        // no (more) input data
        u8 rb = GET_নিচেৰ(rx_bytes);
        যদি (!rb)
            ঘূৰাই;

        // lookup escape sequences
        u8 next_char = GET_নিচেৰ(rx_buf[0]);
        যদি (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            যদি (seq >= 0) {
                enqueue_চাবি(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                অনুৱর্ত;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        যদি (next_char == 0x1b && গুনতি)
            ঘূৰাই;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        যদি (keycode)
            enqueue_চাবি(keycode);
        shiftbuf(1);
    }
}
