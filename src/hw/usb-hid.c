// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // GET_GLOBAL
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "output.h" // dprintf, warn_noalloc
#অন্তর্ভাৱ "malloc.h" // malloc_fseg
#অন্তর্ভাৱ "ps2port.h" // ATKBD_CMD_GETID
#অন্তর্ভাৱ "usb.h" // usb_ctrlrequest
#অন্তর্ভাৱ "usb-hid.h" // usb_keyboard_setup
#অন্তর্ভাৱ "util.h" // process_key

সংরচনা pipe_গ্ৰন্থি {
    সংরচনা usb_pipe *pipe;
    সংরচনা pipe_গ্ৰন্থি *next;
};

সংরচনা pipe_গ্ৰন্থি *keyboards VARFSEG = রিক্ত;
সংরচনা pipe_গ্ৰন্থি *mice VARFSEG = রিক্ত;

স্থির পূর্ণাঙ্ক
add_pipe_গ্ৰন্থি(সংরচনা pipe_গ্ৰন্থি **list
              , সংরচনা usbdevice_s *usbdev
              , সংরচনা usb_endpoint_descriptor *epdesc)
{
    সংরচনা usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    যদি (!pipe)
        ঘূৰাই -1;

    সংরচনা pipe_গ্ৰন্থি *new_গ্ৰন্থি = malloc_fseg(পরিমাণ(সংরচনা pipe_গ্ৰন্থি));
    যদি (!new_গ্ৰন্থি) {
        warn_noalloc();
        ঘূৰাই -1;
    }

    new_গ্ৰন্থি->pipe = pipe;

    new_গ্ৰন্থি->next = *list;
    *list = new_গ্ৰন্থি;

    ঘূৰাই 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
স্থির পূর্ণাঙ্ক
set_protocol(সংরচনা usb_pipe *pipe, u16 val, u16 inferface)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.w_মান = val;
    req.wIndex = inferface;
    req.wLength = 0;
    ঘূৰাই usb_send_default_control(pipe, &req, রিক্ত);
}

// Send USB HID SetIdle request.
স্থির পূর্ণাঙ্ক
set_idle(সংরচনা usb_pipe *pipe, পূর্ণাঙ্ক ms)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.w_মান = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    ঘূৰাই usb_send_default_control(pipe, &req, রিক্ত);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
সংরচনা keyevent {
    u8 modifiers;
    u8 reserved;
    u8 চাবিগুলি[6];
};

#define MAX_KBD_EVENT 16

স্থির পূর্ণাঙ্ক
usb_kbd_setup(সংরচনা usbdevice_s *usbdev
              , সংরচনা usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_KEYBOARD)
        ঘূৰাই -1;

    যদি (epdesc->wMaxPacketSize < পরিমাণ(সংরচনা keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        ঘূৰাই -1;
    }

    // Enable "boot" protocol.
    যদি (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_সংখ্যা)) {
        dprintf(3, "Failed to set boot protocol\n");
        ঘূৰাই -1;
    }

    // Periodically send reports to enable key repeat.
    যদি (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    যদি (add_pipe_গ্ৰন্থি(&keyboards, usbdev, epdesc))
        ঘূৰাই -1;

    dprintf(1, "USB keyboard initialized\n");
    ঘূৰাই 0;
}

// Format of USB mouse event data
সংরচনা mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

স্থির পূর্ণাঙ্ক
usb_mouse_setup(সংরচনা usbdevice_s *usbdev
                , সংরচনা usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_MOUSE)
        ঘূৰাই -1;

    যদি (epdesc->wMaxPacketSize < পরিমাণ(সংরচনা mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        ঘূৰাই -1;
    }

    // Enable "boot" protocol.
    যদি (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_সংখ্যা))
        ঘূৰাই -1;

    যদি (add_pipe_গ্ৰন্থি(&mice, usbdev, epdesc))
        ঘূৰাই -1;

    dprintf(1, "USB mouse initialized\n");
    ঘূৰাই 0;
}

