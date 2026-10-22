// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_LOW
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pic.h" // pic_eoi1
#അന്തര്ഭാവ "ps2port.h" // ps2_kbd_command
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "util.h" // udelay
#അന്തര്ഭാവ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

സ്ഥിതം പൂർണ്ണസംഖ്യ
i8042_wait_read(ശൂന്യം)
{
    dprintf(7, "i8042_wait_read\n");
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        എങ്കിൽ (status & I8042_STR_OBF)
            തിരികെ 0;
        udelay(50);
    }
    warn_timeout();
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
i8042_wait_write(ശൂന്യം)
{
    dprintf(7, "i8042_wait_write\n");
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        എങ്കിൽ (! (status & I8042_STR_IBF))
            തിരികെ 0;
        udelay(50);
    }
    warn_timeout();
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
i8042_flush(ശൂന്യം)
{
    dprintf(7, "i8042_flush\n");
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        എങ്കിൽ (! (status & I8042_STR_OBF))
            തിരികെ 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
__i8042_command(പൂർണ്ണസംഖ്യ command, u8 *param)
{
    പൂർണ്ണസംഖ്യ receive = (command >> 8) & 0xf;
    പൂർണ്ണസംഖ്യ send = (command >> 12) & 0xf;

    // Send the command.
    പൂർണ്ണസംഖ്യ ret = i8042_wait_write();
    എങ്കിൽ (ret)
        തിരികെ ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        എങ്കിൽ (ret)
            തിരികെ ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    വേണ്ടി (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        എങ്കിൽ (ret)
            തിരികെ ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
i8042_command(പൂർണ്ണസംഖ്യ command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    പൂർണ്ണസംഖ്യ ret = __i8042_command(command, param);
    എങ്കിൽ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    പൂർണ്ണസംഖ്യ ret = i8042_wait_write();
    എങ്കിൽ (! ret)
        outb(c, PORT_PS2_DATA);
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
i8042_aux_write(u8 c)
{
    തിരികെ i8042_command(I8042_CMD_AUX_SEND, &c);
}

ശൂന്യം
i8042_reboot(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PS2PORT)
       തിരികെ;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<10; i++) {
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

സ്ഥിതം പൂർണ്ണസംഖ്യ
ps2_recvbyte(പൂർണ്ണസംഖ്യ aux, പൂർണ്ണസംഖ്യ needack, പൂർണ്ണസംഖ്യ timeout)
{
    u32 end = timer_calc(timeout);
    വേണ്ടി (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        എങ്കിൽ (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            എങ്കിൽ (!!(status & I8042_STR_AUXDATA) == aux) {
                എങ്കിൽ (!needack)
                    തിരികെ data;
                എങ്കിൽ (data == PS2_RET_ACK)
                    തിരികെ data;
                എങ്കിൽ (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    തിരികെ data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ -1;
        }
        yield();
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
ps2_sendbyte(പൂർണ്ണസംഖ്യ aux, u8 command, പൂർണ്ണസംഖ്യ timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    പൂർണ്ണസംഖ്യ ret;
    എങ്കിൽ (aux)
        ret = i8042_aux_write(command);
    അല്ലെങ്കിൽ
        ret = i8042_kbd_write(command);
    എങ്കിൽ (ret)
        തിരികെ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    എങ്കിൽ (ret < 0)
        തിരികെ ret;
    എങ്കിൽ (ret != PS2_RET_ACK)
        തിരികെ -1;

    തിരികെ 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

സ്ഥിതം പൂർണ്ണസംഖ്യ
__ps2_command(പൂർണ്ണസംഖ്യ aux, പൂർണ്ണസംഖ്യ command, u8 *param)
{
    പൂർണ്ണസംഖ്യ ret2;
    പൂർണ്ണസംഖ്യ receive = (command >> 8) & 0xf;
    പൂർണ്ണസംഖ്യ send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_താഴെയുള്ള(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    പൂർണ്ണസംഖ്യ ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    എങ്കിൽ (ret)
        തിരികെ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_താഴെയുള്ള(Ps2ctr, newctr);
    എങ്കിൽ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    അല്ലെങ്കിൽ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    എങ്കിൽ (ret)
        പോകുക fail;

    എങ്കിൽ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        എങ്കിൽ (ret)
            പോകുക fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        എങ്കിൽ (ret < 0)
            പോകുക fail;
        param[0] = ret;
        എങ്കിൽ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            എങ്കിൽ (ret < 0)
                പോകുക fail;
            param[1] = ret;
        }
    } അല്ലെങ്കിൽ എങ്കിൽ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        എങ്കിൽ (ret)
            പോകുക fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        എങ്കിൽ (ret < 0)
            പോകുക fail;
        param[0] = ret;
        എങ്കിൽ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            എങ്കിൽ (ret < 0)
                പോകുക fail;
            param[1] = ret;
        } അല്ലെങ്കിൽ {
            param[1] = 0;
        }
    } അല്ലെങ്കിൽ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        എങ്കിൽ (ret)
            പോകുക fail;

        // Send parameters (if any).
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            എങ്കിൽ (ret)
                പോകുക fail;
        }

        // Receive parameters (if any).
        വേണ്ടി (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            എങ്കിൽ (ret < 0)
                പോകുക fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_താഴെയുള്ള(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    എങ്കിൽ (ret2)
        തിരികെ ret2;

    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
ps2_command(പൂർണ്ണസംഖ്യ aux, പൂർണ്ണസംഖ്യ command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    പൂർണ്ണസംഖ്യ ret = __ps2_command(aux, command, param);
    എങ്കിൽ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    തിരികെ ret;
}

പൂർണ്ണസംഖ്യ
ps2_kbd_command(പൂർണ്ണസംഖ്യ command, u8 *param)
{
    എങ്കിൽ (! CONFIG_PS2PORT)
        തിരികെ -1;
    തിരികെ ps2_command(0, command, param);
}

പൂർണ്ണസംഖ്യ
ps2_mouse_command(പൂർണ്ണസംഖ്യ command, u8 *param)
{
    എങ്കിൽ (! CONFIG_PS2PORT)
        തിരികെ -1;

    // Update ps2ctr for mouse enable/disable.
    എങ്കിൽ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_താഴെയുള്ള(Ps2ctr);
        എങ്കിൽ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        അല്ലെങ്കിൽ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_താഴെയുള്ള(Ps2ctr, ps2ctr);
    }

    തിരികെ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
ശൂന്യം VISIBLE16
handle_74(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PS2PORT)
        തിരികെ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    എങ്കിൽ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        പോകുക done;
    }
    v = inb(PORT_PS2_DATA);

    എങ്കിൽ (!(GET_താഴെയുള്ള(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        പോകുക done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
ശൂന്യം VISIBLE16
handle_09(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PS2PORT)
        തിരികെ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    എങ്കിൽ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        പോകുക done;
    }
    v = inb(PORT_PS2_DATA);

    എങ്കിൽ (!(GET_താഴെയുള്ള(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        പോകുക done;

    process_താക്കോൽ(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ശൂന്യ);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
ശൂന്യം
ps2_check_event(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        തിരികെ;
    u8 ps2ctr = GET_താഴെയുള്ള(Ps2ctr);
    എങ്കിൽ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        തിരികെ;
    വേണ്ടി (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        എങ്കിൽ (!(status & I8042_STR_OBF))
            നിർത്തുക;
        u8 data = inb(PORT_PS2_DATA);
        എങ്കിൽ (status & I8042_STR_AUXDATA) {
            എങ്കിൽ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } അല്ലെങ്കിൽ {
            എങ്കിൽ (!(ps2ctr & I8042_CTR_KBDDIS))
                process_താക്കോൽ(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

സ്ഥിതം ശൂന്യം
ps2_keyboard_setup(ശൂന്യം *data)
{
    // flush incoming keys (also verifies port is likely present)
    പൂർണ്ണസംഖ്യ ret = i8042_flush();
    എങ്കിൽ (ret)
        തിരികെ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ശൂന്യ);
    എങ്കിൽ (ret)
        തിരികെ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ശൂന്യ);
    എങ്കിൽ (ret)
        തിരികെ;
    ret = i8042_flush();
    എങ്കിൽ (ret)
        തിരികെ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    എങ്കിൽ (ret)
        തിരികെ;
    എങ്കിൽ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        തിരികെ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    എങ്കിൽ (ret)
        തിരികെ;
    എങ്കിൽ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        തിരികെ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    പൂർണ്ണസംഖ്യ spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    വേണ്ടി (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        എങ്കിൽ (!ret)
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            എങ്കിൽ (spinupdelay)
                warn_timeout();
            തിരികെ;
        }
        yield();
    }
    എങ്കിൽ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        തിരികെ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ശൂന്യ);
    എങ്കിൽ (ret)
        തിരികെ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    എങ്കിൽ (ret)
        തിരികെ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ശൂന്യ);
    എങ്കിൽ (ret)
        തിരികെ;

    dprintf(1, "PS2 keyboard initialized\n");
}

ശൂന്യം
ps2port_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_PS2PORT)
        തിരികെ;
    എങ്കിൽ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        തിരികെ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, ശൂന്യ);
}
