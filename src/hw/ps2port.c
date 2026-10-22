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

static ကိန်းပြည့်
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    ကိန်းပြည့် i;
    အတွက် (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        အကယ်၍ (status & I8042_STR_OBF)
            ပြန်ပေး 0;
        udelay(50);
    }
    warn_timeout();
    ပြန်ပေး -1;
}

static ကိန်းပြည့်
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    ကိန်းပြည့် i;
    အတွက် (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        အကယ်၍ (! (status & I8042_STR_IBF))
            ပြန်ပေး 0;
        udelay(50);
    }
    warn_timeout();
    ပြန်ပေး -1;
}

static ကိန်းပြည့်
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    ကိန်းပြည့် i;
    အတွက် (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        အကယ်၍ (! (status & I8042_STR_OBF))
            ပြန်ပေး 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ပြန်ပေး -1;
}

static ကိန်းပြည့်
__i8042_command(ကိန်းပြည့် command, u8 *param)
{
    ကိန်းပြည့် receive = (command >> 8) & 0xf;
    ကိန်းပြည့် send = (command >> 12) & 0xf;

    // Send the command.
    ကိန်းပြည့် ret = i8042_wait_write();
    အကယ်၍ (ret)
        ပြန်ပေး ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    ကိန်းပြည့် i;
    အတွက် (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        အကယ်၍ (ret)
            ပြန်ပေး ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    အတွက် (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        အကယ်၍ (ret)
            ပြန်ပေး ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ပြန်ပေး 0;
}

static ကိန်းပြည့်
i8042_command(ကိန်းပြည့် command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ကိန်းပြည့် ret = __i8042_command(command, param);
    အကယ်၍ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ပြန်ပေး ret;
}

static ကိန်းပြည့်
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ကိန်းပြည့် ret = i8042_wait_write();
    အကယ်၍ (! ret)
        outb(c, PORT_PS2_DATA);
    ပြန်ပေး ret;
}

static ကိန်းပြည့်
i8042_aux_write(u8 c)
{
    ပြန်ပေး i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    အကယ်၍ (! CONFIG_PS2PORT)
       ပြန်ပေး;
    ကိန်းပြည့် i;
    အတွက် (i=0; i<10; i++) {
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

static ကိန်းပြည့်
ps2_recvbyte(ကိန်းပြည့် aux, ကိန်းပြည့် needack, ကိန်းပြည့် timeout)
{
    u32 end = timer_calc(timeout);
    အတွက် (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        အကယ်၍ (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            အကယ်၍ (!!(status & I8042_STR_AUXDATA) == aux) {
                အကယ်၍ (!needack)
                    ပြန်ပေး data;
                အကယ်၍ (data == PS2_RET_ACK)
                    ပြန်ပေး data;
                အကယ်၍ (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ပြန်ပေး data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        အကယ်၍ (timer_check(end)) {
            warn_timeout();
            ပြန်ပေး -1;
        }
        yield();
    }
}

static ကိန်းပြည့်
ps2_sendbyte(ကိန်းပြည့် aux, u8 command, ကိန်းပြည့် timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ကိန်းပြည့် ret;
    အကယ်၍ (aux)
        ret = i8042_aux_write(command);
    မဟုတ်ပါက
        ret = i8042_kbd_write(command);
    အကယ်၍ (ret)
        ပြန်ပေး ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    အကယ်၍ (ret < 0)
        ပြန်ပေး ret;
    အကယ်၍ (ret != PS2_RET_ACK)
        ပြန်ပေး -1;

    ပြန်ပေး 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static ကိန်းပြည့်
__ps2_command(ကိန်းပြည့် aux, ကိန်းပြည့် command, u8 *param)
{
    ကိန်းပြည့် ret2;
    ကိန်းပြည့် receive = (command >> 8) & 0xf;
    ကိန်းပြည့် send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ကိန်းပြည့် ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    အကယ်၍ (ret)
        ပြန်ပေး ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    အကယ်၍ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    မဟုတ်ပါက
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    အကယ်၍ (ret)
        goto fail;

    အကယ်၍ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        အကယ်၍ (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        အကယ်၍ (ret < 0)
            goto fail;
        param[0] = ret;
        အကယ်၍ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            အကယ်၍ (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } မဟုတ်ပါက အကယ်၍ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        အကယ်၍ (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        အကယ်၍ (ret < 0)
            goto fail;
        param[0] = ret;
        အကယ်၍ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            အကယ်၍ (ret < 0)
                goto fail;
            param[1] = ret;
        } မဟုတ်ပါက {
            param[1] = 0;
        }
    } မဟုတ်ပါက {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        အကယ်၍ (ret)
            goto fail;

        // Send parameters (if any).
        ကိန်းပြည့် i;
        အတွက် (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            အကယ်၍ (ret)
                goto fail;
        }

        // Receive parameters (if any).
        အတွက် (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            အကယ်၍ (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    အကယ်၍ (ret2)
        ပြန်ပေး ret2;

    ပြန်ပေး ret;
}

static ကိန်းပြည့်
ps2_command(ကိန်းပြည့် aux, ကိန်းပြည့် command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ကိန်းပြည့် ret = __ps2_command(aux, command, param);
    အကယ်၍ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ပြန်ပေး ret;
}

ကိန်းပြည့်
ps2_kbd_command(ကိန်းပြည့် command, u8 *param)
{
    အကယ်၍ (! CONFIG_PS2PORT)
        ပြန်ပေး -1;
    ပြန်ပေး ps2_command(0, command, param);
}

ကိန်းပြည့်
ps2_mouse_command(ကိန်းပြည့် command, u8 *param)
{
    အကယ်၍ (! CONFIG_PS2PORT)
        ပြန်ပေး -1;

    // Update ps2ctr for mouse enable/disable.
    အကယ်၍ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        အကယ်၍ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        မဟုတ်ပါက
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    ပြန်ပေး ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    အကယ်၍ (! CONFIG_PS2PORT)
        ပြန်ပေး;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    အကယ်၍ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    အကယ်၍ (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    အကယ်၍ (! CONFIG_PS2PORT)
        ပြန်ပေး;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    အကယ်၍ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    အကယ်၍ (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    အကယ်၍ (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ပြန်ပေး;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    အကယ်၍ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ပြန်ပေး;
    အတွက် (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        အကယ်၍ (!(status & I8042_STR_OBF))
            bha_nga;
        u8 data = inb(PORT_PS2_DATA);
        အကယ်၍ (status & I8042_STR_AUXDATA) {
            အကယ်၍ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } မဟုတ်ပါက {
            အကယ်၍ (!(ps2ctr & I8042_CTR_KBDDIS))
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
    ကိန်းပြည့် ret = i8042_flush();
    အကယ်၍ (ret)
        ပြန်ပေး;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    အကယ်၍ (ret)
        ပြန်ပေး;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    အကယ်၍ (ret)
        ပြန်ပေး;
    ret = i8042_flush();
    အကယ်၍ (ret)
        ပြန်ပေး;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    အကယ်၍ (ret)
        ပြန်ပေး;
    အကယ်၍ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ပြန်ပေး;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    အကယ်၍ (ret)
        ပြန်ပေး;
    အကယ်၍ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ပြန်ပေး;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ကိန်းပြည့် spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    အတွက် (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        အကယ်၍ (!ret)
            bha_nga;
        အကယ်၍ (timer_check(end)) {
            အကယ်၍ (spinupdelay)
                warn_timeout();
            ပြန်ပေး;
        }
        yield();
    }
    အကယ်၍ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ပြန်ပေး;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    အကယ်၍ (ret)
        ပြန်ပေး;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    အကယ်၍ (ret)
        ပြန်ပေး;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    အကယ်၍ (ret)
        ပြန်ပေး;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    အကယ်၍ (! CONFIG_PS2PORT)
        ပြန်ပေး;
    အကယ်၍ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ပြန်ပေး;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
