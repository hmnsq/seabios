// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_DEBUG_SERIAL
#অন্তর্ভাব "fw/paravirt.h" // RunningOnQEMU
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "serialio.h" // serial_debug_preinit
#অন্তর্ভাব "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
স্থির শূন্য
serial_debug_write(u8 offset, u8 val)
{
    যদি (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } অন্যথা যদি (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((শূন্য*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
স্থির u8
serial_debug_read(u8 offset)
{
    যদি (CONFIG_DEBUG_SERIAL)
        ফেরত inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    যদি (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        ফেরত readb((শূন্য*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
শূন্য
serial_debug_preinit(শূন্য)
{
    যদি (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ফেরত;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    যদি (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
স্থির শূন্য
serial_debug(অক্ষর c)
{
    যদি (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ফেরত;
    পূর্ণসংখ্যা timeout = DEBUG_TIMEOUT;
    যখন ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        যদি (!timeout--)
            // Ran out of time.
            ফেরত;
    serial_debug_write(SEROFF_তথ্য, c);
}

শূন্য
serial_debug_putc(অক্ষর c)
{
    যদি (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
শূন্য
serial_debug_flush(শূন্য)
{
    যদি (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ফেরত;
    পূর্ণসংখ্যা timeout = DEBUG_TIMEOUT;
    যখন ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        যদি (!timeout--)
            // Ran out of time.
            ফেরত;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

শূন্য
qemu_debug_preinit(শূন্য)
{
    /* Xen doesn't support checking if debug output is active. */
    যদি (runningOnXen())
        ফেরত;

    /* Check if the QEMU debug output port is active */
    যদি (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
শূন্য
qemu_debug_putc(অক্ষর c)
{
    যদি (!CONFIG_DEBUG_IO || !runningOnQEMU())
        ফেরত;
    u16 port = GET_GLOBAL(DebugOutputPort);
    যদি (port)
        // Send character to debug port.
        outb(c, port);
}
