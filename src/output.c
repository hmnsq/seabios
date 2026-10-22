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
    शून्य (*func)(संरचना putcinfo *info, अक्षर c);
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
debug_putc(संरचना putcinfo *action, अक्षर c)
{
    जर (! CONFIG_DEBUG_LEVEL)
        परत;
    qemu_debug_putc(c);
    जर (!MODESEGMENT)
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
#जर MODE16
स्थिर संरचना putcinfo debuginfo VAR16;
#किंवाजर MODESEGMENT
स्थिर संरचना putcinfo debuginfo VAR32SEG;
#अन्यथा
स्थिर संरचना putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
स्थिर शून्य
screenc(अक्षर c)
{
    जर (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        परत;
    संरचना bregs br;
    स_रखो(&br, 0, आकार(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
स्थिर शून्य
screen_putc(संरचना putcinfo *action, अक्षर c)
{
    जर (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    जर (c == '\n')
        screenc('\r');
    screenc(c);
}

स्थिर संरचना putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
स्थिर शून्य
अ_दो(संरचना putcinfo *action, अक्षर c)
{
    जर (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        परत;
    }

    शून्य (*func)(संरचना putcinfo *info, अक्षर c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
स्थिर शून्य
माला_दो(संरचना putcinfo *action, स्थिरांक अक्षर *s)
{
    जर (!MODESEGMENT && !s)
        s = "(NULL)";
    साठी (; *s; s++)
        अ_दो(action, *s);
}

// Output a string that is in the CS segment.
स्थिर शून्य
puts_cs(संरचना putcinfo *action, स्थिरांक अक्षर *s)
{
    अक्षर *vs = (अक्षर*)s;
    साठी (;; vs++) {
        अक्षर c = GET_GLOBAL(*vs);
        जर (!c)
            थांबा;
        अ_दो(action, c);
    }
}

// Output an unsigned integer.
स्थिर शून्य
putuint(संरचना putcinfo *action, u32 val)
{
    अक्षर buf[12];
    अक्षर *d = &buf[आकार(buf) - 1];
    *d-- = '\0';
    साठी (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        जर (!val)
            थांबा;
        d--;
    }
    माला_दो(action, d);
}

// Output a single digit hex character.
स्थिर ओळीत शून्य
putsinglehex(संरचना putcinfo *action, u32 val, पूर्णांक uc)
{
    जर (val <= 9)
        val = '0' + val;
    अन्यथा जर (uc)
        val = 'A' + val - 10;
    अन्यथा
        val = 'a' + val - 10;
    अ_दो(action, val);
}

// Output an integer in hexadecimal with a specified width.
स्थिर शून्य
puthex(संरचना putcinfo *action, u32 val, पूर्णांक width, पूर्णांक uc)
{
    निवड (width) {
    पूर्वनिर्धारित: putsinglehex(action, (val >> 28) & 0xf, uc);
    स्थिती 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    स्थिती 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    स्थिती 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    स्थिती 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    स्थिती 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    स्थिती 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    स्थिती 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
स्थिर शून्य
putprettyhex(संरचना putcinfo *action, u32 val, पूर्णांक width, अक्षर padchar, पूर्णांक uc)
{
    u32 tmp = val;
    पूर्णांक मोजणी = 1;
    पर्यंत (tmp >>= 4)
        मोजणी++;
    width -= मोजणी;
    पर्यंत (width-- > 0)
        अ_दो(action, padchar);
    puthex(action, val, मोजणी, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
स्थिर शून्य
put_pci_device(संरचना putcinfo *action, संरचना pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    अ_दो(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    अ_दो(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

स्थिर ओळीत पूर्णांक
है_अंक(u8 c)
{
    परत ((u8)(c - '0')) < 10;
}

स्थिर शून्य
bvprintf(संरचना putcinfo *action, स्थिरांक अक्षर *fmt, बहु_सूची args)
{
    स्थिरांक अक्षर *s = fmt;
    पूर्णांक uc;
    साठी (;; s++) {
        अक्षर c = GET_GLOBAL(*(u8*)s);
        जर (!c)
            थांबा;
        जर (c != '%') {
            अ_दो(action, c);
            सुरूठेवा;
        }
        स्थिरांक अक्षर *n = s+1;
        पूर्णांक field_width = 0;
        अक्षर padchar = ' ';
        u8 is64 = 0;
        साठी (;;) {
            c = GET_GLOBAL(*(u8*)n);
            जर (!है_अंक(c))
                थांबा;
            जर (!field_width && (c == '0'))
                padchar = '0';
            अन्यथा
                field_width = field_width * 10 + c - '0';
            n++;
        }
        जर (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        जर (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        स्थिरांक अक्षर *sarg;
        निवड (c) {
        स्थिती '%':
            अ_दो(action, '%');
            थांबा;
        स्थिती 'd':
            val = बहु_तर्क(args, s32);
            जर (is64)
                बहु_तर्क(args, s32);
            जर (val < 0) {
                अ_दो(action, '-');
                val = -val;
            }
            putuint(action, val);
            थांबा;
        स्थिती 'u':
            val = बहु_तर्क(args, s32);
            जर (is64)
                बहु_तर्क(args, s32);
            putuint(action, val);
            थांबा;
        स्थिती 'p':
            val = बहु_तर्क(args, s32);
            जर (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (शून्य*)val);
                n++;
                थांबा;
            }
            अ_दो(action, '0');
            अ_दो(action, 'x');
            puthex(action, val, 8, 0);
            थांबा;
        स्थिती 'X':
        स्थिती 'x':
            uc = (c == 'X');
            val = बहु_तर्क(args, s32);
            जर (is64) {
                u32 upper = बहु_तर्क(args, s32);
                जर (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    थांबा;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            थांबा;
        स्थिती 'c':
            val = बहु_तर्क(args, पूर्णांक);
            अ_दो(action, val);
            थांबा;
        स्थिती '.':
            // Hack to support "%.s" - meaning string on stack.
            जर (GET_GLOBAL(*(u8*)(n+1)) != 's')
                थांबा;
            n++;
            sarg = बहु_तर्क(args, स्थिरांक अक्षर *);
            माला_दो(action, sarg);
            थांबा;
        स्थिती 's':
            sarg = बहु_तर्क(args, स्थिरांक अक्षर *);
            puts_cs(action, sarg);
            थांबा;
        पूर्वनिर्धारित:
            अ_दो(action, '%');
            n = s;
        }
        s = n;
    }
}

शून्य
panic(स्थिरांक अक्षर *fmt, ...)
{
    जर (CONFIG_DEBUG_LEVEL) {
        बहु_सूची args;
        बहु_शुरू(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        बहु_पूर्ण(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    साठी (;;)
        hlt();
}

शून्य
__dprintf(स्थिरांक अक्षर *fmt, ...)
{
    जर (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        संरचना thread_info *cur = getCurThread();
        जर (cur != &MainThread) {
            // Show "thread id" for this debug message.
            debug_putc(&debuginfo, '|');
            puthex(&debuginfo, (u32)cur, 8, 0);
            debug_putc(&debuginfo, '|');
            debug_putc(&debuginfo, ' ');
        }
    }

    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&debuginfo, fmt, args);
    बहु_पूर्ण(args);
    debug_flush();
}

शून्य
म_लिखो(स्थिरांक अक्षर *fmt, ...)
{
    ASSERT32FLAT();
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    बहु_पूर्ण(args);
    जर (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

संरचना snprintfinfo {
    संरचना putcinfo info;
    अक्षर *str, *end;
};

स्थिर शून्य
putc_str(संरचना putcinfo *info, अक्षर c)
{
    संरचना snprintfinfo *sinfo = container_of(info, संरचना snprintfinfo, info);
    जर (sinfo->str >= sinfo->end)
        परत;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
पूर्णांक
snprintf(अक्षर *str, माप_प्रकार size, स्थिरांक अक्षर *fmt, ...)
{
    ASSERT32FLAT();
    जर (!size)
        परत 0;
    संरचना snprintfinfo sinfo = { { putc_str }, str, str + size };
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    बहु_पूर्ण(args);
    अक्षर *end = sinfo.str;
    जर (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    परत end - str;
}

// Build a formatted string - malloc'ing the memory.
अक्षर *
znprintf(माप_प्रकार size, स्थिरांक अक्षर *fmt, ...)
{
    ASSERT32FLAT();
    जर (!size)
        परत रिक्त;
    अक्षर *str = malloc_tmp(size);
    जर (!str) {
        warn_noalloc();
        परत रिक्त;
    }
    संरचना snprintfinfo sinfo = { { putc_str }, str, str + size };
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    बहु_पूर्ण(args);
    अक्षर *end = sinfo.str;
    जर (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    परत str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

शून्य
hexdump(स्थिरांक शून्य *d, पूर्णांक len)
{
    पूर्णांक मोजणी=0;
    पर्यंत (len > 0) {
        जर (मोजणी % 8 == 0) {
            अ_दो(&debuginfo, '\n');
            puthex(&debuginfo, मोजणी*4, 8, 0);
            अ_दो(&debuginfo, ':');
        } अन्यथा {
            अ_दो(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        मोजणी++;
        len-=4;
        d+=4;
    }
    अ_दो(&debuginfo, '\n');
    debug_flush();
}

स्थिर शून्य
dump_regs(संरचना bregs *regs)
{
    जर (!regs) {
        dprintf(1, "  NULL\n");
        परत;
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
__debug_isr(स्थिरांक अक्षर *fname)
{
    puts_cs(&debuginfo, fname);
    अ_दो(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
शून्य
__debug_enter(संरचना bregs *regs, स्थिरांक अक्षर *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
शून्य
__debug_stub(संरचना bregs *regs, पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
शून्य
__warn_invalid(संरचना bregs *regs, पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    जर (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
शून्य
__warn_unimplemented(संरचना bregs *regs, पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    जर (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
शून्य
__warn_internalerror(पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
शून्य
__warn_noalloc(पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
शून्य
__warn_timeout(पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
शून्य
__set_invalid(संरचना bregs *regs, पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
शून्य
__set_unimplemented(संरचना bregs *regs, पूर्णांक lineno, स्थिरांक अक्षर *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
शून्य
__set_code_invalid(संरचना bregs *regs, u32 linecode, स्थिरांक अक्षर *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
शून्य
__set_code_unimplemented(संरचना bregs *regs, u32 linecode, स्थिरांक अक्षर *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
