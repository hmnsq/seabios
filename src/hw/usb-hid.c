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

ساخت pipe_گرہ {
    ساخت usb_pipe *pipe;
    ساخت pipe_گرہ *اگلا;
};

ساخت pipe_گرہ *keyboards VARFSEG = تہی;
ساخت pipe_گرہ *mice VARFSEG = تہی;

ساکن صحیح
جوڑو_pipe_گرہ(ساخت pipe_گرہ **list
              , ساخت usbdevice_s *usbdev
              , ساخت usb_endpoint_descriptor *epdesc)
{
    ساخت usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    اگر (!pipe)
        واپس -1;

    ساخت pipe_گرہ *new_گرہ = malloc_fseg(حجم(ساخت pipe_گرہ));
    اگر (!new_گرہ) {
        warn_noalloc();
        واپس -1;
    }

    new_گرہ->pipe = pipe;

    new_گرہ->اگلا = *list;
    *list = new_گرہ;

    واپس 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
ساکن صحیح
سیٹ_protocol(ساخت usb_pipe *pipe, u16 val, u16 inferface)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_برآمد | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_سیٹ_PROTOCOL;
    req.w_قدر = val;
    req.w_اشاریہ = inferface;
    req.w_لمبائی = 0;
    واپس usb_send_default_control(pipe, &req, تہی);
}

// Send USB HID SetIdle request.
ساکن صحیح
سیٹ_idle(ساخت usb_pipe *pipe, صحیح ms)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_برآمد | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_سیٹ_IDLE;
    req.w_قدر = (ms/4)<<8;
    req.w_اشاریہ = 0;
    req.w_لمبائی = 0;
    واپس usb_send_default_control(pipe, &req, تہی);
}

#تعریف_کرو KEYREPEATWAITMS 500
#تعریف_کرو KEYREPEATMS 33

// Format of USB keyboard event data
ساخت keyevent {
    u8 modifiers;
    u8 reserved;
    u8 کلیدیں[6];
};

#تعریف_کرو MAX_KBD_EVENT 16

ساکن صحیح
usb_kbd_تیاری(ساخت usbdevice_s *usbdev
              , ساخت usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_KEYBOARD)
        واپس -1;

    اگر (epdesc->w_Max_Packet_سائز < حجم(ساخت keyevent)
        || epdesc->w_Max_Packet_سائز > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_سائز);
        واپس -1;
    }

    // Enable "boot" protocol.
    اگر (سیٹ_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_نمبر)) {
        dprintf(3, "Failed to set boot protocol\n");
        واپس -1;
    }

    // Periodically send reports to enable key repeat.
    اگر (سیٹ_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    اگر (جوڑو_pipe_گرہ(&keyboards, usbdev, epdesc))
        واپس -1;

    dprintf(1, "USB keyboard initialized\n");
    واپس 0;
}

// Format of USB mouse event data
ساخت mouseevent {
    u8 buttons;
    u8 x, y;
};

#تعریف_کرو MAX_MOUSE_EVENT 8

ساکن صحیح
usb_mouse_تیاری(ساخت usbdevice_s *usbdev
                , ساخت usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_MOUSE)
        واپس -1;

    اگر (epdesc->w_Max_Packet_سائز < حجم(ساخت mouseevent)
        || epdesc->w_Max_Packet_سائز > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_سائز);
        واپس -1;
    }

    // Enable "boot" protocol.
    اگر (سیٹ_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_نمبر))
        واپس -1;

    اگر (جوڑو_pipe_گرہ(&mice, usbdev, epdesc))
        واپس -1;

    dprintf(1, "USB mouse initialized\n");
    واپس 0;
}

