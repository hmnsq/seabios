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
#تعریف_کرو I8042_CTL_TIMEOUT       10000

#تعریف_کرو I8042_بفر_سائز       16

ساکن صحیح
i8042_wait_read(خالی)
{
    dprintf(7, "i8042_wait_read\n");
    صحیح i;
    برائے (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 حالت_2 = inb(پورٹ_PS2_حالت);
        اگر (حالت_2 & I8042_STR_OBF)
            واپس 0;
        udelay(50);
    }
    warn_timeout();
    واپس -1;
}

ساکن صحیح
i8042_wait_write(خالی)
{
    dprintf(7, "i8042_wait_write\n");
    صحیح i;
    برائے (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 حالت_2 = inb(پورٹ_PS2_حالت);
        اگر (! (حالت_2 & I8042_STR_IBF))
            واپس 0;
        udelay(50);
    }
    warn_timeout();
    واپس -1;
}

ساکن صحیح
i8042_flush(خالی)
{
    dprintf(7, "i8042_flush\n");
    صحیح i;
    برائے (i=0; i<I8042_بفر_سائز; i++) {
        u8 حالت_2 = inb(پورٹ_PS2_حالت);
        اگر (! (حالت_2 & I8042_STR_OBF))
            واپس 0;
        udelay(50);
        u8 ڈیٹا = inb(پورٹ_PS2_ڈیٹا);
        dprintf(7, "i8042 flushed %x (status=%x)\n", ڈیٹا, حالت_2);
    }

    warn_timeout();
    واپس -1;
}

