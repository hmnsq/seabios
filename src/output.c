// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ <stdarg.h> // va_list

#அந்தர்பாவ "farptr.h" // GET_VAR
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "hw/pci.h" // pci_bdf_to_bus
#அந்தர்பாவ "hw/pcidevice.h" // pci_device
#அந்தர்பாவ "hw/serialio.h" // serial_debug_putc
#அந்தர்பாவ "malloc.h" // malloc_tmp
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "stacks.h" // call16_int
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // ScreenAndDebug

கட்டமைப்பு putcinfo {
    வெறுமை (*func)(கட்டமைப்பு putcinfo *info, எழுத்து c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

வெறுமை
debug_banner(வெறுமை)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
நிலையான வெறுமை
debug_putc(கட்டமைப்பு putcinfo *action, எழுத்து c)
{
    எனில் (! CONFIG_DEBUG_LEVEL)
        திரும்பு;
    qemu_debug_putc(c);
    எனில் (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
நிலையான வெறுமை
debug_flush(வெறுமை)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#எனில் MODE16
நிலையான கட்டமைப்பு putcinfo debuginfo VAR16;
#அல்லதெனில் MODESEGMENT
நிலையான கட்டமைப்பு putcinfo debuginfo VAR32SEG;
#இல்லையெனில்
நிலையான கட்டமைப்பு putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
நிலையான வெறுமை
screenc(எழுத்து c)
{
    எனில் (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        திரும்பு;
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
நிலையான வெறுமை
screen_putc(கட்டமைப்பு putcinfo *action, எழுத்து c)
{
    எனில் (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    எனில் (c == '\n')
        screenc('\r');
    screenc(c);
}

நிலையான கட்டமைப்பு putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
நிலையான வெறுமை
putc(கட்டமைப்பு putcinfo *action, எழுத்து c)
{
    எனில் (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        திரும்பு;
    }

    வெறுமை (*func)(கட்டமைப்பு putcinfo *info, எழுத்து c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
நிலையான வெறுமை
puts(கட்டமைப்பு putcinfo *action, மாறிலி எழுத்து *s)
{
    எனில் (!MODESEGMENT && !s)
        s = "(NULL)";
    க்கு (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
நிலையான வெறுமை
puts_cs(கட்டமைப்பு putcinfo *action, மாறிலி எழுத்து *s)
{
    எழுத்து *vs = (எழுத்து*)s;
    க்கு (;; vs++) {
        எழுத்து c = GET_GLOBAL(*vs);
        எனில் (!c)
            நிறுத்து;
        putc(action, c);
    }
}

// Output an unsigned integer.
நிலையான வெறுமை
putuint(கட்டமைப்பு putcinfo *action, u32 val)
{
    எழுத்து buf[12];
    எழுத்து *d = &buf[அளவு(buf) - 1];
    *d-- = '\0';
    க்கு (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        எனில் (!val)
            நிறுத்து;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
நிலையான உள்வரி வெறுமை
putsinglehex(கட்டமைப்பு putcinfo *action, u32 val, முழு uc)
{
    எனில் (val <= 9)
        val = '0' + val;
    இல்லையெனில் எனில் (uc)
        val = 'A' + val - 10;
    இல்லையெனில்
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
நிலையான வெறுமை
puthex(கட்டமைப்பு putcinfo *action, u32 val, முழு width, முழு uc)
{
    தேர்வு (width) {
    இயல்பு: putsinglehex(action, (val >> 28) & 0xf, uc);
    நிலை 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    நிலை 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    நிலை 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    நிலை 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    நிலை 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    நிலை 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    நிலை 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
நிலையான வெறுமை
putprettyhex(கட்டமைப்பு putcinfo *action, u32 val, முழு width, எழுத்து padchar, முழு uc)
{
    u32 tmp = val;
    முழு மொத்தம் = 1;
    போது (tmp >>= 4)
        மொத்தம்++;
    width -= மொத்தம்;
    போது (width-- > 0)
        putc(action, padchar);
    puthex(action, val, மொத்தம், uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
நிலையான வெறுமை
put_pci_device(கட்டமைப்பு putcinfo *action, கட்டமைப்பு pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

நிலையான உள்வரி முழு
isdigit(u8 c)
{
    திரும்பு ((u8)(c - '0')) < 10;
}

நிலையான வெறுமை
bvprintf(கட்டமைப்பு putcinfo *action, மாறிலி எழுத்து *fmt, va_list args)
{
    மாறிலி எழுத்து *s = fmt;
    முழு uc;
    க்கு (;; s++) {
        எழுத்து c = GET_GLOBAL(*(u8*)s);
        எனில் (!c)
            நிறுத்து;
        எனில் (c != '%') {
            putc(action, c);
            தொடர்;
        }
        மாறிலி எழுத்து *n = s+1;
        முழு field_width = 0;
        எழுத்து padchar = ' ';
        u8 is64 = 0;
        க்கு (;;) {
            c = GET_GLOBAL(*(u8*)n);
            எனில் (!isdigit(c))
                நிறுத்து;
            எனில் (!field_width && (c == '0'))
                padchar = '0';
            இல்லையெனில்
                field_width = field_width * 10 + c - '0';
            n++;
        }
        எனில் (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        எனில் (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        மாறிலி எழுத்து *sarg;
        தேர்வு (c) {
        நிலை '%':
            putc(action, '%');
            நிறுத்து;
        நிலை 'd':
            val = va_arg(args, s32);
            எனில் (is64)
                va_arg(args, s32);
            எனில் (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            நிறுத்து;
        நிலை 'u':
            val = va_arg(args, s32);
            எனில் (is64)
                va_arg(args, s32);
            putuint(action, val);
            நிறுத்து;
        நிலை 'p':
            val = va_arg(args, s32);
            எனில் (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (வெறுமை*)val);
                n++;
                நிறுத்து;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            நிறுத்து;
        நிலை 'X':
        நிலை 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            எனில் (is64) {
                u32 upper = va_arg(args, s32);
                எனில் (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    நிறுத்து;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            நிறுத்து;
        நிலை 'c':
            val = va_arg(args, முழு);
            putc(action, val);
            நிறுத்து;
        நிலை '.':
            // Hack to support "%.s" - meaning string on stack.
            எனில் (GET_GLOBAL(*(u8*)(n+1)) != 's')
                நிறுத்து;
            n++;
            sarg = va_arg(args, மாறிலி எழுத்து *);
            puts(action, sarg);
            நிறுத்து;
        நிலை 's':
            sarg = va_arg(args, மாறிலி எழுத்து *);
            puts_cs(action, sarg);
            நிறுத்து;
        இயல்பு:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

வெறுமை
panic(மாறிலி எழுத்து *fmt, ...)
{
    எனில் (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    க்கு (;;)
        hlt();
}

வெறுமை
__dprintf(மாறிலி எழுத்து *fmt, ...)
{
    எனில் (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        கட்டமைப்பு thread_info *cur = getCurThread();
        எனில் (cur != &MainThread) {
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

வெறுமை
printf(மாறிலி எழுத்து *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    எனில் (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

கட்டமைப்பு snprintfinfo {
    கட்டமைப்பு putcinfo info;
    எழுத்து *str, *end;
};

நிலையான வெறுமை
putc_str(கட்டமைப்பு putcinfo *info, எழுத்து c)
{
    கட்டமைப்பு snprintfinfo *sinfo = container_of(info, கட்டமைப்பு snprintfinfo, info);
    எனில் (sinfo->str >= sinfo->end)
        திரும்பு;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
முழு
snprintf(எழுத்து *str, size_t size, மாறிலி எழுத்து *fmt, ...)
{
    ASSERT32FLAT();
    எனில் (!size)
        திரும்பு 0;
    கட்டமைப்பு snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    எழுத்து *end = sinfo.str;
    எனில் (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    திரும்பு end - str;
}

// Build a formatted string - malloc'ing the memory.
எழுத்து *
znprintf(size_t size, மாறிலி எழுத்து *fmt, ...)
{
    ASSERT32FLAT();
    எனில் (!size)
        திரும்பு வெற்று;
    எழுத்து *str = malloc_tmp(size);
    எனில் (!str) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    கட்டமைப்பு snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    எழுத்து *end = sinfo.str;
    எனில் (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    திரும்பு str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

வெறுமை
hexdump(மாறிலி வெறுமை *d, முழு len)
{
    முழு மொத்தம்=0;
    போது (len > 0) {
        எனில் (மொத்தம் % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, மொத்தம்*4, 8, 0);
            putc(&debuginfo, ':');
        } இல்லையெனில் {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        மொத்தம்++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

நிலையான வெறுமை
dump_regs(கட்டமைப்பு bregs *regs)
{
    எனில் (!regs) {
        dprintf(1, "  NULL\n");
        திரும்பு;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
வெறுமை
__debug_isr(மாறிலி எழுத்து *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
வெறுமை
__debug_enter(கட்டமைப்பு bregs *regs, மாறிலி எழுத்து *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
வெறுமை
__debug_stub(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
வெறுமை
__warn_invalid(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname)
{
    எனில் (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
வெறுமை
__warn_unimplemented(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname)
{
    எனில் (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
வெறுமை
__warn_internalerror(முழு lineno, மாறிலி எழுத்து *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
வெறுமை
__warn_noalloc(முழு lineno, மாறிலி எழுத்து *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
வெறுமை
__warn_timeout(முழு lineno, மாறிலி எழுத்து *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
வெறுமை
__set_invalid(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
வெறுமை
__set_unimplemented(கட்டமைப்பு bregs *regs, முழு lineno, மாறிலி எழுத்து *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
வெறுமை
__set_code_invalid(கட்டமைப்பு bregs *regs, u32 linecode, மாறிலி எழுத்து *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
வெறுமை
__set_code_unimplemented(கட்டமைப்பு bregs *regs, u32 linecode, மாறிலி எழுத்து *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
