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

static පූර්ණ_සංඛ්‍යා
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        නම් (status & I8042_STR_OBF)
            ආපසු 0;
        udelay(50);
    }
    warn_timeout();
    ආපසු -1;
}

static පූර්ණ_සංඛ්‍යා
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        නම් (! (status & I8042_STR_IBF))
            ආපසු 0;
        udelay(50);
    }
    warn_timeout();
    ආපසු -1;
}

static පූර්ණ_සංඛ්‍යා
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        නම් (! (status & I8042_STR_OBF))
            ආපසු 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ආපසු -1;
}

static පූර්ණ_සංඛ්‍යා
__i8042_command(පූර්ණ_සංඛ්‍යා command, u8 *param)
{
    පූර්ණ_සංඛ්‍යා receive = (command >> 8) & 0xf;
    පූර්ණ_සංඛ්‍යා send = (command >> 12) & 0xf;

    // Send the command.
    පූර්ණ_සංඛ්‍යා ret = i8042_wait_write();
    නම් (ret)
        ආපසු ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        නම් (ret)
            ආපසු ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    සඳහා (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        නම් (ret)
            ආපසු ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා
i8042_command(පූර්ණ_සංඛ්‍යා command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    පූර්ණ_සංඛ්‍යා ret = __i8042_command(command, param);
    නම් (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    පූර්ණ_සංඛ්‍යා ret = i8042_wait_write();
    නම් (! ret)
        outb(c, PORT_PS2_DATA);
    ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා
i8042_aux_write(u8 c)
{
    ආපසු i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    නම් (! CONFIG_PS2PORT)
       ආපසු;
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i=0; i<10; i++) {
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

static පූර්ණ_සංඛ්‍යා
ps2_recvbyte(පූර්ණ_සංඛ්‍යා aux, පූර්ණ_සංඛ්‍යා needack, පූර්ණ_සංඛ්‍යා timeout)
{
    u32 end = timer_calc(timeout);
    සඳහා (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        නම් (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            නම් (!!(status & I8042_STR_AUXDATA) == aux) {
                නම් (!needack)
                    ආපසු data;
                නම් (data == PS2_RET_ACK)
                    ආපසු data;
                නම් (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ආපසු data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        නම් (timer_check(end)) {
            warn_timeout();
            ආපසු -1;
        }
        yield();
    }
}

static පූර්ණ_සංඛ්‍යා
ps2_sendbyte(පූර්ණ_සංඛ්‍යා aux, u8 command, පූර්ණ_සංඛ්‍යා timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    පූර්ණ_සංඛ්‍යා ret;
    නම් (aux)
        ret = i8042_aux_write(command);
    නැතහොත්
        ret = i8042_kbd_write(command);
    නම් (ret)
        ආපසු ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    නම් (ret < 0)
        ආපසු ret;
    නම් (ret != PS2_RET_ACK)
        ආපසු -1;

    ආපසු 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static පූර්ණ_සංඛ්‍යා
__ps2_command(පූර්ණ_සංඛ්‍යා aux, පූර්ණ_සංඛ්‍යා command, u8 *param)
{
    පූර්ණ_සංඛ්‍යා ret2;
    පූර්ණ_සංඛ්‍යා receive = (command >> 8) & 0xf;
    පූර්ණ_සංඛ්‍යා send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    පූර්ණ_සංඛ්‍යා ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    නම් (ret)
        ආපසු ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    නම් (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    නැතහොත්
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    නම් (ret)
        goto fail;

    නම් ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        නම් (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        නම් (ret < 0)
            goto fail;
        param[0] = ret;
        නම් (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            නම් (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } නැතහොත් නම් (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        නම් (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        නම් (ret < 0)
            goto fail;
        param[0] = ret;
        නම් (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            නම් (ret < 0)
                goto fail;
            param[1] = ret;
        } නැතහොත් {
            param[1] = 0;
        }
    } නැතහොත් {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        නම් (ret)
            goto fail;

        // Send parameters (if any).
        පූර්ණ_සංඛ්‍යා i;
        සඳහා (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            නම් (ret)
                goto fail;
        }

        // Receive parameters (if any).
        සඳහා (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            නම් (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    නම් (ret2)
        ආපසු ret2;

    ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා
ps2_command(පූර්ණ_සංඛ්‍යා aux, පූර්ණ_සංඛ්‍යා command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    පූර්ණ_සංඛ්‍යා ret = __ps2_command(aux, command, param);
    නම් (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ආපසු ret;
}

පූර්ණ_සංඛ්‍යා
ps2_kbd_command(පූර්ණ_සංඛ්‍යා command, u8 *param)
{
    නම් (! CONFIG_PS2PORT)
        ආපසු -1;
    ආපසු ps2_command(0, command, param);
}

පූර්ණ_සංඛ්‍යා
ps2_mouse_command(පූර්ණ_සංඛ්‍යා command, u8 *param)
{
    නම් (! CONFIG_PS2PORT)
        ආපසු -1;

    // Update ps2ctr for mouse enable/disable.
    නම් (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        නම් (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        නැතහොත්
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    ආපසු ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    නම් (! CONFIG_PS2PORT)
        ආපසු;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    නම් ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    නම් (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    නම් (! CONFIG_PS2PORT)
        ආපසු;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    නම් (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    නම් (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    නම් (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ආපසු;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    නම් ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ආපසු;
    සඳහා (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        නම් (!(status & I8042_STR_OBF))
            bha_nga;
        u8 data = inb(PORT_PS2_DATA);
        නම් (status & I8042_STR_AUXDATA) {
            නම් (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } නැතහොත් {
            නම් (!(ps2ctr & I8042_CTR_KBDDIS))
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
    පූර්ණ_සංඛ්‍යා ret = i8042_flush();
    නම් (ret)
        ආපසු;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    නම් (ret)
        ආපසු;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    නම් (ret)
        ආපසු;
    ret = i8042_flush();
    නම් (ret)
        ආපසු;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    නම් (ret)
        ආපසු;
    නම් (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ආපසු;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    නම් (ret)
        ආපසු;
    නම් (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ආපසු;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    පූර්ණ_සංඛ්‍යා spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    සඳහා (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        නම් (!ret)
            bha_nga;
        නම් (timer_check(end)) {
            නම් (spinupdelay)
                warn_timeout();
            ආපසු;
        }
        yield();
    }
    නම් (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ආපසු;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    නම් (ret)
        ආපසු;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    නම් (ret)
        ආපසු;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    නම් (ret)
        ආපසු;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    නම් (! CONFIG_PS2PORT)
        ආපසු;
    නම් (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ආපසු;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
