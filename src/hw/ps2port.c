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

static puur_naa_nka
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    puur_naa_nka i;
    for (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ပေႃး (status & I8042_STR_OBF)
            ပွၵ်ႈ 0;
        udelay(50);
    }
    warn_timeout();
    ပွၵ်ႈ -1;
}

static puur_naa_nka
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    puur_naa_nka i;
    for (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ပေႃး (! (status & I8042_STR_IBF))
            ပွၵ်ႈ 0;
        udelay(50);
    }
    warn_timeout();
    ပွၵ်ႈ -1;
}

static puur_naa_nka
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    puur_naa_nka i;
    for (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ပေႃး (! (status & I8042_STR_OBF))
            ပွၵ်ႈ 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ပွၵ်ႈ -1;
}

static puur_naa_nka
__i8042_command(puur_naa_nka command, u8 *param)
{
    puur_naa_nka receive = (command >> 8) & 0xf;
    puur_naa_nka send = (command >> 12) & 0xf;

    // Send the command.
    puur_naa_nka ret = i8042_wait_write();
    ပေႃး (ret)
        ပွၵ်ႈ ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    puur_naa_nka i;
    for (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ပေႃး (ret)
            ပွၵ်ႈ ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    for (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ပေႃး (ret)
            ပွၵ်ႈ ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ပွၵ်ႈ 0;
}

static puur_naa_nka
i8042_command(puur_naa_nka command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    puur_naa_nka ret = __i8042_command(command, param);
    ပေႃး (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ပွၵ်ႈ ret;
}

static puur_naa_nka
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    puur_naa_nka ret = i8042_wait_write();
    ပေႃး (! ret)
        outb(c, PORT_PS2_DATA);
    ပွၵ်ႈ ret;
}

static puur_naa_nka
i8042_aux_write(u8 c)
{
    ပွၵ်ႈ i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    ပေႃး (! CONFIG_PS2PORT)
       ပွၵ်ႈ;
    puur_naa_nka i;
    for (i=0; i<10; i++) {
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

static puur_naa_nka
ps2_recvbyte(puur_naa_nka aux, puur_naa_nka needack, puur_naa_nka timeout)
{
    u32 end = timer_calc(timeout);
    for (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ပေႃး (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            ပေႃး (!!(status & I8042_STR_AUXDATA) == aux) {
                ပေႃး (!needack)
                    ပွၵ်ႈ data;
                ပေႃး (data == PS2_RET_ACK)
                    ပွၵ်ႈ data;
                ပေႃး (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ပွၵ်ႈ data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        ပေႃး (timer_check(end)) {
            warn_timeout();
            ပွၵ်ႈ -1;
        }
        yield();
    }
}

static puur_naa_nka
ps2_sendbyte(puur_naa_nka aux, u8 command, puur_naa_nka timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    puur_naa_nka ret;
    ပေႃး (aux)
        ret = i8042_aux_write(command);
    ပေႃးဢမ်ႇ
        ret = i8042_kbd_write(command);
    ပေႃး (ret)
        ပွၵ်ႈ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ပေႃး (ret < 0)
        ပွၵ်ႈ ret;
    ပေႃး (ret != PS2_RET_ACK)
        ပွၵ်ႈ -1;

    ပွၵ်ႈ 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static puur_naa_nka
__ps2_command(puur_naa_nka aux, puur_naa_nka command, u8 *param)
{
    puur_naa_nka ret2;
    puur_naa_nka receive = (command >> 8) & 0xf;
    puur_naa_nka send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    puur_naa_nka ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ပေႃး (ret)
        ပွၵ်ႈ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    ပေႃး (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ပေႃးဢမ်ႇ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ပေႃး (ret)
        goto fail;

    ပေႃး ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ပေႃး (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ပေႃး (ret < 0)
            goto fail;
        param[0] = ret;
        ပေႃး (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ပေႃး (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } ပေႃးဢမ်ႇ ပေႃး (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ပေႃး (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ပေႃး (ret < 0)
            goto fail;
        param[0] = ret;
        ပေႃး (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ပေႃး (ret < 0)
                goto fail;
            param[1] = ret;
        } ပေႃးဢမ်ႇ {
            param[1] = 0;
        }
    } ပေႃးဢမ်ႇ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ပေႃး (ret)
            goto fail;

        // Send parameters (if any).
        puur_naa_nka i;
        for (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ပေႃး (ret)
                goto fail;
        }

        // Receive parameters (if any).
        for (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ပေႃး (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ပေႃး (ret2)
        ပွၵ်ႈ ret2;

    ပွၵ်ႈ ret;
}

static puur_naa_nka
ps2_command(puur_naa_nka aux, puur_naa_nka command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    puur_naa_nka ret = __ps2_command(aux, command, param);
    ပေႃး (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ပွၵ်ႈ ret;
}

puur_naa_nka
ps2_kbd_command(puur_naa_nka command, u8 *param)
{
    ပေႃး (! CONFIG_PS2PORT)
        ပွၵ်ႈ -1;
    ပွၵ်ႈ ps2_command(0, command, param);
}

puur_naa_nka
ps2_mouse_command(puur_naa_nka command, u8 *param)
{
    ပေႃး (! CONFIG_PS2PORT)
        ပွၵ်ႈ -1;

    // Update ps2ctr for mouse enable/disable.
    ပေႃး (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        ပေႃး (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ပေႃးဢမ်ႇ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    ပွၵ်ႈ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    ပေႃး (! CONFIG_PS2PORT)
        ပွၵ်ႈ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    ပေႃး ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ပေႃး (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    ပေႃး (! CONFIG_PS2PORT)
        ပွၵ်ႈ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    ပေႃး (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ပေႃး (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    ပေႃး (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ပွၵ်ႈ;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    ပေႃး ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ပွၵ်ႈ;
    for (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ပေႃး (!(status & I8042_STR_OBF))
            bha_nga;
        u8 data = inb(PORT_PS2_DATA);
        ပေႃး (status & I8042_STR_AUXDATA) {
            ပေႃး (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } ပေႃးဢမ်ႇ {
            ပေႃး (!(ps2ctr & I8042_CTR_KBDDIS))
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
    puur_naa_nka ret = i8042_flush();
    ပေႃး (ret)
        ပွၵ်ႈ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    ပေႃး (ret)
        ပွၵ်ႈ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    ပေႃး (ret)
        ပွၵ်ႈ;
    ret = i8042_flush();
    ပေႃး (ret)
        ပွၵ်ႈ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ပေႃး (ret)
        ပွၵ်ႈ;
    ပေႃး (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ပွၵ်ႈ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ပေႃး (ret)
        ပွၵ်ႈ;
    ပေႃး (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ပွၵ်ႈ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    puur_naa_nka spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    for (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ပေႃး (!ret)
            bha_nga;
        ပေႃး (timer_check(end)) {
            ပေႃး (spinupdelay)
                warn_timeout();
            ပွၵ်ႈ;
        }
        yield();
    }
    ပေႃး (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ပွၵ်ႈ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    ပေႃး (ret)
        ပွၵ်ႈ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ပေႃး (ret)
        ပွၵ်ႈ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    ပေႃး (ret)
        ပွၵ်ႈ;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    ပေႃး (! CONFIG_PS2PORT)
        ပွၵ်ႈ;
    ပေႃး (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ပွၵ်ႈ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
