// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // SET_BDA
#include "bregs.h" // struct bregs
#include "stacks.h" // yield
#include "output.h" // dprintf
#include "util.h" // irqtimer_calc_ticks
#include "string.h" // memcpy
#include "romfile.h" // romfile_loadint
#include "hw/serialio.h" // SEROFF_IER
#include "cp437.h"

static u8 video_rows(void)
{
    ပြန်ပေး GET_BDA(video_rows)+1;
}

static u8 video_cols(void)
{
    ပြန်ပေး GET_BDA(video_cols);
}

static u8 cursor_pos_col(void)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ပြန်ပေး pos & 0xff;
}

static u8 cursor_pos_row(void)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ပြန်ပေး (pos >> 8) & 0xff;
}

static void cursor_pos_set(u8 row, u8 col)
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
VARFSEG sa_nracanaa segoff_s sercon_real_vga_handler;

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

static VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

static ကိန်းပြည့် sercon_splitmode(void)
{
    ပြန်ပေး GET_LOW(sercon_split);
}

static void sercon_putchar(u8 chr)
{
    u16 addr = GET_LOW(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#အကယ်၍ 0
    /* for visual control sequence debugging */
    အကယ်၍ (chr == '\x1b')
        chr = '*';
#endif

    အတွက် (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        အကယ်၍ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            bha_nga;
        }
        အကယ်၍ (irqtimer_check(end)) {
            bha_nga;
        }
        yield();
    }
}

static void sercon_term_reset(void)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

static void sercon_term_clear_screen(void)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

static void sercon_term_no_linewrap(void)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

static void sercon_term_cursor_goto(u8 row, u8 col)
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

