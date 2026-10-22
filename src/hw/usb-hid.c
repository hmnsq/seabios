// Code for handling USB Human Interface Devices (HID).
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2024  Daniel Khodabakhsh <d.khodabakhsh@gmail.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "output.h" // dprintf, warn_noalloc
#అన్తర్భావ "malloc.h" // malloc_fseg
#అన్తర్భావ "ps2port.h" // ATKBD_CMD_GETID
#అన్తర్భావ "usb.h" // usb_ctrlrequest
#అన్తర్భావ "usb-hid.h" // usb_keyboard_setup
#అన్తర్భావ "util.h" // process_key

నిర్మాణం pipe_కణుపు {
    నిర్మాణం usb_pipe *pipe;
    నిర్మాణం pipe_కణుపు *తదుపరి;
};

నిర్మాణం pipe_కణుపు *keyboards VARFSEG = ఖాళీ;
నిర్మాణం pipe_కణుపు *mice VARFSEG = ఖాళీ;

స్థిర పూర్ణాంకం
కూడు_pipe_కణుపు(నిర్మాణం pipe_కణుపు **list
              , నిర్మాణం usbdevice_s *usbdev
              , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    నిర్మాణం usb_pipe *pipe = usb_alloc_pipe(usbdev, epdesc);
    ఒకవేళ (!pipe)
        తిరిగి -1;

    నిర్మాణం pipe_కణుపు *new_కణుపు = malloc_fseg(పరిమాణం(నిర్మాణం pipe_కణుపు));
    ఒకవేళ (!new_కణుపు) {
        warn_noalloc();
        తిరిగి -1;
    }

    new_కణుపు->pipe = pipe;

    new_కణుపు->తదుపరి = *list;
    *list = new_కణుపు;

    తిరిగి 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Send USB HID protocol message.
స్థిర పూర్ణాంకం
సమితి_protocol(నిర్మాణం usb_pipe *pipe, u16 val, u16 inferface)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_బహిర్గతం | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_సమితి_PROTOCOL;
    req.w_విలువ = val;
    req.w_సూచీ = inferface;
    req.w_నిడివి = 0;
    తిరిగి usb_send_default_control(pipe, &req, ఖాళీ);
}

// Send USB HID SetIdle request.
స్థిర పూర్ణాంకం
సమితి_idle(నిర్మాణం usb_pipe *pipe, పూర్ణాంకం ms)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_బహిర్గతం | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = HID_REQ_సమితి_IDLE;
    req.w_విలువ = (ms/4)<<8;
    req.w_సూచీ = 0;
    req.w_నిడివి = 0;
    తిరిగి usb_send_default_control(pipe, &req, ఖాళీ);
}

#నిర్వచించు KEYREPEATWAITMS 500
#నిర్వచించు KEYREPEATMS 33

// Format of USB keyboard event data
నిర్మాణం keyevent {
    u8 modifiers;
    u8 reserved;
    u8 కీలకాలు[6];
};

#నిర్వచించు MAX_KBD_EVENT 16

స్థిర పూర్ణాంకం
usb_kbd_అమరిక(నిర్మాణం usbdevice_s *usbdev
              , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    ఒకవేళ (! CONFIG_USB_KEYBOARD)
        తిరిగి -1;

    ఒకవేళ (epdesc->w_Max_Packet_కొలత < పరిమాణం(నిర్మాణం keyevent)
        || epdesc->w_Max_Packet_కొలత > MAX_KBD_EVENT) {
        dprintf(1, "USB keyboard wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_కొలత);
        తిరిగి -1;
    }

    // Enable "boot" protocol.
    ఒకవేళ (సమితి_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_సంఖ్య)) {
        dprintf(3, "Failed to set boot protocol\n");
        తిరిగి -1;
    }

    // Periodically send reports to enable key repeat.
    ఒకవేళ (సమితి_idle(usbdev->defpipe, KEYREPEATMS))
        dprintf(3, "Warning: Failed to set key repeat rate\n");

    ఒకవేళ (కూడు_pipe_కణుపు(&keyboards, usbdev, epdesc))
        తిరిగి -1;

    dprintf(1, "USB keyboard initialized\n");
    తిరిగి 0;
}

// Format of USB mouse event data
నిర్మాణం mouseevent {
    u8 buttons;
    u8 x, y;
};

#నిర్వచించు MAX_MOUSE_EVENT 8

