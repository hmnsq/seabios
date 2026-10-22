// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "config.h" // CONFIG_*
#شامل "output.h" // dprintf, warn_noalloc
#شامل "malloc.h" // malloc_fseg
#شامل "ps2port.h" // ATKBD_CMD_GETID
#شامل "usb.h" // usb_ctrlrequest
#شامل "usb-hid.h" // usb_keyboard_setup
#شامل "util.h" // process_key

ساختار pipe_گره {
    ساختار usb_pipe *pipe;
    ساختار pipe_گره *next;
};

ساختار pipe_گره *keyboards VARFSEG = پوچ;
ساختار pipe_گره *mice VARFSEG = پوچ;

ایستا صحیح
add_pipe_گره(ساختار pipe_گره **list
              , ساختار usbdevice_s *usbdev
              , ساختار usb_endpoint_descriptor *epdesc)
{
    ساختار usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    اگر (!pipe)
        بازگشت -1;

    ساختار pipe_گره *new_گره = malloc_fseg(اندازه(ساختار pipe_گره));
    اگر (!new_گره) {
        warn_noalloc();
        بازگشت -1;
    }

    new_گره->pipe = pipe;

    new_گره->next = *list;
    *list = new_گره;

    بازگشت 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
ایستا صحیح
set_protocol(ساختار usb_pipe *pipe, u16 val, u16 inferface)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_PROTOCOL;
    req.w_مقدار = val;
    req.wIndex = inferface;
    req.wLength = 0;
    بازگشت usb_send_default_control(pipe, &req, پوچ);
}

// Send USB HID SetIdle request.
ایستا صحیح
set_idle(ساختار usb_pipe *pipe, صحیح ms)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_SET_IDLE;
    req.w_مقدار = (ms/4)<<8;
    req.wIndex = 0;
    req.wLength = 0;
    بازگشت usb_send_default_control(pipe, &req, پوچ);
}

#define KEYREPEATWAITMS 500
#define KEYREPEATMS 33

// Format of USB keyboard event data
ساختار keyevent {
    u8 modifiers;
    u8 reserved;
    u8 کلیدها[6];
};

#define MAX_KBD_EVENT 16

ایستا صحیح
usb_kbd_setup(ساختار usbdevice_s *usbdev
              , ساختار usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_KEYBOARD)
        بازگشت -1;

    اگر (epdesc->wMaxPacketSize < اندازه(ساختار keyevent)
        || epdesc->wMaxPacketSize > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        بازگشت -1;
    }

    // Enable "boot" protocol.
    اگر (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_عدد)) {
        dprintf(3, "Failed to set boot protocol\n");
        بازگشت -1;
    }

    // Periodically send reports to enable key repeat.
    اگر (set_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    اگر (add_pipe_گره(&keyboards, usbdev, epdesc))
        بازگشت -1;

    dprintf(1, "USB keyboard initialized\n");
    بازگشت 0;
}

// Format of USB mouse event data
ساختار mouseevent {
    u8 buttons;
    u8 x, y;
};

#define MAX_MOUSE_EVENT 8

ایستا صحیح
usb_mouse_setup(ساختار usbdevice_s *usbdev
                , ساختار usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_MOUSE)
        بازگشت -1;

    اگر (epdesc->wMaxPacketSize < اندازه(ساختار mouseevent)
        || epdesc->wMaxPacketSize > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->wMaxPacketSize);
        بازگشت -1;
    }

    // Enable "boot" protocol.
    اگر (set_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_عدد))
        بازگشت -1;

    اگر (add_pipe_گره(&mice, usbdev, epdesc))
        بازگشت -1;

    dprintf(1, "USB mouse initialized\n");
    بازگشت 0;
}

