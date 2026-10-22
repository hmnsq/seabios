// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // SET_BDA
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "util.h" // irqtimer_calc_ticks
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "hw/serialio.h" // SEROFF_IER
#ଅନ୍ତର୍ଭାଵ "cp437.h"

ସ୍ଥିର u8 video_rows(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ GET_BDA(video_rows)+1;
}

ସ୍ଥିର u8 video_cols(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ GET_BDA(video_cols);
}

ସ୍ଥିର u8 cursor_pos_col(ଶୂନ୍ୟ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ଫେରନ୍ତୁ pos & 0xff;
}

ସ୍ଥିର u8 cursor_pos_row(ଶୂନ୍ୟ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ଫେରନ୍ତୁ (pos >> 8) & 0xff;
}

ସ୍ଥିର ଶୂନ୍ୟ cursor_pos_set(u8 row, u8 col)
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
VARFSEG ସଂରଚନା segoff_s sercon_real_vga_handler;

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

ସ୍ଥିର VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sercon_splitmode(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ GET_ତଳ(sercon_split);
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_putchar(u8 chr)
{
    u16 addr = GET_ତଳ(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#ଯଦି 0
    /* for visual control sequence debugging */
    ଯଦି (chr == '\x1b')
        chr = '*';
#endif

    ପାଇଁ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ଯଦି ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            ଭାଙ୍ଗ;
        }
        ଯଦି (irqtimer_check(end)) {
            ଭାଙ୍ଗ;
        }
        yield();
    }
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_term_reset(ଶୂନ୍ୟ)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_term_clear_screen(ଶୂନ୍ୟ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_term_no_linewrap(ଶୂନ୍ୟ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_term_cursor_goto(u8 row, u8 col)
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

ସ୍ଥିର ଶୂନ୍ୟ sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    ଯଦି (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    ଯଦି (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    ଯଦି (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_set_attr(u8 attr)
{
    ଯଦି (attr == GET_ତଳ(sercon_attr_last))
        ଫେରନ୍ତୁ;

    SET_ତଳ(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    ଯଦି (unicode < 0x7f) {
        sercon_putchar(unicode);
    } ଅନ୍ୟଥା ଯଦି (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } ଅନ୍ୟଥା {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_cursor_pos_set(u8 row, u8 col)
{
    ଯଦି (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } ଅନ୍ୟଥା {
        /* let vgabios update cursor */
    }
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_cursor_sync(ଶୂନ୍ୟ)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    ଯଦି (GET_ତଳ(sercon_row_last) == row &&
        GET_ତଳ(sercon_col_last) == col)
        ଫେରନ୍ତୁ;

    ଯଦି (col == 0 && GET_ତଳ(sercon_row_last) <= row) {
        ଯଦି (GET_ତଳ(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_ତଳ(sercon_col_last, 0);
        }
        ଯେତେବେଳେ (GET_ତଳ(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_ତଳ(sercon_row_last, GET_ତଳ(sercon_row_last)+1);
        }
        ଯଦି (GET_ତଳ(sercon_row_last) == row &&
            GET_ତଳ(sercon_col_last) == col)
            ଫେରନ୍ତୁ;
    }

    sercon_term_cursor_goto(row, col);
    SET_ତଳ(sercon_row_last, row);
    SET_ତଳ(sercon_col_last, col);
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_flush(ଶୂନ୍ୟ)
{
    u8 chr, attr;

    chr = GET_ତଳ(sercon_char);
    attr = GET_ତଳ(sercon_attr);
    ଯଦି (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_ତଳ(sercon_col_last, GET_ତଳ(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_ତଳ(sercon_attr, 0x07);
    SET_ତଳ(sercon_char, 0x00);
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_ତଳ(sercon_row_last, row);
    SET_ତଳ(sercon_col_last, col);
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_backspace(ଶୂନ୍ୟ)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    ଯଦି (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_cr(ଶୂନ୍ୟ)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_lf(ଶୂନ୍ୟ)
{
    u8 row;

    row = cursor_pos_row() + 1;
    ଯଦି (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        ଯଦି (GET_ତଳ(sercon_row_last) > 0) {
            SET_ତଳ(sercon_row_last, GET_ତଳ(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_move_cursor(ଶୂନ୍ୟ)
{
    u8 col;

    col = cursor_pos_col() + 1;
    ଯଦି (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } ଅନ୍ୟଥା {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    ଯଦି (cursor_pos_row() != GET_ତଳ(sercon_row_last) ||
        cursor_pos_col() != GET_ତଳ(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_ତଳ(sercon_char, chr);
    ଯଦି (teletype)
        sercon_lazy_move_cursor();
    ଅନ୍ୟଥା
        SET_ତଳ(sercon_attr, attr);
}

/* Set video mode */
ସ୍ଥିର ଶୂନ୍ୟ sercon_1000(ସଂରଚନା bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    ଯଦି (!sercon_splitmode()) {
        ବାଛ (mode) {
        ସ୍ଥିତି 0x00:
        ସ୍ଥିତି 0x01:
        ସ୍ଥିତି 0x04: /* 320x200 */
        ସ୍ଥିତି 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x02:
        ସ୍ଥିତି 0x03:
        ସ୍ଥିତି 0x06: /* 640x200 */
        ସ୍ଥିତି 0x07:
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            ଭାଙ୍ଗ;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } ଅନ୍ୟଥା {
        /* let vgabios handle mode init */
    }

    SET_ତଳ(sercon_enable, mode <= 0x07);
    SET_ତଳ(sercon_col_last, 0);
    SET_ତଳ(sercon_row_last, 0);
    SET_ତଳ(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    ଯଦି (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
ସ୍ଥିର ଶୂନ୍ୟ sercon_1001(ସଂରଚନା bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ସ୍ଥିର ଶୂନ୍ୟ sercon_1002(ସଂରଚନା bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
ସ୍ଥିର ଶୂନ୍ୟ sercon_1003(ସଂରଚନା bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
ସ୍ଥିର ଶୂନ୍ୟ sercon_1006(ସଂରଚନା bregs *regs)
{
    sercon_lazy_flush();
    ଯଦି (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        ଯଦି (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } ଅନ୍ୟଥା {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
ସ୍ଥିର ଶୂନ୍ୟ sercon_1008(ସଂରଚନା bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
ସ୍ଥିର ଶୂନ୍ୟ sercon_1009(ସଂରଚନା bregs *regs)
{
    u16 ଗଣନା_2 = regs->cx;

    ଯଦି (ଗଣନା_2 == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } ଅନ୍ୟଥା ଯଦି (regs->al == 0x20 &&
               video_rows() * video_cols() == ଗଣନା_2 &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } ଅନ୍ୟଥା {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        ଯେତେବେଳେ (ଗଣନା_2) {
            sercon_print_utf8(regs->al);
            ଗଣନା_2--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
ସ୍ଥିର ଶୂନ୍ୟ sercon_100e(ସଂରଚନା bregs *regs)
{
    ବାଛ (regs->al) {
    ସ୍ଥିତି 7:
        sercon_putchar(0x07);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 8:
        sercon_lazy_backspace();
        ଭାଙ୍ଗ;
    ସ୍ଥିତି '\r':
        sercon_lazy_cr();
        ଭାଙ୍ଗ;
    ସ୍ଥିତି '\n':
        sercon_lazy_lf();
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        sercon_lazy_putchar(regs->al, 0, 1);
        ଭାଙ୍ଗ;
    }
}

/* Get current video mode */
ସ୍ଥିର ଶୂନ୍ୟ sercon_100f(ସଂରଚନା bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
ସ୍ଥିର ଶୂନ୍ୟ sercon_104f(ସଂରଚନା bregs *regs)
{
    ଯଦି (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } ଅନ୍ୟଥା {
        // Disable sercon entry point on any vesa modeset
        ଯଦି (regs->al == 0x02)
            SET_ତଳ(sercon_enable, 0);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ sercon_10XX(ସଂରଚନା bregs *regs)
{
    warn_unimplemented(regs);
}

ଶୂନ୍ୟ VISIBLE16
handle_sercon(ସଂରଚନା bregs *regs)
{
    ଯଦି (!CONFIG_SERCON)
        ଫେରନ୍ତୁ;
    ଯଦି (!GET_ତଳ(sercon_port))
        ଫେରନ୍ତୁ;

    ବାଛ (regs->ah) {
    ସ୍ଥିତି 0x01:
    ସ୍ଥିତି 0x02:
    ସ୍ଥିତି 0x03:
    ସ୍ଥିତି 0x08:
    ସ୍ଥିତି 0x0f:
        ଯଦି (sercon_splitmode())
            /* nothing, vgabios handles it */
            ଫେରନ୍ତୁ;
    }

    ବାଛ (regs->ah) {
    ସ୍ଥିତି 0x00: sercon_1000(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: sercon_1001(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x02: sercon_1002(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x03: sercon_1003(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x06: sercon_1006(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x08: sercon_1008(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x09: sercon_1009(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0e: sercon_100e(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0f: sercon_100f(regs); ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x4f: sercon_104f(regs); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:   sercon_10XX(regs); ଭାଙ୍ଗ;
    }
}

ଶୂନ୍ୟ sercon_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_SERCON)
        ଫେରନ୍ତୁ;

    ସଂରଚନା segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    ଯଦି (CONFIG_DEBUG_SERIAL)
        ଯଦି (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    ଯଦି (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_ତଳ(sercon_split, 1);
    } ଅନ୍ୟଥା {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_ତଳ(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

ସ୍ଥିର VAR16 ସଂରଚନା {
    ଅକ୍ଷର seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //ଭର୍ତ୍ତି
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //ଡାହାଣ
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //ବାମ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

ସ୍ଥିର ଶୂନ୍ୟ shiftbuf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା remove)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, remaining;

    remaining = GET_ତଳ(rx_bytes) - remove;
    SET_ତଳ(rx_bytes, remaining);
    ପାଇଁ (i = 0; i < remaining; i++)
        SET_ତଳ(rx_buf[i], GET_ତଳ(rx_buf[i + remove]));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmpbuf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    ଯଦି (GET_ତଳ(rx_bytes) < len + 1)
        ଫେରନ୍ତୁ 0;
    ପାଇଁ (chr = 0; chr < len; chr++)
        ଯଦି (GET_GLOBAL(termseq[seq].seq[chr]) != GET_ତଳ(rx_buf[chr + 1]))
            ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା findseq(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq;

    ପାଇଁ (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        ଯଦି (cmpbuf(seq))
            ଫେରନ୍ତୁ seq;
    ଫେରନ୍ତୁ -1;
}

ଶୂନ୍ୟ
sercon_check_event(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_SERCON)
        ଫେରନ୍ତୁ;

    u16 addr = GET_ତଳ(sercon_port);
    u16 keycode;
    u8 byte, ଗଣନା_2 = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq;

    // check to see if there is a active serial port
    ଯଦି (!addr)
        ଫେରନ୍ତୁ;
    ଯଦି (inb(addr + SEROFF_LSR) == 0xFF)
        ଫେରନ୍ତୁ;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    ଯେତେବେଳେ (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_ତଳ(rx_bytes);
        ଯଦି (rb < ଆକାର(rx_buf)) {
            SET_ତଳ(rx_buf[rb], byte);
            SET_ତଳ(rx_bytes, rb + 1);
            ଗଣନା_2++;
        }
    }

    ପାଇଁ (;;) {
        // no (more) input data
        u8 rb = GET_ତଳ(rx_bytes);
        ଯଦି (!rb)
            ଫେରନ୍ତୁ;

        // lookup escape sequences
        u8 next_char = GET_ତଳ(rx_buf[0]);
        ଯଦି (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            ଯଦି (seq >= 0) {
                enqueue_ଚାବି(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                ଜାରି;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        ଯଦି (next_char == 0x1b && ଗଣନା_2)
            ଫେରନ୍ତୁ;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        ଯଦି (keycode)
            enqueue_ଚାବି(keycode);
        shiftbuf(1);
    }
}
