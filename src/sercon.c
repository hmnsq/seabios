// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // SET_BDA
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "util.h" // irqtimer_calc_ticks
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "hw/serialio.h" // SEROFF_IER
#అన్తర్భావ "cp437.h"

స్థిర u8 video_అడ్డువరుసలు(శూన్య)
{
    తిరిగి GET_BDA(video_అడ్డువరుసలు)+1;
}

స్థిర u8 video_నిలువుల_సంఖ్య(శూన్య)
{
    తిరిగి GET_BDA(video_నిలువుల_సంఖ్య);
}

స్థిర u8 cursor_pos_నిలువు(శూన్య)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    తిరిగి pos & 0xff;
}

స్థిర u8 cursor_pos_అడ్డువరుస(శూన్య)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    తిరిగి (pos >> 8) & 0xff;
}

స్థిర శూన్య cursor_pos_సమితి(u8 అడ్డువరుస, u8 నిలువు)
{
    u16 pos = ((u16)అడ్డువరుస << 8) | నిలువు;
    సమితి_BDA(cursor_pos[0], pos);
}

/****************************************************************
 * serial console output
 ****************************************************************/

VARLOW u16 sercon_పోర్టు;
VARLOW u8 sercon_విభజించు;
VARLOW u8 sercon_enable;
VARFSEG నిర్మాణం segoff_s sercon_real_vga_handler;

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
VARLOW u8 sercon_attr_చివరి;
VARLOW u8 sercon_నిలువు_చివరి;
VARLOW u8 sercon_అడ్డువరుస_చివరి;
VARLOW u8 sercon_char;
VARLOW u8 sercon_attr = 0x07;

స్థిర VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

స్థిర పూర్ణాంకం sercon_splitmode(శూన్య)
{
    తిరిగి GET_దిగువ(sercon_విభజించు);
}

స్థిర శూన్య sercon_putchar(u8 chr)
{
    u16 addr = GET_దిగువ(sercon_పోర్టు);
    u32 చివర = irqtimer_calc_ticks(0x0a);

#ఒకవేళ 0
    /* for visual control sequence debugging */
    ఒకవేళ (chr == '\x1b')
        chr = '*';
#అయితే_ముగింపు

    కొరకు (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ఒకవేళ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_దత్తాంశం);
            ఆపు;
        }
        ఒకవేళ (irqtimer_check(చివర)) {
            ఆపు;
        }
        yield();
    }
}

స్థిర శూన్య sercon_పదం_reset(శూన్య)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

స్థిర శూన్య sercon_పదం_clear_screen(శూన్య)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

స్థిర శూన్య sercon_పదం_no_linewrap(శూన్య)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

స్థిర శూన్య sercon_పదం_cursor_goto(u8 అడ్డువరుస, u8 నిలువు)
{
    అడ్డువరుస++; నిలువు++;
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0' + అడ్డువరుస / 10);
    sercon_putchar('0' + అడ్డువరుస % 10);
    sercon_putchar(';');
    sercon_putchar('0' + నిలువు / 10);
    sercon_putchar('0' + నిలువు % 10);
    sercon_putchar('H');
}

