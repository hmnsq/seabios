#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __SERIALIO_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __SERIALIO_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u16

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LPT2              0x0278
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SERIAL4           0x02e8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SERIAL2           0x02f8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LPT1              0x0378
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SERIAL3           0x03e8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SERIAL1           0x03f8

// Serial port offsets
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_ਅੰਕੜੇ    0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_DLL     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_IER     1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_DLH     1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_IIR     2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_LCR     3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_LSR     5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEROFF_MSR     6

ਖਾਲੀ serial_debug_preinit(ਖਾਲੀ);
ਖਾਲੀ serial_debug_putc(ਅੱਖਰ c);
ਖਾਲੀ serial_debug_flush(ਖਾਲੀ);
ਬਾਹਰੀ u16 Debug_Output_ਪੋਰਟ;
ਖਾਲੀ qemu_debug_preinit(ਖਾਲੀ);
ਖਾਲੀ qemu_debug_putc(ਅੱਖਰ c);

#ਜੇ_ਅੰਤ // serialio.h
