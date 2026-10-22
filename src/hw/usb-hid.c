// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "config.h" // CONFIG_*
#ضمن "output.h" // dprintf, warn_noalloc
#ضمن "malloc.h" // malloc_fseg
#ضمن "ps2port.h" // ATKBD_CMD_GETID
#ضمن "usb.h" // usb_ctrlrequest
#ضمن "usb-hid.h" // usb_keyboard_setup
#ضمن "util.h" // process_key

بنية pipe_node {
    بنية usb_pipe *pipe;
    بنية pipe_node *next;
};

بنية pipe_node *keyboards VARFSEG = عدم;
بنية pipe_node *mice VARFSEG = عدم;

ساكن صحيح
add_pipe_node(بنية pipe_node **list
              , بنية usbdevice_s *usbdev
              , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    𐡄𐡍 (!pipe)
        أرجع -1;

    بنية pipe_node *new_node = malloc_fseg(حجم(بنية pipe_node));
    𐡄𐡍 (!new_node) {
        warn_noalloc();
        أرجع -1;
    }

    new_node->pipe = pipe;

    new_node->next = *list;
    *list = new_node;

    أرجع 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
ساكن صحيح
set_protocol(بنية usb_pipe *pipe, u16 val, u16 inferface)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.wValue = val;
    req.wIndex = inferface;
    req.wLength = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

// Send USB HID SetIdle request.
ساكن صحيح
set_idle(بنية usb_pipe *pipe, صحيح ms)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.wValue = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
بنية keyevent {
    u8 modifiers;
    u8 reserved;
    u8 keys[6];
};

#define MAX_KBD_EVENT 16

ساكن صحيح
usb_kbd_setup(بنية usbdevice_s *usbdev
              , بنية usb_endpoint_descriptor *epdesc)
{
    𐡄𐡍 (! CONFIG_USB_KEYBOARD)
        أرجع -1;

    𐡄𐡍 (epdesc->wMaxPacketSize < حجم(بنية keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        أرجع -1;
    }

    // Enable "boot" protocol.
    𐡄𐡍 (set_protocol(usbdev->defpipe, 0, usbdev->iface->bInterfaceNumber)) {
        dprintf(3, "Failed to set boot protocol\n");
        أرجع -1;
    }

    // Periodically send reports to enable key repeat.
    𐡄𐡍 (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    𐡄𐡍 (add_pipe_node(&keyboards, usbdev, epdesc))
        أرجع -1;

    dprintf(1, "USB keyboard initialized\n");
    أرجع 0;
}

// Format of USB mouse event data
بنية mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

ساكن صحيح
usb_mouse_setup(بنية usbdevice_s *usbdev
                , بنية usb_endpoint_descriptor *epdesc)
{
    𐡄𐡍 (! CONFIG_USB_MOUSE)
        أرجع -1;

    𐡄𐡍 (epdesc->wMaxPacketSize < حجم(بنية mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        أرجع -1;
    }

    // Enable "boot" protocol.
    𐡄𐡍 (set_protocol(usbdev->defpipe, 0, usbdev->iface->bInterfaceNumber))
        أرجع -1;

    𐡄𐡍 (add_pipe_node(&mice, usbdev, epdesc))
        أرجع -1;

    dprintf(1, "USB mouse initialized\n");
    أرجع 0;
}

// Initialize a found USB HID device (if applicable).
صحيح
usb_hid_setup(بنية usbdevice_s *usbdev)
{
    𐡄𐡍 (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        أرجع -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    بنية usb_interface_descriptor *iface = usbdev->iface;
    𐡄𐡍 (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        أرجع -1;

    // Find intr in endpoint.
    بنية usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    𐡄𐡍 (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        أرجع -1;
    }

    𐡄𐡍 (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        أرجع usb_kbd_setup(usbdev, epdesc);
    𐡄𐡍 (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        أرجع usb_mouse_setup(usbdev, epdesc);
    أرجع -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
ساكن u16 KeyToScanCode[] VAR16 = {
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
ساكن u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
ساكن فراغ
prockeys(u16 scancode, u8 key_release, u8 mods)
{
    𐡄𐡍 (scancode > 0xff) {
        𐡄𐡍 (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            𐡄𐡍 (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_key(0xe0);
                process_key(0x46 | key_release);
            } وإلا {
                // Pause key
                process_key(0xe1);
                process_key(0x1d | key_release);
                process_key(0x45 | key_release);
            }
            أرجع;
        } وإلا 𐡄𐡍 (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_key(0x54 | key_release);
            أرجع;
        }
        process_key(0xe0);
    }
    process_key(scancode | key_release);
}

// Handle a USB key press/release event.
ساكن فراغ
procscankey(u8 key, u8 key_release, u8 mods)
{
    𐡄𐡍 (key >= ARRAY_SIZE(KeyToScanCode))
        أرجع;
    u16 scancode = GET_GLOBAL(KeyToScanCode[key]);
    𐡄𐡍 (scancode)
        prockeys(scancode, key_release, mods);
}

// Handle a USB modifier press/release event.
ساكن فراغ
procmodkey(u8 mods, u8 key_release)
{
    صحيح i;
    لكل (i=0; mods; i++)
        𐡄𐡍 (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), key_release, 0);
            mods &= ~(1<<i);
        }
}

بنية usbkeyinfo {
    اتحاد {
        بنية {
            u8 modifiers;
            u8 repeatcount;
            u8 keys[6];
        };
        u64 data;
    };
};
بنية usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
ساكن فراغ
handle_key(بنية keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->keys[0]);

    // Load old keys.
    بنية usbkeyinfo old;
    old.data = GET_LOW(LastUSBkey.data);

    // Check for keys no longer pressed.
    صحيح addpos = 0;
    صحيح i;
    لكل (i=0; i<ARRAY_SIZE(old.keys); i++) {
        u8 key = old.keys[i];
        𐡄𐡍 (!key)
            اكسر;
        صحيح j;
        لكل (j=0;; j++) {
            𐡄𐡍 (j>=ARRAY_SIZE(data->keys)) {
                // Key released.
                procscankey(key, RELEASEBIT, data->modifiers);
                𐡄𐡍 (i+1 >= ARRAY_SIZE(old.keys) || !old.keys[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                اكسر;
            }
            𐡄𐡍 (data->keys[j] == key) {
                // Key still pressed.
                data->keys[j] = 0;
                old.keys[addpos++] = key;
                اكسر;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    لكل (i=0; i<ARRAY_SIZE(data->keys); i++) {
        u8 key = data->keys[i];
        𐡄𐡍 (!key)
            استمر;
        // New key pressed.
        procscankey(key, 0, data->modifiers);
        old.keys[addpos++] = key;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    𐡄𐡍 (addpos < ARRAY_SIZE(old.keys))
        old.keys[addpos] = 0;

    // Check for key repeat event.
    𐡄𐡍 (addpos) {
        𐡄𐡍 (!old.repeatcount)
            procscankey(old.keys[addpos-1], 0, data->modifiers);
        وإلا 𐡄𐡍 (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_LOW(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
ساكن فراغ
usb_check_key(فراغ)
{
    𐡄𐡍 (! CONFIG_USB_KEYBOARD)
        أرجع;

    لكل (بنية pipe_node *node = GET_GLOBAL(keyboards);
         node;
         node = GET_GLOBALFLAT(node->next)) {
        بنية usb_pipe *pipe = GET_GLOBALFLAT(node->pipe);

        لكل (;;) {
            u8 data[MAX_KBD_EVENT];
            صحيح ret = usb_poll_intr(pipe, data);
            𐡄𐡍 (ret)
                اكسر;
            handle_key((فراغ*)data);
        }
    }
}

// Test if USB keyboard is active.
مضمن صحيح
usb_kbd_active(فراغ)
{
    𐡄𐡍 (! CONFIG_USB_KEYBOARD)
        أرجع 0;

    أرجع GET_GLOBAL(keyboards) != عدم;
}

// Handle a ps2 style keyboard command.
مضمن صحيح
usb_kbd_command(صحيح command, u8 *param)
{
    𐡄𐡍 (! CONFIG_USB_KEYBOARD)
        أرجع -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    بدل (command) {
    حالة ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        أرجع 0;
    افتراضي:
        أرجع -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
ساكن فراغ
handle_mouse(بنية mouseevent *data)
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
ساكن فراغ
usb_check_mouse(فراغ)
{
    𐡄𐡍 (! CONFIG_USB_MOUSE)
        أرجع;

    لكل (بنية pipe_node *node = GET_GLOBAL(mice);
         node;
         node = GET_GLOBALFLAT(node->next)) {
        بنية usb_pipe *pipe = GET_GLOBALFLAT(node->pipe);

        لكل (;;) {
            u8 data[MAX_MOUSE_EVENT];
            صحيح ret = usb_poll_intr(pipe, data);
            𐡄𐡍 (ret)
                اكسر;
            handle_mouse((فراغ*)data);
        }
    }
}

// Test if USB mouse is active.
مضمن صحيح
usb_mouse_active(فراغ)
{
    𐡄𐡍 (! CONFIG_USB_MOUSE)
        أرجع 0;

    أرجع GET_GLOBAL(mice) != عدم;
}

// Handle a ps2 style mouse command.
مضمن صحيح
usb_mouse_command(صحيح command, u8 *param)
{
    𐡄𐡍 (! CONFIG_USB_MOUSE)
        أرجع -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    بدل (command) {
    حالة PSMOUSE_CMD_ENABLE:
    حالة PSMOUSE_CMD_DISABLE:
    حالة PSMOUSE_CMD_SETSCALE11:
        أرجع 0;
    حالة PSMOUSE_CMD_SETSCALE21:
    حالة PSMOUSE_CMD_SETRATE:
    حالة PSMOUSE_CMD_SETRES:
        // XXX
        أرجع 0;
    حالة PSMOUSE_CMD_RESET_BAT:
    حالة PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        أرجع 0;

    حالة PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        أرجع 0;

    افتراضي:
        أرجع -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
فراغ
usb_check_event(فراغ)
{
    usb_check_key();
    usb_check_mouse();
}
