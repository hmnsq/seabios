// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل <stdarg.h> // va_list

#شامل "farptr.h" // GET_VAR
#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "biosvar.h" // GET_GLOBAL
#شامل "hw/pci.h" // pci_bdf_to_bus
#شامل "hw/pcidevice.h" // pci_device
#شامل "hw/serialio.h" // serial_debug_putc
#شامل "malloc.h" // malloc_tmp
#شامل "output.h" // dprintf
#شامل "stacks.h" // call16_int
#شامل "string.h" // memset
#شامل "util.h" // ScreenAndDebug

ساخت putcinfo {
    خالی (*func)(ساخت putcinfo *info, حرف c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

خالی
debug_banner(خالی)
{
    dprintf(1, "SeaBIOS (version %s)\n", نسخہ);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
ساکن خالی
debug_putc(ساخت putcinfo *action, حرف c)
{
    اگر (! CONFIG_DEBUG_LEVEL)
        واپس;
    qemu_debug_putc(c);
    اگر (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
ساکن خالی
debug_flush(خالی)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#اگر MODE16
ساکن ساخت putcinfo debuginfo VAR16;
#وگرنہ_اگر MODESEGMENT
ساکن ساخت putcinfo debuginfo VAR32SEG;
#ورنہ
ساکن ساخت putcinfo debuginfo = { debug_putc };
#اگر_ختم


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
ساکن خالی
screenc(حرف c)
{
    اگر (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(اندراج_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        واپس;
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
ساکن خالی
screen_putc(ساخت putcinfo *action, حرف c)
{
    اگر (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    اگر (c == '\n')
        screenc('\r');
    screenc(c);
}

ساکن ساخت putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
ساکن خالی
putc(ساخت putcinfo *action, حرف c)
{
    اگر (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        واپس;
    }

    خالی (*func)(ساخت putcinfo *info, حرف c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
ساکن خالی
puts(ساخت putcinfo *action, ثابت حرف *s)
{
    اگر (!MODESEGMENT && !s)
        s = "(NULL)";
    برائے (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
ساکن خالی
puts_cs(ساخت putcinfo *action, ثابت حرف *s)
{
    حرف *vs = (حرف*)s;
    برائے (;; vs++) {
        حرف c = GET_GLOBAL(*vs);
        اگر (!c)
            توڑو;
        putc(action, c);
    }
}

// Output an unsigned integer.
ساکن خالی
putuint(ساخت putcinfo *action, u32 val)
{
    حرف buf[12];
    حرف *d = &buf[حجم(buf) - 1];
    *d-- = '\0';
    برائے (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        اگر (!val)
            توڑو;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
ساکن اندرونی خالی
putsinglehex(ساخت putcinfo *action, u32 val, صحیح uc)
{
    اگر (val <= 9)
        val = '0' + val;
    ورنہ اگر (uc)
        val = 'A' + val - 10;
    ورنہ
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
ساکن خالی
puthex(ساخت putcinfo *action, u32 val, صحیح چوڑائی, صحیح uc)
{
    انتخاب (چوڑائی) {
    طےشدہ: putsinglehex(action, (val >> 28) & 0xf, uc);
    حالت 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    حالت 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    حالت 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    حالت 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    حالت 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    حالت 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    حالت 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
ساکن خالی
putprettyhex(ساخت putcinfo *action, u32 val, صحیح چوڑائی, حرف padchar, صحیح uc)
{
    u32 tmp = val;
    صحیح گنتی = 1;
    جب (tmp >>= 4)
        گنتی++;
    چوڑائی -= گنتی;
    جب (چوڑائی-- > 0)
        putc(action, padchar);
    puthex(action, val, گنتی, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
ساکن خالی
put_pci_device(ساخت putcinfo *action, ساخت pci_device *pci)
{
    puthex(action, pci_bdf_مقصد_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_مقصد_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_مقصد_fn(pci->bdf), 1, 0);
}

ساکن اندرونی صحیح
ہندسہ_ہے(u8 c)
{
    واپس ((u8)(c - '0')) < 10;
}

ساکن خالی
bvprintf(ساخت putcinfo *action, ثابت حرف *fmt, va_list دلائل)
{
    ثابت حرف *s = fmt;
    صحیح uc;
    برائے (;; s++) {
        حرف c = GET_GLOBAL(*(u8*)s);
        اگر (!c)
            توڑو;
        اگر (c != '%') {
            putc(action, c);
            جاری;
        }
        ثابت حرف *n = s+1;
        صحیح میدان_چوڑائی = 0;
        حرف padchar = ' ';
        u8 is64 = 0;
        برائے (;;) {
            c = GET_GLOBAL(*(u8*)n);
            اگر (!ہندسہ_ہے(c))
                توڑو;
            اگر (!میدان_چوڑائی && (c == '0'))
                padchar = '0';
            ورنہ
                میدان_چوڑائی = میدان_چوڑائی * 10 + c - '0';
            n++;
        }
        اگر (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        اگر (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ثابت حرف *sarg;
        انتخاب (c) {
        حالت '%':
            putc(action, '%');
            توڑو;
        حالت 'd':
            val = va_arg(دلائل, s32);
            اگر (is64)
                va_arg(دلائل, s32);
            اگر (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            توڑو;
        حالت 'u':
            val = va_arg(دلائل, s32);
            اگر (is64)
                va_arg(دلائل, s32);
            putuint(action, val);
            توڑو;
        حالت 'p':
            val = va_arg(دلائل, s32);
            اگر (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (خالی*)val);
                n++;
                توڑو;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            توڑو;
        حالت 'X':
        حالت 'x':
            uc = (c == 'X');
            val = va_arg(دلائل, s32);
            اگر (is64) {
                u32 بالائی = va_arg(دلائل, s32);
                اگر (بالائی) {
                    putprettyhex(action, بالائی, میدان_چوڑائی - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    توڑو;
                }
            }
            putprettyhex(action, val, میدان_چوڑائی, padchar, uc);
            توڑو;
        حالت 'c':
            val = va_arg(دلائل, صحیح);
            putc(action, val);
            توڑو;
        حالت '.':
            // Hack to support "%.s" - meaning string on stack.
            اگر (GET_GLOBAL(*(u8*)(n+1)) != 's')
                توڑو;
            n++;
            sarg = va_arg(دلائل, ثابت حرف *);
            puts(action, sarg);
            توڑو;
        حالت 's':
            sarg = va_arg(دلائل, ثابت حرف *);
            puts_cs(action, sarg);
            توڑو;
        طےشدہ:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

خالی
panic(ثابت حرف *fmt, ...)
{
    اگر (CONFIG_DEBUG_LEVEL) {
        va_list دلائل;
        va_start(دلائل, fmt);
        bvprintf(&debuginfo, fmt, دلائل);
        va_end(دلائل);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    برائے (;;)
        hlt();
}

خالی
__dprintf(ثابت حرف *fmt, ...)
{
    اگر (!MODESEGMENT && CONFIG_دھاگے && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        ساخت thread_info *cur = getCurThread();
        اگر (cur != &MainThread) {
            // Show "thread id" for this debug message.
            debug_putc(&debuginfo, '|');
            puthex(&debuginfo, (u32)cur, 8, 0);
            debug_putc(&debuginfo, '|');
            debug_putc(&debuginfo, ' ');
        }
    }

    va_list دلائل;
    va_start(دلائل, fmt);
    bvprintf(&debuginfo, fmt, دلائل);
    va_end(دلائل);
    debug_flush();
}

خالی
لکھو(ثابت حرف *fmt, ...)
{
    ASSERT32FLAT();
    va_list دلائل;
    va_start(دلائل, fmt);
    bvprintf(&screeninfo, fmt, دلائل);
    va_end(دلائل);
    اگر (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

ساخت snprintfinfo {
    ساخت putcinfo info;
    حرف *str, *انتہا;
};

ساکن خالی
putc_str(ساخت putcinfo *info, حرف c)
{
    ساخت snprintfinfo *sinfo = container_کا(info, ساخت snprintfinfo, info);
    اگر (sinfo->str >= sinfo->انتہا)
        واپس;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
صحیح
snprintf(حرف *str, سائز_t سائز, ثابت حرف *fmt, ...)
{
    ASSERT32FLAT();
    اگر (!سائز)
        واپس 0;
    ساخت snprintfinfo sinfo = { { putc_str }, str, str + سائز };
    va_list دلائل;
    va_start(دلائل, fmt);
    bvprintf(&sinfo.info, fmt, دلائل);
    va_end(دلائل);
    حرف *انتہا = sinfo.str;
    اگر (انتہا >= sinfo.انتہا)
        انتہا = sinfo.انتہا - 1;
    *انتہا = '\0';
    واپس انتہا - str;
}

// Build a formatted string - malloc'ing the memory.
حرف *
znprintf(سائز_t سائز, ثابت حرف *fmt, ...)
{
    ASSERT32FLAT();
    اگر (!سائز)
        واپس تہی;
    حرف *str = malloc_tmp(سائز);
    اگر (!str) {
        warn_noalloc();
        واپس تہی;
    }
    ساخت snprintfinfo sinfo = { { putc_str }, str, str + سائز };
    va_list دلائل;
    va_start(دلائل, fmt);
    bvprintf(&sinfo.info, fmt, دلائل);
    va_end(دلائل);
    حرف *انتہا = sinfo.str;
    اگر (انتہا >= sinfo.انتہا)
        انتہا = sinfo.انتہا - 1;
    *انتہا = '\0';
    واپس str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

خالی
hexdump(ثابت خالی *d, صحیح len)
{
    صحیح گنتی=0;
    جب (len > 0) {
        اگر (گنتی % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, گنتی*4, 8, 0);
            putc(&debuginfo, ':');
        } ورنہ {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        گنتی++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

ساکن خالی
dump_regs(ساخت bregs *regs)
{
    اگر (!regs) {
        dprintf(1, "  NULL\n");
        واپس;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->رمز.seg, regs->رمز.انحراف_2, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
خالی
__debug_isr(ثابت حرف *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
خالی
__debug_enter(ساخت bregs *regs, ثابت حرف *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
خالی
__debug_stub(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
خالی
__warn_invalid(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname)
{
    اگر (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
خالی
__warn_unimplemented(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname)
{
    اگر (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
خالی
__warn_internalerror(صحیح lineno, ثابت حرف *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
خالی
__warn_noalloc(صحیح lineno, ثابت حرف *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
خالی
__warn_timeout(صحیح lineno, ثابت حرف *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
خالی
__set_invalid(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname)
{
    __warn_invalid(regs, lineno, fname);
    سیٹ_invalid_silent(regs);
}

// Report a call of an unimplemented function.
خالی
__set_unimplemented(ساخت bregs *regs, صحیح lineno, ثابت حرف *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    سیٹ_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
خالی
__set_code_invalid(ساخت bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 رمز = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    سیٹ_رمز_invalid_silent(regs, رمز);
}

// Report a call of an unimplemented function.
خالی
__set_code_unimplemented(ساخت bregs *regs, u32 linecode, ثابت حرف *fname)
{
    u8 رمز = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    سیٹ_رمز_invalid_silent(regs, رمز);
}
