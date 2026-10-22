// serial console support
//
// Copyright (C) 2016 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // SET_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // irqtimer_calc_ticks
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/serialio.h" // SEROFF_IER
#ਅਨ੍ਤਰ੍ਭਾਵ "cp437.h"

ਸਥਿਰ u8 video_ਪੰਕਤੀਆਂ(ਖਾਲੀ)
{
    ਵਾਪਸ GET_BDA(video_ਪੰਕਤੀਆਂ)+1;
}

ਸਥਿਰ u8 video_ਖਾਨੇ_ਗਿਣਤੀ(ਖਾਲੀ)
{
    ਵਾਪਸ GET_BDA(video_ਖਾਨੇ_ਗਿਣਤੀ);
}

ਸਥਿਰ u8 cursor_pos_ਖਾਨਾ(ਖਾਲੀ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ਵਾਪਸ pos & 0xff;
}

ਸਥਿਰ u8 cursor_pos_ਪੰਕਤੀ(ਖਾਲੀ)
{
    u16 pos = GET_BDA(cursor_pos[0]);
    ਵਾਪਸ (pos >> 8) & 0xff;
}

ਸਥਿਰ ਖਾਲੀ cursor_pos_ਸੈੱਟ(u8 ਪੰਕਤੀ, u8 ਖਾਨਾ)
{
    u16 pos = ((u16)ਪੰਕਤੀ << 8) | ਖਾਨਾ;
    ਸੈੱਟ_BDA(cursor_pos[0], pos);
}

/****************************************************************
 * serial console output
 ****************************************************************/

VARLOW u16 sercon_ਪੋਰਟ;
VARLOW u8 sercon_ਵੰਡੋ;
VARLOW u8 sercon_enable;
VARFSEG ਬਣਤਰ segoff_s sercon_real_vga_handler;

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
VARLOW u8 sercon_attr_ਆਖਰੀ;
VARLOW u8 sercon_ਖਾਨਾ_ਆਖਰੀ;
VARLOW u8 sercon_ਪੰਕਤੀ_ਆਖਰੀ;
VARLOW u8 sercon_char;
VARLOW u8 sercon_attr = 0x07;

ਸਥਿਰ VAR16 u8 sercon_cmap[8] = { '0', '4', '2', '6', '1', '5', '3', '7' };

ਸਥਿਰ ਪੂਰਨ sercon_splitmode(ਖਾਲੀ)
{
    ਵਾਪਸ GET_ਨੀਵਾਂ(sercon_ਵੰਡੋ);
}

ਸਥਿਰ ਖਾਲੀ sercon_putchar(u8 chr)
{
    u16 addr = GET_ਨੀਵਾਂ(sercon_ਪੋਰਟ);
    u32 ਸਿਰਾ = irqtimer_calc_ticks(0x0a);

#ਜੇ 0
    /* for visual control sequence debugging */
    ਜੇ (chr == '\x1b')
        chr = '*';
#ਜੇ_ਅੰਤ

    ਲਈ (;;) {
        u8 lsr = inb(addr+SEROFF_LSR);
        ਜੇ ((lsr & 0x60) == 0x60) {
            // Success - can write data
            outb(chr, addr+SEROFF_ਅੰਕੜੇ);
            ਰੋਕੋ;
        }
        ਜੇ (irqtimer_check(ਸਿਰਾ)) {
            ਰੋਕੋ;
        }
        yield();
    }
}

ਸਥਿਰ ਖਾਲੀ sercon_ਪਦ_reset(ਖਾਲੀ)
{
    sercon_putchar('\x1b');
    sercon_putchar('c');
}

ਸਥਿਰ ਖਾਲੀ sercon_ਪਦ_clear_screen(ਖਾਲੀ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('2');
    sercon_putchar('J');
}

ਸਥਿਰ ਖਾਲੀ sercon_ਪਦ_no_linewrap(ਖਾਲੀ)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('?');
    sercon_putchar('7');
    sercon_putchar('l');
}

