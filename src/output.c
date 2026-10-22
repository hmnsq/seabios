// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ <stdarg.h> // va_list

#അന്തര്ഭാവ "farptr.h" // GET_VAR
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "hw/pci.h" // pci_bdf_to_bus
#അന്തര്ഭാവ "hw/pcidevice.h" // pci_device
#അന്തര്ഭാവ "hw/serialio.h" // serial_debug_putc
#അന്തര്ഭാവ "malloc.h" // malloc_tmp
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // call16_int
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // ScreenAndDebug

ഘടന putcinfo {
    ശൂന്യം (*func)(ഘടന putcinfo *info, അക്ഷരം c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

ശൂന്യം
debug_banner(ശൂന്യം)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
സ്ഥിതം ശൂന്യം
debug_putc(ഘടന putcinfo *action, അക്ഷരം c)
{
    എങ്കിൽ (! CONFIG_DEBUG_LEVEL)
        തിരികെ;
    qemu_debug_putc(c);
    എങ്കിൽ (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
സ്ഥിതം ശൂന്യം
debug_flush(ശൂന്യം)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#എങ്കിൽ MODE16
സ്ഥിതം ഘടന putcinfo debuginfo VAR16;
#അല്ലെങ്കിലെങ്കിൽ MODESEGMENT
സ്ഥിതം ഘടന putcinfo debuginfo VAR32SEG;
#അല്ലെങ്കിൽ
സ്ഥിതം ഘടന putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
സ്ഥിതം ശൂന്യം
screenc(അക്ഷരം c)
{
    എങ്കിൽ (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        തിരികെ;
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
സ്ഥിതം ശൂന്യം
screen_putc(ഘടന putcinfo *action, അക്ഷരം c)
{
    എങ്കിൽ (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    എങ്കിൽ (c == '\n')
        screenc('\r');
    screenc(c);
}

സ്ഥിതം ഘടന putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
സ്ഥിതം ശൂന്യം
putc(ഘടന putcinfo *action, അക്ഷരം c)
{
    എങ്കിൽ (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        തിരികെ;
    }

    ശൂന്യം (*func)(ഘടന putcinfo *info, അക്ഷരം c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
സ്ഥിതം ശൂന്യം
puts(ഘടന putcinfo *action, സ്ഥിരം അക്ഷരം *s)
{
    എങ്കിൽ (!MODESEGMENT && !s)
        s = "(NULL)";
    വേണ്ടി (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
സ്ഥിതം ശൂന്യം
puts_cs(ഘടന putcinfo *action, സ്ഥിരം അക്ഷരം *s)
{
    അക്ഷരം *vs = (അക്ഷരം*)s;
    വേണ്ടി (;; vs++) {
        അക്ഷരം c = GET_GLOBAL(*vs);
        എങ്കിൽ (!c)
            നിർത്തുക;
        putc(action, c);
    }
}

// Output an unsigned integer.
സ്ഥിതം ശൂന്യം
putuint(ഘടന putcinfo *action, u32 val)
{
    അക്ഷരം buf[12];
    അക്ഷരം *d = &buf[വലുപ്പം(buf) - 1];
    *d-- = '\0';
    വേണ്ടി (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        എങ്കിൽ (!val)
            നിർത്തുക;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
സ്ഥിതം വരിയിൽ ശൂന്യം
putsinglehex(ഘടന putcinfo *action, u32 val, പൂർണ്ണസംഖ്യ uc)
{
    എങ്കിൽ (val <= 9)
        val = '0' + val;
    അല്ലെങ്കിൽ എങ്കിൽ (uc)
        val = 'A' + val - 10;
    അല്ലെങ്കിൽ
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
സ്ഥിതം ശൂന്യം
puthex(ഘടന putcinfo *action, u32 val, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ uc)
{
    തിരഞ്ഞെടുപ്പ് (width) {
    സ്ഥിരസ്ഥിതി: putsinglehex(action, (val >> 28) & 0xf, uc);
    അവസ്ഥ 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    അവസ്ഥ 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    അവസ്ഥ 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    അവസ്ഥ 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    അവസ്ഥ 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    അവസ്ഥ 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    അവസ്ഥ 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
സ്ഥിതം ശൂന്യം
putprettyhex(ഘടന putcinfo *action, u32 val, പൂർണ്ണസംഖ്യ width, അക്ഷരം padchar, പൂർണ്ണസംഖ്യ uc)
{
    u32 tmp = val;
    പൂർണ്ണസംഖ്യ എണ്ണം = 1;
    ആകുമ്പോൾ (tmp >>= 4)
        എണ്ണം++;
    width -= എണ്ണം;
    ആകുമ്പോൾ (width-- > 0)
        putc(action, padchar);
    puthex(action, val, എണ്ണം, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
സ്ഥിതം ശൂന്യം
put_pci_device(ഘടന putcinfo *action, ഘടന pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ
isdigit(u8 c)
{
    തിരികെ ((u8)(c - '0')) < 10;
}

സ്ഥിതം ശൂന്യം
bvprintf(ഘടന putcinfo *action, സ്ഥിരം അക്ഷരം *fmt, va_list args)
{
    സ്ഥിരം അക്ഷരം *s = fmt;
    പൂർണ്ണസംഖ്യ uc;
    വേണ്ടി (;; s++) {
        അക്ഷരം c = GET_GLOBAL(*(u8*)s);
        എങ്കിൽ (!c)
            നിർത്തുക;
        എങ്കിൽ (c != '%') {
            putc(action, c);
            തുടരുക;
        }
        സ്ഥിരം അക്ഷരം *n = s+1;
        പൂർണ്ണസംഖ്യ field_width = 0;
        അക്ഷരം padchar = ' ';
        u8 is64 = 0;
        വേണ്ടി (;;) {
            c = GET_GLOBAL(*(u8*)n);
            എങ്കിൽ (!isdigit(c))
                നിർത്തുക;
            എങ്കിൽ (!field_width && (c == '0'))
                padchar = '0';
            അല്ലെങ്കിൽ
                field_width = field_width * 10 + c - '0';
            n++;
        }
        എങ്കിൽ (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        എങ്കിൽ (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        സ്ഥിരം അക്ഷരം *sarg;
        തിരഞ്ഞെടുപ്പ് (c) {
        അവസ്ഥ '%':
            putc(action, '%');
            നിർത്തുക;
        അവസ്ഥ 'd':
            val = va_arg(args, s32);
            എങ്കിൽ (is64)
                va_arg(args, s32);
            എങ്കിൽ (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            നിർത്തുക;
        അവസ്ഥ 'u':
            val = va_arg(args, s32);
            എങ്കിൽ (is64)
                va_arg(args, s32);
            putuint(action, val);
            നിർത്തുക;
        അവസ്ഥ 'p':
            val = va_arg(args, s32);
            എങ്കിൽ (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (ശൂന്യം*)val);
                n++;
                നിർത്തുക;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            നിർത്തുക;
        അവസ്ഥ 'X':
        അവസ്ഥ 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            എങ്കിൽ (is64) {
                u32 upper = va_arg(args, s32);
                എങ്കിൽ (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    നിർത്തുക;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            നിർത്തുക;
        അവസ്ഥ 'c':
            val = va_arg(args, പൂർണ്ണസംഖ്യ);
            putc(action, val);
            നിർത്തുക;
        അവസ്ഥ '.':
            // Hack to support "%.s" - meaning string on stack.
            എങ്കിൽ (GET_GLOBAL(*(u8*)(n+1)) != 's')
                നിർത്തുക;
            n++;
            sarg = va_arg(args, സ്ഥിരം അക്ഷരം *);
            puts(action, sarg);
            നിർത്തുക;
        അവസ്ഥ 's':
            sarg = va_arg(args, സ്ഥിരം അക്ഷരം *);
            puts_cs(action, sarg);
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

ശൂന്യം
panic(സ്ഥിരം അക്ഷരം *fmt, ...)
{
    എങ്കിൽ (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    വേണ്ടി (;;)
        hlt();
}

ശൂന്യം
__dprintf(സ്ഥിരം അക്ഷരം *fmt, ...)
{
    എങ്കിൽ (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        ഘടന thread_info *cur = getCurThread();
        എങ്കിൽ (cur != &MainThread) {
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

ശൂന്യം
printf(സ്ഥിരം അക്ഷരം *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    എങ്കിൽ (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

ഘടന snprintfinfo {
    ഘടന putcinfo info;
    അക്ഷരം *str, *end;
};

സ്ഥിതം ശൂന്യം
putc_str(ഘടന putcinfo *info, അക്ഷരം c)
{
    ഘടന snprintfinfo *sinfo = container_of(info, ഘടന snprintfinfo, info);
    എങ്കിൽ (sinfo->str >= sinfo->end)
        തിരികെ;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
പൂർണ്ണസംഖ്യ
snprintf(അക്ഷരം *str, size_t size, സ്ഥിരം അക്ഷരം *fmt, ...)
{
    ASSERT32FLAT();
    എങ്കിൽ (!size)
        തിരികെ 0;
    ഘടന snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    അക്ഷരം *end = sinfo.str;
    എങ്കിൽ (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    തിരികെ end - str;
}

// Build a formatted string - malloc'ing the memory.
അക്ഷരം *
znprintf(size_t size, സ്ഥിരം അക്ഷരം *fmt, ...)
{
    ASSERT32FLAT();
    എങ്കിൽ (!size)
        തിരികെ ശൂന്യ;
    അക്ഷരം *str = malloc_tmp(size);
    എങ്കിൽ (!str) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    ഘടന snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    അക്ഷരം *end = sinfo.str;
    എങ്കിൽ (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    തിരികെ str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

ശൂന്യം
hexdump(സ്ഥിരം ശൂന്യം *d, പൂർണ്ണസംഖ്യ len)
{
    പൂർണ്ണസംഖ്യ എണ്ണം=0;
    ആകുമ്പോൾ (len > 0) {
        എങ്കിൽ (എണ്ണം % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, എണ്ണം*4, 8, 0);
            putc(&debuginfo, ':');
        } അല്ലെങ്കിൽ {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        എണ്ണം++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

സ്ഥിതം ശൂന്യം
dump_regs(ഘടന bregs *regs)
{
    എങ്കിൽ (!regs) {
        dprintf(1, "  NULL\n");
        തിരികെ;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
ശൂന്യം
__debug_isr(സ്ഥിരം അക്ഷരം *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
ശൂന്യം
__debug_enter(ഘടന bregs *regs, സ്ഥിരം അക്ഷരം *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
ശൂന്യം
__debug_stub(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
ശൂന്യം
__warn_invalid(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    എങ്കിൽ (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
ശൂന്യം
__warn_unimplemented(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    എങ്കിൽ (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
ശൂന്യം
__warn_internalerror(പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
ശൂന്യം
__warn_noalloc(പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
ശൂന്യം
__warn_timeout(പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
ശൂന്യം
__set_invalid(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
ശൂന്യം
__set_unimplemented(ഘടന bregs *regs, പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
ശൂന്യം
__set_code_invalid(ഘടന bregs *regs, u32 linecode, സ്ഥിരം അക്ഷരം *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
ശൂന്യം
__set_code_unimplemented(ഘടന bregs *regs, u32 linecode, സ്ഥിരം അക്ഷരം *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
