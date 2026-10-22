// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_DEBUG_SERIAL
#અન્તર્ભાવ "fw/paravirt.h" // RunningOnQEMU
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "serialio.h" // serial_debug_preinit
#અન્તર્ભાવ "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
સ્થિર શૂન્ય
serial_debug_write(u8 offset, u8 val)
{
    જો (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } નહીંતર જો (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((શૂન્ય*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
સ્થિર u8
serial_debug_read(u8 offset)
{
    જો (CONFIG_DEBUG_SERIAL)
        પાછા inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    જો (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        પાછા readb((શૂન્ય*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
શૂન્ય
serial_debug_preinit(શૂન્ય)
{
    જો (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        પાછા;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    જો (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
સ્થિર શૂન્ય
serial_debug(અક્ષર c)
{
    જો (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        પાછા;
    પૂર્ણાંક timeout = DEBUG_TIMEOUT;
    જ્યારે ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        જો (!timeout--)
            // Ran out of time.
            પાછા;
    serial_debug_write(SEROFF_DATA, c);
}

શૂન્ય
serial_debug_putc(અક્ષર c)
{
    જો (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
શૂન્ય
serial_debug_flush(શૂન્ય)
{
    જો (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        પાછા;
    પૂર્ણાંક timeout = DEBUG_TIMEOUT;
    જ્યારે ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        જો (!timeout--)
            // Ran out of time.
            પાછા;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

શૂન્ય
qemu_debug_preinit(શૂન્ય)
{
    /* Xen doesn't support checking if debug output is active. */
    જો (runningOnXen())
        પાછા;

    /* Check if the QEMU debug output port is active */
    જો (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
શૂન્ય
qemu_debug_putc(અક્ષર c)
{
    જો (!CONFIG_DEBUG_IO || !runningOnQEMU())
        પાછા;
    u16 port = GET_GLOBAL(DebugOutputPort);
    જો (port)
        // Send character to debug port.
        outb(c, port);
}
