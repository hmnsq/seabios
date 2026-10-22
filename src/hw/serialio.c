// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_DEBUG_SERIAL
#ضمن "fw/paravirt.h" // RunningOnQEMU
#ضمن "output.h" // dprintf
#ضمن "serialio.h" // serial_debug_preinit
#ضمن "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
ساكن فراغ
serial_debug_write(u8 offset, u8 val)
{
    إذا (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } وإلا إذا (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((فراغ*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
ساكن u8
serial_debug_read(u8 offset)
{
    إذا (CONFIG_DEBUG_SERIAL)
        أرجع inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    إذا (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        أرجع readb((فراغ*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
فراغ
serial_debug_preinit(فراغ)
{
    إذا (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        أرجع;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    إذا (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
ساكن فراغ
serial_debug(حرف c)
{
    إذا (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        أرجع;
    صحيح timeout = DEBUG_TIMEOUT;
    طالما ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        إذا (!timeout--)
            // Ran out of time.
            أرجع;
    serial_debug_write(SEROFF_DATA, c);
}

فراغ
serial_debug_putc(حرف c)
{
    إذا (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
فراغ
serial_debug_flush(فراغ)
{
    إذا (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        أرجع;
    صحيح timeout = DEBUG_TIMEOUT;
    طالما ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        إذا (!timeout--)
            // Ran out of time.
            أرجع;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

فراغ
qemu_debug_preinit(فراغ)
{
    /* Xen doesn't support checking if debug output is active. */
    إذا (runningOnXen())
        أرجع;

    /* Check if the QEMU debug output port is active */
    إذا (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
فراغ
qemu_debug_putc(حرف c)
{
    إذا (!CONFIG_DEBUG_IO || !runningOnQEMU())
        أرجع;
    u16 port = GET_GLOBAL(DebugOutputPort);
    إذا (port)
        // Send character to debug port.
        outb(c, port);
}