స్థిర శూన్య sercon_పదం_సమితి_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    ఒకవేళ (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    ఒకవేళ (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    ఒకవేళ (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

స్థిర శూన్య sercon_సమితి_attr(u8 attr)
{
    ఒకవేళ (attr == GET_దిగువ(sercon_attr_చివరి))
        తిరిగి;

    సమితి_దిగువ(sercon_attr_చివరి, attr);
    sercon_పదం_సమితి_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

స్థిర శూన్య sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_గమ్యం_unicode(chr);

    ఒకవేళ (unicode < 0x7f) {
        sercon_putchar(unicode);
    } లేకపోతే ఒకవేళ (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } లేకపోతే {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

స్థిర శూన్య sercon_cursor_pos_సమితి(u8 అడ్డువరుస, u8 నిలువు)
{
    ఒకవేళ (!sercon_splitmode()) {
        cursor_pos_సమితి(అడ్డువరుస, నిలువు);
    } లేకపోతే {
        /* let vgabios update cursor */
    }
}

స్థిర శూన్య sercon_lazy_cursor_sync(శూన్య)
{
    u8 అడ్డువరుస = cursor_pos_అడ్డువరుస();
    u8 నిలువు = cursor_pos_నిలువు();

    ఒకవేళ (GET_దిగువ(sercon_అడ్డువరుస_చివరి) == అడ్డువరుస &&
        GET_దిగువ(sercon_నిలువు_చివరి) == నిలువు)
        తిరిగి;

    ఒకవేళ (నిలువు == 0 && GET_దిగువ(sercon_అడ్డువరుస_చివరి) <= అడ్డువరుస) {
        ఒకవేళ (GET_దిగువ(sercon_నిలువు_చివరి) != 0) {
            sercon_putchar('\r');
            సమితి_దిగువ(sercon_నిలువు_చివరి, 0);
        }
        అయితే (GET_దిగువ(sercon_అడ్డువరుస_చివరి) < అడ్డువరుస) {
            sercon_putchar('\n');
            సమితి_దిగువ(sercon_అడ్డువరుస_చివరి, GET_దిగువ(sercon_అడ్డువరుస_చివరి)+1);
        }
        ఒకవేళ (GET_దిగువ(sercon_అడ్డువరుస_చివరి) == అడ్డువరుస &&
            GET_దిగువ(sercon_నిలువు_చివరి) == నిలువు)
            తిరిగి;
    }

    sercon_పదం_cursor_goto(అడ్డువరుస, నిలువు);
    సమితి_దిగువ(sercon_అడ్డువరుస_చివరి, అడ్డువరుస);
    సమితి_దిగువ(sercon_నిలువు_చివరి, నిలువు);
}

స్థిర శూన్య sercon_lazy_flush(శూన్య)
{
    u8 chr, attr;

    chr = GET_దిగువ(sercon_char);
    attr = GET_దిగువ(sercon_attr);
    ఒకవేళ (chr) {
        sercon_సమితి_attr(attr);
        sercon_print_utf8(chr);
        సమితి_దిగువ(sercon_నిలువు_చివరి, GET_దిగువ(sercon_నిలువు_చివరి) + 1);
    }

    sercon_lazy_cursor_sync();

    సమితి_దిగువ(sercon_attr, 0x07);
    సమితి_దిగువ(sercon_char, 0x00);
}

స్థిర శూన్య sercon_lazy_cursor_update(u8 అడ్డువరుస, u8 నిలువు)
{
    sercon_cursor_pos_సమితి(అడ్డువరుస, నిలువు);
    సమితి_దిగువ(sercon_అడ్డువరుస_చివరి, అడ్డువరుస);
    సమితి_దిగువ(sercon_నిలువు_చివరి, నిలువు);
}

స్థిర శూన్య sercon_lazy_backspace(శూన్య)
{
    u8 నిలువు;

    sercon_lazy_flush();
    నిలువు = cursor_pos_నిలువు();
    ఒకవేళ (నిలువు > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_అడ్డువరుస(), నిలువు-1);
    }
}

స్థిర శూన్య sercon_lazy_cr(శూన్య)
{
    sercon_cursor_pos_సమితి(cursor_pos_అడ్డువరుస(), 0);
}

స్థిర శూన్య sercon_lazy_lf(శూన్య)
{
    u8 అడ్డువరుస;

    అడ్డువరుస = cursor_pos_అడ్డువరుస() + 1;
    ఒకవేళ (అడ్డువరుస >= video_అడ్డువరుసలు()) {
        /* scrolling up */
        అడ్డువరుస = video_అడ్డువరుసలు()-1;
        ఒకవేళ (GET_దిగువ(sercon_అడ్డువరుస_చివరి) > 0) {
            సమితి_దిగువ(sercon_అడ్డువరుస_చివరి, GET_దిగువ(sercon_అడ్డువరుస_చివరి) - 1);
        }
    }
    sercon_cursor_pos_సమితి(అడ్డువరుస, cursor_pos_నిలువు());
}

స్థిర శూన్య sercon_lazy_move_cursor(శూన్య)
{
    u8 నిలువు;

    నిలువు = cursor_pos_నిలువు() + 1;
    ఒకవేళ (నిలువు >= video_నిలువుల_సంఖ్య()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } లేకపోతే {
        sercon_cursor_pos_సమితి(cursor_pos_అడ్డువరుస(), నిలువు);
    }
}

స్థిర శూన్య sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    ఒకవేళ (cursor_pos_అడ్డువరుస() != GET_దిగువ(sercon_అడ్డువరుస_చివరి) ||
        cursor_pos_నిలువు() != GET_దిగువ(sercon_నిలువు_చివరి)) {
        sercon_lazy_flush();
    }

    సమితి_దిగువ(sercon_char, chr);
    ఒకవేళ (teletype)
        sercon_lazy_move_cursor();
    లేకపోతే
        సమితి_దిగువ(sercon_attr, attr);
}

/* Set video mode */
స్థిర శూన్య sercon_1000(నిర్మాణం bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 బహుళకం = regs->al & 0x7f;
    u8 అడ్డువరుసలు, నిలువుల_సంఖ్య;

    ఒకవేళ (!sercon_splitmode()) {
        ఎంపిక (బహుళకం) {
        స్థితి 0x00:
        స్థితి 0x01:
        స్థితి 0x04: /* 320x200 */
        స్థితి 0x05: /* 320x200 */
            నిలువుల_సంఖ్య = 40;
            అడ్డువరుసలు = 25;
            regs->al = 0x30;
            ఆపు;
        స్థితి 0x02:
        స్థితి 0x03:
        స్థితి 0x06: /* 640x200 */
        స్థితి 0x07:
        అప్రమేయ:
            నిలువుల_సంఖ్య = 80;
            అడ్డువరుసలు = 25;
            regs->al = 0x30;
            ఆపు;
        }
        cursor_pos_సమితి(0, 0);
        సమితి_BDA(video_బహుళకం, బహుళకం);
        సమితి_BDA(video_నిలువుల_సంఖ్య, నిలువుల_సంఖ్య);
        సమితి_BDA(video_అడ్డువరుసలు, అడ్డువరుసలు-1);
        సమితి_BDA(cursor_type, 0x0007);
    } లేకపోతే {
        /* let vgabios handle mode init */
    }

    సమితి_దిగువ(sercon_enable, బహుళకం <= 0x07);
    సమితి_దిగువ(sercon_నిలువు_చివరి, 0);
    సమితి_దిగువ(sercon_అడ్డువరుస_చివరి, 0);
    సమితి_దిగువ(sercon_attr_చివరి, 0);

    sercon_పదం_reset();
    sercon_పదం_no_linewrap();
    ఒకవేళ (clearscreen)
        sercon_పదం_clear_screen();
}

/* Set text-mode cursor shape */
స్థిర శూన్య sercon_1001(నిర్మాణం bregs *regs)
{
    /* show/hide cursor? */
    సమితి_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
స్థిర శూన్య sercon_1002(నిర్మాణం bregs *regs)
{
    sercon_cursor_pos_సమితి(regs->dh, regs->dl);
}

/* Get cursor position */
స్థిర శూన్య sercon_1003(నిర్మాణం bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_అడ్డువరుస();
    regs->dl = cursor_pos_నిలువు();
}

/* Scroll up window */
స్థిర శూన్య sercon_1006(నిర్మాణం bregs *regs)
{
    sercon_lazy_flush();
    ఒకవేళ (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        ఒకవేళ (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_అడ్డువరుసలు()-1 &&
            regs->dl == video_నిలువుల_సంఖ్య()-1) {
            sercon_సమితి_attr(regs->bh);
            sercon_పదం_clear_screen();
        }
    } లేకపోతే {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
స్థిర శూన్య sercon_1008(నిర్మాణం bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
స్థిర శూన్య sercon_1009(నిర్మాణం bregs *regs)
{
    u16 లెక్క = regs->cx;

    ఒకవేళ (లెక్క == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } లేకపోతే ఒకవేళ (regs->al == 0x20 &&
               video_అడ్డువరుసలు() * video_నిలువుల_సంఖ్య() == లెక్క &&
               cursor_pos_అడ్డువరుస() == 0 &&
               cursor_pos_నిలువు() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_సమితి_attr(regs->bl);
        sercon_పదం_clear_screen();

    } లేకపోతే {
        sercon_lazy_flush();
        sercon_సమితి_attr(regs->bl);
        అయితే (లెక్క) {
            sercon_print_utf8(regs->al);
            లెక్క--;
        }
        sercon_పదం_cursor_goto(cursor_pos_అడ్డువరుస(),
                                cursor_pos_నిలువు());
    }
}

/* Teletype output */
స్థిర శూన్య sercon_100e(నిర్మాణం bregs *regs)
{
    ఎంపిక (regs->al) {
    స్థితి 7:
        sercon_putchar(0x07);
        ఆపు;
    స్థితి 8:
        sercon_lazy_backspace();
        ఆపు;
    స్థితి '\r':
        sercon_lazy_cr();
        ఆపు;
    స్థితి '\n':
        sercon_lazy_lf();
        ఆపు;
    అప్రమేయ:
        sercon_lazy_putchar(regs->al, 0, 1);
        ఆపు;
    }
}

/* Get current video mode */
స్థిర శూన్య sercon_100f(నిర్మాణం bregs *regs)
{
    regs->al = GET_BDA(video_బహుళకం);
    regs->ah = GET_BDA(video_నిలువుల_సంఖ్య);
}

/* VBE 2.0 */
స్థిర శూన్య sercon_104f(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } లేకపోతే {
        // Disable sercon entry point on any vesa modeset
        ఒకవేళ (regs->al == 0x02)
            సమితి_దిగువ(sercon_enable, 0);
    }
}

స్థిర శూన్య sercon_10XX(నిర్మాణం bregs *regs)
{
    warn_unimplemented(regs);
}

శూన్య VISIBLE16
handle_sercon(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!CONFIG_SERCON)
        తిరిగి;
    ఒకవేళ (!GET_దిగువ(sercon_పోర్టు))
        తిరిగి;

    ఎంపిక (regs->ah) {
    స్థితి 0x01:
    స్థితి 0x02:
    స్థితి 0x03:
    స్థితి 0x08:
    స్థితి 0x0f:
        ఒకవేళ (sercon_splitmode())
            /* nothing, vgabios handles it */
            తిరిగి;
    }

    ఎంపిక (regs->ah) {
    స్థితి 0x00: sercon_1000(regs); ఆపు;
    స్థితి 0x01: sercon_1001(regs); ఆపు;
    స్థితి 0x02: sercon_1002(regs); ఆపు;
    స్థితి 0x03: sercon_1003(regs); ఆపు;
    స్థితి 0x06: sercon_1006(regs); ఆపు;
    స్థితి 0x08: sercon_1008(regs); ఆపు;
    స్థితి 0x09: sercon_1009(regs); ఆపు;
    స్థితి 0x0e: sercon_100e(regs); ఆపు;
    స్థితి 0x0f: sercon_100f(regs); ఆపు;
    స్థితి 0x4f: sercon_104f(regs); ఆపు;
    అప్రమేయ:   sercon_10XX(regs); ఆపు;
    }
}

శూన్య sercon_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_SERCON)
        తిరిగి;

    నిర్మాణం segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    ఒకవేళ (!addr)
        తిరిగి;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    ఒకవేళ (CONFIG_DEBUG_SERIAL)
        ఒకవేళ (addr == CONFIG_DEBUG_SERIAL_పోర్టు)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(నమోదు_10);
    ఒకవేళ (vgabios.seg != seabios.seg ||
        vgabios.స్థానభ్రంశం_2 != seabios.స్థానభ్రంశం_2) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.స్థానభ్రంశం_2);
        sercon_real_vga_handler = vgabios;
        సమితి_దిగువ(sercon_విభజించు, 1);
    } లేకపోతే {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    సమితి_IVT(0x10, FUNC16(నమోదు_sercon));
    సమితి_దిగువ(sercon_పోర్టు, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_బైట్లు;

స్థిర VAR16 నిర్మాణం {
    అక్షరం seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //చొప్పించు
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    //పైకి
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    //కిందికి
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //కుడి
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //ఎడమ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    //చివర
};

స్థిర శూన్య shiftbuf(పూర్ణాంకం remove)
{
    పూర్ణాంకం i, మిగిలిన;

    మిగిలిన = GET_దిగువ(rx_బైట్లు) - remove;
    సమితి_దిగువ(rx_బైట్లు, మిగిలిన);
    కొరకు (i = 0; i < మిగిలిన; i++)
        సమితి_దిగువ(rx_buf[i], GET_దిగువ(rx_buf[i + remove]));
}

స్థిర పూర్ణాంకం cmpbuf(పూర్ణాంకం seq)
{
    పూర్ణాంకం chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    ఒకవేళ (GET_దిగువ(rx_బైట్లు) < len + 1)
        తిరిగి 0;
    కొరకు (chr = 0; chr < len; chr++)
        ఒకవేళ (GET_GLOBAL(termseq[seq].seq[chr]) != GET_దిగువ(rx_buf[chr + 1]))
            తిరిగి 0;
    తిరిగి 1;
}

స్థిర పూర్ణాంకం findseq(శూన్య)
{
    పూర్ణాంకం seq;

    కొరకు (seq = 0; seq < ARRAY_కొలత(termseq); seq++)
        ఒకవేళ (cmpbuf(seq))
            తిరిగి seq;
    తిరిగి -1;
}

శూన్య
sercon_check_event(శూన్య)
{
    ఒకవేళ (!CONFIG_SERCON)
        తిరిగి;

    u16 addr = GET_దిగువ(sercon_పోర్టు);
    u16 keycode;
    u8 byte, లెక్క = 0;
    పూర్ణాంకం seq;

    // check to see if there is a active serial port
    ఒకవేళ (!addr)
        తిరిగి;
    ఒకవేళ (inb(addr + SEROFF_LSR) == 0xFF)
        తిరిగి;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    అయితే (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_దత్తాంశం);
        u8 rb = GET_దిగువ(rx_బైట్లు);
        ఒకవేళ (rb < పరిమాణం(rx_buf)) {
            సమితి_దిగువ(rx_buf[rb], byte);
            సమితి_దిగువ(rx_బైట్లు, rb + 1);
            లెక్క++;
        }
    }

    కొరకు (;;) {
        // no (more) input data
        u8 rb = GET_దిగువ(rx_బైట్లు);
        ఒకవేళ (!rb)
            తిరిగి;

        // lookup escape sequences
        u8 తదుపరి_char = GET_దిగువ(rx_buf[0]);
        ఒకవేళ (rb > 1 && తదుపరి_char == 0x1b) {
            seq = findseq();
            ఒకవేళ (seq >= 0) {
                వరుసలో_చేర్చు_కీలకం(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                కొనసాగు;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        ఒకవేళ (తదుపరి_char == 0x1b && లెక్క)
            తిరిగి;

        // Handle input as individual char.
        keycode = ascii_గమ్యం_keycode(తదుపరి_char);
        ఒకవేళ (keycode)
            వరుసలో_చేర్చు_కీలకం(keycode);
        shiftbuf(1);
    }
}
