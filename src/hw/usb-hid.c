// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBAL
#include "config.h" // CONFIG_*
#include "output.h" // dprintf, warn_noalloc
#include "malloc.h" // malloc_fseg
#include "ps2port.h" // ATKBD_CMD_GETID
#include "usb.h" // usb_ctrlrequest
#include "usb-hid.h" // usb_keyboard_setup
#include "util.h" // process_key

sa_nracanaa pipe_node {
    sa_nracanaa usb_pipe *pipe;
    sa_nracanaa pipe_node *next;
};

sa_nracanaa pipe_node *keyboards VARFSEG = NULL;
sa_nracanaa pipe_node *mice VARFSEG = NULL;

static ຈຳນວນເຕັມ
add_pipe_node(sa_nracanaa pipe_node **list
              , sa_nracanaa usbdevice_s *usbdev
              , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    sa_nracanaa usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    ຖ້າ (!pipe)
        ສົ່ງຄືນ -1;

    sa_nracanaa pipe_node *new_node = malloc_fseg(parimaa_na(sa_nracanaa pipe_node));
    ຖ້າ (!new_node) {
        warn_noalloc();
        ສົ່ງຄືນ -1;
    }

    new_node->pipe = pipe;

    new_node->next = *list;
    *list = new_node;

    ສົ່ງຄືນ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
static ຈຳນວນເຕັມ
set_protocol(sa_nracanaa usb_pipe *pipe, u16 val, u16 inferface)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.wValue = val;
    req.wIndex = inferface;
    req.wLength = 0;
    ສົ່ງຄືນ usb_send_default_control(pipe, &req, NULL);
}

// Send USB HID SetIdle request.
static ຈຳນວນເຕັມ
set_idle(sa_nracanaa usb_pipe *pipe, ຈຳນວນເຕັມ ms)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.wValue = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    ສົ່ງຄືນ usb_send_default_control(pipe, &req, NULL);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
sa_nracanaa keyevent {
    u8 modifiers;
    u8 reserved;
    u8 keys[6];
};

#define MAX_KBD_EVENT 16

