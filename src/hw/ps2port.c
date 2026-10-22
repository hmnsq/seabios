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

static 정수
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    정수 i;
    위해 (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        만약 (status & I8042_STR_OBF)
            반환 0;
        udelay(50);
    }
    warn_timeout();
    반환 -1;
}

static 정수
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    정수 i;
    위해 (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        만약 (! (status & I8042_STR_IBF))
            반환 0;
        udelay(50);
    }
    warn_timeout();
    반환 -1;
}

static 정수
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    정수 i;
    위해 (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        만약 (! (status & I8042_STR_OBF))
            반환 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    반환 -1;
}

static 정수
__i8042_command(정수 command, u8 *param)
{
    정수 receive = (command >> 8) & 0xf;
    정수 send = (command >> 12) & 0xf;

    // Send the command.
    정수 ret = i8042_wait_write();
    만약 (ret)
        반환 ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    정수 i;
    위해 (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        만약 (ret)
            반환 ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    위해 (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        만약 (ret)
            반환 ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    반환 0;
}

static 정수
i8042_command(정수 command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    정수 ret = __i8042_command(command, param);
    만약 (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    반환 ret;
}

static 정수
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    정수 ret = i8042_wait_write();
    만약 (! ret)
        outb(c, PORT_PS2_DATA);
    반환 ret;
}

static 정수
i8042_aux_write(u8 c)
{
    반환 i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    만약 (! CONFIG_PS2PORT)
       반환;
    정수 i;
    위해 (i=0; i<10; i++) {
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

static 정수
ps2_recvbyte(정수 aux, 정수 needack, 정수 timeout)
{
    u32 end = timer_calc(timeout);
    위해 (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        만약 (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            만약 (!!(status & I8042_STR_AUXDATA) == aux) {
                만약 (!needack)
                    반환 data;
                만약 (data == PS2_RET_ACK)
                    반환 data;
                만약 (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    반환 data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        만약 (timer_check(end)) {
            warn_timeout();
            반환 -1;
        }
        yield();
    }
}

static 정수
ps2_sendbyte(정수 aux, u8 command, 정수 timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    정수 ret;
    만약 (aux)
        ret = i8042_aux_write(command);
    아니면
        ret = i8042_kbd_write(command);
    만약 (ret)
        반환 ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    만약 (ret < 0)
        반환 ret;
    만약 (ret != PS2_RET_ACK)
        반환 -1;

    반환 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static 정수
__ps2_command(정수 aux, 정수 command, u8 *param)
{
    정수 ret2;
    정수 receive = (command >> 8) & 0xf;
    정수 send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    정수 ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    만약 (ret)
        반환 ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    만약 (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    아니면
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    만약 (ret)
        goto fail;

    만약 ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        만약 (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        만약 (ret < 0)
            goto fail;
        param[0] = ret;
        만약 (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            만약 (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } 아니면 만약 (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        만약 (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        만약 (ret < 0)
            goto fail;
        param[0] = ret;
        만약 (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            만약 (ret < 0)
                goto fail;
            param[1] = ret;
        } 아니면 {
            param[1] = 0;
        }
    } 아니면 {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        만약 (ret)
            goto fail;

        // Send parameters (if any).
        정수 i;
        위해 (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            만약 (ret)
                goto fail;
        }

        // Receive parameters (if any).
        위해 (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            만약 (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    만약 (ret2)
        반환 ret2;

    반환 ret;
}

static 정수
ps2_command(정수 aux, 정수 command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    정수 ret = __ps2_command(aux, command, param);
    만약 (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    반환 ret;
}

정수
ps2_kbd_command(정수 command, u8 *param)
{
    만약 (! CONFIG_PS2PORT)
        반환 -1;
    반환 ps2_command(0, command, param);
}

정수
ps2_mouse_command(정수 command, u8 *param)
{
    만약 (! CONFIG_PS2PORT)
        반환 -1;

    // Update ps2ctr for mouse enable/disable.
    만약 (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        만약 (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        아니면
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    반환 ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    만약 (! CONFIG_PS2PORT)
        반환;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    만약 ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    만약 (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    만약 (! CONFIG_PS2PORT)
        반환;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    만약 (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    만약 (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    만약 (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        반환;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    만약 ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        반환;
    위해 (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        만약 (!(status & I8042_STR_OBF))
            break;
        u8 data = inb(PORT_PS2_DATA);
        만약 (status & I8042_STR_AUXDATA) {
            만약 (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } 아니면 {
            만약 (!(ps2ctr & I8042_CTR_KBDDIS))
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
    정수 ret = i8042_flush();
    만약 (ret)
        반환;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    만약 (ret)
        반환;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    만약 (ret)
        반환;
    ret = i8042_flush();
    만약 (ret)
        반환;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    만약 (ret)
        반환;
    만약 (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        반환;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    만약 (ret)
        반환;
    만약 (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        반환;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    정수 spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    위해 (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        만약 (!ret)
            break;
        만약 (timer_check(end)) {
            만약 (spinupdelay)
                warn_timeout();
            반환;
        }
        yield();
    }
    만약 (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        반환;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    만약 (ret)
        반환;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    만약 (ret)
        반환;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    만약 (ret)
        반환;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    만약 (! CONFIG_PS2PORT)
        반환;
    만약 (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        반환;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
