// Support for handling the PS/2 mouse/keyboard ports.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Several ideas taken from code Copyright (c) 1999-2004 Vojtech Pavlik
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_LOW
#include "output.h" // dprintf
#include "pic.h" // pic_eoi1
#include "ps2port.h" // ps2_kbd_command
#include "romfile.h" // romfile_loadint
#include "stacks.h" // yield
#include "util.h" // udelay
#include "x86.h" // inb


/****************************************************************
 * Low level i8042 commands.
 ****************************************************************/

// Timeout value.
#define I8042_CTL_TIMEOUT       10000

#define I8042_BUFFER_SIZE       16

static ціле
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    ціле i;
    для (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        якщо (status & I8042_STR_OBF)
            повернути 0;
        udelay(50);
    }
    warn_timeout();
    повернути -1;
}

static ціле
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    ціле i;
    для (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        якщо (! (status & I8042_STR_IBF))
            повернути 0;
        udelay(50);
    }
    warn_timeout();
    повернути -1;
}

static ціле
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    ціле i;
    для (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        якщо (! (status & I8042_STR_OBF))
            повернути 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    повернути -1;
}

static ціле
__i8042_command(ціле command, u8 *param)
{
    ціле receive = (command >> 8) & 0xf;
    ціле send = (command >> 12) & 0xf;

    // Send the command.
    ціле ret = i8042_wait_write();
    якщо (ret)
        повернути ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    ціле i;
    для (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        якщо (ret)
            повернути ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    для (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        якщо (ret)
            повернути ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    повернути 0;
}

static ціле
i8042_command(ціле command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ціле ret = __i8042_command(command, param);
    якщо (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    повернути ret;
}

static ціле
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ціле ret = i8042_wait_write();
    якщо (! ret)
        outb(c, PORT_PS2_DATA);
    повернути ret;
}

static ціле
i8042_aux_write(u8 c)
{
    повернути i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    якщо (! CONFIG_PS2PORT)
       повернути;
    ціле i;
    для (i=0; i<10; i++) {
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

static ціле
ps2_recvbyte(ціле aux, ціле needack, ціле timeout)
{
    u32 end = timer_calc(timeout);
    для (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        якщо (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            якщо (!!(status & I8042_STR_AUXDATA) == aux) {
                якщо (!needack)
                    повернути data;
                якщо (data == PS2_RET_ACK)
                    повернути data;
                якщо (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    повернути data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        якщо (timer_check(end)) {
            warn_timeout();
            повернути -1;
        }
        yield();
    }
}

static ціле
ps2_sendbyte(ціле aux, u8 command, ціле timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ціле ret;
    якщо (aux)
        ret = i8042_aux_write(command);
    інакше
        ret = i8042_kbd_write(command);
    якщо (ret)
        повернути ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    якщо (ret < 0)
        повернути ret;
    якщо (ret != PS2_RET_ACK)
        повернути -1;

    повернути 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static ціле
__ps2_command(ціле aux, ціле command, u8 *param)
{
    ціле ret2;
    ціле receive = (command >> 8) & 0xf;
    ціле send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ціле ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    якщо (ret)
        повернути ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    якщо (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    інакше
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    якщо (ret)
        goto fail;

    якщо ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        якщо (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        якщо (ret < 0)
            goto fail;
        param[0] = ret;
        якщо (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            якщо (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } інакше якщо (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        якщо (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        якщо (ret < 0)
            goto fail;
        param[0] = ret;
        якщо (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            якщо (ret < 0)
                goto fail;
            param[1] = ret;
        } інакше {
            param[1] = 0;
        }
    } інакше {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        якщо (ret)
            goto fail;

        // Send parameters (if any).
        ціле i;
        для (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            якщо (ret)
                goto fail;
        }

        // Receive parameters (if any).
        для (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            якщо (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    якщо (ret2)
        повернути ret2;

    повернути ret;
}

static ціле
ps2_command(ціле aux, ціле command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ціле ret = __ps2_command(aux, command, param);
    якщо (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    повернути ret;
}

ціле
ps2_kbd_command(ціле command, u8 *param)
{
    якщо (! CONFIG_PS2PORT)
        повернути -1;
    повернути ps2_command(0, command, param);
}

ціле
ps2_mouse_command(ціле command, u8 *param)
{
    якщо (! CONFIG_PS2PORT)
        повернути -1;

    // Update ps2ctr for mouse enable/disable.
    якщо (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        якщо (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        інакше
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    повернути ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    якщо (! CONFIG_PS2PORT)
        повернути;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    якщо ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    якщо (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
        // Interrupts not enabled.
        goto done;

    process_mouse(v);

done:
    pic_eoi2();
}

// INT09h : Keyboard Hardware Service Entry Point
void VISIBLE16
handle_09(void)
{
    якщо (! CONFIG_PS2PORT)
        повернути;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    якщо (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    якщо (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
        // Interrupts not enabled.
        goto done;

    process_key(v);

    // Some old programs expect ISR to turn keyboard back on.
    i8042_command(I8042_CMD_KBD_ENABLE, NULL);

done:
    pic_eoi1();
}

// Check for ps2 activity on machines without hardware irqs
void
ps2_check_event(void)
{
    якщо (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        повернути;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    якщо ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        повернути;
    для (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        якщо (!(status & I8042_STR_OBF))
            break;
        u8 data = inb(PORT_PS2_DATA);
        якщо (status & I8042_STR_AUXDATA) {
            якщо (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } інакше {
            якщо (!(ps2ctr & I8042_CTR_KBDDIS))
                process_key(data);
        }
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

static void
ps2_keyboard_setup(void *data)
{
    // flush incoming keys (also verifies port is likely present)
    ціле ret = i8042_flush();
    якщо (ret)
        повернути;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    якщо (ret)
        повернути;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    якщо (ret)
        повернути;
    ret = i8042_flush();
    якщо (ret)
        повернути;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    якщо (ret)
        повернути;
    якщо (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        повернути;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    якщо (ret)
        повернути;
    якщо (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        повернути;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ціле spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    для (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        якщо (!ret)
            break;
        якщо (timer_check(end)) {
            якщо (spinupdelay)
                warn_timeout();
            повернути;
        }
        yield();
    }
    якщо (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        повернути;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    якщо (ret)
        повернути;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    якщо (ret)
        повернути;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    якщо (ret)
        повернути;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    якщо (! CONFIG_PS2PORT)
        повернути;
    якщо (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        повернути;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
