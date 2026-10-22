// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_LOW
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pic.h" // pic_eoi1
#अन्तर्भाव "ps2port.h" // ps2_kbd_command
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "util.h" // udelay
#अन्तर्भाव "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

स्थिर पूर्णाङ्क
i8042_wait_read(शून्य)
{
    dprintf(7, "i8042_wait_read\n");
    पूर्णाङ्क i;
    प्रति (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        जइ (status & I8042_STR_OBF)
            प्रत्यागम 0;
        udelay(50);
    }
    warn_timeout();
    प्रत्यागम -1;
}

स्थिर पूर्णाङ्क
i8042_wait_write(शून्य)
{
    dprintf(7, "i8042_wait_write\n");
    पूर्णाङ्क i;
    प्रति (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        जइ (! (status & I8042_STR_IBF))
            प्रत्यागम 0;
        udelay(50);
    }
    warn_timeout();
    प्रत्यागम -1;
}

स्थिर पूर्णाङ्क
i8042_flush(शून्य)
{
    dprintf(7, "i8042_flush\n");
    पूर्णाङ्क i;
    प्रति (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        जइ (! (status & I8042_STR_OBF))
            प्रत्यागम 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    प्रत्यागम -1;
}

स्थिर पूर्णाङ्क
__i8042_command(पूर्णाङ्क command, u8 *param)
{
    पूर्णाङ्क receive = (command >> 8) & 0xf;
    पूर्णाङ्क send = (command >> 12) & 0xf;

    // Send the command.
    पूर्णाङ्क ret = i8042_wait_write();
    जइ (ret)
        प्रत्यागम ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    पूर्णाङ्क i;
    प्रति (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        जइ (ret)
            प्रत्यागम ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    प्रति (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        जइ (ret)
            प्रत्यागम ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    प्रत्यागम 0;
}

स्थिर पूर्णाङ्क
i8042_command(पूर्णाङ्क command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    पूर्णाङ्क ret = __i8042_command(command, param);
    जइ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    प्रत्यागम ret;
}

स्थिर पूर्णाङ्क
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    पूर्णाङ्क ret = i8042_wait_write();
    जइ (! ret)
        outb(c, PORT_PS2_DATA);
    प्रत्यागम ret;
}

स्थिर पूर्णाङ्क
i8042_aux_write(u8 c)
{
    प्रत्यागम i8042_command(I8042_CMD_AUX_SEND, &c);
}

शून्य
i8042_reboot(शून्य)
{
    जइ (! CONFIG_PS2PORT)
       प्रत्यागम;
    पूर्णाङ्क i;
    प्रति (i=0; i<10; i++) {
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

स्थिर पूर्णाङ्क
ps2_recvbyte(पूर्णाङ्क aux, पूर्णाङ्क needack, पूर्णाङ्क timeout)
{
    u32 end = timer_calc(timeout);
    प्रति (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        जइ (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            जइ (!!(status & I8042_STR_AUXDATA) == aux) {
                जइ (!needack)
                    प्रत्यागम data;
                जइ (data == PS2_RET_ACK)
                    प्रत्यागम data;
                जइ (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    प्रत्यागम data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        जइ (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }
}

स्थिर पूर्णाङ्क
ps2_sendbyte(पूर्णाङ्क aux, u8 command, पूर्णाङ्क timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    पूर्णाङ्क ret;
    जइ (aux)
        ret = i8042_aux_write(command);
    अण्णहा
        ret = i8042_kbd_write(command);
    जइ (ret)
        प्रत्यागम ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    जइ (ret < 0)
        प्रत्यागम ret;
    जइ (ret != PS2_RET_ACK)
        प्रत्यागम -1;

    प्रत्यागम 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

स्थिर पूर्णाङ्क
__ps2_command(पूर्णाङ्क aux, पूर्णाङ्क command, u8 *param)
{
    पूर्णाङ्क ret2;
    पूर्णाङ्क receive = (command >> 8) & 0xf;
    पूर्णाङ्क send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_नीचः(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    पूर्णाङ्क ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    जइ (ret)
        प्रत्यागम ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_नीचः(Ps2ctr, newctr);
    जइ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    अण्णहा
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    जइ (ret)
        गच्छ fail;

    जइ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        जइ (ret)
            गच्छ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        जइ (ret < 0)
            गच्छ fail;
        param[0] = ret;
        जइ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            जइ (ret < 0)
                गच्छ fail;
            param[1] = ret;
        }
    } अण्णहा जइ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        जइ (ret)
            गच्छ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        जइ (ret < 0)
            गच्छ fail;
        param[0] = ret;
        जइ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            जइ (ret < 0)
                गच्छ fail;
            param[1] = ret;
        } अण्णहा {
            param[1] = 0;
        }
    } अण्णहा {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        जइ (ret)
            गच्छ fail;

        // Send parameters (if any).
        पूर्णाङ्क i;
        प्रति (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            जइ (ret)
                गच्छ fail;
        }

        // Receive parameters (if any).
        प्रति (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            जइ (ret < 0)
                गच्छ fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_नीचः(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    जइ (ret2)
        प्रत्यागम ret2;

    प्रत्यागम ret;
}

स्थिर पूर्णाङ्क
ps2_command(पूर्णाङ्क aux, पूर्णाङ्क command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    पूर्णाङ्क ret = __ps2_command(aux, command, param);
    जइ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    प्रत्यागम ret;
}

पूर्णाङ्क
ps2_kbd_command(पूर्णाङ्क command, u8 *param)
{
    जइ (! CONFIG_PS2PORT)
        प्रत्यागम -1;
    प्रत्यागम ps2_command(0, command, param);
}

पूर्णाङ्क
ps2_mouse_command(पूर्णाङ्क command, u8 *param)
{
    जइ (! CONFIG_PS2PORT)
        प्रत्यागम -1;

    // Update ps2ctr for mouse enable/disable.
    जइ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_नीचः(Ps2ctr);
        जइ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        अण्णहा
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_नीचः(Ps2ctr, ps2ctr);
    }

    प्रत्यागम ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
शून्य VISIBLE16
handle_74(शून्य)
{
    जइ (! CONFIG_PS2PORT)
        प्रत्यागम;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    जइ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        गच्छ done;
    }
    v = inb(PORT_PS2_DATA);

    जइ (!(GET_नीचः(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        गच्छ done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
शून्य VISIBLE16
handle_09(शून्य)
{
    जइ (! CONFIG_PS2PORT)
        प्रत्यागम;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    जइ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        गच्छ done;
    }
    v = inb(PORT_PS2_DATA);

    जइ (!(GET_नीचः(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        गच्छ done;

    process_कुञ्चिका(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, रिक्त);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
शून्य
ps2_check_event(शून्य)
{
    जइ (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        प्रत्यागम;
    u8 ps2ctr = GET_नीचः(Ps2ctr);
    जइ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        प्रत्यागम;
    प्रति (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        जइ (!(status & I8042_STR_OBF))
            भङ्ग;
        u8 data = inb(PORT_PS2_DATA);
        जइ (status & I8042_STR_AUXDATA) {
            जइ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } अण्णहा {
            जइ (!(ps2ctr & I8042_CTR_KBDDIS))
                process_कुञ्चिका(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

स्थिर शून्य
ps2_keyboard_setup(शून्य *data)
{
    // flush incoming keys (also verifies port is likely present)
    पूर्णाङ्क ret = i8042_flush();
    जइ (ret)
        प्रत्यागम;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, रिक्त);
    जइ (ret)
        प्रत्यागम;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, रिक्त);
    जइ (ret)
        प्रत्यागम;
    ret = i8042_flush();
    जइ (ret)
        प्रत्यागम;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    जइ (ret)
        प्रत्यागम;
    जइ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        प्रत्यागम;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    जइ (ret)
        प्रत्यागम;
    जइ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        प्रत्यागम;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    पूर्णाङ्क spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    प्रति (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        जइ (!ret)
            भङ्ग;
        जइ (timer_check(end)) {
            जइ (spinupdelay)
                warn_timeout();
            प्रत्यागम;
        }
        yield();
    }
    जइ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        प्रत्यागम;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, रिक्त);
    जइ (ret)
        प्रत्यागम;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    जइ (ret)
        प्रत्यागम;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, रिक्त);
    जइ (ret)
        प्रत्यागम;

    dprintf(1, "PS2 keyboard initialized\n");
}

शून्य
ps2port_setup(शून्य)
{
    ASSERT32FLAT();
    जइ (! CONFIG_PS2PORT)
        प्रत्यागम;
    जइ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        प्रत्यागम;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, रिक्त);
}
