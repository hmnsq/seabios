// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf, warn_noalloc
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_fseg
#ಅನ್ತರ್ಭಾವ "ps2port.h" // ATKBD_CMD_GETID
#ಅನ್ತರ್ಭಾವ "usb.h" // usb_ctrlrequest
#ಅನ್ತರ್ಭಾವ "usb-hid.h" // usb_keyboard_setup
#ಅನ್ತರ್ಭಾವ "util.h" // process_key

ಸಂರಚನಾ pipe_ಗಂಟು {
    ಸಂರಚನಾ usb_pipe *pipe;
    ಸಂರಚನಾ pipe_ಗಂಟು *next;
};

ಸಂರಚನಾ pipe_ಗಂಟು *keyboards VARFSEG = ರಿಕ್ತ;
ಸಂರಚನಾ pipe_ಗಂಟು *mice VARFSEG = ರಿಕ್ತ;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
add_pipe_ಗಂಟು(ಸಂರಚನಾ pipe_ಗಂಟು **list
              , ಸಂರಚನಾ usbdevice_s *usbdev
              , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಸಂರಚನಾ usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    ಯದಿ (!pipe)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ pipe_ಗಂಟು *new_ಗಂಟು = malloc_fseg(ಪರಿಮಾಣ(ಸಂರಚನಾ pipe_ಗಂಟು));
    ಯದಿ (!new_ಗಂಟು) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    new_ಗಂಟು->pipe = pipe;

    new_ಗಂಟು->next = *list;
    *list = new_ಗಂಟು;

    ಪ್ರತ್ಯಾಗಮ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
set_protocol(ಸಂರಚನಾ usb_pipe *pipe, u16 val, u16 inferface)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.w_ಮೌಲ್ಯ = val;
    req.wIndex = inferface;
    req.wLength = 0;
    ಪ್ರತ್ಯಾಗಮ usb_send_default_control(pipe, &req, ರಿಕ್ತ);
}

// Send USB HID SetIdle request.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
set_idle(ಸಂರಚನಾ usb_pipe *pipe, ಪೂರ್ಣಾಙ್ಕ ms)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.w_ಮೌಲ್ಯ = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    ಪ್ರತ್ಯಾಗಮ usb_send_default_control(pipe, &req, ರಿಕ್ತ);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
ಸಂರಚನಾ keyevent {
    u8 modifiers;
    u8 reserved;
    u8 ಕೀಲಿಗಳು[6];
};