ਸਥਿਰ ਖਾਲੀ sercon_ਪਦ_cursor_goto(u8 ਪੰਕਤੀ, u8 ਖਾਨਾ)
{
    ਪੰਕਤੀ++; ਖਾਨਾ++;
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0' + ਪੰਕਤੀ / 10);
    sercon_putchar('0' + ਪੰਕਤੀ % 10);
    sercon_putchar(';');
    sercon_putchar('0' + ਖਾਨਾ / 10);
    sercon_putchar('0' + ਖਾਨਾ % 10);
    sercon_putchar('H');
}

ਸਥਿਰ ਖਾਲੀ sercon_ਪਦ_ਸੈੱਟ_color(u8 fg, u8 bg, u8 bold)
{
    sercon_putchar('\x1b');
    sercon_putchar('[');
    sercon_putchar('0');
    ਜੇ (fg != 7) {
        sercon_putchar(';');
        sercon_putchar('3');
        sercon_putchar(GET_GLOBAL(sercon_cmap[fg & 7]));
    }
    ਜੇ (bg != 0) {
        sercon_putchar(';');
        sercon_putchar('4');
        sercon_putchar(GET_GLOBAL(sercon_cmap[bg & 7]));
    }
    ਜੇ (bold) {
        sercon_putchar(';');
        sercon_putchar('1');
    }
    sercon_putchar('m');
}

ਸਥਿਰ ਖਾਲੀ sercon_ਸੈੱਟ_attr(u8 attr)
{
    ਜੇ (attr == GET_ਨੀਵਾਂ(sercon_attr_ਆਖਰੀ))
        ਵਾਪਸ;

    ਸੈੱਟ_ਨੀਵਾਂ(sercon_attr_ਆਖਰੀ, attr);
    sercon_ਪਦ_ਸੈੱਟ_color((attr >> 0) & 7,
                          (attr >> 4) & 7,
                          attr & 0x08);
}

