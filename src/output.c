// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश <stdarg.h> // va_list

#समावेश "farptr.h" // GET_VAR
#समावेश "bregs.h" // struct bregs
#समावेश "config.h" // CONFIG_*
#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "hw/pci.h" // pci_bdf_to_bus
#समावेश "hw/pcidevice.h" // pci_device
#समावेश "hw/serialio.h" // serial_debug_putc
#समावेश "malloc.h" // malloc_tmp
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // call16_int
#समावेश "string.h" // memset
#समावेश "util.h" // ScreenAndDebug

काष्ठा putcinfo {
    व्योम (*func)(काष्ठा putcinfo *info, अक्षर c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

व्योम
debug_banner(व्योम)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
अटल व्योम
debug_putc(काष्ठा putcinfo *action, अक्षर c)
{
    अगर (! CONFIG_DEBUG_LEVEL)
        वापस;
    qemu_debug_putc(c);
    अगर (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
अटल व्योम
debug_flush(व्योम)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#अगर MODE16
अटल काष्ठा putcinfo debuginfo VAR16;
#या_अगर MODESEGMENT
अटल काष्ठा putcinfo debuginfo VAR32SEG;
#अन्यथा
अटल काष्ठा putcinfo debuginfo = { debug_putc };
#पूर्ण_अगर


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
अटल व्योम
screenc(अक्षर c)
{
    अगर (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        वापस;
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
अटल व्योम
screen_putc(काष्ठा putcinfo *action, अक्षर c)
{
    अगर (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    अगर (c == '\n')
        screenc('\r');
    screenc(c);
}

अटल काष्ठा putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
अटल व्योम
अ_दो(काष्ठा putcinfo *action, अक्षर c)
{
    अगर (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        वापस;
    }

    व्योम (*func)(काष्ठा putcinfo *info, अक्षर c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
अटल व्योम
माला_दो(काष्ठा putcinfo *action, स्थिर अक्षर *s)
{
    अगर (!MODESEGMENT && !s)
        s = "(NULL)";
    क्रम (; *s; s++)
        अ_दो(action, *s);
}

// Output a string that is in the CS segment.
अटल व्योम
puts_cs(काष्ठा putcinfo *action, स्थिर अक्षर *s)
{
    अक्षर *vs = (अक्षर*)s;
    क्रम (;; vs++) {
        अक्षर c = GET_GLOBAL(*vs);
        अगर (!c)
            अवरोध;
        अ_दो(action, c);
    }
}

// Output an unsigned integer.
अटल व्योम
putuint(काष्ठा putcinfo *action, u32 val)
{
    अक्षर buf[12];
    अक्षर *d = &buf[माप(buf) - 1];
    *d-- = '\0';
    क्रम (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        अगर (!val)
            अवरोध;
        d--;
    }
    माला_दो(action, d);
}

// Output a single digit hex character.
अटल अंतरभूत व्योम
putsinglehex(काष्ठा putcinfo *action, u32 val, पूर्णांक uc)
{
    अगर (val <= 9)
        val = '0' + val;
    अन्यथा अगर (uc)
        val = 'A' + val - 10;
    अन्यथा
        val = 'a' + val - 10;
    अ_दो(action, val);
}

// Output an integer in hexadecimal with a specified width.
अटल व्योम
puthex(काष्ठा putcinfo *action, u32 val, पूर्णांक width, पूर्णांक uc)
{
    चयन (width) {
    शेष: putsinglehex(action, (val >> 28) & 0xf, uc);
    हाल 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    हाल 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    हाल 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    हाल 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    हाल 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    हाल 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    हाल 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
अटल व्योम
putprettyhex(काष्ठा putcinfo *action, u32 val, पूर्णांक width, अक्षर padchar, पूर्णांक uc)
{
    u32 tmp = val;
    पूर्णांक गिनती = 1;
    जबतक (tmp >>= 4)
        गिनती++;
    width -= गिनती;
    जबतक (width-- > 0)
        अ_दो(action, padchar);
    puthex(action, val, गिनती, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
अटल व्योम
put_pci_device(काष्ठा putcinfo *action, काष्ठा pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    अ_दो(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    अ_दो(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

अटल अंतरभूत पूर्णांक
है_अंक(u8 c)
{
    वापस ((u8)(c - '0')) < 10;
}

अटल व्योम
bvprintf(काष्ठा putcinfo *action, स्थिर अक्षर *fmt, बहु_सूची args)
{
    स्थिर अक्षर *s = fmt;
    पूर्णांक uc;
    क्रम (;; s++) {
        अक्षर c = GET_GLOBAL(*(u8*)s);
        अगर (!c)
            अवरोध;
        अगर (c != '%') {
            अ_दो(action, c);
            जारी;
        }
        स्थिर अक्षर *n = s+1;
        पूर्णांक field_width = 0;
        अक्षर padchar = ' ';
        u8 is64 = 0;
        क्रम (;;) {
            c = GET_GLOBAL(*(u8*)n);
            अगर (!है_अंक(c))
                अवरोध;
            अगर (!field_width && (c == '0'))
                padchar = '0';
            अन्यथा
                field_width = field_width * 10 + c - '0';
            n++;
        }
        अगर (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        अगर (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        स्थिर अक्षर *sarg;
        चयन (c) {
        हाल '%':
            अ_दो(action, '%');
            अवरोध;
        हाल 'd':
            val = बहु_तर्क(args, s32);
            अगर (is64)
                बहु_तर्क(args, s32);
            अगर (val < 0) {
                अ_दो(action, '-');
                val = -val;
            }
            putuint(action, val);
            अवरोध;
        हाल 'u':
            val = बहु_तर्क(args, s32);
            अगर (is64)
                बहु_तर्क(args, s32);
            putuint(action, val);
            अवरोध;
        हाल 'p':
            val = बहु_तर्क(args, s32);
            अगर (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (व्योम*)val);
                n++;
                अवरोध;
            }
            अ_दो(action, '0');
            अ_दो(action, 'x');
            puthex(action, val, 8, 0);
            अवरोध;
        हाल 'X':
        हाल 'x':
            uc = (c == 'X');
            val = बहु_तर्क(args, s32);
            अगर (is64) {
                u32 upper = बहु_तर्क(args, s32);
                अगर (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    अवरोध;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            अवरोध;
        हाल 'c':
            val = बहु_तर्क(args, पूर्णांक);
            अ_दो(action, val);
            अवरोध;
        हाल '.':
            // Hack to support "%.s" - meaning string on stack.
            अगर (GET_GLOBAL(*(u8*)(n+1)) != 's')
                अवरोध;
            n++;
            sarg = बहु_तर्क(args, स्थिर अक्षर *);
            माला_दो(action, sarg);
            अवरोध;
        हाल 's':
            sarg = बहु_तर्क(args, स्थिर अक्षर *);
            puts_cs(action, sarg);
            अवरोध;
        शेष:
            अ_दो(action, '%');
            n = s;
        }
        s = n;
    }
}

व्योम
panic(स्थिर अक्षर *fmt, ...)
{
    अगर (CONFIG_DEBUG_LEVEL) {
        बहु_सूची args;
        बहु_शुरू(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        बहु_पूर्ण(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    क्रम (;;)
        hlt();
}

व्योम
__dprintf(स्थिर अक्षर *fmt, ...)
{
    अगर (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        काष्ठा thread_info *cur = getCurThread();
        अगर (cur != &MainThread) {
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

व्योम
म_लिखो(स्थिर अक्षर *fmt, ...)
{
    ASSERT32FLAT();
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    बहु_पूर्ण(args);
    अगर (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

काष्ठा snprintfinfo {
    काष्ठा putcinfo info;
    अक्षर *str, *end;
};

अटल व्योम
putc_str(काष्ठा putcinfo *info, अक्षर c)
{
    काष्ठा snprintfinfo *sinfo = container_of(info, काष्ठा snprintfinfo, info);
    अगर (sinfo->str >= sinfo->end)
        वापस;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
पूर्णांक
snprintf(अक्षर *str, माप_प्रकार आकार, स्थिर अक्षर *fmt, ...)
{
    ASSERT32FLAT();
    अगर (!आकार)
        वापस 0;
    काष्ठा snprintfinfo sinfo = { { putc_str }, str, str + आकार };
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    बहु_पूर्ण(args);
    अक्षर *end = sinfo.str;
    अगर (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    वापस end - str;
}

// Build a formatted string - malloc'ing the memory.
अक्षर *
znprintf(माप_प्रकार आकार, स्थिर अक्षर *fmt, ...)
{
    ASSERT32FLAT();
    अगर (!आकार)
        वापस शून्य;
    अक्षर *str = malloc_tmp(आकार);
    अगर (!str) {
        warn_noalloc();
        वापस शून्य;
    }
    काष्ठा snprintfinfo sinfo = { { putc_str }, str, str + आकार };
    बहु_सूची args;
    बहु_शुरू(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    बहु_पूर्ण(args);
    अक्षर *end = sinfo.str;
    अगर (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    वापस str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

व्योम
hexdump(स्थिर व्योम *d, पूर्णांक len)
{
    पूर्णांक गिनती=0;
    जबतक (len > 0) {
        अगर (गिनती % 8 == 0) {
            अ_दो(&debuginfo, '\n');
            puthex(&debuginfo, गिनती*4, 8, 0);
            अ_दो(&debuginfo, ':');
        } अन्यथा {
            अ_दो(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        गिनती++;
        len-=4;
        d+=4;
    }
    अ_दो(&debuginfo, '\n');
    debug_flush();
}

अटल व्योम
dump_regs(काष्ठा bregs *regs)
{
    अगर (!regs) {
        dprintf(1, "  NULL\n");
        वापस;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
व्योम
__debug_isr(स्थिर अक्षर *fname)
{
    puts_cs(&debuginfo, fname);
    अ_दो(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
व्योम
__debug_enter(काष्ठा bregs *regs, स्थिर अक्षर *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
व्योम
__debug_stub(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
व्योम
__warn_invalid(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname)
{
    अगर (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
व्योम
__warn_unimplemented(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname)
{
    अगर (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
व्योम
__warn_internalerror(पूर्णांक lineno, स्थिर अक्षर *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
व्योम
__warn_noalloc(पूर्णांक lineno, स्थिर अक्षर *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
व्योम
__warn_timeout(पूर्णांक lineno, स्थिर अक्षर *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
व्योम
__set_invalid(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
व्योम
__set_unimplemented(काष्ठा bregs *regs, पूर्णांक lineno, स्थिर अक्षर *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
व्योम
__set_code_invalid(काष्ठा bregs *regs, u32 linecode, स्थिर अक्षर *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
व्योम
__set_code_unimplemented(काष्ठा bregs *regs, u32 linecode, स्थिर अक्षर *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
