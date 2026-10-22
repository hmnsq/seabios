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

ساختار putcinfo {
    تهی (*func)(ساختار putcinfo *info, نویسه c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

تهی
debug_banner(تهی)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
ایستا تهی
debug_putc(ساختار putcinfo *action, نویسه c)
{
    اگر (! CONFIG_DEBUG_LEVEL)
        بازگشت;
    qemu_debug_putc(c);
    اگر (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
ایستا تهی
debug_flush(تهی)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#اگر MODE16
ایستا ساختار putcinfo debuginfo VAR16;
#وگرنهاگر MODESEGMENT
ایستا ساختار putcinfo debuginfo VAR32SEG;
#وگرنه
ایستا ساختار putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
ایستا تهی
screenc(نویسه c)
{
    اگر (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        بازگشت;
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
ایستا تهی
screen_putc(ساختار putcinfo *action, نویسه c)
{
    اگر (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    اگر (c == '\n')
        screenc('\r');
    screenc(c);
}

ایستا ساختار putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
ایستا تهی
putc(ساختار putcinfo *action, نویسه c)
{
    اگر (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        بازگشت;
    }

    تهی (*func)(ساختار putcinfo *info, نویسه c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
ایستا تهی
puts(ساختار putcinfo *action, ثابت نویسه *s)
{
    اگر (!MODESEGMENT && !s)
        s = "(NULL)";
    برای (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
ایستا تهی
puts_cs(ساختار putcinfo *action, ثابت نویسه *s)
{
    نویسه *vs = (نویسه*)s;
    برای (;; vs++) {
        نویسه c = GET_GLOBAL(*vs);
        اگر (!c)
            بشکن;
        putc(action, c);
    }
}

// Output an unsigned integer.
ایستا تهی
putuint(ساختار putcinfo *action, u32 val)
{
    نویسه buf[12];
    نویسه *d = &buf[اندازه(buf) - 1];
    *d-- = '\0';
    برای (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        اگر (!val)
            بشکن;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
ایستا درونخطی تهی
putsinglehex(ساختار putcinfo *action, u32 val, صحیح uc)
{
    اگر (val <= 9)
        val = '0' + val;
    وگرنه اگر (uc)
        val = 'A' + val - 10;
    وگرنه
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
ایستا تهی
puthex(ساختار putcinfo *action, u32 val, صحیح width, صحیح uc)
{
    انتخاب (width) {
    پیشفرض: putsinglehex(action, (val >> 28) & 0xf, uc);
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
ایستا تهی
putprettyhex(ساختار putcinfo *action, u32 val, صحیح width, نویسه padchar, صحیح uc)
{
    u32 tmp = val;
    صحیح شمار = 1;
    وقتی (tmp >>= 4)
        شمار++;
    width -= شمار;
    وقتی (width-- > 0)
        putc(action, padchar);
    puthex(action, val, شمار, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
ایستا تهی
put_pci_device(ساختار putcinfo *action, ساختار pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

ایستا درونخطی صحیح
isdigit(u8 c)
{
    بازگشت ((u8)(c - '0')) < 10;
}

ایستا تهی
bvprintf(ساختار putcinfo *action, ثابت نویسه *fmt, va_list args)
{
    ثابت نویسه *s = fmt;
    صحیح uc;
    برای (;; s++) {
        نویسه c = GET_GLOBAL(*(u8*)s);
        اگر (!c)
            بشکن;
        اگر (c != '%') {
            putc(action, c);
            ادامه;
        }
        ثابت نویسه *n = s+1;
        صحیح field_width = 0;
        نویسه padchar = ' ';
        u8 is64 = 0;
        برای (;;) {
            c = GET_GLOBAL(*(u8*)n);
            اگر (!isdigit(c))
                بشکن;
            اگر (!field_width && (c == '0'))
                padchar = '0';
            وگرنه
                field_width = field_width * 10 + c - '0';
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
        ثابت نویسه *sarg;
        انتخاب (c) {
        حالت '%':
            putc(action, '%');
            بشکن;
        حالت 'd':
            val = va_arg(args, s32);
            اگر (is64)
                va_arg(args, s32);
            اگر (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            بشکن;
        حالت 'u':
            val = va_arg(args, s32);
            اگر (is64)
                va_arg(args, s32);
            putuint(action, val);
            بشکن;
        حالت 'p':
            val = va_arg(args, s32);
            اگر (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (تهی*)val);
                n++;
                بشکن;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            بشکن;
        حالت 'X':
        حالت 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            اگر (is64) {
                u32 upper = va_arg(args, s32);
                اگر (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    بشکن;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            بشکن;
        حالت 'c':
            val = va_arg(args, صحیح);
            putc(action, val);
            بشکن;
        حالت '.':
            // Hack to support "%.s" - meaning string on stack.
            اگر (GET_GLOBAL(*(u8*)(n+1)) != 's')
                بشکن;
            n++;
            sarg = va_arg(args, ثابت نویسه *);
            puts(action, sarg);
            بشکن;
        حالت 's':
            sarg = va_arg(args, ثابت نویسه *);
            puts_cs(action, sarg);
            بشکن;
        پیشفرض:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

تهی
panic(ثابت نویسه *fmt, ...)
{
    اگر (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    برای (;;)
        hlt();
}

تهی
__dprintf(ثابت نویسه *fmt, ...)
{
    اگر (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        ساختار thread_info *cur = getCurThread();
        اگر (cur != &MainThread) {
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

تهی
printf(ثابت نویسه *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    اگر (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

ساختار snprintfinfo {
    ساختار putcinfo info;
    نویسه *str, *end;
};

ایستا تهی
putc_str(ساختار putcinfo *info, نویسه c)
{
    ساختار snprintfinfo *sinfo = container_of(info, ساختار snprintfinfo, info);
    اگر (sinfo->str >= sinfo->end)
        بازگشت;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
صحیح
snprintf(نویسه *str, size_t size, ثابت نویسه *fmt, ...)
{
    ASSERT32FLAT();
    اگر (!size)
        بازگشت 0;
    ساختار snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    نویسه *end = sinfo.str;
    اگر (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    بازگشت end - str;
}

// Build a formatted string - malloc'ing the memory.
نویسه *
znprintf(size_t size, ثابت نویسه *fmt, ...)
{
    ASSERT32FLAT();
    اگر (!size)
        بازگشت پوچ;
    نویسه *str = malloc_tmp(size);
    اگر (!str) {
        warn_noalloc();
        بازگشت پوچ;
    }
    ساختار snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    نویسه *end = sinfo.str;
    اگر (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    بازگشت str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

تهی
hexdump(ثابت تهی *d, صحیح len)
{
    صحیح شمار=0;
    وقتی (len > 0) {
        اگر (شمار % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, شمار*4, 8, 0);
            putc(&debuginfo, ':');
        } وگرنه {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        شمار++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

ایستا تهی
dump_regs(ساختار bregs *regs)
{
    اگر (!regs) {
        dprintf(1, "  NULL\n");
        بازگشت;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
تهی
__debug_isr(ثابت نویسه *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
تهی
__debug_enter(ساختار bregs *regs, ثابت نویسه *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
تهی
__debug_stub(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
تهی
__warn_invalid(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname)
{
    اگر (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
تهی
__warn_unimplemented(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname)
{
    اگر (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
تهی
__warn_internalerror(صحیح lineno, ثابت نویسه *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
تهی
__warn_noalloc(صحیح lineno, ثابت نویسه *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
تهی
__warn_timeout(صحیح lineno, ثابت نویسه *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
تهی
__set_invalid(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
تهی
__set_unimplemented(ساختار bregs *regs, صحیح lineno, ثابت نویسه *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
تهی
__set_code_invalid(ساختار bregs *regs, u32 linecode, ثابت نویسه *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
تهی
__set_code_unimplemented(ساختار bregs *regs, u32 linecode, ثابت نویسه *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
