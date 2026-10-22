// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // SET_BDA
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "util.h" // irqtimer_calc_ticks
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "hw/serialio.h" // SEROFF_IER
#अन्तर्भाव "cp437.h"

स्थिर u8 video_rows(शून्य)
{
    पटिनिवत्त GET_BDA(video_rows)+1;
}

स्थिर u8 video_cols(शून्य)
{
    पटिनिवत्त GET_BDA(video_cols);
}

स्थिर u8 cursor_pos_col(शून्य)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    पटिनिवत्त pos & 0xff;
}

स्थिर u8 cursor_pos_row(शून्य)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    पटिनिवत्त (pos >> 8) & 0xff;
}

स्थिर शून्य cursor_pos_set(u8 row, u8 col)
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
VARFSEG संरचना segoff_s sercon_real_vga_handler;

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

स्थिर VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

स्थिर पूर्णाङ्क sercon_splitmode(शून्य)
{
    पटिनिवत्त GET_नीचः(sercon_split);
}

स्थिर शून्य sercon_putchar(u8 chr)
{
    u16 addr = GET_नीचः(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#सचे 0
    /* for visual control sequence debugging */
    सचे (chr == '\x1b')
        chr = '*';
#endif

    अत्थाय (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        सचे ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            भङ्ग;
        }
        सचे (irqtimer_check(end)) {
            भङ्ग;
        }
        yield();
    }
}

स्थिर शून्य sercon_term_reset(शून्य)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

स्थिर शून्य sercon_term_clear_screen(शून्य)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

स्थिर शून्य sercon_term_no_linewrap(शून्य)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

स्थिर शून्य sercon_term_cursor_goto(u8 row, u8 col)
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