ساکن صحیح
__i8042_command(صحیح command, u8 *param)
{
    صحیح receive = (command >> 8) & 0xf;
    صحیح send = (command >> 12) & 0xf;

    // Send the command.
    صحیح ret = i8042_wait_write();
    اگر (ret)
        واپس ret;
    outb(command, پورٹ_PS2_حالت);

    // Send parameters (if any).
    صحیح i;
    برائے (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        اگر (ret)
            واپس ret;
        outb(param[i], پورٹ_PS2_ڈیٹا);
    }

    // Receive parameters (if any).
    برائے (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        اگر (ret)
            واپس ret;
        param[i] = inb(پورٹ_PS2_ڈیٹا);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    واپس 0;
}

ساکن صحیح
i8042_command(صحیح command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    صحیح ret = __i8042_command(command, param);
    اگر (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    واپس ret;
}

ساکن صحیح
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    صحیح ret = i8042_wait_write();
    اگر (! ret)
        outb(c, پورٹ_PS2_ڈیٹا);
    واپس ret;
}

ساکن صحیح
i8042_aux_write(u8 c)
{
    واپس i8042_command(I8042_CMD_AUX_SEND, &c);
}

خالی
i8042_reboot(خالی)
{
    اگر (! CONFIG_PS2_پورٹ)
       واپس;
    صحیح i;
    برائے (i=0; i<10; i++) {
        i8042_wait_write();
        udelay(50);
        outb(0xfe, پورٹ_PS2_حالت); /* pulse reset low */
        udelay(50);
    }
}


/****************************************************************
 * Device commands.
 ****************************************************************/

#تعریف_کرو PS2_RET_ACK             0xfa
#تعریف_کرو PS2_RET_NAK             0xfe

ساکن صحیح
ps2_recvbyte(صحیح aux, صحیح needack, صحیح timeout)
{
    u32 انتہا = timer_calc(timeout);
    برائے (;;) {
        u8 حالت_2 = inb(پورٹ_PS2_حالت);
        اگر (حالت_2 & I8042_STR_OBF) {
            u8 ڈیٹا = inb(پورٹ_PS2_ڈیٹا);
            dprintf(7, "ps2 read %x\n", ڈیٹا);

            اگر (!!(حالت_2 & I8042_STR_AUXDATA) == aux) {
                اگر (!needack)
                    واپس ڈیٹا;
                اگر (ڈیٹا == PS2_RET_ACK)
                    واپس ڈیٹا;
                اگر (ڈیٹا == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", حالت_2);
                    واپس ڈیٹا;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", ڈیٹا, حالت_2);
        }

        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس -1;
        }
        yield();
    }
}

ساکن صحیح
ps2_sendbyte(صحیح aux, u8 command, صحیح timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    صحیح ret;
    اگر (aux)
        ret = i8042_aux_write(command);
    ورنہ
        ret = i8042_kbd_write(command);
    اگر (ret)
        واپس ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    اگر (ret < 0)
        واپس ret;
    اگر (ret != PS2_RET_ACK)
        واپس -1;

    واپس 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

ساکن صحیح
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
        واپس ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    سیٹ_کم(Ps2ctr, newctr);
    اگر (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ورنہ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    اگر (ret)
        جاؤ fail;

    اگر ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        اگر (ret)
            جاؤ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        اگر (ret < 0)
            جاؤ fail;
        param[0] = ret;
        اگر (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            اگر (ret < 0)
                جاؤ fail;
            param[1] = ret;
        }
    } ورنہ اگر (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        اگر (ret)
            جاؤ fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        اگر (ret < 0)
            جاؤ fail;
        param[0] = ret;
        اگر (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            اگر (ret < 0)
                جاؤ fail;
            param[1] = ret;
        } ورنہ {
            param[1] = 0;
        }
    } ورنہ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        اگر (ret)
            جاؤ fail;

        // Send parameters (if any).
        صحیح i;
        برائے (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            اگر (ret)
                جاؤ fail;
        }

        // Receive parameters (if any).
        برائے (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            اگر (ret < 0)
                جاؤ fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    سیٹ_کم(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    اگر (ret2)
        واپس ret2;

    واپس ret;
}

ساکن صحیح
ps2_command(صحیح aux, صحیح command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    صحیح ret = __ps2_command(aux, command, param);
    اگر (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    واپس ret;
}

صحیح
ps2_kbd_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_PS2_پورٹ)
        واپس -1;
    واپس ps2_command(0, command, param);
}

صحیح
ps2_mouse_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_PS2_پورٹ)
        واپس -1;

    // Update ps2ctr for mouse enable/disable.
    اگر (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_کم(Ps2ctr);
        اگر (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ورنہ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        سیٹ_کم(Ps2ctr, ps2ctr);
    }

    واپس ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
خالی VISIBLE16
handle_74(خالی)
{
    اگر (! CONFIG_PS2_پورٹ)
        واپس;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(پورٹ_PS2_حالت);
    اگر ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        جاؤ مکمل;
    }
    v = inb(پورٹ_PS2_ڈیٹا);

    اگر (!(GET_کم(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        جاؤ مکمل;

    process_mouse(v);

مکمل:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
خالی VISIBLE16
handle_09(خالی)
{
    اگر (! CONFIG_PS2_پورٹ)
        واپس;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(پورٹ_PS2_حالت);
    اگر (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        جاؤ مکمل;
    }
    v = inb(پورٹ_PS2_ڈیٹا);

    اگر (!(GET_کم(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        جاؤ مکمل;

    process_کلید(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, تہی);

مکمل:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
خالی
ps2_check_event(خالی)
{
    اگر (! CONFIG_PS2_پورٹ || CONFIG_HARDWARE_IRQ)
        واپس;
    u8 ps2ctr = GET_کم(Ps2ctr);
    اگر ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        واپس;
    برائے (;;) {
        u8 حالت_2 = inb(پورٹ_PS2_حالت);
        اگر (!(حالت_2 & I8042_STR_OBF))
            توڑو;
        u8 ڈیٹا = inb(پورٹ_PS2_ڈیٹا);
        اگر (حالت_2 & I8042_STR_AUXDATA) {
            اگر (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(ڈیٹا);
        } ورنہ {
            اگر (!(ps2ctr & I8042_CTR_KBDDIS))
                process_کلید(ڈیٹا);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ساکن خالی
ps2_keyboard_تیاری(خالی *ڈیٹا)
{
    // flush incoming keys (also verifies port is likely present)
    صحیح ret = i8042_flush();
    اگر (ret)
        واپس;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, تہی);
    اگر (ret)
        واپس;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, تہی);
    اگر (ret)
        واپس;
    ret = i8042_flush();
    اگر (ret)
        واپس;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    اگر (ret)
        واپس;
    اگر (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        واپس;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    اگر (ret)
        واپس;
    اگر (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        واپس;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    صحیح spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 انتہا = timer_calc(spinupdelay);
    برائے (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        اگر (!ret)
            توڑو;
        اگر (timer_check(انتہا)) {
            اگر (spinupdelay)
                warn_timeout();
            واپس;
        }
        yield();
    }
    اگر (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        واپس;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, تہی);
    اگر (ret)
        واپس;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    اگر (ret)
        واپس;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, تہی);
    اگر (ret)
        واپس;

    dprintf(1, "PS2 keyboard initialized\n");
}

خالی
ps2port_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (! CONFIG_PS2_پورٹ)
        واپس;
    اگر (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        واپس;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(اندراج_09));
    enable_hwirq(12, FUNC16(اندراج_74));

    تسلسل_thread(ps2_keyboard_تیاری, تہی);
}
