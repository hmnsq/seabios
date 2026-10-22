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

स्थिर पूर्णांक
i8042_wait_read(शून्य)
{
    dprintf(7, "i8042_wait_read\n");
    पूर्णांक i;
    साठी (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        जर (status & I8042_STR_OBF)
            परत 0;
        udelay(50);
    }
    warn_timeout();
    परत -1;
}

स्थिर पूर्णांक
i8042_wait_write(शून्य)
{
    dprintf(7, "i8042_wait_write\n");
    पूर्णांक i;
    साठी (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        जर (! (status & I8042_STR_IBF))
            परत 0;
        udelay(50);
    }
    warn_timeout();
    परत -1;
}

स्थिर पूर्णांक
i8042_flush(शून्य)
{
    dprintf(7, "i8042_flush\n");
    पूर्णांक i;
    साठी (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        जर (! (status & I8042_STR_OBF))
            परत 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    परत -1;
}

स्थिर पूर्णांक
__i8042_command(पूर्णांक command, u8 *param)
{
    पूर्णांक receive = (command >> 8) & 0xf;
    पूर्णांक send = (command >> 12) & 0xf;

    // Send the command.
    पूर्णांक ret = i8042_wait_write();
    जर (ret)
        परत ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    पूर्णांक i;
    साठी (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        जर (ret)
            परत ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    साठी (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        जर (ret)
            परत ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    परत 0;
}

स्थिर पूर्णांक
i8042_command(पूर्णांक command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    पूर्णांक ret = __i8042_command(command, param);
    जर (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    परत ret;
}

स्थिर पूर्णांक
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    पूर्णांक ret = i8042_wait_write();
    जर (! ret)
        outb(c, PORT_PS2_DATA);
    परत ret;
}

स्थिर पूर्णांक
i8042_aux_write(u8 c)
{
    परत i8042_command(I8042_CMD_AUX_SEND, &c);
}

शून्य
i8042_reboot(शून्य)
{
    जर (! CONFIG_PS2PORT)
       परत;
    पूर्णांक i;
    साठी (i=0; i<10; i++) {
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

स्थिर पूर्णांक
ps2_recvbyte(पूर्णांक aux, पूर्णांक needack, पूर्णांक timeout)
{
    u32 end = timer_calc(timeout);
    साठी (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        जर (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            जर (!!(status & I8042_STR_AUXDATA) == aux) {
                जर (!needack)
                    परत data;
                जर (data == PS2_RET_ACK)
                    परत data;
                जर (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    परत data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        जर (timer_check(end)) {
            warn_timeout();
            परत -1;
        }
        yield();
    }
}

स्थिर पूर्णांक
ps2_sendbyte(पूर्णांक aux, u8 command, पूर्णांक timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    पूर्णांक ret;
    जर (aux)
        ret = i8042_aux_write(command);
    अन्यथा
        ret = i8042_kbd_write(command);
    जर (ret)
        परत ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    जर (ret < 0)
        परत ret;
    जर (ret != PS2_RET_ACK)
        परत -1;

    परत 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

स्थिर पूर्णांक
__ps2_command(पूर्णांक aux, पूर्णांक command, u8 *param)
{
    पूर्णांक ret2;
    पूर्णांक receive = (command >> 8) & 0xf;
    पूर्णांक send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_खालचा(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    पूर्णांक ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    जर (ret)
        परत ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_खालचा(Ps2ctr, newctr);
    जर (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    अन्यथा
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    जर (ret)
        जा fail;

    जर ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        जर (ret)
            जा fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        जर (ret < 0)
            जा fail;
        param[0] = ret;
        जर (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            जर (ret < 0)
                जा fail;
            param[1] = ret;
        }
    } अन्यथा जर (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        जर (ret)
            जा fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        जर (ret < 0)
            जा fail;
        param[0] = ret;
        जर (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            जर (ret < 0)
                जा fail;
            param[1] = ret;
        } अन्यथा {
            param[1] = 0;
        }
    } अन्यथा {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        जर (ret)
            जा fail;

        // Send parameters (if any).
        पूर्णांक i;
        साठी (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            जर (ret)
                जा fail;
        }

        // Receive parameters (if any).
        साठी (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            जर (ret < 0)
                जा fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_खालचा(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    जर (ret2)
        परत ret2;

    परत ret;
}

स्थिर पूर्णांक
ps2_command(पूर्णांक aux, पूर्णांक command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    पूर्णांक ret = __ps2_command(aux, command, param);
    जर (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    परत ret;
}

पूर्णांक
ps2_kbd_command(पूर्णांक command, u8 *param)
{
    जर (! CONFIG_PS2PORT)
        परत -1;
    परत ps2_command(0, command, param);
}

पूर्णांक
ps2_mouse_command(पूर्णांक command, u8 *param)
{
    जर (! CONFIG_PS2PORT)
        परत -1;

    // Update ps2ctr for mouse enable/disable.
    जर (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_खालचा(Ps2ctr);
        जर (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        अन्यथा
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_खालचा(Ps2ctr, ps2ctr);
    }

    परत ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
शून्य VISIBLE16
handle_74(शून्य)
{
    जर (! CONFIG_PS2PORT)
        परत;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    जर ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        जा done;
    }
    v = inb(PORT_PS2_DATA);

    जर (!(GET_खालचा(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        जा done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
शून्य VISIBLE16
handle_09(शून्य)
{
    जर (! CONFIG_PS2PORT)
        परत;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    जर (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        जा done;
    }
    v = inb(PORT_PS2_DATA);

    जर (!(GET_खालचा(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        जा done;

    process_किल्ली(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, रिक्त);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
शून्य
ps2_check_event(शून्य)
{
    जर (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        परत;
    u8 ps2ctr = GET_खालचा(Ps2ctr);
    जर ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        परत;
    साठी (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        जर (!(status & I8042_STR_OBF))
            थांबा;
        u8 data = inb(PORT_PS2_DATA);
        जर (status & I8042_STR_AUXDATA) {
            जर (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } अन्यथा {
            जर (!(ps2ctr & I8042_CTR_KBDDIS))
                process_किल्ली(data);
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
    पूर्णांक ret = i8042_flush();
    जर (ret)
        परत;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, रिक्त);
    जर (ret)
        परत;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, रिक्त);
    जर (ret)
        परत;
    ret = i8042_flush();
    जर (ret)
        परत;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    जर (ret)
        परत;
    जर (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        परत;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    जर (ret)
        परत;
    जर (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        परत;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    पूर्णांक spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    साठी (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        जर (!ret)
            थांबा;
        जर (timer_check(end)) {
            जर (spinupdelay)
                warn_timeout();
            परत;
        }
        yield();
    }
    जर (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        परत;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, रिक्त);
    जर (ret)
        परत;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    जर (ret)
        परत;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, रिक्त);
    जर (ret)
        परत;

    dprintf(1, "PS2 keyboard initialized\n");
}

शून्य
ps2port_setup(शून्य)
{
    ASSERT32FLAT();
    जर (! CONFIG_PS2PORT)
        परत;
    जर (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        परत;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, रिक्त);
}
