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

#परिभाषय DEBUG_TIMEOUT 100000

// Write to a serial port register
स्थिर शून्य
serial_debug_write(u8 विस्थापनम्_2, u8 val)
{
    यदि (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_पत्तनम् + विस्थापनम्_2);
    } अन्यथा यदि (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((शून्य*)CONFIG_DEBUG_SERIAL_MEM_स्थानाङ्कः + 4*विस्थापनम्_2, val);
    }
}

// Read from a serial port register
स्थिर u8
serial_debug_read(u8 विस्थापनम्_2)
{
    यदि (CONFIG_DEBUG_SERIAL)
        प्रत्यागम inb(CONFIG_DEBUG_SERIAL_पत्तनम् + विस्थापनम्_2);
    यदि (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        प्रत्यागम readb((शून्य*)CONFIG_DEBUG_SERIAL_MEM_स्थानाङ्कः + 4*विस्थापनम्_2);
    }
}

// Setup the debug serial port for output.
शून्य
serial_debug_preinit(शून्य)
{
    यदि (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        प्रत्यागम;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    यदि (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
स्थिर शून्य
serial_debug(वर्ण c)
{
    यदि (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        प्रत्यागम;
    पूर्णाङ्क timeout = DEBUG_TIMEOUT;
    यावत् ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        यदि (!timeout--)
            // Ran out of time.
            प्रत्यागम;
    serial_debug_write(SEROFF_दत्त, c);
}

शून्य
serial_debug_putc(वर्ण c)
{
    यदि (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
शून्य
serial_debug_flush(शून्य)
{
    यदि (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        प्रत्यागम;
    पूर्णाङ्क timeout = DEBUG_TIMEOUT;
    यावत् ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        यदि (!timeout--)
            // Ran out of time.
            प्रत्यागम;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 Debug_Output_पत्तनम् VARFSEG = 0x402;

शून्य
qemu_debug_preinit(शून्य)
{
    /* Xen doesn't support checking if debug output is active. */
    यदि (runningOnXen())
        प्रत्यागम;

    /* Check if the QEMU debug output port is active */
    यदि (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(Debug_Output_पत्तनम्)) != QEMU_DEBUGCON_READBACK)
        Debug_Output_पत्तनम् = 0;
}

// Write a character to the special debugging port.
शून्य
qemu_debug_putc(वर्ण c)
{
    यदि (!CONFIG_DEBUG_IO || !runningOnQEMU())
        प्रत्यागम;
    u16 पत्तनम् = GET_GLOBAL(Debug_Output_पत्तनम्);
    यदि (पत्तनम्)
        // Send character to debug port.
        outb(c, पत्तनम्);
}
