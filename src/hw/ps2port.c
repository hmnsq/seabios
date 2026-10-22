// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_LOW
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pic.h" // pic_eoi1
#ਅਨ੍ਤਰ੍ਭਾਵ "ps2port.h" // ps2_kbd_command
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // udelay
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_CTL_TIMEOUT       10000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I8042_ਬਫਰ_ਮਾਪ       16

ਸਥਿਰ ਪੂਰਨ
i8042_wait_read(ਖਾਲੀ)
{
    dprintf(7, "i8042_wait_read\n");
    ਪੂਰਨ i;
    ਲਈ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 ਸਥਿਤੀ = inb(ਪੋਰਟ_PS2_ਸਥਿਤੀ);
        ਜੇ (ਸਥਿਤੀ & I8042_STR_OBF)
            ਵਾਪਸ 0;
        udelay(50);
    }
    warn_timeout();
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
i8042_wait_write(ਖਾਲੀ)
{
    dprintf(7, "i8042_wait_write\n");
    ਪੂਰਨ i;
    ਲਈ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 ਸਥਿਤੀ = inb(ਪੋਰਟ_PS2_ਸਥਿਤੀ);
        ਜੇ (! (ਸਥਿਤੀ & I8042_STR_IBF))
            ਵਾਪਸ 0;
        udelay(50);
    }
    warn_timeout();
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
i8042_flush(ਖਾਲੀ)
{
    dprintf(7, "i8042_flush\n");
    ਪੂਰਨ i;
    ਲਈ (i=0; i<I8042_ਬਫਰ_ਮਾਪ; i++) {
        u8 ਸਥਿਤੀ = inb(ਪੋਰਟ_PS2_ਸਥਿਤੀ);
        ਜੇ (! (ਸਥਿਤੀ & I8042_STR_OBF))
            ਵਾਪਸ 0;
        udelay(50);
        u8 ਅੰਕੜੇ = inb(ਪੋਰਟ_PS2_ਅੰਕੜੇ);
        dprintf(7, "i8042 flushed %x (status=%x)\n", ਅੰਕੜੇ, ਸਥਿਤੀ);
    }

    warn_timeout();
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
__i8042_command(ਪੂਰਨ command, u8 *param)
{
    ਪੂਰਨ receive = (command >> 8) & 0xf;
    ਪੂਰਨ send = (command >> 12) & 0xf;

    // Send the command.
    ਪੂਰਨ ret = i8042_wait_write();
    ਜੇ (ret)
        ਵਾਪਸ ret;
    outb(command, ਪੋਰਟ_PS2_ਸਥਿਤੀ);

    // Send parameters (if any).
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ਜੇ (ret)
            ਵਾਪਸ ret;
        outb(param[i], ਪੋਰਟ_PS2_ਅੰਕੜੇ);
    }

    // Receive parameters (if any).
    ਲਈ (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ਜੇ (ret)
            ਵਾਪਸ ret;
        param[i] = inb(ਪੋਰਟ_PS2_ਅੰਕੜੇ);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
i8042_command(ਪੂਰਨ command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ਪੂਰਨ ret = __i8042_command(command, param);
    ਜੇ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ਪੂਰਨ ret = i8042_wait_write();
    ਜੇ (! ret)
        outb(c, ਪੋਰਟ_PS2_ਅੰਕੜੇ);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
i8042_aux_write(u8 c)
{
    ਵਾਪਸ i8042_command(I8042_CMD_AUX_SEND, &c);
}

ਖਾਲੀ
i8042_reboot(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PS2_ਪੋਰਟ)
       ਵਾਪਸ;
    ਪੂਰਨ i;
    ਲਈ (i=0; i<10; i++) {
        i8042_wait_write();
        udelay(50);
        outb(0xfe, ਪੋਰਟ_PS2_ਸਥਿਤੀ); /* pulse reset low */
        udelay(50);
    }
}


/****************************************************************
 * Device commands.
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PS2_RET_ACK             0xfa
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PS2_RET_NAK             0xfe

ਸਥਿਰ ਪੂਰਨ
ps2_recvbyte(ਪੂਰਨ aux, ਪੂਰਨ needack, ਪੂਰਨ timeout)
{
    u32 ਸਿਰਾ = timer_calc(timeout);
    ਲਈ (;;) {
        u8 ਸਥਿਤੀ = inb(ਪੋਰਟ_PS2_ਸਥਿਤੀ);
        ਜੇ (ਸਥਿਤੀ & I8042_STR_OBF) {
            u8 ਅੰਕੜੇ = inb(ਪੋਰਟ_PS2_ਅੰਕੜੇ);
            dprintf(7, "ps2 read %x\n", ਅੰਕੜੇ);

            ਜੇ (!!(ਸਥਿਤੀ & I8042_STR_AUXDATA) == aux) {
                ਜੇ (!needack)
                    ਵਾਪਸ ਅੰਕੜੇ;
                ਜੇ (ਅੰਕੜੇ == PS2_RET_ACK)
                    ਵਾਪਸ ਅੰਕੜੇ;
                ਜੇ (ਅੰਕੜੇ == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", ਸਥਿਤੀ);
                    ਵਾਪਸ ਅੰਕੜੇ;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", ਅੰਕੜੇ, ਸਥਿਤੀ);
        }

        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }
}

ਸਥਿਰ ਪੂਰਨ
ps2_sendbyte(ਪੂਰਨ aux, u8 command, ਪੂਰਨ timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ਪੂਰਨ ret;
    ਜੇ (aux)
        ret = i8042_aux_write(command);
    ਨਹੀਂਤਾਂ
        ret = i8042_kbd_write(command);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ਜੇ (ret < 0)
        ਵਾਪਸ ret;
    ਜੇ (ret != PS2_RET_ACK)
        ਵਾਪਸ -1;

    ਵਾਪਸ 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

ਸਥਿਰ ਪੂਰਨ
__ps2_command(ਪੂਰਨ aux, ਪੂਰਨ command, u8 *param)
{
    ਪੂਰਨ ret2;
    ਪੂਰਨ receive = (command >> 8) & 0xf;
    ਪੂਰਨ send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_ਨੀਵਾਂ(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ਪੂਰਨ ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    ਸੈੱਟ_ਨੀਵਾਂ(Ps2ctr, newctr);
    ਜੇ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ਨਹੀਂਤਾਂ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ਜੇ (ret)
        ਜਾਓ fail;

    ਜੇ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ਜੇ (ret)
            ਜਾਓ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ਜੇ (ret < 0)
            ਜਾਓ fail;
        param[0] = ret;
        ਜੇ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ਜੇ (ret < 0)
                ਜਾਓ fail;
            param[1] = ret;
        }
    } ਨਹੀਂਤਾਂ ਜੇ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ਜੇ (ret)
            ਜਾਓ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ਜੇ (ret < 0)
            ਜਾਓ fail;
        param[0] = ret;
        ਜੇ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ਜੇ (ret < 0)
                ਜਾਓ fail;
            param[1] = ret;
        } ਨਹੀਂਤਾਂ {
            param[1] = 0;
        }
    } ਨਹੀਂਤਾਂ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ਜੇ (ret)
            ਜਾਓ fail;

        // Send parameters (if any).
        ਪੂਰਨ i;
        ਲਈ (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ਜੇ (ret)
                ਜਾਓ fail;
        }

        // Receive parameters (if any).
        ਲਈ (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ਜੇ (ret < 0)
                ਜਾਓ fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    ਸੈੱਟ_ਨੀਵਾਂ(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ਜੇ (ret2)
        ਵਾਪਸ ret2;

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
ps2_command(ਪੂਰਨ aux, ਪੂਰਨ command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ਪੂਰਨ ret = __ps2_command(aux, command, param);
    ਜੇ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ਵਾਪਸ ret;
}

ਪੂਰਨ
ps2_kbd_command(ਪੂਰਨ command, u8 *param)
{
    ਜੇ (! CONFIG_PS2_ਪੋਰਟ)
        ਵਾਪਸ -1;
    ਵਾਪਸ ps2_command(0, command, param);
}

ਪੂਰਨ
ps2_mouse_command(ਪੂਰਨ command, u8 *param)
{
    ਜੇ (! CONFIG_PS2_ਪੋਰਟ)
        ਵਾਪਸ -1;

    // Update ps2ctr for mouse enable/disable.
    ਜੇ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_ਨੀਵਾਂ(Ps2ctr);
        ਜੇ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ਨਹੀਂਤਾਂ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        ਸੈੱਟ_ਨੀਵਾਂ(Ps2ctr, ps2ctr);
    }

    ਵਾਪਸ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
ਖਾਲੀ VISIBLE16
handle_74(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PS2_ਪੋਰਟ)
        ਵਾਪਸ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(ਪੋਰਟ_PS2_ਸਥਿਤੀ);
    ਜੇ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        ਜਾਓ ਪੂਰਾ;
    }
    v = inb(ਪੋਰਟ_PS2_ਅੰਕੜੇ);

    ਜੇ (!(GET_ਨੀਵਾਂ(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        ਜਾਓ ਪੂਰਾ;

    process_mouse(v);

ਪੂਰਾ:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
ਖਾਲੀ VISIBLE16
handle_09(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PS2_ਪੋਰਟ)
        ਵਾਪਸ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(ਪੋਰਟ_PS2_ਸਥਿਤੀ);
    ਜੇ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        ਜਾਓ ਪੂਰਾ;
    }
    v = inb(ਪੋਰਟ_PS2_ਅੰਕੜੇ);

    ਜੇ (!(GET_ਨੀਵਾਂ(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        ਜਾਓ ਪੂਰਾ;

    process_ਕੁੰਜੀ(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ਨਿਰਾ);

ਪੂਰਾ:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
ਖਾਲੀ
ps2_check_event(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PS2_ਪੋਰਟ || CONFIG_HARDWARE_IRQ)
        ਵਾਪਸ;
    u8 ps2ctr = GET_ਨੀਵਾਂ(Ps2ctr);
    ਜੇ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ਵਾਪਸ;
    ਲਈ (;;) {
        u8 ਸਥਿਤੀ = inb(ਪੋਰਟ_PS2_ਸਥਿਤੀ);
        ਜੇ (!(ਸਥਿਤੀ & I8042_STR_OBF))
            ਰੋਕੋ;
        u8 ਅੰਕੜੇ = inb(ਪੋਰਟ_PS2_ਅੰਕੜੇ);
        ਜੇ (ਸਥਿਤੀ & I8042_STR_AUXDATA) {
            ਜੇ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(ਅੰਕੜੇ);
        } ਨਹੀਂਤਾਂ {
            ਜੇ (!(ps2ctr & I8042_CTR_KBDDIS))
                process_ਕੁੰਜੀ(ਅੰਕੜੇ);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ਸਥਿਰ ਖਾਲੀ
ps2_keyboard_ਤਿਆਰੀ(ਖਾਲੀ *ਅੰਕੜੇ)
{
    // flush incoming keys (also verifies port is likely present)
    ਪੂਰਨ ret = i8042_flush();
    ਜੇ (ret)
        ਵਾਪਸ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ਨਿਰਾ);
    ਜੇ (ret)
        ਵਾਪਸ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ਨਿਰਾ);
    ਜੇ (ret)
        ਵਾਪਸ;
    ret = i8042_flush();
    ਜੇ (ret)
        ਵਾਪਸ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ਜੇ (ret)
        ਵਾਪਸ;
    ਜੇ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ਵਾਪਸ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ਜੇ (ret)
        ਵਾਪਸ;
    ਜੇ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ਵਾਪਸ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ਪੂਰਨ spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 ਸਿਰਾ = timer_calc(spinupdelay);
    ਲਈ (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ਜੇ (!ret)
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            ਜੇ (spinupdelay)
                warn_timeout();
            ਵਾਪਸ;
        }
        yield();
    }
    ਜੇ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ਵਾਪਸ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ਨਿਰਾ);
    ਜੇ (ret)
        ਵਾਪਸ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ਜੇ (ret)
        ਵਾਪਸ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ਨਿਰਾ);
    ਜੇ (ret)
        ਵਾਪਸ;

    dprintf(1, "PS2 keyboard initialized\n");
}

ਖਾਲੀ
ps2port_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_PS2_ਪੋਰਟ)
        ਵਾਪਸ;
    ਜੇ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ਵਾਪਸ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(ਇੰਦਰਾਜ_09));
    enable_hwirq(12, FUNC16(ਇੰਦਰਾਜ_74));

    ਲੜੀਵਾਰ_thread(ps2_keyboard_ਤਿਆਰੀ, ਨਿਰਾ);
}
