// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_DEBUG_SERIAL
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // RunningOnQEMU
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "serialio.h" // serial_debug_preinit
#ಅನ್ತರ್ಭಾವ "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
ಸ್ಥಿರ ಶೂನ್ಯ
serial_debug_write(u8 offset, u8 val)
{
    ಯದಿ (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } ಅನ್ಯಥಾ ಯದಿ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((ಶೂನ್ಯ*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
ಸ್ಥಿರ u8
serial_debug_read(u8 offset)
{
    ಯದಿ (CONFIG_DEBUG_SERIAL)
        ಪ್ರತ್ಯಾಗಮ inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    ಯದಿ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        ಪ್ರತ್ಯಾಗಮ readb((ಶೂನ್ಯ*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
ಶೂನ್ಯ
serial_debug_preinit(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ಪ್ರತ್ಯಾಗಮ;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    ಯದಿ (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
ಸ್ಥಿರ ಶೂನ್ಯ
serial_debug(ವರ್ಣ c)
{
    ಯದಿ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ timeout = DEBUG_TIMEOUT;
    ಯಾವತ್ ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        ಯದಿ (!timeout--)
            // Ran out of time.
            ಪ್ರತ್ಯಾಗಮ;
    serial_debug_write(SEROFF_DATA, c);
}

ಶೂನ್ಯ
serial_debug_putc(ವರ್ಣ c)
{
    ಯದಿ (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
ಶೂನ್ಯ
serial_debug_flush(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ timeout = DEBUG_TIMEOUT;
    ಯಾವತ್ ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        ಯದಿ (!timeout--)
            // Ran out of time.
            ಪ್ರತ್ಯಾಗಮ;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

ಶೂನ್ಯ
qemu_debug_preinit(ಶೂನ್ಯ)
{
    /* Xen doesn't support checking if debug output is active. */
    ಯದಿ (runningOnXen())
        ಪ್ರತ್ಯಾಗಮ;

    /* Check if the QEMU debug output port is active */
    ಯದಿ (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
ಶೂನ್ಯ
qemu_debug_putc(ವರ್ಣ c)
{
    ಯದಿ (!CONFIG_DEBUG_IO || !runningOnQEMU())
        ಪ್ರತ್ಯಾಗಮ;
    u16 port = GET_GLOBAL(DebugOutputPort);
    ಯದಿ (port)
        // Send character to debug port.
        outb(c, port);
}
