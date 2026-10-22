#که_نه_تعریف_شوی __SERIALIO_H
#تعریف_کړه __SERIALIO_H

#ضمن "types.h" // u16

#تعریف_کړه پورټ_LPT2              0x0278
#تعریف_کړه پورټ_SERIAL4           0x02e8
#تعریف_کړه پورټ_SERIAL2           0x02f8
#تعریف_کړه پورټ_LPT1              0x0378
#تعریف_کړه پورټ_SERIAL3           0x03e8
#تعریف_کړه پورټ_SERIAL1           0x03f8

// Serial port offsets
#تعریف_کړه SEROFF_معلومات    0
#تعریف_کړه SEROFF_DLL     0
#تعریف_کړه SEROFF_IER     1
#تعریف_کړه SEROFF_DLH     1
#تعریف_کړه SEROFF_IIR     2
#تعریف_کړه SEROFF_LCR     3
#تعریف_کړه SEROFF_LSR     5
#تعریف_کړه SEROFF_MSR     6

فراغ serial_debug_preinit(فراغ);
فراغ serial_debug_putc(حرف c);
فراغ serial_debug_flush(فراغ);
خارجي u16 Debug_Output_پورټ;
فراغ qemu_debug_preinit(فراغ);
فراغ qemu_debug_putc(حرف c);

#د_که_پای // serialio.h