// Initialize a found USB HID device (if applicable).
صحیح
usb_hid_setup(ساختار usbdevice_s *usbdev)
{
    اگر (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        بازگشت -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    ساختار usb_interface_descriptor *iface = usbdev->iface;
    اگر (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        بازگشت -1;

    // Find intr in endpoint.
    ساختار usb_endpoint_descriptor *epdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
    اگر (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        بازگشت -1;
    }

    اگر (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        بازگشت usb_kbd_setup(usbdev, epdesc);
    اگر (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        بازگشت usb_mouse_setup(usbdev, epdesc);
    بازگشت -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
ایستا u16 کلید_To_Scan_Code[] VAR16 = {
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
ایستا u16 ModifierToScanCode[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#define RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
ایستا تهی
prockeys(u16 scancode, u8 کلید_release, u8 mods)
{
    اگر (scancode > 0xff) {
        اگر (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            اگر (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_کلید(0xe0);
                process_کلید(0x46 | کلید_release);
            } وگرنه {
                // Pause key
                process_کلید(0xe1);
                process_کلید(0x1d | کلید_release);
                process_کلید(0x45 | کلید_release);
            }
            بازگشت;
        } وگرنه اگر (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_کلید(0x54 | کلید_release);
            بازگشت;
        }
        process_کلید(0xe0);
    }
    process_کلید(scancode | کلید_release);
}

// Handle a USB key press/release event.
ایستا تهی
procscankey(u8 کلید, u8 کلید_release, u8 mods)
{
    اگر (کلید >= ARRAY_SIZE(کلید_To_Scan_Code))
        بازگشت;
    u16 scancode = GET_GLOBAL(کلید_To_Scan_Code[کلید]);
    اگر (scancode)
        prockeys(scancode, کلید_release, mods);
}

// Handle a USB modifier press/release event.
ایستا تهی
procmodkey(u8 mods, u8 کلید_release)
{
    صحیح i;
    برای (i=0; mods; i++)
        اگر (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(ModifierToScanCode[i]), کلید_release, 0);
            mods &= ~(1<<i);
        }
}

ساختار usbkeyinfo {
    اجتماع {
        ساختار {
            u8 modifiers;
            u8 repeatcount;
            u8 کلیدها[6];
        };
        u64 data;
    };
};
ساختار usbkeyinfo LastUSBkey VARLOW;

// Process USB keyboard data.
ایستا تهی
handle_کلید(ساختار keyevent *data)
{
    dprintf(9, "Got key %x %x\n", data->modifiers, data->کلیدها[0]);

    // Load old keys.
    ساختار usbkeyinfo old;
    old.data = GET_کم(LastUSBkey.data);

    // Check for keys no longer pressed.
    صحیح addpos = 0;
    صحیح i;
    برای (i=0; i<ARRAY_SIZE(old.کلیدها); i++) {
        u8 کلید = old.کلیدها[i];
        اگر (!کلید)
            بشکن;
        صحیح j;
        برای (j=0;; j++) {
            اگر (j>=ARRAY_SIZE(data->کلیدها)) {
                // Key released.
                procscankey(کلید, RELEASEBIT, data->modifiers);
                اگر (i+1 >= ARRAY_SIZE(old.کلیدها) || !old.کلیدها[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                بشکن;
            }
            اگر (data->کلیدها[j] == کلید) {
                // Key still pressed.
                data->کلیدها[j] = 0;
                old.کلیدها[addpos++] = کلید;
                بشکن;
            }
        }
    }
    procmodkey(old.modifiers & ~data->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(data->modifiers & ~old.modifiers, 0);
    old.modifiers = data->modifiers;
    برای (i=0; i<ARRAY_SIZE(data->کلیدها); i++) {
        u8 کلید = data->کلیدها[i];
        اگر (!کلید)
            ادامه;
        // New key pressed.
        procscankey(کلید, 0, data->modifiers);
        old.کلیدها[addpos++] = کلید;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    اگر (addpos < ARRAY_SIZE(old.کلیدها))
        old.کلیدها[addpos] = 0;

    // Check for key repeat event.
    اگر (addpos) {
        اگر (!old.repeatcount)
            procscankey(old.کلیدها[addpos-1], 0, data->modifiers);
        وگرنه اگر (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    SET_کم(LastUSBkey.data, old.data);
}

// Check if a USB keyboard event is pending and process it if so.
ایستا تهی
usb_check_کلید(تهی)
{
    اگر (! CONFIG_USB_KEYBOARD)
        بازگشت;

    برای (ساختار pipe_گره *گره = GET_GLOBAL(keyboards);
         گره;
         گره = GET_GLOBALFLAT(گره->next)) {
        ساختار usb_pipe *pipe = GET_GLOBALFLAT(گره->pipe);

        برای (;;) {
            u8 data[MAX_KBD_EVENT];
            صحیح ret = usb_poll_intr(pipe, data);
            اگر (ret)
                بشکن;
            handle_کلید((تهی*)data);
        }
    }
}

// Test if USB keyboard is active.
درونخطی صحیح
usb_kbd_active(تهی)
{
    اگر (! CONFIG_USB_KEYBOARD)
        بازگشت 0;

    بازگشت GET_GLOBAL(keyboards) != پوچ;
}

// Handle a ps2 style keyboard command.
درونخطی صحیح
usb_kbd_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_USB_KEYBOARD)
        بازگشت -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    انتخاب (command) {
    حالت ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        بازگشت 0;
    پیشفرض:
        بازگشت -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
ایستا تهی
handle_mouse(ساختار mouseevent *data)
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
ایستا تهی
usb_check_mouse(تهی)
{
    اگر (! CONFIG_USB_MOUSE)
        بازگشت;

    برای (ساختار pipe_گره *گره = GET_GLOBAL(mice);
         گره;
         گره = GET_GLOBALFLAT(گره->next)) {
        ساختار usb_pipe *pipe = GET_GLOBALFLAT(گره->pipe);

        برای (;;) {
            u8 data[MAX_MOUSE_EVENT];
            صحیح ret = usb_poll_intr(pipe, data);
            اگر (ret)
                بشکن;
            handle_mouse((تهی*)data);
        }
    }
}

// Test if USB mouse is active.
درونخطی صحیح
usb_mouse_active(تهی)
{
    اگر (! CONFIG_USB_MOUSE)
        بازگشت 0;

    بازگشت GET_GLOBAL(mice) != پوچ;
}

// Handle a ps2 style mouse command.
درونخطی صحیح
usb_mouse_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_USB_MOUSE)
        بازگشت -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    انتخاب (command) {
    حالت PSMOUSE_CMD_ENABLE:
    حالت PSMOUSE_CMD_DISABLE:
    حالت PSMOUSE_CMD_SETSCALE11:
        بازگشت 0;
    حالت PSMOUSE_CMD_SETSCALE21:
    حالت PSMOUSE_CMD_SETRATE:
    حالت PSMOUSE_CMD_SETRES:
        // XXX
        بازگشت 0;
    حالت PSMOUSE_CMD_RESET_BAT:
    حالت PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        بازگشت 0;

    حالت PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        بازگشت 0;

    پیشفرض:
        بازگشت -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
تهی
usb_check_event(تهی)
{
    usb_check_کلید();
    usb_check_mouse();
}
