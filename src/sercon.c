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

ಸ್ಥಿರ u8 video_ಅಡ್ಡಸಾಲುಗಳು(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ GET_BDA(video_ಅಡ್ಡಸಾಲುಗಳು)+1;
}

ಸ್ಥಿರ u8 video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ GET_BDA(video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
}

ಸ್ಥಿರ u8 cursor_pos_ಲಂಬಸಾಲು(ಶೂನ್ಯ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ಹಿಂತಿರುಗಿ pos & 0xff;
}

ಸ್ಥಿರ u8 cursor_pos_ಅಡ್ಡಸಾಲು(ಶೂನ್ಯ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ಹಿಂತಿರುಗಿ (pos >> 8) & 0xff;
}

ಸ್ಥಿರ ಶೂನ್ಯ cursor_pos_ಗಣ(u8 ಅಡ್ಡಸಾಲು, u8 ಲಂಬಸಾಲು)
{
    u16 pos = ((u16)ಅಡ್ಡಸಾಲು << 8) | ಲಂಬಸಾಲು;
    ಗಣ_BDA(cursor_pos[0], pos);
}

/****************************************************************
 * serial console output
 ****************************************************************/

VARLOW u16 sercon_ಪೋರ್ಟ್;
VARLOW u8 sercon_ವಿಭಜಿಸು;
VARLOW u8 sercon_enable;
VARFSEG ರಚನೆ segoff_s sercon_real_vga_handler;

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
VARLOW u8 sercon_attr_ಕೊನೆಯ;
VARLOW u8 sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ;
VARLOW u8 sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ;
VARLOW u8 sercon_char;
VARLOW u8 sercon_attr = 0x07;

