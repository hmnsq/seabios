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

static ཧྲིལ
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    ཧྲིལ i;
    དོན་ལུ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        གལ་སྲིད (status & I8042_STR_OBF)
            ལོག 0;
        udelay(50);
    }
    warn_timeout();
    ལོག -1;
}

static ཧྲིལ
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    ཧྲིལ i;
    དོན་ལུ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        གལ་སྲིད (! (status & I8042_STR_IBF))
            ལོག 0;
        udelay(50);
    }
    warn_timeout();
    ལོག -1;
}

static ཧྲིལ
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    ཧྲིལ i;
    དོན་ལུ (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        གལ་སྲིད (! (status & I8042_STR_OBF))
            ལོག 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ལོག -1;
}

static ཧྲིལ
__i8042_command(ཧྲིལ command, u8 *param)
{
    ཧྲིལ receive = (command >> 8) & 0xf;
    ཧྲིལ send = (command >> 12) & 0xf;

    // Send the command.
    ཧྲིལ ret = i8042_wait_write();
    གལ་སྲིད (ret)
        ལོག ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    ཧྲིལ i;
    དོན་ལུ (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        གལ་སྲིད (ret)
            ལོག ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    དོན་ལུ (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        གལ་སྲིད (ret)
            ལོག ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ལོག 0;
}

static ཧྲིལ
i8042_command(ཧྲིལ command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ཧྲིལ ret = __i8042_command(command, param);
    གལ་སྲིད (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ལོག ret;
}

static ཧྲིལ
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ཧྲིལ ret = i8042_wait_write();
    གལ་སྲིད (! ret)
        outb(c, PORT_PS2_DATA);
    ལོག ret;
}

static ཧྲིལ
i8042_aux_write(u8 c)
{
    ལོག i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    གལ་སྲིད (! CONFIG_PS2PORT)
       ལོག;
    ཧྲིལ i;
    དོན་ལུ (i=0; i<10; i++) {
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

static ཧྲིལ
ps2_recvbyte(ཧྲིལ aux, ཧྲིལ needack, ཧྲིལ timeout)
{
    u32 end = timer_calc(timeout);
    དོན་ལུ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        གལ་སྲིད (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            གལ་སྲིད (!!(status & I8042_STR_AUXDATA) == aux) {
                གལ་སྲིད (!needack)
                    ལོག data;
                གལ་སྲིད (data == PS2_RET_ACK)
                    ལོག data;
                གལ་སྲིད (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ལོག data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        གལ་སྲིད (timer_check(end)) {
            warn_timeout();
            ལོག -1;
        }
        yield();
    }
}

static ཧྲིལ
ps2_sendbyte(ཧྲིལ aux, u8 command, ཧྲིལ timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ཧྲིལ ret;
    གལ་སྲིད (aux)
        ret = i8042_aux_write(command);
    དེ་མིན
        ret = i8042_kbd_write(command);
    གལ་སྲིད (ret)
        ལོག ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    གལ་སྲིད (ret < 0)
        ལོག ret;
    གལ་སྲིད (ret != PS2_RET_ACK)
        ལོག -1;

    ལོག 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static ཧྲིལ
__ps2_command(ཧྲིལ aux, ཧྲིལ command, u8 *param)
{
    ཧྲིལ ret2;
    ཧྲིལ receive = (command >> 8) & 0xf;
    ཧྲིལ send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ཧྲིལ ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    གལ་སྲིད (ret)
        ལོག ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    གལ་སྲིད (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    དེ་མིན
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    གལ་སྲིད (ret)
        goto fail;

    གལ་སྲིད ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        གལ་སྲིད (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        གལ་སྲིད (ret < 0)
            goto fail;
        param[0] = ret;
        གལ་སྲིད (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            གལ་སྲིད (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } དེ་མིན གལ་སྲིད (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        གལ་སྲིད (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        གལ་སྲིད (ret < 0)
            goto fail;
        param[0] = ret;
        གལ་སྲིད (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            གལ་སྲིད (ret < 0)
                goto fail;
            param[1] = ret;
        } དེ་མིན {
            param[1] = 0;
        }
    } དེ་མིན {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        གལ་སྲིད (ret)
            goto fail;

        // Send parameters (if any).
        ཧྲིལ i;
        དོན་ལུ (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            གལ་སྲིད (ret)
                goto fail;
        }

        // Receive parameters (if any).
        དོན་ལུ (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            གལ་སྲིད (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    གལ་སྲིད (ret2)
        ལོག ret2;

    ལོག ret;
}

static ཧྲིལ
ps2_command(ཧྲིལ aux, ཧྲིལ command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ཧྲིལ ret = __ps2_command(aux, command, param);
    གལ་སྲིད (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ལོག ret;
}

ཧྲིལ
ps2_kbd_command(ཧྲིལ command, u8 *param)
{
    གལ་སྲིད (! CONFIG_PS2PORT)
        ལོག -1;
    ལོག ps2_command(0, command, param);
}

ཧྲིལ
ps2_mouse_command(ཧྲིལ command, u8 *param)
{
    གལ་སྲིད (! CONFIG_PS2PORT)
        ལོག -1;

    // Update ps2ctr for mouse enable/disable.
    གལ་སྲིད (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        གལ་སྲིད (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        དེ་མིན
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    ལོག ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    གལ་སྲིད (! CONFIG_PS2PORT)
        ལོག;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    གལ་སྲིད ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    གལ་སྲིད (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    གལ་སྲིད (! CONFIG_PS2PORT)
        ལོག;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    གལ་སྲིད (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    གལ་སྲིད (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    གལ་སྲིད (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ལོག;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    གལ་སྲིད ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ལོག;
    དོན་ལུ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        གལ་སྲིད (!(status & I8042_STR_OBF))
            bha_nga;
        u8 data = inb(PORT_PS2_DATA);
        གལ་སྲིད (status & I8042_STR_AUXDATA) {
            གལ་སྲིད (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } དེ་མིན {
            གལ་སྲིད (!(ps2ctr & I8042_CTR_KBDDIS))
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
    ཧྲིལ ret = i8042_flush();
    གལ་སྲིད (ret)
        ལོག;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    གལ་སྲིད (ret)
        ལོག;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    གལ་སྲིད (ret)
        ལོག;
    ret = i8042_flush();
    གལ་སྲིད (ret)
        ལོག;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    གལ་སྲིད (ret)
        ལོག;
    གལ་སྲིད (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ལོག;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    གལ་སྲིད (ret)
        ལོག;
    གལ་སྲིད (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ལོག;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ཧྲིལ spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    དོན་ལུ (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        གལ་སྲིད (!ret)
            bha_nga;
        གལ་སྲིད (timer_check(end)) {
            གལ་སྲིད (spinupdelay)
                warn_timeout();
            ལོག;
        }
        yield();
    }
    གལ་སྲིད (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ལོག;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    གལ་སྲིད (ret)
        ལོག;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    གལ་སྲིད (ret)
        ལོག;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    གལ་སྲིད (ret)
        ལོག;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    གལ་སྲིད (! CONFIG_PS2PORT)
        ལོག;
    གལ་སྲིད (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ལོག;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