// Initialize a found USB HID device (if applicable).
صحیح
usb_hid_تیاری(ساخت usbdevice_s *usbdev)
{
    اگر (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        واپس -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    ساخت usb_interface_descriptor *iface = usbdev->iface;
    اگر (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        واپس -1;

    // Find intr in endpoint.
    ساخت usb_endpoint_descriptor *epdesc = usb_ڈھونڈو_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_اندر);
    اگر (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        واپس -1;
    }

    اگر (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        واپس usb_kbd_تیاری(usbdev, epdesc);
    اگر (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        واپس usb_mouse_تیاری(usbdev, epdesc);
    واپس -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
ساکن u16 کلید_مقصد_جائزہ_رمز[] VAR16 = {
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
ساکن u16 Modifier_مقصد_جائزہ_رمز[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#تعریف_کرو RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
ساکن خالی
prockeys(u16 scancode, u8 کلید_release, u8 mods)
{
    اگر (scancode > 0xff) {
        اگر (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            اگر (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_کلید(0xe0);
                process_کلید(0x46 | کلید_release);
            } ورنہ {
                // Pause key
                process_کلید(0xe1);
                process_کلید(0x1d | کلید_release);
                process_کلید(0x45 | کلید_release);
            }
            واپس;
        } ورنہ اگر (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_کلید(0x54 | کلید_release);
            واپس;
        }
        process_کلید(0xe0);
    }
    process_کلید(scancode | کلید_release);
}

// Handle a USB key press/release event.
ساکن خالی
procscankey(u8 کلید, u8 کلید_release, u8 mods)
{
    اگر (کلید >= ARRAY_سائز(کلید_مقصد_جائزہ_رمز))
        واپس;
    u16 scancode = GET_GLOBAL(کلید_مقصد_جائزہ_رمز[کلید]);
    اگر (scancode)
        prockeys(scancode, کلید_release, mods);
}

// Handle a USB modifier press/release event.
ساکن خالی
procmodkey(u8 mods, u8 کلید_release)
{
    صحیح i;
    برائے (i=0; mods; i++)
        اگر (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(Modifier_مقصد_جائزہ_رمز[i]), کلید_release, 0);
            mods &= ~(1<<i);
        }
}

ساخت usbkeyinfo {
    اتحاد {
        ساخت {
            u8 modifiers;
            u8 repeatcount;
            u8 کلیدیں[6];
        };
        u64 ڈیٹا;
    };
};
ساخت usbkeyinfo آخری_USBkey VARLOW;

// Process USB keyboard data.
ساکن خالی
handle_کلید(ساخت keyevent *ڈیٹا)
{
    dprintf(9, "Got key %x %x\n", ڈیٹا->modifiers, ڈیٹا->کلیدیں[0]);

    // Load old keys.
    ساخت usbkeyinfo old;
    old.ڈیٹا = GET_کم(آخری_USBkey.ڈیٹا);

    // Check for keys no longer pressed.
    صحیح addpos = 0;
    صحیح i;
    برائے (i=0; i<ARRAY_سائز(old.کلیدیں); i++) {
        u8 کلید = old.کلیدیں[i];
        اگر (!کلید)
            توڑو;
        صحیح j;
        برائے (j=0;; j++) {
            اگر (j>=ARRAY_سائز(ڈیٹا->کلیدیں)) {
                // Key released.
                procscankey(کلید, RELEASEBIT, ڈیٹا->modifiers);
                اگر (i+1 >= ARRAY_سائز(old.کلیدیں) || !old.کلیدیں[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                توڑو;
            }
            اگر (ڈیٹا->کلیدیں[j] == کلید) {
                // Key still pressed.
                ڈیٹا->کلیدیں[j] = 0;
                old.کلیدیں[addpos++] = کلید;
                توڑو;
            }
        }
    }
    procmodkey(old.modifiers & ~ڈیٹا->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(ڈیٹا->modifiers & ~old.modifiers, 0);
    old.modifiers = ڈیٹا->modifiers;
    برائے (i=0; i<ARRAY_سائز(ڈیٹا->کلیدیں); i++) {
        u8 کلید = ڈیٹا->کلیدیں[i];
        اگر (!کلید)
            جاری;
        // New key pressed.
        procscankey(کلید, 0, ڈیٹا->modifiers);
        old.کلیدیں[addpos++] = کلید;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    اگر (addpos < ARRAY_سائز(old.کلیدیں))
        old.کلیدیں[addpos] = 0;

    // Check for key repeat event.
    اگر (addpos) {
        اگر (!old.repeatcount)
            procscankey(old.کلیدیں[addpos-1], 0, ڈیٹا->modifiers);
        ورنہ اگر (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    سیٹ_کم(آخری_USBkey.ڈیٹا, old.ڈیٹا);
}

// Check if a USB keyboard event is pending and process it if so.
ساکن خالی
usb_check_کلید(خالی)
{
    اگر (! CONFIG_USB_KEYBOARD)
        واپس;

    برائے (ساخت pipe_گرہ *گرہ = GET_GLOBAL(keyboards);
         گرہ;
         گرہ = GET_GLOBALFLAT(گرہ->اگلا)) {
        ساخت usb_pipe *pipe = GET_GLOBALFLAT(گرہ->pipe);

        برائے (;;) {
            u8 ڈیٹا[MAX_KBD_EVENT];
            صحیح ret = usb_poll_intr(pipe, ڈیٹا);
            اگر (ret)
                توڑو;
            handle_کلید((خالی*)ڈیٹا);
        }
    }
}

// Test if USB keyboard is active.
اندرونی صحیح
usb_kbd_فعال(خالی)
{
    اگر (! CONFIG_USB_KEYBOARD)
        واپس 0;

    واپس GET_GLOBAL(keyboards) != تہی;
}

// Handle a ps2 style keyboard command.
اندرونی صحیح
usb_kbd_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_USB_KEYBOARD)
        واپس -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    انتخاب (command) {
    حالت ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        واپس 0;
    طےشدہ:
        واپس -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
ساکن خالی
handle_mouse(ساخت mouseevent *ڈیٹا)
{
    dprintf(9, "Got mouse b=%x x=%x y=%x\n", ڈیٹا->buttons, ڈیٹا->x, ڈیٹا->y);

    s8 x = ڈیٹا->x, y = -ڈیٹا->y;
    u8 flag = ((ڈیٹا->buttons & 0x7) | (1<<3)
               | (x & 0x80 ? (1<<4) : 0) | (y & 0x80 ? (1<<5) : 0));
    process_mouse(flag);
    process_mouse(x);
    process_mouse(y);
}

// Check if a USB mouse event is pending and process it if so.
ساکن خالی
usb_check_mouse(خالی)
{
    اگر (! CONFIG_USB_MOUSE)
        واپس;

    برائے (ساخت pipe_گرہ *گرہ = GET_GLOBAL(mice);
         گرہ;
         گرہ = GET_GLOBALFLAT(گرہ->اگلا)) {
        ساخت usb_pipe *pipe = GET_GLOBALFLAT(گرہ->pipe);

        برائے (;;) {
            u8 ڈیٹا[MAX_MOUSE_EVENT];
            صحیح ret = usb_poll_intr(pipe, ڈیٹا);
            اگر (ret)
                توڑو;
            handle_mouse((خالی*)ڈیٹا);
        }
    }
}

// Test if USB mouse is active.
اندرونی صحیح
usb_mouse_فعال(خالی)
{
    اگر (! CONFIG_USB_MOUSE)
        واپس 0;

    واپس GET_GLOBAL(mice) != تہی;
}

// Handle a ps2 style mouse command.
اندرونی صحیح
usb_mouse_command(صحیح command, u8 *param)
{
    اگر (! CONFIG_USB_MOUSE)
        واپس -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    انتخاب (command) {
    حالت PSMOUSE_CMD_ENABLE:
    حالت PSMOUSE_CMD_DISABLE:
    حالت PSMOUSE_CMD_SETSCALE11:
        واپس 0;
    حالت PSMOUSE_CMD_SETSCALE21:
    حالت PSMOUSE_CMD_SETRATE:
    حالت PSMOUSE_CMD_SETRES:
        // XXX
        واپس 0;
    حالت PSMOUSE_CMD_RESET_BAT:
    حالت PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        واپس 0;

    حالت PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        واپس 0;

    طےشدہ:
        واپس -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
خالی
usb_check_event(خالی)
{
    usb_check_کلید();
    usb_check_mouse();
}
