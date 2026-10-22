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

static ຈຳນວນເຕັມ
i8042_wait_read(void)
{
    dprintf(7, "i8042_wait_read\n");
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ຖ້າ (status & I8042_STR_OBF)
            ສົ່ງຄືນ 0;
        udelay(50);
    }
    warn_timeout();
    ສົ່ງຄືນ -1;
}

static ຈຳນວນເຕັມ
i8042_wait_write(void)
{
    dprintf(7, "i8042_wait_write\n");
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<I8042_CTL_TIMEOUT; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ຖ້າ (! (status & I8042_STR_IBF))
            ສົ່ງຄືນ 0;
        udelay(50);
    }
    warn_timeout();
    ສົ່ງຄືນ -1;
}

static ຈຳນວນເຕັມ
i8042_flush(void)
{
    dprintf(7, "i8042_flush\n");
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<I8042_BUFFER_SIZE; i++) {
        u8 status = inb(PORT_PS2_STATUS);
        ຖ້າ (! (status & I8042_STR_OBF))
            ສົ່ງຄືນ 0;
        udelay(50);
        u8 data = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 flushed %x (status=%x)\n", data, status);
    }

    warn_timeout();
    ສົ່ງຄືນ -1;
}

static ຈຳນວນເຕັມ
__i8042_command(ຈຳນວນເຕັມ command, u8 *param)
{
    ຈຳນວນເຕັມ receive = (command >> 8) & 0xf;
    ຈຳນວນເຕັມ send = (command >> 12) & 0xf;

    // Send the command.
    ຈຳນວນເຕັມ ret = i8042_wait_write();
    ຖ້າ (ret)
        ສົ່ງຄືນ ret;
    outb(command, PORT_PS2_STATUS);

    // Send parameters (if any).
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i = 0; i < send; i++) {
        ret = i8042_wait_write();
        ຖ້າ (ret)
            ສົ່ງຄືນ ret;
        outb(param[i], PORT_PS2_DATA);
    }

    // Receive parameters (if any).
    ສຳລັບ (i = 0; i < receive; i++) {
        ret = i8042_wait_read();
        ຖ້າ (ret)
            ສົ່ງຄືນ ret;
        param[i] = inb(PORT_PS2_DATA);
        dprintf(7, "i8042 param=%x\n", param[i]);
    }

    ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ
i8042_command(ຈຳນວນເຕັມ command, u8 *param)
{
    dprintf(7, "i8042_command cmd=%x\n", command);
    ຈຳນວນເຕັມ ret = __i8042_command(command, param);
    ຖ້າ (ret)
        dprintf(2, "i8042 command %x failed\n", command);
    ສົ່ງຄືນ ret;
}

static ຈຳນວນເຕັມ
i8042_kbd_write(u8 c)
{
    dprintf(7, "i8042_kbd_write c=%d\n", c);
    ຈຳນວນເຕັມ ret = i8042_wait_write();
    ຖ້າ (! ret)
        outb(c, PORT_PS2_DATA);
    ສົ່ງຄືນ ret;
}

static ຈຳນວນເຕັມ
i8042_aux_write(u8 c)
{
    ສົ່ງຄືນ i8042_command(I8042_CMD_AUX_SEND, &c);
}

void
i8042_reboot(void)
{
    ຖ້າ (! CONFIG_PS2PORT)
       ສົ່ງຄືນ;
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<10; i++) {
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

static ຈຳນວນເຕັມ
ps2_recvbyte(ຈຳນວນເຕັມ aux, ຈຳນວນເຕັມ needack, ຈຳນວນເຕັມ timeout)
{
    u32 end = timer_calc(timeout);
    ສຳລັບ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ຖ້າ (status & I8042_STR_OBF) {
            u8 data = inb(PORT_PS2_DATA);
            dprintf(7, "ps2 read %x\n", data);

            ຖ້າ (!!(status & I8042_STR_AUXDATA) == aux) {
                ຖ້າ (!needack)
                    ສົ່ງຄືນ data;
                ຖ້າ (data == PS2_RET_ACK)
                    ສົ່ງຄືນ data;
                ຖ້າ (data == PS2_RET_NAK) {
                    dprintf(1, "Got ps2 nak (status=%x)\n", status);
                    ສົ່ງຄືນ data;
                }
            }

            // This data not part of command - just discard it.
            dprintf(1, "Discarding ps2 data %02x (status=%02x)\n", data, status);
        }

        ຖ້າ (timer_check(end)) {
            warn_timeout();
            ສົ່ງຄືນ -1;
        }
        yield();
    }
}