స్థిర పూర్ణాంకం
usb_mouse_అమరిక(నిర్మాణం usbdevice_s *usbdev
                , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    ఒకవేళ (! CONFIG_USB_MOUSE)
        తిరిగి -1;

    ఒకవేళ (epdesc->w_Max_Packet_కొలత < పరిమాణం(నిర్మాణం mouseevent)
        || epdesc->w_Max_Packet_కొలత > MAX_MOUSE_EVENT) {
        dprintf(1, "USB mouse wMaxPacketSize=%d; aborting\n"
                , epdesc->w_Max_Packet_కొలత);
        తిరిగి -1;
    }

    // Enable "boot" protocol.
    ఒకవేళ (సమితి_protocol(usbdev->defpipe, 0, usbdev->iface->b_Interface_సంఖ్య))
        తిరిగి -1;

    ఒకవేళ (కూడు_pipe_కణుపు(&mice, usbdev, epdesc))
        తిరిగి -1;

    dprintf(1, "USB mouse initialized\n");
    తిరిగి 0;
}

// Initialize a found USB HID device (if applicable).
పూర్ణాంకం
usb_hid_అమరిక(నిర్మాణం usbdevice_s *usbdev)
{
    ఒకవేళ (! CONFIG_USB_KEYBOARD && ! CONFIG_USB_MOUSE)
        తిరిగి -1;
    dprintf(2, "usb_hid_setup %p\n", usbdev->defpipe);

    నిర్మాణం usb_interface_descriptor *iface = usbdev->iface;
    ఒకవేళ (iface->bInterfaceSubClass != USB_INTERFACE_SUBCLASS_BOOT)
        // Doesn't support boot protocol.
        తిరిగి -1;

    // Find intr in endpoint.
    నిర్మాణం usb_endpoint_descriptor *epdesc = usb_కనుగొను_desc(
        usbdev, USB_ENDPOINT_XFER_INT, USB_DIR_లోపల);
    ఒకవేళ (!epdesc) {
        dprintf(1, "No usb hid intr in?\n");
        తిరిగి -1;
    }

    ఒకవేళ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD)
        తిరిగి usb_kbd_అమరిక(usbdev, epdesc);
    ఒకవేళ (iface->bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
        తిరిగి usb_mouse_అమరిక(usbdev, epdesc);
    తిరిగి -1;
}


/****************************************************************
 * Keyboard events
 ****************************************************************/

