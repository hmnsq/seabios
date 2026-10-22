// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن <stdarg.h> // va_list

#ضمن "farptr.h" // GET_VAR
#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "hw/pci.h" // pci_bdf_to_bus
#ضمن "hw/pcidevice.h" // pci_device
#ضمن "hw/serialio.h" // serial_debug_putc
#ضمن "malloc.h" // malloc_tmp
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // call16_int
#ضمن "string.h" // memset
#ضمن "util.h" // ScreenAndDebug

بنية putcinfo {
    فراغ (*func)(بنية putcinfo *info, حرف c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

فراغ
debug_banner(فراغ)
{
    dprintf(1, "SeaBIOS (version %s)\n", نسخو);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
ساكن فراغ
debug_putc(بنية putcinfo *action, حرف c)
{
    إذا (! CONFIG_DEBUG_LEVEL)
        أرجع;
    qemu_debug_putc(c);
    إذا (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
ساكن فراغ
debug_flush(فراغ)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#إذا MODE16
ساكن بنية putcinfo debuginfo VAR16;
#وإلاإذا MODESEGMENT
ساكن بنية putcinfo debuginfo VAR32SEG;
#وإلا
ساكن بنية putcinfo debuginfo = { debug_putc };
#جيڪڏهن_پورو


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
ساكن فراغ
screenc(حرف c)
{
    إذا (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(داخلا_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        أرجع;
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
ساكن فراغ
screen_putc(بنية putcinfo *action, حرف c)
{
    إذا (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    إذا (c == '\n')
        screenc('\r');
    screenc(c);
}

ساكن بنية putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
ساكن فراغ
putc(بنية putcinfo *action, حرف c)
{
    إذا (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        أرجع;
    }

    فراغ (*func)(بنية putcinfo *info, حرف c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
ساكن فراغ
puts(بنية putcinfo *action, ثابت حرف *s)
{
    إذا (!MODESEGMENT && !s)
        s = "(NULL)";
    لكل (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
ساكن فراغ
puts_cs(بنية putcinfo *action, ثابت حرف *s)
{
    حرف *vs = (حرف*)s;
    لكل (;; vs++) {
        حرف c = GET_GLOBAL(*vs);
        إذا (!c)
            اكسر;
        putc(action, c);
    }
}

// Output an unsigned integer.
ساكن فراغ
putuint(بنية putcinfo *action, u32 val)
{
    حرف buf[12];
    حرف *d = &buf[حجم(buf) - 1];
    *d-- = '\0';
    لكل (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        إذا (!val)
            اكسر;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
ساكن مضمن فراغ
putsinglehex(بنية putcinfo *action, u32 val, صحيح uc)
{
    إذا (val <= 9)
        val = '0' + val;
    وإلا إذا (uc)
        val = 'A' + val - 10;
    وإلا
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
ساكن فراغ
puthex(بنية putcinfo *action, u32 val, صحيح ويڪر, صحيح uc)
{
    بدل (ويڪر) {
    افتراضي: putsinglehex(action, (val >> 28) & 0xf, uc);
    حالة 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    حالة 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    حالة 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    حالة 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    حالة 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    حالة 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    حالة 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
ساكن فراغ
putprettyhex(بنية putcinfo *action, u32 val, صحيح ويڪر, حرف padchar, صحيح uc)
{
    u32 tmp = val;
    صحيح ڳڻپ = 1;
    طالما (tmp >>= 4)
        ڳڻپ++;
    ويڪر -= ڳڻپ;
    طالما (ويڪر-- > 0)
        putc(action, padchar);
    puthex(action, val, ڳڻپ, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
ساكن فراغ
put_pci_device(بنية putcinfo *action, بنية pci_device *pci)
{
    puthex(action, pci_bdf_منزل_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_منزل_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_منزل_fn(pci->bdf), 1, 0);
}

ساكن مضمن صحيح
انگ_آهي(u8 c)
{
    أرجع ((u8)(c - '0')) < 10;
}

ساكن فراغ
bvprintf(بنية putcinfo *action, ثابت حرف *fmt, va_list دليلون)
{
    ثابت حرف *s = fmt;
    صحيح uc;
    لكل (;; s++) {
        حرف c = GET_GLOBAL(*(u8*)s);
        إذا (!c)
            اكسر;
        إذا (c != '%') {
            putc(action, c);
            استمر;
        }
        ثابت حرف *n = s+1;
        صحيح ميدان_ويڪر = 0;
        حرف padchar = ' ';
        u8 is64 = 0;
        لكل (;;) {
            c = GET_GLOBAL(*(u8*)n);
            إذا (!انگ_آهي(c))
                اكسر;
            إذا (!ميدان_ويڪر && (c == '0'))
                padchar = '0';
            وإلا
                ميدان_ويڪر = ميدان_ويڪر * 10 + c - '0';
            n++;
        }
        إذا (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        إذا (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ثابت حرف *sarg;
        بدل (c) {
        حالة '%':
            putc(action, '%');
            اكسر;
        حالة 'd':
            val = va_arg(دليلون, s32);
            إذا (is64)
                va_arg(دليلون, s32);
            إذا (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            اكسر;
        حالة 'u':
            val = va_arg(دليلون, s32);
            إذا (is64)
                va_arg(دليلون, s32);
            putuint(action, val);
            اكسر;
        حالة 'p':
            val = va_arg(دليلون, s32);
            إذا (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (فراغ*)val);
                n++;
                اكسر;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            اكسر;
        حالة 'X':
        حالة 'x':
            uc = (c == 'X');
            val = va_arg(دليلون, s32);
            إذا (is64) {
                u32 مٿانهون = va_arg(دليلون, s32);
                إذا (مٿانهون) {
                    putprettyhex(action, مٿانهون, ميدان_ويڪر - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    اكسر;
                }
            }
            putprettyhex(action, val, ميدان_ويڪر, padchar, uc);
            اكسر;
        حالة 'c':
            val = va_arg(دليلون, صحيح);
            putc(action, val);
            اكسر;
        حالة '.':
            // Hack to support "%.s" - meaning string on stack.
            إذا (GET_GLOBAL(*(u8*)(n+1)) != 's')
                اكسر;
            n++;
            sarg = va_arg(دليلون, ثابت حرف *);
            puts(action, sarg);
            اكسر;
        حالة 's':
            sarg = va_arg(دليلون, ثابت حرف *);
            puts_cs(action, sarg);
            اكسر;
        افتراضي:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

فراغ
panic(ثابت حرف *fmt, ...)
{
    إذا (CONFIG_DEBUG_LEVEL) {
        va_list دليلون;
        va_start(دليلون, fmt);
        bvprintf(&debuginfo, fmt, دليلون);
        va_end(دليلون);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    لكل (;;)
        hlt();
}

فراغ
__dprintf(ثابت حرف *fmt, ...)
{
    إذا (!MODESEGMENT && CONFIG_ڌاڳا && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        بنية thread_info *cur = getCurThread();
        إذا (cur != &MainThread) {
            // Show "thread id" for this debug message.
            debug_putc(&debuginfo, '|');
            puthex(&debuginfo, (u32)cur, 8, 0);
            debug_putc(&debuginfo, '|');
            debug_putc(&debuginfo, ' ');
        }
    }

    va_list دليلون;
    va_start(دليلون, fmt);
    bvprintf(&debuginfo, fmt, دليلون);
    va_end(دليلون);
    debug_flush();
}

فراغ
لکو(ثابت حرف *fmt, ...)
{
    ASSERT32FLAT();
    va_list دليلون;
    va_start(دليلون, fmt);
    bvprintf(&screeninfo, fmt, دليلون);
    va_end(دليلون);
    إذا (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

بنية snprintfinfo {
    بنية putcinfo info;
    حرف *str, *پڇاڙي;
};

ساكن فراغ
putc_str(بنية putcinfo *info, حرف c)
{
    بنية snprintfinfo *sinfo = container_جو(info, بنية snprintfinfo, info);
    إذا (sinfo->str >= sinfo->پڇاڙي)
        أرجع;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
صحيح
snprintf(حرف *str, سائيز_t سائيز, ثابت حرف *fmt, ...)
{
    ASSERT32FLAT();
    إذا (!سائيز)
        أرجع 0;
    بنية snprintfinfo sinfo = { { putc_str }, str, str + سائيز };
    va_list دليلون;
    va_start(دليلون, fmt);
    bvprintf(&sinfo.info, fmt, دليلون);
    va_end(دليلون);
    حرف *پڇاڙي = sinfo.str;
    إذا (پڇاڙي >= sinfo.پڇاڙي)
        پڇاڙي = sinfo.پڇاڙي - 1;
    *پڇاڙي = '\0';
    أرجع پڇاڙي - str;
}

// Build a formatted string - malloc'ing the memory.
حرف *
znprintf(سائيز_t سائيز, ثابت حرف *fmt, ...)
{
    ASSERT32FLAT();
    إذا (!سائيز)
        أرجع عدم;
    حرف *str = malloc_tmp(سائيز);
    إذا (!str) {
        warn_noalloc();
        أرجع عدم;
    }
    بنية snprintfinfo sinfo = { { putc_str }, str, str + سائيز };
    va_list دليلون;
    va_start(دليلون, fmt);
    bvprintf(&sinfo.info, fmt, دليلون);
    va_end(دليلون);
    حرف *پڇاڙي = sinfo.str;
    إذا (پڇاڙي >= sinfo.پڇاڙي)
        پڇاڙي = sinfo.پڇاڙي - 1;
    *پڇاڙي = '\0';
    أرجع str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

فراغ
hexdump(ثابت فراغ *d, صحيح len)
{
    صحيح ڳڻپ=0;
    طالما (len > 0) {
        إذا (ڳڻپ % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, ڳڻپ*4, 8, 0);
            putc(&debuginfo, ':');
        } وإلا {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        ڳڻپ++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

ساكن فراغ
dump_regs(بنية bregs *regs)
{
    إذا (!regs) {
        dprintf(1, "  NULL\n");
        أرجع;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->ڪوڊ.seg, regs->ڪوڊ.وٿي_2, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
فراغ
__debug_isr(ثابت حرف *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
فراغ
__debug_enter(بنية bregs *regs, ثابت حرف *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
فراغ
__debug_stub(بنية bregs *regs, صحيح lineno, ثابت حرف *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
فراغ
__warn_invalid(بنية bregs *regs, صحيح lineno, ثابت حرف *fname)
{
    إذا (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
فراغ
__warn_unimplemented(بنية bregs *regs, صحيح lineno, ثابت حرف *fname)
{
    إذا (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
فراغ
__warn_internalerror(صحيح lineno, ثابت حرف *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
فراغ
__warn_noalloc(صحيح lineno, ثابت حرف *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
فراغ
__warn_timeout(صحيح lineno, ثابت حرف *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
فراغ
__set_invalid(بنية bregs *regs, صحيح lineno, ثابت حرف *fname)
{
    __warn_invalid(regs, lineno, fname);
    سيٽ_invalid_silent(regs);
}

// Report a call of an unimplemented function.
فراغ
__set_unimplemented(بنية bregs *regs, صحيح lineno, ثابت حرف *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    سيٽ_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
فراغ
__set_code_invalid(بنية bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 ڪوڊ = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    سيٽ_ڪوڊ_invalid_silent(regs, ڪوڊ);
}

// Report a call of an unimplemented function.
فراغ
__set_code_unimplemented(بنية bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 ڪوڊ = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    سيٽ_ڪوڊ_invalid_silent(regs, ڪوڊ);
}
