// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_DEBUG_SERIAL
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // RunningOnQEMU
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "serialio.h" // serial_debug_preinit
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TIMEOUT 100000

// Write to a serial port register
ਸਥਿਰ ਖਾਲੀ
serial_debug_write(u8 ਵਿੱਥ_2, u8 val)
{
    ਜੇ (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_ਪੋਰਟ + ਵਿੱਥ_2);
    } ਨਹੀਂਤਾਂ ਜੇ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((ਖਾਲੀ*)CONFIG_DEBUG_SERIAL_MEM_ਪਤਾ + 4*ਵਿੱਥ_2, val);
    }
}

// Read from a serial port register
ਸਥਿਰ u8
serial_debug_read(u8 ਵਿੱਥ_2)
{
    ਜੇ (CONFIG_DEBUG_SERIAL)
        ਵਾਪਸ inb(CONFIG_DEBUG_SERIAL_ਪੋਰਟ + ਵਿੱਥ_2);
    ਜੇ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        ਵਾਪਸ readb((ਖਾਲੀ*)CONFIG_DEBUG_SERIAL_MEM_ਪਤਾ + 4*ਵਿੱਥ_2);
    }
}

// Setup the debug serial port for output.
ਖਾਲੀ
serial_debug_preinit(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ਵਾਪਸ;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    ਜੇ (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
ਸਥਿਰ ਖਾਲੀ
serial_debug(ਅੱਖਰ c)
{
    ਜੇ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ਵਾਪਸ;
    ਪੂਰਨ timeout = DEBUG_TIMEOUT;
    ਜਦੋਂ ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        ਜੇ (!timeout--)
            // Ran out of time.
            ਵਾਪਸ;
    serial_debug_write(SEROFF_ਅੰਕੜੇ, c);
}

ਖਾਲੀ
serial_debug_putc(ਅੱਖਰ c)
{
    ਜੇ (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
ਖਾਲੀ
serial_debug_flush(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ਵਾਪਸ;
    ਪੂਰਨ timeout = DEBUG_TIMEOUT;
    ਜਦੋਂ ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        ਜੇ (!timeout--)
            // Ran out of time.
            ਵਾਪਸ;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 Debug_Output_ਪੋਰਟ VARFSEG = 0x402;

ਖਾਲੀ
qemu_debug_preinit(ਖਾਲੀ)
{
    /* Xen doesn't support checking if debug output is active. */
    ਜੇ (runningOnXen())
        ਵਾਪਸ;

    /* Check if the QEMU debug output port is active */
    ਜੇ (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(Debug_Output_ਪੋਰਟ)) != QEMU_DEBUGCON_READBACK)
        Debug_Output_ਪੋਰਟ = 0;
}

// Write a character to the special debugging port.
ਖਾਲੀ
qemu_debug_putc(ਅੱਖਰ c)
{
    ਜੇ (!CONFIG_DEBUG_IO || !runningOnQEMU())
        ਵਾਪਸ;
    u16 ਪੋਰਟ = GET_GLOBAL(Debug_Output_ਪੋਰਟ);
    ਜੇ (ਪੋਰਟ)
        // Send character to debug port.
        outb(c, ਪੋਰਟ);
}
