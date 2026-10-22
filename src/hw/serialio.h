#اگر_تعریف_نشده __SERIALIO_H
#تعریف_کن __SERIALIO_H

#ضمن "types.h" // u16

#تعریف_کن پورت_LPT2              0x0278
#تعریف_کن پورت_SERIAL4           0x02e8
#تعریف_کن پورت_SERIAL2           0x02f8
#تعریف_کن پورت_LPT1              0x0378
#تعریف_کن پورت_SERIAL3           0x03e8
#تعریف_کن پورت_SERIAL1           0x03f8

// Serial port offsets
#تعریف_کن SEROFF_داده    0
#تعریف_کن SEROFF_DLL     0
#تعریف_کن SEROFF_IER     1
#تعریف_کن SEROFF_DLH     1
#تعریف_کن SEROFF_IIR     2
#تعریف_کن SEROFF_LCR     3
#تعریف_کن SEROFF_LSR     5
#تعریف_کن SEROFF_MSR     6

فراغ serial_debug_preinit(فراغ);
فراغ serial_debug_putc(حرف c);
فراغ serial_debug_flush(فراغ);
خارجي u16 Debug_Output_پورت;
فراغ qemu_debug_preinit(فراغ);
فراغ qemu_debug_putc(حرف c);

#پایان_اگر // serialio.h
