// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOW
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pic.h" // pic_eoi1
#ಅನ್ತರ್ಭಾವ "ps2port.h" // ps2_kbd_command
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "util.h" // udelay
#ಅನ್ತರ್ಭಾವ "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#ವ್ಯಾಖ್ಯಾನಿಸು I8042_CTL_TIMEOUT       10000

#ವ್ಯಾಖ್ಯಾನಿಸು I8042_ಬಫರ್_ಗಾತ್ರ       16

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
i8042_wait_read(ಶೂನ್ಯ)
{
    dprintf(7, "i8042_wait_read\n");
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 ಸ್ಥಿತಿ_2 = inb(ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);
        ಆದರೆ (ಸ್ಥಿತಿ_2 & I8042_STR_OBF)
            ಹಿಂತಿರುಗಿ 0;
        udelay(50);
    }
    warn_timeout();
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
i8042_wait_write(ಶೂನ್ಯ)
{
    dprintf(7, "i8042_wait_write\n");
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 ಸ್ಥಿತಿ_2 = inb(ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);
        ಆದರೆ (! (ಸ್ಥಿತಿ_2 & I8042_STR_IBF))
            ಹಿಂತಿರುಗಿ 0;
        udelay(50);
    }
    warn_timeout();
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
i8042_flush(ಶೂನ್ಯ)
{
    dprintf(7, "i8042_flush\n");
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<I8042_ಬಫರ್_ಗಾತ್ರ; i++) {
        u8 ಸ್ಥಿತಿ_2 = inb(ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);
        ಆದರೆ (! (ಸ್ಥಿತಿ_2 & I8042_STR_OBF))
            ಹಿಂತಿರುಗಿ 0;
        udelay(50);
        u8 ದತ್ತಾಂಶ = inb(ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);
        dprintf(7, "i8042 flushed %x (status=%x)\n", ದತ್ತಾಂಶ, ಸ್ಥಿತಿ_2);
    }

    warn_timeout();
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
__i8042_command(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    ಪೂರ್ಣಾಂಕ receive = (command >> 8) & 0xf;
    ಪೂರ್ಣಾಂಕ send = (command >> 12) & 0xf;

    // Send the command.
    ಪೂರ್ಣಾಂಕ ret = i8042_wait_write();
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    outb(command, ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);

    // Send parameters (if any).
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        outb(param[i], ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);
    }

    // Receive parameters (if any).
    ಗಾಗಿ (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        param[i] = inb(ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
i8042_command(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ಪೂರ್ಣಾಂಕ ret = __i8042_command(command, param);
    ಆದರೆ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ಪೂರ್ಣಾಂಕ ret = i8042_wait_write();
    ಆದರೆ (! ret)
        outb(c, ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
i8042_aux_write(u8 c)
{
    ಹಿಂತಿರುಗಿ i8042_command(I8042_CMD_AUX_SEND, &c);
}

ಶೂನ್ಯ
i8042_reboot(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PS2_ಪೋರ್ಟ್)
       ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<10; i++) {
        i8042_wait_write();
        udelay(50);
        outb(0xfe, ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ); /* pulse reset low */
        udelay(50);
    }
}


/****************************************************************
 * Device commands.
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು PS2_RET_ACK             0xfa
#ವ್ಯಾಖ್ಯಾನಿಸು PS2_RET_NAK             0xfe

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ps2_recvbyte(ಪೂರ್ಣಾಂಕ aux, ಪೂರ್ಣಾಂಕ needack, ಪೂರ್ಣಾಂಕ timeout)
{
    u32 ಕೊನೆ = timer_calc(timeout);
    ಗಾಗಿ (;;) {
        u8 ಸ್ಥಿತಿ_2 = inb(ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);
        ಆದರೆ (ಸ್ಥಿತಿ_2 & I8042_STR_OBF) {
            u8 ದತ್ತಾಂಶ = inb(ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);
            dprintf(7, "ps2 read %x\n", ದತ್ತಾಂಶ);

            ಆದರೆ (!!(ಸ್ಥಿತಿ_2 & I8042_STR_AUXDATA) == aux) {
                ಆದರೆ (!needack)
                    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
                ಆದರೆ (ದತ್ತಾಂಶ == PS2_RET_ACK)
                    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
                ಆದರೆ (ದತ್ತಾಂಶ == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", ಸ್ಥಿತಿ_2);
                    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", ದತ್ತಾಂಶ, ಸ್ಥಿತಿ_2);
        }

        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ps2_sendbyte(ಪೂರ್ಣಾಂಕ aux, u8 command, ಪೂರ್ಣಾಂಕ timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ಪೂರ್ಣಾಂಕ ret;
    ಆದರೆ (aux)
        ret = i8042_aux_write(command);
    ಇಲ್ಲದಿದ್ದರೆ
        ret = i8042_kbd_write(command);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ಆದರೆ (ret < 0)
        ಹಿಂತಿರುಗಿ ret;
    ಆದರೆ (ret != PS2_RET_ACK)
        ಹಿಂತಿರುಗಿ -1;

    ಹಿಂತಿರುಗಿ 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
__ps2_command(ಪೂರ್ಣಾಂಕ aux, ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    ಪೂರ್ಣಾಂಕ ret2;
    ಪೂರ್ಣಾಂಕ receive = (command >> 8) & 0xf;
    ಪೂರ್ಣಾಂಕ send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_ಕೆಳಗಿನ(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ಪೂರ್ಣಾಂಕ ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    ಗಣ_ಕೆಳಗಿನ(Ps2ctr, newctr);
    ಆದರೆ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ಇಲ್ಲದಿದ್ದರೆ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ಆದರೆ (ret)
        ಹೋಗು fail;

    ಆದರೆ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ಆದರೆ (ret)
            ಹೋಗು fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ಆದರೆ (ret < 0)
            ಹೋಗು fail;
        param[0] = ret;
        ಆದರೆ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ಆದರೆ (ret < 0)
                ಹೋಗು fail;
            param[1] = ret;
        }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ಆದರೆ (ret)
            ಹೋಗು fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ಆದರೆ (ret < 0)
            ಹೋಗು fail;
        param[0] = ret;
        ಆದರೆ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ಆದರೆ (ret < 0)
                ಹೋಗು fail;
            param[1] = ret;
        } ಇಲ್ಲದಿದ್ದರೆ {
            param[1] = 0;
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ಆದರೆ (ret)
            ಹೋಗು fail;

        // Send parameters (if any).
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ಆದರೆ (ret)
                ಹೋಗು fail;
        }

        // Receive parameters (if any).
        ಗಾಗಿ (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ಆದರೆ (ret < 0)
                ಹೋಗು fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    ಗಣ_ಕೆಳಗಿನ(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ಆದರೆ (ret2)
        ಹಿಂತಿರುಗಿ ret2;

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ps2_command(ಪೂರ್ಣಾಂಕ aux, ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ಪೂರ್ಣಾಂಕ ret = __ps2_command(aux, command, param);
    ಆದರೆ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ
ps2_kbd_command(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    ಆದರೆ (! CONFIG_PS2_ಪೋರ್ಟ್)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ ps2_command(0, command, param);
}

ಪೂರ್ಣಾಂಕ
ps2_mouse_command(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    ಆದರೆ (! CONFIG_PS2_ಪೋರ್ಟ್)
        ಹಿಂತಿರುಗಿ -1;

    // Update ps2ctr for mouse enable/disable.
    ಆದರೆ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_ಕೆಳಗಿನ(Ps2ctr);
        ಆದರೆ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ಇಲ್ಲದಿದ್ದರೆ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        ಗಣ_ಕೆಳಗಿನ(Ps2ctr, ps2ctr);
    }

    ಹಿಂತಿರುಗಿ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
ಶೂನ್ಯ VISIBLE16
handle_74(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PS2_ಪೋರ್ಟ್)
        ಹಿಂತಿರುಗಿ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);
    ಆದರೆ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        ಹೋಗು ಮುಗಿದ;
    }
    v = inb(ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);

    ಆದರೆ (!(GET_ಕೆಳಗಿನ(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        ಹೋಗು ಮುಗಿದ;

    process_mouse(v);

ಮುಗಿದ:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
ಶೂನ್ಯ VISIBLE16
handle_09(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PS2_ಪೋರ್ಟ್)
        ಹಿಂತಿರುಗಿ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);
    ಆದರೆ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        ಹೋಗು ಮುಗಿದ;
    }
    v = inb(ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);

    ಆದರೆ (!(GET_ಕೆಳಗಿನ(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        ಹೋಗು ಮುಗಿದ;

    process_ಕೀಲಿ(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, ಖಾಲಿ);

ಮುಗಿದ:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
ಶೂನ್ಯ
ps2_check_event(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PS2_ಪೋರ್ಟ್ || CONFIG_HARDWARE_IRQ)
        ಹಿಂತಿರುಗಿ;
    u8 ps2ctr = GET_ಕೆಳಗಿನ(Ps2ctr);
    ಆದರೆ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ಹಿಂತಿರುಗಿ;
    ಗಾಗಿ (;;) {
        u8 ಸ್ಥಿತಿ_2 = inb(ಪೋರ್ಟ್_PS2_ಸ್ಥಿತಿ);
        ಆದರೆ (!(ಸ್ಥಿತಿ_2 & I8042_STR_OBF))
            ನಿಲ್ಲಿಸು;
        u8 ದತ್ತಾಂಶ = inb(ಪೋರ್ಟ್_PS2_ದತ್ತಾಂಶ);
        ಆದರೆ (ಸ್ಥಿತಿ_2 & I8042_STR_AUXDATA) {
            ಆದರೆ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(ದತ್ತಾಂಶ);
        } ಇಲ್ಲದಿದ್ದರೆ {
            ಆದರೆ (!(ps2ctr & I8042_CTR_KBDDIS))
                process_ಕೀಲಿ(ದತ್ತಾಂಶ);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ
ps2_keyboard_ಸಿದ್ಧತೆ(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    // flush incoming keys (also verifies port is likely present)
    ಪೂರ್ಣಾಂಕ ret = i8042_flush();
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;
    ret = i8042_flush();
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ಹಿಂತಿರುಗಿ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ಹಿಂತಿರುಗಿ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ಪೂರ್ಣಾಂಕ spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 ಕೊನೆ = timer_calc(spinupdelay);
    ಗಾಗಿ (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ಆದರೆ (!ret)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            ಆದರೆ (spinupdelay)
                warn_timeout();
            ಹಿಂತಿರುಗಿ;
        }
        yield();
    }
    ಆದರೆ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ಹಿಂತಿರುಗಿ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, ಖಾಲಿ);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ;

    dprintf(1, "PS2 keyboard initialized\n");
}

ಶೂನ್ಯ
ps2port_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_PS2_ಪೋರ್ಟ್)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ಹಿಂತಿರುಗಿ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(ನಮೂದು_09));
    enable_hwirq(12, FUNC16(ನಮೂದು_74));

    ಸತತ_thread(ps2_keyboard_ಸಿದ್ಧತೆ, ಖಾಲಿ);
}
