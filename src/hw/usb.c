// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-ehci.h" // ehci_setup
#অন্তর্ভাব "usb-xhci.h" // xhci_setup
#অন্তর্ভাব "usb-hid.h" // usb_keyboard_setup
#অন্তর্ভাব "usb-hub.h" // usb_hub_setup
#অন্তর্ভাব "usb-msc.h" // usb_msc_setup
#অন্তর্ভাব "usb-ohci.h" // ohci_setup
#অন্তর্ভাব "usb-uas.h" // usb_uas_setup
#অন্তর্ভাব "usb-uhci.h" // uhci_setup
#অন্তর্ভাব "util.h" // msleep
#অন্তর্ভাব "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
স্থির সংরচনা usb_pipe *
usb_realloc_pipe(সংরচনা usbdevice_s *usbdev, সংরচনা usb_pipe *pipe
                 , সংরচনা usb_endpoint_descriptor *epdesc)
{
    বিকল্প (usbdev->hub->cntl->type) {
    স্বভাব:
    পক্ষ USB_TYPE_UHCI:
        প্রত্যাগম uhci_realloc_pipe(usbdev, pipe, epdesc);
    পক্ষ USB_TYPE_OHCI:
        প্রত্যাগম ohci_realloc_pipe(usbdev, pipe, epdesc);
    পক্ষ USB_TYPE_EHCI:
        প্রত্যাগম ehci_realloc_pipe(usbdev, pipe, epdesc);
    পক্ষ USB_TYPE_XHCI:
        প্রত্যাগম xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
স্থির পূর্ণাঙ্ক
usb_send_pipe(সংরচনা usb_pipe *pipe_fl, পূর্ণাঙ্ক dir, ধ্রুব শূন্য *cmd
              , শূন্য *data, পূর্ণাঙ্ক datasize)
{
    বিকল্প (GET_LOWFLAT(pipe_fl->type)) {
    স্বভাব:
    পক্ষ USB_TYPE_UHCI:
        প্রত্যাগম uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    পক্ষ USB_TYPE_OHCI:
        যদি (MODESEGMENT)
            প্রত্যাগম -1;
        প্রত্যাগম ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    পক্ষ USB_TYPE_EHCI:
        প্রত্যাগম ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    পক্ষ USB_TYPE_XHCI:
        যদি (MODESEGMENT)
            প্রত্যাগম -1;
        প্রত্যাগম xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

পূর্ণাঙ্ক
usb_poll_intr(সংরচনা usb_pipe *pipe_fl, শূন্য *data)
{
    ASSERT16();
    বিকল্প (GET_LOWFLAT(pipe_fl->type)) {
    স্বভাব:
    পক্ষ USB_TYPE_UHCI:
        প্রত্যাগম uhci_poll_intr(pipe_fl, data);
    পক্ষ USB_TYPE_OHCI:
        প্রত্যাগম ohci_poll_intr(pipe_fl, data);
    পক্ষ USB_TYPE_EHCI:
        প্রত্যাগম ehci_poll_intr(pipe_fl, data);
    পক্ষ USB_TYPE_XHCI: ;
        প্রত্যাগম call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

পূর্ণাঙ্ক usb_32bit_pipe(সংরচনা usb_pipe *pipe_fl)
{
    প্রত্যাগম (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
সংরচনা usb_pipe *
usb_alloc_pipe(সংরচনা usbdevice_s *usbdev
               , সংরচনা usb_endpoint_descriptor *epdesc)
{
    প্রত্যাগম usb_realloc_pipe(usbdev, রিক্ত, epdesc);
}

// Free an allocated control or bulk pipe.
শূন্য
usb_free_pipe(সংরচনা usbdevice_s *usbdev, সংরচনা usb_pipe *pipe)
{
    যদি (!pipe)
        প্রত্যাগম;
    usb_realloc_pipe(usbdev, pipe, রিক্ত);
}

// Send a message to the default control pipe of a device.
পূর্ণাঙ্ক
usb_send_default_control(সংরচনা usb_pipe *pipe, ধ্রুব সংরচনা usb_ctrlrequest *req
                         , শূন্য *data)
{
    প্রত্যাগম usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
পূর্ণাঙ্ক
usb_send_bulk(সংরচনা usb_pipe *pipe_fl, পূর্ণাঙ্ক dir, শূন্য *data, পূর্ণাঙ্ক datasize)
{
    প্রত্যাগম usb_send_pipe(pipe_fl, dir, রিক্ত, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
পূর্ণাঙ্ক
usb_is_freelist(সংরচনা usb_s *cntl, সংরচনা usb_pipe *pipe)
{
    প্রত্যাগম pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
শূন্য
usb_add_freelist(সংরচনা usb_pipe *pipe)
{
    যদি (!pipe)
        প্রত্যাগম;
    সংরচনা usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
সংরচনা usb_pipe *
usb_get_freelist(সংরচনা usb_s *cntl, u8 eptype)
{
    সংরচনা usb_pipe **pfree = &cntl->freelist;
    প্রতি (;;) {
        সংরচনা usb_pipe *pipe = *pfree;
        যদি (!pipe)
            প্রত্যাগম রিক্ত;
        যদি (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            প্রত্যাগম pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
শূন্য
usb_desc2pipe(সংরচনা usb_pipe *pipe, সংরচনা usbdevice_s *usbdev
              , সংরচনা usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_সংখ্যা_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
পূর্ণাঙ্ক
usb_get_period(সংরচনা usbdevice_s *usbdev
               , সংরচনা usb_endpoint_descriptor *epdesc)
{
    পূর্ণাঙ্ক period = epdesc->bInterval;
    যদি (usbdev->speed != USB_HIGHSPEED)
        প্রত্যাগম (period <= 0) ? 0 : __fls(period);
    প্রত্যাগম (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
পূর্ণাঙ্ক
usb_xfer_time(সংরচনা usb_pipe *pipe, পূর্ণাঙ্ক datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    যদি (!GET_LOWFLAT(pipe->devaddr))
        প্রত্যাগম USB_TIME_STATUS + 100;
    প্রত্যাগম USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
সংরচনা usb_endpoint_descriptor *
usb_find_desc(সংরচনা usbdevice_s *usbdev, পূর্ণাঙ্ক type, পূর্ণাঙ্ক dir)
{
    সংরচনা usb_endpoint_descriptor *epdesc = (শূন্য*)&usbdev->iface[1];
    প্রতি (;;) {
        যদি ((শূন্য*)epdesc >= (শূন্য*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            প্রত্যাগম রিক্ত;
        }
        যদি (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            প্রত্যাগম epdesc;
        epdesc = (শূন্য*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
স্থির পূর্ণাঙ্ক
get_device_info8(সংরচনা usb_pipe *pipe, সংরচনা usb_device_descriptor *dinfo)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_মান = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    প্রত্যাগম usb_send_default_control(pipe, &req, dinfo);
}

স্থির সংরচনা usb_config_descriptor *
get_device_config(সংরচনা usb_pipe *pipe)
{
    সংরচনা usb_config_descriptor cfg;

    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_মান = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = পরিমাণ(cfg);
    পূর্ণাঙ্ক ret = usb_send_default_control(pipe, &req, &cfg);
    যদি (ret)
        প্রত্যাগম রিক্ত;

    সংরচনা usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    যদি (!config) {
        warn_noalloc();
        প্রত্যাগম রিক্ত;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    যদি (ret || config->wTotalLength != cfg.wTotalLength) {
        মুক্ত(config);
        প্রত্যাগম রিক্ত;
    }
    //hexdump(config, cfg.wTotalLength);
    প্রত্যাগম config;
}

স্থির পূর্ণাঙ্ক
set_configuration(সংরচনা usb_pipe *pipe, u16 val)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_মান = val;
    req.wIndex = 0;
    req.wLength = 0;
    প্রত্যাগম usb_send_default_control(pipe, &req, রিক্ত);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

স্থির ধ্রুব পূর্ণাঙ্ক speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
স্থির পূর্ণাঙ্ক
usb_set_address(সংরচনা usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    সংরচনা usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    যদি (cntl->maxaddr >= USB_MAXADDR)
        প্রত্যাগম -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    সংরচনা usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    যদি (!usbdev->defpipe)
        প্রত্যাগম -1;

    // Send set_address command.
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_মান = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    পূর্ণাঙ্ক ret = usb_send_default_control(usbdev->defpipe, &req, রিক্ত);
    যদি (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        প্রত্যাগম -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    যদি (!usbdev->defpipe)
        প্রত্যাগম -1;
    প্রত্যাগম 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
স্থির পূর্ণাঙ্ক
configure_usb_device(সংরচনা usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    সংরচনা usb_device_descriptor dinfo;
    পূর্ণাঙ্ক ret = get_device_info8(usbdev->defpipe, &dinfo);
    যদি (ret)
        প্রত্যাগম 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    যদি (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    যদি (maxpacket < 8)
        প্রত্যাগম 0;
    সংরচনা usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    যদি (!usbdev->defpipe)
        প্রত্যাগম -1;

    // Get configuration
    সংরচনা usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    যদি (!config)
        প্রত্যাগম 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    পূর্ণাঙ্ক অঙ্ক_iface = config->b_অঙ্ক_Interfaces;
    শূন্য *config_end = (শূন্য*)config + config->wTotalLength;
    সংরচনা usb_interface_descriptor *iface = (শূন্য*)(&config[1]);
    প্রতি (;;) {
        যদি (!অঙ্ক_iface || (শূন্য*)iface + iface->bLength > config_end)
            // Not a supported device.
            গচ্ছ fail;
        যদি (iface->bDescriptorType == USB_DT_INTERFACE) {
            অঙ্ক_iface--;
            যদি (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                ভঙ্গ;
        }
        iface = (শূন্য*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_মান);
    যদি (ret)
        গচ্ছ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (শূন্য*)config + config->wTotalLength - (শূন্য*)iface;
    যদি (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    অন্যথা যদি (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        যদি (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        যদি (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } অন্যথা
        ret = usb_hid_setup(usbdev);
    যদি (ret)
        গচ্ছ fail;

    মুক্ত(config);
    প্রত্যাগম 1;
fail:
    মুক্ত(config);
    প্রত্যাগম 0;
}

স্থির শূন্য
usb_hub_port_setup(শূন্য *data)
{
    সংরচনা usbdevice_s *usbdev = data;
    সংরচনা usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    প্রতি (;;) {
        // Detect if device present (and possibly start reset)
        পূর্ণাঙ্ক ret = hub->op->detect(hub, port);
        যদি (ret > 0)
            // Device connected.
            ভঙ্গ;
        যদি (ret < 0 || timer_check(hub->detectend))
            // No device found.
            গচ্ছ done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    পূর্ণাঙ্ক ret = hub->op->reset(hub, port);
    যদি (ret < 0)
        // Reset failed
        গচ্ছ resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    যদি (ret) {
        hub->op->disconnect(hub, port);
        গচ্ছ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    পূর্ণাঙ্ক গুনতি = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    যদি (!গুনতি)
        hub->op->disconnect(hub, port);
    hub->devcount += গুনতি;
done:
    hub->threads--;
    মুক্ত(usbdev);
    প্রত্যাগম;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    গচ্ছ done;
}

স্থির u32 usb_time_sigatt;

শূন্য
usb_enumerate(সংরচনা usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<portcount; i++) {
        সংরচনা usbdevice_s *usbdev = malloc_tmphigh(পরিমাণ(*usbdev));
        যদি (!usbdev) {
            warn_noalloc();
            অনুবর্ত;
        }
        memset(usbdev, 0, পরিমাণ(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    যাবত্ (hub->threads)
        yield();
}

শূন্য
usb_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (! CONFIG_USB)
        প্রত্যাগম;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
