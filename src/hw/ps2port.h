// Basic ps2 port (keyboard/mouse) command handling.
#यदि_अपरिभाषितम् __PS2PORT_H
#परिभाषय __PS2PORT_H

#अन्तर्भाव "types.h" // u8

#परिभाषय पत्तनम्_PS2_दत्त          0x0060
#परिभाषय पत्तनम्_PS2_स्थितिः        0x0064

// Standard commands.
#परिभाषय I8042_CMD_CTL_RCTR      0x0120
#परिभाषय I8042_CMD_CTL_WCTR      0x1060
#परिभाषय I8042_CMD_CTL_TEST      0x01aa

#परिभाषय I8042_CMD_KBD_TEST      0x01ab
#परिभाषय I8042_CMD_KBD_DISABLE   0x00ad
#परिभाषय I8042_CMD_KBD_ENABLE    0x00ae

#परिभाषय I8042_CMD_AUX_DISABLE   0x00a7
#परिभाषय I8042_CMD_AUX_ENABLE    0x00a8
#परिभाषय I8042_CMD_AUX_SEND      0x10d4

// Keyboard commands
#परिभाषय ATKBD_CMD_SETLEDS       0x10ed
#परिभाषय ATKBD_CMD_SSCANSET      0x10f0
#परिभाषय ATKBD_CMD_GETID         0x02f2
#परिभाषय ATKBD_CMD_ENABLE        0x00f4
#परिभाषय ATKBD_CMD_RESET_DIS     0x00f5
#परिभाषय ATKBD_CMD_RESET_BAT     0x01ff

// Mouse commands
#परिभाषय PSMOUSE_CMD_SETSCALE11  0x00e6
#परिभाषय PSMOUSE_CMD_SETSCALE21  0x00e7
#परिभाषय PSMOUSE_CMD_SETRES      0x10e8
#परिभाषय PSMOUSE_CMD_GETINFO     0x03e9
#परिभाषय PSMOUSE_CMD_GETID       0x02f2
#परिभाषय PSMOUSE_CMD_SETRATE     0x10f3
#परिभाषय PSMOUSE_CMD_ENABLE      0x00f4
#परिभाषय PSMOUSE_CMD_DISABLE     0x00f5
#परिभाषय PSMOUSE_CMD_RESET_BAT   0x02ff

// Status register bits.
#परिभाषय I8042_STR_PARITY        0x80
#परिभाषय I8042_STR_TIMEOUT       0x40
#परिभाषय I8042_STR_AUXDATA       0x20
#परिभाषय I8042_STR_KEYLOCK       0x10
#परिभाषय I8042_STR_CMDDAT        0x08
#परिभाषय I8042_STR_MUXERR        0x04
#परिभाषय I8042_STR_IBF           0x02
#परिभाषय I8042_STR_OBF           0x01

// Control register bits.
#परिभाषय I8042_CTR_KBDINT        0x01
#परिभाषय I8042_CTR_AUXINT        0x02
#परिभाषय I8042_CTR_IGNKEYLOCK    0x08
#परिभाषय I8042_CTR_KBDDIS        0x10
#परिभाषय I8042_CTR_AUXDIS        0x20
#परिभाषय I8042_CTR_XLATE         0x40

// ps2port.c
शून्य i8042_reboot(शून्य);
पूर्णाङ्क ps2_kbd_command(पूर्णाङ्क command, u8 *param);
पूर्णाङ्क ps2_mouse_command(पूर्णाङ्क command, u8 *param);
शून्य ps2_check_event(शून्य);
शून्य ps2port_सज्जता(शून्य);

#यदि_अन्तः // ps2port.h
