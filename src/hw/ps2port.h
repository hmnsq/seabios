// Basic ps2 port (keyboard/mouse) command handling.
#నిర్వచితం_కాకపోతే __PS2PORT_H
#నిర్వచించు __PS2PORT_H

#అన్తర్భావ "types.h" // u8

#నిర్వచించు పోర్టు_PS2_దత్తాంశం          0x0060
#నిర్వచించు పోర్టు_PS2_స్థితి        0x0064

// Standard commands.
#నిర్వచించు I8042_CMD_CTL_RCTR      0x0120
#నిర్వచించు I8042_CMD_CTL_WCTR      0x1060
#నిర్వచించు I8042_CMD_CTL_TEST      0x01aa

#నిర్వచించు I8042_CMD_KBD_TEST      0x01ab
#నిర్వచించు I8042_CMD_KBD_DISABLE   0x00ad
#నిర్వచించు I8042_CMD_KBD_ENABLE    0x00ae

#నిర్వచించు I8042_CMD_AUX_DISABLE   0x00a7
#నిర్వచించు I8042_CMD_AUX_ENABLE    0x00a8
#నిర్వచించు I8042_CMD_AUX_SEND      0x10d4

// Keyboard commands
#నిర్వచించు ATKBD_CMD_SETLEDS       0x10ed
#నిర్వచించు ATKBD_CMD_SSCANSET      0x10f0
#నిర్వచించు ATKBD_CMD_GETID         0x02f2
#నిర్వచించు ATKBD_CMD_ENABLE        0x00f4
#నిర్వచించు ATKBD_CMD_RESET_DIS     0x00f5
#నిర్వచించు ATKBD_CMD_RESET_BAT     0x01ff

// Mouse commands
#నిర్వచించు PSMOUSE_CMD_SETSCALE11  0x00e6
#నిర్వచించు PSMOUSE_CMD_SETSCALE21  0x00e7
#నిర్వచించు PSMOUSE_CMD_SETRES      0x10e8
#నిర్వచించు PSMOUSE_CMD_GETINFO     0x03e9
#నిర్వచించు PSMOUSE_CMD_GETID       0x02f2
#నిర్వచించు PSMOUSE_CMD_SETRATE     0x10f3
#నిర్వచించు PSMOUSE_CMD_ENABLE      0x00f4
#నిర్వచించు PSMOUSE_CMD_DISABLE     0x00f5
#నిర్వచించు PSMOUSE_CMD_RESET_BAT   0x02ff

// Status register bits.
#నిర్వచించు I8042_STR_PARITY        0x80
#నిర్వచించు I8042_STR_TIMEOUT       0x40
#నిర్వచించు I8042_STR_AUXDATA       0x20
#నిర్వచించు I8042_STR_KEYLOCK       0x10
#నిర్వచించు I8042_STR_CMDDAT        0x08
#నిర్వచించు I8042_STR_MUXERR        0x04
#నిర్వచించు I8042_STR_IBF           0x02
#నిర్వచించు I8042_STR_OBF           0x01

// Control register bits.
#నిర్వచించు I8042_CTR_KBDINT        0x01
#నిర్వచించు I8042_CTR_AUXINT        0x02
#నిర్వచించు I8042_CTR_IGNKEYLOCK    0x08
#నిర్వచించు I8042_CTR_KBDDIS        0x10
#నిర్వచించు I8042_CTR_AUXDIS        0x20
#నిర్వచించు I8042_CTR_XLATE         0x40

// ps2port.c
శూన్య i8042_reboot(శూన్య);
పూర్ణాంకం ps2_kbd_command(పూర్ణాంకం command, u8 *param);
పూర్ణాంకం ps2_mouse_command(పూర్ణాంకం command, u8 *param);
శూన్య ps2_check_event(శూన్య);
శూన్య ps2port_అమరిక(శూన్య);

#అయితే_ముగింపు // ps2port.h
