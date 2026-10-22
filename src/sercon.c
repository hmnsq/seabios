// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // SET_BDA
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "util.h" // irqtimer_calc_ticks
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "hw/serialio.h" // SEROFF_IER
#അന്തര്ഭാവ "cp437.h"

സ്ഥിതം u8 video_rows(ശൂന്യം)
{
    തിരികെ GET_BDA(video_rows)+1;
}

സ്ഥിതം u8 video_cols(ശൂന്യം)
{
    തിരികെ GET_BDA(video_cols);
}

സ്ഥിതം u8 cursor_pos_col(ശൂന്യം)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    തിരികെ pos & 0xff;
}

സ്ഥിതം u8 cursor_pos_row(ശൂന്യം)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    തിരികെ (pos >> 8) & 0xff;
}

സ്ഥിതം ശൂന്യം cursor_pos_set(u8 row, u8 col)
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
VARFSEG ഘടന segoff_s sercon_real_vga_handler;

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

സ്ഥിതം VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

സ്ഥിതം പൂർണ്ണസംഖ്യ sercon_splitmode(ശൂന്യം)
{
    തിരികെ GET_താഴെയുള്ള(sercon_split);
}

സ്ഥിതം ശൂന്യം sercon_putchar(u8 chr)
{
    u16 addr = GET_താഴെയുള്ള(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#എങ്കിൽ 0
    /* for visual control sequence debugging */
    എങ്കിൽ (chr == '\x1b')
        chr = '*';
#endif

    വേണ്ടി (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        എങ്കിൽ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            നിർത്തുക;
        }
        എങ്കിൽ (irqtimer_check(end)) {
            നിർത്തുക;
        }
        yield();
    }
}

സ്ഥിതം ശൂന്യം sercon_term_reset(ശൂന്യം)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

സ്ഥിതം ശൂന്യം sercon_term_clear_screen(ശൂന്യം)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

സ്ഥിതം ശൂന്യം sercon_term_no_linewrap(ശൂന്യം)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

സ്ഥിതം ശൂന്യം sercon_term_cursor_goto(u8 row, u8 col)
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

