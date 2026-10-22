// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_DEBUG_SERIAL
#அந்தர்பாவ "fw/paravirt.h" // RunningOnQEMU
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "serialio.h" // serial_debug_preinit
#அந்தர்பாவ "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
நிலையான வெறுமை
serial_debug_write(u8 offset, u8 val)
{
    எனில் (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } இல்லையெனில் எனில் (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((வெறுமை*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
நிலையான u8
serial_debug_read(u8 offset)
{
    எனில் (CONFIG_DEBUG_SERIAL)
        திரும்பு inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    எனில் (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        திரும்பு readb((வெறுமை*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
வெறுமை
serial_debug_preinit(வெறுமை)
{
    எனில் (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        திரும்பு;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    எனில் (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
நிலையான வெறுமை
serial_debug(எழுத்து c)
{
    எனில் (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        திரும்பு;
    முழு timeout = DEBUG_TIMEOUT;
    போது ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        எனில் (!timeout--)
            // Ran out of time.
            திரும்பு;
    serial_debug_write(SEROFF_DATA, c);
}

வெறுமை
serial_debug_putc(எழுத்து c)
{
    எனில் (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
வெறுமை
serial_debug_flush(வெறுமை)
{
    எனில் (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        திரும்பு;
    முழு timeout = DEBUG_TIMEOUT;
    போது ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        எனில் (!timeout--)
            // Ran out of time.
            திரும்பு;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

வெறுமை
qemu_debug_preinit(வெறுமை)
{
    /* Xen doesn't support checking if debug output is active. */
    எனில் (runningOnXen())
        திரும்பு;

    /* Check if the QEMU debug output port is active */
    எனில் (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
வெறுமை
qemu_debug_putc(எழுத்து c)
{
    எனில் (!CONFIG_DEBUG_IO || !runningOnQEMU())
        திரும்பு;
    u16 port = GET_GLOBAL(DebugOutputPort);
    எனில் (port)
        // Send character to debug port.
        outb(c, port);
}