// Mapping from USB key id to ps2 key sequence.
స్థిర u16 కీలకం_గమ్యం_పరిశీలన_సంకేతం[] VAR16 = {
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
స్థిర u16 Modifier_గమ్యం_పరిశీలన_సంకేతం[] VAR16 = {
    //lcntl, lshift, lalt, lgui, rcntl, rshift, ralt, rgui
    0x001d, 0x002a, 0x0038, 0xe05b, 0xe01d, 0x0036, 0xe038, 0xe05c
};

#నిర్వచించు RELEASEBIT 0x80

// Translate data from KeyToScanCode[] to calls to process_key().
స్థిర శూన్య
prockeys(u16 scancode, u8 కీలకం_release, u8 mods)
{
    ఒకవేళ (scancode > 0xff) {
        ఒకవేళ (scancode == 0xe145) {
            // XXX - a real AT keyboard would immediately send the key release
            ఒకవేళ (mods & ((1<<0) | (1<<4))) {
                // Cntr+Break key
                process_కీలకం(0xe0);
                process_కీలకం(0x46 | కీలకం_release);
            } లేకపోతే {
                // Pause key
                process_కీలకం(0xe1);
                process_కీలకం(0x1d | కీలకం_release);
                process_కీలకం(0x45 | కీలకం_release);
            }
            తిరిగి;
        } లేకపోతే ఒకవేళ (scancode == 0xe037 && mods & ((1<<2) | (1<<6))) {
            // Alt+SysReq key
            process_కీలకం(0x54 | కీలకం_release);
            తిరిగి;
        }
        process_కీలకం(0xe0);
    }
    process_కీలకం(scancode | కీలకం_release);
}

// Handle a USB key press/release event.
స్థిర శూన్య
procscankey(u8 కీలకం, u8 కీలకం_release, u8 mods)
{
    ఒకవేళ (కీలకం >= ARRAY_కొలత(కీలకం_గమ్యం_పరిశీలన_సంకేతం))
        తిరిగి;
    u16 scancode = GET_GLOBAL(కీలకం_గమ్యం_పరిశీలన_సంకేతం[కీలకం]);
    ఒకవేళ (scancode)
        prockeys(scancode, కీలకం_release, mods);
}

// Handle a USB modifier press/release event.
స్థిర శూన్య
procmodkey(u8 mods, u8 కీలకం_release)
{
    పూర్ణాంకం i;
    కొరకు (i=0; mods; i++)
        ఒకవేళ (mods & (1<<i)) {
            // Modifier key change.
            prockeys(GET_GLOBAL(Modifier_గమ్యం_పరిశీలన_సంకేతం[i]), కీలకం_release, 0);
            mods &= ~(1<<i);
        }
}

నిర్మాణం usbkeyinfo {
    సంఘం {
        నిర్మాణం {
            u8 modifiers;
            u8 repeatcount;
            u8 కీలకాలు[6];
        };
        u64 దత్తాంశం;
    };
};
నిర్మాణం usbkeyinfo చివరి_USBkey VARLOW;

// Process USB keyboard data.
స్థిర శూన్య
handle_కీలకం(నిర్మాణం keyevent *దత్తాంశం)
{
    dprintf(9, "Got key %x %x\n", దత్తాంశం->modifiers, దత్తాంశం->కీలకాలు[0]);

    // Load old keys.
    నిర్మాణం usbkeyinfo old;
    old.దత్తాంశం = GET_దిగువ(చివరి_USBkey.దత్తాంశం);

    // Check for keys no longer pressed.
    పూర్ణాంకం addpos = 0;
    పూర్ణాంకం i;
    కొరకు (i=0; i<ARRAY_కొలత(old.కీలకాలు); i++) {
        u8 కీలకం = old.కీలకాలు[i];
        ఒకవేళ (!కీలకం)
            ఆపు;
        పూర్ణాంకం j;
        కొరకు (j=0;; j++) {
            ఒకవేళ (j>=ARRAY_కొలత(దత్తాంశం->కీలకాలు)) {
                // Key released.
                procscankey(కీలకం, RELEASEBIT, దత్తాంశం->modifiers);
                ఒకవేళ (i+1 >= ARRAY_కొలత(old.కీలకాలు) || !old.కీలకాలు[i+1])
                    // Last pressed key released - disable repeat.
                    old.repeatcount = 0xff;
                ఆపు;
            }
            ఒకవేళ (దత్తాంశం->కీలకాలు[j] == కీలకం) {
                // Key still pressed.
                దత్తాంశం->కీలకాలు[j] = 0;
                old.కీలకాలు[addpos++] = కీలకం;
                ఆపు;
            }
        }
    }
    procmodkey(old.modifiers & ~దత్తాంశం->modifiers, RELEASEBIT);

    // Process new keys
    procmodkey(దత్తాంశం->modifiers & ~old.modifiers, 0);
    old.modifiers = దత్తాంశం->modifiers;
    కొరకు (i=0; i<ARRAY_కొలత(దత్తాంశం->కీలకాలు); i++) {
        u8 కీలకం = దత్తాంశం->కీలకాలు[i];
        ఒకవేళ (!కీలకం)
            కొనసాగు;
        // New key pressed.
        procscankey(కీలకం, 0, దత్తాంశం->modifiers);
        old.కీలకాలు[addpos++] = కీలకం;
        old.repeatcount = KEYREPEATWAITMS / KEYREPEATMS + 1;
    }
    ఒకవేళ (addpos < ARRAY_కొలత(old.కీలకాలు))
        old.కీలకాలు[addpos] = 0;

    // Check for key repeat event.
    ఒకవేళ (addpos) {
        ఒకవేళ (!old.repeatcount)
            procscankey(old.కీలకాలు[addpos-1], 0, దత్తాంశం->modifiers);
        లేకపోతే ఒకవేళ (old.repeatcount != 0xff)
            old.repeatcount--;
    }

    // Update old keys
    సమితి_దిగువ(చివరి_USBkey.దత్తాంశం, old.దత్తాంశం);
}

// Check if a USB keyboard event is pending and process it if so.
స్థిర శూన్య
usb_check_కీలకం(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_KEYBOARD)
        తిరిగి;

    కొరకు (నిర్మాణం pipe_కణుపు *కణుపు = GET_GLOBAL(keyboards);
         కణుపు;
         కణుపు = GET_GLOBALFLAT(కణుపు->తదుపరి)) {
        నిర్మాణం usb_pipe *pipe = GET_GLOBALFLAT(కణుపు->pipe);

        కొరకు (;;) {
            u8 దత్తాంశం[MAX_KBD_EVENT];
            పూర్ణాంకం ret = usb_poll_intr(pipe, దత్తాంశం);
            ఒకవేళ (ret)
                ఆపు;
            handle_కీలకం((శూన్య*)దత్తాంశం);
        }
    }
}

