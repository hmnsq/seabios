// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव <stdarg.h> // va_list

#अन्तर्भाव "farptr.h" // GET_VAR
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "hw/pci.h" // pci_bdf_to_bus
#अन्तर्भाव "hw/pcidevice.h" // pci_device
#अन्तर्भाव "hw/serialio.h" // serial_debug_putc
#अन्तर्भाव "malloc.h" // malloc_tmp
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // call16_int
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // ScreenAndDebug

संरचना putcinfo {
    शून्य (*func)(संरचना putcinfo *info, वर्ण c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

शून्य
debug_banner(शून्य)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
स्थिर शून्य
debug_putc(संरचना putcinfo *action, वर्ण c)
{
    यदि (! CONFIG_DEBUG_LEVEL)
        प्रत्यागम;
    qemu_debug_putc(c);
    यदि (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
स्थिर शून्य
debug_flush(शून्य)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#यदि MODE16
स्थिर संरचना putcinfo debuginfo VAR16;
#नोचेद्यदि MODESEGMENT
स्थिर संरचना putcinfo debuginfo VAR32SEG;
#अन्यथा
स्थिर संरचना putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
स्थिर शून्य
screenc(वर्ण c)
{
    यदि (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        प्रत्यागम;
    संरचना bregs br;
    memset(&br, 0, परिमाण(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
स्थिर शून्य
screen_putc(संरचना putcinfo *action, वर्ण c)
{
    यदि (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    यदि (c == '\n')
        screenc('\r');
    screenc(c);
}

स्थिर संरचना putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
स्थिर शून्य
putc(संरचना putcinfo *action, वर्ण c)
{
    यदि (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        प्रत्यागम;
    }

    शून्य (*func)(संरचना putcinfo *info, वर्ण c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
स्थिर शून्य
puts(संरचना putcinfo *action, ध्रुव वर्ण *s)
{
    यदि (!MODESEGMENT && !s)
        s = "(NULL)";
    प्रति (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
स्थिर शून्य
puts_cs(संरचना putcinfo *action, ध्रुव वर्ण *s)
{
    वर्ण *vs = (वर्ण*)s;
    प्रति (;; vs++) {
        वर्ण c = GET_GLOBAL(*vs);
        यदि (!c)
            भङ्ग;
        putc(action, c);
    }
}

// Output an unsigned integer.
स्थिर शून्य
putuint(संरचना putcinfo *action, u32 val)
{
    वर्ण buf[12];
    वर्ण *d = &buf[परिमाण(buf) - 1];
    *d-- = '\0';
    प्रति (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        यदि (!val)
            भङ्ग;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
स्थिर अन्तःस्थ शून्य
putsinglehex(संरचना putcinfo *action, u32 val, पूर्णाङ्क uc)
{
    यदि (val <= 9)
        val = '0' + val;
    अन्यथा यदि (uc)
        val = 'A' + val - 10;
    अन्यथा
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
स्थिर शून्य
puthex(संरचना putcinfo *action, u32 val, पूर्णाङ्क width, पूर्णाङ्क uc)
{
    विकल्प (width) {
    स्वभाव: putsinglehex(action, (val >> 28) & 0xf, uc);
    पक्ष 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    पक्ष 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    पक्ष 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    पक्ष 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    पक्ष 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    पक्ष 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    पक्ष 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
स्थिर शून्य
putprettyhex(संरचना putcinfo *action, u32 val, पूर्णाङ्क width, वर्ण padchar, पूर्णाङ्क uc)
{
    u32 tmp = val;
    पूर्णाङ्क मोजणी = 1;
    यावत् (tmp >>= 4)
        मोजणी++;
    width -= मोजणी;
    यावत् (width-- > 0)
        putc(action, padchar);
    puthex(action, val, मोजणी, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
स्थिर शून्य
put_pci_device(संरचना putcinfo *action, संरचना pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

स्थिर अन्तःस्थ पूर्णाङ्क
isdigit(u8 c)
{
    प्रत्यागम ((u8)(c - '0')) < 10;
}

स्थिर शून्य
bvprintf(संरचना putcinfo *action, ध्रुव वर्ण *fmt, va_list args)
{
    ध्रुव वर्ण *s = fmt;
    पूर्णाङ्क uc;
    प्रति (;; s++) {
        वर्ण c = GET_GLOBAL(*(u8*)s);
        यदि (!c)
            भङ्ग;
        यदि (c != '%') {
            putc(action, c);
            अनुवर्त;
        }
        ध्रुव वर्ण *n = s+1;
        पूर्णाङ्क field_width = 0;
        वर्ण padchar = ' ';
        u8 is64 = 0;
        प्रति (;;) {
            c = GET_GLOBAL(*(u8*)n);
            यदि (!isdigit(c))
                भङ्ग;
            यदि (!field_width && (c == '0'))
                padchar = '0';
            अन्यथा
                field_width = field_width * 10 + c - '0';
            n++;
        }
        यदि (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        यदि (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ध्रुव वर्ण *sarg;
        विकल्प (c) {
        पक्ष '%':
            putc(action, '%');
            भङ्ग;
        पक्ष 'd':
            val = va_arg(args, s32);
            यदि (is64)
                va_arg(args, s32);
            यदि (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            भङ्ग;
        पक्ष 'u':
            val = va_arg(args, s32);
            यदि (is64)
                va_arg(args, s32);
            putuint(action, val);
            भङ्ग;
        पक्ष 'p':
            val = va_arg(args, s32);
            यदि (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (शून्य*)val);
                n++;
                भङ्ग;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            भङ्ग;
        पक्ष 'X':
        पक्ष 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            यदि (is64) {
                u32 upper = va_arg(args, s32);
                यदि (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    भङ्ग;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            भङ्ग;
        पक्ष 'c':
            val = va_arg(args, पूर्णाङ्क);
            putc(action, val);
            भङ्ग;
        पक्ष '.':
            // Hack to support "%.s" - meaning string on stack.
            यदि (GET_GLOBAL(*(u8*)(n+1)) != 's')
                भङ्ग;
            n++;
            sarg = va_arg(args, ध्रुव वर्ण *);
            puts(action, sarg);
            भङ्ग;
        पक्ष 's':
            sarg = va_arg(args, ध्रुव वर्ण *);
            puts_cs(action, sarg);
            भङ्ग;
        स्वभाव:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

शून्य
panic(ध्रुव वर्ण *fmt, ...)
{
    यदि (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    प्रति (;;)
        hlt();
}

शून्य
__dprintf(ध्रुव वर्ण *fmt, ...)
{
    यदि (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        संरचना thread_info *cur = getCurThread();
        यदि (cur != &MainThread) {
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

शून्य
printf(ध्रुव वर्ण *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    यदि (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

संरचना snprintfinfo {
    संरचना putcinfo info;
    वर्ण *str, *end;
};

स्थिर शून्य
putc_str(संरचना putcinfo *info, वर्ण c)
{
    संरचना snprintfinfo *sinfo = container_of(info, संरचना snprintfinfo, info);
    यदि (sinfo->str >= sinfo->end)
        प्रत्यागम;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
पूर्णाङ्क
snprintf(वर्ण *str, size_t size, ध्रुव वर्ण *fmt, ...)
{
    ASSERT32FLAT();
    यदि (!size)
        प्रत्यागम 0;
    संरचना snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    वर्ण *end = sinfo.str;
    यदि (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    प्रत्यागम end - str;
}

// Build a formatted string - malloc'ing the memory.
वर्ण *
znprintf(size_t size, ध्रुव वर्ण *fmt, ...)
{
    ASSERT32FLAT();
    यदि (!size)
        प्रत्यागम रिक्त;
    वर्ण *str = malloc_tmp(size);
    यदि (!str) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    संरचना snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    वर्ण *end = sinfo.str;
    यदि (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    प्रत्यागम str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

शून्य
hexdump(ध्रुव शून्य *d, पूर्णाङ्क len)
{
    पूर्णाङ्क मोजणी=0;
    यावत् (len > 0) {
        यदि (मोजणी % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, मोजणी*4, 8, 0);
            putc(&debuginfo, ':');
        } अन्यथा {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        मोजणी++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

स्थिर शून्य
dump_regs(संरचना bregs *regs)
{
    यदि (!regs) {
        dprintf(1, "  NULL\n");
        प्रत्यागम;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
शून्य
__debug_isr(ध्रुव वर्ण *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
शून्य
__debug_enter(संरचना bregs *regs, ध्रुव वर्ण *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
शून्य
__debug_stub(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
शून्य
__warn_invalid(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    यदि (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
शून्य
__warn_unimplemented(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    यदि (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
शून्य
__warn_internalerror(पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
शून्य
__warn_noalloc(पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
शून्य
__warn_timeout(पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
शून्य
__set_invalid(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
शून्य
__set_unimplemented(संरचना bregs *regs, पूर्णाङ्क lineno, ध्रुव वर्ण *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
शून्य
__set_code_invalid(संरचना bregs *regs, u32 linecode, ध्रुव वर्ण *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
शून्य
__set_code_unimplemented(संरचना bregs *regs, u32 linecode, ध्रुव वर्ण *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
