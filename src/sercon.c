// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // SET_BDA
#समावेश "bregs.h" // struct bregs
#समावेश "stacks.h" // yield
#समावेश "output.h" // dprintf
#समावेश "util.h" // irqtimer_calc_ticks
#समावेश "string.h" // memcpy
#समावेश "romfile.h" // romfile_loadint
#समावेश "hw/serialio.h" // SEROFF_IER
#समावेश "cp437.h"

अटल u8 video_rows(व्योम)
{
    वापस GET_BDA(video_rows)+1;
}

अटल u8 video_cols(व्योम)
{
    वापस GET_BDA(video_cols);
}

अटल u8 cursor_pos_col(व्योम)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    वापस pos & 0xff;
}

अटल u8 cursor_pos_row(व्योम)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    वापस (pos >> 8) & 0xff;
}

अटल व्योम cursor_pos_set(u8 row, u8 col)
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
VARFSEG काष्ठा segoff_s sercon_real_vga_handler;

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

अटल VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

अटल पूर्णांक sercon_splitmode(व्योम)
{
    वापस GET_निचला(sercon_split);
}

अटल व्योम sercon_putchar(u8 chr)
{
    u16 addr = GET_निचला(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#अगर 0
    /* for visual control sequence debugging */
    अगर (chr == '\x1b')
        chr = '*';
#पूर्ण_अगर

    क्रम (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        अगर ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_तथ्य);
            अवरोध;
        }
        अगर (irqtimer_check(end)) {
            अवरोध;
        }
        yield();
    }
}

अटल व्योम sercon_term_reset(व्योम)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

अटल व्योम sercon_term_clear_screen(व्योम)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

अटल व्योम sercon_term_no_linewrap(व्योम)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

अटल व्योम sercon_term_cursor_goto(u8 row, u8 col)
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

