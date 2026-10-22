// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ <stdarg.h> // va_list

#ଅନ୍ତର୍ଭାଵ "farptr.h" // GET_VAR
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_bdf_to_bus
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // pci_device
#ଅନ୍ତର୍ଭାଵ "hw/serialio.h" // serial_debug_putc
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_tmp
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "stacks.h" // call16_int
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // ScreenAndDebug

ସଂରଚନା putcinfo {
    ଶୂନ୍ୟ (*func)(ସଂରଚନା putcinfo *info, ଅକ୍ଷର c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

ଶୂନ୍ୟ
debug_banner(ଶୂନ୍ୟ)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
ସ୍ଥିର ଶୂନ୍ୟ
debug_putc(ସଂରଚନା putcinfo *action, ଅକ୍ଷର c)
{
    ଯଦି (! CONFIG_DEBUG_LEVEL)
        ଫେରନ୍ତୁ;
    qemu_debug_putc(c);
    ଯଦି (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
ସ୍ଥିର ଶୂନ୍ୟ
debug_flush(ଶୂନ୍ୟ)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#ଯଦି MODE16
ସ୍ଥିର ସଂରଚନା putcinfo debuginfo VAR16;
#ନଚେତଯଦି MODESEGMENT
ସ୍ଥିର ସଂରଚନା putcinfo debuginfo VAR32SEG;
#ଅନ୍ୟଥା
ସ୍ଥିର ସଂରଚନା putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
ସ୍ଥିର ଶୂନ୍ୟ
screenc(ଅକ୍ଷର c)
{
    ଯଦି (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        ଫେରନ୍ତୁ;
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
ସ୍ଥିର ଶୂନ୍ୟ
screen_putc(ସଂରଚନା putcinfo *action, ଅକ୍ଷର c)
{
    ଯଦି (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    ଯଦି (c == '\n')
        screenc('\r');
    screenc(c);
}

ସ୍ଥିର ସଂରଚନା putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
ସ୍ଥିର ଶୂନ୍ୟ
putc(ସଂରଚନା putcinfo *action, ଅକ୍ଷର c)
{
    ଯଦି (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        ଫେରନ୍ତୁ;
    }

    ଶୂନ୍ୟ (*func)(ସଂରଚନା putcinfo *info, ଅକ୍ଷର c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
ସ୍ଥିର ଶୂନ୍ୟ
puts(ସଂରଚନା putcinfo *action, ଅଚଳ ଅକ୍ଷର *s)
{
    ଯଦି (!MODESEGMENT && !s)
        s = "(NULL)";
    ପାଇଁ (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
ସ୍ଥିର ଶୂନ୍ୟ
puts_cs(ସଂରଚନା putcinfo *action, ଅଚଳ ଅକ୍ଷର *s)
{
    ଅକ୍ଷର *vs = (ଅକ୍ଷର*)s;
    ପାଇଁ (;; vs++) {
        ଅକ୍ଷର c = GET_GLOBAL(*vs);
        ଯଦି (!c)
            ଭାଙ୍ଗ;
        putc(action, c);
    }
}

// Output an unsigned integer.
ସ୍ଥିର ଶୂନ୍ୟ
putuint(ସଂରଚନା putcinfo *action, u32 val)
{
    ଅକ୍ଷର buf[12];
    ଅକ୍ଷର *d = &buf[ଆକାର(buf) - 1];
    *d-- = '\0';
    ପାଇଁ (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        ଯଦି (!val)
            ଭାଙ୍ଗ;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
putsinglehex(ସଂରଚନା putcinfo *action, u32 val, ପୂର୍ଣ୍ଣସଂଖ୍ୟା uc)
{
    ଯଦି (val <= 9)
        val = '0' + val;
    ଅନ୍ୟଥା ଯଦି (uc)
        val = 'A' + val - 10;
    ଅନ୍ୟଥା
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
ସ୍ଥିର ଶୂନ୍ୟ
puthex(ସଂରଚନା putcinfo *action, u32 val, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା uc)
{
    ବାଛ (width) {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: putsinglehex(action, (val >> 28) & 0xf, uc);
    ସ୍ଥିତି 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    ସ୍ଥିତି 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    ସ୍ଥିତି 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    ସ୍ଥିତି 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    ସ୍ଥିତି 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    ସ୍ଥିତି 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    ସ୍ଥିତି 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
ସ୍ଥିର ଶୂନ୍ୟ
putprettyhex(ସଂରଚନା putcinfo *action, u32 val, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ଅକ୍ଷର padchar, ପୂର୍ଣ୍ଣସଂଖ୍ୟା uc)
{
    u32 tmp = val;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 1;
    ଯେତେବେଳେ (tmp >>= 4)
        ଗଣନା_2++;
    width -= ଗଣନା_2;
    ଯେତେବେଳେ (width-- > 0)
        putc(action, padchar);
    puthex(action, val, ଗଣନା_2, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
ସ୍ଥିର ଶୂନ୍ୟ
put_pci_device(ସଂରଚନା putcinfo *action, ସଂରଚନା pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା
isdigit(u8 c)
{
    ଫେରନ୍ତୁ ((u8)(c - '0')) < 10;
}

ସ୍ଥିର ଶୂନ୍ୟ
bvprintf(ସଂରଚନା putcinfo *action, ଅଚଳ ଅକ୍ଷର *fmt, va_list args)
{
    ଅଚଳ ଅକ୍ଷର *s = fmt;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା uc;
    ପାଇଁ (;; s++) {
        ଅକ୍ଷର c = GET_GLOBAL(*(u8*)s);
        ଯଦି (!c)
            ଭାଙ୍ଗ;
        ଯଦି (c != '%') {
            putc(action, c);
            ଜାରି;
        }
        ଅଚଳ ଅକ୍ଷର *n = s+1;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା field_width = 0;
        ଅକ୍ଷର padchar = ' ';
        u8 is64 = 0;
        ପାଇଁ (;;) {
            c = GET_GLOBAL(*(u8*)n);
            ଯଦି (!isdigit(c))
                ଭାଙ୍ଗ;
            ଯଦି (!field_width && (c == '0'))
                padchar = '0';
            ଅନ୍ୟଥା
                field_width = field_width * 10 + c - '0';
            n++;
        }
        ଯଦି (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        ଯଦି (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ଅଚଳ ଅକ୍ଷର *sarg;
        ବାଛ (c) {
        ସ୍ଥିତି '%':
            putc(action, '%');
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 'd':
            val = va_arg(args, s32);
            ଯଦି (is64)
                va_arg(args, s32);
            ଯଦି (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 'u':
            val = va_arg(args, s32);
            ଯଦି (is64)
                va_arg(args, s32);
            putuint(action, val);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 'p':
            val = va_arg(args, s32);
            ଯଦି (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (ଶୂନ୍ୟ*)val);
                n++;
                ଭାଙ୍ଗ;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 'X':
        ସ୍ଥିତି 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            ଯଦି (is64) {
                u32 upper = va_arg(args, s32);
                ଯଦି (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    ଭାଙ୍ଗ;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 'c':
            val = va_arg(args, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
            putc(action, val);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି '.':
            // Hack to support "%.s" - meaning string on stack.
            ଯଦି (GET_GLOBAL(*(u8*)(n+1)) != 's')
                ଭାଙ୍ଗ;
            n++;
            sarg = va_arg(args, ଅଚଳ ଅକ୍ଷର *);
            puts(action, sarg);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 's':
            sarg = va_arg(args, ଅଚଳ ଅକ୍ଷର *);
            puts_cs(action, sarg);
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

ଶୂନ୍ୟ
panic(ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
    ଯଦି (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    ପାଇଁ (;;)
        hlt();
}

ଶୂନ୍ୟ
__dprintf(ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
    ଯଦି (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        ସଂରଚନା thread_info *cur = getCurThread();
        ଯଦି (cur != &MainThread) {
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

ଶୂନ୍ୟ
printf(ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    ଯଦି (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

ସଂରଚନା snprintfinfo {
    ସଂରଚନା putcinfo info;
    ଅକ୍ଷର *str, *end;
};

ସ୍ଥିର ଶୂନ୍ୟ
putc_str(ସଂରଚନା putcinfo *info, ଅକ୍ଷର c)
{
    ସଂରଚନା snprintfinfo *sinfo = container_of(info, ସଂରଚନା snprintfinfo, info);
    ଯଦି (sinfo->str >= sinfo->end)
        ଫେରନ୍ତୁ;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
snprintf(ଅକ୍ଷର *str, size_t size, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
    ASSERT32FLAT();
    ଯଦି (!size)
        ଫେରନ୍ତୁ 0;
    ସଂରଚନା snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    ଅକ୍ଷର *end = sinfo.str;
    ଯଦି (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ଫେରନ୍ତୁ end - str;
}

// Build a formatted string - malloc'ing the memory.
ଅକ୍ଷର *
znprintf(size_t size, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
    ASSERT32FLAT();
    ଯଦି (!size)
        ଫେରନ୍ତୁ ଖାଲି;
    ଅକ୍ଷର *str = malloc_tmp(size);
    ଯଦି (!str) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ସଂରଚନା snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    ଅକ୍ଷର *end = sinfo.str;
    ଯଦି (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ଫେରନ୍ତୁ str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

ଶୂନ୍ୟ
hexdump(ଅଚଳ ଶୂନ୍ୟ *d, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2=0;
    ଯେତେବେଳେ (len > 0) {
        ଯଦି (ଗଣନା_2 % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, ଗଣନା_2*4, 8, 0);
            putc(&debuginfo, ':');
        } ଅନ୍ୟଥା {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        ଗଣନା_2++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

ସ୍ଥିର ଶୂନ୍ୟ
dump_regs(ସଂରଚନା bregs *regs)
{
    ଯଦି (!regs) {
        dprintf(1, "  NULL\n");
        ଫେରନ୍ତୁ;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
ଶୂନ୍ୟ
__debug_isr(ଅଚଳ ଅକ୍ଷର *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
ଶୂନ୍ୟ
__debug_enter(ସଂରଚନା bregs *regs, ଅଚଳ ଅକ୍ଷର *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
ଶୂନ୍ୟ
__debug_stub(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
ଶୂନ୍ୟ
__warn_invalid(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    ଯଦି (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
ଶୂନ୍ୟ
__warn_unimplemented(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    ଯଦି (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
ଶୂନ୍ୟ
__warn_internalerror(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
ଶୂନ୍ୟ
__warn_noalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
ଶୂନ୍ୟ
__warn_timeout(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
ଶୂନ୍ୟ
__set_invalid(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
ଶୂନ୍ୟ
__set_unimplemented(ସଂରଚନା bregs *regs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
ଶୂନ୍ୟ
__set_code_invalid(ସଂରଚନା bregs *regs, u32 linecode, ଅଚଳ ଅକ୍ଷର *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
ଶୂନ୍ୟ
__set_code_unimplemented(ସଂରଚନା bregs *regs, u32 linecode, ଅଚଳ ଅକ୍ଷର *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
