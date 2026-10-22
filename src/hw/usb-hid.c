// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "output.h" // dprintf, warn_noalloc
#അന്തര്ഭാവ "malloc.h" // malloc_fseg
#അന്തര്ഭാവ "ps2port.h" // ATKBD_CMD_GETID
#അന്തര്ഭാവ "usb.h" // usb_ctrlrequest
#അന്തര്ഭാവ "usb-hid.h" // usb_keyboard_setup
#അന്തര്ഭാവ "util.h" // process_key

ഘടന pipe_കണ്ണി {
    ഘടന usb_pipe *pipe;
    ഘടന pipe_കണ്ണി *next;
};

ഘടന pipe_കണ്ണി *keyboards VARFSEG = ശൂന്യ;
ഘടന pipe_കണ്ണി *mice VARFSEG = ശൂന്യ;

സ്ഥിതം പൂർണ്ണസംഖ്യ
add_pipe_കണ്ണി(ഘടന pipe_കണ്ണി **list
              , ഘടന usbdevice_s *usbdev
              , ഘടന usb_endpoint_descriptor *epdesc)
{
    ഘടന usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    എങ്കിൽ (!pipe)
        തിരികെ -1;

    ഘടന pipe_കണ്ണി *new_കണ്ണി = malloc_fseg(വലുപ്പം(ഘടന pipe_കണ്ണി));
    എങ്കിൽ (!new_കണ്ണി) {
        warn_noalloc();
        തിരികെ -1;
    }

    new_കണ്ണി->pipe = pipe;

    new_കണ്ണി->next = *list;
    *list = new_കണ്ണി;

    തിരികെ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
സ്ഥിതം പൂർണ്ണസംഖ്യ
set_protocol(ഘടന usb_pipe *pipe, u16 val, u16 inferface)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.w_മൂല്യം = val;
    req.wIndex = inferface;
    req.wLength = 0;
    തിരികെ usb_send_default_control(pipe, &req, ശൂന്യ);
}

// Send USB HID SetIdle request.
സ്ഥിതം പൂർണ്ണസംഖ്യ
set_idle(ഘടന usb_pipe *pipe, പൂർണ്ണസംഖ്യ ms)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.w_മൂല്യം = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    തിരികെ usb_send_default_control(pipe, &req, ശൂന്യ);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
ഘടന keyevent {
    u8 modifiers;
    u8 reserved;
    u8 താക്കോലുകൾ[6];
};

#define MAX_KBD_EVENT 16

സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_kbd_setup(ഘടന usbdevice_s *usbdev
              , ഘടന usb_endpoint_descriptor *epdesc)
{
    എങ്കിൽ (! CONFIG_USB_KEYBOARD)
        തിരികെ -1;

    എങ്കിൽ (epdesc->wMaxPacketSize < വലുപ്പം(ഘടന keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        തിരികെ -1;
    }

    // Enable "boot" protocol.
    എങ്കിൽ (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_സംഖ്യ)) {
        dprintf(3, "Failed to set boot protocol\n");
        തിരികെ -1;
    }

    // Periodically send reports to enable key repeat.
    എങ്കിൽ (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    എങ്കിൽ (add_pipe_കണ്ണി(&keyboards, usbdev, epdesc))
        തിരികെ -1;

    dprintf(1, "USB keyboard initialized\n");
    തിരികെ 0;
}

// Format of USB mouse event data
ഘടന mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_mouse_setup(ഘടന usbdevice_s *usbdev
                , ഘടന usb_endpoint_descriptor *epdesc)
{
    എങ്കിൽ (! CONFIG_USB_MOUSE)
        തിരികെ -1;

    എങ്കിൽ (epdesc->wMaxPacketSize < വലുപ്പം(ഘടന mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        തിരികെ -1;
    }

    // Enable "boot" protocol.
    എങ്കിൽ (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_സംഖ്യ))
        തിരികെ -1;

    എങ്കിൽ (add_pipe_കണ്ണി(&mice, usbdev, epdesc))
        തിരികെ -1;

    dprintf(1, "USB mouse initialized\n");
    തിരികെ 0;
}

// Initialize a found USB HID device (if applicable).
പൂർണ്ണസംഖ്യ
usb_hid_setup(ഘടന usbdevice_s *usbdev)
{
    എങ്കിൽ (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        തിരികെ -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    ഘടന usb_interface_descriptor *iface = usbdev->iface;
    എങ്കിൽ (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        തിരികെ -1;

    // Find intr in endpoint.
    ഘടന usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    എങ്കിൽ (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        തിരികെ -1;
    }

    എങ്കിൽ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        തിരികെ usb_kbd_setup(usbdev, epdesc);
    എങ്കിൽ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        തിരികെ usb_mouse_setup(usbdev, epdesc);
    തിരികെ -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
സ്ഥിതം u16 താക്കോൽ_To_Scan_Code[] VAR16 = {
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
സ്ഥിതം u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
സ്ഥിതം ശൂന്യം
prockeys(u16 scancode, u8 താക്കോൽ_release, u8 mods)
{
    എങ്കിൽ (scancode > 0xff) {
        എങ്കിൽ (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            എങ്കിൽ (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_താക്കോൽ(0xe0);
                process_താക്കോൽ(0x46 | താക്കോൽ_release);
            } അല്ലെങ്കിൽ {
                // Pause key
                process_താക്കോൽ(0xe1);
                process_താക്കോൽ(0x1d | താക്കോൽ_release);
                process_താക്കോൽ(0x45 | താക്കോൽ_release);
            }
            തിരികെ;
        } അല്ലെങ്കിൽ എങ്കിൽ (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_താക്കോൽ(0x54 | താക്കോൽ_release);
            തിരികെ;
        }
        process_താക്കോൽ(0xe0);
    }
    process_താക്കോൽ(scancode | താക്കോൽ_release);
}

// Handle a USB key press/release event.
സ്ഥിതം ശൂന്യം
procscankey(u8 താക്കോൽ, u8 താക്കോൽ_release, u8 mods)
{
    എങ്കിൽ (താക്കോൽ >= ARRAY_SIZE(താക്കോൽ_To_Scan_Code))
        തിരികെ;
    u16 scancode = GET_GLOBAL(താക്കോൽ_To_Scan_Code[താക്കോൽ]);
    എങ്കിൽ (scancode)
        prockeys(scancode, താക്കോൽ_release, mods);
}

// Handle a USB modifier press/release event.
സ്ഥിതം ശൂന്യം
procmodkey(u8 mods, u8 താക്കോൽ_release)
{
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; mods; i++)
        എങ്കിൽ (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), താക്കോൽ_release, 0);
            mods &= ~(1<<i);
        }
}

ഘടന usbkeyinfo {
    സംഘം {
        ഘടന {
            u8 modifiers;
            u8 repeatcount;
            u8 താക്കോലുകൾ[6];
        };
        u64 data;
    };
};
ഘടന usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
സ്ഥിതം ശൂന്യം
handle_താക്കോൽ(ഘടന keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->താക്കോലുകൾ[0]);

    // Load old keys.
    ഘടന usbkeyinfo old;
    old.data = GET_താഴെയുള്ള(LastUSBkey.data);

    // Check for keys no longer pressed.
    പൂർണ്ണസംഖ്യ addpos = 0;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<ARRAY_SIZE(old.താക്കോലുകൾ); i++) {
        u8 താക്കോൽ = old.താക്കോലുകൾ[i];
        എങ്കിൽ (!താക്കോൽ)
            നിർത്തുക;
        പൂർണ്ണസംഖ്യ j;
        വേണ്ടി (j=0;; j++) {
            എങ്കിൽ (j>=ARRAY_SIZE(data->താക്കോലുകൾ)) {
                // Key released.
                procscankey(താക്കോൽ, RELEASEBIT, data->modifiers);
                എങ്കിൽ (i+1 >= ARRAY_SIZE(old.താക്കോലുകൾ) || !old.താക്കോലുകൾ[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                നിർത്തുക;
            }
            എങ്കിൽ (data->താക്കോലുകൾ[j] == താക്കോൽ) {
                // Key still pressed.
                data->താക്കോലുകൾ[j] = 0;
                old.താക്കോലുകൾ[addpos++] = താക്കോൽ;
                നിർത്തുക;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    വേണ്ടി (i=0; i<ARRAY_SIZE(data->താക്കോലുകൾ); i++) {
        u8 താക്കോൽ = data->താക്കോലുകൾ[i];
        എങ്കിൽ (!താക്കോൽ)
            തുടരുക;
        // New key pressed.
        procscankey(താക്കോൽ, 0, data->modifiers);
        old.താക്കോലുകൾ[addpos++] = താക്കോൽ;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    എങ്കിൽ (addpos < ARRAY_SIZE(old.താക്കോലുകൾ))
        old.താക്കോലുകൾ[addpos] = 0;

    // Check for key repeat event.
    എങ്കിൽ (addpos) {
        എങ്കിൽ (!old.repeatcount)
            procscankey(old.താക്കോലുകൾ[addpos-1], 0, data->modifiers);
        അല്ലെങ്കിൽ എങ്കിൽ (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_താഴെയുള്ള(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
സ്ഥിതം ശൂന്യം
usb_check_താക്കോൽ(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_KEYBOARD)
        തിരികെ;

    വേണ്ടി (ഘടന pipe_കണ്ണി *കണ്ണി = GET_GLOBAL(keyboards);
         കണ്ണി;
         കണ്ണി = GET_GLOBALFLAT(കണ്ണി->next)) {
        ഘടന usb_pipe *pipe = GET_GLOBALFLAT(കണ്ണി->pipe);

        വേണ്ടി (;;) {
            u8 data[MAX_KBD_EVENT];
            പൂർണ്ണസംഖ്യ ret = usb_poll_intr(pipe, data);
            എങ്കിൽ (ret)
                നിർത്തുക;
            handle_താക്കോൽ((ശൂന്യം*)data);
        }
    }
}

// Test if USB keyboard is active.
വരിയിൽ പൂർണ്ണസംഖ്യ
usb_kbd_active(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_KEYBOARD)
        തിരികെ 0;

    തിരികെ GET_GLOBAL(keyboards) != ശൂന്യ;
}

// Handle a ps2 style keyboard command.
വരിയിൽ പൂർണ്ണസംഖ്യ
usb_kbd_command(പൂർണ്ണസംഖ്യ command, u8 *param)
{
    എങ്കിൽ (! CONFIG_USB_KEYBOARD)
        തിരികെ -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    തിരഞ്ഞെടുപ്പ് (command) {
    അവസ്ഥ ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        തിരികെ 0;
    സ്ഥിരസ്ഥിതി:
        തിരികെ -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
സ്ഥിതം ശൂന്യം
handle_mouse(ഘടന mouseevent *data)
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
സ്ഥിതം ശൂന്യം
usb_check_mouse(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_MOUSE)
        തിരികെ;

    വേണ്ടി (ഘടന pipe_കണ്ണി *കണ്ണി = GET_GLOBAL(mice);
         കണ്ണി;
         കണ്ണി = GET_GLOBALFLAT(കണ്ണി->next)) {
        ഘടന usb_pipe *pipe = GET_GLOBALFLAT(കണ്ണി->pipe);

        വേണ്ടി (;;) {
            u8 data[MAX_MOUSE_EVENT];
            പൂർണ്ണസംഖ്യ ret = usb_poll_intr(pipe, data);
            എങ്കിൽ (ret)
                നിർത്തുക;
            handle_mouse((ശൂന്യം*)data);
        }
    }
}

// Test if USB mouse is active.
വരിയിൽ പൂർണ്ണസംഖ്യ
usb_mouse_active(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_MOUSE)
        തിരികെ 0;

    തിരികെ GET_GLOBAL(mice) != ശൂന്യ;
}

// Handle a ps2 style mouse command.
വരിയിൽ പൂർണ്ണസംഖ്യ
usb_mouse_command(പൂർണ്ണസംഖ്യ command, u8 *param)
{
    എങ്കിൽ (! CONFIG_USB_MOUSE)
        തിരികെ -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    തിരഞ്ഞെടുപ്പ് (command) {
    അവസ്ഥ PSMOUSE_CMD_ENABLE:
    അവസ്ഥ PSMOUSE_CMD_DISABLE:
    അവസ്ഥ PSMOUSE_CMD_SETSCALE11:
        തിരികെ 0;
    അവസ്ഥ PSMOUSE_CMD_SETSCALE21:
    അവസ്ഥ PSMOUSE_CMD_SETRATE:
    അവസ്ഥ PSMOUSE_CMD_SETRES:
        // XXX
        തിരികെ 0;
    അവസ്ഥ PSMOUSE_CMD_RESET_BAT:
    അവസ്ഥ PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        തിരികെ 0;

    അവസ്ഥ PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        തിരികെ 0;

    സ്ഥിരസ്ഥിതി:
        തിരികെ -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
ശൂന്യം
usb_check_event(ശൂന്യം)
{
    usb_check_താക്കോൽ();
    usb_check_mouse();
}