#define MAX_KBD_EVENT 16

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_kbd_setup(ಸಂರಚನಾ usbdevice_s *usbdev
              , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಯದಿ (! CONFIG_USB_KEYBOARD)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (epdesc->wMaxPacketSize < ಪರಿಮಾಣ(ಸಂರಚನಾ keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    // Enable "boot" protocol.
    ಯದಿ (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_ಸಂಖ್ಯೆ)) {
        dprintf(3, "Failed to set boot protocol\n");
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    // Periodically send reports to enable key repeat.
    ಯದಿ (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    ಯದಿ (add_pipe_ಗಂಟು(&keyboards, usbdev, epdesc))
        ಪ್ರತ್ಯಾಗಮ -1;

    dprintf(1, "USB keyboard initialized\n");
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Format of USB mouse event data
ಸಂರಚನಾ mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_mouse_setup(ಸಂರಚನಾ usbdevice_s *usbdev
                , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಯದಿ (! CONFIG_USB_MOUSE)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (epdesc->wMaxPacketSize < ಪರಿಮಾಣ(ಸಂರಚನಾ mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    // Enable "boot" protocol.
    ಯದಿ (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_ಸಂಖ್ಯೆ))
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (add_pipe_ಗಂಟು(&mice, usbdev, epdesc))
        ಪ್ರತ್ಯಾಗಮ -1;

    dprintf(1, "USB mouse initialized\n");
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Initialize a found USB HID device (if applicable).
ಪೂರ್ಣಾಙ್ಕ
usb_hid_setup(ಸಂರಚನಾ usbdevice_s *usbdev)
{
    ಯದಿ (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        ಪ್ರತ್ಯಾಗಮ -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    ಸಂರಚನಾ usb_interface_descriptor *iface = usbdev->iface;
    ಯದಿ (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        ಪ್ರತ್ಯಾಗಮ -1;

    // Find intr in endpoint.
    ಸಂರಚನಾ usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    ಯದಿ (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಯದಿ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        ಪ್ರತ್ಯಾಗಮ usb_kbd_setup(usbdev, epdesc);
    ಯದಿ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        ಪ್ರತ್ಯಾಗಮ usb_mouse_setup(usbdev, epdesc);
    ಪ್ರತ್ಯಾಗಮ -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
ಸ್ಥಿರ u16 ಕೀಲಿ_To_Scan_Code[] VAR16 = {
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
ಸ್ಥಿರ u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
ಸ್ಥಿರ ಶೂನ್ಯ
prockeys(u16 scancode, u8 ಕೀಲಿ_release, u8 mods)
{
    ಯದಿ (scancode > 0xff) {
        ಯದಿ (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            ಯದಿ (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_ಕೀಲಿ(0xe0);
                process_ಕೀಲಿ(0x46 | ಕೀಲಿ_release);
            } ಅನ್ಯಥಾ {
                // Pause key
                process_ಕೀಲಿ(0xe1);
                process_ಕೀಲಿ(0x1d | ಕೀಲಿ_release);
                process_ಕೀಲಿ(0x45 | ಕೀಲಿ_release);
            }
            ಪ್ರತ್ಯಾಗಮ;
        } ಅನ್ಯಥಾ ಯದಿ (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_ಕೀಲಿ(0x54 | ಕೀಲಿ_release);
            ಪ್ರತ್ಯಾಗಮ;
        }
        process_ಕೀಲಿ(0xe0);
    }
    process_ಕೀಲಿ(scancode | ಕೀಲಿ_release);
}

// Handle a USB key press/release event.
ಸ್ಥಿರ ಶೂನ್ಯ
procscankey(u8 ಕೀಲಿ, u8 ಕೀಲಿ_release, u8 mods)
{
    ಯದಿ (ಕೀಲಿ >= ARRAY_SIZE(ಕೀಲಿ_To_Scan_Code))
        ಪ್ರತ್ಯಾಗಮ;
    u16 scancode = GET_GLOBAL(ಕೀಲಿ_To_Scan_Code[ಕೀಲಿ]);
    ಯದಿ (scancode)
        prockeys(scancode, ಕೀಲಿ_release, mods);
}

// Handle a USB modifier press/release event.
ಸ್ಥಿರ ಶೂನ್ಯ
procmodkey(u8 mods, u8 ಕೀಲಿ_release)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; mods; i++)
        ಯದಿ (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), ಕೀಲಿ_release, 0);
            mods &= ~(1<<i);
        }
}

ಸಂರಚನಾ usbkeyinfo {
    ಸಙ್ಘ {
        ಸಂರಚನಾ {
            u8 modifiers;
            u8 repeatcount;
            u8 ಕೀಲಿಗಳು[6];
        };
        u64 data;
    };
};
ಸಂರಚನಾ usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
ಸ್ಥಿರ ಶೂನ್ಯ
handle_ಕೀಲಿ(ಸಂರಚನಾ keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->ಕೀಲಿಗಳು[0]);

    // Load old keys.
    ಸಂರಚನಾ usbkeyinfo old;
    old.data = GET_ಕೆಳಗಿನ(LastUSBkey.data);

    // Check for keys no longer pressed.
    ಪೂರ್ಣಾಙ್ಕ addpos = 0;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ARRAY_SIZE(old.ಕೀಲಿಗಳು); i++) {
        u8 ಕೀಲಿ = old.ಕೀಲಿಗಳು[i];
        ಯದಿ (!ಕೀಲಿ)
            ಭಙ್ಗ;
        ಪೂರ್ಣಾಙ್ಕ j;
        ಪ್ರತಿ (j=0;; j++) {
            ಯದಿ (j>=ARRAY_SIZE(data->ಕೀಲಿಗಳು)) {
                // Key released.
                procscankey(ಕೀಲಿ, RELEASEBIT, data->modifiers);
                ಯದಿ (i+1 >= ARRAY_SIZE(old.ಕೀಲಿಗಳು) || !old.ಕೀಲಿಗಳು[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                ಭಙ್ಗ;
            }
            ಯದಿ (data->ಕೀಲಿಗಳು[j] == ಕೀಲಿ) {
                // Key still pressed.
                data->ಕೀಲಿಗಳು[j] = 0;
                old.ಕೀಲಿಗಳು[addpos++] = ಕೀಲಿ;
                ಭಙ್ಗ;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    ಪ್ರತಿ (i=0; i<ARRAY_SIZE(data->ಕೀಲಿಗಳು); i++) {
        u8 ಕೀಲಿ = data->ಕೀಲಿಗಳು[i];
        ಯದಿ (!ಕೀಲಿ)
            ಅನುವರ್ತ;
        // New key pressed.
        procscankey(ಕೀಲಿ, 0, data->modifiers);
        old.ಕೀಲಿಗಳು[addpos++] = ಕೀಲಿ;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    ಯದಿ (addpos < ARRAY_SIZE(old.ಕೀಲಿಗಳು))
        old.ಕೀಲಿಗಳು[addpos] = 0;

    // Check for key repeat event.
    ಯದಿ (addpos) {
        ಯದಿ (!old.repeatcount)
            procscankey(old.ಕೀಲಿಗಳು[addpos-1], 0, data->modifiers);
        ಅನ್ಯಥಾ ಯದಿ (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_ಕೆಳಗಿನ(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
ಸ್ಥಿರ ಶೂನ್ಯ
usb_check_ಕೀಲಿ(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_KEYBOARD)
        ಪ್ರತ್ಯಾಗಮ;

    ಪ್ರತಿ (ಸಂರಚನಾ pipe_ಗಂಟು *ಗಂಟು = GET_GLOBAL(keyboards);
         ಗಂಟು;
         ಗಂಟು = GET_GLOBALFLAT(ಗಂಟು->next)) {
        ಸಂರಚನಾ usb_pipe *pipe = GET_GLOBALFLAT(ಗಂಟು->pipe);

        ಪ್ರತಿ (;;) {
            u8 data[MAX_KBD_EVENT];
            ಪೂರ್ಣಾಙ್ಕ ret = usb_poll_intr(pipe, data);
            ಯದಿ (ret)
                ಭಙ್ಗ;
            handle_ಕೀಲಿ((ಶೂನ್ಯ*)data);
        }
    }
}

// Test if USB keyboard is active.
ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
usb_kbd_active(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_KEYBOARD)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಪ್ರತ್ಯಾಗಮ GET_GLOBAL(keyboards) != ರಿಕ್ತ;
}

// Handle a ps2 style keyboard command.
ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
usb_kbd_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    ಯದಿ (! CONFIG_USB_KEYBOARD)
        ಪ್ರತ್ಯಾಗಮ -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    ವಿಕಲ್ಪ (command) {
    ಪಕ್ಷ ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        ಪ್ರತ್ಯಾಗಮ 0;
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
ಸ್ಥಿರ ಶೂನ್ಯ
handle_mouse(ಸಂರಚನಾ mouseevent *data)
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
ಸ್ಥಿರ ಶೂನ್ಯ
usb_check_mouse(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_MOUSE)
        ಪ್ರತ್ಯಾಗಮ;

    ಪ್ರತಿ (ಸಂರಚನಾ pipe_ಗಂಟು *ಗಂಟು = GET_GLOBAL(mice);
         ಗಂಟು;
         ಗಂಟು = GET_GLOBALFLAT(ಗಂಟು->next)) {
        ಸಂರಚನಾ usb_pipe *pipe = GET_GLOBALFLAT(ಗಂಟು->pipe);

        ಪ್ರತಿ (;;) {
            u8 data[MAX_MOUSE_EVENT];
            ಪೂರ್ಣಾಙ್ಕ ret = usb_poll_intr(pipe, data);
            ಯದಿ (ret)
                ಭಙ್ಗ;
            handle_mouse((ಶೂನ್ಯ*)data);
        }
    }
}

// Test if USB mouse is active.
ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
usb_mouse_active(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_MOUSE)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಪ್ರತ್ಯಾಗಮ GET_GLOBAL(mice) != ರಿಕ್ತ;
}

// Handle a ps2 style mouse command.
ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
usb_mouse_command(ಪೂರ್ಣಾಙ್ಕ command, u8 *param)
{
    ಯದಿ (! CONFIG_USB_MOUSE)
        ಪ್ರತ್ಯಾಗಮ -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    ವಿಕಲ್ಪ (command) {
    ಪಕ್ಷ PSMOUSE_CMD_ENABLE:
    ಪಕ್ಷ PSMOUSE_CMD_DISABLE:
    ಪಕ್ಷ PSMOUSE_CMD_SETSCALE11:
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪಕ್ಷ PSMOUSE_CMD_SETSCALE21:
    ಪಕ್ಷ PSMOUSE_CMD_SETRATE:
    ಪಕ್ಷ PSMOUSE_CMD_SETRES:
        // XXX
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪಕ್ಷ PSMOUSE_CMD_RESET_BAT:
    ಪಕ್ಷ PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        ಪ್ರತ್ಯಾಗಮ 0;

    ಪಕ್ಷ PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        ಪ್ರತ್ಯಾಗಮ 0;

    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
ಶೂನ್ಯ
usb_check_event(ಶೂನ್ಯ)
{
    usb_check_ಕೀಲಿ();
    usb_check_mouse();
}
