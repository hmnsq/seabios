#यदि_अपरिभाषितम् __SERIALIO_H
#परिभाषय __SERIALIO_H

#अन्तर्भाव "types.h" // u16

#परिभाषय पत्तनम्_LPT2              0x0278
#परिभाषय पत्तनम्_SERIAL4           0x02e8
#परिभाषय पत्तनम्_SERIAL2           0x02f8
#परिभाषय पत्तनम्_LPT1              0x0378
#परिभाषय पत्तनम्_SERIAL3           0x03e8
#परिभाषय पत्तनम्_SERIAL1           0x03f8

// Serial port offsets
#परिभाषय SEROFF_दत्त    0
#परिभाषय SEROFF_DLL     0
#परिभाषय SEROFF_IER     1
#परिभाषय SEROFF_DLH     1
#परिभाषय SEROFF_IIR     2
#परिभाषय SEROFF_LCR     3
#परिभाषय SEROFF_LSR     5
#परिभाषय SEROFF_MSR     6

शून्य serial_debug_preinit(शून्य);
शून्य serial_debug_putc(वर्ण c);
शून्य serial_debug_flush(शून्य);
बाह्य u16 Debug_Output_पत्तनम्;
शून्य qemu_debug_preinit(शून्य);
शून्य qemu_debug_putc(वर्ण c);

#यदि_अन्तः // serialio.h
