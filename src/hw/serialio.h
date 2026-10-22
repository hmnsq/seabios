#اگر_غیر_تعریف_شدہ __SERIALIO_H
#تعریف_کرو __SERIALIO_H

#شامل "types.h" // u16

#تعریف_کرو پورٹ_LPT2              0x0278
#تعریف_کرو پورٹ_SERIAL4           0x02e8
#تعریف_کرو پورٹ_SERIAL2           0x02f8
#تعریف_کرو پورٹ_LPT1              0x0378
#تعریف_کرو پورٹ_SERIAL3           0x03e8
#تعریف_کرو پورٹ_SERIAL1           0x03f8

// Serial port offsets
#تعریف_کرو SEROFF_ڈیٹا    0
#تعریف_کرو SEROFF_DLL     0
#تعریف_کرو SEROFF_IER     1
#تعریف_کرو SEROFF_DLH     1
#تعریف_کرو SEROFF_IIR     2
#تعریف_کرو SEROFF_LCR     3
#تعریف_کرو SEROFF_LSR     5
#تعریف_کرو SEROFF_MSR     6

خالی serial_debug_preinit(خالی);
خالی serial_debug_putc(حرف c);
خالی serial_debug_flush(خالی);
خارجی u16 Debug_Output_پورٹ;
خالی qemu_debug_preinit(خالی);
خالی qemu_debug_putc(حرف c);

#اگر_ختم // serialio.h
