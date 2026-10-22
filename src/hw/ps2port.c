// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_LOW
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pic.h" // pic_eoi1
#அந்தர்பாவ "ps2port.h" // ps2_kbd_command
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "util.h" // udelay
#அந்தர்பாவ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

நிலையான முழு
i8042_wait_read(வெறுமை)
{
    dprintf(7, "i8042_wait_read\n");
    முழு i;
    க்கு (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        எனில் (status & I8042_STR_OBF)
            திரும்பு 0;
        udelay(50);
    }
    warn_timeout();
    திரும்பு -1;
}

நிலையான முழு
i8042_wait_write(வெறுமை)
{
    dprintf(7, "i8042_wait_write\n");
    முழு i;
    க்கு (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        எனில் (! (status & I8042_STR_IBF))
            திரும்பு 0;
        udelay(50);
    }
    warn_timeout();
    திரும்பு -1;
}

நிலையான முழு
i8042_flush(வெறுமை)
{
    dprintf(7, "i8042_flush\n");
    முழு i;
    க்கு (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        எனில் (! (status & I8042_STR_OBF))
            திரும்பு 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    திரும்பு -1;
}

நிலையான முழு
__i8042_command(முழு command, u8 *param)
{
    முழு receive = (command >> 8) & 0xf;
    முழு send = (command >> 12) & 0xf;

    // Send the command.
    முழு ret = i8042_wait_write();
    எனில் (ret)
        திரும்பு ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    முழு i;
    க்கு (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        எனில் (ret)
            திரும்பு ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    க்கு (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        எனில் (ret)
            திரும்பு ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    திரும்பு 0;
}

நிலையான முழு
i8042_command(முழு command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    முழு ret = __i8042_command(command, param);
    எனில் (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    திரும்பு ret;
}

நிலையான முழு
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    முழு ret = i8042_wait_write();
    எனில் (! ret)
        outb(c, PORT_PS2_DATA);
    திரும்பு ret;
}

நிலையான முழு
i8042_aux_write(u8 c)
{
    திரும்பு i8042_command(I8042_CMD_AUX_SEND, &c);
}

வெறுமை
i8042_reboot(வெறுமை)
{
    எனில் (! CONFIG_PS2PORT)
       திரும்பு;
    முழு i;
    க்கு (i=0; i<10; i++) {
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

நிலையான முழு
ps2_recvbyte(முழு aux, முழு needack, முழு timeout)
{
    u32 end = timer_calc(timeout);
    க்கு (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        எனில் (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            எனில் (!!(status & I8042_STR_AUXDATA) == aux) {
                எனில் (!needack)
                    திரும்பு data;
                எனில் (data == PS2_RET_ACK)
                    திரும்பு data;
                எனில் (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    திரும்பு data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }
}

நிலையான முழு
ps2_sendbyte(முழு aux, u8 command, முழு timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    முழு ret;
    எனில் (aux)
        ret = i8042_aux_write(command);
    இல்லையெனில்
        ret = i8042_kbd_write(command);
    எனில் (ret)
        திரும்பு ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    எனில் (ret < 0)
        திரும்பு ret;
    எனில் (ret != PS2_RET_ACK)
        திரும்பு -1;

    திரும்பு 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

நிலையான முழு
__ps2_command(முழு aux, முழு command, u8 *param)
{
    முழு ret2;
    முழு receive = (command >> 8) & 0xf;
    முழு send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_கீழ்(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    முழு ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    எனில் (ret)
        திரும்பு ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_கீழ்(Ps2ctr, newctr);
    எனில் (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    இல்லையெனில்
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    எனில் (ret)
        செல் fail;

    எனில் ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        எனில் (ret)
            செல் fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        எனில் (ret < 0)
            செல் fail;
        param[0] = ret;
        எனில் (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            எனில் (ret < 0)
                செல் fail;
            param[1] = ret;
        }
    } இல்லையெனில் எனில் (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        எனில் (ret)
            செல் fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        எனில் (ret < 0)
            செல் fail;
        param[0] = ret;
        எனில் (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            எனில் (ret < 0)
                செல் fail;
            param[1] = ret;
        } இல்லையெனில் {
            param[1] = 0;
        }
    } இல்லையெனில் {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        எனில் (ret)
            செல் fail;

        // Send parameters (if any).
        முழு i;
        க்கு (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            எனில் (ret)
                செல் fail;
        }

        // Receive parameters (if any).
        க்கு (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            எனில் (ret < 0)
                செல் fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_கீழ்(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    எனில் (ret2)
        திரும்பு ret2;

    திரும்பு ret;
}

நிலையான முழு
ps2_command(முழு aux, முழு command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    முழு ret = __ps2_command(aux, command, param);
    எனில் (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    திரும்பு ret;
}

முழு
ps2_kbd_command(முழு command, u8 *param)
{
    எனில் (! CONFIG_PS2PORT)
        திரும்பு -1;
    திரும்பு ps2_command(0, command, param);
}

முழு
ps2_mouse_command(முழு command, u8 *param)
{
    எனில் (! CONFIG_PS2PORT)
        திரும்பு -1;

    // Update ps2ctr for mouse enable/disable.
    எனில் (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_கீழ்(Ps2ctr);
        எனில் (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        இல்லையெனில்
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_கீழ்(Ps2ctr, ps2ctr);
    }

    திரும்பு ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
வெறுமை VISIBLE16
handle_74(வெறுமை)
{
    எனில் (! CONFIG_PS2PORT)
        திரும்பு;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    எனில் ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        செல் done;
    }
    v = inb(PORT_PS2_DATA);

    எனில் (!(GET_கீழ்(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        செல் done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
வெறுமை VISIBLE16
handle_09(வெறுமை)
{
    எனில் (! CONFIG_PS2PORT)
        திரும்பு;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    எனில் (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        செல் done;
    }
    v = inb(PORT_PS2_DATA);

    எனில் (!(GET_கீழ்(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        செல் done;

    process_சாவி(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, வெற்று);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
வெறுமை
ps2_check_event(வெறுமை)
{
    எனில் (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        திரும்பு;
    u8 ps2ctr = GET_கீழ்(Ps2ctr);
    எனில் ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        திரும்பு;
    க்கு (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        எனில் (!(status & I8042_STR_OBF))
            நிறுத்து;
        u8 data = inb(PORT_PS2_DATA);
        எனில் (status & I8042_STR_AUXDATA) {
            எனில் (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } இல்லையெனில் {
            எனில் (!(ps2ctr & I8042_CTR_KBDDIS))
                process_சாவி(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

நிலையான வெறுமை
ps2_keyboard_setup(வெறுமை *data)
{
    // flush incoming keys (also verifies port is likely present)
    முழு ret = i8042_flush();
    எனில் (ret)
        திரும்பு;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, வெற்று);
    எனில் (ret)
        திரும்பு;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, வெற்று);
    எனில் (ret)
        திரும்பு;
    ret = i8042_flush();
    எனில் (ret)
        திரும்பு;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    எனில் (ret)
        திரும்பு;
    எனில் (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        திரும்பு;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    எனில் (ret)
        திரும்பு;
    எனில் (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        திரும்பு;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    முழு spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    க்கு (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        எனில் (!ret)
            நிறுத்து;
        எனில் (timer_check(end)) {
            எனில் (spinupdelay)
                warn_timeout();
            திரும்பு;
        }
        yield();
    }
    எனில் (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        திரும்பு;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, வெற்று);
    எனில் (ret)
        திரும்பு;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    எனில் (ret)
        திரும்பு;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, வெற்று);
    எனில் (ret)
        திரும்பு;

    dprintf(1, "PS2 keyboard initialized\n");
}

வெறுமை
ps2port_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_PS2PORT)
        திரும்பு;
    எனில் (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        திரும்பு;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, வெற்று);
}