// Initialize a found USB HID device (if applicable).
পূর্ণাঙ্ক
usb_hid_setup(সংরচনা usbdevice_s *usbdev)
{
    যদি (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        ঘূৰাই -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    সংরচনা usb_interface_descriptor *iface = usbdev->iface;
    যদি (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        ঘূৰাই -1;

    // Find intr in endpoint.
    সংরচনা usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    যদি (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        ঘূৰাই -1;
    }

    যদি (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        ঘূৰাই usb_kbd_setup(usbdev, epdesc);
    যদি (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        ঘূৰাই usb_mouse_setup(usbdev, epdesc);
    ঘূৰাই -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
স্থির u16 চাবি_To_Scan_Code[] VAR16 = {
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
স্থির u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
স্থির শূন্য
prockeys(u16 scancode, u8 চাবি_release, u8 mods)
{
    যদি (scancode > 0xff) {
        যদি (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            যদি (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_চাবি(0xe0);
                process_চাবি(0x46 | চাবি_release);
            } অন্যথা {
                // Pause key
                process_চাবি(0xe1);
                process_চাবি(0x1d | চাবি_release);
                process_চাবি(0x45 | চাবি_release);
            }
            ঘূৰাই;
        } অন্যথা যদি (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_চাবি(0x54 | চাবি_release);
            ঘূৰাই;
        }
        process_চাবি(0xe0);
    }
    process_চাবি(scancode | চাবি_release);
}

// Handle a USB key press/release event.
স্থির শূন্য
procscankey(u8 চাবি, u8 চাবি_release, u8 mods)
{
    যদি (চাবি >= ARRAY_SIZE(চাবি_To_Scan_Code))
        ঘূৰাই;
    u16 scancode = GET_GLOBAL(চাবি_To_Scan_Code[চাবি]);
    যদি (scancode)
        prockeys(scancode, চাবি_release, mods);
}

// Handle a USB modifier press/release event.
স্থির শূন্য
procmodkey(u8 mods, u8 চাবি_release)
{
    পূর্ণাঙ্ক i;
    বাবে (i=0; mods; i++)
        যদি (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), চাবি_release, 0);
            mods &= ~(1<<i);
        }
}

সংরচনা usbkeyinfo {
    সঙ্ঘ {
        সংরচনা {
            u8 modifiers;
            u8 repeatcount;
            u8 চাবিগুলি[6];
        };
        u64 data;
    };
};
সংরচনা usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
স্থির শূন্য
handle_চাবি(সংরচনা keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->চাবিগুলি[0]);

    // Load old keys.
    সংরচনা usbkeyinfo old;
    old.data = GET_নিচেৰ(LastUSBkey.data);

    // Check for keys no longer pressed.
    পূর্ণাঙ্ক addpos = 0;
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<ARRAY_SIZE(old.চাবিগুলি); i++) {
        u8 চাবি = old.চাবিগুলি[i];
        যদি (!চাবি)
            ভঙ্গ;
        পূর্ণাঙ্ক j;
        বাবে (j=0;; j++) {
            যদি (j>=ARRAY_SIZE(data->চাবিগুলি)) {
                // Key released.
                procscankey(চাবি, RELEASEBIT, data->modifiers);
                যদি (i+1 >= ARRAY_SIZE(old.চাবিগুলি) || !old.চাবিগুলি[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                ভঙ্গ;
            }
            যদি (data->চাবিগুলি[j] == চাবি) {
                // Key still pressed.
                data->চাবিগুলি[j] = 0;
                old.চাবিগুলি[addpos++] = চাবি;
                ভঙ্গ;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    বাবে (i=0; i<ARRAY_SIZE(data->চাবিগুলি); i++) {
        u8 চাবি = data->চাবিগুলি[i];
        যদি (!চাবি)
            অনুৱর্ত;
        // New key pressed.
        procscankey(চাবি, 0, data->modifiers);
        old.চাবিগুলি[addpos++] = চাবি;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    যদি (addpos < ARRAY_SIZE(old.চাবিগুলি))
        old.চাবিগুলি[addpos] = 0;

    // Check for key repeat event.
    যদি (addpos) {
        যদি (!old.repeatcount)
            procscankey(old.চাবিগুলি[addpos-1], 0, data->modifiers);
        অন্যথা যদি (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_নিচেৰ(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
স্থির শূন্য
usb_check_চাবি(শূন্য)
{
    যদি (! CONFIG_USB_KEYBOARD)
        ঘূৰাই;

    বাবে (সংরচনা pipe_গ্ৰন্থি *গ্ৰন্থি = GET_GLOBAL(keyboards);
         গ্ৰন্থি;
         গ্ৰন্থি = GET_GLOBALFLAT(গ্ৰন্থি->next)) {
        সংরচনা usb_pipe *pipe = GET_GLOBALFLAT(গ্ৰন্থি->pipe);

        বাবে (;;) {
            u8 data[MAX_KBD_EVENT];
            পূর্ণাঙ্ক ret = usb_poll_intr(pipe, data);
            যদি (ret)
                ভঙ্গ;
            handle_চাবি((শূন্য*)data);
        }
    }
}

// Test if USB keyboard is active.
অন্তঃস্থ পূর্ণাঙ্ক
usb_kbd_active(শূন্য)
{
    যদি (! CONFIG_USB_KEYBOARD)
        ঘূৰাই 0;

    ঘূৰাই GET_GLOBAL(keyboards) != রিক্ত;
}

// Handle a ps2 style keyboard command.
অন্তঃস্থ পূর্ণাঙ্ক
usb_kbd_command(পূর্ণাঙ্ক command, u8 *param)
{
    যদি (! CONFIG_USB_KEYBOARD)
        ঘূৰাই -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    ৱিকল্প (command) {
    পক্ষ ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        ঘূৰাই 0;
    স্ৱভাৱ:
        ঘূৰাই -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
স্থির শূন্য
handle_mouse(সংরচনা mouseevent *data)
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
স্থির শূন্য
usb_check_mouse(শূন্য)
{
    যদি (! CONFIG_USB_MOUSE)
        ঘূৰাই;

    বাবে (সংরচনা pipe_গ্ৰন্থি *গ্ৰন্থি = GET_GLOBAL(mice);
         গ্ৰন্থি;
         গ্ৰন্থি = GET_GLOBALFLAT(গ্ৰন্থি->next)) {
        সংরচনা usb_pipe *pipe = GET_GLOBALFLAT(গ্ৰন্থি->pipe);

        বাবে (;;) {
            u8 data[MAX_MOUSE_EVENT];
            পূর্ণাঙ্ক ret = usb_poll_intr(pipe, data);
            যদি (ret)
                ভঙ্গ;
            handle_mouse((শূন্য*)data);
        }
    }
}

// Test if USB mouse is active.
অন্তঃস্থ পূর্ণাঙ্ক
usb_mouse_active(শূন্য)
{
    যদি (! CONFIG_USB_MOUSE)
        ঘূৰাই 0;

    ঘূৰাই GET_GLOBAL(mice) != রিক্ত;
}

// Handle a ps2 style mouse command.
অন্তঃস্থ পূর্ণাঙ্ক
usb_mouse_command(পূর্ণাঙ্ক command, u8 *param)
{
    যদি (! CONFIG_USB_MOUSE)
        ঘূৰাই -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    ৱিকল্প (command) {
    পক্ষ PSMOUSE_CMD_ENABLE:
    পক্ষ PSMOUSE_CMD_DISABLE:
    পক্ষ PSMOUSE_CMD_SETSCALE11:
        ঘূৰাই 0;
    পক্ষ PSMOUSE_CMD_SETSCALE21:
    পক্ষ PSMOUSE_CMD_SETRATE:
    পক্ষ PSMOUSE_CMD_SETRES:
        // XXX
        ঘূৰাই 0;
    পক্ষ PSMOUSE_CMD_RESET_BAT:
    পক্ষ PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        ঘূৰাই 0;

    পক্ষ PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        ঘূৰাই 0;

    স্ৱভাৱ:
        ঘূৰাই -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
শূন্য
usb_check_event(শূন্য)
{
    usb_check_চাবি();
    usb_check_mouse();
}
