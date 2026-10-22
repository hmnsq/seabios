// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "output.h" // dprintf, warn_noalloc
#अन्तर्भाव "malloc.h" // malloc_fseg
#अन्तर्भाव "ps2port.h" // ATKBD_CMD_GETID
#अन्तर्भाव "usb.h" // usb_ctrlrequest
#अन्तर्भाव "usb-hid.h" // usb_keyboard_setup
#अन्तर्भाव "util.h" // process_key

संरचना pipe_गाठ {
    संरचना usb_pipe *pipe;
    संरचना pipe_गाठ *next;
};

संरचना pipe_गाठ *keyboards VARFSEG = रिक्त;
संरचना pipe_गाठ *mice VARFSEG = रिक्त;

स्थिर पूर्णाङ्क
add_pipe_गाठ(संरचना pipe_गाठ **list
              , संरचना usbdevice_s *usbdev
              , संरचना usb_endpoint_descriptor *epdesc)
{
    संरचना usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    यदि (!pipe)
        प्रत्यागम -1;

    संरचना pipe_गाठ *new_गाठ = malloc_fseg(परिमाण(संरचना pipe_गाठ));
    यदि (!new_गाठ) {
        warn_noalloc();
        प्रत्यागम -1;
    }

    new_गाठ->pipe = pipe;

    new_गाठ->next = *list;
    *list = new_गाठ;

    प्रत्यागम 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
स्थिर पूर्णाङ्क
set_protocol(संरचना usb_pipe *pipe, u16 val, u16 inferface)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.w_मूल्य = val;
    req.wIndex = inferface;
    req.wLength = 0;
    प्रत्यागम usb_send_default_control(pipe, &req, रिक्त);
}

// Send USB HID SetIdle request.
स्थिर पूर्णाङ्क
set_idle(संरचना usb_pipe *pipe, पूर्णाङ्क ms)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.w_मूल्य = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    प्रत्यागम usb_send_default_control(pipe, &req, रिक्त);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
संरचना keyevent {
    u8 modifiers;
    u8 reserved;
    u8 किल्ल्या[6];
};

#define MAX_KBD_EVENT 16

