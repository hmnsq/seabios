// Basic ps2 port (keyboard/mouse) command handling.
#اگر_تعریف_نشده __PS2PORT_H
#تعریف_کن __PS2PORT_H

#ضمن "types.h" // u8

#تعریف_کن پورت_PS2_داده          0x0060
#تعریف_کن پورت_PS2_وضعیت        0x0064

// Standard commands.
#تعریف_کن I8042_CMD_CTL_RCTR      0x0120
#تعریف_کن I8042_CMD_CTL_WCTR      0x1060
#تعریف_کن I8042_CMD_CTL_TEST      0x01aa

#تعریف_کن I8042_CMD_KBD_TEST      0x01ab
#تعریف_کن I8042_CMD_KBD_DISABLE   0x00ad
#تعریف_کن I8042_CMD_KBD_ENABLE    0x00ae

#تعریف_کن I8042_CMD_AUX_DISABLE   0x00a7
#تعریف_کن I8042_CMD_AUX_ENABLE    0x00a8
#تعریف_کن I8042_CMD_AUX_SEND      0x10d4

// Keyboard commands
#تعریف_کن ATKBD_CMD_SETLEDS       0x10ed
#تعریف_کن ATKBD_CMD_SSCANSET      0x10f0
#تعریف_کن ATKBD_CMD_GETID         0x02f2
#تعریف_کن ATKBD_CMD_ENABLE        0x00f4
#تعریف_کن ATKBD_CMD_RESET_DIS     0x00f5
#تعریف_کن ATKBD_CMD_RESET_BAT     0x01ff

// Mouse commands
#تعریف_کن PSMOUSE_CMD_SETSCALE11  0x00e6
#تعریف_کن PSMOUSE_CMD_SETSCALE21  0x00e7
#تعریف_کن PSMOUSE_CMD_SETRES      0x10e8
#تعریف_کن PSMOUSE_CMD_GETINFO     0x03e9
#تعریف_کن PSMOUSE_CMD_GETID       0x02f2
#تعریف_کن PSMOUSE_CMD_SETRATE     0x10f3
#تعریف_کن PSMOUSE_CMD_ENABLE      0x00f4
#تعریف_کن PSMOUSE_CMD_DISABLE     0x00f5
#تعریف_کن PSMOUSE_CMD_RESET_BAT   0x02ff

// Status register bits.
#تعریف_کن I8042_STR_PARITY        0x80
#تعریف_کن I8042_STR_TIMEOUT       0x40
#تعریف_کن I8042_STR_AUXDATA       0x20
#تعریف_کن I8042_STR_KEYLOCK       0x10
#تعریف_کن I8042_STR_CMDDAT        0x08
#تعریف_کن I8042_STR_MUXERR        0x04
#تعریف_کن I8042_STR_IBF           0x02
#تعریف_کن I8042_STR_OBF           0x01

// Control register bits.
#تعریف_کن I8042_CTR_KBDINT        0x01
#تعریف_کن I8042_CTR_AUXINT        0x02
#تعریف_کن I8042_CTR_IGNKEYLOCK    0x08
#تعریف_کن I8042_CTR_KBDDIS        0x10
#تعریف_کن I8042_CTR_AUXDIS        0x20
#تعریف_کن I8042_CTR_XLATE         0x40

// ps2port.c
فراغ i8042_reboot(فراغ);
صحيح ps2_kbd_command(صحيح command, u8 *param);
صحيح ps2_mouse_command(صحيح command, u8 *param);
فراغ ps2_check_event(فراغ);
فراغ ps2port_راهاندازی(فراغ);

#پایان_اگر // ps2port.h