// Test if USB keyboard is active.
అంతఃవరుస పూర్ణాంకం
usb_kbd_చురుకైన(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_KEYBOARD)
        తిరిగి 0;

    తిరిగి GET_GLOBAL(keyboards) != ఖాళీ;
}

// Handle a ps2 style keyboard command.
అంతఃవరుస పూర్ణాంకం
usb_kbd_command(పూర్ణాంకం command, u8 *param)
{
    ఒకవేళ (! CONFIG_USB_KEYBOARD)
        తిరిగి -1;
    dprintf(9, "usb keyboard cmd=%x\n", command);
    ఎంపిక (command) {
    స్థితి ATKBD_CMD_GETID:
        // Return the id of a standard AT keyboard.
        param[0] = 0xab;
        param[1] = 0x83;
        తిరిగి 0;
    అప్రమేయ:
        తిరిగి -1;
    }
}


/****************************************************************
 * Mouse events
 ****************************************************************/

// Process USB mouse data.
స్థిర శూన్య
handle_mouse(నిర్మాణం mouseevent *దత్తాంశం)
{
    dprintf(9, "Got mouse b=%x x=%x y=%x\n", దత్తాంశం->buttons, దత్తాంశం->x, దత్తాంశం->y);

    s8 x = దత్తాంశం->x, y = -దత్తాంశం->y;
    u8 flag = ((దత్తాంశం->buttons & 0x7) | (1<<3)
               | (x & 0x80 ? (1<<4) : 0) | (y & 0x80 ? (1<<5) : 0));
    process_mouse(flag);
    process_mouse(x);
    process_mouse(y);
}

// Check if a USB mouse event is pending and process it if so.
స్థిర శూన్య
usb_check_mouse(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_MOUSE)
        తిరిగి;

    కొరకు (నిర్మాణం pipe_కణుపు *కణుపు = GET_GLOBAL(mice);
         కణుపు;
         కణుపు = GET_GLOBALFLAT(కణుపు->తదుపరి)) {
        నిర్మాణం usb_pipe *pipe = GET_GLOBALFLAT(కణుపు->pipe);

        కొరకు (;;) {
            u8 దత్తాంశం[MAX_MOUSE_EVENT];
            పూర్ణాంకం ret = usb_poll_intr(pipe, దత్తాంశం);
            ఒకవేళ (ret)
                ఆపు;
            handle_mouse((శూన్య*)దత్తాంశం);
        }
    }
}

// Test if USB mouse is active.
అంతఃవరుస పూర్ణాంకం
usb_mouse_చురుకైన(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_MOUSE)
        తిరిగి 0;

    తిరిగి GET_GLOBAL(mice) != ఖాళీ;
}

// Handle a ps2 style mouse command.
అంతఃవరుస పూర్ణాంకం
usb_mouse_command(పూర్ణాంకం command, u8 *param)
{
    ఒకవేళ (! CONFIG_USB_MOUSE)
        తిరిగి -1;
    dprintf(9, "usb mouse cmd=%x\n", command);
    ఎంపిక (command) {
    స్థితి PSMOUSE_CMD_ENABLE:
    స్థితి PSMOUSE_CMD_DISABLE:
    స్థితి PSMOUSE_CMD_SETSCALE11:
        తిరిగి 0;
    స్థితి PSMOUSE_CMD_SETSCALE21:
    స్థితి PSMOUSE_CMD_SETRATE:
    స్థితి PSMOUSE_CMD_SETRES:
        // XXX
        తిరిగి 0;
    స్థితి PSMOUSE_CMD_RESET_BAT:
    స్థితి PSMOUSE_CMD_GETID:
        // Return the id of a standard AT mouse.
        param[0] = 0xaa;
        param[1] = 0x00;
        తిరిగి 0;

    స్థితి PSMOUSE_CMD_GETINFO:
        param[0] = 0x00;
        param[1] = 4;
        param[2] = 100;
        తిరిగి 0;

    అప్రమేయ:
        తిరిగి -1;
    }
}

// Check for USB events pending - called periodically from timer interrupt.
శూన్య
usb_check_event(శూన్య)
{
    usb_check_కీలకం();
    usb_check_mouse();
}
