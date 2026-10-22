// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_DEBUG_SERIAL
#ଅନ୍ତର୍ଭାଵ "fw/paravirt.h" // RunningOnQEMU
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "serialio.h" // serial_debug_preinit
#ଅନ୍ତର୍ଭାଵ "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#define DEBUG_TIMEOUT 100000

// Write to a serial port register
ସ୍ଥିର ଶୂନ୍ୟ
serial_debug_write(u8 offset, u8 val)
{
    ଯଦି (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_PORT + offset);
    } ଅନ୍ୟଥା ଯଦି (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((ଶୂନ୍ୟ*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset, val);
    }
}

// Read from a serial port register
ସ୍ଥିର u8
serial_debug_read(u8 offset)
{
    ଯଦି (CONFIG_DEBUG_SERIAL)
        ଫେରନ୍ତୁ inb(CONFIG_DEBUG_SERIAL_PORT + offset);
    ଯଦି (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        ଫେରନ୍ତୁ readb((ଶୂନ୍ୟ*)CONFIG_DEBUG_SERIAL_MEM_ADDRESS + 4*offset);
    }
}

// Setup the debug serial port for output.
ଶୂନ୍ୟ
serial_debug_preinit(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ଫେରନ୍ତୁ;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    ଯଦି (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
ସ୍ଥିର ଶୂନ୍ୟ
serial_debug(ଅକ୍ଷର c)
{
    ଯଦି (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout = DEBUG_TIMEOUT;
    ଯେତେବେଳେ ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        ଯଦି (!timeout--)
            // Ran out of time.
            ଫେରନ୍ତୁ;
    serial_debug_write(SEROFF_DATA, c);
}

ଶୂନ୍ୟ
serial_debug_putc(ଅକ୍ଷର c)
{
    ଯଦି (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
ଶୂନ୍ୟ
serial_debug_flush(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout = DEBUG_TIMEOUT;
    ଯେତେବେଳେ ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        ଯଦି (!timeout--)
            // Ran out of time.
            ଫେରନ୍ତୁ;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 DebugOutputPort VARFSEG = 0x402;

ଶୂନ୍ୟ
qemu_debug_preinit(ଶୂନ୍ୟ)
{
    /* Xen doesn't support checking if debug output is active. */
    ଯଦି (runningOnXen())
        ଫେରନ୍ତୁ;

    /* Check if the QEMU debug output port is active */
    ଯଦି (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(DebugOutputPort)) != QEMU_DEBUGCON_READBACK)
        DebugOutputPort = 0;
}

// Write a character to the special debugging port.
ଶୂନ୍ୟ
qemu_debug_putc(ଅକ୍ଷର c)
{
    ଯଦି (!CONFIG_DEBUG_IO || !runningOnQEMU())
        ଫେରନ୍ତୁ;
    u16 port = GET_GLOBAL(DebugOutputPort);
    ଯଦି (port)
        // Send character to debug port.
        outb(c, port);
}