ਸਥਿਰ ਖਾਲੀ sercon_print_utf8(u8 chr)
{
    u16 unicode = cp437_ਮੰਜ਼ਿਲ_unicode(chr);

    ਜੇ (unicode < 0x7f) {
        sercon_putchar(unicode);
    } ਨਹੀਂਤਾਂ ਜੇ (unicode < 0x7ff) {
        sercon_putchar(0xc0 | ((unicode >>  6) & 0x1f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    } ਨਹੀਂਤਾਂ {
        sercon_putchar(0xe0 | ((unicode >> 12) & 0x0f));
        sercon_putchar(0x80 | ((unicode >>  6) & 0x3f));
        sercon_putchar(0x80 | ((unicode >>  0) & 0x3f));
    }
}

ਸਥਿਰ ਖਾਲੀ sercon_cursor_pos_ਸੈੱਟ(u8 ਪੰਕਤੀ, u8 ਖਾਨਾ)
{
    ਜੇ (!sercon_splitmode()) {
        cursor_pos_ਸੈੱਟ(ਪੰਕਤੀ, ਖਾਨਾ);
    } ਨਹੀਂਤਾਂ {
        /* let vgabios update cursor */
    }
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_cursor_sync(ਖਾਲੀ)
{
    u8 ਪੰਕਤੀ = cursor_pos_ਪੰਕਤੀ();
    u8 ਖਾਨਾ = cursor_pos_ਖਾਨਾ();

    ਜੇ (GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ) == ਪੰਕਤੀ &&
        GET_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ) == ਖਾਨਾ)
        ਵਾਪਸ;

    ਜੇ (ਖਾਨਾ == 0 && GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ) <= ਪੰਕਤੀ) {
        ਜੇ (GET_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ) != 0) {
            sercon_putchar('\r');
            ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ, 0);
        }
        ਜਦੋਂ (GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ) < ਪੰਕਤੀ) {
            sercon_putchar('\n');
            ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ, GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ)+1);
        }
        ਜੇ (GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ) == ਪੰਕਤੀ &&
            GET_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ) == ਖਾਨਾ)
            ਵਾਪਸ;
    }

    sercon_ਪਦ_cursor_goto(ਪੰਕਤੀ, ਖਾਨਾ);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ, ਪੰਕਤੀ);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ, ਖਾਨਾ);
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_flush(ਖਾਲੀ)
{
    u8 chr, attr;

    chr = GET_ਨੀਵਾਂ(sercon_char);
    attr = GET_ਨੀਵਾਂ(sercon_attr);
    ਜੇ (chr) {
        sercon_ਸੈੱਟ_attr(attr);
        sercon_print_utf8(chr);
        ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ, GET_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ) + 1);
    }

    sercon_lazy_cursor_sync();

    ਸੈੱਟ_ਨੀਵਾਂ(sercon_attr, 0x07);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_char, 0x00);
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_cursor_update(u8 ਪੰਕਤੀ, u8 ਖਾਨਾ)
{
    sercon_cursor_pos_ਸੈੱਟ(ਪੰਕਤੀ, ਖਾਨਾ);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ, ਪੰਕਤੀ);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ, ਖਾਨਾ);
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_backspace(ਖਾਲੀ)
{
    u8 ਖਾਨਾ;

    sercon_lazy_flush();
    ਖਾਨਾ = cursor_pos_ਖਾਨਾ();
    ਜੇ (ਖਾਨਾ > 0) {
        sercon_putchar(8);
        sercon_lazy_cursor_update(cursor_pos_ਪੰਕਤੀ(), ਖਾਨਾ-1);
    }
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_cr(ਖਾਲੀ)
{
    sercon_cursor_pos_ਸੈੱਟ(cursor_pos_ਪੰਕਤੀ(), 0);
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_lf(ਖਾਲੀ)
{
    u8 ਪੰਕਤੀ;

    ਪੰਕਤੀ = cursor_pos_ਪੰਕਤੀ() + 1;
    ਜੇ (ਪੰਕਤੀ >= video_ਪੰਕਤੀਆਂ()) {
        /* scrolling up */
        ਪੰਕਤੀ = video_ਪੰਕਤੀਆਂ()-1;
        ਜੇ (GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ) > 0) {
            ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ, GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ) - 1);
        }
    }
    sercon_cursor_pos_ਸੈੱਟ(ਪੰਕਤੀ, cursor_pos_ਖਾਨਾ());
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_move_cursor(ਖਾਲੀ)
{
    u8 ਖਾਨਾ;

    ਖਾਨਾ = cursor_pos_ਖਾਨਾ() + 1;
    ਜੇ (ਖਾਨਾ >= video_ਖਾਨੇ_ਗਿਣਤੀ()) {
        sercon_lazy_cr();
        sercon_lazy_lf();
    } ਨਹੀਂਤਾਂ {
        sercon_cursor_pos_ਸੈੱਟ(cursor_pos_ਪੰਕਤੀ(), ਖਾਨਾ);
    }
}

ਸਥਿਰ ਖਾਲੀ sercon_lazy_putchar(u8 chr, u8 attr, u8 teletype)
{
    ਜੇ (cursor_pos_ਪੰਕਤੀ() != GET_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ) ||
        cursor_pos_ਖਾਨਾ() != GET_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ)) {
        sercon_lazy_flush();
    }

    ਸੈੱਟ_ਨੀਵਾਂ(sercon_char, chr);
    ਜੇ (teletype)
        sercon_lazy_move_cursor();
    ਨਹੀਂਤਾਂ
        ਸੈੱਟ_ਨੀਵਾਂ(sercon_attr, attr);
}

