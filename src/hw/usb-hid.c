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

بنية pipe_گره {
    بنية usb_pipe *pipe;
    بنية pipe_گره *بعدی;
};

بنية pipe_گره *keyboards VARFSEG = عدم;
بنية pipe_گره *mice VARFSEG = عدم;

ساكن صحيح
جمع_کن_pipe_گره(بنية pipe_گره **list
              , بنية usbdevice_s *usbdev
              , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    إذا (!pipe)
        أرجع -1;

    بنية pipe_گره *new_گره = malloc_fseg(حجم(بنية pipe_گره));
    إذا (!new_گره) {
        warn_noalloc();
        أرجع -1;
    }

    new_گره->pipe = pipe;

    new_گره->بعدی = *list;
    *list = new_گره;

    أرجع 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
ساكن صحيح
مجموعه_protocol(بنية usb_pipe *pipe, u16 val, u16 inferface)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بیرون | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_مجموعه_PROTOCOL;
    req.w_مقدار = val;
    req.w_شاخص = inferface;
    req.w_درازی = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

// Send USB HID SetIdle request.
ساكن صحيح
مجموعه_idle(بنية usb_pipe *pipe, صحيح ms)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بیرون | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_مجموعه_IDLE;
    req.w_مقدار = (ms/4)<<8;
    req.w_شاخص = 0;
    req.w_درازی = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

#تعریف_کن KEYREPEATWAITMS 500
#تعریف_کن KEYREPEATMS 33

// Format of USB keyboard event data
بنية keyevent {
    u8 modifiers;
    u8 reserved;
    u8 کلیدها[6];
};

#تعریف_کن MAX_KBD_EVENT 16

ساكن صحيح
usb_kbd_راهاندازی(بنية usbdevice_s *usbdev
              , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (! CONFIG_USB_KEYBOARD)
        أرجع -1;

    إذا (epdesc->w_Max_Packet_اندازه < حجم(بنية keyevent)
        || epdesc->w_Max_Packet_اندازه > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_اندازه);
        أرجع -1;
    }

    // Enable "boot" protocol.
    إذا (مجموعه_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_عدد)) {
        dprintf(3, "Failed to set boot protocol\n");
        أرجع -1;
    }

    // Periodically send reports to enable key repeat.
    إذا (مجموعه_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    إذا (جمع_کن_pipe_گره(&keyboards, usbdev, epdesc))
        أرجع -1;

    dprintf(1, "USB keyboard initialized\n");
    أرجع 0;
}

// Format of USB mouse event data
بنية mouseevent {
    u8 buttons;
    u8 x, y;
};

#تعریف_کن MAX_MOUSE_EVENT 8

ساكن صحيح
usb_mouse_راهاندازی(بنية usbdevice_s *usbdev
                , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (! CONFIG_USB_MOUSE)
        أرجع -1;

    إذا (epdesc->w_Max_Packet_اندازه < حجم(بنية mouseevent)
        || epdesc->w_Max_Packet_اندازه > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_اندازه);
        أرجع -1;
    }

    // Enable "boot" protocol.
    إذا (مجموعه_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_عدد))
        أرجع -1;

    إذا (جمع_کن_pipe_گره(&mice, usbdev, epdesc))
        أرجع -1;

    dprintf(1, "USB mouse initialized\n");
    أرجع 0;
}

