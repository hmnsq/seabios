// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include <stdarg.h> // va_list

#include "farptr.h" // GET_VAR
#include "bregs.h" // struct bregs
#include "config.h" // CONFIG_*
#include "biosvar.h" // GET_GLOBAL
#include "hw/pci.h" // pci_bdf_to_bus
#include "hw/pcidevice.h" // pci_device
#include "hw/serialio.h" // serial_debug_putc
#include "malloc.h" // malloc_tmp
#include "output.h" // dprintf
#include "stacks.h" // call16_int
#include "string.h" // memset
#include "util.h" // ScreenAndDebug

sa_nracanaa putcinfo {
    void (*func)(sa_nracanaa putcinfo *info, var_na c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

void
debug_banner(void)
{
    dprintf(1, "SeaBIOS (version %s)\n", VERSION);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
static void
debug_putc(sa_nracanaa putcinfo *action, var_na c)
{
    အကယ်၍ (! CONFIG_DEBUG_LEVEL)
        ပြန်ပေး;
    qemu_debug_putc(c);
    အကယ်၍ (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
static void
debug_flush(void)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#အကယ်၍ MODE16
static sa_nracanaa putcinfo debuginfo VAR16;
#elif MODESEGMENT
static sa_nracanaa putcinfo debuginfo VAR32SEG;
#မဟုတ်ပါက
static sa_nracanaa putcinfo debuginfo = { debug_putc };
#endif


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
static void
screenc(var_na c)
{
    အကယ်၍ (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(entry_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        ပြန်ပေး;
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
static void
screen_putc(sa_nracanaa putcinfo *action, var_na c)
{
    အကယ်၍ (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    အကယ်၍ (c == '\n')
        screenc('\r');
    screenc(c);
}

static sa_nracanaa putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
static void
putc(sa_nracanaa putcinfo *action, var_na c)
{
    အကယ်၍ (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        ပြန်ပေး;
    }

    void (*func)(sa_nracanaa putcinfo *info, var_na c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
static void
puts(sa_nracanaa putcinfo *action, const var_na *s)
{
    အကယ်၍ (!MODESEGMENT && !s)
        s = "(NULL)";
    အတွက် (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
static void
puts_cs(sa_nracanaa putcinfo *action, const var_na *s)
{
    var_na *vs = (var_na*)s;
    အတွက် (;; vs++) {
        var_na c = GET_GLOBAL(*vs);
        အကယ်၍ (!c)
            bha_nga;
        putc(action, c);
    }
}

// Output an unsigned integer.
static void
putuint(sa_nracanaa putcinfo *action, u32 val)
{
    var_na buf[12];
    var_na *d = &buf[parimaa_na(buf) - 1];
    *d-- = '\0';
    အတွက် (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        အကယ်၍ (!val)
            bha_nga;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
static anta_hstha void
putsinglehex(sa_nracanaa putcinfo *action, u32 val, ကိန်းပြည့် uc)
{
    အကယ်၍ (val <= 9)
        val = '0' + val;
    မဟုတ်ပါက အကယ်၍ (uc)
        val = 'A' + val - 10;
    မဟုတ်ပါက
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
static void
puthex(sa_nracanaa putcinfo *action, u32 val, ကိန်းပြည့် width, ကိန်းပြည့် uc)
{
    switch (width) {
    default: putsinglehex(action, (val >> 28) & 0xf, uc);
    pak_sa 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    pak_sa 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    pak_sa 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    pak_sa 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    pak_sa 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    pak_sa 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    pak_sa 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
static void
putprettyhex(sa_nracanaa putcinfo *action, u32 val, ကိန်းပြည့် width, var_na padchar, ကိန်းပြည့် uc)
{
    u32 tmp = val;
    ကိန်းပြည့် count = 1;
    နေစဉ် (tmp >>= 4)
        count++;
    width -= count;
    နေစဉ် (width-- > 0)
        putc(action, padchar);
    puthex(action, val, count, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
static void
put_pci_device(sa_nracanaa putcinfo *action, sa_nracanaa pci_device *pci)
{
    puthex(action, pci_bdf_to_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_to_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_to_fn(pci->bdf), 1, 0);
}

static anta_hstha ကိန်းပြည့်
isdigit(u8 c)
{
    ပြန်ပေး ((u8)(c - '0')) < 10;
}

static void
bvprintf(sa_nracanaa putcinfo *action, const var_na *fmt, va_list args)
{
    const var_na *s = fmt;
    ကိန်းပြည့် uc;
    အတွက် (;; s++) {
        var_na c = GET_GLOBAL(*(u8*)s);
        အကယ်၍ (!c)
            bha_nga;
        အကယ်၍ (c != '%') {
            putc(action, c);
            continue;
        }
        const var_na *n = s+1;
        ကိန်းပြည့် field_width = 0;
        var_na padchar = ' ';
        u8 is64 = 0;
        အတွက် (;;) {
            c = GET_GLOBAL(*(u8*)n);
            အကယ်၍ (!isdigit(c))
                bha_nga;
            အကယ်၍ (!field_width && (c == '0'))
                padchar = '0';
            မဟုတ်ပါက
                field_width = field_width * 10 + c - '0';
            n++;
        }
        အကယ်၍ (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        အကယ်၍ (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        const var_na *sarg;
        switch (c) {
        pak_sa '%':
            putc(action, '%');
            bha_nga;
        pak_sa 'd':
            val = va_arg(args, s32);
            အကယ်၍ (is64)
                va_arg(args, s32);
            အကယ်၍ (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            bha_nga;
        pak_sa 'u':
            val = va_arg(args, s32);
            အကယ်၍ (is64)
                va_arg(args, s32);
            putuint(action, val);
            bha_nga;
        pak_sa 'p':
            val = va_arg(args, s32);
            အကယ်၍ (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (void*)val);
                n++;
                bha_nga;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            bha_nga;
        pak_sa 'X':
        pak_sa 'x':
            uc = (c == 'X');
            val = va_arg(args, s32);
            အကယ်၍ (is64) {
                u32 upper = va_arg(args, s32);
                အကယ်၍ (upper) {
                    putprettyhex(action, upper, field_width - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    bha_nga;
                }
            }
            putprettyhex(action, val, field_width, padchar, uc);
            bha_nga;
        pak_sa 'c':
            val = va_arg(args, ကိန်းပြည့်);
            putc(action, val);
            bha_nga;
        pak_sa '.':
            // Hack to support "%.s" - meaning string on stack.
            အကယ်၍ (GET_GLOBAL(*(u8*)(n+1)) != 's')
                bha_nga;
            n++;
            sarg = va_arg(args, const var_na *);
            puts(action, sarg);
            bha_nga;
        pak_sa 's':
            sarg = va_arg(args, const var_na *);
            puts_cs(action, sarg);
            bha_nga;
        default:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

void
panic(const var_na *fmt, ...)
{
    အကယ်၍ (CONFIG_DEBUG_LEVEL) {
        va_list args;
        va_start(args, fmt);
        bvprintf(&debuginfo, fmt, args);
        va_end(args);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    အတွက် (;;)
        hlt();
}

void
__dprintf(const var_na *fmt, ...)
{
    အကယ်၍ (!MODESEGMENT && CONFIG_THREADS && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        sa_nracanaa thread_info *cur = getCurThread();
        အကယ်၍ (cur != &MainThread) {
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

void
printf(const var_na *fmt, ...)
{
    ASSERT32FLAT();
    va_list args;
    va_start(args, fmt);
    bvprintf(&screeninfo, fmt, args);
    va_end(args);
    အကယ်၍ (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

sa_nracanaa snprintfinfo {
    sa_nracanaa putcinfo info;
    var_na *str, *end;
};

static void
putc_str(sa_nracanaa putcinfo *info, var_na c)
{
    sa_nracanaa snprintfinfo *sinfo = container_of(info, sa_nracanaa snprintfinfo, info);
    အကယ်၍ (sinfo->str >= sinfo->end)
        ပြန်ပေး;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
ကိန်းပြည့်
snprintf(var_na *str, size_t size, const var_na *fmt, ...)
{
    ASSERT32FLAT();
    အကယ်၍ (!size)
        ပြန်ပေး 0;
    sa_nracanaa snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    var_na *end = sinfo.str;
    အကယ်၍ (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ပြန်ပေး end - str;
}

// Build a formatted string - malloc'ing the memory.
var_na *
znprintf(size_t size, const var_na *fmt, ...)
{
    ASSERT32FLAT();
    အကယ်၍ (!size)
        ပြန်ပေး NULL;
    var_na *str = malloc_tmp(size);
    အကယ်၍ (!str) {
        warn_noalloc();
        ပြန်ပေး NULL;
    }
    sa_nracanaa snprintfinfo sinfo = { { putc_str }, str, str + size };
    va_list args;
    va_start(args, fmt);
    bvprintf(&sinfo.info, fmt, args);
    va_end(args);
    var_na *end = sinfo.str;
    အကယ်၍ (end >= sinfo.end)
        end = sinfo.end - 1;
    *end = '\0';
    ပြန်ပေး str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

void
hexdump(const void *d, ကိန်းပြည့် len)
{
    ကိန်းပြည့် count=0;
    နေစဉ် (len > 0) {
        အကယ်၍ (count % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, count*4, 8, 0);
            putc(&debuginfo, ':');
        } မဟုတ်ပါက {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        count++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

static void
dump_regs(sa_nracanaa bregs *regs)
{
    အကယ်၍ (!regs) {
        dprintf(1, "  NULL\n");
        ပြန်ပေး;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->code.seg, regs->code.offset, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
void
__debug_isr(const var_na *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
void
__debug_enter(sa_nracanaa bregs *regs, const var_na *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
void
__debug_stub(sa_nracanaa bregs *regs, ကိန်းပြည့် lineno, const var_na *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
void
__warn_invalid(sa_nracanaa bregs *regs, ကိန်းပြည့် lineno, const var_na *fname)
{
    အကယ်၍ (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
void
__warn_unimplemented(sa_nracanaa bregs *regs, ကိန်းပြည့် lineno, const var_na *fname)
{
    အကယ်၍ (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
void
__warn_internalerror(ကိန်းပြည့် lineno, const var_na *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
void
__warn_noalloc(ကိန်းပြည့် lineno, const var_na *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
void
__warn_timeout(ကိန်းပြည့် lineno, const var_na *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
void
__set_invalid(sa_nracanaa bregs *regs, ကိန်းပြည့် lineno, const var_na *fname)
{
    __warn_invalid(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a call of an unimplemented function.
void
__set_unimplemented(sa_nracanaa bregs *regs, ကိန်းပြည့် lineno, const var_na *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    set_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
void
__set_code_invalid(sa_nracanaa bregs *regs, u32 linecode, const var_na *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}

// Report a call of an unimplemented function.
void
__set_code_unimplemented(sa_nracanaa bregs *regs, u32 linecode, const var_na *fname)
{
    u8 code = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    set_code_invalid_silent(regs, code);
}
