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

ರಚನೆ pipe_ಗಂಟು {
    ರಚನೆ usb_pipe *pipe;
    ರಚನೆ pipe_ಗಂಟು *ಮುಂದಿನ;
};

ರಚನೆ pipe_ಗಂಟು *keyboards VARFSEG = ಖಾಲಿ;
ರಚನೆ pipe_ಗಂಟು *mice VARFSEG = ಖಾಲಿ;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಕೂಡಿಸು_pipe_ಗಂಟು(ರಚನೆ pipe_ಗಂಟು **list
              , ರಚನೆ usbdevice_s *usbdev
              , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ರಚನೆ usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    ಆದರೆ (!pipe)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ pipe_ಗಂಟು *new_ಗಂಟು = malloc_fseg(ಗಾತ್ರ(ರಚನೆ pipe_ಗಂಟು));
    ಆದರೆ (!new_ಗಂಟು) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }

    new_ಗಂಟು->pipe = pipe;

    new_ಗಂಟು->ಮುಂದಿನ = *list;
    *list = new_ಗಂಟು;

    ಹಿಂತಿರುಗಿ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಗಣ_protocol(ರಚನೆ usb_pipe *pipe, u16 val, u16 inferface)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಹೊರಹೋಗು | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_ಗಣ_PROTOCOL;
    req.w_ಮೌಲ್ಯ = val;
    req.w_ಸೂಚ್ಯಂಕ = inferface;
    req.w_ಉದ್ದಳತೆ = 0;
    ಹಿಂತಿರುಗಿ usb_send_default_control(pipe, &req, ಖಾಲಿ);
}

// Send USB HID SetIdle request.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಗಣ_idle(ರಚನೆ usb_pipe *pipe, ಪೂರ್ಣಾಂಕ ms)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಹೊರಹೋಗು | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_ಗಣ_IDLE;
    req.w_ಮೌಲ್ಯ = (ms/4)<<8;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = 0;
    ಹಿಂತಿರುಗಿ usb_send_default_control(pipe, &req, ಖಾಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿಸು KEYREPEATWAITMS 500
#ವ್ಯಾಖ್ಯಾನಿಸು KEYREPEATMS 33

// Format of USB keyboard event data
ರಚನೆ keyevent {
    u8 modifiers;
    u8 reserved;
    u8 ಕೀಲಿಗಳು[6];
};

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_KBD_EVENT 16

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_kbd_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev
              , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಆದರೆ (! CONFIG_USB_KEYBOARD)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (epdesc->w_Max_Packet_ಗಾತ್ರ < ಗಾತ್ರ(ರಚನೆ keyevent)
        || epdesc->w_Max_Packet_ಗಾತ್ರ > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_ಗಾತ್ರ);
        ಹಿಂತಿರುಗಿ -1;
    }

    // Enable "boot" protocol.
    ಆದರೆ (ಗಣ_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_ಸಂಖ್ಯೆ)) {
        dprintf(3, "Failed to set boot protocol\n");
        ಹಿಂತಿರುಗಿ -1;
    }

    // Periodically send reports to enable key repeat.
    ಆದರೆ (ಗಣ_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    ಆದರೆ (ಕೂಡಿಸು_pipe_ಗಂಟು(&keyboards, usbdev, epdesc))
        ಹಿಂತಿರುಗಿ -1;

    dprintf(1, "USB keyboard initialized\n");
    ಹಿಂತಿರುಗಿ 0;
}

// Format of USB mouse event data
ರಚನೆ mouseevent {
    u8 buttons;
    u8 x, y;
};

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_MOUSE_EVENT 8

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_mouse_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev
                , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಆದರೆ (! CONFIG_USB_MOUSE)
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (epdesc->w_Max_Packet_ಗಾತ್ರ < ಗಾತ್ರ(ರಚನೆ mouseevent)
        || epdesc->w_Max_Packet_ಗಾತ್ರ > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_ಗಾತ್ರ);
        ಹಿಂತಿರುಗಿ -1;
    }

    // Enable "boot" protocol.
    ಆದರೆ (ಗಣ_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_ಸಂಖ್ಯೆ))
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (ಕೂಡಿಸು_pipe_ಗಂಟು(&mice, usbdev, epdesc))
        ಹಿಂತಿರುಗಿ -1;

    dprintf(1, "USB mouse initialized\n");
    ಹಿಂತಿರುಗಿ 0;
}

