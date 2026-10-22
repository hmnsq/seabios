// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_LOW
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pic.h" // pic_eoi1
#અન્તર્ભાવ "ps2port.h" // ps2_kbd_command
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "util.h" // udelay
#અન્તર્ભાવ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

સ્થિર પૂર્ણાંક
i8042_wait_read(શૂન્ય)
{
    dprintf(7, "i8042_wait_read\n");
    પૂર્ણાંક i;
    માટે (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        જો (status & I8042_STR_OBF)
            પાછા 0;
        udelay(50);
    }
    warn_timeout();
    પાછા -1;
}

સ્થિર પૂર્ણાંક
i8042_wait_write(શૂન્ય)
{
    dprintf(7, "i8042_wait_write\n");
    પૂર્ણાંક i;
    માટે (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        જો (! (status & I8042_STR_IBF))
            પાછા 0;
        udelay(50);
    }
    warn_timeout();
    પાછા -1;
}

સ્થિર પૂર્ણાંક
i8042_flush(શૂન્ય)
{
    dprintf(7, "i8042_flush\n");
    પૂર્ણાંક i;
    માટે (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        જો (! (status & I8042_STR_OBF))
            પાછા 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    પાછા -1;
}

સ્થિર પૂર્ણાંક
__i8042_command(પૂર્ણાંક command, u8 *param)
{
    પૂર્ણાંક receive = (command >> 8) & 0xf;
    પૂર્ણાંક send = (command >> 12) & 0xf;

    // Send the command.
    પૂર્ણાંક ret = i8042_wait_write();
    જો (ret)
        પાછા ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    પૂર્ણાંક i;
    માટે (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        જો (ret)
            પાછા ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    માટે (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        જો (ret)
            પાછા ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    પાછા 0;
}

સ્થિર પૂર્ણાંક
i8042_command(પૂર્ણાંક command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    પૂર્ણાંક ret = __i8042_command(command, param);
    જો (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    પાછા ret;
}

સ્થિર પૂર્ણાંક
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    પૂર્ણાંક ret = i8042_wait_write();
    જો (! ret)
        outb(c, PORT_PS2_DATA);
    પાછા ret;
}

સ્થિર પૂર્ણાંક
i8042_aux_write(u8 c)
{
    પાછા i8042_command(I8042_CMD_AUX_SEND, &c);
}

શૂન્ય
i8042_reboot(શૂન્ય)
{
    જો (! CONFIG_PS2PORT)
       પાછા;
    પૂર્ણાંક i;
    માટે (i=0; i<10; i++) {
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

સ્થિર પૂર્ણાંક
ps2_recvbyte(પૂર્ણાંક aux, પૂર્ણાંક needack, પૂર્ણાંક timeout)
{
    u32 end = timer_calc(timeout);
    માટે (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        જો (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            જો (!!(status & I8042_STR_AUXDATA) == aux) {
                જો (!needack)
                    પાછા data;
                જો (data == PS2_RET_ACK)
                    પાછા data;
                જો (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    પાછા data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        જો (timer_check(end)) {
            warn_timeout();
            પાછા -1;
        }
        yield();
    }
}

સ્થિર પૂર્ણાંક
ps2_sendbyte(પૂર્ણાંક aux, u8 command, પૂર્ણાંક timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    પૂર્ણાંક ret;
    જો (aux)
        ret = i8042_aux_write(command);
    નહીંતર
        ret = i8042_kbd_write(command);
    જો (ret)
        પાછા ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    જો (ret < 0)
        પાછા ret;
    જો (ret != PS2_RET_ACK)
        પાછા -1;

    પાછા 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

સ્થિર પૂર્ણાંક
__ps2_command(પૂર્ણાંક aux, પૂર્ણાંક command, u8 *param)
{
    પૂર્ણાંક ret2;
    પૂર્ણાંક receive = (command >> 8) & 0xf;
    પૂર્ણાંક send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_નીચલું(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    પૂર્ણાંક ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    જો (ret)
        પાછા ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_નીચલું(Ps2ctr, newctr);
    જો (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    નહીંતર
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    જો (ret)
        જાઓ fail;

    જો ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        જો (ret)
            જાઓ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        જો (ret < 0)
            જાઓ fail;
        param[0] = ret;
        જો (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            જો (ret < 0)
                જાઓ fail;
            param[1] = ret;
        }
    } નહીંતર જો (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        જો (ret)
            જાઓ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        જો (ret < 0)
            જાઓ fail;
        param[0] = ret;
        જો (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            જો (ret < 0)
                જાઓ fail;
            param[1] = ret;
        } નહીંતર {
            param[1] = 0;
        }
    } નહીંતર {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        જો (ret)
            જાઓ fail;

        // Send parameters (if any).
        પૂર્ણાંક i;
        માટે (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            જો (ret)
                જાઓ fail;
        }

        // Receive parameters (if any).
        માટે (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            જો (ret < 0)
                જાઓ fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_નીચલું(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    જો (ret2)
        પાછા ret2;

    પાછા ret;
}

સ્થિર પૂર્ણાંક
ps2_command(પૂર્ણાંક aux, પૂર્ણાંક command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    પૂર્ણાંક ret = __ps2_command(aux, command, param);
    જો (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    પાછા ret;
}

પૂર્ણાંક
ps2_kbd_command(પૂર્ણાંક command, u8 *param)
{
    જો (! CONFIG_PS2PORT)
        પાછા -1;
    પાછા ps2_command(0, command, param);
}

પૂર્ણાંક
ps2_mouse_command(પૂર્ણાંક command, u8 *param)
{
    જો (! CONFIG_PS2PORT)
        પાછા -1;

    // Update ps2ctr for mouse enable/disable.
    જો (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_નીચલું(Ps2ctr);
        જો (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        નહીંતર
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_નીચલું(Ps2ctr, ps2ctr);
    }

    પાછા ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
શૂન્ય VISIBLE16
handle_74(શૂન્ય)
{
    જો (! CONFIG_PS2PORT)
        પાછા;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    જો ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        જાઓ done;
    }
    v = inb(PORT_PS2_DATA);

    જો (!(GET_નીચલું(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        જાઓ done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
શૂન્ય VISIBLE16
handle_09(શૂન્ય)
{
    જો (! CONFIG_PS2PORT)
        પાછા;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    જો (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        જાઓ done;
    }
    v = inb(PORT_PS2_DATA);

    જો (!(GET_નીચલું(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        જાઓ done;

    process_ચાવી(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ખાલી);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
શૂન્ય
ps2_check_event(શૂન્ય)
{
    જો (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        પાછા;
    u8 ps2ctr = GET_નીચલું(Ps2ctr);
    જો ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        પાછા;
    માટે (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        જો (!(status & I8042_STR_OBF))
            તોડો;
        u8 data = inb(PORT_PS2_DATA);
        જો (status & I8042_STR_AUXDATA) {
            જો (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } નહીંતર {
            જો (!(ps2ctr & I8042_CTR_KBDDIS))
                process_ચાવી(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

સ્થિર શૂન્ય
ps2_keyboard_setup(શૂન્ય *data)
{
    // flush incoming keys (also verifies port is likely present)
    પૂર્ણાંક ret = i8042_flush();
    જો (ret)
        પાછા;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ખાલી);
    જો (ret)
        પાછા;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ખાલી);
    જો (ret)
        પાછા;
    ret = i8042_flush();
    જો (ret)
        પાછા;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    જો (ret)
        પાછા;
    જો (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        પાછા;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    જો (ret)
        પાછા;
    જો (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        પાછા;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    પૂર્ણાંક spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    માટે (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        જો (!ret)
            તોડો;
        જો (timer_check(end)) {
            જો (spinupdelay)
                warn_timeout();
            પાછા;
        }
        yield();
    }
    જો (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        પાછા;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ખાલી);
    જો (ret)
        પાછા;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    જો (ret)
        પાછા;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ખાલી);
    જો (ret)
        પાછા;

    dprintf(1, "PS2 keyboard initialized\n");
}

શૂન્ય
ps2port_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (! CONFIG_PS2PORT)
        પાછા;
    જો (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        પાછા;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, ખાલી);
}