स्थिर पूर्णाङ्क
usb_kbd_setup(संरचना usbdevice_s *usbdev
              , संरचना usb_endpoint_descriptor *epdesc)
{
    यदि (! CONFIG_USB_KEYBOARD)
        प्रत्यागम -1;

    यदि (epdesc->wMaxPacketSize < परिमाण(संरचना keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        प्रत्यागम -1;
    }

    // Enable "boot" protocol.
    यदि (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_संख्या)) {
        dprintf(3, "Failed to set boot protocol\n");
        प्रत्यागम -1;
    }

    // Periodically send reports to enable key repeat.
    यदि (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    यदि (add_pipe_गाठ(&keyboards, usbdev, epdesc))
        प्रत्यागम -1;

    dprintf(1, "USB keyboard initialized\n");
    प्रत्यागम 0;
}

// Format of USB mouse event data
संरचना mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

स्थिर पूर्णाङ्क
usb_mouse_setup(संरचना usbdevice_s *usbdev
                , संरचना usb_endpoint_descriptor *epdesc)
{
    यदि (! CONFIG_USB_MOUSE)
        प्रत्यागम -1;

    यदि (epdesc->wMaxPacketSize < परिमाण(संरचना mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        प्रत्यागम -1;
    }

    // Enable "boot" protocol.
    यदि (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_संख्या))
        प्रत्यागम -1;

    यदि (add_pipe_गाठ(&mice, usbdev, epdesc))
        प्रत्यागम -1;

    dprintf(1, "USB mouse initialized\n");
    प्रत्यागम 0;
}

// Initialize a found USB HID device (if applicable).
पूर्णाङ्क
usb_hid_setup(संरचना usbdevice_s *usbdev)
{
    यदि (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        प्रत्यागम -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    संरचना usb_interface_descriptor *iface = usbdev->iface;
    यदि (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        प्रत्यागम -1;

    // Find intr in endpoint.
    संरचना usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    यदि (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        प्रत्यागम -1;
    }

    यदि (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        प्रत्यागम usb_kbd_setup(usbdev, epdesc);
    यदि (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        प्रत्यागम usb_mouse_setup(usbdev, epdesc);
    प्रत्यागम -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
स्थिर u16 किल्ली_To_Scan_Code[] VAR16 = {
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
स्थिर u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
स्थिर शून्य
prockeys(u16 scancode, u8 किल्ली_release, u8 mods)
{
    यदि (scancode > 0xff) {
        यदि (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            यदि (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_किल्ली(0xe0);
                process_किल्ली(0x46 | किल्ली_release);
            } अन्यथा {
                // Pause key
                process_किल्ली(0xe1);
                process_किल्ली(0x1d | किल्ली_release);
                process_किल्ली(0x45 | किल्ली_release);
            }
            प्रत्यागम;
        } अन्यथा यदि (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_किल्ली(0x54 | किल्ली_release);
            प्रत्यागम;
        }
        process_किल्ली(0xe0);
    }
    process_किल्ली(scancode | किल्ली_release);
}

// Handle a USB key press/release event.
स्थिर शून्य
procscankey(u8 किल्ली, u8 किल्ली_release, u8 mods)
{
    यदि (किल्ली >= ARRAY_SIZE(किल्ली_To_Scan_Code))
        प्रत्यागम;
    u16 scancode = GET_GLOBAL(किल्ली_To_Scan_Code[किल्ली]);
    यदि (scancode)
        prockeys(scancode, किल्ली_release, mods);
}

// Handle a USB modifier press/release event.
स्थिर शून्य
procmodkey(u8 mods, u8 किल्ली_release)
{
    पूर्णाङ्क i;
    प्रति (i=0; mods; i++)
        यदि (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), किल्ली_release, 0);
            mods &= ~(1<<i);
        }
}

संरचना usbkeyinfo {
    सङ्घ {
        संरचना {
            u8 modifiers;
            u8 repeatcount;
            u8 किल्ल्या[6];
        };
        u64 data;
    };
};
संरचना usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
स्थिर शून्य
handle_किल्ली(संरचना keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->किल्ल्या[0]);

    // Load old keys.
    संरचना usbkeyinfo old;
    old.data = GET_खालचा(LastUSBkey.data);

    // Check for keys no longer pressed.
    पूर्णाङ्क addpos = 0;
    पूर्णाङ्क i;
    प्रति (i=0; i<ARRAY_SIZE(old.किल्ल्या); i++) {
        u8 किल्ली = old.किल्ल्या[i];
        यदि (!किल्ली)
            भङ्ग;
        पूर्णाङ्क j;
        प्रति (j=0;; j++) {
            यदि (j>=ARRAY_SIZE(data->किल्ल्या)) {
                // Key released.
                procscankey(किल्ली, RELEASEBIT, data->modifiers);
                यदि (i+1 >= ARRAY_SIZE(old.किल्ल्या) || !old.किल्ल्या[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                भङ्ग;
            }
            यदि (data->किल्ल्या[j] == किल्ली) {
                // Key still pressed.
                data->किल्ल्या[j] = 0;
                old.किल्ल्या[addpos++] = किल्ली;
                भङ्ग;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    प्रति (i=0; i<ARRAY_SIZE(data->किल्ल्या); i++) {
        u8 किल्ली = data->किल्ल्या[i];
        यदि (!किल्ली)
            अनुवर्त;
        // New key pressed.
        procscankey(किल्ली, 0, data->modifiers);
        old.किल्ल्या[addpos++] = किल्ली;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    यदि (addpos < ARRAY_SIZE(old.किल्ल्या))
        old.किल्ल्या[addpos] = 0;

    // Check for key repeat event.
    यदि (addpos) {
        यदि (!old.repeatcount)
            procscankey(old.किल्ल्या[addpos-1], 0, data->modifiers);
        अन्यथा यदि (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_खालचा(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
स्थिर शून्य
usb_check_किल्ली(शून्य)
{
    यदि (! CONFIG_USB_KEYBOARD)
        प्रत्यागम;

    प्रति (संरचना pipe_गाठ *गाठ = GET_GLOBAL(keyboards);
         गाठ;
         गाठ = GET_GLOBALFLAT(गाठ->next)) {
        संरचना usb_pipe *pipe = GET_GLOBALFLAT(गाठ->pipe);

        प्रति (;;) {
            u8 data[MAX_KBD_EVENT];
            पूर्णाङ्क ret = usb_poll_intr(pipe, data);
            यदि (ret)
                भङ्ग;
            handle_किल्ली((शून्य*)data);
        }
    }
}

// Test if USB keyboard is active.
अन्तःस्थ पूर्णाङ्क
usb_kbd_active(शून्य)
{
    यदि (! CONFIG_USB_KEYBOARD)
        प्रत्यागम 0;

    प्रत्यागम GET_GLOBAL(keyboards) != रिक्त;
}

// Handle a ps2 style keyboard command.
अन्तःस्थ पूर्णाङ्क
usb_kbd_command(पूर्णाङ्क command, u8 *param)
{
    यदि (! CONFIG_USB_KEYBOARD)
        प्रत्यागम -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    विकल्प (command) {
    पक्ष ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        प्रत्यागम 0;
    स्वभाव:
        प्रत्यागम -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
स्थिर शून्य
handle_mouse(संरचना mouseevent *data)
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
स्थिर शून्य
usb_check_mouse(शून्य)
{
    यदि (! CONFIG_USB_MOUSE)
        प्रत्यागम;

    प्रति (संरचना pipe_गाठ *गाठ = GET_GLOBAL(mice);
         गाठ;
         गाठ = GET_GLOBALFLAT(गाठ->next)) {
        संरचना usb_pipe *pipe = GET_GLOBALFLAT(गाठ->pipe);

        प्रति (;;) {
            u8 data[MAX_MOUSE_EVENT];
            पूर्णाङ्क ret = usb_poll_intr(pipe, data);
            यदि (ret)
                भङ्ग;
            handle_mouse((शून्य*)data);
        }
    }
}

// Test if USB mouse is active.
अन्तःस्थ पूर्णाङ्क
usb_mouse_active(शून्य)
{
    यदि (! CONFIG_USB_MOUSE)
        प्रत्यागम 0;

    प्रत्यागम GET_GLOBAL(mice) != रिक्त;
}

// Handle a ps2 style mouse command.
अन्तःस्थ पूर्णाङ्क
usb_mouse_command(पूर्णाङ्क command, u8 *param)
{
    यदि (! CONFIG_USB_MOUSE)
        प्रत्यागम -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    विकल्प (command) {
    पक्ष PSMOUSE_CMD_ENABLE:
    पक्ष PSMOUSE_CMD_DISABLE:
    पक्ष PSMOUSE_CMD_SETSCALE11:
        प्रत्यागम 0;
    पक्ष PSMOUSE_CMD_SETSCALE21:
    पक्ष PSMOUSE_CMD_SETRATE:
    पक्ष PSMOUSE_CMD_SETRES:
        // XXX
        प्रत्यागम 0;
    पक्ष PSMOUSE_CMD_RESET_BAT:
    पक्ष PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        प्रत्यागम 0;

    पक्ष PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        प्रत्यागम 0;

    स्वभाव:
        प्रत्यागम -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
शून्य
usb_check_event(शून्य)
{
    usb_check_किल्ली();
    usb_check_mouse();
}
