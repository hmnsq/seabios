// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_DEBUG_SERIAL
#അന്തര്ഭാവ "fw/paravirt.h" // RunningOnQEMU
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "serialio.h" // serial_debug_preinit
#അന്തര്ഭാവ "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
സ്ഥിതം ശൂന്യം
serial_debug_write(u8 offset, u8 val)
{
    എങ്കിൽ (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((ശൂന്യം*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
സ്ഥിതം u8
serial_debug_read(u8 offset)
{
    എങ്കിൽ (CONFIG_DEBUG_SERIAL)
        തിരികെ inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    എങ്കിൽ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        തിരികെ readb((ശൂന്യം*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
ശൂന്യം
serial_debug_preinit(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        തിരികെ;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    എങ്കിൽ (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
സ്ഥിതം ശൂന്യം
serial_debug(അക്ഷരം c)
{
    എങ്കിൽ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        തിരികെ;
    പൂർണ്ണസംഖ്യ timeout = DEBUG_TIMEOUT;
    ആകുമ്പോൾ ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        എങ്കിൽ (!timeout--)
            // Ran out of time.
            തിരികെ;
    serial_debug_write(SEROFF_DATA, c);
}

ശൂന്യം
serial_debug_putc(അക്ഷരം c)
{
    എങ്കിൽ (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
ശൂന്യം
serial_debug_flush(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        തിരികെ;
    പൂർണ്ണസംഖ്യ timeout = DEBUG_TIMEOUT;
    ആകുമ്പോൾ ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        എങ്കിൽ (!timeout--)
            // Ran out of time.
            തിരികെ;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

ശൂന്യം
qemu_debug_preinit(ശൂന്യം)
{
    /* Xen doesn't support checking if debug output is active. */
    എങ്കിൽ (runningOnXen())
        തിരികെ;

    /* Check if the QEMU debug output port is active */
    എങ്കിൽ (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
ശൂന്യം
qemu_debug_putc(അക്ഷരം c)
{
    എങ്കിൽ (!CONFIG_DEBUG_IO || !runningOnQEMU())
        തിരികെ;
    u16 port = GET_GLOBAL(DebugOutputPort);
    എങ്കിൽ (port)
        // Send character to debug port.
        outb(c, port);
}
