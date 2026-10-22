// Basic ps2 port (keyboard/mouse) command handling.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PS2PORT_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PS2PORT_H

#ಅನ್ತರ್ಭಾವ "types.h" // u8

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ          0x0060
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ        0x0064

// Standard commands.
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_CTL_RCTR      0x0120
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_CTL_WCTR      0x1060
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_CTL_TEST      0x01aa

#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_KBD_TEST      0x01ab
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_KBD_DISABLE   0x00ad
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_KBD_ENABLE    0x00ae

#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_AUX_DISABLE   0x00a7
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_AUX_ENABLE    0x00a8
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CMD_AUX_SEND      0x10d4

// Keyboard commands
#ವ್ಯಾಖ್ಯಾನಿಸು ATKBD_CMD_SETLEDS       0x10ed
#ವ್ಯಾಖ್ಯಾನಿಸು ATKBD_CMD_SSCANSET      0x10f0
#ವ್ಯಾಖ್ಯಾನಿಸು ATKBD_CMD_GETID         0x02f2
#ವ್ಯಾಖ್ಯಾನಿಸು ATKBD_CMD_ENABLE        0x00f4
#ವ್ಯಾಖ್ಯಾನಿಸು ATKBD_CMD_RESET_DIS     0x00f5
#ವ್ಯಾಖ್ಯಾನಿಸು ATKBD_CMD_RESET_BAT     0x01ff

// Mouse commands
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_SETSCALE11  0x00e6
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_SETSCALE21  0x00e7
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_SETRES      0x10e8
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_GETINFO     0x03e9
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_GETID       0x02f2
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_SETRATE     0x10f3
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_ENABLE      0x00f4
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_DISABLE     0x00f5
#ವ್ಯಾಖ್ಯಾನಿಸು PSMOUSE_CMD_RESET_BAT   0x02ff

// Status register bits.
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_PARITY        0x80
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_TIMEOUT       0x40
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_AUXDATA       0x20
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_KEYLOCK       0x10
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_CMDDAT        0x08
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_MUXERR        0x04
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_IBF           0x02
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_STR_OBF           0x01

// Control register bits.
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CTR_KBDINT        0x01
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CTR_AUXINT        0x02
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CTR_IGNKEYLOCK    0x08
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CTR_KBDDIS        0x10
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CTR_AUXDIS        0x20
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CTR_XLATE         0x40

// ps2port.c
ಶೂನ್ಯ i8042_reboot(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ ps2_kbd_command(ಪೂರ್ಣಾಂಕ command, u8 *param);
ಪೂರ್ಣಾಂಕ ps2_mouse_command(ಪೂರ್ಣಾಂಕ command, u8 *param);
ಶೂನ್ಯ ps2_check_event(ಶೂನ್ಯ);
ಶೂನ್ಯ ps2port_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ // ps2port.h
