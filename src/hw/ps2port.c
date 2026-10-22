// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOW
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pic.h" // pic_eoi1
#ಅನ್ತರ್ಭಾವ "ps2port.h" // ps2_kbd_command
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "util.h" // udelay
#ಅನ್ತರ್ಭಾವ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
i8042_wait_read(ಶೂನ್ಯ)
{
    dprintf(7, "i8042_wait_read\n");
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ಯದಿ (status & I8042_STR_OBF)
            ಪ್ರತ್ಯಾಗಮ 0;
        udelay(50);
    }
    warn_timeout();
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
i8042_wait_write(ಶೂನ್ಯ)
{
    dprintf(7, "i8042_wait_write\n");
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ಯದಿ (! (status & I8042_STR_IBF))
            ಪ್ರತ್ಯಾಗಮ 0;
        udelay(50);
    }
    warn_timeout();
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
i8042_flush(ಶೂನ್ಯ)
{
    dprintf(7, "i8042_flush\n");
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ಯದಿ (! (status & I8042_STR_OBF))
            ಪ್ರತ್ಯಾಗಮ 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
__i8042_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    ಪೂರ್ಣಾಙ್ಕ receive = (command >> 8) & 0xf;
    ಪೂರ್ಣಾಙ್ಕ send = (command >> 12) & 0xf;

    // Send the command.
    ಪೂರ್ಣಾಙ್ಕ ret = i8042_wait_write();
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    ಪ್ರತಿ (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
i8042_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ಪೂರ್ಣಾಙ್ಕ ret = __i8042_command(command, param);
    ಯದಿ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ಪೂರ್ಣಾಙ್ಕ ret = i8042_wait_write();
    ಯದಿ (! ret)
        outb(c, PORT_PS2_DATA);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
i8042_aux_write(u8 c)
{
    ಪ್ರತ್ಯಾಗಮ i8042_command(I8042_CMD_AUX_SEND, &c);
}

ಶೂನ್ಯ
i8042_reboot(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PS2PORT)
       ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<10; i++) {
        i8042_wait_write();
        udelay(50);
        outb(0xfe, PORT_PS2_STATUS); /* pulse reset low */
        udelay(50);
    }
}


/****************************************************************
 * Device commands.
 ****************************************************************/