// Initialize a found USB HID device (if applicable).
صحيح
usb_hid_راهاندازی(بنية usbdevice_s *usbdev)
{
    إذا (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        أرجع -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    بنية usb_interface_descriptor *iface = usbdev->iface;
    إذا (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        أرجع -1;

    // Find intr in endpoint.
    بنية usb_endpoint_descriptor *epdesc = usb_پیدا_کن_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_درون);
    إذا (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        أرجع -1;
    }

    إذا (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        أرجع usb_kbd_راهاندازی(usbdev, epdesc);
    إذا (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        أرجع usb_mouse_راهاندازی(usbdev, epdesc);
    أرجع -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
ساكن u16 کلید_مقصد_پیمایش_کد[] VAR16 = {
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
ساكن u16 Modifier_مقصد_پیمایش_کد[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#تعریف_کن RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
ساكن فراغ
prockeys(u16 scancode, u8 کلید_release, u8 mods)
{
    إذا (scancode > 0xff) {
        إذا (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            إذا (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_کلید(0xe0);
                process_کلید(0x46 | کلید_release);
            } وإلا {
                // Pause key
                process_کلید(0xe1);
                process_کلید(0x1d | کلید_release);
                process_کلید(0x45 | کلید_release);
            }
            أرجع;
        } وإلا إذا (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_کلید(0x54 | کلید_release);
            أرجع;
        }
        process_کلید(0xe0);
    }
    process_کلید(scancode | کلید_release);
}

// Handle a USB key press/release event.
ساكن فراغ
procscankey(u8 کلید, u8 کلید_release, u8 mods)
{
    إذا (کلید >= ARRAY_اندازه(کلید_مقصد_پیمایش_کد))
        أرجع;
    u16 scancode = GET_GLOBAL(کلید_مقصد_پیمایش_کد[کلید]);
    إذا (scancode)
        prockeys(scancode, کلید_release, mods);
}

// Handle a USB modifier press/release event.
ساكن فراغ
procmodkey(u8 mods, u8 کلید_release)
{
    صحيح i;
    لكل (i=0; mods; i++)
        إذا (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(Modifier_مقصد_پیمایش_کد[i]), کلید_release, 0);
            mods &= ~(1<<i);
        }
}

بنية usbkeyinfo {
    اتحاد {
        بنية {
            u8 modifiers;
            u8 repeatcount;
            u8 کلیدها[6];
        };
        u64 داده;
    };
};
بنية usbkeyinfo آخرین_USBkey VARLOW;

// Process USB keyboard data.
ساكن فراغ
handle_کلید(بنية keyevent *داده)
{
    dprintf(9, "Got key %x %x\n", داده->modifiers, داده->کلیدها[0]);

    // Load old keys.
    بنية usbkeyinfo old;
    old.داده = GET_کم(آخرین_USBkey.داده);

    // Check for keys no longer pressed.
    صحيح addpos = 0;
    صحيح i;
    لكل (i=0; i<ARRAY_اندازه(old.کلیدها); i++) {
        u8 کلید = old.کلیدها[i];
        إذا (!کلید)
            اكسر;
        صحيح j;
        لكل (j=0;; j++) {
            إذا (j>=ARRAY_اندازه(داده->کلیدها)) {
                // Key released.
                procscankey(کلید, RELEASEBIT, داده->modifiers);
                إذا (i+1 >= ARRAY_اندازه(old.کلیدها) || !old.کلیدها[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                اكسر;
            }
            إذا (داده->کلیدها[j] == کلید) {
                // Key still pressed.
                داده->کلیدها[j] = 0;
                old.کلیدها[addpos++] = کلید;
                اكسر;
            }
        }
    }
    procmodkey(old.modifiers & ~داده->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(داده->modifiers & ~old.modifiers, 0);
    old.modifiers = داده->modifiers;
    لكل (i=0; i<ARRAY_اندازه(داده->کلیدها); i++) {
        u8 کلید = داده->کلیدها[i];
        إذا (!کلید)
            استمر;
        // New key pressed.
        procscankey(کلید, 0, داده->modifiers);
        old.کلیدها[addpos++] = کلید;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    إذا (addpos < ARRAY_اندازه(old.کلیدها))
        old.کلیدها[addpos] = 0;

    // Check for key repeat event.
    إذا (addpos) {
        إذا (!old.repeatcount)
            procscankey(old.کلیدها[addpos-1], 0, داده->modifiers);
        وإلا إذا (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    مجموعه_کم(آخرین_USBkey.داده, old.داده);
}

// Check if a USB keyboard event is pending and process it if so.
ساكن فراغ
usb_check_کلید(فراغ)
{
    إذا (! CONFIG_USB_KEYBOARD)
        أرجع;

    لكل (بنية pipe_گره *گره = GET_GLOBAL(keyboards);
         گره;
         گره = GET_GLOBALFLAT(گره->بعدی)) {
        بنية usb_pipe *pipe = GET_GLOBALFLAT(گره->pipe);

        لكل (;;) {
            u8 داده[MAX_KBD_EVENT];
            صحيح ret = usb_poll_intr(pipe, داده);
            إذا (ret)
                اكسر;
            handle_کلید((فراغ*)داده);
        }
    }
}

// Test if USB keyboard is active.
مضمن صحيح
usb_kbd_فعال(فراغ)
{
    إذا (! CONFIG_USB_KEYBOARD)
        أرجع 0;

    أرجع GET_GLOBAL(keyboards) != عدم;
}

// Handle a ps2 style keyboard command.
مضمن صحيح
usb_kbd_command(صحيح command, u8 *param)
{
    إذا (! CONFIG_USB_KEYBOARD)
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
handle_mouse(بنية mouseevent *داده)
{
    dprintf(9, "Got mouse b=%x x=%x y=%x\n", داده->buttons, داده->x, داده->y);

    s8 x = داده->x, y = -داده->y;
    u8 flag = ((داده->buttons & 0x7) | (1<<3)
               | (x & 0x80 ? (1<<4) : 0) | (y & 0x80 ? (1<<5) : 0));
    process_mouse(flag);
    process_mouse(x);
    process_mouse(y);
}

// Check if a USB mouse event is pending and process it if so.
ساكن فراغ
usb_check_mouse(فراغ)
{
    إذا (! CONFIG_USB_MOUSE)
        أرجع;

    لكل (بنية pipe_گره *گره = GET_GLOBAL(mice);
         گره;
         گره = GET_GLOBALFLAT(گره->بعدی)) {
        بنية usb_pipe *pipe = GET_GLOBALFLAT(گره->pipe);

        لكل (;;) {
            u8 داده[MAX_MOUSE_EVENT];
            صحيح ret = usb_poll_intr(pipe, داده);
            إذا (ret)
                اكسر;
            handle_mouse((فراغ*)داده);
        }
    }
}

// Test if USB mouse is active.
مضمن صحيح
usb_mouse_فعال(فراغ)
{
    إذا (! CONFIG_USB_MOUSE)
        أرجع 0;

    أرجع GET_GLOBAL(mice) != عدم;
}

// Handle a ps2 style mouse command.
مضمن صحيح
usb_mouse_command(صحيح command, u8 *param)
{
    إذا (! CONFIG_USB_MOUSE)
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
    usb_check_کلید();
    usb_check_mouse();
}
