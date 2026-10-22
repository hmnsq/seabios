// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_DEBUG_SERIAL
#अन्तर्भाव "fw/paravirt.h" // RunningOnQEMU
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "serialio.h" // serial_debug_preinit
#अन्तर्भाव "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
स्थिर शून्य
serial_debug_write(u8 offset, u8 val)
{
    सचे (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } अञ्ञथा सचे (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((शून्य*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
स्थिर u8
serial_debug_read(u8 offset)
{
    सचे (CONFIG_DEBUG_SERIAL)
        पटिनिवत्त inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    सचे (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        पटिनिवत्त readb((शून्य*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
शून्य
serial_debug_preinit(शून्य)
{
    सचे (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        पटिनिवत्त;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    सचे (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
स्थिर शून्य
serial_debug(वर्ण c)
{
    सचे (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        पटिनिवत्त;
    पूर्णाङ्क timeout = DEBUG_TIMEOUT;
    याव ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        सचे (!timeout--)
            // Ran out of time.
            पटिनिवत्त;
    serial_debug_write(SEROFF_DATA, c);
}

शून्य
serial_debug_putc(वर्ण c)
{
    सचे (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
शून्य
serial_debug_flush(शून्य)
{
    सचे (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        पटिनिवत्त;
    पूर्णाङ्क timeout = DEBUG_TIMEOUT;
    याव ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        सचे (!timeout--)
            // Ran out of time.
            पटिनिवत्त;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

शून्य
qemu_debug_preinit(शून्य)
{
    /* Xen doesn't support checking if debug output is active. */
    सचे (runningOnXen())
        पटिनिवत्त;

    /* Check if the QEMU debug output port is active */
    सचे (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
शून्य
qemu_debug_putc(वर्ण c)
{
    सचे (!CONFIG_DEBUG_IO || !runningOnQEMU())
        पटिनिवत्त;
    u16 port = GET_GLOBAL(DebugOutputPort);
    सचे (port)
        // Send character to debug port.
        outb(c, port);
}
