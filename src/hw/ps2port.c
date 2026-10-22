// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_LOW
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pic.h" // pic_eoi1
#অন্তর্ভাব "ps2port.h" // ps2_kbd_command
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "util.h" // udelay
#অন্তর্ভাব "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_বাফার_আকার       16

স্থির পূর্ণসংখ্যা
i8042_wait_read(শূন্য)
{
    dprintf(7, "i8042_wait_read\n");
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        যদি (status & I8042_STR_OBF)
            ফেরত 0;
        udelay(50);
    }
    warn_timeout();
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
i8042_wait_write(শূন্য)
{
    dprintf(7, "i8042_wait_write\n");
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        যদি (! (status & I8042_STR_IBF))
            ফেরত 0;
        udelay(50);
    }
    warn_timeout();
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
i8042_flush(শূন্য)
{
    dprintf(7, "i8042_flush\n");
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<I8042_বাফার_আকার; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        যদি (! (status & I8042_STR_OBF))
            ফেরত 0;
        udelay(50);
        u8 তথ্য = inb(PORT_PS2_তথ্য);
        dprintf(7, "i8042 flushed %x (status=%x)\n", তথ্য, status);
    }

    warn_timeout();
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
__i8042_command(পূর্ণসংখ্যা command, u8 *param)
{
    পূর্ণসংখ্যা receive = (command >> 8) & 0xf;
    পূর্ণসংখ্যা send = (command >> 12) & 0xf;

    // Send the command.
    পূর্ণসংখ্যা ret = i8042_wait_write();
    যদি (ret)
        ফেরত ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        যদি (ret)
            ফেরত ret;
        outb(param[i], PORT_PS2_তথ্য);
    }

    // Receive parameters (if any).
    জন্য (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        যদি (ret)
            ফেরত ret;
        param[i] = inb(PORT_PS2_তথ্য);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
i8042_command(পূর্ণসংখ্যা command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    পূর্ণসংখ্যা ret = __i8042_command(command, param);
    যদি (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    পূর্ণসংখ্যা ret = i8042_wait_write();
    যদি (! ret)
        outb(c, PORT_PS2_তথ্য);
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
i8042_aux_write(u8 c)
{
    ফেরত i8042_command(I8042_CMD_AUX_SEND, &c);
}

শূন্য
i8042_reboot(শূন্য)
{
    যদি (! CONFIG_PS2PORT)
       ফেরত;
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<10; i++) {
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

স্থির পূর্ণসংখ্যা
ps2_recvbyte(পূর্ণসংখ্যা aux, পূর্ণসংখ্যা needack, পূর্ণসংখ্যা timeout)
{
    u32 end = timer_calc(timeout);
    জন্য (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        যদি (status & I8042_STR_OBF) {
            u8 তথ্য = inb(PORT_PS2_তথ্য);
            dprintf(7, "ps2 read %x\n", তথ্য);

            যদি (!!(status & I8042_STR_AUXDATA) == aux) {
                যদি (!needack)
                    ফেরত তথ্য;
                যদি (তথ্য == PS2_RET_ACK)
                    ফেরত তথ্য;
                যদি (তথ্য == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ফেরত তথ্য;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", তথ্য, status);
        }

        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত -1;
        }
        yield();
    }
}

স্থির পূর্ণসংখ্যা
ps2_sendbyte(পূর্ণসংখ্যা aux, u8 command, পূর্ণসংখ্যা timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    পূর্ণসংখ্যা ret;
    যদি (aux)
        ret = i8042_aux_write(command);
    অন্যথা
        ret = i8042_kbd_write(command);
    যদি (ret)
        ফেরত ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    যদি (ret < 0)
        ফেরত ret;
    যদি (ret != PS2_RET_ACK)
        ফেরত -1;

    ফেরত 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

স্থির পূর্ণসংখ্যা
__ps2_command(পূর্ণসংখ্যা aux, পূর্ণসংখ্যা command, u8 *param)
{
    পূর্ণসংখ্যা ret2;
    পূর্ণসংখ্যা receive = (command >> 8) & 0xf;
    পূর্ণসংখ্যা send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_নিচের(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    পূর্ণসংখ্যা ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    যদি (ret)
        ফেরত ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_নিচের(Ps2ctr, newctr);
    যদি (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    অন্যথা
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    যদি (ret)
        যাও fail;

    যদি ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        যদি (ret)
            যাও fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        যদি (ret < 0)
            যাও fail;
        param[0] = ret;
        যদি (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            যদি (ret < 0)
                যাও fail;
            param[1] = ret;
        }
    } অন্যথা যদি (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        যদি (ret)
            যাও fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        যদি (ret < 0)
            যাও fail;
        param[0] = ret;
        যদি (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            যদি (ret < 0)
                যাও fail;
            param[1] = ret;
        } অন্যথা {
            param[1] = 0;
        }
    } অন্যথা {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        যদি (ret)
            যাও fail;

        // Send parameters (if any).
        পূর্ণসংখ্যা i;
        জন্য (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            যদি (ret)
                যাও fail;
        }

        // Receive parameters (if any).
        জন্য (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            যদি (ret < 0)
                যাও fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_নিচের(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    যদি (ret2)
        ফেরত ret2;

    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
ps2_command(পূর্ণসংখ্যা aux, পূর্ণসংখ্যা command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    পূর্ণসংখ্যা ret = __ps2_command(aux, command, param);
    যদি (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ফেরত ret;
}

পূর্ণসংখ্যা
ps2_kbd_command(পূর্ণসংখ্যা command, u8 *param)
{
    যদি (! CONFIG_PS2PORT)
        ফেরত -1;
    ফেরত ps2_command(0, command, param);
}

পূর্ণসংখ্যা
ps2_mouse_command(পূর্ণসংখ্যা command, u8 *param)
{
    যদি (! CONFIG_PS2PORT)
        ফেরত -1;

    // Update ps2ctr for mouse enable/disable.
    যদি (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_নিচের(Ps2ctr);
        যদি (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        অন্যথা
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_নিচের(Ps2ctr, ps2ctr);
    }

    ফেরত ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
শূন্য VISIBLE16
handle_74(শূন্য)
{
    যদি (! CONFIG_PS2PORT)
        ফেরত;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    যদি ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        যাও done;
    }
    v = inb(PORT_PS2_তথ্য);

    যদি (!(GET_নিচের(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        যাও done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
শূন্য VISIBLE16
handle_09(শূন্য)
{
    যদি (! CONFIG_PS2PORT)
        ফেরত;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    যদি (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        যাও done;
    }
    v = inb(PORT_PS2_তথ্য);

    যদি (!(GET_নিচের(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        যাও done;

    process_চাবি(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ফাঁকা);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
শূন্য
ps2_check_event(শূন্য)
{
    যদি (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ফেরত;
    u8 ps2ctr = GET_নিচের(Ps2ctr);
    যদি ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ফেরত;
    জন্য (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        যদি (!(status & I8042_STR_OBF))
            বিরতি;
        u8 তথ্য = inb(PORT_PS2_তথ্য);
        যদি (status & I8042_STR_AUXDATA) {
            যদি (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(তথ্য);
        } অন্যথা {
            যদি (!(ps2ctr & I8042_CTR_KBDDIS))
                process_চাবি(তথ্য);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

স্থির শূন্য
ps2_keyboard_setup(শূন্য *তথ্য)
{
    // flush incoming keys (also verifies port is likely present)
    পূর্ণসংখ্যা ret = i8042_flush();
    যদি (ret)
        ফেরত;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ফাঁকা);
    যদি (ret)
        ফেরত;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ফাঁকা);
    যদি (ret)
        ফেরত;
    ret = i8042_flush();
    যদি (ret)
        ফেরত;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    যদি (ret)
        ফেরত;
    যদি (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ফেরত;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    যদি (ret)
        ফেরত;
    যদি (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ফেরত;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    পূর্ণসংখ্যা spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    জন্য (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        যদি (!ret)
            বিরতি;
        যদি (timer_check(end)) {
            যদি (spinupdelay)
                warn_timeout();
            ফেরত;
        }
        yield();
    }
    যদি (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ফেরত;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ফাঁকা);
    যদি (ret)
        ফেরত;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    যদি (ret)
        ফেরত;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ফাঁকা);
    যদি (ret)
        ফেরত;

    dprintf(1, "PS2 keyboard initialized\n");
}

শূন্য
ps2port_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (! CONFIG_PS2PORT)
        ফেরত;
    যদি (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ফেরত;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, ফাঁকা);
}