static void sercon_term_set_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    အကယ်၍ (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    အကယ်၍ (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    အကယ်၍ (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

static void sercon_set_attr(u8 attr)
{
    အကယ်၍ (attr == GET_LOW(sercon_attr_last))
        ပြန်ပေး;

    SET_LOW(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

static void sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    အကယ်၍ (unicode < 0x7f) {
        sercon_putchar(unicode);
    } မဟုတ်ပါက အကယ်၍ (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } မဟုတ်ပါက {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

static void sercon_cursor_pos_set(u8 row, u8 col)
{
    အကယ်၍ (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } မဟုတ်ပါက {
        /* let vgabios update cursor */
    }
}

static void sercon_lazy_cursor_sync(void)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    အကယ်၍ (GET_LOW(sercon_row_last) == row &&
        GET_LOW(sercon_col_last) == col)
        ပြန်ပေး;

    အကယ်၍ (col == 0 && GET_LOW(sercon_row_last) <= row) {
        အကယ်၍ (GET_LOW(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_LOW(sercon_col_last, 0);
        }
        နေစဉ် (GET_LOW(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_LOW(sercon_row_last, GET_LOW(sercon_row_last)+1);
        }
        အကယ်၍ (GET_LOW(sercon_row_last) == row &&
            GET_LOW(sercon_col_last) == col)
            ပြန်ပေး;
    }

    sercon_term_cursor_goto(row, col);
    SET_LOW(sercon_row_last, row);
    SET_LOW(sercon_col_last, col);
}

static void sercon_lazy_flush(void)
{
    u8 chr, attr;

    chr = GET_LOW(sercon_char);
    attr = GET_LOW(sercon_attr);
    အကယ်၍ (chr) {
        sercon_set_attr(attr);
        sercon_print_utf8(chr);
        SET_LOW(sercon_col_last, GET_LOW(sercon_col_last) + 1);
    }

    sercon_lazy_cursor_sync();

    SET_LOW(sercon_attr, 0x07);
    SET_LOW(sercon_char, 0x00);
}

static void sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_set(row, col);
    SET_LOW(sercon_row_last, row);
    SET_LOW(sercon_col_last, col);
}

static void sercon_lazy_backspace(void)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    အကယ်၍ (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

static void sercon_lazy_cr(void)
{
    sercon_cursor_pos_set(cursor_pos_row(), 0);
}

static void sercon_lazy_lf(void)
{
    u8 row;

    row = cursor_pos_row() + 1;
    အကယ်၍ (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        အကယ်၍ (GET_LOW(sercon_row_last) > 0) {
            SET_LOW(sercon_row_last, GET_LOW(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

static void sercon_lazy_move_cursor(void)
{
    u8 col;

    col = cursor_pos_col() + 1;
    အကယ်၍ (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } မဟုတ်ပါက {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

static void sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    အကယ်၍ (cursor_pos_row() != GET_LOW(sercon_row_last) ||
        cursor_pos_col() != GET_LOW(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_LOW(sercon_char, chr);
    အကယ်၍ (teletype)
        sercon_lazy_move_cursor();
    မဟုတ်ပါက
        SET_LOW(sercon_attr, attr);
}

/* Set video mode */
static void sercon_1000(sa_nracanaa bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    အကယ်၍ (!sercon_splitmode()) {
        switch (mode) {
        pak_sa 0x00:
        pak_sa 0x01:
        pak_sa 0x04: /* 320x200 */
        pak_sa 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            bha_nga;
        pak_sa 0x02:
        pak_sa 0x03:
        pak_sa 0x06: /* 640x200 */
        pak_sa 0x07:
        default:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            bha_nga;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } မဟုတ်ပါက {
        /* let vgabios handle mode init */
    }

    SET_LOW(sercon_enable, mode <= 0x07);
    SET_LOW(sercon_col_last, 0);
    SET_LOW(sercon_row_last, 0);
    SET_LOW(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    အကယ်၍ (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
static void sercon_1001(sa_nracanaa bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
static void sercon_1002(sa_nracanaa bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
static void sercon_1003(sa_nracanaa bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
static void sercon_1006(sa_nracanaa bregs *regs)
{
    sercon_lazy_flush();
    အကယ်၍ (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        အကယ်၍ (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } မဟုတ်ပါက {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
static void sercon_1008(sa_nracanaa bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
static void sercon_1009(sa_nracanaa bregs *regs)
{
    u16 count = regs->cx;

    အကယ်၍ (count == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } မဟုတ်ပါက အကယ်၍ (regs->al == 0x20 &&
               video_rows() * video_cols() == count &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } မဟုတ်ပါက {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        နေစဉ် (count) {
            sercon_print_utf8(regs->al);
            count--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
static void sercon_100e(sa_nracanaa bregs *regs)
{
    switch (regs->al) {
    pak_sa 7:
        sercon_putchar(0x07);
        bha_nga;
    pak_sa 8:
        sercon_lazy_backspace();
        bha_nga;
    pak_sa '\r':
        sercon_lazy_cr();
        bha_nga;
    pak_sa '\n':
        sercon_lazy_lf();
        bha_nga;
    default:
        sercon_lazy_putchar(regs->al, 0, 1);
        bha_nga;
    }
}

/* Get current video mode */
static void sercon_100f(sa_nracanaa bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
static void sercon_104f(sa_nracanaa bregs *regs)
{
    အကယ်၍ (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } မဟုတ်ပါက {
        // Disable sercon entry point on any vesa modeset
        အကယ်၍ (regs->al == 0x02)
            SET_LOW(sercon_enable, 0);
    }
}

static void sercon_10XX(sa_nracanaa bregs *regs)
{
    warn_unimplemented(regs);
}

void VISIBLE16
handle_sercon(sa_nracanaa bregs *regs)
{
    အကယ်၍ (!CONFIG_SERCON)
        ပြန်ပေး;
    အကယ်၍ (!GET_LOW(sercon_port))
        ပြန်ပေး;

    switch (regs->ah) {
    pak_sa 0x01:
    pak_sa 0x02:
    pak_sa 0x03:
    pak_sa 0x08:
    pak_sa 0x0f:
        အကယ်၍ (sercon_splitmode())
            /* nothing, vgabios handles it */
            ပြန်ပေး;
    }

    switch (regs->ah) {
    pak_sa 0x00: sercon_1000(regs); bha_nga;
    pak_sa 0x01: sercon_1001(regs); bha_nga;
    pak_sa 0x02: sercon_1002(regs); bha_nga;
    pak_sa 0x03: sercon_1003(regs); bha_nga;
    pak_sa 0x06: sercon_1006(regs); bha_nga;
    pak_sa 0x08: sercon_1008(regs); bha_nga;
    pak_sa 0x09: sercon_1009(regs); bha_nga;
    pak_sa 0x0e: sercon_100e(regs); bha_nga;
    pak_sa 0x0f: sercon_100f(regs); bha_nga;
    pak_sa 0x4f: sercon_104f(regs); bha_nga;
    default:   sercon_10XX(regs); bha_nga;
    }
}

void sercon_setup(void)
{
    အကယ်၍ (!CONFIG_SERCON)
        ပြန်ပေး;

    sa_nracanaa segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    အကယ်၍ (!addr)
        ပြန်ပေး;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    အကယ်၍ (CONFIG_DEBUG_SERIAL)
        အကယ်၍ (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    အကယ်၍ (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_LOW(sercon_split, 1);
    } မဟုတ်ပါက {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    SET_IVT(0x10, FUNC16(entry_sercon));
    SET_LOW(sercon_port, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_bytes;

static VAR16 sa_nracanaa {
    var_na seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    // insert
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    // up
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    // down
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    // right
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    // left

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    // end
};

static void shiftbuf(ကိန်းပြည့် remove)
{
    ကိန်းပြည့် i, remaining;

    remaining = GET_LOW(rx_bytes) - remove;
    SET_LOW(rx_bytes, remaining);
    အတွက် (i = 0; i < remaining; i++)
        SET_LOW(rx_buf[i], GET_LOW(rx_buf[i + remove]));
}

static ကိန်းပြည့် cmpbuf(ကိန်းပြည့် seq)
{
    ကိန်းပြည့် chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    အကယ်၍ (GET_LOW(rx_bytes) < len + 1)
        ပြန်ပေး 0;
    အတွက် (chr = 0; chr < len; chr++)
        အကယ်၍ (GET_GLOBAL(termseq[seq].seq[chr]) != GET_LOW(rx_buf[chr + 1]))
            ပြန်ပေး 0;
    ပြန်ပေး 1;
}

static ကိန်းပြည့် findseq(void)
{
    ကိန်းပြည့် seq;

    အတွက် (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        အကယ်၍ (cmpbuf(seq))
            ပြန်ပေး seq;
    ပြန်ပေး -1;
}

void
sercon_check_event(void)
{
    အကယ်၍ (!CONFIG_SERCON)
        ပြန်ပေး;

    u16 addr = GET_LOW(sercon_port);
    u16 keycode;
    u8 byte, count = 0;
    ကိန်းပြည့် seq;

    // check to see if there is a active serial port
    အကယ်၍ (!addr)
        ပြန်ပေး;
    အကယ်၍ (inb(addr + SEROFF_LSR) == 0xFF)
        ပြန်ပေး;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    နေစဉ် (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_LOW(rx_bytes);
        အကယ်၍ (rb < parimaa_na(rx_buf)) {
            SET_LOW(rx_buf[rb], byte);
            SET_LOW(rx_bytes, rb + 1);
            count++;
        }
    }

    အတွက် (;;) {
        // no (more) input data
        u8 rb = GET_LOW(rx_bytes);
        အကယ်၍ (!rb)
            ပြန်ပေး;

        // lookup escape sequences
        u8 next_char = GET_LOW(rx_buf[0]);
        အကယ်၍ (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            အကယ်၍ (seq >= 0) {
                enqueue_key(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                continue;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        အကယ်၍ (next_char == 0x1b && count)
            ပြန်ပေး;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        အကယ်၍ (keycode)
            enqueue_key(keycode);
        shiftbuf(1);
    }
}
