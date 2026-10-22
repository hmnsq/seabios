// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_LOW
#ضمن "output.h" // dprintf
#ضمن "pic.h" // pic_eoi1
#ضمن "ps2port.h" // ps2_kbd_command
#ضمن "romfile.h" // romfile_loadint
#ضمن "stacks.h" // yield
#ضمن "util.h" // udelay
#ضمن "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

ساكن صحيح
i8042_wait_read(فراغ)
{
    dprintf(7, "i8042_wait_read\n");
    صحيح i;
    لكل (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        إذا (status & I8042_STR_OBF)
            أرجع 0;
        udelay(50);
    }
    warn_timeout();
    أرجع -1;
}

ساكن صحيح
i8042_wait_write(فراغ)
{
    dprintf(7, "i8042_wait_write\n");
    صحيح i;
    لكل (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        إذا (! (status & I8042_STR_IBF))
            أرجع 0;
        udelay(50);
    }
    warn_timeout();
    أرجع -1;
}

ساكن صحيح
i8042_flush(فراغ)
{
    dprintf(7, "i8042_flush\n");
    صحيح i;
    لكل (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        إذا (! (status & I8042_STR_OBF))
            أرجع 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    أرجع -1;
}

ساكن صحيح
__i8042_command(صحيح command, u8 *param)
{
    صحيح receive = (command >> 8) & 0xf;
    صحيح send = (command >> 12) & 0xf;

    // Send the command.
    صحيح ret = i8042_wait_write();
    إذا (ret)
        أرجع ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    صحيح i;
    لكل (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        إذا (ret)
            أرجع ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    لكل (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        إذا (ret)
            أرجع ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    أرجع 0;
}

ساكن صحيح
i8042_command(صحيح command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    صحيح ret = __i8042_command(command, param);
    إذا (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    أرجع ret;
}

ساكن صحيح
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    صحيح ret = i8042_wait_write();
    إذا (! ret)
        outb(c, PORT_PS2_DATA);
    أرجع ret;
}

ساكن صحيح
i8042_aux_write(u8 c)
{
    أرجع i8042_command(I8042_CMD_AUX_SEND, &c);
}

فراغ
i8042_reboot(فراغ)
{
    إذا (! CONFIG_PS2PORT)
       أرجع;
    صحيح i;
    لكل (i=0; i<10; i++) {
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

ساكن صحيح
ps2_recvbyte(صحيح aux, صحيح needack, صحيح timeout)
{
    u32 end = timer_calc(timeout);
    لكل (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        إذا (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            إذا (!!(status & I8042_STR_AUXDATA) == aux) {
                إذا (!needack)
                    أرجع data;
                إذا (data == PS2_RET_ACK)
                    أرجع data;
                إذا (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    أرجع data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        إذا (timer_check(end)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
}

ساكن صحيح
ps2_sendbyte(صحيح aux, u8 command, صحيح timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    صحيح ret;
    إذا (aux)
        ret = i8042_aux_write(command);
    وإلا
        ret = i8042_kbd_write(command);
    إذا (ret)
        أرجع ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    إذا (ret < 0)
        أرجع ret;
    إذا (ret != PS2_RET_ACK)
        أرجع -1;

    أرجع 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

ساكن صحيح
__ps2_command(صحيح aux, صحيح command, u8 *param)
{
    صحيح ret2;
    صحيح receive = (command >> 8) & 0xf;
    صحيح send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_كس(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    صحيح ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    إذا (ret)
        أرجع ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_كس(Ps2ctr, newctr);
    إذا (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    وإلا
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    إذا (ret)
        اذهب fail;

    إذا ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        إذا (ret)
            اذهب fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        إذا (ret < 0)
            اذهب fail;
        param[0] = ret;
        إذا (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            إذا (ret < 0)
                اذهب fail;
            param[1] = ret;
        }
    } وإلا إذا (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        إذا (ret)
            اذهب fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        إذا (ret < 0)
            اذهب fail;
        param[0] = ret;
        إذا (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            إذا (ret < 0)
                اذهب fail;
            param[1] = ret;
        } وإلا {
            param[1] = 0;
        }
    } وإلا {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        إذا (ret)
            اذهب fail;

        // Send parameters (if any).
        صحيح i;
        لكل (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            إذا (ret)
                اذهب fail;
        }

        // Receive parameters (if any).
        لكل (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            إذا (ret < 0)
                اذهب fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_كس(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    إذا (ret2)
        أرجع ret2;

    أرجع ret;
}

ساكن صحيح
ps2_command(صحيح aux, صحيح command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    صحيح ret = __ps2_command(aux, command, param);
    إذا (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    أرجع ret;
}

صحيح
ps2_kbd_command(صحيح command, u8 *param)
{
    إذا (! CONFIG_PS2PORT)
        أرجع -1;
    أرجع ps2_command(0, command, param);
}

صحيح
ps2_mouse_command(صحيح command, u8 *param)
{
    إذا (! CONFIG_PS2PORT)
        أرجع -1;

    // Update ps2ctr for mouse enable/disable.
    إذا (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_كس(Ps2ctr);
        إذا (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        وإلا
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_كس(Ps2ctr, ps2ctr);
    }

    أرجع ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
فراغ VISIBLE16
handle_74(فراغ)
{
    إذا (! CONFIG_PS2PORT)
        أرجع;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    إذا ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        اذهب done;
    }
    v = inb(PORT_PS2_DATA);

    إذا (!(GET_كس(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        اذهب done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
فراغ VISIBLE16
handle_09(فراغ)
{
    إذا (! CONFIG_PS2PORT)
        أرجع;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    إذا (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        اذهب done;
    }
    v = inb(PORT_PS2_DATA);

    إذا (!(GET_كس(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        اذهب done;

    process_مبودي(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, عدم);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
فراغ
ps2_check_event(فراغ)
{
    إذا (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        أرجع;
    u8 ps2ctr = GET_كس(Ps2ctr);
    إذا ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        أرجع;
    لكل (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        إذا (!(status & I8042_STR_OBF))
            اكسر;
        u8 data = inb(PORT_PS2_DATA);
        إذا (status & I8042_STR_AUXDATA) {
            إذا (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } وإلا {
            إذا (!(ps2ctr & I8042_CTR_KBDDIS))
                process_مبودي(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ساكن فراغ
ps2_keyboard_setup(فراغ *data)
{
    // flush incoming keys (also verifies port is likely present)
    صحيح ret = i8042_flush();
    إذا (ret)
        أرجع;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, عدم);
    إذا (ret)
        أرجع;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, عدم);
    إذا (ret)
        أرجع;
    ret = i8042_flush();
    إذا (ret)
        أرجع;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    إذا (ret)
        أرجع;
    إذا (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        أرجع;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    إذا (ret)
        أرجع;
    إذا (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        أرجع;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    صحيح spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    لكل (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        إذا (!ret)
            اكسر;
        إذا (timer_check(end)) {
            إذا (spinupdelay)
                warn_timeout();
            أرجع;
        }
        yield();
    }
    إذا (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        أرجع;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, عدم);
    إذا (ret)
        أرجع;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    إذا (ret)
        أرجع;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, عدم);
    إذا (ret)
        أرجع;

    dprintf(1, "PS2 keyboard initialized\n");
}

فراغ
ps2port_setup(فراغ)
{
    ASSERT32FLAT();
    إذا (! CONFIG_PS2PORT)
        أرجع;
    إذا (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        أرجع;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, عدم);
}
