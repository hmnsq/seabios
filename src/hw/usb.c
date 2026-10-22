// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-ehci.h" // ehci_setup
#അന്തര്ഭാവ "usb-xhci.h" // xhci_setup
#അന്തര്ഭാവ "usb-hid.h" // usb_keyboard_setup
#അന്തര്ഭാവ "usb-hub.h" // usb_hub_setup
#അന്തര്ഭാവ "usb-msc.h" // usb_msc_setup
#അന്തര്ഭാവ "usb-ohci.h" // ohci_setup
#അന്തര്ഭാവ "usb-uas.h" // usb_uas_setup
#അന്തര്ഭാവ "usb-uhci.h" // uhci_setup
#അന്തര്ഭാവ "util.h" // msleep
#അന്തര്ഭാവ "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
സ്ഥിതം ഘടന usb_pipe *
usb_realloc_pipe(ഘടന usbdevice_s *usbdev, ഘടന usb_pipe *pipe
                 , ഘടന usb_endpoint_descriptor *epdesc)
{
    തിരഞ്ഞെടുപ്പ് (usbdev->hub->cntl->type) {
    സ്ഥിരസ്ഥിതി:
    അവസ്ഥ USB_TYPE_UHCI:
        തിരികെ uhci_realloc_pipe(usbdev, pipe, epdesc);
    അവസ്ഥ USB_TYPE_OHCI:
        തിരികെ ohci_realloc_pipe(usbdev, pipe, epdesc);
    അവസ്ഥ USB_TYPE_EHCI:
        തിരികെ ehci_realloc_pipe(usbdev, pipe, epdesc);
    അവസ്ഥ USB_TYPE_XHCI:
        തിരികെ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_send_pipe(ഘടന usb_pipe *pipe_fl, പൂർണ്ണസംഖ്യ dir, സ്ഥിരം ശൂന്യം *cmd
              , ശൂന്യം *data, പൂർണ്ണസംഖ്യ datasize)
{
    തിരഞ്ഞെടുപ്പ് (GET_LOWFLAT(pipe_fl->type)) {
    സ്ഥിരസ്ഥിതി:
    അവസ്ഥ USB_TYPE_UHCI:
        തിരികെ uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    അവസ്ഥ USB_TYPE_OHCI:
        എങ്കിൽ (MODESEGMENT)
            തിരികെ -1;
        തിരികെ ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    അവസ്ഥ USB_TYPE_EHCI:
        തിരികെ ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    അവസ്ഥ USB_TYPE_XHCI:
        എങ്കിൽ (MODESEGMENT)
            തിരികെ -1;
        തിരികെ xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

പൂർണ്ണസംഖ്യ
usb_poll_intr(ഘടന usb_pipe *pipe_fl, ശൂന്യം *data)
{
    ASSERT16();
    തിരഞ്ഞെടുപ്പ് (GET_LOWFLAT(pipe_fl->type)) {
    സ്ഥിരസ്ഥിതി:
    അവസ്ഥ USB_TYPE_UHCI:
        തിരികെ uhci_poll_intr(pipe_fl, data);
    അവസ്ഥ USB_TYPE_OHCI:
        തിരികെ ohci_poll_intr(pipe_fl, data);
    അവസ്ഥ USB_TYPE_EHCI:
        തിരികെ ehci_poll_intr(pipe_fl, data);
    അവസ്ഥ USB_TYPE_XHCI: ;
        തിരികെ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

പൂർണ്ണസംഖ്യ usb_32bit_pipe(ഘടന usb_pipe *pipe_fl)
{
    തിരികെ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
ഘടന usb_pipe *
usb_alloc_pipe(ഘടന usbdevice_s *usbdev
               , ഘടന usb_endpoint_descriptor *epdesc)
{
    തിരികെ usb_realloc_pipe(usbdev, ശൂന്യ, epdesc);
}

// Free an allocated control or bulk pipe.
ശൂന്യം
usb_free_pipe(ഘടന usbdevice_s *usbdev, ഘടന usb_pipe *pipe)
{
    എങ്കിൽ (!pipe)
        തിരികെ;
    usb_realloc_pipe(usbdev, pipe, ശൂന്യ);
}

// Send a message to the default control pipe of a device.
പൂർണ്ണസംഖ്യ
usb_send_default_control(ഘടന usb_pipe *pipe, സ്ഥിരം ഘടന usb_ctrlrequest *req
                         , ശൂന്യം *data)
{
    തിരികെ usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
പൂർണ്ണസംഖ്യ
usb_send_bulk(ഘടന usb_pipe *pipe_fl, പൂർണ്ണസംഖ്യ dir, ശൂന്യം *data, പൂർണ്ണസംഖ്യ datasize)
{
    തിരികെ usb_send_pipe(pipe_fl, dir, ശൂന്യ, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
പൂർണ്ണസംഖ്യ
usb_is_freelist(ഘടന usb_s *cntl, ഘടന usb_pipe *pipe)
{
    തിരികെ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
ശൂന്യം
usb_add_freelist(ഘടന usb_pipe *pipe)
{
    എങ്കിൽ (!pipe)
        തിരികെ;
    ഘടന usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
ഘടന usb_pipe *
usb_get_freelist(ഘടന usb_s *cntl, u8 eptype)
{
    ഘടന usb_pipe **pfree = &cntl->freelist;
    വേണ്ടി (;;) {
        ഘടന usb_pipe *pipe = *pfree;
        എങ്കിൽ (!pipe)
            തിരികെ ശൂന്യ;
        എങ്കിൽ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            തിരികെ pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
ശൂന്യം
usb_desc2pipe(ഘടന usb_pipe *pipe, ഘടന usbdevice_s *usbdev
              , ഘടന usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_സംഖ്യ_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
പൂർണ്ണസംഖ്യ
usb_get_period(ഘടന usbdevice_s *usbdev
               , ഘടന usb_endpoint_descriptor *epdesc)
{
    പൂർണ്ണസംഖ്യ period = epdesc->bInterval;
    എങ്കിൽ (usbdev->speed != USB_HIGHSPEED)
        തിരികെ (period <= 0) ? 0 : __fls(period);
    തിരികെ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
പൂർണ്ണസംഖ്യ
usb_xfer_time(ഘടന usb_pipe *pipe, പൂർണ്ണസംഖ്യ datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    എങ്കിൽ (!GET_LOWFLAT(pipe->devaddr))
        തിരികെ USB_TIME_STATUS + 100;
    തിരികെ USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
ഘടന usb_endpoint_descriptor *
usb_find_desc(ഘടന usbdevice_s *usbdev, പൂർണ്ണസംഖ്യ type, പൂർണ്ണസംഖ്യ dir)
{
    ഘടന usb_endpoint_descriptor *epdesc = (ശൂന്യം*)&usbdev->iface[1];
    വേണ്ടി (;;) {
        എങ്കിൽ ((ശൂന്യം*)epdesc >= (ശൂന്യം*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            തിരികെ ശൂന്യ;
        }
        എങ്കിൽ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            തിരികെ epdesc;
        epdesc = (ശൂന്യം*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
സ്ഥിതം പൂർണ്ണസംഖ്യ
get_device_info8(ഘടന usb_pipe *pipe, ഘടന usb_device_descriptor *dinfo)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_മൂല്യം = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    തിരികെ usb_send_default_control(pipe, &req, dinfo);
}

സ്ഥിതം ഘടന usb_config_descriptor *
get_device_config(ഘടന usb_pipe *pipe)
{
    ഘടന usb_config_descriptor cfg;

    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_മൂല്യം = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = വലുപ്പം(cfg);
    പൂർണ്ണസംഖ്യ ret = usb_send_default_control(pipe, &req, &cfg);
    എങ്കിൽ (ret)
        തിരികെ ശൂന്യ;

    ഘടന usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    എങ്കിൽ (!config) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    എങ്കിൽ (ret || config->wTotalLength != cfg.wTotalLength) {
        സ്വതന്ത്രം(config);
        തിരികെ ശൂന്യ;
    }
    //hexdump(config, cfg.wTotalLength);
    തിരികെ config;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
set_configuration(ഘടന usb_pipe *pipe, u16 val)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_മൂല്യം = val;
    req.wIndex = 0;
    req.wLength = 0;
    തിരികെ usb_send_default_control(pipe, &req, ശൂന്യ);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_set_address(ഘടന usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ഘടന usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    എങ്കിൽ (cntl->maxaddr >= USB_MAXADDR)
        തിരികെ -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    ഘടന usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    എങ്കിൽ (!usbdev->defpipe)
        തിരികെ -1;

    // Send set_address command.
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_മൂല്യം = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    പൂർണ്ണസംഖ്യ ret = usb_send_default_control(usbdev->defpipe, &req, ശൂന്യ);
    എങ്കിൽ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        തിരികെ -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    എങ്കിൽ (!usbdev->defpipe)
        തിരികെ -1;
    തിരികെ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
സ്ഥിതം പൂർണ്ണസംഖ്യ
configure_usb_device(ഘടന usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    ഘടന usb_device_descriptor dinfo;
    പൂർണ്ണസംഖ്യ ret = get_device_info8(usbdev->defpipe, &dinfo);
    എങ്കിൽ (ret)
        തിരികെ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    എങ്കിൽ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    എങ്കിൽ (maxpacket < 8)
        തിരികെ 0;
    ഘടന usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    എങ്കിൽ (!usbdev->defpipe)
        തിരികെ -1;

    // Get configuration
    ഘടന usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    എങ്കിൽ (!config)
        തിരികെ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    പൂർണ്ണസംഖ്യ അക്കം_iface = config->b_അക്കം_Interfaces;
    ശൂന്യം *config_end = (ശൂന്യം*)config + config->wTotalLength;
    ഘടന usb_interface_descriptor *iface = (ശൂന്യം*)(&config[1]);
    വേണ്ടി (;;) {
        എങ്കിൽ (!അക്കം_iface || (ശൂന്യം*)iface + iface->bLength > config_end)
            // Not a supported device.
            പോകുക fail;
        എങ്കിൽ (iface->bDescriptorType == USB_DT_INTERFACE) {
            അക്കം_iface--;
            എങ്കിൽ (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                നിർത്തുക;
        }
        iface = (ശൂന്യം*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_മൂല്യം);
    എങ്കിൽ (ret)
        പോകുക fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (ശൂന്യം*)config + config->wTotalLength - (ശൂന്യം*)iface;
    എങ്കിൽ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    അല്ലെങ്കിൽ എങ്കിൽ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        എങ്കിൽ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        എങ്കിൽ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } അല്ലെങ്കിൽ
        ret = usb_hid_setup(usbdev);
    എങ്കിൽ (ret)
        പോകുക fail;

    സ്വതന്ത്രം(config);
    തിരികെ 1;
fail:
    സ്വതന്ത്രം(config);
    തിരികെ 0;
}

സ്ഥിതം ശൂന്യം
usb_hub_port_setup(ശൂന്യം *data)
{
    ഘടന usbdevice_s *usbdev = data;
    ഘടന usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    വേണ്ടി (;;) {
        // Detect if device present (and possibly start reset)
        പൂർണ്ണസംഖ്യ ret = hub->op->detect(hub, port);
        എങ്കിൽ (ret > 0)
            // Device connected.
            നിർത്തുക;
        എങ്കിൽ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            പോകുക done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    പൂർണ്ണസംഖ്യ ret = hub->op->reset(hub, port);
    എങ്കിൽ (ret < 0)
        // Reset failed
        പോകുക resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    എങ്കിൽ (ret) {
        hub->op->disconnect(hub, port);
        പോകുക resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    പൂർണ്ണസംഖ്യ എണ്ണം = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    എങ്കിൽ (!എണ്ണം)
        hub->op->disconnect(hub, port);
    hub->devcount += എണ്ണം;
done:
    hub->threads--;
    സ്വതന്ത്രം(usbdev);
    തിരികെ;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    പോകുക done;
}

സ്ഥിതം u32 usb_time_sigatt;

ശൂന്യം
usb_enumerate(ഘടന usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<portcount; i++) {
        ഘടന usbdevice_s *usbdev = malloc_tmphigh(വലുപ്പം(*usbdev));
        എങ്കിൽ (!usbdev) {
            warn_noalloc();
            തുടരുക;
        }
        memset(usbdev, 0, വലുപ്പം(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    ആകുമ്പോൾ (hub->threads)
        yield();
}

ശൂന്യം
usb_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_USB)
        തിരികെ;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
