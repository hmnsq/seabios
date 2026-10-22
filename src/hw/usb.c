// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-ehci.h" // ehci_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-xhci.h" // xhci_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-hid.h" // usb_keyboard_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-hub.h" // usb_hub_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-msc.h" // usb_msc_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-ohci.h" // ohci_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-uas.h" // usb_uas_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-uhci.h" // uhci_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // msleep
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
ਸਥਿਰ ਬਣਤਰ usb_pipe *
usb_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev, ਬਣਤਰ usb_pipe *pipe
                 , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਚੋਣ (usbdev->hub->cntl->type) {
    ਮੂਲ:
    ਹਾਲਤ USB_TYPE_UHCI:
        ਵਾਪਸ uhci_realloc_pipe(usbdev, pipe, epdesc);
    ਹਾਲਤ USB_TYPE_OHCI:
        ਵਾਪਸ ohci_realloc_pipe(usbdev, pipe, epdesc);
    ਹਾਲਤ USB_TYPE_EHCI:
        ਵਾਪਸ ehci_realloc_pipe(usbdev, pipe, epdesc);
    ਹਾਲਤ USB_TYPE_XHCI:
        ਵਾਪਸ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ਸਥਿਰ ਪੂਰਨ
usb_send_pipe(ਬਣਤਰ usb_pipe *pipe_fl, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
              , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize)
{
    ਚੋਣ (GET_LOWFLAT(pipe_fl->type)) {
    ਮੂਲ:
    ਹਾਲਤ USB_TYPE_UHCI:
        ਵਾਪਸ uhci_send_pipe(pipe_fl, dir, cmd, ਅੰਕੜੇ, datasize);
    ਹਾਲਤ USB_TYPE_OHCI:
        ਜੇ (MODESEGMENT)
            ਵਾਪਸ -1;
        ਵਾਪਸ ohci_send_pipe(pipe_fl, dir, cmd, ਅੰਕੜੇ, datasize);
    ਹਾਲਤ USB_TYPE_EHCI:
        ਵਾਪਸ ehci_send_pipe(pipe_fl, dir, cmd, ਅੰਕੜੇ, datasize);
    ਹਾਲਤ USB_TYPE_XHCI:
        ਜੇ (MODESEGMENT)
            ਵਾਪਸ -1;
        ਵਾਪਸ xhci_send_pipe(pipe_fl, dir, cmd, ਅੰਕੜੇ, datasize);
    }
}

ਪੂਰਨ
usb_poll_intr(ਬਣਤਰ usb_pipe *pipe_fl, ਖਾਲੀ *ਅੰਕੜੇ)
{
    ASSERT16();
    ਚੋਣ (GET_LOWFLAT(pipe_fl->type)) {
    ਮੂਲ:
    ਹਾਲਤ USB_TYPE_UHCI:
        ਵਾਪਸ uhci_poll_intr(pipe_fl, ਅੰਕੜੇ);
    ਹਾਲਤ USB_TYPE_OHCI:
        ਵਾਪਸ ohci_poll_intr(pipe_fl, ਅੰਕੜੇ);
    ਹਾਲਤ USB_TYPE_EHCI:
        ਵਾਪਸ ehci_poll_intr(pipe_fl, ਅੰਕੜੇ);
    ਹਾਲਤ USB_TYPE_XHCI: ;
        ਵਾਪਸ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), ਅੰਕੜੇ), 0, -1);
    }
}

