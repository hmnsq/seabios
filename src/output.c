// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ <stdarg.h> // va_list

#ಅನ್ತರ್ಭಾವ "farptr.h" // GET_VAR
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_bdf_to_bus
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_device
#ಅನ್ತರ್ಭಾವ "hw/serialio.h" // serial_debug_putc
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_tmp
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // call16_int
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // ScreenAndDebug

ಸಂರಚನಾ putcinfo {
    ಶೂನ್ಯ (*func)(ಸಂರಚನಾ putcinfo *info, ವರ್ಣ c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

ಶೂನ್ಯ
debug_banner(ಶೂನ್ಯ)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
ಸ್ಥಿರ ಶೂನ್ಯ
debug_putc(ಸಂರಚನಾ putcinfo *action, ವರ್ಣ c)
{
    ಯದಿ (! CONFIG_DEBUG_LEVEL)
        ಪ್ರತ್ಯಾಗಮ;
    qemu_debug_putc(c);
    ಯದಿ (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
ಸ್ಥಿರ ಶೂನ್ಯ
debug_flush(ಶೂನ್ಯ)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#ಯದಿ MODE16
ಸ್ಥಿರ ಸಂರಚನಾ putcinfo debuginfo VAR16;
#ನೋಚೇದ್ಯದಿ MODESEGMENT
ಸ್ಥಿರ ಸಂರಚನಾ putcinfo debuginfo VAR32SEG;
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಸಂರಚನಾ putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
ಸ್ಥಿರ ಶೂನ್ಯ
screenc(ವರ್ಣ c)
{
    ಯದಿ (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
ಸ್ಥಿರ ಶೂನ್ಯ
screen_putc(ಸಂರಚನಾ putcinfo *action, ವರ್ಣ c)
{
    ಯದಿ (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    ಯದಿ (c == '\n')
        screenc('\r');
    screenc(c);
}

ಸ್ಥಿರ ಸಂರಚನಾ putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
ಸ್ಥಿರ ಶೂನ್ಯ
putc(ಸಂರಚನಾ putcinfo *action, ವರ್ಣ c)
{
    ಯದಿ (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        ಪ್ರತ್ಯಾಗಮ;
    }

    ಶೂನ್ಯ (*func)(ಸಂರಚನಾ putcinfo *info, ವರ್ಣ c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
ಸ್ಥಿರ ಶೂನ್ಯ
puts(ಸಂರಚನಾ putcinfo *action, ಧ್ರುವ ವರ್ಣ *s)
{
    ಯದಿ (!MODESEGMENT && !s)
        s = "(NULL)";
    ಪ್ರತಿ (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
ಸ್ಥಿರ ಶೂನ್ಯ
puts_cs(ಸಂರಚನಾ putcinfo *action, ಧ್ರುವ ವರ್ಣ *s)
{
    ವರ್ಣ *vs = (ವರ್ಣ*)s;
    ಪ್ರತಿ (;; vs++) {
        ವರ್ಣ c = GET_GLOBAL(*vs);
        ಯದಿ (!c)
            ಭಙ್ಗ;
        putc(action, c);
    }
}

// Output an unsigned integer.
ಸ್ಥಿರ ಶೂನ್ಯ
putuint(ಸಂರಚನಾ putcinfo *action, u32 val)
{
    ವರ್ಣ buf[12];
    ವರ್ಣ *d = &buf[ಪರಿಮಾಣ(buf) - 1];
    *d-- = '\0';
    ಪ್ರತಿ (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        ಯದಿ (!val)
            ಭಙ್ಗ;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
putsinglehex(ಸಂರಚನಾ putcinfo *action, u32 val, ಪೂರ್ಣಾಙ್ಕ uc)
{
    ಯದಿ (val <= 9)
        val = '0' + val;
    ಅನ್ಯಥಾ ಯದಿ (uc)
        val = 'A' + val - 10;
    ಅನ್ಯಥಾ
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
ಸ್ಥಿರ ಶೂನ್ಯ
puthex(ಸಂರಚನಾ putcinfo *action, u32 val, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ uc)
{
    ವಿಕಲ್ಪ (width) {
    ಸ್ವಭಾವ: putsinglehex(action, (val >> 28) & 0xf, uc);
    ಪಕ್ಷ 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    ಪಕ್ಷ 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    ಪಕ್ಷ 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    ಪಕ್ಷ 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    ಪಕ್ಷ 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    ಪಕ್ಷ 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    ಪಕ್ಷ 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
ಸ್ಥಿರ ಶೂನ್ಯ
putprettyhex(ಸಂರಚನಾ putcinfo *action, u32 val, ಪೂರ್ಣಾಙ್ಕ width, ವರ್ಣ padchar, ಪೂರ್ಣಾಙ್ಕ uc)
{
    u32 tmp = val;
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 1;
    ಯಾವತ್ (tmp >>= 4)
        ಎಣಿಕೆ++;
    width -= ಎಣಿಕೆ;
    ಯಾವತ್ (width-- > 0)
        putc(action, padchar);
    puthex(action, val, ಎಣಿಕೆ, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
ಸ್ಥಿರ ಶೂನ್ಯ
put_pci_device(ಸಂರಚನಾ putcinfo *action, ಸಂರಚನಾ pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
isdigit(u8 c)
{
    ಪ್ರತ್ಯಾಗಮ ((u8)(c - '0')) < 10;
}

ಸ್ಥಿರ ಶೂನ್ಯ
bvprintf(ಸಂರಚನಾ putcinfo *action, ಧ್ರುವ ವರ್ಣ *fmt, va_list args)
{
    ಧ್ರುವ ವರ್ಣ *s = fmt;
    ಪೂರ್ಣಾಙ್ಕ uc;
    ಪ್ರತಿ (;; s++) {
        ವರ್ಣ c = GET_GLOBAL(*(u8*)s);
        ಯದಿ (!c)
            ಭಙ್ಗ;
        ಯದಿ (c != '%') {
            putc(action, c);
            ಅನುವರ್ತ;
        }
        ಧ್ರುವ ವರ್ಣ *n = s+1;
        ಪೂರ್ಣಾಙ್ಕ field_width = 0;
        ವರ್ಣ padchar = ' ';
        u8 is64 = 0;
        ಪ್ರತಿ (;;) {
            c = GET_GLOBAL(*(u8*)n);
            ಯದಿ (!isdigit(c))
                ಭಙ್ಗ;
            ಯದಿ (!field_width && (c == '0'))
                padchar = '0';
            ಅನ್ಯಥಾ
                field_width = field_width * 10 + c - '0';
            n++;
        }
        ಯದಿ (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        ಯದಿ (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ಧ್ರುವ ವರ್ಣ *sarg;
        ವಿಕಲ್ಪ (c) {
        ಪಕ್ಷ '%':
            putc(action, '%');
            ಭಙ್ಗ;
        ಪಕ್ಷ 'd':
            val = va_arg(args, s32);
            ಯದಿ (is64)
                va_arg(args, s32);
            ಯದಿ (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            ಭಙ್ಗ;
        ಪಕ್ಷ 'u':
            val = va_arg(args, s32);
            ಯದಿ (is64)
                va_arg(args, s32);
            putuint(action, val);
            ಭಙ್ಗ;
        ಪಕ್ಷ 'p':
            val = va_arg(args, s32);
            ಯದಿ (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (ಶೂನ್ಯ*)val);
                n++;
                ಭಙ್ಗ;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            ಭಙ್ಗ;
        ಪಕ್ಷ 'X':
        ಪಕ್ಷ 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            ಯದಿ (is64) {
                u32 upper = va_arg(args, s32);
                ಯದಿ (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    ಭಙ್ಗ;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            ಭಙ್ಗ;
        ಪಕ್ಷ 'c':
            val = va_arg(args, ಪೂರ್ಣಾಙ್ಕ);
            putc(action, val);
            ಭಙ್ಗ;
        ಪಕ್ಷ '.':
            // Hack to support "%.s" - meaning string on stack.
            ಯದಿ (GET_GLOBAL(*(u8*)(n+1)) != 's')
                ಭಙ್ಗ;
            n++;
            sarg = va_arg(args, ಧ್ರುವ ವರ್ಣ *);
            puts(action, sarg);
            ಭಙ್ಗ;
        ಪಕ್ಷ 's':
            sarg = va_arg(args, ಧ್ರುವ ವರ್ಣ *);
            puts_cs(action, sarg);
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

ಶೂನ್ಯ
panic(ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
    ಯದಿ (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    ಪ್ರತಿ (;;)
        hlt();
}

ಶೂನ್ಯ
__dprintf(ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
    ಯದಿ (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        ಸಂರಚನಾ thread_info *cur = getCurThread();
        ಯದಿ (cur != &MainThread) {
            // Show "thread id" for this debug message.
            debug_putc(&debuginfo, '|');
            puthex(&debuginfo, (u32)cur, 8, 0);
            debug_putc(&debuginfo, '|');
            debug_putc(&debuginfo, ' ');
        }
    }

    va_list args;
    va_start(args, fmt);
    bvprintf(&debuginfo, fmt, args);
    va_end(args);
    debug_flush();
}

ಶೂನ್ಯ
printf(ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    ಯದಿ (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

ಸಂರಚನಾ snprintfinfo {
    ಸಂರಚನಾ putcinfo info;
    ವರ್ಣ *str, *end;
};

ಸ್ಥಿರ ಶೂನ್ಯ
putc_str(ಸಂರಚನಾ putcinfo *info, ವರ್ಣ c)
{
    ಸಂರಚನಾ snprintfinfo *sinfo = container_of(info, ಸಂರಚನಾ snprintfinfo, info);
    ಯದಿ (sinfo->str >= sinfo->end)
        ಪ್ರತ್ಯಾಗಮ;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
ಪೂರ್ಣಾಙ್ಕ
snprintf(ವರ್ಣ *str, size_t size, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
    ASSERT32FLAT();
    ಯದಿ (!size)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಸಂರಚನಾ snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    ವರ್ಣ *end = sinfo.str;
    ಯದಿ (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ಪ್ರತ್ಯಾಗಮ end - str;
}

// Build a formatted string - malloc'ing the memory.
ವರ್ಣ *
znprintf(size_t size, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
    ASSERT32FLAT();
    ಯದಿ (!size)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ವರ್ಣ *str = malloc_tmp(size);
    ಯದಿ (!str) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    ಸಂರಚನಾ snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    ವರ್ಣ *end = sinfo.str;
    ಯದಿ (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ಪ್ರತ್ಯಾಗಮ str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

ಶೂನ್ಯ
hexdump(ಧ್ರುವ ಶೂನ್ಯ *d, ಪೂರ್ಣಾಙ್ಕ len)
{
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ=0;
    ಯಾವತ್ (len > 0) {
        ಯದಿ (ಎಣಿಕೆ % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, ಎಣಿಕೆ*4, 8, 0);
            putc(&debuginfo, ':');
        } ಅನ್ಯಥಾ {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        ಎಣಿಕೆ++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

ಸ್ಥಿರ ಶೂನ್ಯ
dump_regs(ಸಂರಚನಾ bregs *regs)
{
    ಯದಿ (!regs) {
        dprintf(1, "  NULL\n");
        ಪ್ರತ್ಯಾಗಮ;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
ಶೂನ್ಯ
__debug_isr(ಧ್ರುವ ವರ್ಣ *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
ಶೂನ್ಯ
__debug_enter(ಸಂರಚನಾ bregs *regs, ಧ್ರುವ ವರ್ಣ *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
ಶೂನ್ಯ
__debug_stub(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
ಶೂನ್ಯ
__warn_invalid(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    ಯದಿ (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
ಶೂನ್ಯ
__warn_unimplemented(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    ಯದಿ (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
ಶೂನ್ಯ
__warn_internalerror(ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
ಶೂನ್ಯ
__warn_noalloc(ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
ಶೂನ್ಯ
__warn_timeout(ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
ಶೂನ್ಯ
__set_invalid(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
ಶೂನ್ಯ
__set_unimplemented(ಸಂರಚನಾ bregs *regs, ಪೂರ್ಣಾಙ್ಕ lineno, ಧ್ರುವ ವರ್ಣ *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
ಶೂನ್ಯ
__set_code_invalid(ಸಂರಚನಾ bregs *regs, u32 linecode, ಧ್ರುವ ವರ್ಣ *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
ಶೂನ್ಯ
__set_code_unimplemented(ಸಂರಚನಾ bregs *regs, u32 linecode, ಧ್ರುವ ವರ್ಣ *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
