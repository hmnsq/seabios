// Low-level serial (and serial-like) device access.
//
// Copyright (C) 2008-1013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_DEBUG_SERIAL
#అన్తర్భావ "fw/paravirt.h" // RunningOnQEMU
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "serialio.h" // serial_debug_preinit
#అన్తర్భావ "x86.h" // outb


/****************************************************************
 * Serial port debug output
 ****************************************************************/

#నిర్వచించు DEBUG_TIMEOUT 100000

// Write to a serial port register
స్థిర శూన్య
serial_debug_write(u8 స్థానభ్రంశం_2, u8 val)
{
    ఒకవేళ (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_పోర్టు + స్థానభ్రంశం_2);
    } లేకపోతే ఒకవేళ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((శూన్య*)CONFIG_DEBUG_SERIAL_MEM_చిరునామా + 4*స్థానభ్రంశం_2, val);
    }
}

// Read from a serial port register
స్థిర u8
serial_debug_read(u8 స్థానభ్రంశం_2)
{
    ఒకవేళ (CONFIG_DEBUG_SERIAL)
        తిరిగి inb(CONFIG_DEBUG_SERIAL_పోర్టు + స్థానభ్రంశం_2);
    ఒకవేళ (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        తిరిగి readb((శూన్య*)CONFIG_DEBUG_SERIAL_MEM_చిరునామా + 4*స్థానభ్రంశం_2);
    }
}

// Setup the debug serial port for output.
శూన్య
serial_debug_preinit(శూన్య)
{
    ఒకవేళ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        తిరిగి;
    // setup for serial logging: 8N1
    u8 oldparam, newparam = 0x03;
    oldparam = serial_debug_read(SEROFF_LCR);
    serial_debug_write(SEROFF_LCR, newparam);
    // Disable irqs
    u8 oldier, newier = 0;
    oldier = serial_debug_read(SEROFF_IER);
    serial_debug_write(SEROFF_IER, newier);

    ఒకవేళ (oldparam != newparam || oldier != newier)
        dprintf(1, "Changing serial settings was %x/%x now %x/%x\n"
                , oldparam, oldier, newparam, newier);
}

// Write a character to the serial port.
స్థిర శూన్య
serial_debug(అక్షరం c)
{
    ఒకవేళ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        తిరిగి;
    పూర్ణాంకం timeout = DEBUG_TIMEOUT;
    అయితే ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        ఒకవేళ (!timeout--)
            // Ran out of time.
            తిరిగి;
    serial_debug_write(SEROFF_దత్తాంశం, c);
}

శూన్య
serial_debug_putc(అక్షరం c)
{
    ఒకవేళ (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
శూన్య
serial_debug_flush(శూన్య)
{
    ఒకవేళ (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        తిరిగి;
    పూర్ణాంకం timeout = DEBUG_TIMEOUT;
    అయితే ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        ఒకవేళ (!timeout--)
            // Ran out of time.
            తిరిగి;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 Debug_Output_పోర్టు VARFSEG = 0x402;

శూన్య
qemu_debug_preinit(శూన్య)
{
    /* Xen doesn't support checking if debug output is active. */
    ఒకవేళ (runningOnXen())
        తిరిగి;

    /* Check if the QEMU debug output port is active */
    ఒకవేళ (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(Debug_Output_పోర్టు)) != QEMU_DEBUGCON_READBACK)
        Debug_Output_పోర్టు = 0;
}

// Write a character to the special debugging port.
శూన్య
qemu_debug_putc(అక్షరం c)
{
    ఒకవేళ (!CONFIG_DEBUG_IO || !runningOnQEMU())
        తిరిగి;
    u16 పోర్టు = GET_GLOBAL(Debug_Output_పోర్టు);
    ఒకవేళ (పోర్టు)
        // Send character to debug port.
        outb(c, పోర్టు);
}
