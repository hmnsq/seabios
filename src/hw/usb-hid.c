// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "output.h" // dprintf, warn_noalloc
#அந்தர்பாவ "malloc.h" // malloc_fseg
#அந்தர்பாவ "ps2port.h" // ATKBD_CMD_GETID
#அந்தர்பாவ "usb.h" // usb_ctrlrequest
#அந்தர்பாவ "usb-hid.h" // usb_keyboard_setup
#அந்தர்பாவ "util.h" // process_key

கட்டமைப்பு pipe_முனை {
    கட்டமைப்பு usb_pipe *pipe;
    கட்டமைப்பு pipe_முனை *next;
};

கட்டமைப்பு pipe_முனை *keyboards VARFSEG = வெற்று;
கட்டமைப்பு pipe_முனை *mice VARFSEG = வெற்று;

நிலையான முழு
add_pipe_முனை(கட்டமைப்பு pipe_முனை **list
              , கட்டமைப்பு usbdevice_s *usbdev
              , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    கட்டமைப்பு usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    எனில் (!pipe)
        திரும்பு -1;

    கட்டமைப்பு pipe_முனை *new_முனை = malloc_fseg(அளவு(கட்டமைப்பு pipe_முனை));
    எனில் (!new_முனை) {
        warn_noalloc();
        திரும்பு -1;
    }

    new_முனை->pipe = pipe;

    new_முனை->next = *list;
    *list = new_முனை;

    திரும்பு 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
நிலையான முழு
set_protocol(கட்டமைப்பு usb_pipe *pipe, u16 val, u16 inferface)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.w_மதிப்பு = val;
    req.wIndex = inferface;
    req.wLength = 0;
    திரும்பு usb_send_default_control(pipe, &req, வெற்று);
}

// Send USB HID SetIdle request.
நிலையான முழு
set_idle(கட்டமைப்பு usb_pipe *pipe, முழு ms)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.w_மதிப்பு = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    திரும்பு usb_send_default_control(pipe, &req, வெற்று);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
கட்டமைப்பு keyevent {
    u8 modifiers;
    u8 reserved;
    u8 சாவிகள்[6];
};

#define MAX_KBD_EVENT 16