ਪੂਰਨ usb_32bit_pipe(ਬਣਤਰ usb_pipe *pipe_fl)
{
    ਵਾਪਸ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
ਬਣਤਰ usb_pipe *
usb_alloc_pipe(ਬਣਤਰ usbdevice_s *usbdev
               , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਵਾਪਸ usb_realloc_pipe(usbdev, ਨਿਰਾ, epdesc);
}

// Free an allocated control or bulk pipe.
ਖਾਲੀ
usb_free_pipe(ਬਣਤਰ usbdevice_s *usbdev, ਬਣਤਰ usb_pipe *pipe)
{
    ਜੇ (!pipe)
        ਵਾਪਸ;
    usb_realloc_pipe(usbdev, pipe, ਨਿਰਾ);
}

// Send a message to the default control pipe of a device.
ਪੂਰਨ
usb_send_default_control(ਬਣਤਰ usb_pipe *pipe, ਅਚਲ ਬਣਤਰ usb_ctrlrequest *req
                         , ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਵਾਪਸ usb_send_pipe(pipe, req->bRequestType & USB_DIR_ਵਿੱਚ, req
                         , ਅੰਕੜੇ, req->w_ਲੰਮਾਈ);
}

// Send a message to a bulk endpoint
ਪੂਰਨ
usb_send_bulk(ਬਣਤਰ usb_pipe *pipe_fl, ਪੂਰਨ dir, ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize)
{
    ਵਾਪਸ usb_send_pipe(pipe_fl, dir, ਨਿਰਾ, ਅੰਕੜੇ, datasize);
}

// Check if a pipe for a given controller is on the freelist
ਪੂਰਨ
usb_is_freelist(ਬਣਤਰ usb_s *cntl, ਬਣਤਰ usb_pipe *pipe)
{
    ਵਾਪਸ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
ਖਾਲੀ
usb_ਜੋੜੋ_freelist(ਬਣਤਰ usb_pipe *pipe)
{
    ਜੇ (!pipe)
        ਵਾਪਸ;
    ਬਣਤਰ usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
ਬਣਤਰ usb_pipe *
usb_get_freelist(ਬਣਤਰ usb_s *cntl, u8 eptype)
{
    ਬਣਤਰ usb_pipe **pfree = &cntl->freelist;
    ਲਈ (;;) {
        ਬਣਤਰ usb_pipe *pipe = *pfree;
        ਜੇ (!pipe)
            ਵਾਪਸ ਨਿਰਾ;
        ਜੇ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ਵਾਪਸ pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
ਖਾਲੀ
usb_desc2pipe(ਬਣਤਰ usb_pipe *pipe, ਬਣਤਰ usbdevice_s *usbdev
              , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->b_Endpoint_ਪਤਾ & USB_ENDPOINT_ਸੰਖਿਆ_ਨਕਾਬ;
    pipe->devaddr = usbdev->devaddr;
    pipe->ਰਫਤਾਰ = usbdev->ਰਫਤਾਰ;
    pipe->maxpacket = epdesc->w_Max_Packet_ਮਾਪ;
    pipe->eptype = epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ;
}

// Find the exponential period of the requested interrupt end point.
ਪੂਰਨ
usb_get_period(ਬਣਤਰ usbdevice_s *usbdev
               , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਪੂਰਨ period = epdesc->bInterval;
    ਜੇ (usbdev->ਰਫਤਾਰ != USB_HIGHSPEED)
        ਵਾਪਸ (period <= 0) ? 0 : __fls(period);
    ਵਾਪਸ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
ਪੂਰਨ
usb_xfer_ਸਮਾਂ(ਬਣਤਰ usb_pipe *pipe, ਪੂਰਨ datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ਜੇ (!GET_LOWFLAT(pipe->devaddr))
        ਵਾਪਸ USB_ਸਮਾਂ_ਸਥਿਤੀ + 100;
    ਵਾਪਸ USB_ਸਮਾਂ_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
ਬਣਤਰ usb_endpoint_descriptor *
usb_ਲੱਭੋ_desc(ਬਣਤਰ usbdevice_s *usbdev, ਪੂਰਨ type, ਪੂਰਨ dir)
{
    ਬਣਤਰ usb_endpoint_descriptor *epdesc = (ਖਾਲੀ*)&usbdev->iface[1];
    ਲਈ (;;) {
        ਜੇ ((ਖਾਲੀ*)epdesc >= (ਖਾਲੀ*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ਵਾਪਸ ਨਿਰਾ;
        }
        ਜੇ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->b_Endpoint_ਪਤਾ & USB_ENDPOINT_DIR_ਨਕਾਬ) == dir
            && (epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ) == type)
            ਵਾਪਸ epdesc;
        epdesc = (ਖਾਲੀ*)epdesc + epdesc->b_ਲੰਮਾਈ;
    }
}

// Get the first 8 bytes of the device descriptor.
ਸਥਿਰ ਪੂਰਨ
get_device_info8(ਬਣਤਰ usb_pipe *pipe, ਬਣਤਰ usb_device_descriptor *dinfo)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਵਿੱਚ | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ਮੁੱਲ = USB_DT_DEVICE<<8;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = 8;
    ਵਾਪਸ usb_send_default_control(pipe, &req, dinfo);
}

ਸਥਿਰ ਬਣਤਰ usb_config_descriptor *
get_device_config(ਬਣਤਰ usb_pipe *pipe)
{
    ਬਣਤਰ usb_config_descriptor cfg;

    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਵਿੱਚ | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ਮੁੱਲ = USB_DT_CONFIG<<8;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = ਆਕਾਰ(cfg);
    ਪੂਰਨ ret = usb_send_default_control(pipe, &req, &cfg);
    ਜੇ (ret)
        ਵਾਪਸ ਨਿਰਾ;

    ਬਣਤਰ usb_config_descriptor *config = malloc_tmphigh(cfg.w_ਕੁੱਲ_ਲੰਮਾਈ);
    ਜੇ (!config) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    req.w_ਲੰਮਾਈ = cfg.w_ਕੁੱਲ_ਲੰਮਾਈ;
    ret = usb_send_default_control(pipe, &req, config);
    ਜੇ (ret || config->w_ਕੁੱਲ_ਲੰਮਾਈ != cfg.w_ਕੁੱਲ_ਲੰਮਾਈ) {
        ਮੁਕਤ(config);
        ਵਾਪਸ ਨਿਰਾ;
    }
    //hexdump(config, cfg.wTotalLength);
    ਵਾਪਸ config;
}

ਸਥਿਰ ਪੂਰਨ
ਸੈੱਟ_configuration(ਬਣਤਰ usb_pipe *pipe, u16 val)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਬਾਹਰਲਾ | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_ਸੈੱਟ_CONFIGURATION;
    req.w_ਮੁੱਲ = val;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = 0;
    ਵਾਪਸ usb_send_default_control(pipe, &req, ਨਿਰਾ);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਰਫਤਾਰ_ਮੰਜ਼ਿਲ_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ਸਥਿਰ ਪੂਰਨ
usb_ਸੈੱਟ_ਪਤਾ(ਬਣਤਰ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ਬਣਤਰ usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ਜੇ (cntl->maxaddr >= USB_MAXADDR)
        ਵਾਪਸ -1;

    msleep(USB_ਸਮਾਂ_RSTRCY);

    // Create a pipe for the default address.
    ਬਣਤਰ usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_ਮਾਪ = ਰਫਤਾਰ_ਮੰਜ਼ਿਲ_ctlsize[usbdev->ਰਫਤਾਰ],
        .bm_ਗੁਣ = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ਜੇ (!usbdev->defpipe)
        ਵਾਪਸ -1;

    // Send set_address command.
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਬਾਹਰਲਾ | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_ਸੈੱਟ_ਪਤਾ;
    req.w_ਮੁੱਲ = cntl->maxaddr + 1;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = 0;
    ਪੂਰਨ ret = usb_send_default_control(usbdev->defpipe, &req, ਨਿਰਾ);
    ਜੇ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ਵਾਪਸ -1;
    }

    msleep(USB_ਸਮਾਂ_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ਜੇ (!usbdev->defpipe)
        ਵਾਪਸ -1;
    ਵਾਪਸ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ਸਥਿਰ ਪੂਰਨ
configure_usb_device(ਬਣਤਰ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    ਬਣਤਰ usb_device_descriptor dinfo;
    ਪੂਰਨ ret = get_device_info8(usbdev->defpipe, &dinfo);
    ਜੇ (ret)
        ਵਾਪਸ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ਜੇ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ਜੇ (maxpacket < 8)
        ਵਾਪਸ 0;
    ਬਣਤਰ usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_ਮਾਪ = maxpacket,
        .bm_ਗੁਣ = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ਜੇ (!usbdev->defpipe)
        ਵਾਪਸ -1;

    // Get configuration
    ਬਣਤਰ usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ਜੇ (!config)
        ਵਾਪਸ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    ਪੂਰਨ ਅੰਕ_iface = config->b_ਅੰਕ_Interfaces;
    ਖਾਲੀ *config_ਸਿਰਾ = (ਖਾਲੀ*)config + config->w_ਕੁੱਲ_ਲੰਮਾਈ;
    ਬਣਤਰ usb_interface_descriptor *iface = (ਖਾਲੀ*)(&config[1]);
    ਲਈ (;;) {
        ਜੇ (!ਅੰਕ_iface || (ਖਾਲੀ*)iface + iface->b_ਲੰਮਾਈ > config_ਸਿਰਾ)
            // Not a supported device.
            ਜਾਓ fail;
        ਜੇ (iface->bDescriptorType == USB_DT_INTERFACE) {
            ਅੰਕ_iface--;
            ਜੇ (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                ਰੋਕੋ;
        }
        iface = (ਖਾਲੀ*)iface + iface->b_ਲੰਮਾਈ;
    }

    // Set the configuration.
    ret = ਸੈੱਟ_configuration(usbdev->defpipe, config->b_Configuration_ਮੁੱਲ);
    ਜੇ (ret)
        ਜਾਓ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (ਖਾਲੀ*)config + config->w_ਕੁੱਲ_ਲੰਮਾਈ - (ਖਾਲੀ*)iface;
    ਜੇ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_ਤਿਆਰੀ(usbdev);
    ਨਹੀਂਤਾਂ ਜੇ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ਜੇ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_ਤਿਆਰੀ(usbdev);
        ਜੇ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_ਤਿਆਰੀ(usbdev);
    } ਨਹੀਂਤਾਂ
        ret = usb_hid_ਤਿਆਰੀ(usbdev);
    ਜੇ (ret)
        ਜਾਓ fail;

    ਮੁਕਤ(config);
    ਵਾਪਸ 1;
fail:
    ਮੁਕਤ(config);
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ
usb_hub_ਪੋਰਟ_ਤਿਆਰੀ(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ usbdevice_s *usbdev = ਅੰਕੜੇ;
    ਬਣਤਰ usbhub_s *hub = usbdev->hub;
    u32 ਪੋਰਟ = usbdev->ਪੋਰਟ;

    ਲਈ (;;) {
        // Detect if device present (and possibly start reset)
        ਪੂਰਨ ret = hub->op->detect(hub, ਪੋਰਟ);
        ਜੇ (ret > 0)
            // Device connected.
            ਰੋਕੋ;
        ਜੇ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            ਜਾਓ ਪੂਰਾ;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_ਤਾਲਾ(&hub->cntl->resetlock);
    ਪੂਰਨ ret = hub->op->reset(hub, ਪੋਰਟ);
    ਜੇ (ret < 0)
        // Reset failed
        ਜਾਓ resetfail;
    usbdev->ਰਫਤਾਰ = ret;

    // Set address of port
    ret = usb_ਸੈੱਟ_ਪਤਾ(usbdev);
    ਜੇ (ret) {
        hub->op->disconnect(hub, ਪੋਰਟ);
        ਜਾਓ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    ਪੂਰਨ ਗਿਣਤੀ_2 = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ਜੇ (!ਗਿਣਤੀ_2)
        hub->op->disconnect(hub, ਪੋਰਟ);
    hub->devcount += ਗਿਣਤੀ_2;
ਪੂਰਾ:
    hub->ਧਾਗੇ--;
    ਮੁਕਤ(usbdev);
    ਵਾਪਸ;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    ਜਾਓ ਪੂਰਾ;
}

ਸਥਿਰ u32 usb_ਸਮਾਂ_sigatt;

ਖਾਲੀ
usb_enumerate(ਬਣਤਰ usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->ਧਾਗੇ = portcount;
    hub->detectend = timer_calc(usb_ਸਮਾਂ_sigatt);

    // Launch a thread for every port.
    ਪੂਰਨ i;
    ਲਈ (i=0; i<portcount; i++) {
        ਬਣਤਰ usbdevice_s *usbdev = malloc_tmphigh(ਆਕਾਰ(*usbdev));
        ਜੇ (!usbdev) {
            warn_noalloc();
            ਜਾਰੀ;
        }
        memset(usbdev, 0, ਆਕਾਰ(*usbdev));
        usbdev->hub = hub;
        usbdev->ਪੋਰਟ = i;
        ਲੜੀਵਾਰ_thread(usb_hub_ਪੋਰਟ_ਤਿਆਰੀ, usbdev);
    }

    // Wait for threads to complete.
    ਜਦੋਂ (hub->ਧਾਗੇ)
        yield();
}

ਖਾਲੀ
usb_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_USB)
        ਵਾਪਸ;
    dprintf(3, "init usb\n");
    usb_ਸਮਾਂ_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_ਸਮਾਂ_SIGATT);
    xhci_ਤਿਆਰੀ();
    ehci_ਤਿਆਰੀ();
    uhci_ਤਿਆਰੀ();
    ohci_ਤਿਆਰੀ();
}