अटल व्योम sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    अगर (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    अगर (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    अगर (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

अटल व्योम sercon_set_attr(u8 attr)
{
    अगर (attr == GET_निचला(sercon_attr_last))
        वापस;

    SET_निचला(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

अटल व्योम sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    अगर (unicode < 0x7f) {
        sercon_putchar(unicode);
    } अन्यथा अगर (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } अन्यथा {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

अटल व्योम sercon_cursor_pos_set(u8 row, u8 col)
{
    अगर (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } अन्यथा {
        /* let vgabios update cursor */
    }
}

अटल व्योम sercon_lazy_cursor_sync(व्योम)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    अगर (GET_निचला(sercon_row_last) == row &&
        GET_निचला(sercon_col_last) == col)
        वापस;

    अगर (col == 0 && GET_निचला(sercon_row_last) <= row) {
        अगर (GET_निचला(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_निचला(sercon_col_last, 0);
        }
        जबतक (GET_निचला(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_निचला(sercon_row_last, GET_निचला(sercon_row_last)+1);
        }
        अगर (GET_निचला(sercon_row_last) == row &&
            GET_निचला(sercon_col_last) == col)
            वापस;
    }

    sercon_term_cursor_goto(row, col);
    SET_निचला(sercon_row_last, row);
    SET_निचला(sercon_col_last, col);
}

अटल व्योम sercon_lazy_flush(व्योम)
{
    u8 chr, attr;

    chr = GET_निचला(sercon_char);
    attr = GET_निचला(sercon_attr);
    अगर (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_निचला(sercon_col_last, GET_निचला(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_निचला(sercon_attr, 0x07);
    SET_निचला(sercon_char, 0x00);
}

अटल व्योम sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_निचला(sercon_row_last, row);
    SET_निचला(sercon_col_last, col);
}

अटल व्योम sercon_lazy_backspace(व्योम)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    अगर (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

अटल व्योम sercon_lazy_cr(व्योम)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

अटल व्योम sercon_lazy_lf(व्योम)
{
    u8 row;

    row = cursor_pos_row() + 1;
    अगर (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        अगर (GET_निचला(sercon_row_last) > 0) {
            SET_निचला(sercon_row_last, GET_निचला(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

अटल व्योम sercon_lazy_move_cursor(व्योम)
{
    u8 col;

    col = cursor_pos_col() + 1;
    अगर (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } अन्यथा {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

अटल व्योम sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    अगर (cursor_pos_row() != GET_निचला(sercon_row_last) ||
        cursor_pos_col() != GET_निचला(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_निचला(sercon_char, chr);
    अगर (teletype)
        sercon_lazy_move_cursor();
    अन्यथा
        SET_निचला(sercon_attr, attr);
}

/* Set video mode */
अटल व्योम sercon_1000(काष्ठा bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    अगर (!sercon_splitmode()) {
        चयन (mode) {
        हाल 0x00:
        हाल 0x01:
        हाल 0x04: /* 320x200 */
        हाल 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            अवरोध;
        हाल 0x02:
        हाल 0x03:
        हाल 0x06: /* 640x200 */
        हाल 0x07:
        शेष:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            अवरोध;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } अन्यथा {
        /* let vgabios handle mode init */
    }

    SET_निचला(sercon_enable, mode <= 0x07);
    SET_निचला(sercon_col_last, 0);
    SET_निचला(sercon_row_last, 0);
    SET_निचला(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    अगर (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
अटल व्योम sercon_1001(काष्ठा bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
अटल व्योम sercon_1002(काष्ठा bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
अटल व्योम sercon_1003(काष्ठा bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
अटल व्योम sercon_1006(काष्ठा bregs *regs)
{
    sercon_lazy_flush();
    अगर (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        अगर (regs->ch == 0 &&
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
अटल व्योम sercon_1008(काष्ठा bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
अटल व्योम sercon_1009(काष्ठा bregs *regs)
{
    u16 गिनती = regs->cx;

    अगर (गिनती == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } अन्यथा अगर (regs->al == 0x20 &&
               video_rows() * video_cols() == गिनती &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } अन्यथा {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        जबतक (गिनती) {
            sercon_print_utf8(regs->al);
            गिनती--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
अटल व्योम sercon_100e(काष्ठा bregs *regs)
{
    चयन (regs->al) {
    हाल 7:
        sercon_putchar(0x07);
        अवरोध;
    हाल 8:
        sercon_lazy_backspace();
        अवरोध;
    हाल '\r':
        sercon_lazy_cr();
        अवरोध;
    हाल '\n':
        sercon_lazy_lf();
        अवरोध;
    शेष:
        sercon_lazy_putchar(regs->al, 0, 1);
        अवरोध;
    }
}

/* Get current video mode */
अटल व्योम sercon_100f(काष्ठा bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
अटल व्योम sercon_104f(काष्ठा bregs *regs)
{
    अगर (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } अन्यथा {
        // Disable sercon entry point on any vesa modeset
        अगर (regs->al == 0x02)
            SET_निचला(sercon_enable, 0);
    }
}

अटल व्योम sercon_10XX(काष्ठा bregs *regs)
{
    warn_unimplemented(regs);
}

व्योम VISIBLE16
handle_sercon(काष्ठा bregs *regs)
{
    अगर (!CONFIG_SERCON)
        वापस;
    अगर (!GET_निचला(sercon_port))
        वापस;

    चयन (regs->ah) {
    हाल 0x01:
    हाल 0x02:
    हाल 0x03:
    हाल 0x08:
    हाल 0x0f:
        अगर (sercon_splitmode())
            /* nothing, vgabios handles it */
            वापस;
    }

    चयन (regs->ah) {
    हाल 0x00: sercon_1000(regs); अवरोध;
    हाल 0x01: sercon_1001(regs); अवरोध;
    हाल 0x02: sercon_1002(regs); अवरोध;
    हाल 0x03: sercon_1003(regs); अवरोध;
    हाल 0x06: sercon_1006(regs); अवरोध;
    हाल 0x08: sercon_1008(regs); अवरोध;
    हाल 0x09: sercon_1009(regs); अवरोध;
    हाल 0x0e: sercon_100e(regs); अवरोध;
    हाल 0x0f: sercon_100f(regs); अवरोध;
    हाल 0x4f: sercon_104f(regs); अवरोध;
    शेष:   sercon_10XX(regs); अवरोध;
    }
}

व्योम sercon_setup(व्योम)
{
    अगर (!CONFIG_SERCON)
        वापस;

    काष्ठा segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    अगर (!addr)
        वापस;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    अगर (CONFIG_DEBUG_SERIAL)
        अगर (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    अगर (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_निचला(sercon_split, 1);
    } अन्यथा {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_निचला(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

अटल VAR16 काष्ठा {
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //जोड़ो
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //दायाँ
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //बायाँ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

अटल व्योम shiftbuf(पूर्णांक हटाओ)
{
    पूर्णांक i, remaining;

    remaining = GET_निचला(rx_bytes) - हटाओ;
    SET_निचला(rx_bytes, remaining);
    क्रम (i = 0; i < remaining; i++)
        SET_निचला(rx_buf[i], GET_निचला(rx_buf[i + हटाओ]));
}

अटल पूर्णांक cmpbuf(पूर्णांक seq)
{
    पूर्णांक chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    अगर (GET_निचला(rx_bytes) < len + 1)
        वापस 0;
    क्रम (chr = 0; chr < len; chr++)
        अगर (GET_GLOBAL(termseq[seq].seq[chr]) != GET_निचला(rx_buf[chr + 1]))
            वापस 0;
    वापस 1;
}

अटल पूर्णांक findseq(व्योम)
{
    पूर्णांक seq;

    क्रम (seq = 0; seq < ARRAY_आकार(termseq); seq++)
        अगर (cmpbuf(seq))
            वापस seq;
    वापस -1;
}

व्योम
sercon_check_event(व्योम)
{
    अगर (!CONFIG_SERCON)
        वापस;

    u16 addr = GET_निचला(sercon_port);
    u16 keycode;
    u8 byte, गिनती = 0;
    पूर्णांक seq;

    // check to see if there is a active serial port
    अगर (!addr)
        वापस;
    अगर (inb(addr + SEROFF_LSR) == 0xFF)
        वापस;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    जबतक (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_तथ्य);
        u8 rb = GET_निचला(rx_bytes);
        अगर (rb < माप(rx_buf)) {
            SET_निचला(rx_buf[rb], byte);
            SET_निचला(rx_bytes, rb + 1);
            गिनती++;
        }
    }

    क्रम (;;) {
        // no (more) input data
        u8 rb = GET_निचला(rx_bytes);
        अगर (!rb)
            वापस;

        // lookup escape sequences
        u8 अग्र_char = GET_निचला(rx_buf[0]);
        अगर (rb > 1 && अग्र_char == 0x1b) {
            seq = findseq();
            अगर (seq >= 0) {
                enqueue_कुंजी(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                जारी;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        अगर (अग्र_char == 0x1b && गिनती)
            वापस;

        // Handle input as individual char.
        keycode = ascii_to_keycode(अग्र_char);
        अगर (keycode)
            enqueue_कुंजी(keycode);
        shiftbuf(1);
    }
}
