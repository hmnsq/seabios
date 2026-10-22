// Raw screen writing and debug output code.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ <stdarg.h> // va_list

#ಅನ್ತರ್ಭಾವ "farptr.h" // GET_VAR
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_bdf_to_bus
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_device
#ಅನ್ತರ್ಭಾವ "hw/serialio.h" // serial_debug_putc
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_tmp
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // call16_int
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // ScreenAndDebug

ರಚನೆ putcinfo {
    ಶೂನ್ಯ (*func)(ರಚನೆ putcinfo *info, ಅಕ್ಷರ c);
};


/****************************************************************
 * Debug output
 ****************************************************************/

ಶೂನ್ಯ
debug_banner(ಶೂನ್ಯ)
{
    dprintf(1, "SeaBIOS (version %s)\n", ಆವೃತ್ತಿ);
    dprintf(1, "BUILD: %s\n", BUILDINFO);
}

// Write a character to debug port(s).
ಸ್ಥಿರ ಶೂನ್ಯ
debug_putc(ರಚನೆ putcinfo *action, ಅಕ್ಷರ c)
{
    ಆದರೆ (! CONFIG_DEBUG_LEVEL)
        ಹಿಂತಿರುಗಿ;
    qemu_debug_putc(c);
    ಆದರೆ (!MODESEGMENT)
        coreboot_debug_putc(c);
    serial_debug_putc(c);
}

// Flush any pending output to debug port(s).
ಸ್ಥಿರ ಶೂನ್ಯ
debug_flush(ಶೂನ್ಯ)
{
    serial_debug_flush();
}

// In segmented mode just need a dummy variable (debug_putc is always
// used anyway), and in 32bit flat mode need a pointer to the 32bit
// instance of debug_putc().
#ಆದರೆ MODE16
ಸ್ಥಿರ ರಚನೆ putcinfo debuginfo VAR16;
#ಅಥವಾಆದರೆ MODESEGMENT
ಸ್ಥಿರ ರಚನೆ putcinfo debuginfo VAR32SEG;
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ರಚನೆ putcinfo debuginfo = { debug_putc };
#ಆದರೆ_ಅಂತ್ಯ


/****************************************************************
 * Screen writing
 ****************************************************************/

// Show a character on the screen.
ಸ್ಥಿರ ಶೂನ್ಯ
screenc(ಅಕ್ಷರ c)
{
    ಆದರೆ (!MODESEGMENT && GET_IVT(0x10).segoff == FUNC16(ನಮೂದು_10).segoff)
        // No need to thunk to 16bit mode if vgabios is not present
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    br.ah = 0x0e;
    br.al = c;
    br.bl = 0x07;
    call16_int(0x10, &br);
}

// Handle a character from a printf request.
ಸ್ಥಿರ ಶೂನ್ಯ
screen_putc(ರಚನೆ putcinfo *action, ಅಕ್ಷರ c)
{
    ಆದರೆ (ScreenAndDebug)
        debug_putc(&debuginfo, c);
    ಆದರೆ (c == '\n')
        screenc('\r');
    screenc(c);
}

ಸ್ಥಿರ ರಚನೆ putcinfo screeninfo = { screen_putc };


/****************************************************************
 * Xprintf code
 ****************************************************************/

// Output a character.
ಸ್ಥಿರ ಶೂನ್ಯ
putc(ರಚನೆ putcinfo *action, ಅಕ್ಷರ c)
{
    ಆದರೆ (MODESEGMENT) {
        // Only debugging output supported in segmented mode.
        debug_putc(action, c);
        ಹಿಂತಿರುಗಿ;
    }

    ಶೂನ್ಯ (*func)(ರಚನೆ putcinfo *info, ಅಕ್ಷರ c) = GET_GLOBAL(action->func);
    func(action, c);
}

// Ouptut a string.
ಸ್ಥಿರ ಶೂನ್ಯ
puts(ರಚನೆ putcinfo *action, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
    ಆದರೆ (!MODESEGMENT && !s)
        s = "(NULL)";
    ಗಾಗಿ (; *s; s++)
        putc(action, *s);
}

