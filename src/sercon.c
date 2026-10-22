// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // SET_BDA
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "util.h" // irqtimer_calc_ticks
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "hw/serialio.h" // SEROFF_IER
#ಅನ್ತರ್ಭಾವ "cp437.h"

ಸ್ಥಿರ u8 video_rows(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ GET_BDA(video_rows)+1;
}

ಸ್ಥಿರ u8 video_cols(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ GET_BDA(video_cols);
}

ಸ್ಥಿರ u8 cursor_pos_col(ಶೂನ್ಯ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ಪ್ರತ್ಯಾಗಮ pos & 0xff;
}

ಸ್ಥಿರ u8 cursor_pos_row(ಶೂನ್ಯ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ಪ್ರತ್ಯಾಗಮ (pos >> 8) & 0xff;
}

ಸ್ಥಿರ ಶೂನ್ಯ cursor_pos_set(u8 row, u8 col)
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
VARFSEG ಸಂರಚನಾ segoff_s sercon_real_vga_handler;

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

ಸ್ಥಿರ VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sercon_splitmode(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ GET_ಕೆಳಗಿನ(sercon_split);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_putchar(u8 chr)
{
    u16 addr = GET_ಕೆಳಗಿನ(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#ಯದಿ 0
    /* for visual control sequence debugging */
    ಯದಿ (chr == '\x1b')
        chr = '*';
#endif

    ಪ್ರತಿ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ಯದಿ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            ಭಙ್ಗ;
        }
        ಯದಿ (irqtimer_check(end)) {
            ಭಙ್ಗ;
        }
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_term_reset(ಶೂನ್ಯ)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_term_clear_screen(ಶೂನ್ಯ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_term_no_linewrap(ಶೂನ್ಯ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_term_cursor_goto(u8 row, u8 col)
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

ಸ್ಥಿರ ಶೂನ್ಯ sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    ಯದಿ (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    ಯದಿ (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    ಯದಿ (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_set_attr(u8 attr)
{
    ಯದಿ (attr == GET_ಕೆಳಗಿನ(sercon_attr_last))
        ಪ್ರತ್ಯಾಗಮ;

    SET_ಕೆಳಗಿನ(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    ಯದಿ (unicode < 0x7f) {
        sercon_putchar(unicode);
    } ಅನ್ಯಥಾ ಯದಿ (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } ಅನ್ಯಥಾ {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_cursor_pos_set(u8 row, u8 col)
{
    ಯದಿ (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } ಅನ್ಯಥಾ {
        /* let vgabios update cursor */
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_cursor_sync(ಶೂನ್ಯ)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    ಯದಿ (GET_ಕೆಳಗಿನ(sercon_row_last) == row &&
        GET_ಕೆಳಗಿನ(sercon_col_last) == col)
        ಪ್ರತ್ಯಾಗಮ;

    ಯದಿ (col == 0 && GET_ಕೆಳಗಿನ(sercon_row_last) <= row) {
        ಯದಿ (GET_ಕೆಳಗಿನ(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_ಕೆಳಗಿನ(sercon_col_last, 0);
        }
        ಯಾವತ್ (GET_ಕೆಳಗಿನ(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_ಕೆಳಗಿನ(sercon_row_last, GET_ಕೆಳಗಿನ(sercon_row_last)+1);
        }
        ಯದಿ (GET_ಕೆಳಗಿನ(sercon_row_last) == row &&
            GET_ಕೆಳಗಿನ(sercon_col_last) == col)
            ಪ್ರತ್ಯಾಗಮ;
    }

    sercon_term_cursor_goto(row, col);
    SET_ಕೆಳಗಿನ(sercon_row_last, row);
    SET_ಕೆಳಗಿನ(sercon_col_last, col);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_flush(ಶೂನ್ಯ)
{
    u8 chr, attr;

    chr = GET_ಕೆಳಗಿನ(sercon_char);
    attr = GET_ಕೆಳಗಿನ(sercon_attr);
    ಯದಿ (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_ಕೆಳಗಿನ(sercon_col_last, GET_ಕೆಳಗಿನ(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_ಕೆಳಗಿನ(sercon_attr, 0x07);
    SET_ಕೆಳಗಿನ(sercon_char, 0x00);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_ಕೆಳಗಿನ(sercon_row_last, row);
    SET_ಕೆಳಗಿನ(sercon_col_last, col);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_backspace(ಶೂನ್ಯ)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    ಯದಿ (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_cr(ಶೂನ್ಯ)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_lf(ಶೂನ್ಯ)
{
    u8 row;

    row = cursor_pos_row() + 1;
    ಯದಿ (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        ಯದಿ (GET_ಕೆಳಗಿನ(sercon_row_last) > 0) {
            SET_ಕೆಳಗಿನ(sercon_row_last, GET_ಕೆಳಗಿನ(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_move_cursor(ಶೂನ್ಯ)
{
    u8 col;

    col = cursor_pos_col() + 1;
    ಯದಿ (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } ಅನ್ಯಥಾ {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    ಯದಿ (cursor_pos_row() != GET_ಕೆಳಗಿನ(sercon_row_last) ||
        cursor_pos_col() != GET_ಕೆಳಗಿನ(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_ಕೆಳಗಿನ(sercon_char, chr);
    ಯದಿ (teletype)
        sercon_lazy_move_cursor();
    ಅನ್ಯಥಾ
        SET_ಕೆಳಗಿನ(sercon_attr, attr);
}

/* Set video mode */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1000(ಸಂರಚನಾ bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    ಯದಿ (!sercon_splitmode()) {
        ವಿಕಲ್ಪ (mode) {
        ಪಕ್ಷ 0x00:
        ಪಕ್ಷ 0x01:
        ಪಕ್ಷ 0x04: /* 320x200 */
        ಪಕ್ಷ 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x02:
        ಪಕ್ಷ 0x03:
        ಪಕ್ಷ 0x06: /* 640x200 */
        ಪಕ್ಷ 0x07:
        ಸ್ವಭಾವ:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            ಭಙ್ಗ;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } ಅನ್ಯಥಾ {
        /* let vgabios handle mode init */
    }

    SET_ಕೆಳಗಿನ(sercon_enable, mode <= 0x07);
    SET_ಕೆಳಗಿನ(sercon_col_last, 0);
    SET_ಕೆಳಗಿನ(sercon_row_last, 0);
    SET_ಕೆಳಗಿನ(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    ಯದಿ (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1001(ಸಂರಚನಾ bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1002(ಸಂರಚನಾ bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1003(ಸಂರಚನಾ bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1006(ಸಂರಚನಾ bregs *regs)
{
    sercon_lazy_flush();
    ಯದಿ (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        ಯದಿ (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } ಅನ್ಯಥಾ {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1008(ಸಂರಚನಾ bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1009(ಸಂರಚನಾ bregs *regs)
{
    u16 ಎಣಿಕೆ = regs->cx;

    ಯದಿ (ಎಣಿಕೆ == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } ಅನ್ಯಥಾ ಯದಿ (regs->al == 0x20 &&
               video_rows() * video_cols() == ಎಣಿಕೆ &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } ಅನ್ಯಥಾ {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        ಯಾವತ್ (ಎಣಿಕೆ) {
            sercon_print_utf8(regs->al);
            ಎಣಿಕೆ--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_100e(ಸಂರಚನಾ bregs *regs)
{
    ವಿಕಲ್ಪ (regs->al) {
    ಪಕ್ಷ 7:
        sercon_putchar(0x07);
        ಭಙ್ಗ;
    ಪಕ್ಷ 8:
        sercon_lazy_backspace();
        ಭಙ್ಗ;
    ಪಕ್ಷ '\r':
        sercon_lazy_cr();
        ಭಙ್ಗ;
    ಪಕ್ಷ '\n':
        sercon_lazy_lf();
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        sercon_lazy_putchar(regs->al, 0, 1);
        ಭಙ್ಗ;
    }
}

/* Get current video mode */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_100f(ಸಂರಚನಾ bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_104f(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } ಅನ್ಯಥಾ {
        // Disable sercon entry point on any vesa modeset
        ಯದಿ (regs->al == 0x02)
            SET_ಕೆಳಗಿನ(sercon_enable, 0);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_10XX(ಸಂರಚನಾ bregs *regs)
{
    warn_unimplemented(regs);
}

ಶೂನ್ಯ VISIBLE16
handle_sercon(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!CONFIG_SERCON)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (!GET_ಕೆಳಗಿನ(sercon_port))
        ಪ್ರತ್ಯಾಗಮ;

    ವಿಕಲ್ಪ (regs->ah) {
    ಪಕ್ಷ 0x01:
    ಪಕ್ಷ 0x02:
    ಪಕ್ಷ 0x03:
    ಪಕ್ಷ 0x08:
    ಪಕ್ಷ 0x0f:
        ಯದಿ (sercon_splitmode())
            /* nothing, vgabios handles it */
            ಪ್ರತ್ಯಾಗಮ;
    }

    ವಿಕಲ್ಪ (regs->ah) {
    ಪಕ್ಷ 0x00: sercon_1000(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: sercon_1001(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x02: sercon_1002(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x03: sercon_1003(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x06: sercon_1006(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x08: sercon_1008(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x09: sercon_1009(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0e: sercon_100e(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x0f: sercon_100f(regs); ಭಙ್ಗ;
    ಪಕ್ಷ 0x4f: sercon_104f(regs); ಭಙ್ಗ;
    ಸ್ವಭಾವ:   sercon_10XX(regs); ಭಙ್ಗ;
    }
}

ಶೂನ್ಯ sercon_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_SERCON)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    ಯದಿ (!addr)
        ಪ್ರತ್ಯಾಗಮ;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    ಯದಿ (CONFIG_DEBUG_SERIAL)
        ಯದಿ (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    ಯದಿ (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_ಕೆಳಗಿನ(sercon_split, 1);
    } ಅನ್ಯಥಾ {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_ಕೆಳಗಿನ(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

ಸ್ಥಿರ VAR16 ಸಂರಚನಾ {
    ವರ್ಣ seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //ಸೇರಿಸು
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //ಬಲ
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //ಎಡ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

ಸ್ಥಿರ ಶೂನ್ಯ shiftbuf(ಪೂರ್ಣಾಙ್ಕ remove)
{
    ಪೂರ್ಣಾಙ್ಕ i, remaining;

    remaining = GET_ಕೆಳಗಿನ(rx_bytes) - remove;
    SET_ಕೆಳಗಿನ(rx_bytes, remaining);
    ಪ್ರತಿ (i = 0; i < remaining; i++)
        SET_ಕೆಳಗಿನ(rx_buf[i], GET_ಕೆಳಗಿನ(rx_buf[i + remove]));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cmpbuf(ಪೂರ್ಣಾಙ್ಕ seq)
{
    ಪೂರ್ಣಾಙ್ಕ chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    ಯದಿ (GET_ಕೆಳಗಿನ(rx_bytes) < len + 1)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತಿ (chr = 0; chr < len; chr++)
        ಯದಿ (GET_GLOBAL(termseq[seq].seq[chr]) != GET_ಕೆಳಗಿನ(rx_buf[chr + 1]))
            ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ findseq(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ seq;

    ಪ್ರತಿ (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        ಯದಿ (cmpbuf(seq))
            ಪ್ರತ್ಯಾಗಮ seq;
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಶೂನ್ಯ
sercon_check_event(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_SERCON)
        ಪ್ರತ್ಯಾಗಮ;

    u16 addr = GET_ಕೆಳಗಿನ(sercon_port);
    u16 keycode;
    u8 byte, ಎಣಿಕೆ = 0;
    ಪೂರ್ಣಾಙ್ಕ seq;

    // check to see if there is a active serial port
    ಯದಿ (!addr)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (inb(addr + SEROFF_LSR) == 0xFF)
        ಪ್ರತ್ಯಾಗಮ;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    ಯಾವತ್ (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_ಕೆಳಗಿನ(rx_bytes);
        ಯದಿ (rb < ಪರಿಮಾಣ(rx_buf)) {
            SET_ಕೆಳಗಿನ(rx_buf[rb], byte);
            SET_ಕೆಳಗಿನ(rx_bytes, rb + 1);
            ಎಣಿಕೆ++;
        }
    }

    ಪ್ರತಿ (;;) {
        // no (more) input data
        u8 rb = GET_ಕೆಳಗಿನ(rx_bytes);
        ಯದಿ (!rb)
            ಪ್ರತ್ಯಾಗಮ;

        // lookup escape sequences
        u8 next_char = GET_ಕೆಳಗಿನ(rx_buf[0]);
        ಯದಿ (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            ಯದಿ (seq >= 0) {
                enqueue_ಕೀಲಿ(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                ಅನುವರ್ತ;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        ಯದಿ (next_char == 0x1b && ಎಣಿಕೆ)
            ಪ್ರತ್ಯಾಗಮ;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        ಯದಿ (keycode)
            enqueue_ಕೀಲಿ(keycode);
        shiftbuf(1);
    }
}