#define PS2_RET_ACK             0xfa
#define PS2_RET_NAK             0xfe

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ps2_recvbyte(ಪೂರ್ಣಾಙ್ಕ aux, ಪೂರ್ಣಾಙ್ಕ needack, ಪೂರ್ಣಾಙ್ಕ timeout)
{
    u32 end = timer_calc(timeout);
    ಪ್ರತಿ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ಯದಿ (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            ಯದಿ (!!(status & I8042_STR_AUXDATA) == aux) {
                ಯದಿ (!needack)
                    ಪ್ರತ್ಯಾಗಮ data;
                ಯದಿ (data == PS2_RET_ACK)
                    ಪ್ರತ್ಯಾಗಮ data;
                ಯದಿ (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ಪ್ರತ್ಯಾಗಮ data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ps2_sendbyte(ಪೂರ್ಣಾಙ್ಕ aux, u8 command, ಪೂರ್ಣಾಙ್ಕ timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ಪೂರ್ಣಾಙ್ಕ ret;
    ಯದಿ (aux)
        ret = i8042_aux_write(command);
    ಅನ್ಯಥಾ
        ret = i8042_kbd_write(command);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ಯದಿ (ret < 0)
        ಪ್ರತ್ಯಾಗಮ ret;
    ಯದಿ (ret != PS2_RET_ACK)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಪ್ರತ್ಯಾಗಮ 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
__ps2_command(ಪೂರ್ಣಾಙ್ಕ aux, ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    ಪೂರ್ಣಾಙ್ಕ ret2;
    ಪೂರ್ಣಾಙ್ಕ receive = (command >> 8) & 0xf;
    ಪೂರ್ಣಾಙ್ಕ send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_ಕೆಳಗಿನ(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ಪೂರ್ಣಾಙ್ಕ ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_ಕೆಳಗಿನ(Ps2ctr, newctr);
    ಯದಿ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ಅನ್ಯಥಾ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;

    ಯದಿ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ಯದಿ (ret)
            ಗಚ್ಛ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ಯದಿ (ret < 0)
            ಗಚ್ಛ fail;
        param[0] = ret;
        ಯದಿ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ಯದಿ (ret < 0)
                ಗಚ್ಛ fail;
            param[1] = ret;
        }
    } ಅನ್ಯಥಾ ಯದಿ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ಯದಿ (ret)
            ಗಚ್ಛ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ಯದಿ (ret < 0)
            ಗಚ್ಛ fail;
        param[0] = ret;
        ಯದಿ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ಯದಿ (ret < 0)
                ಗಚ್ಛ fail;
            param[1] = ret;
        } ಅನ್ಯಥಾ {
            param[1] = 0;
        }
    } ಅನ್ಯಥಾ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ಯದಿ (ret)
            ಗಚ್ಛ fail;

        // Send parameters (if any).
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ಯದಿ (ret)
                ಗಚ್ಛ fail;
        }

        // Receive parameters (if any).
        ಪ್ರತಿ (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ಯದಿ (ret < 0)
                ಗಚ್ಛ fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_ಕೆಳಗಿನ(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ಯದಿ (ret2)
        ಪ್ರತ್ಯಾಗಮ ret2;

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ps2_command(ಪೂರ್ಣಾಙ್ಕ aux, ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ಪೂರ್ಣಾಙ್ಕ ret = __ps2_command(aux, command, param);
    ಯದಿ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ
ps2_kbd_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    ಯದಿ (! CONFIG_PS2PORT)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ ps2_command(0, command, param);
}

ಪೂರ್ಣಾಙ್ಕ
ps2_mouse_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    ಯದಿ (! CONFIG_PS2PORT)
        ಪ್ರತ್ಯಾಗಮ -1;

    // Update ps2ctr for mouse enable/disable.
    ಯದಿ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_ಕೆಳಗಿನ(Ps2ctr);
        ಯದಿ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ಅನ್ಯಥಾ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_ಕೆಳಗಿನ(Ps2ctr, ps2ctr);
    }

    ಪ್ರತ್ಯಾಗಮ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
ಶೂನ್ಯ VISIBLE16
handle_74(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PS2PORT)
        ಪ್ರತ್ಯಾಗಮ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    ಯದಿ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        ಗಚ್ಛ done;
    }
    v = inb(PORT_PS2_DATA);

    ಯದಿ (!(GET_ಕೆಳಗಿನ(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        ಗಚ್ಛ done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
ಶೂನ್ಯ VISIBLE16
handle_09(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PS2PORT)
        ಪ್ರತ್ಯಾಗಮ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    ಯದಿ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        ಗಚ್ಛ done;
    }
    v = inb(PORT_PS2_DATA);

    ಯದಿ (!(GET_ಕೆಳಗಿನ(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        ಗಚ್ಛ done;

    process_ಕೀಲಿ(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ರಿಕ್ತ);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
ಶೂನ್ಯ
ps2_check_event(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ಪ್ರತ್ಯಾಗಮ;
    u8 ps2ctr = GET_ಕೆಳಗಿನ(Ps2ctr);
    ಯದಿ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ಪ್ರತ್ಯಾಗಮ;
    ಪ್ರತಿ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ಯದಿ (!(status & I8042_STR_OBF))
            ಭಙ್ಗ;
        u8 data = inb(PORT_PS2_DATA);
        ಯದಿ (status & I8042_STR_AUXDATA) {
            ಯದಿ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } ಅನ್ಯಥಾ {
            ಯದಿ (!(ps2ctr & I8042_CTR_KBDDIS))
                process_ಕೀಲಿ(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
ps2_keyboard_setup(ಶೂನ್ಯ *data)
{
    // flush incoming keys (also verifies port is likely present)
    ಪೂರ್ಣಾಙ್ಕ ret = i8042_flush();
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ರಿಕ್ತ);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ರಿಕ್ತ);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;
    ret = i8042_flush();
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ಪ್ರತ್ಯಾಗಮ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ಪ್ರತ್ಯಾಗಮ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ಪೂರ್ಣಾಙ್ಕ spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    ಪ್ರತಿ (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ಯದಿ (!ret)
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            ಯದಿ (spinupdelay)
                warn_timeout();
            ಪ್ರತ್ಯಾಗಮ;
        }
        yield();
    }
    ಯದಿ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ಪ್ರತ್ಯಾಗಮ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ರಿಕ್ತ);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ರಿಕ್ತ);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(1, "PS2 keyboard initialized\n");
}

ಶೂನ್ಯ
ps2port_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_PS2PORT)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ಪ್ರತ್ಯಾಗಮ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, ರಿಕ್ತ);
}
