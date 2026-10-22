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
    परत GET_BDA(video_rows)+1;
}

स्थिर u8 video_cols(शून्य)
{
    परत GET_BDA(video_cols);
}

स्थिर u8 cursor_pos_col(शून्य)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    परत pos & 0xff;
}

स्थिर u8 cursor_pos_row(शून्य)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    परत (pos >> 8) & 0xff;
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

स्थिर पूर्णांक sercon_splitmode(शून्य)
{
    परत GET_खालचा(sercon_split);
}

स्थिर शून्य sercon_putchar(u8 chr)
{
    u16 addr = GET_खालचा(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#जर 0
    /* for visual control sequence debugging */
    जर (chr == '\x1b')
        chr = '*';
#endif

    साठी (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        जर ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            थांबा;
        }
        जर (irqtimer_check(end)) {
            थांबा;
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
    जर (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    जर (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    जर (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

स्थिर शून्य sercon_set_attr(u8 attr)
{
    जर (attr == GET_खालचा(sercon_attr_last))
        परत;

    SET_खालचा(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

स्थिर शून्य sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    जर (unicode < 0x7f) {
        sercon_putchar(unicode);
    } अन्यथा जर (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } अन्यथा {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

स्थिर शून्य sercon_cursor_pos_set(u8 row, u8 col)
{
    जर (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } अन्यथा {
        /* let vgabios update cursor */
    }
}

स्थिर शून्य sercon_lazy_cursor_sync(शून्य)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    जर (GET_खालचा(sercon_row_last) == row &&
        GET_खालचा(sercon_col_last) == col)
        परत;

    जर (col == 0 && GET_खालचा(sercon_row_last) <= row) {
        जर (GET_खालचा(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_खालचा(sercon_col_last, 0);
        }
        पर्यंत (GET_खालचा(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_खालचा(sercon_row_last, GET_खालचा(sercon_row_last)+1);
        }
        जर (GET_खालचा(sercon_row_last) == row &&
            GET_खालचा(sercon_col_last) == col)
            परत;
    }

    sercon_term_cursor_goto(row, col);
    SET_खालचा(sercon_row_last, row);
    SET_खालचा(sercon_col_last, col);
}

स्थिर शून्य sercon_lazy_flush(शून्य)
{
    u8 chr, attr;

    chr = GET_खालचा(sercon_char);
    attr = GET_खालचा(sercon_attr);
    जर (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_खालचा(sercon_col_last, GET_खालचा(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_खालचा(sercon_attr, 0x07);
    SET_खालचा(sercon_char, 0x00);
}

स्थिर शून्य sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_खालचा(sercon_row_last, row);
    SET_खालचा(sercon_col_last, col);
}

स्थिर शून्य sercon_lazy_backspace(शून्य)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    जर (col > 0) {
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
    जर (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        जर (GET_खालचा(sercon_row_last) > 0) {
            SET_खालचा(sercon_row_last, GET_खालचा(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

स्थिर शून्य sercon_lazy_move_cursor(शून्य)
{
    u8 col;

    col = cursor_pos_col() + 1;
    जर (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } अन्यथा {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

स्थिर शून्य sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    जर (cursor_pos_row() != GET_खालचा(sercon_row_last) ||
        cursor_pos_col() != GET_खालचा(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_खालचा(sercon_char, chr);
    जर (teletype)
        sercon_lazy_move_cursor();
    अन्यथा
        SET_खालचा(sercon_attr, attr);
}

/* Set video mode */
स्थिर शून्य sercon_1000(संरचना bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    जर (!sercon_splitmode()) {
        निवड (mode) {
        स्थिती 0x00:
        स्थिती 0x01:
        स्थिती 0x04: /* 320x200 */
        स्थिती 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            थांबा;
        स्थिती 0x02:
        स्थिती 0x03:
        स्थिती 0x06: /* 640x200 */
        स्थिती 0x07:
        पूर्वनिर्धारित:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            थांबा;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } अन्यथा {
        /* let vgabios handle mode init */
    }

    SET_खालचा(sercon_enable, mode <= 0x07);
    SET_खालचा(sercon_col_last, 0);
    SET_खालचा(sercon_row_last, 0);
    SET_खालचा(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    जर (clearscreen)
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
    जर (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        जर (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } अन्यथा {
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
    u16 मोजणी = regs->cx;

    जर (मोजणी == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } अन्यथा जर (regs->al == 0x20 &&
               video_rows() * video_cols() == मोजणी &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } अन्यथा {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        पर्यंत (मोजणी) {
            sercon_print_utf8(regs->al);
            मोजणी--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
स्थिर शून्य sercon_100e(संरचना bregs *regs)
{
    निवड (regs->al) {
    स्थिती 7:
        sercon_putchar(0x07);
        थांबा;
    स्थिती 8:
        sercon_lazy_backspace();
        थांबा;
    स्थिती '\r':
        sercon_lazy_cr();
        थांबा;
    स्थिती '\n':
        sercon_lazy_lf();
        थांबा;
    पूर्वनिर्धारित:
        sercon_lazy_putchar(regs->al, 0, 1);
        थांबा;
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
    जर (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } अन्यथा {
        // Disable sercon entry point on any vesa modeset
        जर (regs->al == 0x02)
            SET_खालचा(sercon_enable, 0);
    }
}

स्थिर शून्य sercon_10XX(संरचना bregs *regs)
{
    warn_unimplemented(regs);
}

शून्य VISIBLE16
handle_sercon(संरचना bregs *regs)
{
    जर (!CONFIG_SERCON)
        परत;
    जर (!GET_खालचा(sercon_port))
        परत;

    निवड (regs->ah) {
    स्थिती 0x01:
    स्थिती 0x02:
    स्थिती 0x03:
    स्थिती 0x08:
    स्थिती 0x0f:
        जर (sercon_splitmode())
            /* nothing, vgabios handles it */
            परत;
    }

    निवड (regs->ah) {
    स्थिती 0x00: sercon_1000(regs); थांबा;
    स्थिती 0x01: sercon_1001(regs); थांबा;
    स्थिती 0x02: sercon_1002(regs); थांबा;
    स्थिती 0x03: sercon_1003(regs); थांबा;
    स्थिती 0x06: sercon_1006(regs); थांबा;
    स्थिती 0x08: sercon_1008(regs); थांबा;
    स्थिती 0x09: sercon_1009(regs); थांबा;
    स्थिती 0x0e: sercon_100e(regs); थांबा;
    स्थिती 0x0f: sercon_100f(regs); थांबा;
    स्थिती 0x4f: sercon_104f(regs); थांबा;
    पूर्वनिर्धारित:   sercon_10XX(regs); थांबा;
    }
}

शून्य sercon_setup(शून्य)
{
    जर (!CONFIG_SERCON)
        परत;

    संरचना segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    जर (!addr)
        परत;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    जर (CONFIG_DEBUG_SERIAL)
        जर (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    जर (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_खालचा(sercon_split, 1);
    } अन्यथा {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_खालचा(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

स्थिर VAR16 संरचना {
    अक्षर seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //घाला
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //उजवा
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //डावा

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

स्थिर शून्य shiftbuf(पूर्णांक हटाओ)
{
    पूर्णांक i, remaining;

    remaining = GET_खालचा(rx_bytes) - हटाओ;
    SET_खालचा(rx_bytes, remaining);
    साठी (i = 0; i < remaining; i++)
        SET_खालचा(rx_buf[i], GET_खालचा(rx_buf[i + हटाओ]));
}

स्थिर पूर्णांक cmpbuf(पूर्णांक seq)
{
    पूर्णांक chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    जर (GET_खालचा(rx_bytes) < len + 1)
        परत 0;
    साठी (chr = 0; chr < len; chr++)
        जर (GET_GLOBAL(termseq[seq].seq[chr]) != GET_खालचा(rx_buf[chr + 1]))
            परत 0;
    परत 1;
}

स्थिर पूर्णांक findseq(शून्य)
{
    पूर्णांक seq;

    साठी (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        जर (cmpbuf(seq))
            परत seq;
    परत -1;
}

शून्य
sercon_check_event(शून्य)
{
    जर (!CONFIG_SERCON)
        परत;

    u16 addr = GET_खालचा(sercon_port);
    u16 keycode;
    u8 byte, मोजणी = 0;
    पूर्णांक seq;

    // check to see if there is a active serial port
    जर (!addr)
        परत;
    जर (inb(addr + SEROFF_LSR) == 0xFF)
        परत;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    पर्यंत (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_खालचा(rx_bytes);
        जर (rb < आकार(rx_buf)) {
            SET_खालचा(rx_buf[rb], byte);
            SET_खालचा(rx_bytes, rb + 1);
            मोजणी++;
        }
    }

    साठी (;;) {
        // no (more) input data
        u8 rb = GET_खालचा(rx_bytes);
        जर (!rb)
            परत;

        // lookup escape sequences
        u8 next_char = GET_खालचा(rx_buf[0]);
        जर (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            जर (seq >= 0) {
                enqueue_किल्ली(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                सुरूठेवा;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        जर (next_char == 0x1b && मोजणी)
            परत;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        जर (keycode)
            enqueue_किल्ली(keycode);
        shiftbuf(1);
    }
}
