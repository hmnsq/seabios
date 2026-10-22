// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_DEBUG_SERIAL
#समावेश "fw/paravirt.h" // RunningOnQEMU
#समावेश "output.h" // dprintf
#समावेश "serialio.h" // serial_debug_preinit
#समावेश "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#घोषणा DEBUG_TIMEOUT 100000

// Write to a serial port register
अटल व्योम
serial_debug_write(u8 offset, u8 val)
{
    अगर (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } अन्यथा अगर (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((व्योम*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
अटल u8
serial_debug_read(u8 offset)
{
    अगर (CONFIG_DEBUG_SERIAL)
        वापस inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    अगर (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        वापस readb((व्योम*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
व्योम
serial_debug_preinit(व्योम)
{
    अगर (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        वापस;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    अगर (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
अटल व्योम
serial_debug(अक्षर c)
{
    अगर (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        वापस;
    पूर्णांक timeout = DEBUG_TIMEOUT;
    जबतक ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        अगर (!timeout--)
            // Ran out of time.
            वापस;
    serial_debug_write(SEROFF_तथ्य, c);
}

व्योम
serial_debug_putc(अक्षर c)
{
    अगर (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
व्योम
serial_debug_flush(व्योम)
{
    अगर (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        वापस;
    पूर्णांक timeout = DEBUG_TIMEOUT;
    जबतक ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        अगर (!timeout--)
            // Ran out of time.
            वापस;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

व्योम
qemu_debug_preinit(व्योम)
{
    /* Xen doesn't support checking if debug output is active. */
    अगर (runningOnXen())
        वापस;

    /* Check if the QEMU debug output port is active */
    अगर (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
व्योम
qemu_debug_putc(अक्षर c)
{
    अगर (!CONFIG_DEBUG_IO || !runningOnQEMU())
        वापस;
    u16 port = GET_GLOBAL(DebugOutputPort);
    अगर (port)
        // Send character to debug port.
        outb(c, port);
}
