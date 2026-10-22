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

static số_nguyên
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    số_nguyên i;
    cho (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        nếu (status & I8042_STR_OBF)
            trả_về 0;
        udelay(50);
    }
    warn_timeout();
    trả_về -1;
}

static số_nguyên
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    số_nguyên i;
    cho (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        nếu (! (status & I8042_STR_IBF))
            trả_về 0;
        udelay(50);
    }
    warn_timeout();
    trả_về -1;
}

static số_nguyên
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    số_nguyên i;
    cho (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        nếu (! (status & I8042_STR_OBF))
            trả_về 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    trả_về -1;
}

static số_nguyên
__i8042_command(số_nguyên command, u8 *param)
{
    số_nguyên receive = (command >> 8) & 0xf;
    số_nguyên send = (command >> 12) & 0xf;

    // Send the command.
    số_nguyên ret = i8042_wait_write();
    nếu (ret)
        trả_về ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    số_nguyên i;
    cho (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        nếu (ret)
            trả_về ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    cho (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        nếu (ret)
            trả_về ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    trả_về 0;
}

static số_nguyên
i8042_command(số_nguyên command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    số_nguyên ret = __i8042_command(command, param);
    nếu (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    trả_về ret;
}

static số_nguyên
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    số_nguyên ret = i8042_wait_write();
    nếu (! ret)
        outb(c, PORT_PS2_DATA);
    trả_về ret;
}

static số_nguyên
i8042_aux_write(u8 c)
{
    trả_về i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    nếu (! CONFIG_PS2PORT)
       trả_về;
    số_nguyên i;
    cho (i=0; i<10; i++) {
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

static số_nguyên
ps2_recvbyte(số_nguyên aux, số_nguyên needack, số_nguyên timeout)
{
    u32 end = timer_calc(timeout);
    cho (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        nếu (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            nếu (!!(status & I8042_STR_AUXDATA) == aux) {
                nếu (!needack)
                    trả_về data;
                nếu (data == PS2_RET_ACK)
                    trả_về data;
                nếu (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    trả_về data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        nếu (timer_check(end)) {
            warn_timeout();
            trả_về -1;
        }
        yield();
    }
}

static số_nguyên
ps2_sendbyte(số_nguyên aux, u8 command, số_nguyên timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    số_nguyên ret;
    nếu (aux)
        ret = i8042_aux_write(command);
    không_thì
        ret = i8042_kbd_write(command);
    nếu (ret)
        trả_về ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    nếu (ret < 0)
        trả_về ret;
    nếu (ret != PS2_RET_ACK)
        trả_về -1;

    trả_về 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static số_nguyên
__ps2_command(số_nguyên aux, số_nguyên command, u8 *param)
{
    số_nguyên ret2;
    số_nguyên receive = (command >> 8) & 0xf;
    số_nguyên send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    số_nguyên ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    nếu (ret)
        trả_về ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    nếu (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    không_thì
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    nếu (ret)
        goto fail;

    nếu ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        nếu (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        nếu (ret < 0)
            goto fail;
        param[0] = ret;
        nếu (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            nếu (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } không_thì nếu (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        nếu (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        nếu (ret < 0)
            goto fail;
        param[0] = ret;
        nếu (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            nếu (ret < 0)
                goto fail;
            param[1] = ret;
        } không_thì {
            param[1] = 0;
        }
    } không_thì {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        nếu (ret)
            goto fail;

        // Send parameters (if any).
        số_nguyên i;
        cho (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            nếu (ret)
                goto fail;
        }

        // Receive parameters (if any).
        cho (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            nếu (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    nếu (ret2)
        trả_về ret2;

    trả_về ret;
}

static số_nguyên
ps2_command(số_nguyên aux, số_nguyên command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    số_nguyên ret = __ps2_command(aux, command, param);
    nếu (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    trả_về ret;
}

số_nguyên
ps2_kbd_command(số_nguyên command, u8 *param)
{
    nếu (! CONFIG_PS2PORT)
        trả_về -1;
    trả_về ps2_command(0, command, param);
}

số_nguyên
ps2_mouse_command(số_nguyên command, u8 *param)
{
    nếu (! CONFIG_PS2PORT)
        trả_về -1;

    // Update ps2ctr for mouse enable/disable.
    nếu (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        nếu (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        không_thì
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    trả_về ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    nếu (! CONFIG_PS2PORT)
        trả_về;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    nếu ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    nếu (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    nếu (! CONFIG_PS2PORT)
        trả_về;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    nếu (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    nếu (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    nếu (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        trả_về;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    nếu ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        trả_về;
    cho (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        nếu (!(status & I8042_STR_OBF))
            break;
        u8 data = inb(PORT_PS2_DATA);
        nếu (status & I8042_STR_AUXDATA) {
            nếu (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } không_thì {
            nếu (!(ps2ctr & I8042_CTR_KBDDIS))
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
    số_nguyên ret = i8042_flush();
    nếu (ret)
        trả_về;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    nếu (ret)
        trả_về;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    nếu (ret)
        trả_về;
    ret = i8042_flush();
    nếu (ret)
        trả_về;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    nếu (ret)
        trả_về;
    nếu (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        trả_về;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    nếu (ret)
        trả_về;
    nếu (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        trả_về;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    số_nguyên spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    cho (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        nếu (!ret)
            break;
        nếu (timer_check(end)) {
            nếu (spinupdelay)
                warn_timeout();
            trả_về;
        }
        yield();
    }
    nếu (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        trả_về;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    nếu (ret)
        trả_về;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    nếu (ret)
        trả_về;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    nếu (ret)
        trả_về;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    nếu (! CONFIG_PS2PORT)
        trả_về;
    nếu (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        trả_về;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
