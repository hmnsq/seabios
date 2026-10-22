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
        फर्काऊ;
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
#अथवायदि MODESEGMENT
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
        फर्काऊ;
    संरचना bregs br;
    स_रखो(&br, 0, परिमाण(br));
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
अ_दो(संरचना putcinfo *action, वर्ण c)
{
    यदि (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        फर्काऊ;
    }

    शून्य (*func)(संरचना putcinfo *info, वर्ण c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
स्थिर शून्य
माला_दो(संरचना putcinfo *action, अचर वर्ण *s)
{
    यदि (!MODESEGMENT && !s)
        s = "(NULL)";
    लागि (; *s; s++)
        अ_दो(action, *s);
}

// Output a string that is in the CS segment.
स्थिर शून्य
puts_cs(संरचना putcinfo *action, अचर वर्ण *s)
{
    वर्ण *vs = (वर्ण*)s;
    लागि (;; vs++) {
        वर्ण c = GET_GLOBAL(*vs);
        यदि (!c)
            रोक;
        अ_दो(action, c);
    }
}

// Output an unsigned integer.
स्थिर शून्य
putuint(संरचना putcinfo *action, u32 val)
{
    वर्ण buf[12];
    वर्ण *d = &buf[परिमाण(buf) - 1];
    *d-- = '\0';
    लागि (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        यदि (!val)
            रोक;
        d--;
    }
    माला_दो(action, d);
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
    अ_दो(action, val);
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
    पूर्णाङ्क गन्ती = 1;
    जबसम्म (tmp >>= 4)
        गन्ती++;
    width -= गन्ती;
    जबसम्म (width-- > 0)
        अ_दो(action, padchar);
    puthex(action, val, गन्ती, uc);
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

स्थिर अन्तःस्थ पूर्णाङ्क
है_अंक(u8 c)
{
    फर्काऊ ((u8)(c - '0')) < 10;
}

स्थिर शून्य
bvprintf(संरचना putcinfo *action, अचर वर्ण *fmt, बहु_सूची args)
{
    अचर वर्ण *s = fmt;
    पूर्णाङ्क uc;
    लागि (;; s++) {
        वर्ण c = GET_GLOBAL(*(u8*)s);
        यदि (!c)
            रोक;
        यदि (c != '%') {
            अ_दो(action, c);
            जारी;
        }
        अचर वर्ण *n = s+1;
        पूर्णाङ्क field_width = 0;
        वर्ण padchar = ' ';
        u8 is64 = 0;
        लागि (;;) {
            c = GET_GLOBAL(*(u8*)n);
            यदि (!है_अंक(c))
                रोक;
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
        अचर वर्ण *sarg;
        विकल्प (c) {
        पक्ष '%':
            अ_दो(action, '%');
            रोक;
        पक्ष 'd':
            val = बहु_तर्क(args, s32);
            यदि (is64)
                बहु_तर्क(args, s32);
            यदि (val < 0) {
                अ_दो(action, '-');
                val = -val;
            }
            putuint(action, val);
            रोक;
        पक्ष 'u':
            val = बहु_तर्क(args, s32);
            यदि (is64)
                बहु_तर्क(args, s32);
            putuint(action, val);
            रोक;
        पक्ष 'p':
            val = बहु_तर्क(args, s32);
            यदि (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (शून्य*)val);
                n++;
                रोक;
            }
            अ_दो(action, '0');
            अ_दो(action, 'x');
            puthex(action, val, 8, 0);
            रोक;
        पक्ष 'X':
        पक्ष 'x':
            uc = (c == 'X');
            val = बहु_तर्क(args, s32);
            यदि (is64) {
                u32 upper = बहु_तर्क(args, s32);
                यदि (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    रोक;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            रोक;
        पक्ष 'c':
            val = बहु_तर्क(args, पूर्णाङ्क);
            अ_दो(action, val);
            रोक;
        पक्ष '.':
            // Hack to support "%.s" - meaning string on stack.
            यदि (GET_GLOBAL(*(u8*)(n+1)) != 's')
                रोक;
            n++;
            sarg = बहु_तर्क(args, अचर वर्ण *);
            माला_दो(action, sarg);
            रोक;
        पक्ष 's':
            sarg = बहु_तर्क(args, अचर वर्ण *);
            puts_cs(action, sarg);
            रोक;
        स्वभाव:
            अ_दो(action, '%');
            n = s;
        }
        s = n;
    }
}

शून्य
panic(अचर वर्ण *fmt, ...)
{
    यदि (CONFIG_DEBUG_LEVEL) {
        बहु_सूची args;
        बहु_शुरू(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        बहु_पूर्ण(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    लागि (;;)
        hlt();
}

शून्य
__dprintf(अचर वर्ण *fmt, ...)
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

    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&debuginfo, fmt, args);
    बहु_पूर्ण(args);
    debug_flush();
}

शून्य
म_लिखो(अचर वर्ण *fmt, ...)
{
    ASSERT32FLAT();
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    बहु_पूर्ण(args);
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
        फर्काऊ;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
पूर्णाङ्क
snprintf(वर्ण *str, माप_प्रकार size, अचर वर्ण *fmt, ...)
{
    ASSERT32FLAT();
    यदि (!size)
        फर्काऊ 0;
    संरचना snprintfinfo sinfo = { { putc_str }, str, str + size };
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    बहु_पूर्ण(args);
    वर्ण *end = sinfo.str;
    यदि (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    फर्काऊ end - str;
}

// Build a formatted string - malloc'ing the memory.
वर्ण *
znprintf(माप_प्रकार size, अचर वर्ण *fmt, ...)
{
    ASSERT32FLAT();
    यदि (!size)
        फर्काऊ रिक्त;
    वर्ण *str = malloc_tmp(size);
    यदि (!str) {
        warn_noalloc();
        फर्काऊ रिक्त;
    }
    संरचना snprintfinfo sinfo = { { putc_str }, str, str + size };
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    बहु_पूर्ण(args);
    वर्ण *end = sinfo.str;
    यदि (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    फर्काऊ str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

शून्य
hexdump(अचर शून्य *d, पूर्णाङ्क len)
{
    पूर्णाङ्क गन्ती=0;
    जबसम्म (len > 0) {
        यदि (गन्ती % 8 == 0) {
            अ_दो(&debuginfo, '\n');
            puthex(&debuginfo, गन्ती*4, 8, 0);
            अ_दो(&debuginfo, ':');
        } अन्यथा {
            अ_दो(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        गन्ती++;
        len-=4;
        d+=4;
    }
    अ_दो(&debuginfo, '\n');
    debug_flush();
}

स्थिर शून्य
dump_regs(संरचना bregs *regs)
{
    यदि (!regs) {
        dprintf(1, "  NULL\n");
        फर्काऊ;
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
__debug_isr(अचर वर्ण *fname)
{
    puts_cs(&debuginfo, fname);
    अ_दो(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
शून्य
__debug_enter(संरचना bregs *regs, अचर वर्ण *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
शून्य
__debug_stub(संरचना bregs *regs, पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
शून्य
__warn_invalid(संरचना bregs *regs, पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    यदि (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
शून्य
__warn_unimplemented(संरचना bregs *regs, पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    यदि (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
शून्य
__warn_internalerror(पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
शून्य
__warn_noalloc(पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
शून्य
__warn_timeout(पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
शून्य
__set_invalid(संरचना bregs *regs, पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
शून्य
__set_unimplemented(संरचना bregs *regs, पूर्णाङ्क lineno, अचर वर्ण *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
शून्य
__set_code_invalid(संरचना bregs *regs, u32 linecode, अचर वर्ण *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
शून्य
__set_code_unimplemented(संरचना bregs *regs, u32 linecode, अचर वर्ण *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