ಸ್ಥಿರ VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sercon_splitmode(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ GET_ಕೆಳಗಿನ(sercon_ವಿಭಜಿಸು);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_putchar(u8 chr)
{
    u16 addr = GET_ಕೆಳಗಿನ(sercon_ಪೋರ್ಟ್);
    u32 ಕೊನೆ = irqtimer_calc_ticks(0x0a);

#ಆದರೆ 0
    /* for visual control sequence debugging */
    ಆದರೆ (chr == '\x1b')
        chr = '*';
#ಆದರೆ_ಅಂತ್ಯ

    ಗಾಗಿ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ಆದರೆ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_ದತ್ತಾಂಶ);
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (irqtimer_check(ಕೊನೆ)) {
            ನಿಲ್ಲಿಸು;
        }
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_ಪದ_reset(ಶೂನ್ಯ)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_ಪದ_clear_screen(ಶೂನ್ಯ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_ಪದ_no_linewrap(ಶೂನ್ಯ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_ಪದ_cursor_goto(u8 ಅಡ್ಡಸಾಲು, u8 ಲಂಬಸಾಲು)
{
    ಅಡ್ಡಸಾಲು++; ಲಂಬಸಾಲು++;
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0' + ಅಡ್ಡಸಾಲು / 10);
    sercon_putchar('0' + ಅಡ್ಡಸಾಲು % 10);
    sercon_putchar(';');
    sercon_putchar('0' + ಲಂಬಸಾಲು / 10);
    sercon_putchar('0' + ಲಂಬಸಾಲು % 10);
    sercon_putchar('H');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_ಪದ_ಗಣ_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    ಆದರೆ (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    ಆದರೆ (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    ಆದರೆ (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_ಗಣ_attr(u8 attr)
{
    ಆದರೆ (attr == GET_ಕೆಳಗಿನ(sercon_attr_ಕೊನೆಯ))
        ಹಿಂತಿರುಗಿ;

    ಗಣ_ಕೆಳಗಿನ(sercon_attr_ಕೊನೆಯ, attr);
    sercon_ಪದ_ಗಣ_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_ಗಮ್ಯ_unicode(chr);

    ಆದರೆ (unicode < 0x7f) {
        sercon_putchar(unicode);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } ಇಲ್ಲದಿದ್ದರೆ {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_cursor_pos_ಗಣ(u8 ಅಡ್ಡಸಾಲು, u8 ಲಂಬಸಾಲು)
{
    ಆದರೆ (!sercon_splitmode()) {
        cursor_pos_ಗಣ(ಅಡ್ಡಸಾಲು, ಲಂಬಸಾಲು);
    } ಇಲ್ಲದಿದ್ದರೆ {
        /* let vgabios update cursor */
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_cursor_sync(ಶೂನ್ಯ)
{
    u8 ಅಡ್ಡಸಾಲು = cursor_pos_ಅಡ್ಡಸಾಲು();
    u8 ಲಂಬಸಾಲು = cursor_pos_ಲಂಬಸಾಲು();

    ಆದರೆ (GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ) == ಅಡ್ಡಸಾಲು &&
        GET_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ) == ಲಂಬಸಾಲು)
        ಹಿಂತಿರುಗಿ;

    ಆದರೆ (ಲಂಬಸಾಲು == 0 && GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ) <= ಅಡ್ಡಸಾಲು) {
        ಆದರೆ (GET_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ) != 0) {
            sercon_putchar('\r');
            ಗಣ_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ, 0);
        }
        ಆಗ (GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ) < ಅಡ್ಡಸಾಲು) {
            sercon_putchar('\n');
            ಗಣ_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ, GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ)+1);
        }
        ಆದರೆ (GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ) == ಅಡ್ಡಸಾಲು &&
            GET_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ) == ಲಂಬಸಾಲು)
            ಹಿಂತಿರುಗಿ;
    }

    sercon_ಪದ_cursor_goto(ಅಡ್ಡಸಾಲು, ಲಂಬಸಾಲು);
    ಗಣ_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ, ಅಡ್ಡಸಾಲು);
    ಗಣ_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ, ಲಂಬಸಾಲು);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_flush(ಶೂನ್ಯ)
{
    u8 chr, attr;

    chr = GET_ಕೆಳಗಿನ(sercon_char);
    attr = GET_ಕೆಳಗಿನ(sercon_attr);
    ಆದರೆ (chr) {
        sercon_ಗಣ_attr(attr);
        sercon_print_utf8(chr);
        ಗಣ_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ, GET_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ) + 1);
    }

    sercon_lazy_cursor_sync();

    ಗಣ_ಕೆಳಗಿನ(sercon_attr, 0x07);
    ಗಣ_ಕೆಳಗಿನ(sercon_char, 0x00);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_cursor_update(u8 ಅಡ್ಡಸಾಲು, u8 ಲಂಬಸಾಲು)
{
    sercon_cursor_pos_ಗಣ(ಅಡ್ಡಸಾಲು, ಲಂಬಸಾಲು);
    ಗಣ_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ, ಅಡ್ಡಸಾಲು);
    ಗಣ_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ, ಲಂಬಸಾಲು);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_backspace(ಶೂನ್ಯ)
{
    u8 ಲಂಬಸಾಲು;

    sercon_lazy_flush();
    ಲಂಬಸಾಲು = cursor_pos_ಲಂಬಸಾಲು();
    ಆದರೆ (ಲಂಬಸಾಲು > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_ಅಡ್ಡಸಾಲು(), ಲಂಬಸಾಲು-1);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_cr(ಶೂನ್ಯ)
{
    sercon_cursor_pos_ಗಣ(cursor_pos_ಅಡ್ಡಸಾಲು(), 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_lf(ಶೂನ್ಯ)
{
    u8 ಅಡ್ಡಸಾಲು;

    ಅಡ್ಡಸಾಲು = cursor_pos_ಅಡ್ಡಸಾಲು() + 1;
    ಆದರೆ (ಅಡ್ಡಸಾಲು >= video_ಅಡ್ಡಸಾಲುಗಳು()) {
        /* scrolling up */
        ಅಡ್ಡಸಾಲು = video_ಅಡ್ಡಸಾಲುಗಳು()-1;
        ಆದರೆ (GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ) > 0) {
            ಗಣ_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ, GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ) - 1);
        }
    }
    sercon_cursor_pos_ಗಣ(ಅಡ್ಡಸಾಲು, cursor_pos_ಲಂಬಸಾಲು());
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_move_cursor(ಶೂನ್ಯ)
{
    u8 ಲಂಬಸಾಲು;

    ಲಂಬಸಾಲು = cursor_pos_ಲಂಬಸಾಲು() + 1;
    ಆದರೆ (ಲಂಬಸಾಲು >= video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } ಇಲ್ಲದಿದ್ದರೆ {
        sercon_cursor_pos_ಗಣ(cursor_pos_ಅಡ್ಡಸಾಲು(), ಲಂಬಸಾಲು);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    ಆದರೆ (cursor_pos_ಅಡ್ಡಸಾಲು() != GET_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ) ||
        cursor_pos_ಲಂಬಸಾಲು() != GET_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ)) {
        sercon_lazy_flush();
    }

    ಗಣ_ಕೆಳಗಿನ(sercon_char, chr);
    ಆದರೆ (teletype)
        sercon_lazy_move_cursor();
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಣ_ಕೆಳಗಿನ(sercon_attr, attr);
}

