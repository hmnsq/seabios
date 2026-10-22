// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব <stdarg.h> // va_list

#অন্তর্ভাব "farptr.h" // GET_VAR
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "hw/pci.h" // pci_bdf_to_bus
#অন্তর্ভাব "hw/pcidevice.h" // pci_device
#অন্তর্ভাব "hw/serialio.h" // serial_debug_putc
#অন্তর্ভাব "malloc.h" // malloc_tmp
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "stacks.h" // call16_int
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // ScreenAndDebug

কাঠামো putcinfo {
    শূন্য (*func)(কাঠামো putcinfo *info, অক্ষর c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

শূন্য
debug_banner(শূন্য)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
স্থির শূন্য
debug_putc(কাঠামো putcinfo *action, অক্ষর c)
{
    যদি (! CONFIG_DEBUG_LEVEL)
        ফেরত;
    qemu_debug_putc(c);
    যদি (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
স্থির শূন্য
debug_flush(শূন্য)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#যদি MODE16
স্থির কাঠামো putcinfo debuginfo VAR16;
#নতুবা_যদি MODESEGMENT
স্থির কাঠামো putcinfo debuginfo VAR32SEG;
#অন্যথা
স্থির কাঠামো putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
স্থির শূন্য
screenc(অক্ষর c)
{
    যদি (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        ফেরত;
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
স্থির শূন্য
screen_putc(কাঠামো putcinfo *action, অক্ষর c)
{
    যদি (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    যদি (c == '\n')
        screenc('\r');
    screenc(c);
}

স্থির কাঠামো putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
স্থির শূন্য
putc(কাঠামো putcinfo *action, অক্ষর c)
{
    যদি (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        ফেরত;
    }

    শূন্য (*func)(কাঠামো putcinfo *info, অক্ষর c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
স্থির শূন্য
puts(কাঠামো putcinfo *action, ধ্রুবক অক্ষর *s)
{
    যদি (!MODESEGMENT && !s)
        s = "(NULL)";
    জন্য (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
স্থির শূন্য
puts_cs(কাঠামো putcinfo *action, ধ্রুবক অক্ষর *s)
{
    অক্ষর *vs = (অক্ষর*)s;
    জন্য (;; vs++) {
        অক্ষর c = GET_GLOBAL(*vs);
        যদি (!c)
            বিরতি;
        putc(action, c);
    }
}

// Output an unsigned integer.
স্থির শূন্য
putuint(কাঠামো putcinfo *action, u32 val)
{
    অক্ষর buf[12];
    অক্ষর *d = &buf[আকার(buf) - 1];
    *d-- = '\0';
    জন্য (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        যদি (!val)
            বিরতি;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
স্থির অন্তঃরেখ শূন্য
putsinglehex(কাঠামো putcinfo *action, u32 val, পূর্ণসংখ্যা uc)
{
    যদি (val <= 9)
        val = '0' + val;
    অন্যথা যদি (uc)
        val = 'A' + val - 10;
    অন্যথা
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
স্থির শূন্য
puthex(কাঠামো putcinfo *action, u32 val, পূর্ণসংখ্যা width, পূর্ণসংখ্যা uc)
{
    নির্বাচন (width) {
    পূর্বনির্ধারিত: putsinglehex(action, (val >> 28) & 0xf, uc);
    ক্ষেত্র 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    ক্ষেত্র 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    ক্ষেত্র 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    ক্ষেত্র 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    ক্ষেত্র 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    ক্ষেত্র 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    ক্ষেত্র 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
স্থির শূন্য
putprettyhex(কাঠামো putcinfo *action, u32 val, পূর্ণসংখ্যা width, অক্ষর padchar, পূর্ণসংখ্যা uc)
{
    u32 tmp = val;
    পূর্ণসংখ্যা গুনতি = 1;
    যখন (tmp >>= 4)
        গুনতি++;
    width -= গুনতি;
    যখন (width-- > 0)
        putc(action, padchar);
    puthex(action, val, গুনতি, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
স্থির শূন্য
put_pci_device(কাঠামো putcinfo *action, কাঠামো pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা
isdigit(u8 c)
{
    ফেরত ((u8)(c - '0')) < 10;
}

স্থির শূন্য
bvprintf(কাঠামো putcinfo *action, ধ্রুবক অক্ষর *fmt, va_list args)
{
    ধ্রুবক অক্ষর *s = fmt;
    পূর্ণসংখ্যা uc;
    জন্য (;; s++) {
        অক্ষর c = GET_GLOBAL(*(u8*)s);
        যদি (!c)
            বিরতি;
        যদি (c != '%') {
            putc(action, c);
            চালু;
        }
        ধ্রুবক অক্ষর *n = s+1;
        পূর্ণসংখ্যা field_width = 0;
        অক্ষর padchar = ' ';
        u8 is64 = 0;
        জন্য (;;) {
            c = GET_GLOBAL(*(u8*)n);
            যদি (!isdigit(c))
                বিরতি;
            যদি (!field_width && (c == '0'))
                padchar = '0';
            অন্যথা
                field_width = field_width * 10 + c - '0';
            n++;
        }
        যদি (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        যদি (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ধ্রুবক অক্ষর *sarg;
        নির্বাচন (c) {
        ক্ষেত্র '%':
            putc(action, '%');
            বিরতি;
        ক্ষেত্র 'd':
            val = va_arg(args, s32);
            যদি (is64)
                va_arg(args, s32);
            যদি (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            বিরতি;
        ক্ষেত্র 'u':
            val = va_arg(args, s32);
            যদি (is64)
                va_arg(args, s32);
            putuint(action, val);
            বিরতি;
        ক্ষেত্র 'p':
            val = va_arg(args, s32);
            যদি (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (শূন্য*)val);
                n++;
                বিরতি;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            বিরতি;
        ক্ষেত্র 'X':
        ক্ষেত্র 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            যদি (is64) {
                u32 upper = va_arg(args, s32);
                যদি (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    বিরতি;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            বিরতি;
        ক্ষেত্র 'c':
            val = va_arg(args, পূর্ণসংখ্যা);
            putc(action, val);
            বিরতি;
        ক্ষেত্র '.':
            // Hack to support "%.s" - meaning string on stack.
            যদি (GET_GLOBAL(*(u8*)(n+1)) != 's')
                বিরতি;
            n++;
            sarg = va_arg(args, ধ্রুবক অক্ষর *);
            puts(action, sarg);
            বিরতি;
        ক্ষেত্র 's':
            sarg = va_arg(args, ধ্রুবক অক্ষর *);
            puts_cs(action, sarg);
            বিরতি;
        পূর্বনির্ধারিত:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

শূন্য
panic(ধ্রুবক অক্ষর *fmt, ...)
{
    যদি (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    জন্য (;;)
        hlt();
}

শূন্য
__dprintf(ধ্রুবক অক্ষর *fmt, ...)
{
    যদি (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        কাঠামো thread_info *cur = getCurThread();
        যদি (cur != &MainThread) {
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

শূন্য
printf(ধ্রুবক অক্ষর *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    যদি (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

কাঠামো snprintfinfo {
    কাঠামো putcinfo info;
    অক্ষর *str, *end;
};

স্থির শূন্য
putc_str(কাঠামো putcinfo *info, অক্ষর c)
{
    কাঠামো snprintfinfo *sinfo = container_of(info, কাঠামো snprintfinfo, info);
    যদি (sinfo->str >= sinfo->end)
        ফেরত;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
পূর্ণসংখ্যা
snprintf(অক্ষর *str, আকার_t আকার_2, ধ্রুবক অক্ষর *fmt, ...)
{
    ASSERT32FLAT();
    যদি (!আকার_2)
        ফেরত 0;
    কাঠামো snprintfinfo sinfo = { { putc_str }, str, str + আকার_2 };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    অক্ষর *end = sinfo.str;
    যদি (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ফেরত end - str;
}

// Build a formatted string - malloc'ing the memory.
অক্ষর *
znprintf(আকার_t আকার_2, ধ্রুবক অক্ষর *fmt, ...)
{
    ASSERT32FLAT();
    যদি (!আকার_2)
        ফেরত ফাঁকা;
    অক্ষর *str = malloc_tmp(আকার_2);
    যদি (!str) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    কাঠামো snprintfinfo sinfo = { { putc_str }, str, str + আকার_2 };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    অক্ষর *end = sinfo.str;
    যদি (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ফেরত str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

শূন্য
hexdump(ধ্রুবক শূন্য *d, পূর্ণসংখ্যা len)
{
    পূর্ণসংখ্যা গুনতি=0;
    যখন (len > 0) {
        যদি (গুনতি % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, গুনতি*4, 8, 0);
            putc(&debuginfo, ':');
        } অন্যথা {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        গুনতি++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

স্থির শূন্য
dump_regs(কাঠামো bregs *regs)
{
    যদি (!regs) {
        dprintf(1, "  NULL\n");
        ফেরত;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
শূন্য
__debug_isr(ধ্রুবক অক্ষর *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
শূন্য
__debug_enter(কাঠামো bregs *regs, ধ্রুবক অক্ষর *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
শূন্য
__debug_stub(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
শূন্য
__warn_invalid(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    যদি (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
শূন্য
__warn_unimplemented(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    যদি (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
শূন্য
__warn_internalerror(পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
শূন্য
__warn_noalloc(পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
শূন্য
__warn_timeout(পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
শূন্য
__set_invalid(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
শূন্য
__set_unimplemented(কাঠামো bregs *regs, পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
শূন্য
__set_code_invalid(কাঠামো bregs *regs, u32 linecode, ধ্রুবক অক্ষর *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
শূন্য
__set_code_unimplemented(কাঠামো bregs *regs, u32 linecode, ধ্রুবক অক্ষর *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