நிலையான முழு
usb_kbd_setup(கட்டமைப்பு usbdevice_s *usbdev
              , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    எனில் (! CONFIG_USB_KEYBOARD)
        திரும்பு -1;

    எனில் (epdesc->wMaxPacketSize < அளவு(கட்டமைப்பு keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        திரும்பு -1;
    }

    // Enable "boot" protocol.
    எனில் (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_எண்)) {
        dprintf(3, "Failed to set boot protocol\n");
        திரும்பு -1;
    }

    // Periodically send reports to enable key repeat.
    எனில் (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    எனில் (add_pipe_முனை(&keyboards, usbdev, epdesc))
        திரும்பு -1;

    dprintf(1, "USB keyboard initialized\n");
    திரும்பு 0;
}

// Format of USB mouse event data
கட்டமைப்பு mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

நிலையான முழு
usb_mouse_setup(கட்டமைப்பு usbdevice_s *usbdev
                , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    எனில் (! CONFIG_USB_MOUSE)
        திரும்பு -1;

    எனில் (epdesc->wMaxPacketSize < அளவு(கட்டமைப்பு mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        திரும்பு -1;
    }

    // Enable "boot" protocol.
    எனில் (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_எண்))
        திரும்பு -1;

    எனில் (add_pipe_முனை(&mice, usbdev, epdesc))
        திரும்பு -1;

    dprintf(1, "USB mouse initialized\n");
    திரும்பு 0;
}

// Initialize a found USB HID device (if applicable).
முழு
usb_hid_setup(கட்டமைப்பு usbdevice_s *usbdev)
{
    எனில் (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        திரும்பு -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    கட்டமைப்பு usb_interface_descriptor *iface = usbdev->iface;
    எனில் (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        திரும்பு -1;

    // Find intr in endpoint.
    கட்டமைப்பு usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    எனில் (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        திரும்பு -1;
    }

    எனில் (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        திரும்பு usb_kbd_setup(usbdev, epdesc);
    எனில் (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        திரும்பு usb_mouse_setup(usbdev, epdesc);
    திரும்பு -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
நிலையான u16 சாவி_To_Scan_Code[] VAR16 = {
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
நிலையான u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
நிலையான வெறுமை
prockeys(u16 scancode, u8 சாவி_release, u8 mods)
{
    எனில் (scancode > 0xff) {
        எனில் (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            எனில் (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_சாவி(0xe0);
                process_சாவி(0x46 | சாவி_release);
            } இல்லையெனில் {
                // Pause key
                process_சாவி(0xe1);
                process_சாவி(0x1d | சாவி_release);
                process_சாவி(0x45 | சாவி_release);
            }
            திரும்பு;
        } இல்லையெனில் எனில் (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_சாவி(0x54 | சாவி_release);
            திரும்பு;
        }
        process_சாவி(0xe0);
    }
    process_சாவி(scancode | சாவி_release);
}

// Handle a USB key press/release event.
நிலையான வெறுமை
procscankey(u8 சாவி, u8 சாவி_release, u8 mods)
{
    எனில் (சாவி >= ARRAY_SIZE(சாவி_To_Scan_Code))
        திரும்பு;
    u16 scancode = GET_GLOBAL(சாவி_To_Scan_Code[சாவி]);
    எனில் (scancode)
        prockeys(scancode, சாவி_release, mods);
}

// Handle a USB modifier press/release event.
நிலையான வெறுமை
procmodkey(u8 mods, u8 சாவி_release)
{
    முழு i;
    க்கு (i=0; mods; i++)
        எனில் (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), சாவி_release, 0);
            mods &= ~(1<<i);
        }
}

கட்டமைப்பு usbkeyinfo {
    ஒன்றிப்பு {
        கட்டமைப்பு {
            u8 modifiers;
            u8 repeatcount;
            u8 சாவிகள்[6];
        };
        u64 data;
    };
};
கட்டமைப்பு usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
நிலையான வெறுமை
handle_சாவி(கட்டமைப்பு keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->சாவிகள்[0]);

    // Load old keys.
    கட்டமைப்பு usbkeyinfo old;
    old.data = GET_கீழ்(LastUSBkey.data);

    // Check for keys no longer pressed.
    முழு addpos = 0;
    முழு i;
    க்கு (i=0; i<ARRAY_SIZE(old.சாவிகள்); i++) {
        u8 சாவி = old.சாவிகள்[i];
        எனில் (!சாவி)
            நிறுத்து;
        முழு j;
        க்கு (j=0;; j++) {
            எனில் (j>=ARRAY_SIZE(data->சாவிகள்)) {
                // Key released.
                procscankey(சாவி, RELEASEBIT, data->modifiers);
                எனில் (i+1 >= ARRAY_SIZE(old.சாவிகள்) || !old.சாவிகள்[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                நிறுத்து;
            }
            எனில் (data->சாவிகள்[j] == சாவி) {
                // Key still pressed.
                data->சாவிகள்[j] = 0;
                old.சாவிகள்[addpos++] = சாவி;
                நிறுத்து;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    க்கு (i=0; i<ARRAY_SIZE(data->சாவிகள்); i++) {
        u8 சாவி = data->சாவிகள்[i];
        எனில் (!சாவி)
            தொடர்;
        // New key pressed.
        procscankey(சாவி, 0, data->modifiers);
        old.சாவிகள்[addpos++] = சாவி;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    எனில் (addpos < ARRAY_SIZE(old.சாவிகள்))
        old.சாவிகள்[addpos] = 0;

    // Check for key repeat event.
    எனில் (addpos) {
        எனில் (!old.repeatcount)
            procscankey(old.சாவிகள்[addpos-1], 0, data->modifiers);
        இல்லையெனில் எனில் (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_கீழ்(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
நிலையான வெறுமை
usb_check_சாவி(வெறுமை)
{
    எனில் (! CONFIG_USB_KEYBOARD)
        திரும்பு;

    க்கு (கட்டமைப்பு pipe_முனை *முனை = GET_GLOBAL(keyboards);
         முனை;
         முனை = GET_GLOBALFLAT(முனை->next)) {
        கட்டமைப்பு usb_pipe *pipe = GET_GLOBALFLAT(முனை->pipe);

        க்கு (;;) {
            u8 data[MAX_KBD_EVENT];
            முழு ret = usb_poll_intr(pipe, data);
            எனில் (ret)
                நிறுத்து;
            handle_சாவி((வெறுமை*)data);
        }
    }
}

// Test if USB keyboard is active.
உள்வரி முழு
usb_kbd_active(வெறுமை)
{
    எனில் (! CONFIG_USB_KEYBOARD)
        திரும்பு 0;

    திரும்பு GET_GLOBAL(keyboards) != வெற்று;
}

// Handle a ps2 style keyboard command.
உள்வரி முழு
usb_kbd_command(முழு command, u8 *param)
{
    எனில் (! CONFIG_USB_KEYBOARD)
        திரும்பு -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    தேர்வு (command) {
    நிலை ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        திரும்பு 0;
    இயல்பு:
        திரும்பு -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
நிலையான வெறுமை
handle_mouse(கட்டமைப்பு mouseevent *data)
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
நிலையான வெறுமை
usb_check_mouse(வெறுமை)
{
    எனில் (! CONFIG_USB_MOUSE)
        திரும்பு;

    க்கு (கட்டமைப்பு pipe_முனை *முனை = GET_GLOBAL(mice);
         முனை;
         முனை = GET_GLOBALFLAT(முனை->next)) {
        கட்டமைப்பு usb_pipe *pipe = GET_GLOBALFLAT(முனை->pipe);

        க்கு (;;) {
            u8 data[MAX_MOUSE_EVENT];
            முழு ret = usb_poll_intr(pipe, data);
            எனில் (ret)
                நிறுத்து;
            handle_mouse((வெறுமை*)data);
        }
    }
}

// Test if USB mouse is active.
உள்வரி முழு
usb_mouse_active(வெறுமை)
{
    எனில் (! CONFIG_USB_MOUSE)
        திரும்பு 0;

    திரும்பு GET_GLOBAL(mice) != வெற்று;
}

// Handle a ps2 style mouse command.
உள்வரி முழு
usb_mouse_command(முழு command, u8 *param)
{
    எனில் (! CONFIG_USB_MOUSE)
        திரும்பு -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    தேர்வு (command) {
    நிலை PSMOUSE_CMD_ENABLE:
    நிலை PSMOUSE_CMD_DISABLE:
    நிலை PSMOUSE_CMD_SETSCALE11:
        திரும்பு 0;
    நிலை PSMOUSE_CMD_SETSCALE21:
    நிலை PSMOUSE_CMD_SETRATE:
    நிலை PSMOUSE_CMD_SETRES:
        // XXX
        திரும்பு 0;
    நிலை PSMOUSE_CMD_RESET_BAT:
    நிலை PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        திரும்பு 0;

    நிலை PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        திரும்பு 0;

    இயல்பு:
        திரும்பு -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
வெறுமை
usb_check_event(வெறுமை)
{
    usb_check_சாவி();
    usb_check_mouse();
}
