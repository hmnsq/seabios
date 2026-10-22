// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ <stdarg.h> // va_list

#અન્તર્ભાવ "farptr.h" // GET_VAR
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "hw/pci.h" // pci_bdf_to_bus
#અન્તર્ભાવ "hw/pcidevice.h" // pci_device
#અન્તર્ભાવ "hw/serialio.h" // serial_debug_putc
#અન્તર્ભાવ "malloc.h" // malloc_tmp
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // call16_int
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // ScreenAndDebug

સંરચના putcinfo {
    શૂન્ય (*func)(સંરચના putcinfo *info, અક્ષર c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

શૂન્ય
debug_banner(શૂન્ય)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
સ્થિર શૂન્ય
debug_putc(સંરચના putcinfo *action, અક્ષર c)
{
    જો (! CONFIG_DEBUG_LEVEL)
        પાછા;
    qemu_debug_putc(c);
    જો (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
સ્થિર શૂન્ય
debug_flush(શૂન્ય)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#જો MODE16
સ્થિર સંરચના putcinfo debuginfo VAR16;
#અથવાજો MODESEGMENT
સ્થિર સંરચના putcinfo debuginfo VAR32SEG;
#નહીંતર
સ્થિર સંરચના putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
સ્થિર શૂન્ય
screenc(અક્ષર c)
{
    જો (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        પાછા;
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
સ્થિર શૂન્ય
screen_putc(સંરચના putcinfo *action, અક્ષર c)
{
    જો (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    જો (c == '\n')
        screenc('\r');
    screenc(c);
}

સ્થિર સંરચના putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
સ્થિર શૂન્ય
putc(સંરચના putcinfo *action, અક્ષર c)
{
    જો (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        પાછા;
    }

    શૂન્ય (*func)(સંરચના putcinfo *info, અક્ષર c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
સ્થિર શૂન્ય
puts(સંરચના putcinfo *action, અચલ અક્ષર *s)
{
    જો (!MODESEGMENT && !s)
        s = "(NULL)";
    માટે (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
સ્થિર શૂન્ય
puts_cs(સંરચના putcinfo *action, અચલ અક્ષર *s)
{
    અક્ષર *vs = (અક્ષર*)s;
    માટે (;; vs++) {
        અક્ષર c = GET_GLOBAL(*vs);
        જો (!c)
            તોડો;
        putc(action, c);
    }
}

// Output an unsigned integer.
સ્થિર શૂન્ય
putuint(સંરચના putcinfo *action, u32 val)
{
    અક્ષર buf[12];
    અક્ષર *d = &buf[કદ(buf) - 1];
    *d-- = '\0';
    માટે (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        જો (!val)
            તોડો;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
સ્થિર અંતઃરેખ શૂન્ય
putsinglehex(સંરચના putcinfo *action, u32 val, પૂર્ણાંક uc)
{
    જો (val <= 9)
        val = '0' + val;
    નહીંતર જો (uc)
        val = 'A' + val - 10;
    નહીંતર
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
સ્થિર શૂન્ય
puthex(સંરચના putcinfo *action, u32 val, પૂર્ણાંક width, પૂર્ણાંક uc)
{
    પસંદગી (width) {
    મૂળભૂત: putsinglehex(action, (val >> 28) & 0xf, uc);
    સ્થિતિ 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    સ્થિતિ 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    સ્થિતિ 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    સ્થિતિ 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    સ્થિતિ 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    સ્થિતિ 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    સ્થિતિ 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
સ્થિર શૂન્ય
putprettyhex(સંરચના putcinfo *action, u32 val, પૂર્ણાંક width, અક્ષર padchar, પૂર્ણાંક uc)
{
    u32 tmp = val;
    પૂર્ણાંક ગણતરી = 1;
    જ્યારે (tmp >>= 4)
        ગણતરી++;
    width -= ગણતરી;
    જ્યારે (width-- > 0)
        putc(action, padchar);
    puthex(action, val, ગણતરી, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
સ્થિર શૂન્ય
put_pci_device(સંરચના putcinfo *action, સંરચના pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

સ્થિર અંતઃરેખ પૂર્ણાંક
isdigit(u8 c)
{
    પાછા ((u8)(c - '0')) < 10;
}

સ્થિર શૂન્ય
bvprintf(સંરચના putcinfo *action, અચલ અક્ષર *fmt, va_list args)
{
    અચલ અક્ષર *s = fmt;
    પૂર્ણાંક uc;
    માટે (;; s++) {
        અક્ષર c = GET_GLOBAL(*(u8*)s);
        જો (!c)
            તોડો;
        જો (c != '%') {
            putc(action, c);
            ચાલુ;
        }
        અચલ અક્ષર *n = s+1;
        પૂર્ણાંક field_width = 0;
        અક્ષર padchar = ' ';
        u8 is64 = 0;
        માટે (;;) {
            c = GET_GLOBAL(*(u8*)n);
            જો (!isdigit(c))
                તોડો;
            જો (!field_width && (c == '0'))
                padchar = '0';
            નહીંતર
                field_width = field_width * 10 + c - '0';
            n++;
        }
        જો (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        જો (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        અચલ અક્ષર *sarg;
        પસંદગી (c) {
        સ્થિતિ '%':
            putc(action, '%');
            તોડો;
        સ્થિતિ 'd':
            val = va_arg(args, s32);
            જો (is64)
                va_arg(args, s32);
            જો (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            તોડો;
        સ્થિતિ 'u':
            val = va_arg(args, s32);
            જો (is64)
                va_arg(args, s32);
            putuint(action, val);
            તોડો;
        સ્થિતિ 'p':
            val = va_arg(args, s32);
            જો (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (શૂન્ય*)val);
                n++;
                તોડો;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            તોડો;
        સ્થિતિ 'X':
        સ્થિતિ 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            જો (is64) {
                u32 upper = va_arg(args, s32);
                જો (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    તોડો;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            તોડો;
        સ્થિતિ 'c':
            val = va_arg(args, પૂર્ણાંક);
            putc(action, val);
            તોડો;
        સ્થિતિ '.':
            // Hack to support "%.s" - meaning string on stack.
            જો (GET_GLOBAL(*(u8*)(n+1)) != 's')
                તોડો;
            n++;
            sarg = va_arg(args, અચલ અક્ષર *);
            puts(action, sarg);
            તોડો;
        સ્થિતિ 's':
            sarg = va_arg(args, અચલ અક્ષર *);
            puts_cs(action, sarg);
            તોડો;
        મૂળભૂત:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

શૂન્ય
panic(અચલ અક્ષર *fmt, ...)
{
    જો (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    માટે (;;)
        hlt();
}

શૂન્ય
__dprintf(અચલ અક્ષર *fmt, ...)
{
    જો (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        સંરચના thread_info *cur = getCurThread();
        જો (cur != &MainThread) {
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

શૂન્ય
printf(અચલ અક્ષર *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    જો (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

સંરચના snprintfinfo {
    સંરચના putcinfo info;
    અક્ષર *str, *end;
};

સ્થિર શૂન્ય
putc_str(સંરચના putcinfo *info, અક્ષર c)
{
    સંરચના snprintfinfo *sinfo = container_of(info, સંરચના snprintfinfo, info);
    જો (sinfo->str >= sinfo->end)
        પાછા;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
પૂર્ણાંક
snprintf(અક્ષર *str, size_t size, અચલ અક્ષર *fmt, ...)
{
    ASSERT32FLAT();
    જો (!size)
        પાછા 0;
    સંરચના snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    અક્ષર *end = sinfo.str;
    જો (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    પાછા end - str;
}

// Build a formatted string - malloc'ing the memory.
અક્ષર *
znprintf(size_t size, અચલ અક્ષર *fmt, ...)
{
    ASSERT32FLAT();
    જો (!size)
        પાછા ખાલી;
    અક્ષર *str = malloc_tmp(size);
    જો (!str) {
        warn_noalloc();
        પાછા ખાલી;
    }
    સંરચના snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    અક્ષર *end = sinfo.str;
    જો (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    પાછા str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

શૂન્ય
hexdump(અચલ શૂન્ય *d, પૂર્ણાંક len)
{
    પૂર્ણાંક ગણતરી=0;
    જ્યારે (len > 0) {
        જો (ગણતરી % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, ગણતરી*4, 8, 0);
            putc(&debuginfo, ':');
        } નહીંતર {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        ગણતરી++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

સ્થિર શૂન્ય
dump_regs(સંરચના bregs *regs)
{
    જો (!regs) {
        dprintf(1, "  NULL\n");
        પાછા;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
શૂન્ય
__debug_isr(અચલ અક્ષર *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
શૂન્ય
__debug_enter(સંરચના bregs *regs, અચલ અક્ષર *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
શૂન્ય
__debug_stub(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
શૂન્ય
__warn_invalid(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    જો (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
શૂન્ય
__warn_unimplemented(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    જો (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
શૂન્ય
__warn_internalerror(પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
શૂન્ય
__warn_noalloc(પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
શૂન્ય
__warn_timeout(પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
શૂન્ય
__set_invalid(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
શૂન્ય
__set_unimplemented(સંરચના bregs *regs, પૂર્ણાંક lineno, અચલ અક્ષર *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
શૂન્ય
__set_code_invalid(સંરચના bregs *regs, u32 linecode, અચલ અક્ષર *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
શૂન્ય
__set_code_unimplemented(સંરચના bregs *regs, u32 linecode, અચલ અક્ષર *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
