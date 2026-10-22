// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadint
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-ehci.h" // ehci_setup
#ضمن "usb-xhci.h" // xhci_setup
#ضمن "usb-hid.h" // usb_keyboard_setup
#ضمن "usb-hub.h" // usb_hub_setup
#ضمن "usb-msc.h" // usb_msc_setup
#ضمن "usb-ohci.h" // ohci_setup
#ضمن "usb-uas.h" // usb_uas_setup
#ضمن "usb-uhci.h" // uhci_setup
#ضمن "util.h" // msleep
#ضمن "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
ساكن بنية usb_pipe *
usb_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *pipe
                 , بنية usb_endpoint_descriptor *epdesc)
{
    بدل (usbdev->hub->cntl->type) {
    افتراضي:
    حالة USB_TYPE_UHCI:
        ܦܢܐ uhci_realloc_pipe(usbdev, pipe, epdesc);
    حالة USB_TYPE_OHCI:
        ܦܢܐ ohci_realloc_pipe(usbdev, pipe, epdesc);
    حالة USB_TYPE_EHCI:
        ܦܢܐ ehci_realloc_pipe(usbdev, pipe, epdesc);
    حالة USB_TYPE_XHCI:
        ܦܢܐ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ساكن صحيح
usb_send_pipe(بنية usb_pipe *pipe_fl, صحيح dir, ثابت فراغ *cmd
              , فراغ *data, صحيح datasize)
{
    بدل (GET_LOWFLAT(pipe_fl->type)) {
    افتراضي:
    حالة USB_TYPE_UHCI:
        ܦܢܐ uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    حالة USB_TYPE_OHCI:
        ܐܢ (MODESEGMENT)
            ܦܢܐ -1;
        ܦܢܐ ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    حالة USB_TYPE_EHCI:
        ܦܢܐ ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    حالة USB_TYPE_XHCI:
        ܐܢ (MODESEGMENT)
            ܦܢܐ -1;
        ܦܢܐ xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

صحيح
usb_poll_intr(بنية usb_pipe *pipe_fl, فراغ *data)
{
    ASSERT16();
    بدل (GET_LOWFLAT(pipe_fl->type)) {
    افتراضي:
    حالة USB_TYPE_UHCI:
        ܦܢܐ uhci_poll_intr(pipe_fl, data);
    حالة USB_TYPE_OHCI:
        ܦܢܐ ohci_poll_intr(pipe_fl, data);
    حالة USB_TYPE_EHCI:
        ܦܢܐ ehci_poll_intr(pipe_fl, data);
    حالة USB_TYPE_XHCI: ;
        ܦܢܐ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

صحيح usb_32bit_pipe(بنية usb_pipe *pipe_fl)
{
    ܦܢܐ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
بنية usb_pipe *
usb_alloc_pipe(بنية usbdevice_s *usbdev
               , بنية usb_endpoint_descriptor *epdesc)
{
    ܦܢܐ usb_realloc_pipe(usbdev, عدم, epdesc);
}

// Free an allocated control or bulk pipe.
فراغ
usb_free_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *pipe)
{
    ܐܢ (!pipe)
        ܦܢܐ;
    usb_realloc_pipe(usbdev, pipe, عدم);
}

// Send a message to the default control pipe of a device.
صحيح
usb_send_default_control(بنية usb_pipe *pipe, ثابت بنية usb_ctrlrequest *req
                         , فراغ *data)
{
    ܦܢܐ usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
صحيح
usb_send_bulk(بنية usb_pipe *pipe_fl, صحيح dir, فراغ *data, صحيح datasize)
{
    ܦܢܐ usb_send_pipe(pipe_fl, dir, عدم, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
صحيح
usb_is_freelist(بنية usb_s *cntl, بنية usb_pipe *pipe)
{
    ܦܢܐ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
فراغ
usb_add_freelist(بنية usb_pipe *pipe)
{
    ܐܢ (!pipe)
        ܦܢܐ;
    بنية usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
بنية usb_pipe *
usb_get_freelist(بنية usb_s *cntl, u8 eptype)
{
    بنية usb_pipe **pfree = &cntl->freelist;
    لكل (;;) {
        بنية usb_pipe *pipe = *pfree;
        ܐܢ (!pipe)
            ܦܢܐ عدم;
        ܐܢ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ܦܢܐ pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
فراغ
usb_desc2pipe(بنية usb_pipe *pipe, بنية usbdevice_s *usbdev
              , بنية usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
صحيح
usb_get_period(بنية usbdevice_s *usbdev
               , بنية usb_endpoint_descriptor *epdesc)
{
    صحيح period = epdesc->bInterval;
    ܐܢ (usbdev->speed != USB_HIGHSPEED)
        ܦܢܐ (period <= 0) ? 0 : __fls(period);
    ܦܢܐ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
صحيح
usb_xfer_time(بنية usb_pipe *pipe, صحيح datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ܐܢ (!GET_LOWFLAT(pipe->devaddr))
        ܦܢܐ USB_TIME_STATUS + 100;
    ܦܢܐ USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
بنية usb_endpoint_descriptor *
usb_find_desc(بنية usbdevice_s *usbdev, صحيح type, صحيح dir)
{
    بنية usb_endpoint_descriptor *epdesc = (فراغ*)&usbdev->iface[1];
    لكل (;;) {
        ܐܢ ((فراغ*)epdesc >= (فراغ*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ܦܢܐ عدم;
        }
        ܐܢ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            ܦܢܐ epdesc;
        epdesc = (فراغ*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
ساكن صحيح
get_device_info8(بنية usb_pipe *pipe, بنية usb_device_descriptor *dinfo)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    ܦܢܐ usb_send_default_control(pipe, &req, dinfo);
}

ساكن بنية usb_config_descriptor *
get_device_config(بنية usb_pipe *pipe)
{
    بنية usb_config_descriptor cfg;

    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = حجم(cfg);
    صحيح ret = usb_send_default_control(pipe, &req, &cfg);
    ܐܢ (ret)
        ܦܢܐ عدم;

    بنية usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    ܐܢ (!config) {
        warn_noalloc();
        ܦܢܐ عدم;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    ܐܢ (ret || config->wTotalLength != cfg.wTotalLength) {
        حرر(config);
        ܦܢܐ عدم;
    }
    //hexdump(config, cfg.wTotalLength);
    ܦܢܐ config;
}

ساكن صحيح
set_configuration(بنية usb_pipe *pipe, u16 val)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.wValue = val;
    req.wIndex = 0;
    req.wLength = 0;
    ܦܢܐ usb_send_default_control(pipe, &req, عدم);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ساكن ثابت صحيح speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ساكن صحيح
usb_set_address(بنية usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    بنية usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ܐܢ (cntl->maxaddr >= USB_MAXADDR)
        ܦܢܐ -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    بنية usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ܐܢ (!usbdev->defpipe)
        ܦܢܐ -1;

    // Send set_address command.
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.wValue = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    صحيح ret = usb_send_default_control(usbdev->defpipe, &req, عدم);
    ܐܢ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ܦܢܐ -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ܐܢ (!usbdev->defpipe)
        ܦܢܐ -1;
    ܦܢܐ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ساكن صحيح
configure_usb_device(بنية usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    بنية usb_device_descriptor dinfo;
    صحيح ret = get_device_info8(usbdev->defpipe, &dinfo);
    ܐܢ (ret)
        ܦܢܐ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ܐܢ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ܐܢ (maxpacket < 8)
        ܦܢܐ 0;
    بنية usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ܐܢ (!usbdev->defpipe)
        ܦܢܐ -1;

    // Get configuration
    بنية usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ܐܢ (!config)
        ܦܢܐ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    صحيح num_iface = config->bNumInterfaces;
    فراغ *config_end = (فراغ*)config + config->wTotalLength;
    بنية usb_interface_descriptor *iface = (فراغ*)(&config[1]);
    لكل (;;) {
        ܐܢ (!num_iface || (فراغ*)iface + iface->bLength > config_end)
            // Not a supported device.
            اذهب fail;
        ܐܢ (iface->bDescriptorType == USB_DT_INTERFACE) {
            num_iface--;
            ܐܢ (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                اكسر;
        }
        iface = (فراغ*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->bConfigurationValue);
    ܐܢ (ret)
        اذهب fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (فراغ*)config + config->wTotalLength - (فراغ*)iface;
    ܐܢ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    ܐܚܪܢܐ ܐܢ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ܐܢ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        ܐܢ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } ܐܚܪܢܐ
        ret = usb_hid_setup(usbdev);
    ܐܢ (ret)
        اذهب fail;

    حرر(config);
    ܦܢܐ 1;
fail:
    حرر(config);
    ܦܢܐ 0;
}

ساكن فراغ
usb_hub_port_setup(فراغ *data)
{
    بنية usbdevice_s *usbdev = data;
    بنية usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    لكل (;;) {
        // Detect if device present (and possibly start reset)
        صحيح ret = hub->op->detect(hub, port);
        ܐܢ (ret > 0)
            // Device connected.
            اكسر;
        ܐܢ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            اذهب done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    صحيح ret = hub->op->reset(hub, port);
    ܐܢ (ret < 0)
        // Reset failed
        اذهب resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    ܐܢ (ret) {
        hub->op->disconnect(hub, port);
        اذهب resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    صحيح count = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ܐܢ (!count)
        hub->op->disconnect(hub, port);
    hub->devcount += count;
done:
    hub->threads--;
    حرر(usbdev);
    ܦܢܐ;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    اذهب done;
}

ساكن u32 usb_time_sigatt;

فراغ
usb_enumerate(بنية usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    صحيح i;
    لكل (i=0; i<portcount; i++) {
        بنية usbdevice_s *usbdev = malloc_tmphigh(حجم(*usbdev));
        ܐܢ (!usbdev) {
            warn_noalloc();
            استمر;
        }
        memset(usbdev, 0, حجم(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    ܟܡܐ (hub->threads)
        yield();
}

فراغ
usb_setup(فراغ)
{
    ASSERT32FLAT();
    ܐܢ (! CONFIG_USB)
        ܦܢܐ;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
