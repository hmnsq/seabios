#جے_بنا_تعریف __SERIALIO_H
#تعریف_کرو __SERIALIO_H

#ضمن "types.h" // u16

#تعریف_کرو پورٹ_LPT2              0x0278
#تعریف_کرو پورٹ_SERIAL4           0x02e8
#تعریف_کرو پورٹ_SERIAL2           0x02f8
#تعریف_کرو پورٹ_LPT1              0x0378
#تعریف_کرو پورٹ_SERIAL3           0x03e8
#تعریف_کرو پورٹ_SERIAL1           0x03f8

// Serial port offsets
#تعریف_کرو SEROFF_انکڑے    0
#تعریف_کرو SEROFF_DLL     0
#تعریف_کرو SEROFF_IER     1
#تعریف_کرو SEROFF_DLH     1
#تعریف_کرو SEROFF_IIR     2
#تعریف_کرو SEROFF_LCR     3
#تعریف_کرو SEROFF_LSR     5
#تعریف_کرو SEROFF_MSR     6

فراغ serial_debug_preinit(فراغ);
فراغ serial_debug_putc(حرف c);
فراغ serial_debug_flush(فراغ);
خارجي u16 Debug_Output_پورٹ;
فراغ qemu_debug_preinit(فراغ);
فراغ qemu_debug_putc(حرف c);

#جے_انت // serialio.h
