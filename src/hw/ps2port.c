// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_LOW
#समावेश "output.h" // dprintf
#समावेश "pic.h" // pic_eoi1
#समावेश "ps2port.h" // ps2_kbd_command
#समावेश "romfile.h" // romfile_loadint
#समावेश "stacks.h" // yield
#समावेश "util.h" // udelay
#समावेश "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#घोषणा I8042_CTL_TIMEOUT       10000

#घोषणा I8042_बफर_आकार       16

अटल पूर्णांक
i8042_wait_read(व्योम)
{
    dprintf(7, "i8042_wait_read\n");
    पूर्णांक i;
    क्रम (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        अगर (status & I8042_STR_OBF)
            वापस 0;
        udelay(50);
    }
    warn_timeout();
    वापस -1;
}

अटल पूर्णांक
i8042_wait_write(व्योम)
{
    dprintf(7, "i8042_wait_write\n");
    पूर्णांक i;
    क्रम (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        अगर (! (status & I8042_STR_IBF))
            वापस 0;
        udelay(50);
    }
    warn_timeout();
    वापस -1;
}

अटल पूर्णांक
i8042_flush(व्योम)
{
    dprintf(7, "i8042_flush\n");
    पूर्णांक i;
    क्रम (i=0; i<I8042_बफर_आकार; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        अगर (! (status & I8042_STR_OBF))
            वापस 0;
        udelay(50);
        u8 तथ्य = inb(PORT_PS2_तथ्य);
        dprintf(7, "i8042 flushed %x (status=%x)\n", तथ्य, status);
    }

    warn_timeout();
    वापस -1;
}

