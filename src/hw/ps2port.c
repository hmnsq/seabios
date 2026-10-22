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

static จำนวนเต็ม
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    จำนวนเต็ม i;
    สำหรับ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ถ้า (status & I8042_STR_OBF)
            คืน 0;
        udelay(50);
    }
    warn_timeout();
    คืน -1;
}

static จำนวนเต็ม
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    จำนวนเต็ม i;
    สำหรับ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ถ้า (! (status & I8042_STR_IBF))
            คืน 0;
        udelay(50);
    }
    warn_timeout();
    คืน -1;
}

static จำนวนเต็ม
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    จำนวนเต็ม i;
    สำหรับ (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ถ้า (! (status & I8042_STR_OBF))
            คืน 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    คืน -1;
}

static จำนวนเต็ม
__i8042_command(จำนวนเต็ม command, u8 *param)
{
    จำนวนเต็ม receive = (command >> 8) & 0xf;
    จำนวนเต็ม send = (command >> 12) & 0xf;

    // Send the command.
    จำนวนเต็ม ret = i8042_wait_write();
    ถ้า (ret)
        คืน ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    จำนวนเต็ม i;
    สำหรับ (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ถ้า (ret)
            คืน ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    สำหรับ (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ถ้า (ret)
            คืน ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    คืน 0;
}

static จำนวนเต็ม
i8042_command(จำนวนเต็ม command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    จำนวนเต็ม ret = __i8042_command(command, param);
    ถ้า (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    คืน ret;
}

static จำนวนเต็ม
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    จำนวนเต็ม ret = i8042_wait_write();
    ถ้า (! ret)
        outb(c, PORT_PS2_DATA);
    คืน ret;
}

static จำนวนเต็ม
i8042_aux_write(u8 c)
{
    คืน i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    ถ้า (! CONFIG_PS2PORT)
       คืน;
    จำนวนเต็ม i;
    สำหรับ (i=0; i<10; i++) {
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

static จำนวนเต็ม
ps2_recvbyte(จำนวนเต็ม aux, จำนวนเต็ม needack, จำนวนเต็ม timeout)
{
    u32 end = timer_calc(timeout);
    สำหรับ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ถ้า (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            ถ้า (!!(status & I8042_STR_AUXDATA) == aux) {
                ถ้า (!needack)
                    คืน data;
                ถ้า (data == PS2_RET_ACK)
                    คืน data;
                ถ้า (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    คืน data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        ถ้า (timer_check(end)) {
            warn_timeout();
            คืน -1;
        }
        yield();
    }
}

static จำนวนเต็ม
ps2_sendbyte(จำนวนเต็ม aux, u8 command, จำนวนเต็ม timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    จำนวนเต็ม ret;
    ถ้า (aux)
        ret = i8042_aux_write(command);
    มิฉะนั้น
        ret = i8042_kbd_write(command);
    ถ้า (ret)
        คืน ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ถ้า (ret < 0)
        คืน ret;
    ถ้า (ret != PS2_RET_ACK)
        คืน -1;

    คืน 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static จำนวนเต็ม
__ps2_command(จำนวนเต็ม aux, จำนวนเต็ม command, u8 *param)
{
    จำนวนเต็ม ret2;
    จำนวนเต็ม receive = (command >> 8) & 0xf;
    จำนวนเต็ม send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    จำนวนเต็ม ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ถ้า (ret)
        คืน ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    ถ้า (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    มิฉะนั้น
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ถ้า (ret)
        goto fail;

    ถ้า ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ถ้า (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ถ้า (ret < 0)
            goto fail;
        param[0] = ret;
        ถ้า (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ถ้า (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } มิฉะนั้น ถ้า (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ถ้า (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ถ้า (ret < 0)
            goto fail;
        param[0] = ret;
        ถ้า (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ถ้า (ret < 0)
                goto fail;
            param[1] = ret;
        } มิฉะนั้น {
            param[1] = 0;
        }
    } มิฉะนั้น {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ถ้า (ret)
            goto fail;

        // Send parameters (if any).
        จำนวนเต็ม i;
        สำหรับ (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ถ้า (ret)
                goto fail;
        }

        // Receive parameters (if any).
        สำหรับ (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ถ้า (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ถ้า (ret2)
        คืน ret2;

    คืน ret;
}

static จำนวนเต็ม
ps2_command(จำนวนเต็ม aux, จำนวนเต็ม command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    จำนวนเต็ม ret = __ps2_command(aux, command, param);
    ถ้า (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    คืน ret;
}

จำนวนเต็ม
ps2_kbd_command(จำนวนเต็ม command, u8 *param)
{
    ถ้า (! CONFIG_PS2PORT)
        คืน -1;
    คืน ps2_command(0, command, param);
}

จำนวนเต็ม
ps2_mouse_command(จำนวนเต็ม command, u8 *param)
{
    ถ้า (! CONFIG_PS2PORT)
        คืน -1;

    // Update ps2ctr for mouse enable/disable.
    ถ้า (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        ถ้า (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        มิฉะนั้น
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    คืน ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    ถ้า (! CONFIG_PS2PORT)
        คืน;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    ถ้า ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ถ้า (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    ถ้า (! CONFIG_PS2PORT)
        คืน;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    ถ้า (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ถ้า (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    ถ้า (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        คืน;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    ถ้า ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        คืน;
    สำหรับ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ถ้า (!(status & I8042_STR_OBF))
            bha_nga;
        u8 data = inb(PORT_PS2_DATA);
        ถ้า (status & I8042_STR_AUXDATA) {
            ถ้า (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } มิฉะนั้น {
            ถ้า (!(ps2ctr & I8042_CTR_KBDDIS))
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
    จำนวนเต็ม ret = i8042_flush();
    ถ้า (ret)
        คืน;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    ถ้า (ret)
        คืน;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    ถ้า (ret)
        คืน;
    ret = i8042_flush();
    ถ้า (ret)
        คืน;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ถ้า (ret)
        คืน;
    ถ้า (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        คืน;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ถ้า (ret)
        คืน;
    ถ้า (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        คืน;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    จำนวนเต็ม spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    สำหรับ (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ถ้า (!ret)
            bha_nga;
        ถ้า (timer_check(end)) {
            ถ้า (spinupdelay)
                warn_timeout();
            คืน;
        }
        yield();
    }
    ถ้า (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        คืน;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    ถ้า (ret)
        คืน;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ถ้า (ret)
        คืน;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    ถ้า (ret)
        คืน;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    ถ้า (! CONFIG_PS2PORT)
        คืน;
    ถ้า (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        คืน;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
