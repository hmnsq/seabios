#ifndef __SERIALIO_H
#define __SERIALIO_H

#अन्तर्भाव "types.h" // u16

#define PORT_LPT2              0x0278
#define PORT_SERIAL4           0x02e8
#define PORT_SERIAL2           0x02f8
#define PORT_LPT1              0x0378
#define PORT_SERIAL3           0x03e8
#define PORT_SERIAL1           0x03f8

// Serial port offsets
#define SEROFF_DATA    0
#define SEROFF_DLL     0
#define SEROFF_IER     1
#define SEROFF_DLH     1
#define SEROFF_IIR     2
#define SEROFF_LCR     3
#define SEROFF_LSR     5
#define SEROFF_MSR     6

शून्य serial_debug_preinit(शून्य);
शून्य serial_debug_putc(वर्ण c);
शून्य serial_debug_flush(शून्य);
बाह्य u16 DebugOutputPort;
शून्य qemu_debug_preinit(शून्य);
शून्य qemu_debug_putc(वर्ण c);

#endif // serialio.h