अटल पूर्णांक
__i8042_command(पूर्णांक command, u8 *param)
{
    पूर्णांक receive = (command >> 8) & 0xf;
    पूर्णांक send = (command >> 12) & 0xf;

    // Send the command.
    पूर्णांक ret = i8042_wait_write();
    अगर (ret)
        वापस ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    पूर्णांक i;
    क्रम (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        अगर (ret)
            वापस ret;
        outb(param[i], PORT_PS2_तथ्य);
    }

    // Receive parameters (if any).
    क्रम (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        अगर (ret)
            वापस ret;
        param[i] = inb(PORT_PS2_तथ्य);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    वापस 0;
}

अटल पूर्णांक
i8042_command(पूर्णांक command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    पूर्णांक ret = __i8042_command(command, param);
    अगर (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    वापस ret;
}

अटल पूर्णांक
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    पूर्णांक ret = i8042_wait_write();
    अगर (! ret)
        outb(c, PORT_PS2_तथ्य);
    वापस ret;
}

अटल पूर्णांक
i8042_aux_write(u8 c)
{
    वापस i8042_command(I8042_CMD_AUX_SEND, &c);
}

व्योम
i8042_reboot(व्योम)
{
    अगर (! CONFIG_PS2PORT)
       वापस;
    पूर्णांक i;
    क्रम (i=0; i<10; i++) {
        i8042_wait_write();
        udelay(50);
        outb(0xfe, PORT_PS2_STATUS); /* pulse reset low */
        udelay(50);
    }
}


/****************************************************************
 * Device commands.
 ****************************************************************/

#घोषणा PS2_RET_ACK             0xfa
#घोषणा PS2_RET_NAK             0xfe

अटल पूर्णांक
ps2_recvbyte(पूर्णांक aux, पूर्णांक needack, पूर्णांक timeout)
{
    u32 end = timer_calc(timeout);
    क्रम (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        अगर (status & I8042_STR_OBF) {
            u8 तथ्य = inb(PORT_PS2_तथ्य);
            dprintf(7, "ps2 read %x\n", तथ्य);

            अगर (!!(status & I8042_STR_AUXDATA) == aux) {
                अगर (!needack)
                    वापस तथ्य;
                अगर (तथ्य == PS2_RET_ACK)
                    वापस तथ्य;
                अगर (तथ्य == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    वापस तथ्य;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", तथ्य, status);
        }

        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
        yield();
    }
}

अटल पूर्णांक
ps2_sendbyte(पूर्णांक aux, u8 command, पूर्णांक timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    पूर्णांक ret;
    अगर (aux)
        ret = i8042_aux_write(command);
    अन्यथा
        ret = i8042_kbd_write(command);
    अगर (ret)
        वापस ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    अगर (ret < 0)
        वापस ret;
    अगर (ret != PS2_RET_ACK)
        वापस -1;

    वापस 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

अटल पूर्णांक
__ps2_command(पूर्णांक aux, पूर्णांक command, u8 *param)
{
    पूर्णांक ret2;
    पूर्णांक receive = (command >> 8) & 0xf;
    पूर्णांक send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_निचला(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    पूर्णांक ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    अगर (ret)
        वापस ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_निचला(Ps2ctr, newctr);
    अगर (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    अन्यथा
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    अगर (ret)
        जाओ fail;

    अगर ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        अगर (ret)
            जाओ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        अगर (ret < 0)
            जाओ fail;
        param[0] = ret;
        अगर (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            अगर (ret < 0)
                जाओ fail;
            param[1] = ret;
        }
    } अन्यथा अगर (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        अगर (ret)
            जाओ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        अगर (ret < 0)
            जाओ fail;
        param[0] = ret;
        अगर (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            अगर (ret < 0)
                जाओ fail;
            param[1] = ret;
        } अन्यथा {
            param[1] = 0;
        }
    } अन्यथा {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        अगर (ret)
            जाओ fail;

        // Send parameters (if any).
        पूर्णांक i;
        क्रम (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            अगर (ret)
                जाओ fail;
        }

        // Receive parameters (if any).
        क्रम (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            अगर (ret < 0)
                जाओ fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_निचला(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    अगर (ret2)
        वापस ret2;

    वापस ret;
}

अटल पूर्णांक
ps2_command(पूर्णांक aux, पूर्णांक command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    पूर्णांक ret = __ps2_command(aux, command, param);
    अगर (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    वापस ret;
}

पूर्णांक
ps2_kbd_command(पूर्णांक command, u8 *param)
{
    अगर (! CONFIG_PS2PORT)
        वापस -1;
    वापस ps2_command(0, command, param);
}

पूर्णांक
ps2_mouse_command(पूर्णांक command, u8 *param)
{
    अगर (! CONFIG_PS2PORT)
        वापस -1;

    // Update ps2ctr for mouse enable/disable.
    अगर (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_निचला(Ps2ctr);
        अगर (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        अन्यथा
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_निचला(Ps2ctr, ps2ctr);
    }

    वापस ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
व्योम VISIBLE16
handle_74(व्योम)
{
    अगर (! CONFIG_PS2PORT)
        वापस;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    अगर ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        जाओ done;
    }
    v = inb(PORT_PS2_तथ्य);

    अगर (!(GET_निचला(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        जाओ done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
व्योम VISIBLE16
handle_09(व्योम)
{
    अगर (! CONFIG_PS2PORT)
        वापस;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    अगर (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        जाओ done;
    }
    v = inb(PORT_PS2_तथ्य);

    अगर (!(GET_निचला(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        जाओ done;

    process_कुंजी(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, शून्य);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
व्योम
ps2_check_event(व्योम)
{
    अगर (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        वापस;
    u8 ps2ctr = GET_निचला(Ps2ctr);
    अगर ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        वापस;
    क्रम (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        अगर (!(status & I8042_STR_OBF))
            अवरोध;
        u8 तथ्य = inb(PORT_PS2_तथ्य);
        अगर (status & I8042_STR_AUXDATA) {
            अगर (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(तथ्य);
        } अन्यथा {
            अगर (!(ps2ctr & I8042_CTR_KBDDIS))
                process_कुंजी(तथ्य);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

अटल व्योम
ps2_keyboard_setup(व्योम *तथ्य)
{
    // flush incoming keys (also verifies port is likely present)
    पूर्णांक ret = i8042_flush();
    अगर (ret)
        वापस;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, शून्य);
    अगर (ret)
        वापस;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, शून्य);
    अगर (ret)
        वापस;
    ret = i8042_flush();
    अगर (ret)
        वापस;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    अगर (ret)
        वापस;
    अगर (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        वापस;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    अगर (ret)
        वापस;
    अगर (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        वापस;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    पूर्णांक spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    क्रम (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        अगर (!ret)
            अवरोध;
        अगर (timer_check(end)) {
            अगर (spinupdelay)
                warn_timeout();
            वापस;
        }
        yield();
    }
    अगर (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        वापस;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, शून्य);
    अगर (ret)
        वापस;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    अगर (ret)
        वापस;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, शून्य);
    अगर (ret)
        वापस;

    dprintf(1, "PS2 keyboard initialized\n");
}

व्योम
ps2port_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (! CONFIG_PS2PORT)
        वापस;
    अगर (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        वापस;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, शून्य);
}
