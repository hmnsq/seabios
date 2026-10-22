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

स्थिर u8 video_पङ्क्तिकाः(शून्य)
{
    प्रत्यागम GET_BDA(video_पङ्क्तिकाः)+1;
}

स्थिर u8 video_स्तम्भसङ्ख्या(शून्य)
{
    प्रत्यागम GET_BDA(video_स्तम्भसङ्ख्या);
}

स्थिर u8 cursor_pos_col(शून्य)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    प्रत्यागम pos & 0xff;
}

स्थिर u8 cursor_pos_row(शून्य)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    प्रत्यागम (pos >> 8) & 0xff;
}

स्थिर शून्य cursor_pos_समुच्चयः(u8 row, u8 col)
{
    u16 pos = ((u16)row << 8) | col;
    समुच्चयः_BDA(cursor_pos[0], pos);
}

/****************************************************************
 * serial console output
 ****************************************************************/

VARLOW u16 sercon_पत्तनम्;
VARLOW u8 sercon_विभज;
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
VARLOW u8 sercon_attr_चरमः;
VARLOW u8 sercon_col_चरमः;
VARLOW u8 sercon_row_चरमः;
VARLOW u8 sercon_char;
VARLOW u8 sercon_attr = 0x07;

स्थिर VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

स्थिर पूर्णाङ्क sercon_splitmode(शून्य)
{
    प्रत्यागम GET_नीचः(sercon_विभज);
}

स्थिर शून्य sercon_putchar(u8 chr)
{
    u16 addr = GET_नीचः(sercon_पत्तनम्);
    u32 अन्तः = irqtimer_calc_ticks(0x0a);

#यदि 0
    /* for visual control sequence debugging */
    यदि (chr == '\x1b')
        chr = '*';
#यदि_अन्तः

    प्रति (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        यदि ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_दत्त);
            भङ्ग;
        }
        यदि (irqtimer_check(अन्तः)) {
            भङ्ग;
        }
        yield();
    }
}

स्थिर शून्य sercon_पदांशः_reset(शून्य)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

स्थिर शून्य sercon_पदांशः_clear_screen(शून्य)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

स्थिर शून्य sercon_पदांशः_no_linewrap(शून्य)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

स्थिर शून्य sercon_पदांशः_cursor_goto(u8 row, u8 col)
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

