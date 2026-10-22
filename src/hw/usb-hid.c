// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf, warn_noalloc
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_fseg
#ਅਨ੍ਤਰ੍ਭਾਵ "ps2port.h" // ATKBD_CMD_GETID
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // usb_ctrlrequest
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-hid.h" // usb_keyboard_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // process_key

ਬਣਤਰ pipe_ਗੰਢ {
    ਬਣਤਰ usb_pipe *pipe;
    ਬਣਤਰ pipe_ਗੰਢ *ਅਗਲਾ;
};

ਬਣਤਰ pipe_ਗੰਢ *keyboards VARFSEG = ਨਿਰਾ;
ਬਣਤਰ pipe_ਗੰਢ *mice VARFSEG = ਨਿਰਾ;

ਸਥਿਰ ਪੂਰਨ
ਜੋੜੋ_pipe_ਗੰਢ(ਬਣਤਰ pipe_ਗੰਢ **list
              , ਬਣਤਰ usbdevice_s *usbdev
              , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਬਣਤਰ usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    ਜੇ (!pipe)
        ਵਾਪਸ -1;

    ਬਣਤਰ pipe_ਗੰਢ *new_ਗੰਢ = malloc_fseg(ਆਕਾਰ(ਬਣਤਰ pipe_ਗੰਢ));
    ਜੇ (!new_ਗੰਢ) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }

    new_ਗੰਢ->pipe = pipe;

    new_ਗੰਢ->ਅਗਲਾ = *list;
    *list = new_ਗੰਢ;

    ਵਾਪਸ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
ਸਥਿਰ ਪੂਰਨ
ਸੈੱਟ_protocol(ਬਣਤਰ usb_pipe *pipe, u16 val, u16 inferface)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਬਾਹਰਲਾ | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_ਸੈੱਟ_PROTOCOL;
    req.w_ਮੁੱਲ = val;
    req.w_ਸੂਚਕਾਂਕ = inferface;
    req.w_ਲੰਮਾਈ = 0;
    ਵਾਪਸ usb_send_default_control(pipe, &req, ਨਿਰਾ);
}

// Send USB HID SetIdle request.
ਸਥਿਰ ਪੂਰਨ
ਸੈੱਟ_idle(ਬਣਤਰ usb_pipe *pipe, ਪੂਰਨ ms)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਬਾਹਰਲਾ | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_ਸੈੱਟ_IDLE;
    req.w_ਮੁੱਲ = (ms/4)<<8;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = 0;
    ਵਾਪਸ usb_send_default_control(pipe, &req, ਨਿਰਾ);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KEYREPEATWAITMS 500
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KEYREPEATMS 33

// Format of USB keyboard event data
ਬਣਤਰ keyevent {
    u8 modifiers;
    u8 reserved;
    u8 ਕੁੰਜੀਆਂ[6];
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_KBD_EVENT 16

ਸਥਿਰ ਪੂਰਨ
usb_kbd_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev
              , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਜੇ (! CONFIG_USB_KEYBOARD)
        ਵਾਪਸ -1;

    ਜੇ (epdesc->w_Max_Packet_ਮਾਪ < ਆਕਾਰ(ਬਣਤਰ keyevent)
        || epdesc->w_Max_Packet_ਮਾਪ > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_ਮਾਪ);
        ਵਾਪਸ -1;
    }

    // Enable "boot" protocol.
    ਜੇ (ਸੈੱਟ_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_ਸੰਖਿਆ)) {
        dprintf(3, "Failed to set boot protocol\n");
        ਵਾਪਸ -1;
    }

    // Periodically send reports to enable key repeat.
    ਜੇ (ਸੈੱਟ_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    ਜੇ (ਜੋੜੋ_pipe_ਗੰਢ(&keyboards, usbdev, epdesc))
        ਵਾਪਸ -1;

    dprintf(1, "USB keyboard initialized\n");
    ਵਾਪਸ 0;
}

