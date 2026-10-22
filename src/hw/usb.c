// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-ehci.h" // ehci_setup
#அந்தர்பாவ "usb-xhci.h" // xhci_setup
#அந்தர்பாவ "usb-hid.h" // usb_keyboard_setup
#அந்தர்பாவ "usb-hub.h" // usb_hub_setup
#அந்தர்பாவ "usb-msc.h" // usb_msc_setup
#அந்தர்பாவ "usb-ohci.h" // ohci_setup
#அந்தர்பாவ "usb-uas.h" // usb_uas_setup
#அந்தர்பாவ "usb-uhci.h" // uhci_setup
#அந்தர்பாவ "util.h" // msleep
#அந்தர்பாவ "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
நிலையான கட்டமைப்பு usb_pipe *
usb_realloc_pipe(கட்டமைப்பு usbdevice_s *usbdev, கட்டமைப்பு usb_pipe *pipe
                 , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    தேர்வு (usbdev->hub->cntl->type) {
    இயல்பு:
    நிலை USB_TYPE_UHCI:
        திரும்பு uhci_realloc_pipe(usbdev, pipe, epdesc);
    நிலை USB_TYPE_OHCI:
        திரும்பு ohci_realloc_pipe(usbdev, pipe, epdesc);
    நிலை USB_TYPE_EHCI:
        திரும்பு ehci_realloc_pipe(usbdev, pipe, epdesc);
    நிலை USB_TYPE_XHCI:
        திரும்பு xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
நிலையான முழு
usb_send_pipe(கட்டமைப்பு usb_pipe *pipe_fl, முழு dir, மாறிலி வெறுமை *cmd
              , வெறுமை *data, முழு datasize)
{
    தேர்வு (GET_LOWFLAT(pipe_fl->type)) {
    இயல்பு:
    நிலை USB_TYPE_UHCI:
        திரும்பு uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    நிலை USB_TYPE_OHCI:
        எனில் (MODESEGMENT)
            திரும்பு -1;
        திரும்பு ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    நிலை USB_TYPE_EHCI:
        திரும்பு ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    நிலை USB_TYPE_XHCI:
        எனில் (MODESEGMENT)
            திரும்பு -1;
        திரும்பு xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

முழு
usb_poll_intr(கட்டமைப்பு usb_pipe *pipe_fl, வெறுமை *data)
{
    ASSERT16();
    தேர்வு (GET_LOWFLAT(pipe_fl->type)) {
    இயல்பு:
    நிலை USB_TYPE_UHCI:
        திரும்பு uhci_poll_intr(pipe_fl, data);
    நிலை USB_TYPE_OHCI:
        திரும்பு ohci_poll_intr(pipe_fl, data);
    நிலை USB_TYPE_EHCI:
        திரும்பு ehci_poll_intr(pipe_fl, data);
    நிலை USB_TYPE_XHCI: ;
        திரும்பு call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

முழு usb_32bit_pipe(கட்டமைப்பு usb_pipe *pipe_fl)
{
    திரும்பு (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
கட்டமைப்பு usb_pipe *
usb_alloc_pipe(கட்டமைப்பு usbdevice_s *usbdev
               , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    திரும்பு usb_realloc_pipe(usbdev, வெற்று, epdesc);
}

// Free an allocated control or bulk pipe.
வெறுமை
usb_free_pipe(கட்டமைப்பு usbdevice_s *usbdev, கட்டமைப்பு usb_pipe *pipe)
{
    எனில் (!pipe)
        திரும்பு;
    usb_realloc_pipe(usbdev, pipe, வெற்று);
}

// Send a message to the default control pipe of a device.
முழு
usb_send_default_control(கட்டமைப்பு usb_pipe *pipe, மாறிலி கட்டமைப்பு usb_ctrlrequest *req
                         , வெறுமை *data)
{
    திரும்பு usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
முழு
usb_send_bulk(கட்டமைப்பு usb_pipe *pipe_fl, முழு dir, வெறுமை *data, முழு datasize)
{
    திரும்பு usb_send_pipe(pipe_fl, dir, வெற்று, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
முழு
usb_is_freelist(கட்டமைப்பு usb_s *cntl, கட்டமைப்பு usb_pipe *pipe)
{
    திரும்பு pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
வெறுமை
usb_add_freelist(கட்டமைப்பு usb_pipe *pipe)
{
    எனில் (!pipe)
        திரும்பு;
    கட்டமைப்பு usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
கட்டமைப்பு usb_pipe *
usb_get_freelist(கட்டமைப்பு usb_s *cntl, u8 eptype)
{
    கட்டமைப்பு usb_pipe **pfree = &cntl->freelist;
    க்கு (;;) {
        கட்டமைப்பு usb_pipe *pipe = *pfree;
        எனில் (!pipe)
            திரும்பு வெற்று;
        எனில் (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            திரும்பு pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
வெறுமை
usb_desc2pipe(கட்டமைப்பு usb_pipe *pipe, கட்டமைப்பு usbdevice_s *usbdev
              , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_எண்_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
முழு
usb_get_period(கட்டமைப்பு usbdevice_s *usbdev
               , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    முழு period = epdesc->bInterval;
    எனில் (usbdev->speed != USB_HIGHSPEED)
        திரும்பு (period <= 0) ? 0 : __fls(period);
    திரும்பு (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
முழு
usb_xfer_time(கட்டமைப்பு usb_pipe *pipe, முழு datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    எனில் (!GET_LOWFLAT(pipe->devaddr))
        திரும்பு USB_TIME_STATUS + 100;
    திரும்பு USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
கட்டமைப்பு usb_endpoint_descriptor *
usb_find_desc(கட்டமைப்பு usbdevice_s *usbdev, முழு type, முழு dir)
{
    கட்டமைப்பு usb_endpoint_descriptor *epdesc = (வெறுமை*)&usbdev->iface[1];
    க்கு (;;) {
        எனில் ((வெறுமை*)epdesc >= (வெறுமை*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            திரும்பு வெற்று;
        }
        எனில் (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            திரும்பு epdesc;
        epdesc = (வெறுமை*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
நிலையான முழு
get_device_info8(கட்டமைப்பு usb_pipe *pipe, கட்டமைப்பு usb_device_descriptor *dinfo)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_மதிப்பு = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    திரும்பு usb_send_default_control(pipe, &req, dinfo);
}

நிலையான கட்டமைப்பு usb_config_descriptor *
get_device_config(கட்டமைப்பு usb_pipe *pipe)
{
    கட்டமைப்பு usb_config_descriptor cfg;

    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_மதிப்பு = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = அளவு(cfg);
    முழு ret = usb_send_default_control(pipe, &req, &cfg);
    எனில் (ret)
        திரும்பு வெற்று;

    கட்டமைப்பு usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    எனில் (!config) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    எனில் (ret || config->wTotalLength != cfg.wTotalLength) {
        விடுவி(config);
        திரும்பு வெற்று;
    }
    //hexdump(config, cfg.wTotalLength);
    திரும்பு config;
}

நிலையான முழு
set_configuration(கட்டமைப்பு usb_pipe *pipe, u16 val)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_மதிப்பு = val;
    req.wIndex = 0;
    req.wLength = 0;
    திரும்பு usb_send_default_control(pipe, &req, வெற்று);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

நிலையான மாறிலி முழு speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
நிலையான முழு
usb_set_address(கட்டமைப்பு usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    கட்டமைப்பு usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    எனில் (cntl->maxaddr >= USB_MAXADDR)
        திரும்பு -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    கட்டமைப்பு usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    எனில் (!usbdev->defpipe)
        திரும்பு -1;

    // Send set_address command.
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_மதிப்பு = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    முழு ret = usb_send_default_control(usbdev->defpipe, &req, வெற்று);
    எனில் (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        திரும்பு -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    எனில் (!usbdev->defpipe)
        திரும்பு -1;
    திரும்பு 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
நிலையான முழு
configure_usb_device(கட்டமைப்பு usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    கட்டமைப்பு usb_device_descriptor dinfo;
    முழு ret = get_device_info8(usbdev->defpipe, &dinfo);
    எனில் (ret)
        திரும்பு 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    எனில் (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    எனில் (maxpacket < 8)
        திரும்பு 0;
    கட்டமைப்பு usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    எனில் (!usbdev->defpipe)
        திரும்பு -1;

    // Get configuration
    கட்டமைப்பு usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    எனில் (!config)
        திரும்பு 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    முழு இலக்கம்_iface = config->b_இலக்கம்_Interfaces;
    வெறுமை *config_end = (வெறுமை*)config + config->wTotalLength;
    கட்டமைப்பு usb_interface_descriptor *iface = (வெறுமை*)(&config[1]);
    க்கு (;;) {
        எனில் (!இலக்கம்_iface || (வெறுமை*)iface + iface->bLength > config_end)
            // Not a supported device.
            செல் fail;
        எனில் (iface->bDescriptorType == USB_DT_INTERFACE) {
            இலக்கம்_iface--;
            எனில் (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                நிறுத்து;
        }
        iface = (வெறுமை*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_மதிப்பு);
    எனில் (ret)
        செல் fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (வெறுமை*)config + config->wTotalLength - (வெறுமை*)iface;
    எனில் (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    இல்லையெனில் எனில் (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        எனில் (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        எனில் (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } இல்லையெனில்
        ret = usb_hid_setup(usbdev);
    எனில் (ret)
        செல் fail;

    விடுவி(config);
    திரும்பு 1;
fail:
    விடுவி(config);
    திரும்பு 0;
}

நிலையான வெறுமை
usb_hub_port_setup(வெறுமை *data)
{
    கட்டமைப்பு usbdevice_s *usbdev = data;
    கட்டமைப்பு usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    க்கு (;;) {
        // Detect if device present (and possibly start reset)
        முழு ret = hub->op->detect(hub, port);
        எனில் (ret > 0)
            // Device connected.
            நிறுத்து;
        எனில் (ret < 0 || timer_check(hub->detectend))
            // No device found.
            செல் done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    முழு ret = hub->op->reset(hub, port);
    எனில் (ret < 0)
        // Reset failed
        செல் resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    எனில் (ret) {
        hub->op->disconnect(hub, port);
        செல் resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    முழு மொத்தம் = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    எனில் (!மொத்தம்)
        hub->op->disconnect(hub, port);
    hub->devcount += மொத்தம்;
done:
    hub->threads--;
    விடுவி(usbdev);
    திரும்பு;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    செல் done;
}

நிலையான u32 usb_time_sigatt;

வெறுமை
usb_enumerate(கட்டமைப்பு usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    முழு i;
    க்கு (i=0; i<portcount; i++) {
        கட்டமைப்பு usbdevice_s *usbdev = malloc_tmphigh(அளவு(*usbdev));
        எனில் (!usbdev) {
            warn_noalloc();
            தொடர்;
        }
        memset(usbdev, 0, அளவு(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    போது (hub->threads)
        yield();
}

வெறுமை
usb_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_USB)
        திரும்பு;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
