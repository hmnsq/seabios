// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_LOW
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pic.h" // pic_eoi1
#అన్తర్భావ "ps2port.h" // ps2_kbd_command
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "util.h" // udelay
#అన్తర్భావ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#నిర్వచించు I8042_CTL_TIMEOUT       10000

#నిర్వచించు I8042_బఫరు_కొలత       16

స్థిర పూర్ణాంకం
i8042_wait_read(శూన్య)
{
    dprintf(7, "i8042_wait_read\n");
    పూర్ణాంకం i;
    కొరకు (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 స్థితి_2 = inb(పోర్టు_PS2_స్థితి);
        ఒకవేళ (స్థితి_2 & I8042_STR_OBF)
            తిరిగి 0;
        udelay(50);
    }
    warn_timeout();
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
i8042_wait_write(శూన్య)
{
    dprintf(7, "i8042_wait_write\n");
    పూర్ణాంకం i;
    కొరకు (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 స్థితి_2 = inb(పోర్టు_PS2_స్థితి);
        ఒకవేళ (! (స్థితి_2 & I8042_STR_IBF))
            తిరిగి 0;
        udelay(50);
    }
    warn_timeout();
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
i8042_flush(శూన్య)
{
    dprintf(7, "i8042_flush\n");
    పూర్ణాంకం i;
    కొరకు (i=0; i<I8042_బఫరు_కొలత; i++) {
        u8 స్థితి_2 = inb(పోర్టు_PS2_స్థితి);
        ఒకవేళ (! (స్థితి_2 & I8042_STR_OBF))
            తిరిగి 0;
        udelay(50);
        u8 దత్తాంశం = inb(పోర్టు_PS2_దత్తాంశం);
        dprintf(7, "i8042 flushed %x (status=%x)\n", దత్తాంశం, స్థితి_2);
    }

    warn_timeout();
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
__i8042_command(పూర్ణాంకం command, u8 *param)
{
    పూర్ణాంకం receive = (command >> 8) & 0xf;
    పూర్ణాంకం send = (command >> 12) & 0xf;

    // Send the command.
    పూర్ణాంకం ret = i8042_wait_write();
    ఒకవేళ (ret)
        తిరిగి ret;
    outb(command, పోర్టు_PS2_స్థితి);

    // Send parameters (if any).
    పూర్ణాంకం i;
    కొరకు (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ఒకవేళ (ret)
            తిరిగి ret;
        outb(param[i], పోర్టు_PS2_దత్తాంశం);
    }

    // Receive parameters (if any).
    కొరకు (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ఒకవేళ (ret)
            తిరిగి ret;
        param[i] = inb(పోర్టు_PS2_దత్తాంశం);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
i8042_command(పూర్ణాంకం command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    పూర్ణాంకం ret = __i8042_command(command, param);
    ఒకవేళ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    పూర్ణాంకం ret = i8042_wait_write();
    ఒకవేళ (! ret)
        outb(c, పోర్టు_PS2_దత్తాంశం);
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
i8042_aux_write(u8 c)
{
    తిరిగి i8042_command(I8042_CMD_AUX_SEND, &c);
}

శూన్య
i8042_reboot(శూన్య)
{
    ఒకవేళ (! CONFIG_PS2_పోర్టు)
       తిరిగి;
    పూర్ణాంకం i;
    కొరకు (i=0; i<10; i++) {
        i8042_wait_write();
        udelay(50);
        outb(0xfe, పోర్టు_PS2_స్థితి); /* pulse reset low */
        udelay(50);
    }
}


/****************************************************************
 * Device commands.
 ****************************************************************/

#నిర్వచించు PS2_RET_ACK             0xfa
#నిర్వచించు PS2_RET_NAK             0xfe

స్థిర పూర్ణాంకం
ps2_recvbyte(పూర్ణాంకం aux, పూర్ణాంకం needack, పూర్ణాంకం timeout)
{
    u32 చివర = timer_calc(timeout);
    కొరకు (;;) {
        u8 స్థితి_2 = inb(పోర్టు_PS2_స్థితి);
        ఒకవేళ (స్థితి_2 & I8042_STR_OBF) {
            u8 దత్తాంశం = inb(పోర్టు_PS2_దత్తాంశం);
            dprintf(7, "ps2 read %x\n", దత్తాంశం);

            ఒకవేళ (!!(స్థితి_2 & I8042_STR_AUXDATA) == aux) {
                ఒకవేళ (!needack)
                    తిరిగి దత్తాంశం;
                ఒకవేళ (దత్తాంశం == PS2_RET_ACK)
                    తిరిగి దత్తాంశం;
                ఒకవేళ (దత్తాంశం == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", స్థితి_2);
                    తిరిగి దత్తాంశం;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", దత్తాంశం, స్థితి_2);
        }

        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }
}

స్థిర పూర్ణాంకం
ps2_sendbyte(పూర్ణాంకం aux, u8 command, పూర్ణాంకం timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    పూర్ణాంకం ret;
    ఒకవేళ (aux)
        ret = i8042_aux_write(command);
    లేకపోతే
        ret = i8042_kbd_write(command);
    ఒకవేళ (ret)
        తిరిగి ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ఒకవేళ (ret < 0)
        తిరిగి ret;
    ఒకవేళ (ret != PS2_RET_ACK)
        తిరిగి -1;

    తిరిగి 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

స్థిర పూర్ణాంకం
__ps2_command(పూర్ణాంకం aux, పూర్ణాంకం command, u8 *param)
{
    పూర్ణాంకం ret2;
    పూర్ణాంకం receive = (command >> 8) & 0xf;
    పూర్ణాంకం send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_దిగువ(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    పూర్ణాంకం ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ఒకవేళ (ret)
        తిరిగి ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    సమితి_దిగువ(Ps2ctr, newctr);
    ఒకవేళ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    లేకపోతే
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ఒకవేళ (ret)
        వెళ్ళు fail;

    ఒకవేళ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ఒకవేళ (ret)
            వెళ్ళు fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ఒకవేళ (ret < 0)
            వెళ్ళు fail;
        param[0] = ret;
        ఒకవేళ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ఒకవేళ (ret < 0)
                వెళ్ళు fail;
            param[1] = ret;
        }
    } లేకపోతే ఒకవేళ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ఒకవేళ (ret)
            వెళ్ళు fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ఒకవేళ (ret < 0)
            వెళ్ళు fail;
        param[0] = ret;
        ఒకవేళ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ఒకవేళ (ret < 0)
                వెళ్ళు fail;
            param[1] = ret;
        } లేకపోతే {
            param[1] = 0;
        }
    } లేకపోతే {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ఒకవేళ (ret)
            వెళ్ళు fail;

        // Send parameters (if any).
        పూర్ణాంకం i;
        కొరకు (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ఒకవేళ (ret)
                వెళ్ళు fail;
        }

        // Receive parameters (if any).
        కొరకు (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ఒకవేళ (ret < 0)
                వెళ్ళు fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    సమితి_దిగువ(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ఒకవేళ (ret2)
        తిరిగి ret2;

    తిరిగి ret;
}

స్థిర పూర్ణాంకం
ps2_command(పూర్ణాంకం aux, పూర్ణాంకం command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    పూర్ణాంకం ret = __ps2_command(aux, command, param);
    ఒకవేళ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    తిరిగి ret;
}

పూర్ణాంకం
ps2_kbd_command(పూర్ణాంకం command, u8 *param)
{
    ఒకవేళ (! CONFIG_PS2_పోర్టు)
        తిరిగి -1;
    తిరిగి ps2_command(0, command, param);
}

పూర్ణాంకం
ps2_mouse_command(పూర్ణాంకం command, u8 *param)
{
    ఒకవేళ (! CONFIG_PS2_పోర్టు)
        తిరిగి -1;

    // Update ps2ctr for mouse enable/disable.
    ఒకవేళ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_దిగువ(Ps2ctr);
        ఒకవేళ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        లేకపోతే
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        సమితి_దిగువ(Ps2ctr, ps2ctr);
    }

    తిరిగి ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
శూన్య VISIBLE16
handle_74(శూన్య)
{
    ఒకవేళ (! CONFIG_PS2_పోర్టు)
        తిరిగి;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(పోర్టు_PS2_స్థితి);
    ఒకవేళ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        వెళ్ళు పూర్తి;
    }
    v = inb(పోర్టు_PS2_దత్తాంశం);

    ఒకవేళ (!(GET_దిగువ(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        వెళ్ళు పూర్తి;

    process_mouse(v);

పూర్తి:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
శూన్య VISIBLE16
handle_09(శూన్య)
{
    ఒకవేళ (! CONFIG_PS2_పోర్టు)
        తిరిగి;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(పోర్టు_PS2_స్థితి);
    ఒకవేళ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        వెళ్ళు పూర్తి;
    }
    v = inb(పోర్టు_PS2_దత్తాంశం);

    ఒకవేళ (!(GET_దిగువ(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        వెళ్ళు పూర్తి;

    process_కీలకం(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ఖాళీ);

పూర్తి:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
శూన్య
ps2_check_event(శూన్య)
{
    ఒకవేళ (! CONFIG_PS2_పోర్టు || CONFIG_HARDWARE_IRQ)
        తిరిగి;
    u8 ps2ctr = GET_దిగువ(Ps2ctr);
    ఒకవేళ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        తిరిగి;
    కొరకు (;;) {
        u8 స్థితి_2 = inb(పోర్టు_PS2_స్థితి);
        ఒకవేళ (!(స్థితి_2 & I8042_STR_OBF))
            ఆపు;
        u8 దత్తాంశం = inb(పోర్టు_PS2_దత్తాంశం);
        ఒకవేళ (స్థితి_2 & I8042_STR_AUXDATA) {
            ఒకవేళ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(దత్తాంశం);
        } లేకపోతే {
            ఒకవేళ (!(ps2ctr & I8042_CTR_KBDDIS))
                process_కీలకం(దత్తాంశం);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

స్థిర శూన్య
ps2_keyboard_అమరిక(శూన్య *దత్తాంశం)
{
    // flush incoming keys (also verifies port is likely present)
    పూర్ణాంకం ret = i8042_flush();
    ఒకవేళ (ret)
        తిరిగి;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ఖాళీ);
    ఒకవేళ (ret)
        తిరిగి;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ఖాళీ);
    ఒకవేళ (ret)
        తిరిగి;
    ret = i8042_flush();
    ఒకవేళ (ret)
        తిరిగి;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ఒకవేళ (ret)
        తిరిగి;
    ఒకవేళ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        తిరిగి;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ఒకవేళ (ret)
        తిరిగి;
    ఒకవేళ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        తిరిగి;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    పూర్ణాంకం spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 చివర = timer_calc(spinupdelay);
    కొరకు (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ఒకవేళ (!ret)
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            ఒకవేళ (spinupdelay)
                warn_timeout();
            తిరిగి;
        }
        yield();
    }
    ఒకవేళ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        తిరిగి;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ఖాళీ);
    ఒకవేళ (ret)
        తిరిగి;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ఒకవేళ (ret)
        తిరిగి;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ఖాళీ);
    ఒకవేళ (ret)
        తిరిగి;

    dprintf(1, "PS2 keyboard initialized\n");
}

శూన్య
ps2port_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_PS2_పోర్టు)
        తిరిగి;
    ఒకవేళ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        తిరిగి;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(నమోదు_09));
    enable_hwirq(12, FUNC16(నమోదు_74));

    నిరంతరం_thread(ps2_keyboard_అమరిక, ఖాళీ);
}