static ຈຳນວນເຕັມ
usb_kbd_setup(sa_nracanaa usbdevice_s *usbdev
              , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ຖ້າ (! CONFIG_USB_KEYBOARD)
        ສົ່ງຄືນ -1;

    ຖ້າ (epdesc->wMaxPacketSize < parimaa_na(sa_nracanaa keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        ສົ່ງຄືນ -1;
    }

    // Enable "boot" protocol.
    ຖ້າ (set_protocol(usbdev->defpipe, 0, usbdev->iface->bInterfaceNumber)) {
        dprintf(3, "Failed to set boot protocol\n");
        ສົ່ງຄືນ -1;
    }

    // Periodically send reports to enable key repeat.
    ຖ້າ (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    ຖ້າ (add_pipe_node(&keyboards, usbdev, epdesc))
        ສົ່ງຄືນ -1;

    dprintf(1, "USB keyboard initialized\n");
    ສົ່ງຄືນ 0;
}

// Format of USB mouse event data
sa_nracanaa mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

static ຈຳນວນເຕັມ
usb_mouse_setup(sa_nracanaa usbdevice_s *usbdev
                , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ຖ້າ (! CONFIG_USB_MOUSE)
        ສົ່ງຄືນ -1;

    ຖ້າ (epdesc->wMaxPacketSize < parimaa_na(sa_nracanaa mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        ສົ່ງຄືນ -1;
    }

    // Enable "boot" protocol.
    ຖ້າ (set_protocol(usbdev->defpipe, 0, usbdev->iface->bInterfaceNumber))
        ສົ່ງຄືນ -1;

    ຖ້າ (add_pipe_node(&mice, usbdev, epdesc))
        ສົ່ງຄືນ -1;

    dprintf(1, "USB mouse initialized\n");
    ສົ່ງຄືນ 0;
}

// Initialize a found USB HID device (if applicable).
ຈຳນວນເຕັມ
usb_hid_setup(sa_nracanaa usbdevice_s *usbdev)
{
    ຖ້າ (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        ສົ່ງຄືນ -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    sa_nracanaa usb_interface_descriptor *iface = usbdev->iface;
    ຖ້າ (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        ສົ່ງຄືນ -1;

    // Find intr in endpoint.
    sa_nracanaa usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    ຖ້າ (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        ສົ່ງຄືນ -1;
    }

    ຖ້າ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        ສົ່ງຄືນ usb_kbd_setup(usbdev, epdesc);
    ຖ້າ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        ສົ່ງຄືນ usb_mouse_setup(usbdev, epdesc);
    ສົ່ງຄືນ -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
static u16 KeyToScanCode[] VAR16 = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x001e, 0x0030, 0x002e, 0x0020,
    0x0012, 0x0021, 0x0022, 0x0023, 0x0017, 0x0024, 0x0025, 0x0026,
    0x0032, 0x0031, 0x0018, 0x0019, 0x0010, 0x0013, 0x001f, 0x0014,
    0x0016, 0x002f, 0x0011, 0x002d, 0x0015, 0x002c, 0x0002, 0x0003,
    0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b,
    0x001c, 0x0001, 0x000e, 0x000f, 0x0039, 0x000c, 0x000d, 0x001a,
    0x001b, 0x002b, 0x0000, 0x0027, 0x0028, 0x0029, 0x0033, 0x0034,
    0x0035, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 0x0040,
    0x0041, 0x0042, 0x0043, 0x0044, 0x0057, 0x0058, 0xe037, 0x0046,
    0xe145, 0xe052, 0xe047, 0xe049, 0xe053, 0xe04f, 0xe051, 0xe04d,
    0xe04b, 0xe050, 0xe048, 0x0045, 0xe035, 0x0037, 0x004a, 0x004e,
    0xe01c, 0x004f, 0x0050, 0x0051, 0x004b, 0x004c, 0x004d, 0x0047,
    0x0048, 0x0049, 0x0052, 0x0053
};

// Mapping from USB modifier id to ps2 key sequence.
static u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
static void
prockeys(u16 scancode, u8 key_release, u8 mods)
{
    ຖ້າ (scancode > 0xff) {
        ຖ້າ (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            ຖ້າ (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_key(0xe0);
                process_key(0x46 | key_release);
            } ບໍ່ດັ່ງນັ້ນ {
                // Pause key
                process_key(0xe1);
                process_key(0x1d | key_release);
                process_key(0x45 | key_release);
            }
            ສົ່ງຄືນ;
        } ບໍ່ດັ່ງນັ້ນ ຖ້າ (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_key(0x54 | key_release);
            ສົ່ງຄືນ;
        }
        process_key(0xe0);
    }
    process_key(scancode | key_release);
}

// Handle a USB key press/release event.
static void
procscankey(u8 key, u8 key_release, u8 mods)
{
    ຖ້າ (key >= ARRAY_SIZE(KeyToScanCode))
        ສົ່ງຄືນ;
    u16 scancode = GET_GLOBAL(KeyToScanCode[key]);
    ຖ້າ (scancode)
        prockeys(scancode, key_release, mods);
}

// Handle a USB modifier press/release event.
static void
procmodkey(u8 mods, u8 key_release)
{
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; mods; i++)
        ຖ້າ (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), key_release, 0);
            mods &= ~(1<<i);
        }
}

sa_nracanaa usbkeyinfo {
    sa_ngha {
        sa_nracanaa {
            u8 modifiers;
            u8 repeatcount;
            u8 keys[6];
        };
        u64 data;
    };
};
sa_nracanaa usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
static void
handle_key(sa_nracanaa keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->keys[0]);

    // Load old keys.
    sa_nracanaa usbkeyinfo old;
    old.data = GET_LOW(LastUSBkey.data);

    // Check for keys no longer pressed.
    ຈຳນວນເຕັມ addpos = 0;
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<ARRAY_SIZE(old.keys); i++) {
        u8 key = old.keys[i];
        ຖ້າ (!key)
            bha_nga;
        ຈຳນວນເຕັມ j;
        ສຳລັບ (j=0;; j++) {
            ຖ້າ (j>=ARRAY_SIZE(data->keys)) {
                // Key released.
                procscankey(key, RELEASEBIT, data->modifiers);
                ຖ້າ (i+1 >= ARRAY_SIZE(old.keys) || !old.keys[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                bha_nga;
            }
            ຖ້າ (data->keys[j] == key) {
                // Key still pressed.
                data->keys[j] = 0;
                old.keys[addpos++] = key;
                bha_nga;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    ສຳລັບ (i=0; i<ARRAY_SIZE(data->keys); i++) {
        u8 key = data->keys[i];
        ຖ້າ (!key)
            continue;
        // New key pressed.
        procscankey(key, 0, data->modifiers);
        old.keys[addpos++] = key;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    ຖ້າ (addpos < ARRAY_SIZE(old.keys))
        old.keys[addpos] = 0;

    // Check for key repeat event.
    ຖ້າ (addpos) {
        ຖ້າ (!old.repeatcount)
            procscankey(old.keys[addpos-1], 0, data->modifiers);
        ບໍ່ດັ່ງນັ້ນ ຖ້າ (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_LOW(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
static void
usb_check_key(void)
{
    ຖ້າ (! CONFIG_USB_KEYBOARD)
        ສົ່ງຄືນ;

    ສຳລັບ (sa_nracanaa pipe_node *node = GET_GLOBAL(keyboards);
         node;
         node = GET_GLOBALFLAT(node->next)) {
        sa_nracanaa usb_pipe *pipe = GET_GLOBALFLAT(node->pipe);

        ສຳລັບ (;;) {
            u8 data[MAX_KBD_EVENT];
            ຈຳນວນເຕັມ ret = usb_poll_intr(pipe, data);
            ຖ້າ (ret)
                bha_nga;
            handle_key((void*)data);
        }
    }
}

// Test if USB keyboard is active.
anta_hstha ຈຳນວນເຕັມ
usb_kbd_active(void)
{
    ຖ້າ (! CONFIG_USB_KEYBOARD)
        ສົ່ງຄືນ 0;

    ສົ່ງຄືນ GET_GLOBAL(keyboards) != NULL;
}

// Handle a ps2 style keyboard command.
anta_hstha ຈຳນວນເຕັມ
usb_kbd_command(ຈຳນວນເຕັມ command, u8 *param)
{
    ຖ້າ (! CONFIG_USB_KEYBOARD)
        ສົ່ງຄືນ -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    switch (command) {
    pak_sa ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        ສົ່ງຄືນ 0;
    default:
        ສົ່ງຄືນ -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
static void
handle_mouse(sa_nracanaa mouseevent *data)
{
    dprintf(9, "Got mouse b=%x x=%x y=%x\n", data->buttons, data->x, data->y);

    s8 x = data->x, y = -data->y;
    u8 flag = ((data->buttons & 0x7) | (1<<3)
               | (x & 0x80 ? (1<<4) : 0) | (y & 0x80 ? (1<<5) : 0));
    process_mouse(flag);
    process_mouse(x);
    process_mouse(y);
}

// Check if a USB mouse event is pending and process it if so.
static void
usb_check_mouse(void)
{
    ຖ້າ (! CONFIG_USB_MOUSE)
        ສົ່ງຄືນ;

    ສຳລັບ (sa_nracanaa pipe_node *node = GET_GLOBAL(mice);
         node;
         node = GET_GLOBALFLAT(node->next)) {
        sa_nracanaa usb_pipe *pipe = GET_GLOBALFLAT(node->pipe);

        ສຳລັບ (;;) {
            u8 data[MAX_MOUSE_EVENT];
            ຈຳນວນເຕັມ ret = usb_poll_intr(pipe, data);
            ຖ້າ (ret)
                bha_nga;
            handle_mouse((void*)data);
        }
    }
}

// Test if USB mouse is active.
anta_hstha ຈຳນວນເຕັມ
usb_mouse_active(void)
{
    ຖ້າ (! CONFIG_USB_MOUSE)
        ສົ່ງຄືນ 0;

    ສົ່ງຄືນ GET_GLOBAL(mice) != NULL;
}

// Handle a ps2 style mouse command.
anta_hstha ຈຳນວນເຕັມ
usb_mouse_command(ຈຳນວນເຕັມ command, u8 *param)
{
    ຖ້າ (! CONFIG_USB_MOUSE)
        ສົ່ງຄືນ -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    switch (command) {
    pak_sa PSMOUSE_CMD_ENABLE:
    pak_sa PSMOUSE_CMD_DISABLE:
    pak_sa PSMOUSE_CMD_SETSCALE11:
        ສົ່ງຄືນ 0;
    pak_sa PSMOUSE_CMD_SETSCALE21:
    pak_sa PSMOUSE_CMD_SETRATE:
    pak_sa PSMOUSE_CMD_SETRES:
        // XXX
        ສົ່ງຄືນ 0;
    pak_sa PSMOUSE_CMD_RESET_BAT:
    pak_sa PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        ສົ່ງຄືນ 0;

    pak_sa PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        ສົ່ງຄືນ 0;

    default:
        ສົ່ງຄືນ -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
void
usb_check_event(void)
{
    usb_check_key();
    usb_check_mouse();
}