// Initialize a found USB HID device (if applicable).
ಪೂರ್ಣಾಂಕ
usb_hid_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev)
{
    ಆದರೆ (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        ಹಿಂತಿರುಗಿ -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    ರಚನೆ usb_interface_descriptor *iface = usbdev->iface;
    ಆದರೆ (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        ಹಿಂತಿರುಗಿ -1;

    // Find intr in endpoint.
    ರಚನೆ usb_endpoint_descriptor *epdesc = usb_ಕಂಡುಹಿಡಿ_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_ಒಳ);
    ಆದರೆ (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        ಹಿಂತಿರುಗಿ -1;
    }

    ಆದರೆ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        ಹಿಂತಿರುಗಿ usb_kbd_ಸಿದ್ಧತೆ(usbdev, epdesc);
    ಆದರೆ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        ಹಿಂತಿರುಗಿ usb_mouse_ಸಿದ್ಧತೆ(usbdev, epdesc);
    ಹಿಂತಿರುಗಿ -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
ಸ್ಥಿರ u16 ಕೀಲಿ_ಗಮ್ಯ_ಪರಿಶೀಲನೆ_ಸಂಕೇತ[] VAR16 = {
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
ಸ್ಥಿರ u16 Modifier_ಗಮ್ಯ_ಪರಿಶೀಲನೆ_ಸಂಕೇತ[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#ವ್ಯಾಖ್ಯಾನಿಸು RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
ಸ್ಥಿರ ಶೂನ್ಯ
prockeys(u16 scancode, u8 ಕೀಲಿ_release, u8 mods)
{
    ಆದರೆ (scancode > 0xff) {
        ಆದರೆ (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            ಆದರೆ (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_ಕೀಲಿ(0xe0);
                process_ಕೀಲಿ(0x46 | ಕೀಲಿ_release);
            } ಇಲ್ಲದಿದ್ದರೆ {
                // Pause key
                process_ಕೀಲಿ(0xe1);
                process_ಕೀಲಿ(0x1d | ಕೀಲಿ_release);
                process_ಕೀಲಿ(0x45 | ಕೀಲಿ_release);
            }
            ಹಿಂತಿರುಗಿ;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_ಕೀಲಿ(0x54 | ಕೀಲಿ_release);
            ಹಿಂತಿರುಗಿ;
        }
        process_ಕೀಲಿ(0xe0);
    }
    process_ಕೀಲಿ(scancode | ಕೀಲಿ_release);
}

// Handle a USB key press/release event.
ಸ್ಥಿರ ಶೂನ್ಯ
procscankey(u8 ಕೀಲಿ, u8 ಕೀಲಿ_release, u8 mods)
{
    ಆದರೆ (ಕೀಲಿ >= ARRAY_ಗಾತ್ರ(ಕೀಲಿ_ಗಮ್ಯ_ಪರಿಶೀಲನೆ_ಸಂಕೇತ))
        ಹಿಂತಿರುಗಿ;
    u16 scancode = GET_GLOBAL(ಕೀಲಿ_ಗಮ್ಯ_ಪರಿಶೀಲನೆ_ಸಂಕೇತ[ಕೀಲಿ]);
    ಆದರೆ (scancode)
        prockeys(scancode, ಕೀಲಿ_release, mods);
}

// Handle a USB modifier press/release event.
ಸ್ಥಿರ ಶೂನ್ಯ
procmodkey(u8 mods, u8 ಕೀಲಿ_release)
{
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; mods; i++)
        ಆದರೆ (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(Modifier_ಗಮ್ಯ_ಪರಿಶೀಲನೆ_ಸಂಕೇತ[i]), ಕೀಲಿ_release, 0);
            mods &= ~(1<<i);
        }
}

ರಚನೆ usbkeyinfo {
    ಒಕ್ಕೂಟ {
        ರಚನೆ {
            u8 modifiers;
            u8 repeatcount;
            u8 ಕೀಲಿಗಳು[6];
        };
        u64 ದತ್ತಾಂಶ;
    };
};
ರಚನೆ usbkeyinfo ಕೊನೆಯ_USBkey VARLOW;

// Process USB keyboard data.
ಸ್ಥಿರ ಶೂನ್ಯ
handle_ಕೀಲಿ(ರಚನೆ keyevent *ದತ್ತಾಂಶ)
{
    dprintf(9, "Got key %x %x\n", ದತ್ತಾಂಶ->modifiers, ದತ್ತಾಂಶ->ಕೀಲಿಗಳು[0]);

    // Load old keys.
    ರಚನೆ usbkeyinfo old;
    old.ದತ್ತಾಂಶ = GET_ಕೆಳಗಿನ(ಕೊನೆಯ_USBkey.ದತ್ತಾಂಶ);

    // Check for keys no longer pressed.
    ಪೂರ್ಣಾಂಕ addpos = 0;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(old.ಕೀಲಿಗಳು); i++) {
        u8 ಕೀಲಿ = old.ಕೀಲಿಗಳು[i];
        ಆದರೆ (!ಕೀಲಿ)
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ಣಾಂಕ j;
        ಗಾಗಿ (j=0;; j++) {
            ಆದರೆ (j>=ARRAY_ಗಾತ್ರ(ದತ್ತಾಂಶ->ಕೀಲಿಗಳು)) {
                // Key released.
                procscankey(ಕೀಲಿ, RELEASEBIT, ದತ್ತಾಂಶ->modifiers);
                ಆದರೆ (i+1 >= ARRAY_ಗಾತ್ರ(old.ಕೀಲಿಗಳು) || !old.ಕೀಲಿಗಳು[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ (ದತ್ತಾಂಶ->ಕೀಲಿಗಳು[j] == ಕೀಲಿ) {
                // Key still pressed.
                ದತ್ತಾಂಶ->ಕೀಲಿಗಳು[j] = 0;
                old.ಕೀಲಿಗಳು[addpos++] = ಕೀಲಿ;
                ನಿಲ್ಲಿಸು;
            }
        }
    }
    procmodkey(old.modifiers & ~ದತ್ತಾಂಶ->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(ದತ್ತಾಂಶ->modifiers & ~old.modifiers, 0);
    old.modifiers = ದತ್ತಾಂಶ->modifiers;
    ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(ದತ್ತಾಂಶ->ಕೀಲಿಗಳು); i++) {
        u8 ಕೀಲಿ = ದತ್ತಾಂಶ->ಕೀಲಿಗಳು[i];
        ಆದರೆ (!ಕೀಲಿ)
            ಮುಂದುವರಿ;
        // New key pressed.
        procscankey(ಕೀಲಿ, 0, ದತ್ತಾಂಶ->modifiers);
        old.ಕೀಲಿಗಳು[addpos++] = ಕೀಲಿ;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    ಆದರೆ (addpos < ARRAY_ಗಾತ್ರ(old.ಕೀಲಿಗಳು))
        old.ಕೀಲಿಗಳು[addpos] = 0;

    // Check for key repeat event.
    ಆದರೆ (addpos) {
        ಆದರೆ (!old.repeatcount)
            procscankey(old.ಕೀಲಿಗಳು[addpos-1], 0, ದತ್ತಾಂಶ->modifiers);
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    ಗಣ_ಕೆಳಗಿನ(ಕೊನೆಯ_USBkey.ದತ್ತಾಂಶ, old.ದತ್ತಾಂಶ);
}

// Check if a USB keyboard event is pending and process it if so.
ಸ್ಥಿರ ಶೂನ್ಯ
usb_check_ಕೀಲಿ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_KEYBOARD)
        ಹಿಂತಿರುಗಿ;

    ಗಾಗಿ (ರಚನೆ pipe_ಗಂಟು *ಗಂಟು = GET_GLOBAL(keyboards);
         ಗಂಟು;
         ಗಂಟು = GET_GLOBALFLAT(ಗಂಟು->ಮುಂದಿನ)) {
        ರಚನೆ usb_pipe *pipe = GET_GLOBALFLAT(ಗಂಟು->pipe);

        ಗಾಗಿ (;;) {
            u8 ದತ್ತಾಂಶ[MAX_KBD_EVENT];
            ಪೂರ್ಣಾಂಕ ret = usb_poll_intr(pipe, ದತ್ತಾಂಶ);
            ಆದರೆ (ret)
                ನಿಲ್ಲಿಸು;
            handle_ಕೀಲಿ((ಶೂನ್ಯ*)ದತ್ತಾಂಶ);
        }
    }
}

// Test if USB keyboard is active.
ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
usb_kbd_ಸಕ್ರಿಯ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_KEYBOARD)
        ಹಿಂತಿರುಗಿ 0;

    ಹಿಂತಿರುಗಿ GET_GLOBAL(keyboards) != ಖಾಲಿ;
}

// Handle a ps2 style keyboard command.
ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
usb_kbd_command(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    ಆದರೆ (! CONFIG_USB_KEYBOARD)
        ಹಿಂತಿರುಗಿ -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    ಆಯ್ಕೆ (command) {
    ಸ್ಥಿತಿ ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        ಹಿಂತಿರುಗಿ 0;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
ಸ್ಥಿರ ಶೂನ್ಯ
handle_mouse(ರಚನೆ mouseevent *ದತ್ತಾಂಶ)
{
    dprintf(9, "Got mouse b=%x x=%x y=%x\n", ದತ್ತಾಂಶ->buttons, ದತ್ತಾಂಶ->x, ದತ್ತಾಂಶ->y);

    s8 x = ದತ್ತಾಂಶ->x, y = -ದತ್ತಾಂಶ->y;
    u8 flag = ((ದತ್ತಾಂಶ->buttons & 0x7) | (1<<3)
               | (x & 0x80 ? (1<<4) : 0) | (y & 0x80 ? (1<<5) : 0));
    process_mouse(flag);
    process_mouse(x);
    process_mouse(y);
}

// Check if a USB mouse event is pending and process it if so.
ಸ್ಥಿರ ಶೂನ್ಯ
usb_check_mouse(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_MOUSE)
        ಹಿಂತಿರುಗಿ;

    ಗಾಗಿ (ರಚನೆ pipe_ಗಂಟು *ಗಂಟು = GET_GLOBAL(mice);
         ಗಂಟು;
         ಗಂಟು = GET_GLOBALFLAT(ಗಂಟು->ಮುಂದಿನ)) {
        ರಚನೆ usb_pipe *pipe = GET_GLOBALFLAT(ಗಂಟು->pipe);

        ಗಾಗಿ (;;) {
            u8 ದತ್ತಾಂಶ[MAX_MOUSE_EVENT];
            ಪೂರ್ಣಾಂಕ ret = usb_poll_intr(pipe, ದತ್ತಾಂಶ);
            ಆದರೆ (ret)
                ನಿಲ್ಲಿಸು;
            handle_mouse((ಶೂನ್ಯ*)ದತ್ತಾಂಶ);
        }
    }
}

// Test if USB mouse is active.
ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
usb_mouse_ಸಕ್ರಿಯ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_MOUSE)
        ಹಿಂತಿರುಗಿ 0;

    ಹಿಂತಿರುಗಿ GET_GLOBAL(mice) != ಖಾಲಿ;
}

// Handle a ps2 style mouse command.
ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
usb_mouse_command(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    ಆದರೆ (! CONFIG_USB_MOUSE)
        ಹಿಂತಿರುಗಿ -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    ಆಯ್ಕೆ (command) {
    ಸ್ಥಿತಿ PSMOUSE_CMD_ENABLE:
    ಸ್ಥಿತಿ PSMOUSE_CMD_DISABLE:
    ಸ್ಥಿತಿ PSMOUSE_CMD_SETSCALE11:
        ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ PSMOUSE_CMD_SETSCALE21:
    ಸ್ಥಿತಿ PSMOUSE_CMD_SETRATE:
    ಸ್ಥಿತಿ PSMOUSE_CMD_SETRES:
        // XXX
        ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ PSMOUSE_CMD_RESET_BAT:
    ಸ್ಥಿತಿ PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        ಹಿಂತಿರುಗಿ 0;

    ಸ್ಥಿತಿ PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        ಹಿಂತಿರುಗಿ 0;

    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
ಶೂನ್ಯ
usb_check_event(ಶೂನ್ಯ)
{
    usb_check_ಕೀಲಿ();
    usb_check_mouse();
}