// Format of USB mouse event data
ਬਣਤਰ mouseevent {
    u8 buttons;
    u8 x, y;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_MOUSE_EVENT 8

ਸਥਿਰ ਪੂਰਨ
usb_mouse_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev
                , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਜੇ (! CONFIG_USB_MOUSE)
        ਵਾਪਸ -1;

    ਜੇ (epdesc->w_Max_Packet_ਮਾਪ < ਆਕਾਰ(ਬਣਤਰ mouseevent)
        || epdesc->w_Max_Packet_ਮਾਪ > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_ਮਾਪ);
        ਵਾਪਸ -1;
    }

    // Enable "boot" protocol.
    ਜੇ (ਸੈੱਟ_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_ਸੰਖਿਆ))
        ਵਾਪਸ -1;

    ਜੇ (ਜੋੜੋ_pipe_ਗੰਢ(&mice, usbdev, epdesc))
        ਵਾਪਸ -1;

    dprintf(1, "USB mouse initialized\n");
    ਵਾਪਸ 0;
}

// Initialize a found USB HID device (if applicable).
ਪੂਰਨ
usb_hid_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev)
{
    ਜੇ (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        ਵਾਪਸ -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    ਬਣਤਰ usb_interface_descriptor *iface = usbdev->iface;
    ਜੇ (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        ਵਾਪਸ -1;

    // Find intr in endpoint.
    ਬਣਤਰ usb_endpoint_descriptor *epdesc = usb_ਲੱਭੋ_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_ਵਿੱਚ);
    ਜੇ (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        ਵਾਪਸ -1;
    }

    ਜੇ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        ਵਾਪਸ usb_kbd_ਤਿਆਰੀ(usbdev, epdesc);
    ਜੇ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        ਵਾਪਸ usb_mouse_ਤਿਆਰੀ(usbdev, epdesc);
    ਵਾਪਸ -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
ਸਥਿਰ u16 ਕੁੰਜੀ_ਮੰਜ਼ਿਲ_ਜਾਂਚ_ਕੋਡ[] VAR16 = {
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
ਸਥਿਰ u16 Modifier_ਮੰਜ਼ਿਲ_ਜਾਂਚ_ਕੋਡ[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
ਸਥਿਰ ਖਾਲੀ
prockeys(u16 scancode, u8 ਕੁੰਜੀ_release, u8 mods)
{
    ਜੇ (scancode > 0xff) {
        ਜੇ (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            ਜੇ (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_ਕੁੰਜੀ(0xe0);
                process_ਕੁੰਜੀ(0x46 | ਕੁੰਜੀ_release);
            } ਨਹੀਂਤਾਂ {
                // Pause key
                process_ਕੁੰਜੀ(0xe1);
                process_ਕੁੰਜੀ(0x1d | ਕੁੰਜੀ_release);
                process_ਕੁੰਜੀ(0x45 | ਕੁੰਜੀ_release);
            }
            ਵਾਪਸ;
        } ਨਹੀਂਤਾਂ ਜੇ (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_ਕੁੰਜੀ(0x54 | ਕੁੰਜੀ_release);
            ਵਾਪਸ;
        }
        process_ਕੁੰਜੀ(0xe0);
    }
    process_ਕੁੰਜੀ(scancode | ਕੁੰਜੀ_release);
}

// Handle a USB key press/release event.
ਸਥਿਰ ਖਾਲੀ
procscankey(u8 ਕੁੰਜੀ, u8 ਕੁੰਜੀ_release, u8 mods)
{
    ਜੇ (ਕੁੰਜੀ >= ARRAY_ਮਾਪ(ਕੁੰਜੀ_ਮੰਜ਼ਿਲ_ਜਾਂਚ_ਕੋਡ))
        ਵਾਪਸ;
    u16 scancode = GET_GLOBAL(ਕੁੰਜੀ_ਮੰਜ਼ਿਲ_ਜਾਂਚ_ਕੋਡ[ਕੁੰਜੀ]);
    ਜੇ (scancode)
        prockeys(scancode, ਕੁੰਜੀ_release, mods);
}

// Handle a USB modifier press/release event.
ਸਥਿਰ ਖਾਲੀ
procmodkey(u8 mods, u8 ਕੁੰਜੀ_release)
{
    ਪੂਰਨ i;
    ਲਈ (i=0; mods; i++)
        ਜੇ (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(Modifier_ਮੰਜ਼ਿਲ_ਜਾਂਚ_ਕੋਡ[i]), ਕੁੰਜੀ_release, 0);
            mods &= ~(1<<i);
        }
}

ਬਣਤਰ usbkeyinfo {
    ਸੰਘ {
        ਬਣਤਰ {
            u8 modifiers;
            u8 repeatcount;
            u8 ਕੁੰਜੀਆਂ[6];
        };
        u64 ਅੰਕੜੇ;
    };
};
ਬਣਤਰ usbkeyinfo ਆਖਰੀ_USBkey VARLOW;

// Process USB keyboard data.
ਸਥਿਰ ਖਾਲੀ
handle_ਕੁੰਜੀ(ਬਣਤਰ keyevent *ਅੰਕੜੇ)
{
    dprintf(9, "Got key %x %x\n", ਅੰਕੜੇ->modifiers, ਅੰਕੜੇ->ਕੁੰਜੀਆਂ[0]);

    // Load old keys.
    ਬਣਤਰ usbkeyinfo old;
    old.ਅੰਕੜੇ = GET_ਨੀਵਾਂ(ਆਖਰੀ_USBkey.ਅੰਕੜੇ);

    // Check for keys no longer pressed.
    ਪੂਰਨ addpos = 0;
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ARRAY_ਮਾਪ(old.ਕੁੰਜੀਆਂ); i++) {
        u8 ਕੁੰਜੀ = old.ਕੁੰਜੀਆਂ[i];
        ਜੇ (!ਕੁੰਜੀ)
            ਰੋਕੋ;
        ਪੂਰਨ j;
        ਲਈ (j=0;; j++) {
            ਜੇ (j>=ARRAY_ਮਾਪ(ਅੰਕੜੇ->ਕੁੰਜੀਆਂ)) {
                // Key released.
                procscankey(ਕੁੰਜੀ, RELEASEBIT, ਅੰਕੜੇ->modifiers);
                ਜੇ (i+1 >= ARRAY_ਮਾਪ(old.ਕੁੰਜੀਆਂ) || !old.ਕੁੰਜੀਆਂ[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                ਰੋਕੋ;
            }
            ਜੇ (ਅੰਕੜੇ->ਕੁੰਜੀਆਂ[j] == ਕੁੰਜੀ) {
                // Key still pressed.
                ਅੰਕੜੇ->ਕੁੰਜੀਆਂ[j] = 0;
                old.ਕੁੰਜੀਆਂ[addpos++] = ਕੁੰਜੀ;
                ਰੋਕੋ;
            }
        }
    }
    procmodkey(old.modifiers & ~ਅੰਕੜੇ->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(ਅੰਕੜੇ->modifiers & ~old.modifiers, 0);
    old.modifiers = ਅੰਕੜੇ->modifiers;
    ਲਈ (i=0; i<ARRAY_ਮਾਪ(ਅੰਕੜੇ->ਕੁੰਜੀਆਂ); i++) {
        u8 ਕੁੰਜੀ = ਅੰਕੜੇ->ਕੁੰਜੀਆਂ[i];
        ਜੇ (!ਕੁੰਜੀ)
            ਜਾਰੀ;
        // New key pressed.
        procscankey(ਕੁੰਜੀ, 0, ਅੰਕੜੇ->modifiers);
        old.ਕੁੰਜੀਆਂ[addpos++] = ਕੁੰਜੀ;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    ਜੇ (addpos < ARRAY_ਮਾਪ(old.ਕੁੰਜੀਆਂ))
        old.ਕੁੰਜੀਆਂ[addpos] = 0;

    // Check for key repeat event.
    ਜੇ (addpos) {
        ਜੇ (!old.repeatcount)
            procscankey(old.ਕੁੰਜੀਆਂ[addpos-1], 0, ਅੰਕੜੇ->modifiers);
        ਨਹੀਂਤਾਂ ਜੇ (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    ਸੈੱਟ_ਨੀਵਾਂ(ਆਖਰੀ_USBkey.ਅੰਕੜੇ, old.ਅੰਕੜੇ);
}

// Check if a USB keyboard event is pending and process it if so.
ਸਥਿਰ ਖਾਲੀ
usb_check_ਕੁੰਜੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_KEYBOARD)
        ਵਾਪਸ;

    ਲਈ (ਬਣਤਰ pipe_ਗੰਢ *ਗੰਢ = GET_GLOBAL(keyboards);
         ਗੰਢ;
         ਗੰਢ = GET_GLOBALFLAT(ਗੰਢ->ਅਗਲਾ)) {
        ਬਣਤਰ usb_pipe *pipe = GET_GLOBALFLAT(ਗੰਢ->pipe);

        ਲਈ (;;) {
            u8 ਅੰਕੜੇ[MAX_KBD_EVENT];
            ਪੂਰਨ ret = usb_poll_intr(pipe, ਅੰਕੜੇ);
            ਜੇ (ret)
                ਰੋਕੋ;
            handle_ਕੁੰਜੀ((ਖਾਲੀ*)ਅੰਕੜੇ);
        }
    }
}

// Test if USB keyboard is active.
ਸਤਰਅੰਦਰ ਪੂਰਨ
usb_kbd_ਸਰਗਰਮ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_KEYBOARD)
        ਵਾਪਸ 0;

    ਵਾਪਸ GET_GLOBAL(keyboards) != ਨਿਰਾ;
}

// Handle a ps2 style keyboard command.
ਸਤਰਅੰਦਰ ਪੂਰਨ
usb_kbd_command(ਪੂਰਨ command, u8 *param)
{
    ਜੇ (! CONFIG_USB_KEYBOARD)
        ਵਾਪਸ -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    ਚੋਣ (command) {
    ਹਾਲਤ ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        ਵਾਪਸ 0;
    ਮੂਲ:
        ਵਾਪਸ -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
ਸਥਿਰ ਖਾਲੀ
handle_mouse(ਬਣਤਰ mouseevent *ਅੰਕੜੇ)
{
    dprintf(9, "Got mouse b=%x x=%x y=%x\n", ਅੰਕੜੇ->buttons, ਅੰਕੜੇ->x, ਅੰਕੜੇ->y);

    s8 x = ਅੰਕੜੇ->x, y = -ਅੰਕੜੇ->y;
    u8 flag = ((ਅੰਕੜੇ->buttons & 0x7) | (1<<3)
               | (x & 0x80 ? (1<<4) : 0) | (y & 0x80 ? (1<<5) : 0));
    process_mouse(flag);
    process_mouse(x);
    process_mouse(y);
}

// Check if a USB mouse event is pending and process it if so.
ਸਥਿਰ ਖਾਲੀ
usb_check_mouse(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_MOUSE)
        ਵਾਪਸ;

    ਲਈ (ਬਣਤਰ pipe_ਗੰਢ *ਗੰਢ = GET_GLOBAL(mice);
         ਗੰਢ;
         ਗੰਢ = GET_GLOBALFLAT(ਗੰਢ->ਅਗਲਾ)) {
        ਬਣਤਰ usb_pipe *pipe = GET_GLOBALFLAT(ਗੰਢ->pipe);

        ਲਈ (;;) {
            u8 ਅੰਕੜੇ[MAX_MOUSE_EVENT];
            ਪੂਰਨ ret = usb_poll_intr(pipe, ਅੰਕੜੇ);
            ਜੇ (ret)
                ਰੋਕੋ;
            handle_mouse((ਖਾਲੀ*)ਅੰਕੜੇ);
        }
    }
}

// Test if USB mouse is active.
ਸਤਰਅੰਦਰ ਪੂਰਨ
usb_mouse_ਸਰਗਰਮ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_MOUSE)
        ਵਾਪਸ 0;

    ਵਾਪਸ GET_GLOBAL(mice) != ਨਿਰਾ;
}

// Handle a ps2 style mouse command.
ਸਤਰਅੰਦਰ ਪੂਰਨ
usb_mouse_command(ਪੂਰਨ command, u8 *param)
{
    ਜੇ (! CONFIG_USB_MOUSE)
        ਵਾਪਸ -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    ਚੋਣ (command) {
    ਹਾਲਤ PSMOUSE_CMD_ENABLE:
    ਹਾਲਤ PSMOUSE_CMD_DISABLE:
    ਹਾਲਤ PSMOUSE_CMD_SETSCALE11:
        ਵਾਪਸ 0;
    ਹਾਲਤ PSMOUSE_CMD_SETSCALE21:
    ਹਾਲਤ PSMOUSE_CMD_SETRATE:
    ਹਾਲਤ PSMOUSE_CMD_SETRES:
        // XXX
        ਵਾਪਸ 0;
    ਹਾਲਤ PSMOUSE_CMD_RESET_BAT:
    ਹਾਲਤ PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        ਵਾਪਸ 0;

    ਹਾਲਤ PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        ਵਾਪਸ 0;

    ਮੂਲ:
        ਵਾਪਸ -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
ਖਾਲੀ
usb_check_event(ਖਾਲੀ)
{
    usb_check_ਕੁੰਜੀ();
    usb_check_mouse();
}