// Output a string that is in the CS segment.
ಸ್ಥಿರ ಶೂನ್ಯ
puts_cs(ರಚನೆ putcinfo *action, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
    ಅಕ್ಷರ *vs = (ಅಕ್ಷರ*)s;
    ಗಾಗಿ (;; vs++) {
        ಅಕ್ಷರ c = GET_GLOBAL(*vs);
        ಆದರೆ (!c)
            ನಿಲ್ಲಿಸು;
        putc(action, c);
    }
}

// Output an unsigned integer.
ಸ್ಥಿರ ಶೂನ್ಯ
putuint(ರಚನೆ putcinfo *action, u32 val)
{
    ಅಕ್ಷರ buf[12];
    ಅಕ್ಷರ *d = &buf[ಗಾತ್ರ(buf) - 1];
    *d-- = '\0';
    ಗಾಗಿ (;;) {
        *d = (val % 10) + '0';
        val /= 10;
        ಆದರೆ (!val)
            ನಿಲ್ಲಿಸು;
        d--;
    }
    puts(action, d);
}

// Output a single digit hex character.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
putsinglehex(ರಚನೆ putcinfo *action, u32 val, ಪೂರ್ಣಾಂಕ uc)
{
    ಆದರೆ (val <= 9)
        val = '0' + val;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (uc)
        val = 'A' + val - 10;
    ಇಲ್ಲದಿದ್ದರೆ
        val = 'a' + val - 10;
    putc(action, val);
}

// Output an integer in hexadecimal with a specified width.
ಸ್ಥಿರ ಶೂನ್ಯ
puthex(ರಚನೆ putcinfo *action, u32 val, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ uc)
{
    ಆಯ್ಕೆ (ಅಗಲ) {
    ಪೂರ್ವನಿಯೋಜಿತ: putsinglehex(action, (val >> 28) & 0xf, uc);
    ಸ್ಥಿತಿ 7:  putsinglehex(action, (val >> 24) & 0xf, uc);
    ಸ್ಥಿತಿ 6:  putsinglehex(action, (val >> 20) & 0xf, uc);
    ಸ್ಥಿತಿ 5:  putsinglehex(action, (val >> 16) & 0xf, uc);
    ಸ್ಥಿತಿ 4:  putsinglehex(action, (val >> 12) & 0xf, uc);
    ಸ್ಥಿತಿ 3:  putsinglehex(action, (val >> 8) & 0xf, uc);
    ಸ್ಥಿತಿ 2:  putsinglehex(action, (val >> 4) & 0xf, uc);
    ಸ್ಥಿತಿ 1:  putsinglehex(action, (val >> 0) & 0xf, uc);
    }
}

// Output an integer in hexadecimal with a minimum width.
ಸ್ಥಿರ ಶೂನ್ಯ
putprettyhex(ರಚನೆ putcinfo *action, u32 val, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಅಕ್ಷರ padchar, ಪೂರ್ಣಾಂಕ uc)
{
    u32 tmp = val;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 1;
    ಆಗ (tmp >>= 4)
        ಎಣಿಕೆ_2++;
    ಅಗಲ -= ಎಣಿಕೆ_2;
    ಆಗ (ಅಗಲ-- > 0)
        putc(action, padchar);
    puthex(action, val, ಎಣಿಕೆ_2, uc);
}

