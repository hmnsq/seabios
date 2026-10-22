#جيڪڏهن_وصف_نه_ٿيل __SERIALIO_H
#وصف_ڪريو __SERIALIO_H

#ضمن "types.h" // u16

#وصف_ڪريو پورٽ_LPT2              0x0278
#وصف_ڪريو پورٽ_SERIAL4           0x02e8
#وصف_ڪريو پورٽ_SERIAL2           0x02f8
#وصف_ڪريو پورٽ_LPT1              0x0378
#وصف_ڪريو پورٽ_SERIAL3           0x03e8
#وصف_ڪريو پورٽ_SERIAL1           0x03f8

// Serial port offsets
#وصف_ڪريو SEROFF_معلومات    0
#وصف_ڪريو SEROFF_DLL     0
#وصف_ڪريو SEROFF_IER     1
#وصف_ڪريو SEROFF_DLH     1
#وصف_ڪريو SEROFF_IIR     2
#وصف_ڪريو SEROFF_LCR     3
#وصف_ڪريو SEROFF_LSR     5
#وصف_ڪريو SEROFF_MSR     6

فراغ serial_debug_preinit(فراغ);
فراغ serial_debug_putc(حرف c);
فراغ serial_debug_flush(فراغ);
خارجي u16 Debug_Output_پورٽ;
فراغ qemu_debug_preinit(فراغ);
فراغ qemu_debug_putc(حرف c);

#جيڪڏهن_پورو // serialio.h