static ຈຳນວນເຕັມ
ps2_sendbyte(ຈຳນວນເຕັມ aux, u8 command, ຈຳນວນເຕັມ timeout)
{
    dprintf(7, "ps2_sendbyte aux=%d cmd=%x\n", aux, command);
    ຈຳນວນເຕັມ ret;
    ຖ້າ (aux)
        ret = i8042_aux_write(command);
    ບໍ່ດັ່ງນັ້ນ
        ret = i8042_kbd_write(command);
    ຖ້າ (ret)
        ສົ່ງຄືນ ret;

    // Read ack.
    ret = ps2_recvbyte(aux, 1, timeout);
    ຖ້າ (ret < 0)
        ສົ່ງຄືນ ret;
    ຖ້າ (ret != PS2_RET_ACK)
        ສົ່ງຄືນ -1;

    ສົ່ງຄືນ 0;
}

u8 Ps2ctr VARLOW = I8042_CTR_KBDDIS | I8042_CTR_AUXDIS;

static ຈຳນວນເຕັມ
__ps2_command(ຈຳນວນເຕັມ aux, ຈຳນວນເຕັມ command, u8 *param)
{
    ຈຳນວນເຕັມ ret2;
    ຈຳນວນເຕັມ receive = (command >> 8) & 0xf;
    ຈຳນວນເຕັມ send = (command >> 12) & 0xf;

    // Disable interrupts and keyboard/mouse.
    u8 ps2ctr = GET_LOW(Ps2ctr);
    u8 newctr = ((ps2ctr | I8042_CTR_AUXDIS | I8042_CTR_KBDDIS)
                 & ~(I8042_CTR_KBDINT|I8042_CTR_AUXINT));
    dprintf(6, "i8042 ctr old=%x new=%x\n", ps2ctr, newctr);
    ຈຳນວນເຕັມ ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ຖ້າ (ret)
        ສົ່ງຄືນ ret;

    // Flush any interrupts already pending.
    yield();

    // Enable port command is being sent to.
    SET_LOW(Ps2ctr, newctr);
    ຖ້າ (aux)
        newctr &= ~I8042_CTR_AUXDIS;
    ບໍ່ດັ່ງນັ້ນ
        newctr &= ~I8042_CTR_KBDDIS;
    ret = i8042_command(I8042_CMD_CTL_WCTR, &newctr);
    ຖ້າ (ret)
        goto fail;

    ຖ້າ ((u8)command == (u8)ATKBD_CMD_RESET_BAT) {
        // Reset is special wrt timeouts.

        // Send command.
        ret = ps2_sendbyte(aux, command, 1000);
        ຖ້າ (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 4000);
        ຖ້າ (ret < 0)
            goto fail;
        param[0] = ret;
        ຖ້າ (receive > 1) {
            ret = ps2_recvbyte(aux, 0, 500);
            ຖ້າ (ret < 0)
                goto fail;
            param[1] = ret;
        }
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (command == ATKBD_CMD_GETID) {
        // Getid is special wrt bytes received.

        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ຖ້າ (ret)
            goto fail;

        // Receive parameters.
        ret = ps2_recvbyte(aux, 0, 500);
        ຖ້າ (ret < 0)
            goto fail;
        param[0] = ret;
        ຖ້າ (ret == 0xab || ret == 0xac || ret == 0x2b || ret == 0x5d
            || ret == 0x60 || ret == 0x47) {
            // These ids (keyboards) return two bytes.
            ret = ps2_recvbyte(aux, 0, 500);
            ຖ້າ (ret < 0)
                goto fail;
            param[1] = ret;
        } ບໍ່ດັ່ງນັ້ນ {
            param[1] = 0;
        }
    } ບໍ່ດັ່ງນັ້ນ {
        // Send command.
        ret = ps2_sendbyte(aux, command, 200);
        ຖ້າ (ret)
            goto fail;

        // Send parameters (if any).
        ຈຳນວນເຕັມ i;
        ສຳລັບ (i = 0; i < send; i++) {
            ret = ps2_sendbyte(aux, param[i], 200);
            ຖ້າ (ret)
                goto fail;
        }

        // Receive parameters (if any).
        ສຳລັບ (i = 0; i < receive; i++) {
            ret = ps2_recvbyte(aux, 0, 500);
            ຖ້າ (ret < 0)
                goto fail;
            param[i] = ret;
        }
    }

    ret = 0;

fail:
    // Restore interrupts and keyboard/mouse.
    SET_LOW(Ps2ctr, ps2ctr);
    ret2 = i8042_command(I8042_CMD_CTL_WCTR, &ps2ctr);
    ຖ້າ (ret2)
        ສົ່ງຄືນ ret2;

    ສົ່ງຄືນ ret;
}

static ຈຳນວນເຕັມ
ps2_command(ຈຳນວນເຕັມ aux, ຈຳນວນເຕັມ command, u8 *param)
{
    dprintf(7, "ps2_command aux=%d cmd=%x\n", aux, command);
    ຈຳນວນເຕັມ ret = __ps2_command(aux, command, param);
    ຖ້າ (ret)
        dprintf(2, "ps2 command %x failed (aux=%d)\n", command, aux);
    ສົ່ງຄືນ ret;
}

ຈຳນວນເຕັມ
ps2_kbd_command(ຈຳນວນເຕັມ command, u8 *param)
{
    ຖ້າ (! CONFIG_PS2PORT)
        ສົ່ງຄືນ -1;
    ສົ່ງຄືນ ps2_command(0, command, param);
}

ຈຳນວນເຕັມ
ps2_mouse_command(ຈຳນວນເຕັມ command, u8 *param)
{
    ຖ້າ (! CONFIG_PS2PORT)
        ສົ່ງຄືນ -1;

    // Update ps2ctr for mouse enable/disable.
    ຖ້າ (command == PSMOUSE_CMD_ENABLE || command == PSMOUSE_CMD_DISABLE) {
        u8 ps2ctr = GET_LOW(Ps2ctr);
        ຖ້າ (command == PSMOUSE_CMD_ENABLE)
            ps2ctr = ((ps2ctr | (CONFIG_HARDWARE_IRQ ? I8042_CTR_AUXINT : 0))
                      & ~I8042_CTR_AUXDIS);
        ບໍ່ດັ່ງນັ້ນ
            ps2ctr = (ps2ctr | I8042_CTR_AUXDIS) & ~I8042_CTR_AUXINT;
        SET_LOW(Ps2ctr, ps2ctr);
    }

    ສົ່ງຄືນ ps2_command(1, command, param);
}


/****************************************************************
 * IRQ handlers
 ****************************************************************/

// INT74h : PS/2 mouse hardware interrupt
void VISIBLE16
handle_74(void)
{
    ຖ້າ (! CONFIG_PS2PORT)
        ສົ່ງຄືນ;

    debug_isr(DEBUG_ISR_74);

    u8 v = inb(PORT_PS2_STATUS);
    ຖ້າ ((v & (I8042_STR_OBF|I8042_STR_AUXDATA))
        != (I8042_STR_OBF|I8042_STR_AUXDATA)) {
        dprintf(1, "ps2 mouse irq but no mouse data.\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ຖ້າ (!(GET_LOW(Ps2ctr) & I8042_CTR_AUXINT))
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
    ຖ້າ (! CONFIG_PS2PORT)
        ສົ່ງຄືນ;

    debug_isr(DEBUG_ISR_09);

    // read key from keyboard controller
    u8 v = inb(PORT_PS2_STATUS);
    ຖ້າ (v & I8042_STR_AUXDATA) {
        dprintf(1, "ps2 keyboard irq but found mouse data?!\n");
        goto done;
    }
    v = inb(PORT_PS2_DATA);

    ຖ້າ (!(GET_LOW(Ps2ctr) & I8042_CTR_KBDINT))
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
    ຖ້າ (! CONFIG_PS2PORT || CONFIG_HARDWARE_IRQ)
        ສົ່ງຄືນ;
    u8 ps2ctr = GET_LOW(Ps2ctr);
    ຖ້າ ((ps2ctr & (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        == (I8042_CTR_KBDDIS|I8042_CTR_AUXDIS))
        ສົ່ງຄືນ;
    ສຳລັບ (;;) {
        u8 status = inb(PORT_PS2_STATUS);
        ຖ້າ (!(status & I8042_STR_OBF))
            bha_nga;
        u8 data = inb(PORT_PS2_DATA);
        ຖ້າ (status & I8042_STR_AUXDATA) {
            ຖ້າ (!(ps2ctr & I8042_CTR_AUXDIS))
                process_mouse(data);
        } ບໍ່ດັ່ງນັ້ນ {
            ຖ້າ (!(ps2ctr & I8042_CTR_KBDDIS))
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
    ຈຳນວນເຕັມ ret = i8042_flush();
    ຖ້າ (ret)
        ສົ່ງຄືນ;

    // Disable keyboard / mouse and drain any input they may have sent
    ret = i8042_command(I8042_CMD_KBD_DISABLE, NULL);
    ຖ້າ (ret)
        ສົ່ງຄືນ;
    ret = i8042_command(I8042_CMD_AUX_DISABLE, NULL);
    ຖ້າ (ret)
        ສົ່ງຄືນ;
    ret = i8042_flush();
    ຖ້າ (ret)
        ສົ່ງຄືນ;

    // Controller self-test.
    u8 param[2];
    ret = i8042_command(I8042_CMD_CTL_TEST, param);
    ຖ້າ (ret)
        ສົ່ງຄືນ;
    ຖ້າ (param[0] != 0x55) {
        dprintf(1, "i8042 self test failed (got %x not 0x55)\n", param[0]);
        ສົ່ງຄືນ;
    }

    // Controller keyboard test.
    ret = i8042_command(I8042_CMD_KBD_TEST, param);
    ຖ້າ (ret)
        ສົ່ງຄືນ;
    ຖ້າ (param[0] != 0x00) {
        dprintf(1, "i8042 keyboard test failed (got %x not 0x00)\n", param[0]);
        ສົ່ງຄືນ;
    }


    /* ------------------- keyboard side ------------------------*/
    /* reset keyboard and self test  (keyboard side) */
    ຈຳນວນເຕັມ spinupdelay = romfile_loadint("etc/ps2-keyboard-spinup", 0);
    u32 end = timer_calc(spinupdelay);
    ສຳລັບ (;;) {
        ret = ps2_kbd_command(ATKBD_CMD_RESET_BAT, param);
        ຖ້າ (!ret)
            bha_nga;
        ຖ້າ (timer_check(end)) {
            ຖ້າ (spinupdelay)
                warn_timeout();
            ສົ່ງຄືນ;
        }
        yield();
    }
    ຖ້າ (param[0] != 0xaa) {
        dprintf(1, "keyboard self test failed (got %x not 0xaa)\n", param[0]);
        ສົ່ງຄືນ;
    }

    /* Disable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_RESET_DIS, NULL);
    ຖ້າ (ret)
        ສົ່ງຄືນ;

    // Set scancode command (mode 2)
    param[0] = 0x02;
    ret = ps2_kbd_command(ATKBD_CMD_SSCANSET, param);
    ຖ້າ (ret)
        ສົ່ງຄືນ;

    // Keyboard Mode: disable mouse, scan code convert, enable kbd IRQ
    Ps2ctr = (I8042_CTR_AUXDIS | I8042_CTR_XLATE
              | (CONFIG_HARDWARE_IRQ ? I8042_CTR_KBDINT : 0));

    /* Enable keyboard */
    ret = ps2_kbd_command(ATKBD_CMD_ENABLE, NULL);
    ຖ້າ (ret)
        ສົ່ງຄືນ;

    dprintf(1, "PS2 keyboard initialized\n");
}

void
ps2port_setup(void)
{
    ASSERT32FLAT();
    ຖ້າ (! CONFIG_PS2PORT)
        ສົ່ງຄືນ;
    ຖ້າ (acpi_dsdt_present_eisaid(0x0303) == 0) {
        dprintf(1, "ACPI: no PS/2 keyboard present\n");
        ສົ່ງຄືນ;
    }
    dprintf(3, "init ps2port\n");

    enable_hwirq(1, FUNC16(entry_09));
    enable_hwirq(12, FUNC16(entry_74));

    run_thread(ps2_keyboard_setup, NULL);
}
