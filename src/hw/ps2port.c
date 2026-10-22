// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_LOW
#شامل "output.h" // dprintf
#شامل "pic.h" // pic_eoi1
#شامل "ps2port.h" // ps2_kbd_command
#شامل "romfile.h" // romfile_loadint
#شامل "stacks.h" // yield
#شامل "util.h" // udelay
#شامل "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

ایستا صحیح
i8042_wait_read(تهی)
{
    dprintf(7, "i8042_wait_read\n");
    صحیح i;
    برای (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        اگر (status & I8042_STR_OBF)
            بازگشت 0;
        udelay(50);
    }
    warn_timeout();
    بازگشت -1;
}

ایستا صحیح
i8042_wait_write(تهی)
{
    dprintf(7, "i8042_wait_write\n");
    صحیح i;
    برای (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        اگر (! (status & I8042_STR_IBF))
            بازگشت 0;
        udelay(50);
    }
    warn_timeout();
    بازگشت -1;
}

ایستا صحیح
i8042_flush(تهی)
{
    dprintf(7, "i8042_flush\n");
    صحیح i;
    برای (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        اگر (! (status & I8042_STR_OBF))
            بازگشت 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    بازگشت -1;
}

ایستا صحیح
__i8042_command(صحیح command, u8 *param)
{
    صحیح receive = (command >> 8) & 0xf;
    صحیح send = (command >> 12) & 0xf;

    // Send the command.
    صحیح ret = i8042_wait_write();
    اگر (ret)
        بازگشت ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    صحیح i;
    برای (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        اگر (ret)
            بازگشت ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    برای (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        اگر (ret)
            بازگشت ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    بازگشت 0;
}

ایستا صحیح
i8042_command(صحیح command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    صحیح ret = __i8042_command(command, param);
    اگر (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    بازگشت ret;
}

ایستا صحیح
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    صحیح ret = i8042_wait_write();
    اگر (! ret)
        outb(c, PORT_PS2_DATA);
    بازگشت ret;
}

ایستا صحیح
i8042_aux_write(u8 c)
{
    بازگشت i8042_command(I8042_CMD_AUX_SEND, &c);
}

تهی
i8042_reboot(تهی)
{
    اگر (! CONFIG_PS2PORT)
       بازگشت;
    صحیح i;
    برای (i=0; i<10; i++) {
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

ایستا صحیح
ps2_recvbyte(صحیح aux, صحیح needack, صحیح timeout)
{
    u32 end = timer_calc(timeout);
    برای (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        اگر (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            اگر (!!(status & I8042_STR_AUXDATA) == aux) {
                اگر (!needack)
                    بازگشت data;
                اگر (data == PS2_RET_ACK)
                    بازگشت data;
                اگر (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    بازگشت data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت -1;
        }
        yield();
    }
}

ایستا صحیح
ps2_sendbyte(صحیح aux, u8 command, صحیح timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    صحیح ret;
    اگر (aux)
        ret = i8042_aux_write(command);
    وگرنه
        ret = i8042_kbd_write(command);
    اگر (ret)
        بازگشت ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    اگر (ret < 0)
        بازگشت ret;
    اگر (ret != PS2_RET_ACK)
        بازگشت -1;

    بازگشت 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

ایستا صحیح
__ps2_command(صحیح aux, صحیح command, u8 *param)
{
    صحیح ret2;
    صحیح receive = (command >> 8) & 0xf;
    صحیح send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_کم(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    صحیح ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    اگر (ret)
        بازگشت ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_کم(Ps2ctr, newctr);
    اگر (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    وگرنه
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    اگر (ret)
        برو fail;

    اگر ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        اگر (ret)
            برو fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        اگر (ret < 0)
            برو fail;
        param[0] = ret;
        اگر (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            اگر (ret < 0)
                برو fail;
            param[1] = ret;
        }
    } وگرنه اگر (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        اگر (ret)
            برو fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        اگر (ret < 0)
            برو fail;
        param[0] = ret;
        اگر (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            اگر (ret < 0)
                برو fail;
            param[1] = ret;
        } وگرنه {
            param[1] = 0;
        }
    } وگرنه {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        اگر (ret)
            برو fail;

        // Send parameters (if any).
        صحیح i;
        برای (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            اگر (ret)
                برو fail;
        }

        // Receive parameters (if any).
        برای (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            اگر (ret < 0)
                برو fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_کم(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    اگر (ret2)
        بازگشت ret2;

    بازگشت ret;
}

ایستا صحیح
ps2_command(صحیح aux, صحیح command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    صحیح ret = __ps2_command(aux, command, param);
    اگر (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    بازگشت ret;
}

صحیح
ps2_kbd_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_PS2PORT)
        بازگشت -1;
    بازگشت ps2_command(0, command, param);
}

صحیح
ps2_mouse_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_PS2PORT)
        بازگشت -1;

    // Update ps2ctr for mouse enable/disable.
    اگر (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_کم(Ps2ctr);
        اگر (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        وگرنه
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_کم(Ps2ctr, ps2ctr);
    }

    بازگشت ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
تهی VISIBLE16
handle_74(تهی)
{
    اگر (! CONFIG_PS2PORT)
        بازگشت;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    اگر ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        برو done;
    }
    v = inb(PORT_PS2_DATA);

    اگر (!(GET_کم(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        برو done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
تهی VISIBLE16
handle_09(تهی)
{
    اگر (! CONFIG_PS2PORT)
        بازگشت;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    اگر (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        برو done;
    }
    v = inb(PORT_PS2_DATA);

    اگر (!(GET_کم(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        برو done;

    process_کلید(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, پوچ);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
تهی
ps2_check_event(تهی)
{
    اگر (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        بازگشت;
    u8 ps2ctr = GET_کم(Ps2ctr);
    اگر ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        بازگشت;
    برای (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        اگر (!(status & I8042_STR_OBF))
            بشکن;
        u8 data = inb(PORT_PS2_DATA);
        اگر (status & I8042_STR_AUXDATA) {
            اگر (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } وگرنه {
            اگر (!(ps2ctr & I8042_CTR_KBDDIS))
                process_کلید(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ایستا تهی
ps2_keyboard_setup(تهی *data)
{
    // flush incoming keys (also verifies port is likely present)
    صحیح ret = i8042_flush();
    اگر (ret)
        بازگشت;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, پوچ);
    اگر (ret)
        بازگشت;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, پوچ);
    اگر (ret)
        بازگشت;
    ret = i8042_flush();
    اگر (ret)
        بازگشت;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    اگر (ret)
        بازگشت;
    اگر (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        بازگشت;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    اگر (ret)
        بازگشت;
    اگر (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        بازگشت;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    صحیح spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    برای (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        اگر (!ret)
            بشکن;
        اگر (timer_check(end)) {
            اگر (spinupdelay)
                warn_timeout();
            بازگشت;
        }
        yield();
    }
    اگر (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        بازگشت;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, پوچ);
    اگر (ret)
        بازگشت;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    اگر (ret)
        بازگشت;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, پوچ);
    اگر (ret)
        بازگشت;

    dprintf(1, "PS2 keyboard initialized\n");
}

تهی
ps2port_setup(تهی)
{
    ASSERT32FLAT();
    اگر (! CONFIG_PS2PORT)
        بازگشت;
    اگر (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        بازگشت;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, پوچ);
}
