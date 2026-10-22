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
        ᤀᤡ᤺ (status & I8042_STR_OBF)
            ᤀᤡᤴ 0;
        udelay(50);
    }
    warn_timeout();
    ᤀᤡᤴ -1;
}

static puur_naa_nka
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    puur_naa_nka i;
    for (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ᤀᤡ᤺ (! (status & I8042_STR_IBF))
            ᤀᤡᤴ 0;
        udelay(50);
    }
    warn_timeout();
    ᤀᤡᤴ -1;
}

static puur_naa_nka
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    puur_naa_nka i;
    for (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ᤀᤡ᤺ (! (status & I8042_STR_OBF))
            ᤀᤡᤴ 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ᤀᤡᤴ -1;
}

static puur_naa_nka
__i8042_command(puur_naa_nka command, u8 *param)
{
    puur_naa_nka receive = (command >> 8) & 0xf;
    puur_naa_nka send = (command >> 12) & 0xf;

    // Send the command.
    puur_naa_nka ret = i8042_wait_write();
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    puur_naa_nka i;
    for (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ᤀᤡ᤺ (ret)
            ᤀᤡᤴ ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    for (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ᤀᤡ᤺ (ret)
            ᤀᤡᤴ ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ᤀᤡᤴ 0;
}

static puur_naa_nka
i8042_command(puur_naa_nka command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    puur_naa_nka ret = __i8042_command(command, param);
    ᤀᤡ᤺ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ᤀᤡᤴ ret;
}

static puur_naa_nka
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    puur_naa_nka ret = i8042_wait_write();
    ᤀᤡ᤺ (! ret)
        outb(c, PORT_PS2_DATA);
    ᤀᤡᤴ ret;
}

static puur_naa_nka
i8042_aux_write(u8 c)
{
    ᤀᤡᤴ i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    ᤀᤡ᤺ (! CONFIG_PS2PORT)
       ᤀᤡᤴ;
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
        ᤀᤡ᤺ (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            ᤀᤡ᤺ (!!(status & I8042_STR_AUXDATA) == aux) {
                ᤀᤡ᤺ (!needack)
                    ᤀᤡᤴ data;
                ᤀᤡ᤺ (data == PS2_RET_ACK)
                    ᤀᤡᤴ data;
                ᤀᤡ᤺ (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ᤀᤡᤴ data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        ᤀᤡ᤺ (timer_check(end)) {
            warn_timeout();
            ᤀᤡᤴ -1;
        }
        yield();
    }
}

static puur_naa_nka
ps2_sendbyte(puur_naa_nka aux, u8 command, puur_naa_nka timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    puur_naa_nka ret;
    ᤀᤡ᤺ (aux)
        ret = i8042_aux_write(command);
    ᤀᤡ᤺ᤆᤡ
        ret = i8042_kbd_write(command);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ᤀᤡ᤺ (ret < 0)
        ᤀᤡᤴ ret;
    ᤀᤡ᤺ (ret != PS2_RET_ACK)
        ᤀᤡᤴ -1;

    ᤀᤡᤴ 0;
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
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    ᤀᤡ᤺ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ᤀᤡ᤺ᤆᤡ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ᤀᤡ᤺ (ret)
        goto fail;

    ᤀᤡ᤺ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ᤀᤡ᤺ (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ᤀᤡ᤺ (ret < 0)
            goto fail;
        param[0] = ret;
        ᤀᤡ᤺ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ᤀᤡ᤺ (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ᤀᤡ᤺ (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ᤀᤡ᤺ (ret < 0)
            goto fail;
        param[0] = ret;
        ᤀᤡ᤺ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ᤀᤡ᤺ (ret < 0)
                goto fail;
            param[1] = ret;
        } ᤀᤡ᤺ᤆᤡ {
            param[1] = 0;
        }
    } ᤀᤡ᤺ᤆᤡ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ᤀᤡ᤺ (ret)
            goto fail;

        // Send parameters (if any).
        puur_naa_nka i;
        for (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ᤀᤡ᤺ (ret)
                goto fail;
        }

        // Receive parameters (if any).
        for (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ᤀᤡ᤺ (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ᤀᤡ᤺ (ret2)
        ᤀᤡᤴ ret2;

    ᤀᤡᤴ ret;
}

static puur_naa_nka
ps2_command(puur_naa_nka aux, puur_naa_nka command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    puur_naa_nka ret = __ps2_command(aux, command, param);
    ᤀᤡ᤺ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ᤀᤡᤴ ret;
}

puur_naa_nka
ps2_kbd_command(puur_naa_nka command, u8 *param)
{
    ᤀᤡ᤺ (! CONFIG_PS2PORT)
        ᤀᤡᤴ -1;
    ᤀᤡᤴ ps2_command(0, command, param);
}

puur_naa_nka
ps2_mouse_command(puur_naa_nka command, u8 *param)
{
    ᤀᤡ᤺ (! CONFIG_PS2PORT)
        ᤀᤡᤴ -1;

    // Update ps2ctr for mouse enable/disable.
    ᤀᤡ᤺ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        ᤀᤡ᤺ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ᤀᤡ᤺ᤆᤡ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    ᤀᤡᤴ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    ᤀᤡ᤺ (! CONFIG_PS2PORT)
        ᤀᤡᤴ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    ᤀᤡ᤺ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ᤀᤡ᤺ (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    ᤀᤡ᤺ (! CONFIG_PS2PORT)
        ᤀᤡᤴ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    ᤀᤡ᤺ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ᤀᤡ᤺ (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    ᤀᤡ᤺ (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ᤀᤡᤴ;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    ᤀᤡ᤺ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ᤀᤡᤴ;
    for (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ᤀᤡ᤺ (!(status & I8042_STR_OBF))
            bha_nga;
        u8 data = inb(PORT_PS2_DATA);
        ᤀᤡ᤺ (status & I8042_STR_AUXDATA) {
            ᤀᤡ᤺ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } ᤀᤡ᤺ᤆᤡ {
            ᤀᤡ᤺ (!(ps2ctr & I8042_CTR_KBDDIS))
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
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;
    ret = i8042_flush();
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;
    ᤀᤡ᤺ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ᤀᤡᤴ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;
    ᤀᤡ᤺ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ᤀᤡᤴ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    puur_naa_nka spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    for (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ᤀᤡ᤺ (!ret)
            bha_nga;
        ᤀᤡ᤺ (timer_check(end)) {
            ᤀᤡ᤺ (spinupdelay)
                warn_timeout();
            ᤀᤡᤴ;
        }
        yield();
    }
    ᤀᤡ᤺ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ᤀᤡᤴ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    ᤀᤡ᤺ (ret)
        ᤀᤡᤴ;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    ᤀᤡ᤺ (! CONFIG_PS2PORT)
        ᤀᤡᤴ;
    ᤀᤡ᤺ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ᤀᤡᤴ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