/* Set video mode */
ਸਥਿਰ ਖਾਲੀ sercon_1000(ਬਣਤਰ bregs *regs)
{
    u8 clearscreen = !(regs->al & 0x80);
    u8 ਬਹੁਲਕ = regs->al & 0x7f;
    u8 ਪੰਕਤੀਆਂ, ਖਾਨੇ_ਗਿਣਤੀ;

    ਜੇ (!sercon_splitmode()) {
        ਚੋਣ (ਬਹੁਲਕ) {
        ਹਾਲਤ 0x00:
        ਹਾਲਤ 0x01:
        ਹਾਲਤ 0x04: /* 320x200 */
        ਹਾਲਤ 0x05: /* 320x200 */
            ਖਾਨੇ_ਗਿਣਤੀ = 40;
            ਪੰਕਤੀਆਂ = 25;
            regs->al = 0x30;
            ਰੋਕੋ;
        ਹਾਲਤ 0x02:
        ਹਾਲਤ 0x03:
        ਹਾਲਤ 0x06: /* 640x200 */
        ਹਾਲਤ 0x07:
        ਮੂਲ:
            ਖਾਨੇ_ਗਿਣਤੀ = 80;
            ਪੰਕਤੀਆਂ = 25;
            regs->al = 0x30;
            ਰੋਕੋ;
        }
        cursor_pos_ਸੈੱਟ(0, 0);
        ਸੈੱਟ_BDA(video_ਬਹੁਲਕ, ਬਹੁਲਕ);
        ਸੈੱਟ_BDA(video_ਖਾਨੇ_ਗਿਣਤੀ, ਖਾਨੇ_ਗਿਣਤੀ);
        ਸੈੱਟ_BDA(video_ਪੰਕਤੀਆਂ, ਪੰਕਤੀਆਂ-1);
        ਸੈੱਟ_BDA(cursor_type, 0x0007);
    } ਨਹੀਂਤਾਂ {
        /* let vgabios handle mode init */
    }

    ਸੈੱਟ_ਨੀਵਾਂ(sercon_enable, ਬਹੁਲਕ <= 0x07);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਖਾਨਾ_ਆਖਰੀ, 0);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਪੰਕਤੀ_ਆਖਰੀ, 0);
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_attr_ਆਖਰੀ, 0);

    sercon_ਪਦ_reset();
    sercon_ਪਦ_no_linewrap();
    ਜੇ (clearscreen)
        sercon_ਪਦ_clear_screen();
}

/* Set text-mode cursor shape */
ਸਥਿਰ ਖਾਲੀ sercon_1001(ਬਣਤਰ bregs *regs)
{
    /* show/hide cursor? */
    ਸੈੱਟ_BDA(cursor_type, regs->cx);
}

/* Set cursor position */
ਸਥਿਰ ਖਾਲੀ sercon_1002(ਬਣਤਰ bregs *regs)
{
    sercon_cursor_pos_ਸੈੱਟ(regs->dh, regs->dl);
}

/* Get cursor position */
ਸਥਿਰ ਖਾਲੀ sercon_1003(ਬਣਤਰ bregs *regs)
{
    regs->cx = GET_BDA(cursor_type);
    regs->dh = cursor_pos_ਪੰਕਤੀ();
    regs->dl = cursor_pos_ਖਾਨਾ();
}

/* Scroll up window */
ਸਥਿਰ ਖਾਲੀ sercon_1006(ਬਣਤਰ bregs *regs)
{
    sercon_lazy_flush();
    ਜੇ (regs->al == 0) {
        /* clear rect, do only in case this looks like a fullscreen clear */
        ਜੇ (regs->ch == 0 &&
            regs->cl == 0 &&
            regs->dh == video_ਪੰਕਤੀਆਂ()-1 &&
            regs->dl == video_ਖਾਨੇ_ਗਿਣਤੀ()-1) {
            sercon_ਸੈੱਟ_attr(regs->bh);
            sercon_ਪਦ_clear_screen();
        }
    } ਨਹੀਂਤਾਂ {
        sercon_putchar('\r');
        sercon_putchar('\n');
    }
}

