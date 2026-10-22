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

#تعریف_کرو DEBUG_TIMEOUT 100000

// Write to a serial port register
ساکن خالی
serial_debug_write(u8 انحراف_2, u8 val)
{
    اگر (CONFIG_DEBUG_SERIAL) {
        outb(val, CONFIG_DEBUG_SERIAL_پورٹ + انحراف_2);
    } ورنہ اگر (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        writeb((خالی*)CONFIG_DEBUG_SERIAL_MEM_پتہ + 4*انحراف_2, val);
    }
}

// Read from a serial port register
ساکن u8
serial_debug_read(u8 انحراف_2)
{
    اگر (CONFIG_DEBUG_SERIAL)
        واپس inb(CONFIG_DEBUG_SERIAL_پورٹ + انحراف_2);
    اگر (CONFIG_DEBUG_SERIAL_MMIO) {
        ASSERT32FLAT();
        واپس readb((خالی*)CONFIG_DEBUG_SERIAL_MEM_پتہ + 4*انحراف_2);
    }
}

// Setup the debug serial port for output.
خالی
serial_debug_preinit(خالی)
{
    اگر (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        واپس;
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
ساکن خالی
serial_debug(حرف c)
{
    اگر (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        واپس;
    صحیح timeout = DEBUG_TIMEOUT;
    جب ((serial_debug_read(SEROFF_LSR) & 0x20) != 0x20)
        اگر (!timeout--)
            // Ran out of time.
            واپس;
    serial_debug_write(SEROFF_ڈیٹا, c);
}

خالی
serial_debug_putc(حرف c)
{
    اگر (c == '\n')
        serial_debug('\r');
    serial_debug(c);
}

// Make sure all serial port writes have been completely sent.
خالی
serial_debug_flush(خالی)
{
    اگر (!CONFIG_DEBUG_SERIAL && (!CONFIG_DEBUG_SERIAL_MMIO || MODESEGMENT))
        واپس;
    صحیح timeout = DEBUG_TIMEOUT;
    جب ((serial_debug_read(SEROFF_LSR) & 0x60) != 0x60)
        اگر (!timeout--)
            // Ran out of time.
            واپس;
}


/****************************************************************
 * QEMU debug port
 ****************************************************************/

u16 Debug_Output_پورٹ VARFSEG = 0x402;

خالی
qemu_debug_preinit(خالی)
{
    /* Xen doesn't support checking if debug output is active. */
    اگر (runningOnXen())
        واپس;

    /* Check if the QEMU debug output port is active */
    اگر (CONFIG_DEBUG_IO &&
        inb(GET_GLOBAL(Debug_Output_پورٹ)) != QEMU_DEBUGCON_READBACK)
        Debug_Output_پورٹ = 0;
}

// Write a character to the special debugging port.
خالی
qemu_debug_putc(حرف c)
{
    اگر (!CONFIG_DEBUG_IO || !runningOnQEMU())
        واپس;
    u16 پورٹ = GET_GLOBAL(Debug_Output_پورٹ);
    اگر (پورٹ)
        // Send character to debug port.
        outb(c, پورٹ);
}
