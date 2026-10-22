// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_DEBUG_SERIAL
#شامل "fw/paravirt.h" // RunningOnQEMU
#شامل "output.h" // dprintf
#شامل "serialio.h" // serial_debug_preinit
#شامل "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
ایستا تهی
serial_debug_write(u8 offset, u8 val)
{
    اگر (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } وگرنه اگر (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((تهی*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
ایستا u8
serial_debug_read(u8 offset)
{
    اگر (CONFIG_DEBUG_SERIAL)
        بازگشت inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    اگر (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        بازگشت readb((تهی*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
تهی
serial_debug_preinit(تهی)
{
    اگر (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        بازگشت;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    اگر (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
ایستا تهی
serial_debug(نویسه c)
{
    اگر (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        بازگشت;
    صحیح timeout = DEBUG_TIMEOUT;
    وقتی ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        اگر (!timeout--)
            // Ran out of time.
            بازگشت;
    serial_debug_write(SEROFF_DATA, c);
}

تهی
serial_debug_putc(نویسه c)
{
    اگر (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
تهی
serial_debug_flush(تهی)
{
    اگر (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        بازگشت;
    صحیح timeout = DEBUG_TIMEOUT;
    وقتی ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        اگر (!timeout--)
            // Ran out of time.
            بازگشت;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

تهی
qemu_debug_preinit(تهی)
{
    /* Xen doesn't support checking if debug output is active. */
    اگر (runningOnXen())
        بازگشت;

    /* Check if the QEMU debug output port is active */
    اگر (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
تهی
qemu_debug_putc(نویسه c)
{
    اگر (!CONFIG_DEBUG_IO || !runningOnQEMU())
        بازگشت;
    u16 port = GET_GLOBAL(DebugOutputPort);
    اگر (port)
        // Send character to debug port.
        outb(c, port);
}