/* Read character and attribute at cursor position */
ਸਥਿਰ ਖਾਲੀ sercon_1008(ਬਣਤਰ bregs *regs)
{
    regs->ah = 0x07;
    regs->bh = ' ';
}

/* Write character and attribute at cursor position */
ਸਥਿਰ ਖਾਲੀ sercon_1009(ਬਣਤਰ bregs *regs)
{
    u16 ਗਿਣਤੀ_2 = regs->cx;

    ਜੇ (ਗਿਣਤੀ_2 == 1) {
        sercon_lazy_putchar(regs->al, regs->bl, 0);

    } ਨਹੀਂਤਾਂ ਜੇ (regs->al == 0x20 &&
               video_ਪੰਕਤੀਆਂ() * video_ਖਾਨੇ_ਗਿਣਤੀ() == ਗਿਣਤੀ_2 &&
               cursor_pos_ਪੰਕਤੀ() == 0 &&
               cursor_pos_ਖਾਨਾ() == 0) {
        /* override everything with spaces -> this is clear screen */
        sercon_lazy_flush();
        sercon_ਸੈੱਟ_attr(regs->bl);
        sercon_ਪਦ_clear_screen();

    } ਨਹੀਂਤਾਂ {
        sercon_lazy_flush();
        sercon_ਸੈੱਟ_attr(regs->bl);
        ਜਦੋਂ (ਗਿਣਤੀ_2) {
            sercon_print_utf8(regs->al);
            ਗਿਣਤੀ_2--;
        }
        sercon_ਪਦ_cursor_goto(cursor_pos_ਪੰਕਤੀ(),
                                cursor_pos_ਖਾਨਾ());
    }
}

/* Teletype output */
ਸਥਿਰ ਖਾਲੀ sercon_100e(ਬਣਤਰ bregs *regs)
{
    ਚੋਣ (regs->al) {
    ਹਾਲਤ 7:
        sercon_putchar(0x07);
        ਰੋਕੋ;
    ਹਾਲਤ 8:
        sercon_lazy_backspace();
        ਰੋਕੋ;
    ਹਾਲਤ '\r':
        sercon_lazy_cr();
        ਰੋਕੋ;
    ਹਾਲਤ '\n':
        sercon_lazy_lf();
        ਰੋਕੋ;
    ਮੂਲ:
        sercon_lazy_putchar(regs->al, 0, 1);
        ਰੋਕੋ;
    }
}

/* Get current video mode */
ਸਥਿਰ ਖਾਲੀ sercon_100f(ਬਣਤਰ bregs *regs)
{
    regs->al = GET_BDA(video_ਬਹੁਲਕ);
    regs->ah = GET_BDA(video_ਖਾਨੇ_ਗਿਣਤੀ);
}

/* VBE 2.0 */
ਸਥਿਰ ਖਾਲੀ sercon_104f(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!sercon_splitmode()) {
        regs->ax = 0x0100;
    } ਨਹੀਂਤਾਂ {
        // Disable sercon entry point on any vesa modeset
        ਜੇ (regs->al == 0x02)
            ਸੈੱਟ_ਨੀਵਾਂ(sercon_enable, 0);
    }
}

ਸਥਿਰ ਖਾਲੀ sercon_10XX(ਬਣਤਰ bregs *regs)
{
    warn_unimplemented(regs);
}

