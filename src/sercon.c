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
    반환 GET_BDA(video_rows)+1;
}

static u8 video_cols(void)
{
    반환 GET_BDA(video_cols);
}

static u8 cursor_pos_col(void)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    반환 pos & 0xff;
}

static u8 cursor_pos_row(void)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    반환 (pos >> 8) & 0xff;
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
VARFSEG struct segoff_s sercon_real_vga_handler;

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

static 정수 sercon_splitmode(void)
{
    반환 GET_LOW(sercon_split);
}

static void sercon_putchar(u8 chr)
{
    u16 addr = GET_LOW(sercon_port);
    u32 end = irqtimer_calc_ticks(0x0a);

#만약 0
    /* for visual control sequence debugging */
    만약 (chr == '\x1b')
        chr = '*';
#endif

    위해 (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        만약 ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_DATA);
            break;
        }
        만약 (irqtimer_check(end)) {
            break;
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
    만약 (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    만약 (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    만약 (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

static void sercon_set_attr(u8 attr)
{
    만약 (attr == GET_LOW(sercon_attr_last))
        반환;

    SET_LOW(sercon_attr_last, attr);
    sercon_term_set_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

static void sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_to_unicode(chr);

    만약 (unicode < 0x7f) {
        sercon_putchar(unicode);
    } 아니면 만약 (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } 아니면 {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

static void sercon_cursor_pos_set(u8 row, u8 col)
{
    만약 (!sercon_splitmode()) {
        cursor_pos_set(row, col);
    } 아니면 {
        /* let vgabios update cursor */
    }
}

static void sercon_lazy_cursor_sync(void)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    만약 (GET_LOW(sercon_row_last) == row &&
        GET_LOW(sercon_col_last) == col)
        반환;

    만약 (col == 0 && GET_LOW(sercon_row_last) <= row) {
        만약 (GET_LOW(sercon_col_last) != 0) {
            sercon_putchar('\r');
            SET_LOW(sercon_col_last, 0);
        }
        동안 (GET_LOW(sercon_row_last) < row) {
            sercon_putchar('\n');
            SET_LOW(sercon_row_last, GET_LOW(sercon_row_last)+1);
        }
        만약 (GET_LOW(sercon_row_last) == row &&
            GET_LOW(sercon_col_last) == col)
            반환;
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
    만약 (chr) {
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
    만약 (col > 0) {
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
    만약 (row >= video_rows()) {
        /* scrolling up */
        row = video_rows()-1;
        만약 (GET_LOW(sercon_row_last) > 0) {
            SET_LOW(sercon_row_last, GET_LOW(sercon_row_last) - 1);
        }
    }
    sercon_cursor_pos_set(row, cursor_pos_col());
}

static void sercon_lazy_move_cursor(void)
{
    u8 col;

    col = cursor_pos_col() + 1;
    만약 (col >= video_cols()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } 아니면 {
        sercon_cursor_pos_set(cursor_pos_row(), col);
    }
}

static void sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    만약 (cursor_pos_row() != GET_LOW(sercon_row_last) ||
        cursor_pos_col() != GET_LOW(sercon_col_last)) {
        sercon_lazy_flush();
    }

    SET_LOW(sercon_char, chr);
    만약 (teletype)
        sercon_lazy_move_cursor();
    아니면
        SET_LOW(sercon_attr, attr);
}

/* Set video mode */
static void sercon_1000(struct bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 mode = regs->al & 0x7f;
    u8 rows, cols;

    만약 (!sercon_splitmode()) {
        switch (mode) {
        case 0x00:
        case 0x01:
        case 0x04: /* 320x200 */
        case 0x05: /* 320x200 */
            cols = 40;
            rows = 25;
            regs->al = 0x30;
            break;
        case 0x02:
        case 0x03:
        case 0x06: /* 640x200 */
        case 0x07:
        default:
            cols = 80;
            rows = 25;
            regs->al = 0x30;
            break;
        }
        cursor_pos_set(0, 0);
        SET_BDA(video_mode, mode);
        SET_BDA(video_cols, cols);
        SET_BDA(video_rows, rows-1);
        SET_BDA(cursor_type, 0x0007);
    } 아니면 {
        /* let vgabios handle mode init */
    }

    SET_LOW(sercon_enable, mode <= 0x07);
    SET_LOW(sercon_col_last, 0);
    SET_LOW(sercon_row_last, 0);
    SET_LOW(sercon_attr_last, 0);

    sercon_term_reset();
    sercon_term_no_linewrap();
    만약 (clearscreen)
        sercon_term_clear_screen();
}

/* Set text-mode cursor shape */
static void sercon_1001(struct bregs *regs)
{
    /* show/hide cursor? */
    SET_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
static void sercon_1002(struct bregs *regs)
{
    sercon_cursor_pos_set(regs->dh, regs->dl);
}

/* Get cursor position */
static void sercon_1003(struct bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_row();
    regs->dl = cursor_pos_col();
}

/* Scroll up window */
static void sercon_1006(struct bregs *regs)
{
    sercon_lazy_flush();
    만약 (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        만약 (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_rows()-1 &&
            regs->dl == video_cols()-1) {
            sercon_set_attr(regs->bh);
            sercon_term_clear_screen();
        }
    } 아니면 {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
static void sercon_1008(struct bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
static void sercon_1009(struct bregs *regs)
{
    u16 count = regs->cx;

    만약 (count == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } 아니면 만약 (regs->al == 0x20 &&
               video_rows() * video_cols() == count &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        sercon_term_clear_screen();

    } 아니면 {
        sercon_lazy_flush();
        sercon_set_attr(regs->bl);
        동안 (count) {
            sercon_print_utf8(regs->al);
            count--;
        }
        sercon_term_cursor_goto(cursor_pos_row(),
                                cursor_pos_col());
    }
}

/* Teletype output */
static void sercon_100e(struct bregs *regs)
{
    switch (regs->al) {
    case 7:
        sercon_putchar(0x07);
        break;
    case 8:
        sercon_lazy_backspace();
        break;
    case '\r':
        sercon_lazy_cr();
        break;
    case '\n':
        sercon_lazy_lf();
        break;
    default:
        sercon_lazy_putchar(regs->al, 0, 1);
        break;
    }
}

/* Get current video mode */
static void sercon_100f(struct bregs *regs)
{
    regs->al = GET_BDA(video_mode);
    regs->ah = GET_BDA(video_cols);
}

/* VBE 2.0 */
static void sercon_104f(struct bregs *regs)
{
    만약 (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } 아니면 {
        // Disable sercon entry point on any vesa modeset
        만약 (regs->al == 0x02)
            SET_LOW(sercon_enable, 0);
    }
}

static void sercon_10XX(struct bregs *regs)
{
    warn_unimplemented(regs);
}

void VISIBLE16
handle_sercon(struct bregs *regs)
{
    만약 (!CONFIG_SERCON)
        반환;
    만약 (!GET_LOW(sercon_port))
        반환;

    switch (regs->ah) {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x08:
    case 0x0f:
        만약 (sercon_splitmode())
            /* nothing, vgabios handles it */
            반환;
    }

    switch (regs->ah) {
    case 0x00: sercon_1000(regs); break;
    case 0x01: sercon_1001(regs); break;
    case 0x02: sercon_1002(regs); break;
    case 0x03: sercon_1003(regs); break;
    case 0x06: sercon_1006(regs); break;
    case 0x08: sercon_1008(regs); break;
    case 0x09: sercon_1009(regs); break;
    case 0x0e: sercon_100e(regs); break;
    case 0x0f: sercon_100f(regs); break;
    case 0x4f: sercon_104f(regs); break;
    default:   sercon_10XX(regs); break;
    }
}

void sercon_setup(void)
{
    만약 (!CONFIG_SERCON)
        반환;

    struct segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    만약 (!addr)
        반환;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    만약 (CONFIG_DEBUG_SERIAL)
        만약 (addr == CONFIG_DEBUG_SERIAL_PORT)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(entry_10);
    만약 (vgabios.seg != seabios.seg ||
        vgabios.offset != seabios.offset) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.offset);
        sercon_real_vga_handler = vgabios;
        SET_LOW(sercon_split, 1);
    } 아니면 {
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

static VAR16 struct {
    char seq[4];
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

static void shiftbuf(정수 remove)
{
    정수 i, remaining;

    remaining = GET_LOW(rx_bytes) - remove;
    SET_LOW(rx_bytes, remaining);
    위해 (i = 0; i < remaining; i++)
        SET_LOW(rx_buf[i], GET_LOW(rx_buf[i + remove]));
}

static 정수 cmpbuf(정수 seq)
{
    정수 chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    만약 (GET_LOW(rx_bytes) < len + 1)
        반환 0;
    위해 (chr = 0; chr < len; chr++)
        만약 (GET_GLOBAL(termseq[seq].seq[chr]) != GET_LOW(rx_buf[chr + 1]))
            반환 0;
    반환 1;
}

static 정수 findseq(void)
{
    정수 seq;

    위해 (seq = 0; seq < ARRAY_SIZE(termseq); seq++)
        만약 (cmpbuf(seq))
            반환 seq;
    반환 -1;
}

void
sercon_check_event(void)
{
    만약 (!CONFIG_SERCON)
        반환;

    u16 addr = GET_LOW(sercon_port);
    u16 keycode;
    u8 byte, count = 0;
    정수 seq;

    // check to see if there is a active serial port
    만약 (!addr)
        반환;
    만약 (inb(addr + SEROFF_LSR) == 0xFF)
        반환;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    동안 (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_DATA);
        u8 rb = GET_LOW(rx_bytes);
        만약 (rb < sizeof(rx_buf)) {
            SET_LOW(rx_buf[rb], byte);
            SET_LOW(rx_bytes, rb + 1);
            count++;
        }
    }

    위해 (;;) {
        // no (more) input data
        u8 rb = GET_LOW(rx_bytes);
        만약 (!rb)
            반환;

        // lookup escape sequences
        u8 next_char = GET_LOW(rx_buf[0]);
        만약 (rb > 1 && next_char == 0x1b) {
            seq = findseq();
            만약 (seq >= 0) {
                enqueue_key(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                continue;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        만약 (next_char == 0x1b && count)
            반환;

        // Handle input as individual char.
        keycode = ascii_to_keycode(next_char);
        만약 (keycode)
            enqueue_key(keycode);
        shiftbuf(1);
    }
}
