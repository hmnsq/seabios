// Basic ps2 port (keyboard/mouse) command handling.
#جيڪڏهن_وصف_نه_ٿيل __PS2PORT_H
#وصف_ڪريو __PS2PORT_H

#ضمن "types.h" // u8

#وصف_ڪريو پورٽ_PS2_معلومات          0x0060
#وصف_ڪريو پورٽ_PS2_حالت        0x0064

// Standard commands.
#وصف_ڪريو I8042_CMD_CTL_RCTR      0x0120
#وصف_ڪريو I8042_CMD_CTL_WCTR      0x1060
#وصف_ڪريو I8042_CMD_CTL_TEST      0x01aa

#وصف_ڪريو I8042_CMD_KBD_TEST      0x01ab
#وصف_ڪريو I8042_CMD_KBD_DISABLE   0x00ad
#وصف_ڪريو I8042_CMD_KBD_ENABLE    0x00ae

#وصف_ڪريو I8042_CMD_AUX_DISABLE   0x00a7
#وصف_ڪريو I8042_CMD_AUX_ENABLE    0x00a8
#وصف_ڪريو I8042_CMD_AUX_SEND      0x10d4

// Keyboard commands
#وصف_ڪريو ATKBD_CMD_SETLEDS       0x10ed
#وصف_ڪريو ATKBD_CMD_SSCANSET      0x10f0
#وصف_ڪريو ATKBD_CMD_GETID         0x02f2
#وصف_ڪريو ATKBD_CMD_ENABLE        0x00f4
#وصف_ڪريو ATKBD_CMD_RESET_DIS     0x00f5
#وصف_ڪريو ATKBD_CMD_RESET_BAT     0x01ff

// Mouse commands
#وصف_ڪريو PSMOUSE_CMD_SETSCALE11  0x00e6
#وصف_ڪريو PSMOUSE_CMD_SETSCALE21  0x00e7
#وصف_ڪريو PSMOUSE_CMD_SETRES      0x10e8
#وصف_ڪريو PSMOUSE_CMD_GETINFO     0x03e9
#وصف_ڪريو PSMOUSE_CMD_GETID       0x02f2
#وصف_ڪريو PSMOUSE_CMD_SETRATE     0x10f3
#وصف_ڪريو PSMOUSE_CMD_ENABLE      0x00f4
#وصف_ڪريو PSMOUSE_CMD_DISABLE     0x00f5
#وصف_ڪريو PSMOUSE_CMD_RESET_BAT   0x02ff

// Status register bits.
#وصف_ڪريو I8042_STR_PARITY        0x80
#وصف_ڪريو I8042_STR_TIMEOUT       0x40
#وصف_ڪريو I8042_STR_AUXDATA       0x20
#وصف_ڪريو I8042_STR_KEYLOCK       0x10
#وصف_ڪريو I8042_STR_CMDDAT        0x08
#وصف_ڪريو I8042_STR_MUXERR        0x04
#وصف_ڪريو I8042_STR_IBF           0x02
#وصف_ڪريو I8042_STR_OBF           0x01

// Control register bits.
#وصف_ڪريو I8042_CTR_KBDINT        0x01
#وصف_ڪريو I8042_CTR_AUXINT        0x02
#وصف_ڪريو I8042_CTR_IGNKEYLOCK    0x08
#وصف_ڪريو I8042_CTR_KBDDIS        0x10
#وصف_ڪريو I8042_CTR_AUXDIS        0x20
#وصف_ڪريو I8042_CTR_XLATE         0x40

// ps2port.c
فراغ i8042_reboot(فراغ);
صحيح ps2_kbd_command(صحيح command, u8 *param);
صحيح ps2_mouse_command(صحيح command, u8 *param);
فراغ ps2_check_event(فراغ);
فراغ ps2port_تياري(فراغ);

#جيڪڏهن_پورو // ps2port.h