/* Set video mode */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1000(ರಚನೆ bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 ಬಹುಲಕ = regs->al & 0x7f;
    u8 ಅಡ್ಡಸಾಲುಗಳು, ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;

    ಆದರೆ (!sercon_splitmode()) {
        ಆಯ್ಕೆ (ಬಹುಲಕ) {
        ಸ್ಥಿತಿ 0x00:
        ಸ್ಥಿತಿ 0x01:
        ಸ್ಥಿತಿ 0x04: /* 320x200 */
        ಸ್ಥಿತಿ 0x05: /* 320x200 */
            ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = 40;
            ಅಡ್ಡಸಾಲುಗಳು = 25;
            regs->al = 0x30;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x02:
        ಸ್ಥಿತಿ 0x03:
        ಸ್ಥಿತಿ 0x06: /* 640x200 */
        ಸ್ಥಿತಿ 0x07:
        ಪೂರ್ವನಿಯೋಜಿತ:
            ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = 80;
            ಅಡ್ಡಸಾಲುಗಳು = 25;
            regs->al = 0x30;
            ನಿಲ್ಲಿಸು;
        }
        cursor_pos_ಗಣ(0, 0);
        ಗಣ_BDA(video_ಬಹುಲಕ, ಬಹುಲಕ);
        ಗಣ_BDA(video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ, ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
        ಗಣ_BDA(video_ಅಡ್ಡಸಾಲುಗಳು, ಅಡ್ಡಸಾಲುಗಳು-1);
        ಗಣ_BDA(cursor_type, 0x0007);
    } ಇಲ್ಲದಿದ್ದರೆ {
        /* let vgabios handle mode init */
    }

    ಗಣ_ಕೆಳಗಿನ(sercon_enable, ಬಹುಲಕ <= 0x07);
    ಗಣ_ಕೆಳಗಿನ(sercon_ಲಂಬಸಾಲು_ಕೊನೆಯ, 0);
    ಗಣ_ಕೆಳಗಿನ(sercon_ಅಡ್ಡಸಾಲು_ಕೊನೆಯ, 0);
    ಗಣ_ಕೆಳಗಿನ(sercon_attr_ಕೊನೆಯ, 0);

    sercon_ಪದ_reset();
    sercon_ಪದ_no_linewrap();
    ಆದರೆ (clearscreen)
        sercon_ಪದ_clear_screen();
}

/* Set text-mode cursor shape */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1001(ರಚನೆ bregs *regs)
{
    /* show/hide cursor? */
    ಗಣ_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1002(ರಚನೆ bregs *regs)
{
    sercon_cursor_pos_ಗಣ(regs->dh, regs->dl);
}

/* Get cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1003(ರಚನೆ bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_ಅಡ್ಡಸಾಲು();
    regs->dl = cursor_pos_ಲಂಬಸಾಲು();
}

/* Scroll up window */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1006(ರಚನೆ bregs *regs)
{
    sercon_lazy_flush();
    ಆದರೆ (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        ಆದರೆ (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_ಅಡ್ಡಸಾಲುಗಳು()-1 &&
            regs->dl == video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ()-1) {
            sercon_ಗಣ_attr(regs->bh);
            sercon_ಪದ_clear_screen();
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1008(ರಚನೆ bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_1009(ರಚನೆ bregs *regs)
{
    u16 ಎಣಿಕೆ_2 = regs->cx;

    ಆದರೆ (ಎಣಿಕೆ_2 == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (regs->al == 0x20 &&
               video_ಅಡ್ಡಸಾಲುಗಳು() * video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ() == ಎಣಿಕೆ_2 &&
               cursor_pos_ಅಡ್ಡಸಾಲು() == 0 &&
               cursor_pos_ಲಂಬಸಾಲು() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_ಗಣ_attr(regs->bl);
        sercon_ಪದ_clear_screen();

    } ಇಲ್ಲದಿದ್ದರೆ {
        sercon_lazy_flush();
        sercon_ಗಣ_attr(regs->bl);
        ಆಗ (ಎಣಿಕೆ_2) {
            sercon_print_utf8(regs->al);
            ಎಣಿಕೆ_2--;
        }
        sercon_ಪದ_cursor_goto(cursor_pos_ಅಡ್ಡಸಾಲು(),
                                cursor_pos_ಲಂಬಸಾಲು());
    }
}

/* Teletype output */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_100e(ರಚನೆ bregs *regs)
{
    ಆಯ್ಕೆ (regs->al) {
    ಸ್ಥಿತಿ 7:
        sercon_putchar(0x07);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8:
        sercon_lazy_backspace();
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\r':
        sercon_lazy_cr();
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\n':
        sercon_lazy_lf();
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        sercon_lazy_putchar(regs->al, 0, 1);
        ನಿಲ್ಲಿಸು;
    }
}

/* Get current video mode */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_100f(ರಚನೆ bregs *regs)
{
    regs->al = GET_BDA(video_ಬಹುಲಕ);
    regs->ah = GET_BDA(video_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
}

/* VBE 2.0 */
ಸ್ಥಿರ ಶೂನ್ಯ sercon_104f(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } ಇಲ್ಲದಿದ್ದರೆ {
        // Disable sercon entry point on any vesa modeset
        ಆದರೆ (regs->al == 0x02)
            ಗಣ_ಕೆಳಗಿನ(sercon_enable, 0);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ sercon_10XX(ರಚನೆ bregs *regs)
{
    warn_unimplemented(regs);
}

ಶೂನ್ಯ VISIBLE16
handle_sercon(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!CONFIG_SERCON)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!GET_ಕೆಳಗಿನ(sercon_ಪೋರ್ಟ್))
        ಹಿಂತಿರುಗಿ;

    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x01:
    ಸ್ಥಿತಿ 0x02:
    ಸ್ಥಿತಿ 0x03:
    ಸ್ಥಿತಿ 0x08:
    ಸ್ಥಿತಿ 0x0f:
        ಆದರೆ (sercon_splitmode())
            /* nothing, vgabios handles it */
            ಹಿಂತಿರುಗಿ;
    }

    ಆಯ್ಕೆ (regs->ah) {
    ಸ್ಥಿತಿ 0x00: sercon_1000(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: sercon_1001(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x02: sercon_1002(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x03: sercon_1003(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x06: sercon_1006(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x08: sercon_1008(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x09: sercon_1009(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0e: sercon_100e(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0f: sercon_100f(regs); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x4f: sercon_104f(regs); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:   sercon_10XX(regs); ನಿಲ್ಲಿಸು;
    }
}

ಶೂನ್ಯ sercon_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_SERCON)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    ಆದರೆ (CONFIG_DEBUG_SERIAL)
        ಆದರೆ (addr == CONFIG_DEBUG_SERIAL_ಪೋರ್ಟ್)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(ನಮೂದು_10);
    ಆದರೆ (vgabios.seg != seabios.seg ||
        vgabios.ಸ್ಥಾನಾಂತರ_2 != seabios.ಸ್ಥಾನಾಂತರ_2) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.ಸ್ಥಾನಾಂತರ_2);
        sercon_real_vga_handler = vgabios;
        ಗಣ_ಕೆಳಗಿನ(sercon_ವಿಭಜಿಸು, 1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    ಗಣ_IVT(0x10, FUNC16(ನಮೂದು_sercon));
    ಗಣ_ಕೆಳಗಿನ(sercon_ಪೋರ್ಟ್, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_ಬೈಟುಗಳು;

ಸ್ಥಿರ VAR16 ರಚನೆ {
    ಅಕ್ಷರ seq[4];
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

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    //ಮೇಲೆ
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    //ಕೆಳಕ್ಕೆ
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //ಬಲ
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //ಎಡ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    //ಕೊನೆ
};

ಸ್ಥಿರ ಶೂನ್ಯ shiftbuf(ಪೂರ್ಣಾಂಕ remove)
{
    ಪೂರ್ಣಾಂಕ i, ಉಳಿದ;

    ಉಳಿದ = GET_ಕೆಳಗಿನ(rx_ಬೈಟುಗಳು) - remove;
    ಗಣ_ಕೆಳಗಿನ(rx_ಬೈಟುಗಳು, ಉಳಿದ);
    ಗಾಗಿ (i = 0; i < ಉಳಿದ; i++)
        ಗಣ_ಕೆಳಗಿನ(rx_buf[i], GET_ಕೆಳಗಿನ(rx_buf[i + remove]));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmpbuf(ಪೂರ್ಣಾಂಕ seq)
{
    ಪೂರ್ಣಾಂಕ chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    ಆದರೆ (GET_ಕೆಳಗಿನ(rx_ಬೈಟುಗಳು) < len + 1)
        ಹಿಂತಿರುಗಿ 0;
    ಗಾಗಿ (chr = 0; chr < len; chr++)
        ಆದರೆ (GET_GLOBAL(termseq[seq].seq[chr]) != GET_ಕೆಳಗಿನ(rx_buf[chr + 1]))
            ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ findseq(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ seq;

    ಗಾಗಿ (seq = 0; seq < ARRAY_ಗಾತ್ರ(termseq); seq++)
        ಆದರೆ (cmpbuf(seq))
            ಹಿಂತಿರುಗಿ seq;
    ಹಿಂತಿರುಗಿ -1;
}

ಶೂನ್ಯ
sercon_check_event(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_SERCON)
        ಹಿಂತಿರುಗಿ;

    u16 addr = GET_ಕೆಳಗಿನ(sercon_ಪೋರ್ಟ್);
    u16 keycode;
    u8 byte, ಎಣಿಕೆ_2 = 0;
    ಪೂರ್ಣಾಂಕ seq;

    // check to see if there is a active serial port
    ಆದರೆ (!addr)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (inb(addr + SEROFF_LSR) == 0xFF)
        ಹಿಂತಿರುಗಿ;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    ಆಗ (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_ದತ್ತಾಂಶ);
        u8 rb = GET_ಕೆಳಗಿನ(rx_ಬೈಟುಗಳು);
        ಆದರೆ (rb < ಗಾತ್ರ(rx_buf)) {
            ಗಣ_ಕೆಳಗಿನ(rx_buf[rb], byte);
            ಗಣ_ಕೆಳಗಿನ(rx_ಬೈಟುಗಳು, rb + 1);
            ಎಣಿಕೆ_2++;
        }
    }

    ಗಾಗಿ (;;) {
        // no (more) input data
        u8 rb = GET_ಕೆಳಗಿನ(rx_ಬೈಟುಗಳು);
        ಆದರೆ (!rb)
            ಹಿಂತಿರುಗಿ;

        // lookup escape sequences
        u8 ಮುಂದಿನ_char = GET_ಕೆಳಗಿನ(rx_buf[0]);
        ಆದರೆ (rb > 1 && ಮುಂದಿನ_char == 0x1b) {
            seq = findseq();
            ಆದರೆ (seq >= 0) {
                ಸರತಿಗೆ_ಸೇರಿಸು_ಕೀಲಿ(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                ಮುಂದುವರಿ;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        ಆದರೆ (ಮುಂದಿನ_char == 0x1b && ಎಣಿಕೆ_2)
            ಹಿಂತಿರುಗಿ;

        // Handle input as individual char.
        keycode = ascii_ಗಮ್ಯ_keycode(ಮುಂದಿನ_char);
        ಆದರೆ (keycode)
            ಸರತಿಗೆ_ಸೇರಿಸು_ಕೀಲಿ(keycode);
        shiftbuf(1);
    }
}
