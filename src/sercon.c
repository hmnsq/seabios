// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // SET_BDA
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "util.h" // irqtimer_calc_ticks
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "hw/serialio.h" // SEROFF_IER
#અન્તર્ભાવ "cp437.h"

સ્થિર u8 video_rows(શૂન્ય)
{
    પાછા GET_BDA(video_rows)+1;
}

સ્થિર u8 video_cols(શૂન્ય)
{
    પાછા GET_BDA(video_cols);
}

સ્થિર u8 cursor_pos_col(શૂન્ય)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    પાછા pos & 0xff;
}

સ્થિર u8 cursor_pos_row(શૂન્ય)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    પાછા (pos >> 8) & 0xff;
}

સ્થિર શૂન્ય cursor_pos_set(u8 row, u8 col)
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
VARFSEG સંરચના segoff_s sercon_real_vga_handler;

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

સ્થિર VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

સ્થિર પૂર્ણાંક sercon_splitmode(શૂન્ય)
{
    પાછા GET_નીચલું(sercon_split);
}

સ્થિર શૂન્ય sercon_putchar(u8 chr)
{
    u16 addr = GET_નીચલું(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#જો 0
    /* for visual control sequence debugging */
    જો (chr == '\x1b')
        chr = '*';
#endif

    માટે (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        જો ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            તોડો;
        }
        જો (irqtimer_check(end)) {
            તોડો;
        }
        yield();
    }
}

સ્થિર શૂન્ય sercon_term_reset(શૂન્ય)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

સ્થિર શૂન્ય sercon_term_clear_screen(શૂન્ય)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

સ્થિર શૂન્ય sercon_term_no_linewrap(શૂન્ય)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

સ્થિર શૂન્ય sercon_term_cursor_goto(u8 row, u8 col)
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

