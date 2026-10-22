// Basic ps2 port (keyboard/mouse) command handling.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PS2PORT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PS2PORT_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PS2_ਅੰਕੜੇ          0x0060
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PS2_ਸਥਿਤੀ        0x0064

// Standard commands.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_CTL_RCTR      0x0120
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_CTL_WCTR      0x1060
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_CTL_TEST      0x01aa

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_KBD_TEST      0x01ab
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_KBD_DISABLE   0x00ad
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_KBD_ENABLE    0x00ae

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_AUX_DISABLE   0x00a7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_AUX_ENABLE    0x00a8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CMD_AUX_SEND      0x10d4

// Keyboard commands
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATKBD_CMD_SETLEDS       0x10ed
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATKBD_CMD_SSCANSET      0x10f0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATKBD_CMD_GETID         0x02f2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATKBD_CMD_ENABLE        0x00f4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATKBD_CMD_RESET_DIS     0x00f5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATKBD_CMD_RESET_BAT     0x01ff

// Mouse commands
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_SETSCALE11  0x00e6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_SETSCALE21  0x00e7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_SETRES      0x10e8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_GETINFO     0x03e9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_GETID       0x02f2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_SETRATE     0x10f3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_ENABLE      0x00f4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_DISABLE     0x00f5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PSMOUSE_CMD_RESET_BAT   0x02ff

// Status register bits.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_PARITY        0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_TIMEOUT       0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_AUXDATA       0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_KEYLOCK       0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_CMDDAT        0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_MUXERR        0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_IBF           0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_STR_OBF           0x01

// Control register bits.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CTR_KBDINT        0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CTR_AUXINT        0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CTR_IGNKEYLOCK    0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CTR_KBDDIS        0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CTR_AUXDIS        0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CTR_XLATE         0x40

// ps2port.c
ਖਾਲੀ i8042_reboot(ਖਾਲੀ);
ਪੂਰਨ ps2_kbd_command(ਪੂਰਨ command, u8 *param);
ਪੂਰਨ ps2_mouse_command(ਪੂਰਨ command, u8 *param);
ਖਾਲੀ ps2_check_event(ਖਾਲੀ);
ਖਾਲੀ ps2port_ਤਿਆਰੀ(ਖਾਲੀ);

#ਜੇ_ਅੰਤ // ps2port.h
