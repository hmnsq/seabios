#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __SERIALIO_H
#ವ್ಯಾಖ್ಯಾನಿಸು __SERIALIO_H

#ಅನ್ತರ್ಭಾವ "types.h" // u16

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LPT2              0x0278
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SERIAL4           0x02e8
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SERIAL2           0x02f8
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LPT1              0x0378
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SERIAL3           0x03e8
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SERIAL1           0x03f8

// Serial port offsets
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_ದತ್ತಾಂಶ    0
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_DLL     0
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_IER     1
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_DLH     1
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_IIR     2
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_LCR     3
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_LSR     5
#ವ್ಯಾಖ್ಯಾನಿಸು SEROFF_MSR     6

ಶೂನ್ಯ serial_debug_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ serial_debug_putc(ಅಕ್ಷರ c);
ಶೂನ್ಯ serial_debug_flush(ಶೂನ್ಯ);
ಬಾಹ್ಯ u16 Debug_Output_ಪೋರ್ಟ್;
ಶೂನ್ಯ qemu_debug_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_debug_putc(ಅಕ್ಷರ c);

#ಆದರೆ_ಅಂತ್ಯ // serialio.h
