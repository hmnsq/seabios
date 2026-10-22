// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ <stdarg.h> // va_list

#అన్తర్భావ "farptr.h" // GET_VAR
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "hw/pci.h" // pci_bdf_to_bus
#అన్తర్భావ "hw/pcidevice.h" // pci_device
#అన్తర్భావ "hw/serialio.h" // serial_debug_putc
#అన్తర్భావ "malloc.h" // malloc_tmp
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // call16_int
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // ScreenAndDebug

నిర్మాణం putcinfo {
    శూన్య (*func)(నిర్మాణం putcinfo *info, అక్షరం c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

శూన్య
debug_banner(శూన్య)
{
    dprintf(1, "SeaBIOS (version %s)\n", సంస్కరణ);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
స్థిర శూన్య
debug_putc(నిర్మాణం putcinfo *action, అక్షరం c)
{
    ఒకవేళ (! CONFIG_DEBUG_LEVEL)
        తిరిగి;
    qemu_debug_putc(c);
    ఒకవేళ (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
స్థిర శూన్య
debug_flush(శూన్య)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#ఒకవేళ MODE16
స్థిర నిర్మాణం putcinfo debuginfo VAR16;
#లేదావేళ MODESEGMENT
స్థిర నిర్మాణం putcinfo debuginfo VAR32SEG;
#లేకపోతే
స్థిర నిర్మాణం putcinfo debuginfo = { debug_putc };
#అయితే_ముగింపు


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
స్థిర శూన్య
screenc(అక్షరం c)
{
    ఒకవేళ (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(నమోదు_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        తిరిగి;
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
స్థిర శూన్య
screen_putc(నిర్మాణం putcinfo *action, అక్షరం c)
{
    ఒకవేళ (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    ఒకవేళ (c == '\n')
        screenc('\r');
    screenc(c);
}

స్థిర నిర్మాణం putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
స్థిర శూన్య
putc(నిర్మాణం putcinfo *action, అక్షరం c)
{
    ఒకవేళ (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        తిరిగి;
    }

    శూన్య (*func)(నిర్మాణం putcinfo *info, అక్షరం c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
స్థిర శూన్య
puts(నిర్మాణం putcinfo *action, స్థిరరాశి అక్షరం *s)
{
    ఒకవేళ (!MODESEGMENT && !s)
        s = "(NULL)";
    కొరకు (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
స్థిర శూన్య
puts_cs(నిర్మాణం putcinfo *action, స్థిరరాశి అక్షరం *s)
{
    అక్షరం *vs = (అక్షరం*)s;
    కొరకు (;; vs++) {
        అక్షరం c = GET_GLOBAL(*vs);
        ఒకవేళ (!c)
            ఆపు;
        putc(action, c);
    }
}

// Output an unsigned integer.
స్థిర శూన్య
putuint(నిర్మాణం putcinfo *action, u32 val)
{
    అక్షరం buf[12];
    అక్షరం *d = &buf[పరిమాణం(buf) - 1];
    *d-- = '\0';
    కొరకు (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        ఒకవేళ (!val)
            ఆపు;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
స్థిర అంతఃవరుస శూన్య
putsinglehex(నిర్మాణం putcinfo *action, u32 val, పూర్ణాంకం uc)
{
    ఒకవేళ (val <= 9)
        val = '0' + val;
    లేకపోతే ఒకవేళ (uc)
        val = 'A' + val - 10;
    లేకపోతే
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
స్థిర శూన్య
puthex(నిర్మాణం putcinfo *action, u32 val, పూర్ణాంకం వెడల్పు, పూర్ణాంకం uc)
{
    ఎంపిక (వెడల్పు) {
    అప్రమేయ: putsinglehex(action, (val >> 28) & 0xf, uc);
    స్థితి 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    స్థితి 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    స్థితి 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    స్థితి 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    స్థితి 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    స్థితి 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    స్థితి 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
స్థిర శూన్య
putprettyhex(నిర్మాణం putcinfo *action, u32 val, పూర్ణాంకం వెడల్పు, అక్షరం padchar, పూర్ణాంకం uc)
{
    u32 tmp = val;
    పూర్ణాంకం లెక్క = 1;
    అయితే (tmp >>= 4)
        లెక్క++;
    వెడల్పు -= లెక్క;
    అయితే (వెడల్పు-- > 0)
        putc(action, padchar);
    puthex(action, val, లెక్క, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
స్థిర శూన్య
put_pci_device(నిర్మాణం putcinfo *action, నిర్మాణం pci_device *pci)
{
    puthex(action, pci_bdf_గమ్యం_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_గమ్యం_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_గమ్యం_fn(pci->bdf), 1, 0);
}

స్థిర అంతఃవరుస పూర్ణాంకం
అంకెనా(u8 c)
{
    తిరిగి ((u8)(c - '0')) < 10;
}

స్థిర శూన్య
bvprintf(నిర్మాణం putcinfo *action, స్థిరరాశి అక్షరం *fmt, va_list వాదనలు)
{
    స్థిరరాశి అక్షరం *s = fmt;
    పూర్ణాంకం uc;
    కొరకు (;; s++) {
        అక్షరం c = GET_GLOBAL(*(u8*)s);
        ఒకవేళ (!c)
            ఆపు;
        ఒకవేళ (c != '%') {
            putc(action, c);
            కొనసాగు;
        }
        స్థిరరాశి అక్షరం *n = s+1;
        పూర్ణాంకం క్షేత్రం_వెడల్పు = 0;
        అక్షరం padchar = ' ';
        u8 is64 = 0;
        కొరకు (;;) {
            c = GET_GLOBAL(*(u8*)n);
            ఒకవేళ (!అంకెనా(c))
                ఆపు;
            ఒకవేళ (!క్షేత్రం_వెడల్పు && (c == '0'))
                padchar = '0';
            లేకపోతే
                క్షేత్రం_వెడల్పు = క్షేత్రం_వెడల్పు * 10 + c - '0';
            n++;
        }
        ఒకవేళ (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        ఒకవేళ (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        స్థిరరాశి అక్షరం *sarg;
        ఎంపిక (c) {
        స్థితి '%':
            putc(action, '%');
            ఆపు;
        స్థితి 'd':
            val = va_arg(వాదనలు, s32);
            ఒకవేళ (is64)
                va_arg(వాదనలు, s32);
            ఒకవేళ (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            ఆపు;
        స్థితి 'u':
            val = va_arg(వాదనలు, s32);
            ఒకవేళ (is64)
                va_arg(వాదనలు, s32);
            putuint(action, val);
            ఆపు;
        స్థితి 'p':
            val = va_arg(వాదనలు, s32);
            ఒకవేళ (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (శూన్య*)val);
                n++;
                ఆపు;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            ఆపు;
        స్థితి 'X':
        స్థితి 'x':
            uc = (c == 'X');
            val = va_arg(వాదనలు, s32);
            ఒకవేళ (is64) {
                u32 పై = va_arg(వాదనలు, s32);
                ఒకవేళ (పై) {
                    putprettyhex(action, పై, క్షేత్రం_వెడల్పు - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    ఆపు;
                }
            }
            putprettyhex(action, val, క్షేత్రం_వెడల్పు, padchar, uc);
            ఆపు;
        స్థితి 'c':
            val = va_arg(వాదనలు, పూర్ణాంకం);
            putc(action, val);
            ఆపు;
        స్థితి '.':
            // Hack to support "%.s" - meaning string on stack.
            ఒకవేళ (GET_GLOBAL(*(u8*)(n+1)) != 's')
                ఆపు;
            n++;
            sarg = va_arg(వాదనలు, స్థిరరాశి అక్షరం *);
            puts(action, sarg);
            ఆపు;
        స్థితి 's':
            sarg = va_arg(వాదనలు, స్థిరరాశి అక్షరం *);
            puts_cs(action, sarg);
            ఆపు;
        అప్రమేయ:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

శూన్య
panic(స్థిరరాశి అక్షరం *fmt, ...)
{
    ఒకవేళ (CONFIG_DEBUG_LEVEL) {
        va_list వాదనలు;
        va_start(వాదనలు, fmt);
        bvprintf(&debuginfo, fmt, వాదనలు);
        va_end(వాదనలు);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    కొరకు (;;)
        hlt();
}

శూన్య
__dprintf(స్థిరరాశి అక్షరం *fmt, ...)
{
    ఒకవేళ (!MODESEGMENT && CONFIG_దారాలు && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        నిర్మాణం thread_info *cur = getCurThread();
        ఒకవేళ (cur != &MainThread) {
            // Show "thread id" for this debug message.
            debug_putc(&debuginfo, '|');
            puthex(&debuginfo, (u32)cur, 8, 0);
            debug_putc(&debuginfo, '|');
            debug_putc(&debuginfo, ' ');
        }
    }

    va_list వాదనలు;
    va_start(వాదనలు, fmt);
    bvprintf(&debuginfo, fmt, వాదనలు);
    va_end(వాదనలు);
    debug_flush();
}

శూన్య
రాయి(స్థిరరాశి అక్షరం *fmt, ...)
{
    ASSERT32FLAT();
    va_list వాదనలు;
    va_start(వాదనలు, fmt);
    bvprintf(&screeninfo, fmt, వాదనలు);
    va_end(వాదనలు);
    ఒకవేళ (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

నిర్మాణం snprintfinfo {
    నిర్మాణం putcinfo info;
    అక్షరం *str, *చివర;
};

స్థిర శూన్య
putc_str(నిర్మాణం putcinfo *info, అక్షరం c)
{
    నిర్మాణం snprintfinfo *sinfo = container_యొక్క(info, నిర్మాణం snprintfinfo, info);
    ఒకవేళ (sinfo->str >= sinfo->చివర)
        తిరిగి;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
పూర్ణాంకం
snprintf(అక్షరం *str, కొలత_t కొలత, స్థిరరాశి అక్షరం *fmt, ...)
{
    ASSERT32FLAT();
    ఒకవేళ (!కొలత)
        తిరిగి 0;
    నిర్మాణం snprintfinfo sinfo = { { putc_str }, str, str + కొలత };
    va_list వాదనలు;
    va_start(వాదనలు, fmt);
    bvprintf(&sinfo.info, fmt, వాదనలు);
    va_end(వాదనలు);
    అక్షరం *చివర = sinfo.str;
    ఒకవేళ (చివర >= sinfo.చివర)
        చివర = sinfo.చివర - 1;
    *చివర = '\0';
    తిరిగి చివర - str;
}

// Build a formatted string - malloc'ing the memory.
అక్షరం *
znprintf(కొలత_t కొలత, స్థిరరాశి అక్షరం *fmt, ...)
{
    ASSERT32FLAT();
    ఒకవేళ (!కొలత)
        తిరిగి ఖాళీ;
    అక్షరం *str = malloc_tmp(కొలత);
    ఒకవేళ (!str) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    నిర్మాణం snprintfinfo sinfo = { { putc_str }, str, str + కొలత };
    va_list వాదనలు;
    va_start(వాదనలు, fmt);
    bvprintf(&sinfo.info, fmt, వాదనలు);
    va_end(వాదనలు);
    అక్షరం *చివర = sinfo.str;
    ఒకవేళ (చివర >= sinfo.చివర)
        చివర = sinfo.చివర - 1;
    *చివర = '\0';
    తిరిగి str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

శూన్య
hexdump(స్థిరరాశి శూన్య *d, పూర్ణాంకం len)
{
    పూర్ణాంకం లెక్క=0;
    అయితే (len > 0) {
        ఒకవేళ (లెక్క % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, లెక్క*4, 8, 0);
            putc(&debuginfo, ':');
        } లేకపోతే {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        లెక్క++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

స్థిర శూన్య
dump_regs(నిర్మాణం bregs *regs)
{
    ఒకవేళ (!regs) {
        dprintf(1, "  NULL\n");
        తిరిగి;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->సంకేతం_2.seg, regs->సంకేతం_2.స్థానభ్రంశం_2, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
శూన్య
__debug_isr(స్థిరరాశి అక్షరం *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
శూన్య
__debug_enter(నిర్మాణం bregs *regs, స్థిరరాశి అక్షరం *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
శూన్య
__debug_stub(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
శూన్య
__warn_invalid(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    ఒకవేళ (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
శూన్య
__warn_unimplemented(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    ఒకవేళ (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
శూన్య
__warn_internalerror(పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
శూన్య
__warn_noalloc(పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
శూన్య
__warn_timeout(పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
శూన్య
__set_invalid(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    __warn_invalid(regs, lineno, fname);
    సమితి_invalid_silent(regs);
}

// Report a call of an unimplemented function.
శూన్య
__set_unimplemented(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    సమితి_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
శూన్య
__set_code_invalid(నిర్మాణం bregs *regs, u32 linecode, స్థిరరాశి అక్షరం *fname)
{
    u8 సంకేతం_2 = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    సమితి_సంకేతం_invalid_silent(regs, సంకేతం_2);
}

// Report a call of an unimplemented function.
శూన్య
__set_code_unimplemented(నిర్మాణం bregs *regs, u32 linecode, స్థిరరాశి అక్షరం *fname)
{
    u8 సంకేతం_2 = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    సమితి_సంకేతం_invalid_silent(regs, సంకేతం_2);
}
