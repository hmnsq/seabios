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

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TIMEOUT 100000

// Write to a serial port register
ಸ್ಥಿರ ಶೂನ್ಯ
serial_debug_write(u8 ಸ್ಥಾನಾಂತರ_2, u8 val)
{
    ಆದರೆ (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_ಪೋರ್ಟ್ + ಸ್ಥಾನಾಂತರ_2);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((ಶೂನ್ಯ*)CONFIG_DEBUG_SERIAL_MEM_ವಿಳಾಸ + 4*ಸ್ಥಾನಾಂತರ_2, val);
    }
}

// Read from a serial port register
ಸ್ಥಿರ u8
serial_debug_read(u8 ಸ್ಥಾನಾಂತರ_2)
{
    ಆದರೆ (CONFIG_DEBUG_SERIAL)
        ಹಿಂತಿರುಗಿ inb(CONFIG_DEBUG_SERIAL_ಪೋರ್ಟ್ + ಸ್ಥಾನಾಂತರ_2);
    ಆದರೆ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        ಹಿಂತಿರುಗಿ readb((ಶೂನ್ಯ*)CONFIG_DEBUG_SERIAL_MEM_ವಿಳಾಸ + 4*ಸ್ಥಾನಾಂತರ_2);
    }
}

// Setup the debug serial port for output.
ಶೂನ್ಯ
serial_debug_preinit(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ಹಿಂತಿರುಗಿ;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    ಆದರೆ (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
ಸ್ಥಿರ ಶೂನ್ಯ
serial_debug(ಅಕ್ಷರ c)
{
    ಆದರೆ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ timeout = DEBUG_TIMEOUT;
    ಆಗ ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        ಆದರೆ (!timeout--)
            // Ran out of time.
            ಹಿಂತಿರುಗಿ;
    serial_debug_write(SEROFF_ದತ್ತಾಂಶ, c);
}

ಶೂನ್ಯ
serial_debug_putc(ಅಕ್ಷರ c)
{
    ಆದರೆ (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
ಶೂನ್ಯ
serial_debug_flush(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ timeout = DEBUG_TIMEOUT;
    ಆಗ ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        ಆದರೆ (!timeout--)
            // Ran out of time.
            ಹಿಂತಿರುಗಿ;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 Debug_Output_ಪೋರ್ಟ್ VARFSEG = 0x402;

ಶೂನ್ಯ
qemu_debug_preinit(ಶೂನ್ಯ)
{
    /* Xen doesn't support checking if debug output is active. */
    ಆದರೆ (runningOnXen())
        ಹಿಂತಿರುಗಿ;

    /* Check if the QEMU debug output port is active */
    ಆದರೆ (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(Debug_Output_ಪೋರ್ಟ್)) != QEMU_DEBUGCON_READBACK)
        Debug_Output_ಪೋರ್ಟ್ = 0;
}

// Write a character to the special debugging port.
ಶೂನ್ಯ
qemu_debug_putc(ಅಕ್ಷರ c)
{
    ಆದರೆ (!CONFIG_DEBUG_IO || !runningOnQEMU())
        ಹಿಂತಿರುಗಿ;
    u16 ಪೋರ್ಟ್ = GET_GLOBAL(Debug_Output_ಪೋರ್ಟ್);
    ಆದರೆ (ಪೋರ್ಟ್)
        // Send character to debug port.
        outb(c, ಪೋರ್ಟ್);
}