ਖਾਲੀ VISIBLE16
handle_sercon(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!CONFIG_SERCON)
        ਵਾਪਸ;
    ਜੇ (!GET_ਨੀਵਾਂ(sercon_ਪੋਰਟ))
        ਵਾਪਸ;

    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x01:
    ਹਾਲਤ 0x02:
    ਹਾਲਤ 0x03:
    ਹਾਲਤ 0x08:
    ਹਾਲਤ 0x0f:
        ਜੇ (sercon_splitmode())
            /* nothing, vgabios handles it */
            ਵਾਪਸ;
    }

    ਚੋਣ (regs->ah) {
    ਹਾਲਤ 0x00: sercon_1000(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x01: sercon_1001(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x02: sercon_1002(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x03: sercon_1003(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x06: sercon_1006(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x08: sercon_1008(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x09: sercon_1009(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0e: sercon_100e(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x0f: sercon_100f(regs); ਰੋਕੋ;
    ਹਾਲਤ 0x4f: sercon_104f(regs); ਰੋਕੋ;
    ਮੂਲ:   sercon_10XX(regs); ਰੋਕੋ;
    }
}

ਖਾਲੀ sercon_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_SERCON)
        ਵਾਪਸ;

    ਬਣਤਰ segoff_s seabios, vgabios;
    u16 addr;

    addr = romfile_loadint("etc/sercon-port", 0);
    ਜੇ (!addr)
        ਵਾਪਸ;
    dprintf(1, "sercon: using ioport 0x%x\n", addr);

    ਜੇ (CONFIG_DEBUG_SERIAL)
        ਜੇ (addr == CONFIG_DEBUG_SERIAL_ਪੋਰਟ)
            ScreenAndDebug = 0;

    vgabios = GET_IVT(0x10);
    seabios = FUNC16(ਇੰਦਰਾਜ_10);
    ਜੇ (vgabios.seg != seabios.seg ||
        vgabios.ਵਿੱਥ_2 != seabios.ਵਿੱਥ_2) {
        dprintf(1, "sercon: configuring in splitmode (vgabios %04x:%04x)\n",
                vgabios.seg, vgabios.ਵਿੱਥ_2);
        sercon_real_vga_handler = vgabios;
        ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਵੰਡੋ, 1);
    } ਨਹੀਂਤਾਂ {
        dprintf(1, "sercon: configuring as primary display\n");
        sercon_real_vga_handler = seabios;
    }

    ਸੈੱਟ_IVT(0x10, FUNC16(ਇੰਦਰਾਜ_sercon));
    ਸੈੱਟ_ਨੀਵਾਂ(sercon_ਪੋਰਟ, addr);
    outb(0x03, addr + SEROFF_LCR); // 8N1
    outb(0x01, addr + 0x02);       // enable fifo
}

/****************************************************************
 * serial input
 ****************************************************************/

VARLOW u8 rx_buf[16];
VARLOW u8 rx_ਬਾਈਟਾਂ;

ਸਥਿਰ VAR16 ਬਣਤਰ {
    ਅੱਖਰ seq[4];
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

    { .seq = "[2~",  .len = 3, .keycode = 0x52e0 },    //ਪਾਓ
    { .seq = "[3~",  .len = 3, .keycode = 0x53e0 },    // delete
    { .seq = "[5~",  .len = 3, .keycode = 0x49e0 },    // page up
    { .seq = "[6~",  .len = 3, .keycode = 0x51e0 },    // page down

    { .seq = "[A",   .len = 2, .keycode = 0x48e0 },    //ਉੱਪਰ
    { .seq = "[B",   .len = 2, .keycode = 0x50e0 },    //ਥੱਲੇ
    { .seq = "[C",   .len = 2, .keycode = 0x4de0 },    //ਸੱਜਾ
    { .seq = "[D",   .len = 2, .keycode = 0x4be0 },    //ਖੱਬਾ

    { .seq = "[H",   .len = 2, .keycode = 0x47e0 },    // home
    { .seq = "[F",   .len = 2, .keycode = 0x4fe0 },    //ਸਿਰਾ
};

ਸਥਿਰ ਖਾਲੀ shiftbuf(ਪੂਰਨ remove)
{
    ਪੂਰਨ i, ਬਚਿਆ;

    ਬਚਿਆ = GET_ਨੀਵਾਂ(rx_ਬਾਈਟਾਂ) - remove;
    ਸੈੱਟ_ਨੀਵਾਂ(rx_ਬਾਈਟਾਂ, ਬਚਿਆ);
    ਲਈ (i = 0; i < ਬਚਿਆ; i++)
        ਸੈੱਟ_ਨੀਵਾਂ(rx_buf[i], GET_ਨੀਵਾਂ(rx_buf[i + remove]));
}

ਸਥਿਰ ਪੂਰਨ cmpbuf(ਪੂਰਨ seq)
{
    ਪੂਰਨ chr, len;

    len = GET_GLOBAL(termseq[seq].len);
    ਜੇ (GET_ਨੀਵਾਂ(rx_ਬਾਈਟਾਂ) < len + 1)
        ਵਾਪਸ 0;
    ਲਈ (chr = 0; chr < len; chr++)
        ਜੇ (GET_GLOBAL(termseq[seq].seq[chr]) != GET_ਨੀਵਾਂ(rx_buf[chr + 1]))
            ਵਾਪਸ 0;
    ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ findseq(ਖਾਲੀ)
{
    ਪੂਰਨ seq;

    ਲਈ (seq = 0; seq < ARRAY_ਮਾਪ(termseq); seq++)
        ਜੇ (cmpbuf(seq))
            ਵਾਪਸ seq;
    ਵਾਪਸ -1;
}

ਖਾਲੀ
sercon_check_event(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_SERCON)
        ਵਾਪਸ;

    u16 addr = GET_ਨੀਵਾਂ(sercon_ਪੋਰਟ);
    u16 keycode;
    u8 byte, ਗਿਣਤੀ_2 = 0;
    ਪੂਰਨ seq;

    // check to see if there is a active serial port
    ਜੇ (!addr)
        ਵਾਪਸ;
    ਜੇ (inb(addr + SEROFF_LSR) == 0xFF)
        ਵਾਪਸ;

    // flush pending output
    sercon_lazy_flush();

    // read all available data
    ਜਦੋਂ (inb(addr + SEROFF_LSR) & 0x01) {
        byte = inb(addr + SEROFF_ਅੰਕੜੇ);
        u8 rb = GET_ਨੀਵਾਂ(rx_ਬਾਈਟਾਂ);
        ਜੇ (rb < ਆਕਾਰ(rx_buf)) {
            ਸੈੱਟ_ਨੀਵਾਂ(rx_buf[rb], byte);
            ਸੈੱਟ_ਨੀਵਾਂ(rx_ਬਾਈਟਾਂ, rb + 1);
            ਗਿਣਤੀ_2++;
        }
    }

    ਲਈ (;;) {
        // no (more) input data
        u8 rb = GET_ਨੀਵਾਂ(rx_ਬਾਈਟਾਂ);
        ਜੇ (!rb)
            ਵਾਪਸ;

        // lookup escape sequences
        u8 ਅਗਲਾ_char = GET_ਨੀਵਾਂ(rx_buf[0]);
        ਜੇ (rb > 1 && ਅਗਲਾ_char == 0x1b) {
            seq = findseq();
            ਜੇ (seq >= 0) {
                ਕਤਾਰ_ਵਿੱਚ_ਪਾਓ_ਕੁੰਜੀ(GET_GLOBAL(termseq[seq].keycode));
                shiftbuf(GET_GLOBAL(termseq[seq].len) + 1);
                ਜਾਰੀ;
            }
        }

        // Seems we got a escape sequence we didn't recognise.
        //  -> If we received data wait for more, maybe it is just incomplete.
        ਜੇ (ਅਗਲਾ_char == 0x1b && ਗਿਣਤੀ_2)
            ਵਾਪਸ;

        // Handle input as individual char.
        keycode = ascii_ਮੰਜ਼ਿਲ_keycode(ਅਗਲਾ_char);
        ਜੇ (keycode)
            ਕਤਾਰ_ਵਿੱਚ_ਪਾਓ_ਕੁੰਜੀ(keycode);
        shiftbuf(1);
    }
}