स्थिर शून्य sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    सचे (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    सचे (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    सचे (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

स्थिर शून्य sercon_set_attr(u8 attr)
{
    सचे (attr == GET_नीचः(sercon_attr_last))
        पटिनिवत्त;

    SET_नीचः(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

स्थिर शून्य sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    सचे (unicode < 0x7f) {
        sercon_putchar(unicode);
    } अञ्ञथा सचे (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } अञ्ञथा {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

स्थिर शून्य sercon_cursor_pos_set(u8 row, u8 col)
{
    सचे (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } अञ्ञथा {
        /* let vgabios update cursor */
    }
}

स्थिर शून्य sercon_lazy_cursor_sync(शून्य)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    सचे (GET_नीचः(sercon_row_last) == row &&
        GET_नीचः(sercon_col_last) == col)
        पटिनिवत्त;

    सचे (col == 0 && GET_नीचः(sercon_row_last) <= row) {
        सचे (GET_नीचः(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_नीचः(sercon_col_last, 0);
        }
        याव (GET_नीचः(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_नीचः(sercon_row_last, GET_नीचः(sercon_row_last)+1);
        }
        सचे (GET_नीचः(sercon_row_last) == row &&
            GET_नीचः(sercon_col_last) == col)
            पटिनिवत्त;
    }

    sercon_term_cursor_goto(row, col);
    SET_नीचः(sercon_row_last, row);
    SET_नीचः(sercon_col_last, col);
}

स्थिर शून्य sercon_lazy_flush(शून्य)
{
    u8 chr, attr;

    chr = GET_नीचः(sercon_char);
    attr = GET_नीचः(sercon_attr);
    सचे (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_नीचः(sercon_col_last, GET_नीचः(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_नीचः(sercon_attr, 0x07);
    SET_नीचः(sercon_char, 0x00);
}

स्थिर शून्य sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_नीचः(sercon_row_last, row);
    SET_नीचः(sercon_col_last, col);
}

स्थिर शून्य sercon_lazy_backspace(शून्य)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    सचे (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

स्थिर शून्य sercon_lazy_cr(शून्य)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

स्थिर शून्य sercon_lazy_lf(शून्य)
{
    u8 row;

    row = cursor_pos_row() + 1;
    सचे (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        सचे (GET_नीचः(sercon_row_last) > 0) {
            SET_नीचः(sercon_row_last, GET_नीचः(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

स्थिर शून्य sercon_lazy_move_cursor(शून्य)
{
    u8 col;

    col = cursor_pos_col() + 1;
    सचे (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } अञ्ञथा {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

स्थिर शून्य sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    सचे (cursor_pos_row() != GET_नीचः(sercon_row_last) ||
        cursor_pos_col() != GET_नीचः(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_नीचः(sercon_char, chr);
    सचे (teletype)
        sercon_lazy_move_cursor();
    अञ्ञथा
        SET_नीचः(sercon_attr, attr);
}

/* Set video mode */
स्थिर शून्य sercon_1000(संरचना bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    सचे (!sercon_splitmode()) {
        विकल्प (mode) {
        पक्ष 0x00:
        पक्ष 0x01:
        पक्ष 0x04: /* 320x200 */
        पक्ष 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            भङ्ग;
        पक्ष 0x02:
        पक्ष 0x03:
        पक्ष 0x06: /* 640x200 */
        पक्ष 0x07:
        स्वभाव:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            भङ्ग;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } अञ्ञथा {
        /* let vgabios handle mode init */
    }

    SET_नीचः(sercon_enable, mode <= 0x07);
    SET_नीचः(sercon_col_last, 0);
    SET_नीचः(sercon_row_last, 0);
    SET_नीचः(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    सचे (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
स्थिर शून्य sercon_1001(संरचना bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
स्थिर शून्य sercon_1002(संरचना bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
स्थिर शून्य sercon_1003(संरचना bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
स्थिर शून्य sercon_1006(संरचना bregs *regs)
{
    sercon_lazy_flush();
    सचे (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        सचे (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } अञ्ञथा {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
स्थिर शून्य sercon_1008(संरचना bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
स्थिर शून्य sercon_1009(संरचना bregs *regs)
{
    u16 गणनम् = regs->cx;

    सचे (गणनम् == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } अञ्ञथा सचे (regs->al == 0x20 &&
               video_rows() * video_cols() == गणनम् &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } अञ्ञथा {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        याव (गणनम्) {
            sercon_print_utf8(regs->al);
            गणनम्--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
स्थिर शून्य sercon_100e(संरचना bregs *regs)
{
    विकल्प (regs->al) {
    पक्ष 7:
        sercon_putchar(0x07);
        भङ्ग;
    पक्ष 8:
        sercon_lazy_backspace();
        भङ्ग;
    पक्ष '\r':
        sercon_lazy_cr();
        भङ्ग;
    पक्ष '\n':
        sercon_lazy_lf();
        भङ्ग;
    स्वभाव:
        sercon_lazy_putchar(regs->al, 0, 1);
        भङ्ग;
    }
}

/* Get current video mode */
स्थिर शून्य sercon_100f(संरचना bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
स्थिर शून्य sercon_104f(संरचना bregs *regs)
{
    सचे (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } अञ्ञथा {
        // Disable sercon entry point on any vesa modeset
        सचे (regs->al == 0x02)
            SET_नीचः(sercon_enable, 0);
    }
}

स्थिर शून्य sercon_10XX(संरचना bregs *regs)
{
    warn_unimplemented(regs);
}

शून्य VISIBLE16
handle_sercon(संरचना bregs *regs)
{
    सचे (!CONFIG_SERCON)
        पटिनिवत्त;
    सचे (!GET_नीचः(sercon_port))
        पटिनिवत्त;

    विकल्प (regs->ah) {
    पक्ष 0x01:
    पक्ष 0x02:
    पक्ष 0x03:
    पक्ष 0x08:
    पक्ष 0x0f:
        सचे (sercon_splitmode())
            /* nothing, vgabios handles it */
            पटिनिवत्त;
    }

    विकल्प (regs->ah) {
    पक्ष 0x00: sercon_1000(regs); भङ्ग;
    पक्ष 0x01: sercon_1001(regs); भङ्ग;
    पक्ष 0x02: sercon_1002(regs); भङ्ग;
    पक्ष 0x03: sercon_1003(regs); भङ्ग;
    पक्ष 0x06: sercon_1006(regs); भङ्ग;
    पक्ष 0x08: sercon_1008(regs); भङ्ग;
    पक्ष 0x09: sercon_1009(regs); भङ्ग;
    पक्ष 0x0e: sercon_100e(regs); भङ्ग;
    पक्ष 0x0f: sercon_100f(regs); भङ्ग;
    पक्ष 0x4f: sercon_104f(regs); भङ्ग;
    स्वभाव:   sercon_10XX(regs); भङ्ग;
    }
}

शून्य sercon_setup(शून्य)
{
    सचे (!CONFIG_SERCON)
        पटिनिवत्त;

    संरचना segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    सचे (!addr)
        पटिनिवत्त;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    सचे (CONFIG_DEBUG_SERIAL)
        सचे (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    सचे (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_नीचः(sercon_split, 1);
    } अञ्ञथा {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_नीचः(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

स्थिर VAR16 संरचना {
    वर्ण seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //निवेशय
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //दक्षिणः
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //वामः

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

स्थिर शून्य shiftbuf(पूर्णाङ्क remove)
{
    पूर्णाङ्क i, remaining;

    remaining = GET_नीचः(rx_bytes) - remove;
    SET_नीचः(rx_bytes, remaining);
    अत्थाय (i = 0; i < remaining; i++)
        SET_नीचः(rx_buf[i], GET_नीचः(rx_buf[i + remove]));
}

स्थिर पूर्णाङ्क cmpbuf(पूर्णाङ्क seq)
{
    पूर्णाङ्क chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    सचे (GET_नीचः(rx_bytes) < len + 1)
        पटिनिवत्त 0;
    अत्थाय (chr = 0; chr < len; chr++)
        सचे (GET_GLOBAL(termseq[seq].seq[chr]) != GET_नीचः(rx_buf[chr + 1]))
            पटिनिवत्त 0;
    पटिनिवत्त 1;
}

स्थिर पूर्णाङ्क findseq(शून्य)
{
    पूर्णाङ्क seq;

    अत्थाय (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        सचे (cmpbuf(seq))
            पटिनिवत्त seq;
    पटिनिवत्त -1;
}

शून्य
sercon_check_event(शून्य)
{
    सचे (!CONFIG_SERCON)
        पटिनिवत्त;

    u16 addr = GET_नीचः(sercon_port);
    u16 keycode;
    u8 byte, गणनम् = 0;
    पूर्णाङ्क seq;

    // check to see if there is a active serial port
    सचे (!addr)
        पटिनिवत्त;
    सचे (inb(addr + SEROFF_LSR) == 0xFF)
        पटिनिवत्त;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    याव (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_नीचः(rx_bytes);
        सचे (rb < परिमाण(rx_buf)) {
            SET_नीचः(rx_buf[rb], byte);
            SET_नीचः(rx_bytes, rb + 1);
            गणनम्++;
        }
    }

    अत्थाय (;;) {
        // no (more) input data
        u8 rb = GET_नीचः(rx_bytes);
        सचे (!rb)
            पटिनिवत्त;

        // lookup escape sequences
        u8 next_char = GET_नीचः(rx_buf[0]);
        सचे (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            सचे (seq >= 0) {
                enqueue_कुञ्चिका(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                अनुवर्त;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        सचे (next_char == 0x1b && गणनम्)
            पटिनिवत्त;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        सचे (keycode)
            enqueue_कुञ्चिका(keycode);
        shiftbuf(1);
    }
}
