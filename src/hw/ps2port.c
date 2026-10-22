// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_LOW
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pic.h" // pic_eoi1
#ଅନ୍ତର୍ଭାଵ "ps2port.h" // ps2_kbd_command
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "util.h" // udelay
#ଅନ୍ତର୍ଭାଵ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
i8042_wait_read(ଶୂନ୍ୟ)
{
    dprintf(7, "i8042_wait_read\n");
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ଯଦି (status & I8042_STR_OBF)
            ଫେରନ୍ତୁ 0;
        udelay(50);
    }
    warn_timeout();
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
i8042_wait_write(ଶୂନ୍ୟ)
{
    dprintf(7, "i8042_wait_write\n");
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ଯଦି (! (status & I8042_STR_IBF))
            ଫେରନ୍ତୁ 0;
        udelay(50);
    }
    warn_timeout();
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
i8042_flush(ଶୂନ୍ୟ)
{
    dprintf(7, "i8042_flush\n");
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ଯଦି (! (status & I8042_STR_OBF))
            ଫେରନ୍ତୁ 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
__i8042_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା receive = (command >> 8) & 0xf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା send = (command >> 12) & 0xf;

    // Send the command.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = i8042_wait_write();
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    ପାଇଁ (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
i8042_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = __i8042_command(command, param);
    ଯଦି (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = i8042_wait_write();
    ଯଦି (! ret)
        outb(c, PORT_PS2_DATA);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
i8042_aux_write(u8 c)
{
    ଫେରନ୍ତୁ i8042_command(I8042_CMD_AUX_SEND, &c);
}

ଶୂନ୍ୟ
i8042_reboot(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PS2PORT)
       ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<10; i++) {
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ps2_recvbyte(ପୂର୍ଣ୍ଣସଂଖ୍ୟା aux, ପୂର୍ଣ୍ଣସଂଖ୍ୟା needack, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
    u32 end = timer_calc(timeout);
    ପାଇଁ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ଯଦି (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            ଯଦି (!!(status & I8042_STR_AUXDATA) == aux) {
                ଯଦି (!needack)
                    ଫେରନ୍ତୁ data;
                ଯଦି (data == PS2_RET_ACK)
                    ଫେରନ୍ତୁ data;
                ଯଦି (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ଫେରନ୍ତୁ data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ps2_sendbyte(ପୂର୍ଣ୍ଣସଂଖ୍ୟା aux, u8 command, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    ଯଦି (aux)
        ret = i8042_aux_write(command);
    ଅନ୍ୟଥା
        ret = i8042_kbd_write(command);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ଯଦି (ret < 0)
        ଫେରନ୍ତୁ ret;
    ଯଦି (ret != PS2_RET_ACK)
        ଫେରନ୍ତୁ -1;

    ଫେରନ୍ତୁ 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
__ps2_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା aux, ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret2;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା receive = (command >> 8) & 0xf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_ତଳ(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_ତଳ(Ps2ctr, newctr);
    ଯଦି (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ଅନ୍ୟଥା
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ଯଦି (ret)
        ଯାଅ fail;

    ଯଦି ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ଯଦି (ret)
            ଯାଅ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ଯଦି (ret < 0)
            ଯାଅ fail;
        param[0] = ret;
        ଯଦି (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ଯଦି (ret < 0)
                ଯାଅ fail;
            param[1] = ret;
        }
    } ଅନ୍ୟଥା ଯଦି (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ଯଦି (ret)
            ଯାଅ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ଯଦି (ret < 0)
            ଯାଅ fail;
        param[0] = ret;
        ଯଦି (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ଯଦି (ret < 0)
                ଯାଅ fail;
            param[1] = ret;
        } ଅନ୍ୟଥା {
            param[1] = 0;
        }
    } ଅନ୍ୟଥା {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ଯଦି (ret)
            ଯାଅ fail;

        // Send parameters (if any).
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ଯଦି (ret)
                ଯାଅ fail;
        }

        // Receive parameters (if any).
        ପାଇଁ (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ଯଦି (ret < 0)
                ଯାଅ fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_ତଳ(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ଯଦି (ret2)
        ଫେରନ୍ତୁ ret2;

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ps2_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା aux, ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = __ps2_command(aux, command, param);
    ଯଦି (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ps2_kbd_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    ଯଦି (! CONFIG_PS2PORT)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ ps2_command(0, command, param);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ps2_mouse_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    ଯଦି (! CONFIG_PS2PORT)
        ଫେରନ୍ତୁ -1;

    // Update ps2ctr for mouse enable/disable.
    ଯଦି (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_ତଳ(Ps2ctr);
        ଯଦି (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ଅନ୍ୟଥା
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_ତଳ(Ps2ctr, ps2ctr);
    }

    ଫେରନ୍ତୁ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
ଶୂନ୍ୟ VISIBLE16
handle_74(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PS2PORT)
        ଫେରନ୍ତୁ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    ଯଦି ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        ଯାଅ done;
    }
    v = inb(PORT_PS2_DATA);

    ଯଦି (!(GET_ତଳ(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        ଯାଅ done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
ଶୂନ୍ୟ VISIBLE16
handle_09(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PS2PORT)
        ଫେରନ୍ତୁ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    ଯଦି (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        ଯାଅ done;
    }
    v = inb(PORT_PS2_DATA);

    ଯଦି (!(GET_ତଳ(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        ଯାଅ done;

    process_ଚାବି(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ଖାଲି);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
ଶୂନ୍ୟ
ps2_check_event(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ଫେରନ୍ତୁ;
    u8 ps2ctr = GET_ତଳ(Ps2ctr);
    ଯଦି ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ଫେରନ୍ତୁ;
    ପାଇଁ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ଯଦି (!(status & I8042_STR_OBF))
            ଭାଙ୍ଗ;
        u8 data = inb(PORT_PS2_DATA);
        ଯଦି (status & I8042_STR_AUXDATA) {
            ଯଦି (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } ଅନ୍ୟଥା {
            ଯଦି (!(ps2ctr & I8042_CTR_KBDDIS))
                process_ଚାବି(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ସ୍ଥିର ଶୂନ୍ୟ
ps2_keyboard_setup(ଶୂନ୍ୟ *data)
{
    // flush incoming keys (also verifies port is likely present)
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = i8042_flush();
    ଯଦି (ret)
        ଫେରନ୍ତୁ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ଖାଲି);
    ଯଦି (ret)
        ଫେରନ୍ତୁ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ଖାଲି);
    ଯଦି (ret)
        ଫେରନ୍ତୁ;
    ret = i8042_flush();
    ଯଦି (ret)
        ଫେରନ୍ତୁ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ଯଦି (ret)
        ଫେରନ୍ତୁ;
    ଯଦି (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ଫେରନ୍ତୁ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ଯଦି (ret)
        ଫେରନ୍ତୁ;
    ଯଦି (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ଫେରନ୍ତୁ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    ପାଇଁ (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ଯଦି (!ret)
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            ଯଦି (spinupdelay)
                warn_timeout();
            ଫେରନ୍ତୁ;
        }
        yield();
    }
    ଯଦି (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ଫେରନ୍ତୁ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ଖାଲି);
    ଯଦି (ret)
        ଫେରନ୍ତୁ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ଯଦି (ret)
        ଫେରନ୍ତୁ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ଖାଲି);
    ଯଦି (ret)
        ଫେରନ୍ତୁ;

    dprintf(1, "PS2 keyboard initialized\n");
}

ଶୂନ୍ୟ
ps2port_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_PS2PORT)
        ଫେରନ୍ତୁ;
    ଯଦି (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ଫେରନ୍ତୁ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, ଖାଲି);
}
