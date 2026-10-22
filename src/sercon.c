// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // SET_BDA
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "util.h" // irqtimer_calc_ticks
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "hw/serialio.h" // SEROFF_IER
#அந்தர்பாவ "cp437.h"

நிலையான u8 video_rows(வெறுமை)
{
    திரும்பு GET_BDA(video_rows)+1;
}

நிலையான u8 video_cols(வெறுமை)
{
    திரும்பு GET_BDA(video_cols);
}

நிலையான u8 cursor_pos_col(வெறுமை)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    திரும்பு pos & 0xff;
}

நிலையான u8 cursor_pos_row(வெறுமை)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    திரும்பு (pos >> 8) & 0xff;
}

நிலையான வெறுமை cursor_pos_set(u8 row, u8 col)
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
VARFSEG கட்டமைப்பு segoff_s sercon_real_vga_handler;

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

நிலையான VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

நிலையான முழு sercon_splitmode(வெறுமை)
{
    திரும்பு GET_கீழ்(sercon_split);
}

நிலையான வெறுமை sercon_putchar(u8 chr)
{
    u16 addr = GET_கீழ்(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#எனில் 0
    /* for visual control sequence debugging */
    எனில் (chr == '\x1b')
        chr = '*';
#endif

    க்கு (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        எனில் ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            நிறுத்து;
        }
        எனில் (irqtimer_check(end)) {
            நிறுத்து;
        }
        yield();
    }
}

நிலையான வெறுமை sercon_term_reset(வெறுமை)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

நிலையான வெறுமை sercon_term_clear_screen(வெறுமை)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

நிலையான வெறுமை sercon_term_no_linewrap(வெறுமை)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

நிலையான வெறுமை sercon_term_cursor_goto(u8 row, u8 col)
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

