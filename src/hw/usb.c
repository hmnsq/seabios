// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadint
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-ehci.h" // ehci_setup
#شامل "usb-xhci.h" // xhci_setup
#شامل "usb-hid.h" // usb_keyboard_setup
#شامل "usb-hub.h" // usb_hub_setup
#شامل "usb-msc.h" // usb_msc_setup
#شامل "usb-ohci.h" // ohci_setup
#شامل "usb-uas.h" // usb_uas_setup
#شامل "usb-uhci.h" // uhci_setup
#شامل "util.h" // msleep
#شامل "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
ایستا ساختار usb_pipe *
usb_realloc_pipe(ساختار usbdevice_s *usbdev, ساختار usb_pipe *pipe
                 , ساختار usb_endpoint_descriptor *epdesc)
{
    انتخاب (usbdev->hub->cntl->type) {
    پیشفرض:
    حالت USB_TYPE_UHCI:
        بازگشت uhci_realloc_pipe(usbdev, pipe, epdesc);
    حالت USB_TYPE_OHCI:
        بازگشت ohci_realloc_pipe(usbdev, pipe, epdesc);
    حالت USB_TYPE_EHCI:
        بازگشت ehci_realloc_pipe(usbdev, pipe, epdesc);
    حالت USB_TYPE_XHCI:
        بازگشت xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ایستا صحیح
usb_send_pipe(ساختار usb_pipe *pipe_fl, صحیح dir, ثابت تهی *cmd
              , تهی *data, صحیح datasize)
{
    انتخاب (GET_LOWFLAT(pipe_fl->type)) {
    پیشفرض:
    حالت USB_TYPE_UHCI:
        بازگشت uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    حالت USB_TYPE_OHCI:
        اگر (MODESEGMENT)
            بازگشت -1;
        بازگشت ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    حالت USB_TYPE_EHCI:
        بازگشت ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    حالت USB_TYPE_XHCI:
        اگر (MODESEGMENT)
            بازگشت -1;
        بازگشت xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

صحیح
usb_poll_intr(ساختار usb_pipe *pipe_fl, تهی *data)
{
    ASSERT16();
    انتخاب (GET_LOWFLAT(pipe_fl->type)) {
    پیشفرض:
    حالت USB_TYPE_UHCI:
        بازگشت uhci_poll_intr(pipe_fl, data);
    حالت USB_TYPE_OHCI:
        بازگشت ohci_poll_intr(pipe_fl, data);
    حالت USB_TYPE_EHCI:
        بازگشت ehci_poll_intr(pipe_fl, data);
    حالت USB_TYPE_XHCI: ;
        بازگشت call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

صحیح usb_32bit_pipe(ساختار usb_pipe *pipe_fl)
{
    بازگشت (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
ساختار usb_pipe *
usb_alloc_pipe(ساختار usbdevice_s *usbdev
               , ساختار usb_endpoint_descriptor *epdesc)
{
    بازگشت usb_realloc_pipe(usbdev, پوچ, epdesc);
}

// Free an allocated control or bulk pipe.
تهی
usb_free_pipe(ساختار usbdevice_s *usbdev, ساختار usb_pipe *pipe)
{
    اگر (!pipe)
        بازگشت;
    usb_realloc_pipe(usbdev, pipe, پوچ);
}

// Send a message to the default control pipe of a device.
صحیح
usb_send_default_control(ساختار usb_pipe *pipe, ثابت ساختار usb_ctrlrequest *req
                         , تهی *data)
{
    بازگشت usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
صحیح
usb_send_bulk(ساختار usb_pipe *pipe_fl, صحیح dir, تهی *data, صحیح datasize)
{
    بازگشت usb_send_pipe(pipe_fl, dir, پوچ, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
صحیح
usb_is_freelist(ساختار usb_s *cntl, ساختار usb_pipe *pipe)
{
    بازگشت pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
تهی
usb_add_freelist(ساختار usb_pipe *pipe)
{
    اگر (!pipe)
        بازگشت;
    ساختار usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
ساختار usb_pipe *
usb_get_freelist(ساختار usb_s *cntl, u8 eptype)
{
    ساختار usb_pipe **pfree = &cntl->freelist;
    برای (;;) {
        ساختار usb_pipe *pipe = *pfree;
        اگر (!pipe)
            بازگشت پوچ;
        اگر (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            بازگشت pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
تهی
usb_desc2pipe(ساختار usb_pipe *pipe, ساختار usbdevice_s *usbdev
              , ساختار usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_عدد_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
صحیح
usb_get_period(ساختار usbdevice_s *usbdev
               , ساختار usb_endpoint_descriptor *epdesc)
{
    صحیح period = epdesc->bInterval;
    اگر (usbdev->speed != USB_HIGHSPEED)
        بازگشت (period <= 0) ? 0 : __fls(period);
    بازگشت (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
صحیح
usb_xfer_time(ساختار usb_pipe *pipe, صحیح datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    اگر (!GET_LOWFLAT(pipe->devaddr))
        بازگشت USB_TIME_STATUS + 100;
    بازگشت USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
ساختار usb_endpoint_descriptor *
usb_find_desc(ساختار usbdevice_s *usbdev, صحیح type, صحیح dir)
{
    ساختار usb_endpoint_descriptor *epdesc = (تهی*)&usbdev->iface[1];
    برای (;;) {
        اگر ((تهی*)epdesc >= (تهی*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            بازگشت پوچ;
        }
        اگر (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            بازگشت epdesc;
        epdesc = (تهی*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
ایستا صحیح
get_device_info8(ساختار usb_pipe *pipe, ساختار usb_device_descriptor *dinfo)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_مقدار = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    بازگشت usb_send_default_control(pipe, &req, dinfo);
}

ایستا ساختار usb_config_descriptor *
get_device_config(ساختار usb_pipe *pipe)
{
    ساختار usb_config_descriptor cfg;

    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_مقدار = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = اندازه(cfg);
    صحیح ret = usb_send_default_control(pipe, &req, &cfg);
    اگر (ret)
        بازگشت پوچ;

    ساختار usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    اگر (!config) {
        warn_noalloc();
        بازگشت پوچ;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    اگر (ret || config->wTotalLength != cfg.wTotalLength) {
        آزاد(config);
        بازگشت پوچ;
    }
    //hexdump(config, cfg.wTotalLength);
    بازگشت config;
}

ایستا صحیح
set_configuration(ساختار usb_pipe *pipe, u16 val)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_مقدار = val;
    req.wIndex = 0;
    req.wLength = 0;
    بازگشت usb_send_default_control(pipe, &req, پوچ);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ایستا ثابت صحیح speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ایستا صحیح
usb_set_address(ساختار usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ساختار usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    اگر (cntl->maxaddr >= USB_MAXADDR)
        بازگشت -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    ساختار usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    اگر (!usbdev->defpipe)
        بازگشت -1;

    // Send set_address command.
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_مقدار = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    صحیح ret = usb_send_default_control(usbdev->defpipe, &req, پوچ);
    اگر (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        بازگشت -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    اگر (!usbdev->defpipe)
        بازگشت -1;
    بازگشت 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ایستا صحیح
configure_usb_device(ساختار usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    ساختار usb_device_descriptor dinfo;
    صحیح ret = get_device_info8(usbdev->defpipe, &dinfo);
    اگر (ret)
        بازگشت 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    اگر (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    اگر (maxpacket < 8)
        بازگشت 0;
    ساختار usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    اگر (!usbdev->defpipe)
        بازگشت -1;

    // Get configuration
    ساختار usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    اگر (!config)
        بازگشت 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    صحیح رقم_iface = config->b_رقم_Interfaces;
    تهی *config_end = (تهی*)config + config->wTotalLength;
    ساختار usb_interface_descriptor *iface = (تهی*)(&config[1]);
    برای (;;) {
        اگر (!رقم_iface || (تهی*)iface + iface->bLength > config_end)
            // Not a supported device.
            برو fail;
        اگر (iface->bDescriptorType == USB_DT_INTERFACE) {
            رقم_iface--;
            اگر (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                بشکن;
        }
        iface = (تهی*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_مقدار);
    اگر (ret)
        برو fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (تهی*)config + config->wTotalLength - (تهی*)iface;
    اگر (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    وگرنه اگر (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        اگر (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        اگر (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } وگرنه
        ret = usb_hid_setup(usbdev);
    اگر (ret)
        برو fail;

    آزاد(config);
    بازگشت 1;
fail:
    آزاد(config);
    بازگشت 0;
}

ایستا تهی
usb_hub_port_setup(تهی *data)
{
    ساختار usbdevice_s *usbdev = data;
    ساختار usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    برای (;;) {
        // Detect if device present (and possibly start reset)
        صحیح ret = hub->op->detect(hub, port);
        اگر (ret > 0)
            // Device connected.
            بشکن;
        اگر (ret < 0 || timer_check(hub->detectend))
            // No device found.
            برو done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    صحیح ret = hub->op->reset(hub, port);
    اگر (ret < 0)
        // Reset failed
        برو resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    اگر (ret) {
        hub->op->disconnect(hub, port);
        برو resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    صحیح شمار = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    اگر (!شمار)
        hub->op->disconnect(hub, port);
    hub->devcount += شمار;
done:
    hub->threads--;
    آزاد(usbdev);
    بازگشت;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    برو done;
}

ایستا u32 usb_time_sigatt;

تهی
usb_enumerate(ساختار usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    صحیح i;
    برای (i=0; i<portcount; i++) {
        ساختار usbdevice_s *usbdev = malloc_tmphigh(اندازه(*usbdev));
        اگر (!usbdev) {
            warn_noalloc();
            ادامه;
        }
        memset(usbdev, 0, اندازه(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    وقتی (hub->threads)
        yield();
}

تهی
usb_setup(تهی)
{
    ASSERT32FLAT();
    اگر (! CONFIG_USB)
        بازگشت;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