स्थिर शून्य sercon_पदांशः_समुच्चयः_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    यदि (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    यदि (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    यदि (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

स्थिर शून्य sercon_समुच्चयः_attr(u8 attr)
{
    यदि (attr == GET_नीचः(sercon_attr_चरमः))
        प्रत्यागम;

    समुच्चयः_नीचः(sercon_attr_चरमः, attr);
    sercon_पदांशः_समुच्चयः_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

स्थिर शून्य sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_गन्तव्यम्_unicode(chr);

    यदि (unicode < 0x7f) {
        sercon_putchar(unicode);
    } अन्यथा यदि (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } अन्यथा {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

स्थिर शून्य sercon_cursor_pos_समुच्चयः(u8 row, u8 col)
{
    यदि (!sercon_splitmode()) {
        cursor_pos_समुच्चयः(row, col);
    } अन्यथा {
        /* let vgabios update cursor */
    }
}

स्थिर शून्य sercon_lazy_cursor_sync(शून्य)
{
    u8 row = cursor_pos_row();
    u8 col = cursor_pos_col();

    यदि (GET_नीचः(sercon_row_चरमः) == row &&
        GET_नीचः(sercon_col_चरमः) == col)
        प्रत्यागम;

    यदि (col == 0 && GET_नीचः(sercon_row_चरमः) <= row) {
        यदि (GET_नीचः(sercon_col_चरमः) != 0) {
            sercon_putchar('\r');
            समुच्चयः_नीचः(sercon_col_चरमः, 0);
        }
        यावत् (GET_नीचः(sercon_row_चरमः) < row) {
            sercon_putchar('\n');
            समुच्चयः_नीचः(sercon_row_चरमः, GET_नीचः(sercon_row_चरमः)+1);
        }
        यदि (GET_नीचः(sercon_row_चरमः) == row &&
            GET_नीचः(sercon_col_चरमः) == col)
            प्रत्यागम;
    }

    sercon_पदांशः_cursor_goto(row, col);
    समुच्चयः_नीचः(sercon_row_चरमः, row);
    समुच्चयः_नीचः(sercon_col_चरमः, col);
}

स्थिर शून्य sercon_lazy_flush(शून्य)
{
    u8 chr, attr;

    chr = GET_नीचः(sercon_char);
    attr = GET_नीचः(sercon_attr);
    यदि (chr) {
        sercon_समुच्चयः_attr(attr);
        sercon_print_utf8(chr);
        समुच्चयः_नीचः(sercon_col_चरमः, GET_नीचः(sercon_col_चरमः) + 1);
    }

    sercon_lazy_cursor_sync();

    समुच्चयः_नीचः(sercon_attr, 0x07);
    समुच्चयः_नीचः(sercon_char, 0x00);
}

स्थिर शून्य sercon_lazy_cursor_update(u8 row, u8 col)
{
    sercon_cursor_pos_समुच्चयः(row, col);
    समुच्चयः_नीचः(sercon_row_चरमः, row);
    समुच्चयः_नीचः(sercon_col_चरमः, col);
}

स्थिर शून्य sercon_lazy_backspace(शून्य)
{
    u8 col;

    sercon_lazy_flush();
    col = cursor_pos_col();
    यदि (col > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_row(), col-1);
    }
}

स्थिर शून्य sercon_lazy_cr(शून्य)
{
    sercon_cursor_pos_समुच्चयः(cursor_pos_row(), 0);
}

स्थिर शून्य sercon_lazy_lf(शून्य)
{
    u8 row;

    row = cursor_pos_row() + 1;
    यदि (row >= video_पङ्क्तिकाः()) {
        /* scrolling up */
        row = video_पङ्क्तिकाः()-1;
        यदि (GET_नीचः(sercon_row_चरमः) > 0) {
            समुच्चयः_नीचः(sercon_row_चरमः, GET_नीचः(sercon_row_चरमः) - 1);
        }
    }
    sercon_cursor_pos_समुच्चयः(row, cursor_pos_col());
}

स्थिर शून्य sercon_lazy_move_cursor(शून्य)
{
    u8 col;

    col = cursor_pos_col() + 1;
    यदि (col >= video_स्तम्भसङ्ख्या()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } अन्यथा {
        sercon_cursor_pos_समुच्चयः(cursor_pos_row(), col);
    }
}

स्थिर शून्य sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    यदि (cursor_pos_row() != GET_नीचः(sercon_row_चरमः) ||
        cursor_pos_col() != GET_नीचः(sercon_col_चरमः)) {
        sercon_lazy_flush();
    }

    समुच्चयः_नीचः(sercon_char, chr);
    यदि (teletype)
        sercon_lazy_move_cursor();
    अन्यथा
        समुच्चयः_नीचः(sercon_attr, attr);
}

/* Set video mode */
स्थिर शून्य sercon_1000(संरचना bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 बहुलकम् = regs->al & 0x7f;
    u8 पङ्क्तिकाः, स्तम्भसङ्ख्या;

    यदि (!sercon_splitmode()) {
        विकल्प (बहुलकम्) {
        पक्ष 0x00:
        पक्ष 0x01:
        पक्ष 0x04: /* 320x200 */
        पक्ष 0x05: /* 320x200 */
            स्तम्भसङ्ख्या = 40;
            पङ्क्तिकाः = 25;
            regs->al = 0x30;
            भङ्ग;
        पक्ष 0x02:
        पक्ष 0x03:
        पक्ष 0x06: /* 640x200 */
        पक्ष 0x07:
        स्वभाव:
            स्तम्भसङ्ख्या = 80;
            पङ्क्तिकाः = 25;
            regs->al = 0x30;
            भङ्ग;
        }
        cursor_pos_समुच्चयः(0, 0);
        समुच्चयः_BDA(video_बहुलकम्, बहुलकम्);
        समुच्चयः_BDA(video_स्तम्भसङ्ख्या, स्तम्भसङ्ख्या);
        समुच्चयः_BDA(video_पङ्क्तिकाः, पङ्क्तिकाः-1);
        समुच्चयः_BDA(cursor_type, 0x0007);
    } अन्यथा {
        /* let vgabios handle mode init */
    }

    समुच्चयः_नीचः(sercon_enable, बहुलकम् <= 0x07);
    समुच्चयः_नीचः(sercon_col_चरमः, 0);
    समुच्चयः_नीचः(sercon_row_चरमः, 0);
    समुच्चयः_नीचः(sercon_attr_चरमः, 0);

    sercon_पदांशः_reset();
    sercon_पदांशः_no_linewrap();
    यदि (clearscreen)
        sercon_पदांशः_clear_screen();
}

/* Set text-mode cursor shape */
स्थिर शून्य sercon_1001(संरचना bregs *regs)
{
    /* show/hide cursor? */
    समुच्चयः_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
स्थिर शून्य sercon_1002(संरचना bregs *regs)
{
    sercon_cursor_pos_समुच्चयः(regs->dh, regs->dl);
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
    यदि (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        यदि (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_पङ्क्तिकाः()-1 &&
            regs->dl == video_स्तम्भसङ्ख्या()-1) {
            sercon_समुच्चयः_attr(regs->bh);
            sercon_पदांशः_clear_screen();
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
    u16 गणनम् = regs->cx;

    यदि (गणनम् == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } अन्यथा यदि (regs->al == 0x20 &&
               video_पङ्क्तिकाः() * video_स्तम्भसङ्ख्या() == गणनम् &&
               cursor_pos_row() == 0 &&
               cursor_pos_col() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_समुच्चयः_attr(regs->bl);
        sercon_पदांशः_clear_screen();

    } अन्यथा {
        sercon_lazy_flush();
        sercon_समुच्चयः_attr(regs->bl);
        यावत् (गणनम्) {
            sercon_print_utf8(regs->al);
            गणनम्--;
        }
        sercon_पदांशः_cursor_goto(cursor_pos_row(),
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
    regs->al = GET_BDA(video_बहुलकम्);
    regs->ah = GET_BDA(video_स्तम्भसङ्ख्या);
}

/* VBE 2.0 */
स्थिर शून्य sercon_104f(संरचना bregs *regs)
{
    यदि (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } अन्यथा {
        // Disable sercon entry point on any vesa modeset
        यदि (regs->al == 0x02)
            समुच्चयः_नीचः(sercon_enable, 0);
    }
}

स्थिर शून्य sercon_10XX(संरचना bregs *regs)
{
    warn_unimplemented(regs);
}

शून्य VISIBLE16
handle_sercon(संरचना bregs *regs)
{
    यदि (!CONFIG_SERCON)
        प्रत्यागम;
    यदि (!GET_नीचः(sercon_पत्तनम्))
        प्रत्यागम;

    विकल्प (regs->ah) {
    पक्ष 0x01:
    पक्ष 0x02:
    पक्ष 0x03:
    पक्ष 0x08:
    पक्ष 0x0f:
        यदि (sercon_splitmode())
            /* nothing, vgabios handles it */
            प्रत्यागम;
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

शून्य sercon_सज्जता(शून्य)
{
    यदि (!CONFIG_SERCON)
        प्रत्यागम;

    संरचना segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    यदि (!addr)
        प्रत्यागम;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    यदि (CONFIG_DEBUG_SERIAL)
        यदि (addr == CONFIG_DEBUG_SERIAL_पत्तनम्)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(प्रविष्टिः_10);
    यदि (vgabios.seg != seabios.seg ||
        vgabios.विस्थापनम्_2 != seabios.विस्थापनम्_2) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.विस्थापनम्_2);
        sercon_real_vga_handler = vgabios;
        समुच्चयः_नीचः(sercon_विभज, 1);
    } अन्यथा {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    समुच्चयः_IVT(0x10, FUNC16(प्रविष्टिः_sercon));
    समुच्चयः_नीचः(sercon_पत्तनम्, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_अष्टकानि;

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

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    //ऊर्ध्वम्
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    //अधस्तात्
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //दक्षिणः
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //वामः

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    //अन्तः
};

स्थिर शून्य shiftbuf(पूर्णाङ्क remove)
{
    पूर्णाङ्क i, remaining;

    remaining = GET_नीचः(rx_अष्टकानि) - remove;
    समुच्चयः_नीचः(rx_अष्टकानि, remaining);
    प्रति (i = 0; i < remaining; i++)
        समुच्चयः_नीचः(rx_buf[i], GET_नीचः(rx_buf[i + remove]));
}

स्थिर पूर्णाङ्क cmpbuf(पूर्णाङ्क seq)
{
    पूर्णाङ्क chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    यदि (GET_नीचः(rx_अष्टकानि) < len + 1)
        प्रत्यागम 0;
    प्रति (chr = 0; chr < len; chr++)
        यदि (GET_GLOBAL(termseq[seq].seq[chr]) != GET_नीचः(rx_buf[chr + 1]))
            प्रत्यागम 0;
    प्रत्यागम 1;
}

स्थिर पूर्णाङ्क findseq(शून्य)
{
    पूर्णाङ्क seq;

    प्रति (seq = 0; seq < ARRAY_आकार(termseq); seq++)
        यदि (cmpbuf(seq))
            प्रत्यागम seq;
    प्रत्यागम -1;
}

शून्य
sercon_check_event(शून्य)
{
    यदि (!CONFIG_SERCON)
        प्रत्यागम;

    u16 addr = GET_नीचः(sercon_पत्तनम्);
    u16 keycode;
    u8 byte, गणनम् = 0;
    पूर्णाङ्क seq;

    // check to see if there is a active serial port
    यदि (!addr)
        प्रत्यागम;
    यदि (inb(addr + SEROFF_LSR) == 0xFF)
        प्रत्यागम;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    यावत् (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_दत्त);
        u8 rb = GET_नीचः(rx_अष्टकानि);
        यदि (rb < परिमाण(rx_buf)) {
            समुच्चयः_नीचः(rx_buf[rb], byte);
            समुच्चयः_नीचः(rx_अष्टकानि, rb + 1);
            गणनम्++;
        }
    }

    प्रति (;;) {
        // no (more) input data
        u8 rb = GET_नीचः(rx_अष्टकानि);
        यदि (!rb)
            प्रत्यागम;

        // lookup escape sequences
        u8 अग्र_char = GET_नीचः(rx_buf[0]);
        यदि (rb > 1 && अग्र_char == 0x1b) {
            seq = findseq();
            यदि (seq >= 0) {
                पङ्क्तौ_निक्षिप_कुञ्चिका(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                अनुवर्त;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        यदि (अग्र_char == 0x1b && गणनम्)
            प्रत्यागम;

        // Handle input as individual char.
        keycode = ascii_गन्तव्यम्_keycode(अग्र_char);
        यदि (keycode)
            पङ्क्तौ_निक्षिप_कुञ्चिका(keycode);
        shiftbuf(1);
    }
}