സ്ഥിതം ശൂന്യം sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    എങ്കിൽ (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    എങ്കിൽ (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    എങ്കിൽ (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

സ്ഥിതം ശൂന്യം sercon_set_attr(u8 attr)
{
    എങ്കിൽ (attr == GET_താഴെയുള്ള(sercon_attr_last))
        തിരികെ;

    SET_താഴെയുള്ള(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

സ്ഥിതം ശൂന്യം sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    എങ്കിൽ (unicode < 0x7f) {
        sercon_putchar(unicode);
    } അല്ലെങ്കിൽ എങ്കിൽ (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } അല്ലെങ്കിൽ {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

സ്ഥിതം ശൂന്യം sercon_cursor_pos_set(u8 row, u8 col)
{
    എങ്കിൽ (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } അല്ലെങ്കിൽ {
        /* let vgabios update cursor */
    }
}

സ്ഥിതം ശൂന്യം sercon_lazy_cursor_sync(ശൂന്യം)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    എങ്കിൽ (GET_താഴെയുള്ള(sercon_row_last) == row &&
        GET_താഴെയുള്ള(sercon_col_last) == col)
        തിരികെ;

    എങ്കിൽ (col == 0 && GET_താഴെയുള്ള(sercon_row_last) <= row) {
        എങ്കിൽ (GET_താഴെയുള്ള(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_താഴെയുള്ള(sercon_col_last, 0);
        }
        ആകുമ്പോൾ (GET_താഴെയുള്ള(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_താഴെയുള്ള(sercon_row_last, GET_താഴെയുള്ള(sercon_row_last)+1);
        }
        എങ്കിൽ (GET_താഴെയുള്ള(sercon_row_last) == row &&
            GET_താഴെയുള്ള(sercon_col_last) == col)
            തിരികെ;
    }

    sercon_term_cursor_goto(row, col);
    SET_താഴെയുള്ള(sercon_row_last, row);
    SET_താഴെയുള്ള(sercon_col_last, col);
}

സ്ഥിതം ശൂന്യം sercon_lazy_flush(ശൂന്യം)
{
    u8 chr, attr;

    chr = GET_താഴെയുള്ള(sercon_char);
    attr = GET_താഴെയുള്ള(sercon_attr);
    എങ്കിൽ (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_താഴെയുള്ള(sercon_col_last, GET_താഴെയുള്ള(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_താഴെയുള്ള(sercon_attr, 0x07);
    SET_താഴെയുള്ള(sercon_char, 0x00);
}

സ്ഥിതം ശൂന്യം sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_താഴെയുള്ള(sercon_row_last, row);
    SET_താഴെയുള്ള(sercon_col_last, col);
}

സ്ഥിതം ശൂന്യം sercon_lazy_backspace(ശൂന്യം)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    എങ്കിൽ (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

സ്ഥിതം ശൂന്യം sercon_lazy_cr(ശൂന്യം)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

സ്ഥിതം ശൂന്യം sercon_lazy_lf(ശൂന്യം)
{
    u8 row;

    row = cursor_pos_row() + 1;
    എങ്കിൽ (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        എങ്കിൽ (GET_താഴെയുള്ള(sercon_row_last) > 0) {
            SET_താഴെയുള്ള(sercon_row_last, GET_താഴെയുള്ള(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

സ്ഥിതം ശൂന്യം sercon_lazy_move_cursor(ശൂന്യം)
{
    u8 col;

    col = cursor_pos_col() + 1;
    എങ്കിൽ (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } അല്ലെങ്കിൽ {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

സ്ഥിതം ശൂന്യം sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    എങ്കിൽ (cursor_pos_row() != GET_താഴെയുള്ള(sercon_row_last) ||
        cursor_pos_col() != GET_താഴെയുള്ള(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_താഴെയുള്ള(sercon_char, chr);
    എങ്കിൽ (teletype)
        sercon_lazy_move_cursor();
    അല്ലെങ്കിൽ
        SET_താഴെയുള്ള(sercon_attr, attr);
}

/* Set video mode */
സ്ഥിതം ശൂന്യം sercon_1000(ഘടന bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    എങ്കിൽ (!sercon_splitmode()) {
        തിരഞ്ഞെടുപ്പ് (mode) {
        അവസ്ഥ 0x00:
        അവസ്ഥ 0x01:
        അവസ്ഥ 0x04: /* 320x200 */
        അവസ്ഥ 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            നിർത്തുക;
        അവസ്ഥ 0x02:
        അവസ്ഥ 0x03:
        അവസ്ഥ 0x06: /* 640x200 */
        അവസ്ഥ 0x07:
        സ്ഥിരസ്ഥിതി:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            നിർത്തുക;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } അല്ലെങ്കിൽ {
        /* let vgabios handle mode init */
    }

    SET_താഴെയുള്ള(sercon_enable, mode <= 0x07);
    SET_താഴെയുള്ള(sercon_col_last, 0);
    SET_താഴെയുള്ള(sercon_row_last, 0);
    SET_താഴെയുള്ള(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    എങ്കിൽ (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
സ്ഥിതം ശൂന്യം sercon_1001(ഘടന bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
സ്ഥിതം ശൂന്യം sercon_1002(ഘടന bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
സ്ഥിതം ശൂന്യം sercon_1003(ഘടന bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
സ്ഥിതം ശൂന്യം sercon_1006(ഘടന bregs *regs)
{
    sercon_lazy_flush();
    എങ്കിൽ (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        എങ്കിൽ (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } അല്ലെങ്കിൽ {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
സ്ഥിതം ശൂന്യം sercon_1008(ഘടന bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
സ്ഥിതം ശൂന്യം sercon_1009(ഘടന bregs *regs)
{
    u16 എണ്ണം = regs->cx;

    എങ്കിൽ (എണ്ണം == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } അല്ലെങ്കിൽ എങ്കിൽ (regs->al == 0x20 &&
               video_rows() * video_cols() == എണ്ണം &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } അല്ലെങ്കിൽ {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        ആകുമ്പോൾ (എണ്ണം) {
            sercon_print_utf8(regs->al);
            എണ്ണം--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
സ്ഥിതം ശൂന്യം sercon_100e(ഘടന bregs *regs)
{
    തിരഞ്ഞെടുപ്പ് (regs->al) {
    അവസ്ഥ 7:
        sercon_putchar(0x07);
        നിർത്തുക;
    അവസ്ഥ 8:
        sercon_lazy_backspace();
        നിർത്തുക;
    അവസ്ഥ '\r':
        sercon_lazy_cr();
        നിർത്തുക;
    അവസ്ഥ '\n':
        sercon_lazy_lf();
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        sercon_lazy_putchar(regs->al, 0, 1);
        നിർത്തുക;
    }
}

/* Get current video mode */
സ്ഥിതം ശൂന്യം sercon_100f(ഘടന bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
സ്ഥിതം ശൂന്യം sercon_104f(ഘടന bregs *regs)
{
    എങ്കിൽ (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } അല്ലെങ്കിൽ {
        // Disable sercon entry point on any vesa modeset
        എങ്കിൽ (regs->al == 0x02)
            SET_താഴെയുള്ള(sercon_enable, 0);
    }
}

സ്ഥിതം ശൂന്യം sercon_10XX(ഘടന bregs *regs)
{
    warn_unimplemented(regs);
}

ശൂന്യം VISIBLE16
handle_sercon(ഘടന bregs *regs)
{
    എങ്കിൽ (!CONFIG_SERCON)
        തിരികെ;
    എങ്കിൽ (!GET_താഴെയുള്ള(sercon_port))
        തിരികെ;

    തിരഞ്ഞെടുപ്പ് (regs->ah) {
    അവസ്ഥ 0x01:
    അവസ്ഥ 0x02:
    അവസ്ഥ 0x03:
    അവസ്ഥ 0x08:
    അവസ്ഥ 0x0f:
        എങ്കിൽ (sercon_splitmode())
            /* nothing, vgabios handles it */
            തിരികെ;
    }

    തിരഞ്ഞെടുപ്പ് (regs->ah) {
    അവസ്ഥ 0x00: sercon_1000(regs); നിർത്തുക;
    അവസ്ഥ 0x01: sercon_1001(regs); നിർത്തുക;
    അവസ്ഥ 0x02: sercon_1002(regs); നിർത്തുക;
    അവസ്ഥ 0x03: sercon_1003(regs); നിർത്തുക;
    അവസ്ഥ 0x06: sercon_1006(regs); നിർത്തുക;
    അവസ്ഥ 0x08: sercon_1008(regs); നിർത്തുക;
    അവസ്ഥ 0x09: sercon_1009(regs); നിർത്തുക;
    അവസ്ഥ 0x0e: sercon_100e(regs); നിർത്തുക;
    അവസ്ഥ 0x0f: sercon_100f(regs); നിർത്തുക;
    അവസ്ഥ 0x4f: sercon_104f(regs); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:   sercon_10XX(regs); നിർത്തുക;
    }
}

ശൂന്യം sercon_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_SERCON)
        തിരികെ;

    ഘടന segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    എങ്കിൽ (!addr)
        തിരികെ;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    എങ്കിൽ (CONFIG_DEBUG_SERIAL)
        എങ്കിൽ (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    എങ്കിൽ (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_താഴെയുള്ള(sercon_split, 1);
    } അല്ലെങ്കിൽ {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_താഴെയുള്ള(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

സ്ഥിതം VAR16 ഘടന {
    അക്ഷരം seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //തിരുകുക
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //വലത്
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //ഇടത്

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

സ്ഥിതം ശൂന്യം shiftbuf(പൂർണ്ണസംഖ്യ remove)
{
    പൂർണ്ണസംഖ്യ i, remaining;

    remaining = GET_താഴെയുള്ള(rx_bytes) - remove;
    SET_താഴെയുള്ള(rx_bytes, remaining);
    വേണ്ടി (i = 0; i < remaining; i++)
        SET_താഴെയുള്ള(rx_buf[i], GET_താഴെയുള്ള(rx_buf[i + remove]));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ cmpbuf(പൂർണ്ണസംഖ്യ seq)
{
    പൂർണ്ണസംഖ്യ chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    എങ്കിൽ (GET_താഴെയുള്ള(rx_bytes) < len + 1)
        തിരികെ 0;
    വേണ്ടി (chr = 0; chr < len; chr++)
        എങ്കിൽ (GET_GLOBAL(termseq[seq].seq[chr]) != GET_താഴെയുള്ള(rx_buf[chr + 1]))
            തിരികെ 0;
    തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ findseq(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ seq;

    വേണ്ടി (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        എങ്കിൽ (cmpbuf(seq))
            തിരികെ seq;
    തിരികെ -1;
}

ശൂന്യം
sercon_check_event(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_SERCON)
        തിരികെ;

    u16 addr = GET_താഴെയുള്ള(sercon_port);
    u16 keycode;
    u8 byte, എണ്ണം = 0;
    പൂർണ്ണസംഖ്യ seq;

    // check to see if there is a active serial port
    എങ്കിൽ (!addr)
        തിരികെ;
    എങ്കിൽ (inb(addr + SEROFF_LSR) == 0xFF)
        തിരികെ;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    ആകുമ്പോൾ (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_താഴെയുള്ള(rx_bytes);
        എങ്കിൽ (rb < വലുപ്പം(rx_buf)) {
            SET_താഴെയുള്ള(rx_buf[rb], byte);
            SET_താഴെയുള്ള(rx_bytes, rb + 1);
            എണ്ണം++;
        }
    }

    വേണ്ടി (;;) {
        // no (more) input data
        u8 rb = GET_താഴെയുള്ള(rx_bytes);
        എങ്കിൽ (!rb)
            തിരികെ;

        // lookup escape sequences
        u8 next_char = GET_താഴെയുള്ള(rx_buf[0]);
        എങ്കിൽ (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            എങ്കിൽ (seq >= 0) {
                enqueue_താക്കോൽ(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                തുടരുക;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        എങ്കിൽ (next_char == 0x1b && എണ്ണം)
            തിരികെ;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        എങ്കിൽ (keycode)
            enqueue_താക്കോൽ(keycode);
        shiftbuf(1);
    }
}