நிலையான வெறுமை sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    எனில் (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    எனில் (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    எனில் (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

நிலையான வெறுமை sercon_set_attr(u8 attr)
{
    எனில் (attr == GET_கீழ்(sercon_attr_last))
        திரும்பு;

    SET_கீழ்(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

நிலையான வெறுமை sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    எனில் (unicode < 0x7f) {
        sercon_putchar(unicode);
    } இல்லையெனில் எனில் (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } இல்லையெனில் {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

நிலையான வெறுமை sercon_cursor_pos_set(u8 row, u8 col)
{
    எனில் (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } இல்லையெனில் {
        /* let vgabios update cursor */
    }
}

நிலையான வெறுமை sercon_lazy_cursor_sync(வெறுமை)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    எனில் (GET_கீழ்(sercon_row_last) == row &&
        GET_கீழ்(sercon_col_last) == col)
        திரும்பு;

    எனில் (col == 0 && GET_கீழ்(sercon_row_last) <= row) {
        எனில் (GET_கீழ்(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_கீழ்(sercon_col_last, 0);
        }
        போது (GET_கீழ்(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_கீழ்(sercon_row_last, GET_கீழ்(sercon_row_last)+1);
        }
        எனில் (GET_கீழ்(sercon_row_last) == row &&
            GET_கீழ்(sercon_col_last) == col)
            திரும்பு;
    }

    sercon_term_cursor_goto(row, col);
    SET_கீழ்(sercon_row_last, row);
    SET_கீழ்(sercon_col_last, col);
}

நிலையான வெறுமை sercon_lazy_flush(வெறுமை)
{
    u8 chr, attr;

    chr = GET_கீழ்(sercon_char);
    attr = GET_கீழ்(sercon_attr);
    எனில் (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_கீழ்(sercon_col_last, GET_கீழ்(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_கீழ்(sercon_attr, 0x07);
    SET_கீழ்(sercon_char, 0x00);
}

நிலையான வெறுமை sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_கீழ்(sercon_row_last, row);
    SET_கீழ்(sercon_col_last, col);
}

நிலையான வெறுமை sercon_lazy_backspace(வெறுமை)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    எனில் (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

நிலையான வெறுமை sercon_lazy_cr(வெறுமை)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

நிலையான வெறுமை sercon_lazy_lf(வெறுமை)
{
    u8 row;

    row = cursor_pos_row() + 1;
    எனில் (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        எனில் (GET_கீழ்(sercon_row_last) > 0) {
            SET_கீழ்(sercon_row_last, GET_கீழ்(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

நிலையான வெறுமை sercon_lazy_move_cursor(வெறுமை)
{
    u8 col;

    col = cursor_pos_col() + 1;
    எனில் (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } இல்லையெனில் {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

நிலையான வெறுமை sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    எனில் (cursor_pos_row() != GET_கீழ்(sercon_row_last) ||
        cursor_pos_col() != GET_கீழ்(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_கீழ்(sercon_char, chr);
    எனில் (teletype)
        sercon_lazy_move_cursor();
    இல்லையெனில்
        SET_கீழ்(sercon_attr, attr);
}

/* Set video mode */
நிலையான வெறுமை sercon_1000(கட்டமைப்பு bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    எனில் (!sercon_splitmode()) {
        தேர்வு (mode) {
        நிலை 0x00:
        நிலை 0x01:
        நிலை 0x04: /* 320x200 */
        நிலை 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            நிறுத்து;
        நிலை 0x02:
        நிலை 0x03:
        நிலை 0x06: /* 640x200 */
        நிலை 0x07:
        இயல்பு:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            நிறுத்து;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } இல்லையெனில் {
        /* let vgabios handle mode init */
    }

    SET_கீழ்(sercon_enable, mode <= 0x07);
    SET_கீழ்(sercon_col_last, 0);
    SET_கீழ்(sercon_row_last, 0);
    SET_கீழ்(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    எனில் (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
நிலையான வெறுமை sercon_1001(கட்டமைப்பு bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
நிலையான வெறுமை sercon_1002(கட்டமைப்பு bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
நிலையான வெறுமை sercon_1003(கட்டமைப்பு bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
நிலையான வெறுமை sercon_1006(கட்டமைப்பு bregs *regs)
{
    sercon_lazy_flush();
    எனில் (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        எனில் (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } இல்லையெனில் {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
நிலையான வெறுமை sercon_1008(கட்டமைப்பு bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
நிலையான வெறுமை sercon_1009(கட்டமைப்பு bregs *regs)
{
    u16 மொத்தம் = regs->cx;

    எனில் (மொத்தம் == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } இல்லையெனில் எனில் (regs->al == 0x20 &&
               video_rows() * video_cols() == மொத்தம் &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } இல்லையெனில் {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        போது (மொத்தம்) {
            sercon_print_utf8(regs->al);
            மொத்தம்--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
நிலையான வெறுமை sercon_100e(கட்டமைப்பு bregs *regs)
{
    தேர்வு (regs->al) {
    நிலை 7:
        sercon_putchar(0x07);
        நிறுத்து;
    நிலை 8:
        sercon_lazy_backspace();
        நிறுத்து;
    நிலை '\r':
        sercon_lazy_cr();
        நிறுத்து;
    நிலை '\n':
        sercon_lazy_lf();
        நிறுத்து;
    இயல்பு:
        sercon_lazy_putchar(regs->al, 0, 1);
        நிறுத்து;
    }
}

/* Get current video mode */
நிலையான வெறுமை sercon_100f(கட்டமைப்பு bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
நிலையான வெறுமை sercon_104f(கட்டமைப்பு bregs *regs)
{
    எனில் (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } இல்லையெனில் {
        // Disable sercon entry point on any vesa modeset
        எனில் (regs->al == 0x02)
            SET_கீழ்(sercon_enable, 0);
    }
}

நிலையான வெறுமை sercon_10XX(கட்டமைப்பு bregs *regs)
{
    warn_unimplemented(regs);
}

வெறுமை VISIBLE16
handle_sercon(கட்டமைப்பு bregs *regs)
{
    எனில் (!CONFIG_SERCON)
        திரும்பு;
    எனில் (!GET_கீழ்(sercon_port))
        திரும்பு;

    தேர்வு (regs->ah) {
    நிலை 0x01:
    நிலை 0x02:
    நிலை 0x03:
    நிலை 0x08:
    நிலை 0x0f:
        எனில் (sercon_splitmode())
            /* nothing, vgabios handles it */
            திரும்பு;
    }

    தேர்வு (regs->ah) {
    நிலை 0x00: sercon_1000(regs); நிறுத்து;
    நிலை 0x01: sercon_1001(regs); நிறுத்து;
    நிலை 0x02: sercon_1002(regs); நிறுத்து;
    நிலை 0x03: sercon_1003(regs); நிறுத்து;
    நிலை 0x06: sercon_1006(regs); நிறுத்து;
    நிலை 0x08: sercon_1008(regs); நிறுத்து;
    நிலை 0x09: sercon_1009(regs); நிறுத்து;
    நிலை 0x0e: sercon_100e(regs); நிறுத்து;
    நிலை 0x0f: sercon_100f(regs); நிறுத்து;
    நிலை 0x4f: sercon_104f(regs); நிறுத்து;
    இயல்பு:   sercon_10XX(regs); நிறுத்து;
    }
}

வெறுமை sercon_setup(வெறுமை)
{
    எனில் (!CONFIG_SERCON)
        திரும்பு;

    கட்டமைப்பு segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    எனில் (!addr)
        திரும்பு;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    எனில் (CONFIG_DEBUG_SERIAL)
        எனில் (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    எனில் (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_கீழ்(sercon_split, 1);
    } இல்லையெனில் {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_கீழ்(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

நிலையான VAR16 கட்டமைப்பு {
    எழுத்து seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //செருகு
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //வலது
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //இடது

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

நிலையான வெறுமை shiftbuf(முழு remove)
{
    முழு i, remaining;

    remaining = GET_கீழ்(rx_bytes) - remove;
    SET_கீழ்(rx_bytes, remaining);
    க்கு (i = 0; i < remaining; i++)
        SET_கீழ்(rx_buf[i], GET_கீழ்(rx_buf[i + remove]));
}

நிலையான முழு cmpbuf(முழு seq)
{
    முழு chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    எனில் (GET_கீழ்(rx_bytes) < len + 1)
        திரும்பு 0;
    க்கு (chr = 0; chr < len; chr++)
        எனில் (GET_GLOBAL(termseq[seq].seq[chr]) != GET_கீழ்(rx_buf[chr + 1]))
            திரும்பு 0;
    திரும்பு 1;
}

நிலையான முழு findseq(வெறுமை)
{
    முழு seq;

    க்கு (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        எனில் (cmpbuf(seq))
            திரும்பு seq;
    திரும்பு -1;
}

வெறுமை
sercon_check_event(வெறுமை)
{
    எனில் (!CONFIG_SERCON)
        திரும்பு;

    u16 addr = GET_கீழ்(sercon_port);
    u16 keycode;
    u8 byte, மொத்தம் = 0;
    முழு seq;

    // check to see if there is a active serial port
    எனில் (!addr)
        திரும்பு;
    எனில் (inb(addr + SEROFF_LSR) == 0xFF)
        திரும்பு;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    போது (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_கீழ்(rx_bytes);
        எனில் (rb < அளவு(rx_buf)) {
            SET_கீழ்(rx_buf[rb], byte);
            SET_கீழ்(rx_bytes, rb + 1);
            மொத்தம்++;
        }
    }

    க்கு (;;) {
        // no (more) input data
        u8 rb = GET_கீழ்(rx_bytes);
        எனில் (!rb)
            திரும்பு;

        // lookup escape sequences
        u8 next_char = GET_கீழ்(rx_buf[0]);
        எனில் (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            எனில் (seq >= 0) {
                enqueue_சாவி(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                தொடர்;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        எனில் (next_char == 0x1b && மொத்தம்)
            திரும்பு;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        எனில் (keycode)
            enqueue_சாவி(keycode);
        shiftbuf(1);
    }
}