સ્થિર શૂન્ય sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    જો (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    જો (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    જો (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

સ્થિર શૂન્ય sercon_set_attr(u8 attr)
{
    જો (attr == GET_નીચલું(sercon_attr_last))
        પાછા;

    SET_નીચલું(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

સ્થિર શૂન્ય sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    જો (unicode < 0x7f) {
        sercon_putchar(unicode);
    } નહીંતર જો (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } નહીંતર {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

સ્થિર શૂન્ય sercon_cursor_pos_set(u8 row, u8 col)
{
    જો (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } નહીંતર {
        /* let vgabios update cursor */
    }
}

સ્થિર શૂન્ય sercon_lazy_cursor_sync(શૂન્ય)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    જો (GET_નીચલું(sercon_row_last) == row &&
        GET_નીચલું(sercon_col_last) == col)
        પાછા;

    જો (col == 0 && GET_નીચલું(sercon_row_last) <= row) {
        જો (GET_નીચલું(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_નીચલું(sercon_col_last, 0);
        }
        જ્યારે (GET_નીચલું(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_નીચલું(sercon_row_last, GET_નીચલું(sercon_row_last)+1);
        }
        જો (GET_નીચલું(sercon_row_last) == row &&
            GET_નીચલું(sercon_col_last) == col)
            પાછા;
    }

    sercon_term_cursor_goto(row, col);
    SET_નીચલું(sercon_row_last, row);
    SET_નીચલું(sercon_col_last, col);
}

સ્થિર શૂન્ય sercon_lazy_flush(શૂન્ય)
{
    u8 chr, attr;

    chr = GET_નીચલું(sercon_char);
    attr = GET_નીચલું(sercon_attr);
    જો (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_નીચલું(sercon_col_last, GET_નીચલું(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_નીચલું(sercon_attr, 0x07);
    SET_નીચલું(sercon_char, 0x00);
}

સ્થિર શૂન્ય sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_નીચલું(sercon_row_last, row);
    SET_નીચલું(sercon_col_last, col);
}

સ્થિર શૂન્ય sercon_lazy_backspace(શૂન્ય)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    જો (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

સ્થિર શૂન્ય sercon_lazy_cr(શૂન્ય)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

સ્થિર શૂન્ય sercon_lazy_lf(શૂન્ય)
{
    u8 row;

    row = cursor_pos_row() + 1;
    જો (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        જો (GET_નીચલું(sercon_row_last) > 0) {
            SET_નીચલું(sercon_row_last, GET_નીચલું(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

સ્થિર શૂન્ય sercon_lazy_move_cursor(શૂન્ય)
{
    u8 col;

    col = cursor_pos_col() + 1;
    જો (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } નહીંતર {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

સ્થિર શૂન્ય sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    જો (cursor_pos_row() != GET_નીચલું(sercon_row_last) ||
        cursor_pos_col() != GET_નીચલું(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_નીચલું(sercon_char, chr);
    જો (teletype)
        sercon_lazy_move_cursor();
    નહીંતર
        SET_નીચલું(sercon_attr, attr);
}

/* Set video mode */
સ્થિર શૂન્ય sercon_1000(સંરચના bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    જો (!sercon_splitmode()) {
        પસંદગી (mode) {
        સ્થિતિ 0x00:
        સ્થિતિ 0x01:
        સ્થિતિ 0x04: /* 320x200 */
        સ્થિતિ 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            તોડો;
        સ્થિતિ 0x02:
        સ્થિતિ 0x03:
        સ્થિતિ 0x06: /* 640x200 */
        સ્થિતિ 0x07:
        મૂળભૂત:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            તોડો;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } નહીંતર {
        /* let vgabios handle mode init */
    }

    SET_નીચલું(sercon_enable, mode <= 0x07);
    SET_નીચલું(sercon_col_last, 0);
    SET_નીચલું(sercon_row_last, 0);
    SET_નીચલું(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    જો (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
સ્થિર શૂન્ય sercon_1001(સંરચના bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
સ્થિર શૂન્ય sercon_1002(સંરચના bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
સ્થિર શૂન્ય sercon_1003(સંરચના bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
સ્થિર શૂન્ય sercon_1006(સંરચના bregs *regs)
{
    sercon_lazy_flush();
    જો (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        જો (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } નહીંતર {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
સ્થિર શૂન્ય sercon_1008(સંરચના bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
સ્થિર શૂન્ય sercon_1009(સંરચના bregs *regs)
{
    u16 ગણતરી = regs->cx;

    જો (ગણતરી == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } નહીંતર જો (regs->al == 0x20 &&
               video_rows() * video_cols() == ગણતરી &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } નહીંતર {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        જ્યારે (ગણતરી) {
            sercon_print_utf8(regs->al);
            ગણતરી--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
સ્થિર શૂન્ય sercon_100e(સંરચના bregs *regs)
{
    પસંદગી (regs->al) {
    સ્થિતિ 7:
        sercon_putchar(0x07);
        તોડો;
    સ્થિતિ 8:
        sercon_lazy_backspace();
        તોડો;
    સ્થિતિ '\r':
        sercon_lazy_cr();
        તોડો;
    સ્થિતિ '\n':
        sercon_lazy_lf();
        તોડો;
    મૂળભૂત:
        sercon_lazy_putchar(regs->al, 0, 1);
        તોડો;
    }
}

/* Get current video mode */
સ્થિર શૂન્ય sercon_100f(સંરચના bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
સ્થિર શૂન્ય sercon_104f(સંરચના bregs *regs)
{
    જો (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } નહીંતર {
        // Disable sercon entry point on any vesa modeset
        જો (regs->al == 0x02)
            SET_નીચલું(sercon_enable, 0);
    }
}

સ્થિર શૂન્ય sercon_10XX(સંરચના bregs *regs)
{
    warn_unimplemented(regs);
}

શૂન્ય VISIBLE16
handle_sercon(સંરચના bregs *regs)
{
    જો (!CONFIG_SERCON)
        પાછા;
    જો (!GET_નીચલું(sercon_port))
        પાછા;

    પસંદગી (regs->ah) {
    સ્થિતિ 0x01:
    સ્થિતિ 0x02:
    સ્થિતિ 0x03:
    સ્થિતિ 0x08:
    સ્થિતિ 0x0f:
        જો (sercon_splitmode())
            /* nothing, vgabios handles it */
            પાછા;
    }

    પસંદગી (regs->ah) {
    સ્થિતિ 0x00: sercon_1000(regs); તોડો;
    સ્થિતિ 0x01: sercon_1001(regs); તોડો;
    સ્થિતિ 0x02: sercon_1002(regs); તોડો;
    સ્થિતિ 0x03: sercon_1003(regs); તોડો;
    સ્થિતિ 0x06: sercon_1006(regs); તોડો;
    સ્થિતિ 0x08: sercon_1008(regs); તોડો;
    સ્થિતિ 0x09: sercon_1009(regs); તોડો;
    સ્થિતિ 0x0e: sercon_100e(regs); તોડો;
    સ્થિતિ 0x0f: sercon_100f(regs); તોડો;
    સ્થિતિ 0x4f: sercon_104f(regs); તોડો;
    મૂળભૂત:   sercon_10XX(regs); તોડો;
    }
}

શૂન્ય sercon_setup(શૂન્ય)
{
    જો (!CONFIG_SERCON)
        પાછા;

    સંરચના segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    જો (!addr)
        પાછા;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    જો (CONFIG_DEBUG_SERIAL)
        જો (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    જો (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_નીચલું(sercon_split, 1);
    } નહીંતર {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_નીચલું(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

સ્થિર VAR16 સંરચના {
    અક્ષર seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //ઉમેરો
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //જમણું
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //ડાબું

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

સ્થિર શૂન્ય shiftbuf(પૂર્ણાંક remove)
{
    પૂર્ણાંક i, remaining;

    remaining = GET_નીચલું(rx_bytes) - remove;
    SET_નીચલું(rx_bytes, remaining);
    માટે (i = 0; i < remaining; i++)
        SET_નીચલું(rx_buf[i], GET_નીચલું(rx_buf[i + remove]));
}

સ્થિર પૂર્ણાંક cmpbuf(પૂર્ણાંક seq)
{
    પૂર્ણાંક chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    જો (GET_નીચલું(rx_bytes) < len + 1)
        પાછા 0;
    માટે (chr = 0; chr < len; chr++)
        જો (GET_GLOBAL(termseq[seq].seq[chr]) != GET_નીચલું(rx_buf[chr + 1]))
            પાછા 0;
    પાછા 1;
}

સ્થિર પૂર્ણાંક findseq(શૂન્ય)
{
    પૂર્ણાંક seq;

    માટે (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        જો (cmpbuf(seq))
            પાછા seq;
    પાછા -1;
}

શૂન્ય
sercon_check_event(શૂન્ય)
{
    જો (!CONFIG_SERCON)
        પાછા;

    u16 addr = GET_નીચલું(sercon_port);
    u16 keycode;
    u8 byte, ગણતરી = 0;
    પૂર્ણાંક seq;

    // check to see if there is a active serial port
    જો (!addr)
        પાછા;
    જો (inb(addr + SEROFF_LSR) == 0xFF)
        પાછા;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    જ્યારે (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_નીચલું(rx_bytes);
        જો (rb < કદ(rx_buf)) {
            SET_નીચલું(rx_buf[rb], byte);
            SET_નીચલું(rx_bytes, rb + 1);
            ગણતરી++;
        }
    }

    માટે (;;) {
        // no (more) input data
        u8 rb = GET_નીચલું(rx_bytes);
        જો (!rb)
            પાછા;

        // lookup escape sequences
        u8 next_char = GET_નીચલું(rx_buf[0]);
        જો (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            જો (seq >= 0) {
                enqueue_ચાવી(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                ચાલુ;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        જો (next_char == 0x1b && ગણતરી)
            પાછા;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        જો (keycode)
            enqueue_ચાવી(keycode);
        shiftbuf(1);
    }
}
