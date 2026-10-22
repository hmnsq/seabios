// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ <stdarg.h> // va_list

#অন্তর্ভাৱ "farptr.h" // GET_VAR
#অন্তর্ভাৱ "bregs.h" // struct bregs
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "biosvar.h" // GET_GLOBAL
#অন্তর্ভাৱ "hw/pci.h" // pci_bdf_to_bus
#অন্তর্ভাৱ "hw/pcidevice.h" // pci_device
#অন্তর্ভাৱ "hw/serialio.h" // serial_debug_putc
#অন্তর্ভাৱ "malloc.h" // malloc_tmp
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "stacks.h" // call16_int
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // ScreenAndDebug

সংরচনা putcinfo {
    শূন্য (*func)(সংরচনা putcinfo *info, ৱর্ণ c);
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
debug_putc(সংরচনা putcinfo *action, ৱর্ণ c)
{
    যদি (! CONFIG_DEBUG_LEVEL)
        ঘূৰাই;
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
স্থির সংরচনা putcinfo debuginfo VAR16;
#নোচেদ্যদি MODESEGMENT
স্থির সংরচনা putcinfo debuginfo VAR32SEG;
#অন্যথা
স্থির সংরচনা putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
স্থির শূন্য
screenc(ৱর্ণ c)
{
    যদি (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        ঘূৰাই;
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
স্থির শূন্য
screen_putc(সংরচনা putcinfo *action, ৱর্ণ c)
{
    যদি (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    যদি (c == '\n')
        screenc('\r');
    screenc(c);
}

স্থির সংরচনা putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
স্থির শূন্য
putc(সংরচনা putcinfo *action, ৱর্ণ c)
{
    যদি (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        ঘূৰাই;
    }

    শূন্য (*func)(সংরচনা putcinfo *info, ৱর্ণ c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
স্থির শূন্য
puts(সংরচনা putcinfo *action, ধ্রুৱ ৱর্ণ *s)
{
    যদি (!MODESEGMENT && !s)
        s = "(NULL)";
    বাবে (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
স্থির শূন্য
puts_cs(সংরচনা putcinfo *action, ধ্রুৱ ৱর্ণ *s)
{
    ৱর্ণ *vs = (ৱর্ণ*)s;
    বাবে (;; vs++) {
        ৱর্ণ c = GET_GLOBAL(*vs);
        যদি (!c)
            ভঙ্গ;
        putc(action, c);
    }
}

// Output an unsigned integer.
স্থির শূন্য
putuint(সংরচনা putcinfo *action, u32 val)
{
    ৱর্ণ buf[12];
    ৱর্ণ *d = &buf[পরিমাণ(buf) - 1];
    *d-- = '\0';
    বাবে (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        যদি (!val)
            ভঙ্গ;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
স্থির অন্তঃস্থ শূন্য
putsinglehex(সংরচনা putcinfo *action, u32 val, পূর্ণাঙ্ক uc)
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
puthex(সংরচনা putcinfo *action, u32 val, পূর্ণাঙ্ক width, পূর্ণাঙ্ক uc)
{
    ৱিকল্প (width) {
    স্ৱভাৱ: putsinglehex(action, (val >> 28) & 0xf, uc);
    পক্ষ 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    পক্ষ 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    পক্ষ 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    পক্ষ 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    পক্ষ 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    পক্ষ 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    পক্ষ 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
স্থির শূন্য
putprettyhex(সংরচনা putcinfo *action, u32 val, পূর্ণাঙ্ক width, ৱর্ণ padchar, পূর্ণাঙ্ক uc)
{
    u32 tmp = val;
    পূর্ণাঙ্ক গুনতি = 1;
    যেতিয়ালৈ (tmp >>= 4)
        গুনতি++;
    width -= গুনতি;
    যেতিয়ালৈ (width-- > 0)
        putc(action, padchar);
    puthex(action, val, গুনতি, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
স্থির শূন্য
put_pci_device(সংরচনা putcinfo *action, সংরচনা pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক
isdigit(u8 c)
{
    ঘূৰাই ((u8)(c - '0')) < 10;
}

স্থির শূন্য
bvprintf(সংরচনা putcinfo *action, ধ্রুৱ ৱর্ণ *fmt, va_list args)
{
    ধ্রুৱ ৱর্ণ *s = fmt;
    পূর্ণাঙ্ক uc;
    বাবে (;; s++) {
        ৱর্ণ c = GET_GLOBAL(*(u8*)s);
        যদি (!c)
            ভঙ্গ;
        যদি (c != '%') {
            putc(action, c);
            অনুৱর্ত;
        }
        ধ্রুৱ ৱর্ণ *n = s+1;
        পূর্ণাঙ্ক field_width = 0;
        ৱর্ণ padchar = ' ';
        u8 is64 = 0;
        বাবে (;;) {
            c = GET_GLOBAL(*(u8*)n);
            যদি (!isdigit(c))
                ভঙ্গ;
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
        ধ্রুৱ ৱর্ণ *sarg;
        ৱিকল্প (c) {
        পক্ষ '%':
            putc(action, '%');
            ভঙ্গ;
        পক্ষ 'd':
            val = va_arg(args, s32);
            যদি (is64)
                va_arg(args, s32);
            যদি (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            ভঙ্গ;
        পক্ষ 'u':
            val = va_arg(args, s32);
            যদি (is64)
                va_arg(args, s32);
            putuint(action, val);
            ভঙ্গ;
        পক্ষ 'p':
            val = va_arg(args, s32);
            যদি (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (শূন্য*)val);
                n++;
                ভঙ্গ;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            ভঙ্গ;
        পক্ষ 'X':
        পক্ষ 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            যদি (is64) {
                u32 upper = va_arg(args, s32);
                যদি (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    ভঙ্গ;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            ভঙ্গ;
        পক্ষ 'c':
            val = va_arg(args, পূর্ণাঙ্ক);
            putc(action, val);
            ভঙ্গ;
        পক্ষ '.':
            // Hack to support "%.s" - meaning string on stack.
            যদি (GET_GLOBAL(*(u8*)(n+1)) != 's')
                ভঙ্গ;
            n++;
            sarg = va_arg(args, ধ্রুৱ ৱর্ণ *);
            puts(action, sarg);
            ভঙ্গ;
        পক্ষ 's':
            sarg = va_arg(args, ধ্রুৱ ৱর্ণ *);
            puts_cs(action, sarg);
            ভঙ্গ;
        স্ৱভাৱ:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

শূন্য
panic(ধ্রুৱ ৱর্ণ *fmt, ...)
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
    বাবে (;;)
        hlt();
}

শূন্য
__dprintf(ধ্রুৱ ৱর্ণ *fmt, ...)
{
    যদি (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        সংরচনা thread_info *cur = getCurThread();
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
printf(ধ্রুৱ ৱর্ণ *fmt, ...)
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

সংরচনা snprintfinfo {
    সংরচনা putcinfo info;
    ৱর্ণ *str, *end;
};

স্থির শূন্য
putc_str(সংরচনা putcinfo *info, ৱর্ণ c)
{
    সংরচনা snprintfinfo *sinfo = container_of(info, সংরচনা snprintfinfo, info);
    যদি (sinfo->str >= sinfo->end)
        ঘূৰাই;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
পূর্ণাঙ্ক
snprintf(ৱর্ণ *str, size_t size, ধ্রুৱ ৱর্ণ *fmt, ...)
{
    ASSERT32FLAT();
    যদি (!size)
        ঘূৰাই 0;
    সংরচনা snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    ৱর্ণ *end = sinfo.str;
    যদি (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ঘূৰাই end - str;
}

// Build a formatted string - malloc'ing the memory.
ৱর্ণ *
znprintf(size_t size, ধ্রুৱ ৱর্ণ *fmt, ...)
{
    ASSERT32FLAT();
    যদি (!size)
        ঘূৰাই রিক্ত;
    ৱর্ণ *str = malloc_tmp(size);
    যদি (!str) {
        warn_noalloc();
        ঘূৰাই রিক্ত;
    }
    সংরচনা snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    ৱর্ণ *end = sinfo.str;
    যদি (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ঘূৰাই str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

শূন্য
hexdump(ধ্রুৱ শূন্য *d, পূর্ণাঙ্ক len)
{
    পূর্ণাঙ্ক গুনতি=0;
    যেতিয়ালৈ (len > 0) {
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
dump_regs(সংরচনা bregs *regs)
{
    যদি (!regs) {
        dprintf(1, "  NULL\n");
        ঘূৰাই;
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
__debug_isr(ধ্রুৱ ৱর্ণ *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
শূন্য
__debug_enter(সংরচনা bregs *regs, ধ্রুৱ ৱর্ণ *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
শূন্য
__debug_stub(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
শূন্য
__warn_invalid(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    যদি (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
শূন্য
__warn_unimplemented(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    যদি (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
শূন্য
__warn_internalerror(পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
শূন্য
__warn_noalloc(পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
শূন্য
__warn_timeout(পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
শূন্য
__set_invalid(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
শূন্য
__set_unimplemented(সংরচনা bregs *regs, পূর্ণাঙ্ক lineno, ধ্রুৱ ৱর্ণ *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
শূন্য
__set_code_invalid(সংরচনা bregs *regs, u32 linecode, ধ্রুৱ ৱর্ণ *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
শূন্য
__set_code_unimplemented(সংরচনা bregs *regs, u32 linecode, ধ্রুৱ ৱর্ণ *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