// Output 'struct pci_device' BDF as %02x:%02x.%x
ಸ್ಥಿರ ಶೂನ್ಯ
put_pci_device(ರಚನೆ putcinfo *action, ರಚನೆ pci_device *pci)
{
    puthex(action, pci_bdf_ಗಮ್ಯ_bus(pci->bdf), 2, 0);
    putc(action, ':');
    puthex(action, pci_bdf_ಗಮ್ಯ_dev(pci->bdf), 2, 0);
    putc(action, '.');
    puthex(action, pci_bdf_ಗಮ್ಯ_fn(pci->bdf), 1, 0);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
ಅಂಕಿಯೇ(u8 c)
{
    ಹಿಂತಿರುಗಿ ((u8)(c - '0')) < 10;
}

ಸ್ಥಿರ ಶೂನ್ಯ
bvprintf(ರಚನೆ putcinfo *action, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, va_list ವಾದಗಳು)
{
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = fmt;
    ಪೂರ್ಣಾಂಕ uc;
    ಗಾಗಿ (;; s++) {
        ಅಕ್ಷರ c = GET_GLOBAL(*(u8*)s);
        ಆದರೆ (!c)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (c != '%') {
            putc(action, c);
            ಮುಂದುವರಿ;
        }
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *n = s+1;
        ಪೂರ್ಣಾಂಕ ಕ್ಷೇತ್ರ_ಅಗಲ = 0;
        ಅಕ್ಷರ padchar = ' ';
        u8 is64 = 0;
        ಗಾಗಿ (;;) {
            c = GET_GLOBAL(*(u8*)n);
            ಆದರೆ (!ಅಂಕಿಯೇ(c))
                ನಿಲ್ಲಿಸು;
            ಆದರೆ (!ಕ್ಷೇತ್ರ_ಅಗಲ && (c == '0'))
                padchar = '0';
            ಇಲ್ಲದಿದ್ದರೆ
                ಕ್ಷೇತ್ರ_ಅಗಲ = ಕ್ಷೇತ್ರ_ಅಗಲ * 10 + c - '0';
            n++;
        }
        ಆದರೆ (c == 'l') {
            // Ignore long format indicator
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        ಆದರೆ (c == 'l') {
            is64 = 1;
            n++;
            c = GET_GLOBAL(*(u8*)n);
        }
        s32 val;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sarg;
        ಆಯ್ಕೆ (c) {
        ಸ್ಥಿತಿ '%':
            putc(action, '%');
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 'd':
            val = va_arg(ವಾದಗಳು, s32);
            ಆದರೆ (is64)
                va_arg(ವಾದಗಳು, s32);
            ಆದರೆ (val < 0) {
                putc(action, '-');
                val = -val;
            }
            putuint(action, val);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 'u':
            val = va_arg(ವಾದಗಳು, s32);
            ಆದರೆ (is64)
                va_arg(ವಾದಗಳು, s32);
            putuint(action, val);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 'p':
            val = va_arg(ವಾದಗಳು, s32);
            ಆದರೆ (!MODESEGMENT && GET_GLOBAL(*(u8*)(n+1)) == 'P') {
                // %pP is 'struct pci_device' printer
                put_pci_device(action, (ಶೂನ್ಯ*)val);
                n++;
                ನಿಲ್ಲಿಸು;
            }
            putc(action, '0');
            putc(action, 'x');
            puthex(action, val, 8, 0);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 'X':
        ಸ್ಥಿತಿ 'x':
            uc = (c == 'X');
            val = va_arg(ವಾದಗಳು, s32);
            ಆದರೆ (is64) {
                u32 ಊರ್ಧ್ವ = va_arg(ವಾದಗಳು, s32);
                ಆದರೆ (ಊರ್ಧ್ವ) {
                    putprettyhex(action, ಊರ್ಧ್ವ, ಕ್ಷೇತ್ರ_ಅಗಲ - 8, padchar, uc);
                    puthex(action, val, 8, uc);
                    ನಿಲ್ಲಿಸು;
                }
            }
            putprettyhex(action, val, ಕ್ಷೇತ್ರ_ಅಗಲ, padchar, uc);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 'c':
            val = va_arg(ವಾದಗಳು, ಪೂರ್ಣಾಂಕ);
            putc(action, val);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ '.':
            // Hack to support "%.s" - meaning string on stack.
            ಆದರೆ (GET_GLOBAL(*(u8*)(n+1)) != 's')
                ನಿಲ್ಲಿಸು;
            n++;
            sarg = va_arg(ವಾದಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
            puts(action, sarg);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 's':
            sarg = va_arg(ವಾದಗಳು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
            puts_cs(action, sarg);
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            putc(action, '%');
            n = s;
        }
        s = n;
    }
}

ಶೂನ್ಯ
panic(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
    ಆದರೆ (CONFIG_DEBUG_LEVEL) {
        va_list ವಾದಗಳು;
        va_start(ವಾದಗಳು, fmt);
        bvprintf(&debuginfo, fmt, ವಾದಗಳು);
        va_end(ವಾದಗಳು);
        debug_flush();
    }

    // XXX - use PANIC PORT.
    irq_disable();
    ಗಾಗಿ (;;)
        hlt();
}

ಶೂನ್ಯ
__dprintf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
    ಆದರೆ (!MODESEGMENT && CONFIG_ಎಳೆಗಳು && CONFIG_DEBUG_LEVEL >= DEBUG_thread
        && *fmt != '\\' && *fmt != '/') {
        ರಚನೆ thread_info *cur = getCurThread();
        ಆದರೆ (cur != &MainThread) {
            // Show "thread id" for this debug message.
            debug_putc(&debuginfo, '|');
            puthex(&debuginfo, (u32)cur, 8, 0);
            debug_putc(&debuginfo, '|');
            debug_putc(&debuginfo, ' ');
        }
    }

    va_list ವಾದಗಳು;
    va_start(ವಾದಗಳು, fmt);
    bvprintf(&debuginfo, fmt, ವಾದಗಳು);
    va_end(ವಾದಗಳು);
    debug_flush();
}

ಶೂನ್ಯ
ಬರೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
    ASSERT32FLAT();
    va_list ವಾದಗಳು;
    va_start(ವಾದಗಳು, fmt);
    bvprintf(&screeninfo, fmt, ವಾದಗಳು);
    va_end(ವಾದಗಳು);
    ಆದರೆ (ScreenAndDebug)
        debug_flush();
}


/****************************************************************
 * snprintf
 ****************************************************************/

ರಚನೆ snprintfinfo {
    ರಚನೆ putcinfo info;
    ಅಕ್ಷರ *str, *ಕೊನೆ;
};

ಸ್ಥಿರ ಶೂನ್ಯ
putc_str(ರಚನೆ putcinfo *info, ಅಕ್ಷರ c)
{
    ರಚನೆ snprintfinfo *sinfo = container_ದ(info, ರಚನೆ snprintfinfo, info);
    ಆದರೆ (sinfo->str >= sinfo->ಕೊನೆ)
        ಹಿಂತಿರುಗಿ;
    *sinfo->str = c;
    sinfo->str++;
}

// Build a formatted string.  Note, this function returns the actual
// number of bytes used (not including null) even in the overflow
// case.
ಪೂರ್ಣಾಂಕ
snprintf(ಅಕ್ಷರ *str, ಗಾತ್ರ_t ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
    ASSERT32FLAT();
    ಆದರೆ (!ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ 0;
    ರಚನೆ snprintfinfo sinfo = { { putc_str }, str, str + ಗಾತ್ರ_2 };
    va_list ವಾದಗಳು;
    va_start(ವಾದಗಳು, fmt);
    bvprintf(&sinfo.info, fmt, ವಾದಗಳು);
    va_end(ವಾದಗಳು);
    ಅಕ್ಷರ *ಕೊನೆ = sinfo.str;
    ಆದರೆ (ಕೊನೆ >= sinfo.ಕೊನೆ)
        ಕೊನೆ = sinfo.ಕೊನೆ - 1;
    *ಕೊನೆ = '\0';
    ಹಿಂತಿರುಗಿ ಕೊನೆ - str;
}

// Build a formatted string - malloc'ing the memory.
ಅಕ್ಷರ *
znprintf(ಗಾತ್ರ_t ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
    ASSERT32FLAT();
    ಆದರೆ (!ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಅಕ್ಷರ *str = malloc_tmp(ಗಾತ್ರ_2);
    ಆದರೆ (!str) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ರಚನೆ snprintfinfo sinfo = { { putc_str }, str, str + ಗಾತ್ರ_2 };
    va_list ವಾದಗಳು;
    va_start(ವಾದಗಳು, fmt);
    bvprintf(&sinfo.info, fmt, ವಾದಗಳು);
    va_end(ವಾದಗಳು);
    ಅಕ್ಷರ *ಕೊನೆ = sinfo.str;
    ಆದರೆ (ಕೊನೆ >= sinfo.ಕೊನೆ)
        ಕೊನೆ = sinfo.ಕೊನೆ - 1;
    *ಕೊನೆ = '\0';
    ಹಿಂತಿರುಗಿ str;
}


/****************************************************************
 * Misc helpers
 ****************************************************************/

ಶೂನ್ಯ
hexdump(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *d, ಪೂರ್ಣಾಂಕ len)
{
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2=0;
    ಆಗ (len > 0) {
        ಆದರೆ (ಎಣಿಕೆ_2 % 8 == 0) {
            putc(&debuginfo, '\n');
            puthex(&debuginfo, ಎಣಿಕೆ_2*4, 8, 0);
            putc(&debuginfo, ':');
        } ಇಲ್ಲದಿದ್ದರೆ {
            putc(&debuginfo, ' ');
        }
        puthex(&debuginfo, *(u32*)d, 8, 0);
        ಎಣಿಕೆ_2++;
        len-=4;
        d+=4;
    }
    putc(&debuginfo, '\n');
    debug_flush();
}

ಸ್ಥಿರ ಶೂನ್ಯ
dump_regs(ರಚನೆ bregs *regs)
{
    ಆದರೆ (!regs) {
        dprintf(1, "  NULL\n");
        ಹಿಂತಿರುಗಿ;
    }
    dprintf(1, "   a=%08x  b=%08x  c=%08x  d=%08x ds=%04x es=%04x ss=%04x\n"
            , regs->eax, regs->ebx, regs->ecx, regs->edx
            , regs->ds, regs->es, GET_SEG(SS));
    dprintf(1, "  si=%08x di=%08x bp=%08x sp=%08x cs=%04x ip=%04x  f=%04x\n"
            , regs->esi, regs->edi, regs->ebp, (u32)&regs[1]
            , regs->ಸಂಕೇತ_2.seg, regs->ಸಂಕೇತ_2.ಸ್ಥಾನಾಂತರ_2, regs->flags);
}

// Report entry to an Interrupt Service Routine (ISR).
ಶೂನ್ಯ
__debug_isr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    puts_cs(&debuginfo, fname);
    putc(&debuginfo, '\n');
    debug_flush();
}

// Function called on handler startup.
ಶೂನ್ಯ
__debug_enter(ರಚನೆ bregs *regs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    dprintf(1, "enter %s:\n", fname);
    dump_regs(regs);
}

// Send debugging output info.
ಶೂನ್ಯ
__debug_stub(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    dprintf(1, "stub %s:%d:\n", fname, lineno);
    dump_regs(regs);
}

// Report on an invalid parameter.
ಶೂನ್ಯ
__warn_invalid(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    ಆದರೆ (CONFIG_DEBUG_LEVEL >= DEBUG_invalid) {
        dprintf(1, "invalid %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report on an unimplemented feature.
ಶೂನ್ಯ
__warn_unimplemented(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    ಆದರೆ (CONFIG_DEBUG_LEVEL >= DEBUG_unimplemented) {
        dprintf(1, "unimplemented %s:%d:\n", fname, lineno);
        dump_regs(regs);
    }
}

// Report a detected internal inconsistency.
ಶೂನ್ಯ
__warn_internalerror(ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    dprintf(1, "WARNING - internal error detected at %s:%d!\n"
            , fname, lineno);
}

// Report on an allocation failure.
ಶೂನ್ಯ
__warn_noalloc(ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    dprintf(1, "WARNING - Unable to allocate resource at %s:%d!\n"
            , fname, lineno);
}

// Report on a timeout exceeded.
ಶೂನ್ಯ
__warn_timeout(ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    dprintf(1, "WARNING - Timeout at %s:%d!\n", fname, lineno);
}

// Report a handler reporting an invalid parameter to the caller.
ಶೂನ್ಯ
__set_invalid(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    __warn_invalid(regs, lineno, fname);
    ಗಣ_invalid_silent(regs);
}

// Report a call of an unimplemented function.
ಶೂನ್ಯ
__set_unimplemented(ರಚನೆ bregs *regs, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    __warn_unimplemented(regs, lineno, fname);
    ಗಣ_invalid_silent(regs);
}

// Report a handler reporting an invalid parameter code to the
// caller.  Note, the lineno and return code are encoded in the same
// parameter as gcc does a better job of scheduling function calls
// when there are 3 or less parameters.
ಶೂನ್ಯ
__set_code_invalid(ರಚನೆ bregs *regs, u32 linecode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    u8 ಸಂಕೇತ_2 = linecode;
    u32 lineno = linecode >> 8;
    __warn_invalid(regs, lineno, fname);
    ಗಣ_ಸಂಕೇತ_invalid_silent(regs, ಸಂಕೇತ_2);
}

// Report a call of an unimplemented function.
ಶೂನ್ಯ
__set_code_unimplemented(ರಚನೆ bregs *regs, u32 linecode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname)
{
    u8 ಸಂಕೇತ_2 = linecode;
    u32 lineno = linecode >> 8;
    __warn_unimplemented(regs, lineno, fname);
    ಗಣ_ಸಂಕೇತ_invalid_silent(regs, ಸಂಕೇತ_2);
}
