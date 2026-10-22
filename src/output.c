// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h> // va_list

#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // GET_VAR
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_bdf_to_bus
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // pci_device
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/serialio.h" // serial_debug_putc
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_tmp
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // call16_int
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // ScreenAndDebug

ਬਣਤਰ putcinfo {
    ਖਾਲੀ (*func)(ਬਣਤਰ putcinfo *info, ਅੱਖਰ c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

ਖਾਲੀ
debug_banner(ਖਾਲੀ)
{
    dprintf(1, "SeaBIOS (version %s)\n", ਸੰਸਕਰਣ);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
ਸਥਿਰ ਖਾਲੀ
debug_putc(ਬਣਤਰ putcinfo *action, ਅੱਖਰ c)
{
    ਜੇ (! CONFIG_DEBUG_LEVEL)
        ਵਾਪਸ;
    qemu_debug_putc(c);
    ਜੇ (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
ਸਥਿਰ ਖਾਲੀ
debug_flush(ਖਾਲੀ)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#ਜੇ MODE16
ਸਥਿਰ ਬਣਤਰ putcinfo debuginfo VAR16;
#ਜਾਂਜੇ MODESEGMENT
ਸਥਿਰ ਬਣਤਰ putcinfo debuginfo VAR32SEG;
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਬਣਤਰ putcinfo debuginfo = { debug_putc };
#ਜੇ_ਅੰਤ


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
ਸਥਿਰ ਖਾਲੀ
screenc(ਅੱਖਰ c)
{
    ਜੇ (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(ਇੰਦਰਾਜ_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        ਵਾਪਸ;
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
ਸਥਿਰ ਖਾਲੀ
screen_putc(ਬਣਤਰ putcinfo *action, ਅੱਖਰ c)
{
    ਜੇ (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    ਜੇ (c == '\n')
        screenc('\r');
    screenc(c);
}

ਸਥਿਰ ਬਣਤਰ putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
ਸਥਿਰ ਖਾਲੀ
putc(ਬਣਤਰ putcinfo *action, ਅੱਖਰ c)
{
    ਜੇ (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        ਵਾਪਸ;
    }

    ਖਾਲੀ (*func)(ਬਣਤਰ putcinfo *info, ਅੱਖਰ c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
ਸਥਿਰ ਖਾਲੀ
puts(ਬਣਤਰ putcinfo *action, ਅਚਲ ਅੱਖਰ *s)
{
    ਜੇ (!MODESEGMENT && !s)
        s = "(NULL)";
    ਲਈ (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
ਸਥਿਰ ਖਾਲੀ
puts_cs(ਬਣਤਰ putcinfo *action, ਅਚਲ ਅੱਖਰ *s)
{
    ਅੱਖਰ *vs = (ਅੱਖਰ*)s;
    ਲਈ (;; vs++) {
        ਅੱਖਰ c = GET_GLOBAL(*vs);
        ਜੇ (!c)
            ਰੋਕੋ;
        putc(action, c);
    }
}

// Output an unsigned integer.
ਸਥਿਰ ਖਾਲੀ
putuint(ਬਣਤਰ putcinfo *action, u32 val)
{
    ਅੱਖਰ buf[12];
    ਅੱਖਰ *d = &buf[ਆਕਾਰ(buf) - 1];
    *d-- = '\0';
    ਲਈ (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        ਜੇ (!val)
            ਰੋਕੋ;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
putsinglehex(ਬਣਤਰ putcinfo *action, u32 val, ਪੂਰਨ uc)
{
    ਜੇ (val <= 9)
        val = '0' + val;
    ਨਹੀਂਤਾਂ ਜੇ (uc)
        val = 'A' + val - 10;
    ਨਹੀਂਤਾਂ
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
ਸਥਿਰ ਖਾਲੀ
puthex(ਬਣਤਰ putcinfo *action, u32 val, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ uc)
{
    ਚੋਣ (ਚੌੜਾਈ) {
    ਮੂਲ: putsinglehex(action, (val >> 28) & 0xf, uc);
    ਹਾਲਤ 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    ਹਾਲਤ 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    ਹਾਲਤ 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    ਹਾਲਤ 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    ਹਾਲਤ 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    ਹਾਲਤ 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    ਹਾਲਤ 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
ਸਥਿਰ ਖਾਲੀ
putprettyhex(ਬਣਤਰ putcinfo *action, u32 val, ਪੂਰਨ ਚੌੜਾਈ, ਅੱਖਰ padchar, ਪੂਰਨ uc)
{
    u32 tmp = val;
    ਪੂਰਨ ਗਿਣਤੀ_2 = 1;
    ਜਦੋਂ (tmp >>= 4)
        ਗਿਣਤੀ_2++;
    ਚੌੜਾਈ -= ਗਿਣਤੀ_2;
    ਜਦੋਂ (ਚੌੜਾਈ-- > 0)
        putc(action, padchar);
    puthex(action, val, ਗਿਣਤੀ_2, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
ਸਥਿਰ ਖਾਲੀ
put_pci_device(ਬਣਤਰ putcinfo *action, ਬਣਤਰ pci_device *pci)
{
    puthex(action, pci_bdf_ਮੰਜ਼ਿਲ_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_ਮੰਜ਼ਿਲ_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_ਮੰਜ਼ਿਲ_fn(pci->bdf), 1, 0);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ
ਅੰਕ_ਹੈ(u8 c)
{
    ਵਾਪਸ ((u8)(c - '0')) < 10;
}

ਸਥਿਰ ਖਾਲੀ
bvprintf(ਬਣਤਰ putcinfo *action, ਅਚਲ ਅੱਖਰ *fmt, va_list ਦਲੀਲਾਂ)
{
    ਅਚਲ ਅੱਖਰ *s = fmt;
    ਪੂਰਨ uc;
    ਲਈ (;; s++) {
        ਅੱਖਰ c = GET_GLOBAL(*(u8*)s);
        ਜੇ (!c)
            ਰੋਕੋ;
        ਜੇ (c != '%') {
            putc(action, c);
            ਜਾਰੀ;
        }
        ਅਚਲ ਅੱਖਰ *n = s+1;
        ਪੂਰਨ ਖੇਤਰ_ਚੌੜਾਈ = 0;
        ਅੱਖਰ padchar = ' ';
        u8 is64 = 0;
        ਲਈ (;;) {
            c = GET_GLOBAL(*(u8*)n);
            ਜੇ (!ਅੰਕ_ਹੈ(c))
                ਰੋਕੋ;
            ਜੇ (!ਖੇਤਰ_ਚੌੜਾਈ && (c == '0'))
                padchar = '0';
            ਨਹੀਂਤਾਂ
                ਖੇਤਰ_ਚੌੜਾਈ = ਖੇਤਰ_ਚੌੜਾਈ * 10 + c - '0';
            n++;
        }
        ਜੇ (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        ਜੇ (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ਅਚਲ ਅੱਖਰ *sarg;
        ਚੋਣ (c) {
        ਹਾਲਤ '%':
            putc(action, '%');
            ਰੋਕੋ;
        ਹਾਲਤ 'd':
            val = va_arg(ਦਲੀਲਾਂ, s32);
            ਜੇ (is64)
                va_arg(ਦਲੀਲਾਂ, s32);
            ਜੇ (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            ਰੋਕੋ;
        ਹਾਲਤ 'u':
            val = va_arg(ਦਲੀਲਾਂ, s32);
            ਜੇ (is64)
                va_arg(ਦਲੀਲਾਂ, s32);
            putuint(action, val);
            ਰੋਕੋ;
        ਹਾਲਤ 'p':
            val = va_arg(ਦਲੀਲਾਂ, s32);
            ਜੇ (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (ਖਾਲੀ*)val);
                n++;
                ਰੋਕੋ;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            ਰੋਕੋ;
        ਹਾਲਤ 'X':
        ਹਾਲਤ 'x':
            uc = (c == 'X');
            val = va_arg(ਦਲੀਲਾਂ, s32);
            ਜੇ (is64) {
                u32 ਉੱਤਲਾ = va_arg(ਦਲੀਲਾਂ, s32);
                ਜੇ (ਉੱਤਲਾ) {
                    putprettyhex(action, ਉੱਤਲਾ, ਖੇਤਰ_ਚੌੜਾਈ - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    ਰੋਕੋ;
                }
            }
            putprettyhex(action, val, ਖੇਤਰ_ਚੌੜਾਈ, padchar, uc);
            ਰੋਕੋ;
        ਹਾਲਤ 'c':
            val = va_arg(ਦਲੀਲਾਂ, ਪੂਰਨ);
            putc(action, val);
            ਰੋਕੋ;
        ਹਾਲਤ '.':
            // Hack to support "%.s" - meaning string on stack.
            ਜੇ (GET_GLOBAL(*(u8*)(n+1)) != 's')
                ਰੋਕੋ;
            n++;
            sarg = va_arg(ਦਲੀਲਾਂ, ਅਚਲ ਅੱਖਰ *);
            puts(action, sarg);
            ਰੋਕੋ;
        ਹਾਲਤ 's':
            sarg = va_arg(ਦਲੀਲਾਂ, ਅਚਲ ਅੱਖਰ *);
            puts_cs(action, sarg);
            ਰੋਕੋ;
        ਮੂਲ:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

ਖਾਲੀ
panic(ਅਚਲ ਅੱਖਰ *fmt, ...)
{
    ਜੇ (CONFIG_DEBUG_LEVEL) {
        va_list ਦਲੀਲਾਂ;
        va_start(ਦਲੀਲਾਂ, fmt);
        bvprintf(&debuginfo, fmt, ਦਲੀਲਾਂ);
        va_end(ਦਲੀਲਾਂ);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    ਲਈ (;;)
        hlt();
}

ਖਾਲੀ
__dprintf(ਅਚਲ ਅੱਖਰ *fmt, ...)
{
    ਜੇ (!MODESEGMENT && CONFIG_ਧਾਗੇ && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        ਬਣਤਰ thread_info *cur = getCurThread();
        ਜੇ (cur != &MainThread) {
            // Show "thread id" for this debug message.
            debug_putc(&debuginfo, '|');
            puthex(&debuginfo, (u32)cur, 8, 0);
            debug_putc(&debuginfo, '|');
            debug_putc(&debuginfo, ' ');
        }
    }

    va_list ਦਲੀਲਾਂ;
    va_start(ਦਲੀਲਾਂ, fmt);
    bvprintf(&debuginfo, fmt, ਦਲੀਲਾਂ);
    va_end(ਦਲੀਲਾਂ);
    debug_flush();
}

ਖਾਲੀ
ਲਿਖੋ(ਅਚਲ ਅੱਖਰ *fmt, ...)
{
    ASSERT32FLAT();
    va_list ਦਲੀਲਾਂ;
    va_start(ਦਲੀਲਾਂ, fmt);
    bvprintf(&screeninfo, fmt, ਦਲੀਲਾਂ);
    va_end(ਦਲੀਲਾਂ);
    ਜੇ (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

ਬਣਤਰ snprintfinfo {
    ਬਣਤਰ putcinfo info;
    ਅੱਖਰ *str, *ਸਿਰਾ;
};

ਸਥਿਰ ਖਾਲੀ
putc_str(ਬਣਤਰ putcinfo *info, ਅੱਖਰ c)
{
    ਬਣਤਰ snprintfinfo *sinfo = container_ਦਾ(info, ਬਣਤਰ snprintfinfo, info);
    ਜੇ (sinfo->str >= sinfo->ਸਿਰਾ)
        ਵਾਪਸ;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
ਪੂਰਨ
snprintf(ਅੱਖਰ *str, ਮਾਪ_t ਮਾਪ, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
    ASSERT32FLAT();
    ਜੇ (!ਮਾਪ)
        ਵਾਪਸ 0;
    ਬਣਤਰ snprintfinfo sinfo = { { putc_str }, str, str + ਮਾਪ };
    va_list ਦਲੀਲਾਂ;
    va_start(ਦਲੀਲਾਂ, fmt);
    bvprintf(&sinfo.info, fmt, ਦਲੀਲਾਂ);
    va_end(ਦਲੀਲਾਂ);
    ਅੱਖਰ *ਸਿਰਾ = sinfo.str;
    ਜੇ (ਸਿਰਾ >= sinfo.ਸਿਰਾ)
        ਸਿਰਾ = sinfo.ਸਿਰਾ - 1;
    *ਸਿਰਾ = '\0';
    ਵਾਪਸ ਸਿਰਾ - str;
}

// Build a formatted string - malloc'ing the memory.
ਅੱਖਰ *
znprintf(ਮਾਪ_t ਮਾਪ, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
    ASSERT32FLAT();
    ਜੇ (!ਮਾਪ)
        ਵਾਪਸ ਨਿਰਾ;
    ਅੱਖਰ *str = malloc_tmp(ਮਾਪ);
    ਜੇ (!str) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਬਣਤਰ snprintfinfo sinfo = { { putc_str }, str, str + ਮਾਪ };
    va_list ਦਲੀਲਾਂ;
    va_start(ਦਲੀਲਾਂ, fmt);
    bvprintf(&sinfo.info, fmt, ਦਲੀਲਾਂ);
    va_end(ਦਲੀਲਾਂ);
    ਅੱਖਰ *ਸਿਰਾ = sinfo.str;
    ਜੇ (ਸਿਰਾ >= sinfo.ਸਿਰਾ)
        ਸਿਰਾ = sinfo.ਸਿਰਾ - 1;
    *ਸਿਰਾ = '\0';
    ਵਾਪਸ str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

ਖਾਲੀ
hexdump(ਅਚਲ ਖਾਲੀ *d, ਪੂਰਨ len)
{
    ਪੂਰਨ ਗਿਣਤੀ_2=0;
    ਜਦੋਂ (len > 0) {
        ਜੇ (ਗਿਣਤੀ_2 % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, ਗਿਣਤੀ_2*4, 8, 0);
            putc(&debuginfo, ':');
        } ਨਹੀਂਤਾਂ {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        ਗਿਣਤੀ_2++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

ਸਥਿਰ ਖਾਲੀ
dump_regs(ਬਣਤਰ bregs *regs)
{
    ਜੇ (!regs) {
        dprintf(1, "  NULL\n");
        ਵਾਪਸ;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->ਕੋਡ.seg, regs->ਕੋਡ.ਵਿੱਥ_2, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
ਖਾਲੀ
__debug_isr(ਅਚਲ ਅੱਖਰ *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
ਖਾਲੀ
__debug_enter(ਬਣਤਰ bregs *regs, ਅਚਲ ਅੱਖਰ *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
ਖਾਲੀ
__debug_stub(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
ਖਾਲੀ
__warn_invalid(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    ਜੇ (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
ਖਾਲੀ
__warn_unimplemented(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    ਜੇ (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
ਖਾਲੀ
__warn_internalerror(ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
ਖਾਲੀ
__warn_noalloc(ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
ਖਾਲੀ
__warn_timeout(ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
ਖਾਲੀ
__set_invalid(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    __warn_invalid(regs, lineno, fname);
    ਸੈੱਟ_invalid_silent(regs);
}

// Report a call of an unimplemented function.
ਖਾਲੀ
__set_unimplemented(ਬਣਤਰ bregs *regs, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    ਸੈੱਟ_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
ਖਾਲੀ
__set_code_invalid(ਬਣਤਰ bregs *regs, u32 linecode, ਅਚਲ ਅੱਖਰ *fname)
{
    u8 ਕੋਡ = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    ਸੈੱਟ_ਕੋਡ_invalid_silent(regs, ਕੋਡ);
}

// Report a call of an unimplemented function.
ਖਾਲੀ
__set_code_unimplemented(ਬਣਤਰ bregs *regs, u32 linecode, ਅਚਲ ਅੱਖਰ *fname)
{
    u8 ਕੋਡ = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    ਸੈੱਟ_ਕੋਡ_invalid_silent(regs, ਕੋਡ);
}
