// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-ehci.h" // ehci_setup
#ಅನ್ತರ್ಭಾವ "usb-xhci.h" // xhci_setup
#ಅನ್ತರ್ಭಾವ "usb-hid.h" // usb_keyboard_setup
#ಅನ್ತರ್ಭಾವ "usb-hub.h" // usb_hub_setup
#ಅನ್ತರ್ಭಾವ "usb-msc.h" // usb_msc_setup
#ಅನ್ತರ್ಭಾವ "usb-ohci.h" // ohci_setup
#ಅನ್ತರ್ಭಾವ "usb-uas.h" // usb_uas_setup
#ಅನ್ತರ್ಭಾವ "usb-uhci.h" // uhci_setup
#ಅನ್ತರ್ಭಾವ "util.h" // msleep
#ಅನ್ತರ್ಭಾವ "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
ಸ್ಥಿರ ಸಂರಚನಾ usb_pipe *
usb_realloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev, ಸಂರಚನಾ usb_pipe *pipe
                 , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ವಿಕಲ್ಪ (usbdev->hub->cntl->type) {
    ಸ್ವಭಾವ:
    ಪಕ್ಷ USB_TYPE_UHCI:
        ಪ್ರತ್ಯಾಗಮ uhci_realloc_pipe(usbdev, pipe, epdesc);
    ಪಕ್ಷ USB_TYPE_OHCI:
        ಪ್ರತ್ಯಾಗಮ ohci_realloc_pipe(usbdev, pipe, epdesc);
    ಪಕ್ಷ USB_TYPE_EHCI:
        ಪ್ರತ್ಯಾಗಮ ehci_realloc_pipe(usbdev, pipe, epdesc);
    ಪಕ್ಷ USB_TYPE_XHCI:
        ಪ್ರತ್ಯಾಗಮ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_send_pipe(ಸಂರಚನಾ usb_pipe *pipe_fl, ಪೂರ್ಣಾಙ್ಕ dir, ಧ್ರುವ ಶೂನ್ಯ *cmd
              , ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datasize)
{
    ವಿಕಲ್ಪ (GET_LOWFLAT(pipe_fl->type)) {
    ಸ್ವಭಾವ:
    ಪಕ್ಷ USB_TYPE_UHCI:
        ಪ್ರತ್ಯಾಗಮ uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    ಪಕ್ಷ USB_TYPE_OHCI:
        ಯದಿ (MODESEGMENT)
            ಪ್ರತ್ಯಾಗಮ -1;
        ಪ್ರತ್ಯಾಗಮ ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    ಪಕ್ಷ USB_TYPE_EHCI:
        ಪ್ರತ್ಯಾಗಮ ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    ಪಕ್ಷ USB_TYPE_XHCI:
        ಯದಿ (MODESEGMENT)
            ಪ್ರತ್ಯಾಗಮ -1;
        ಪ್ರತ್ಯಾಗಮ xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

ಪೂರ್ಣಾಙ್ಕ
usb_poll_intr(ಸಂರಚನಾ usb_pipe *pipe_fl, ಶೂನ್ಯ *data)
{
    ASSERT16();
    ವಿಕಲ್ಪ (GET_LOWFLAT(pipe_fl->type)) {
    ಸ್ವಭಾವ:
    ಪಕ್ಷ USB_TYPE_UHCI:
        ಪ್ರತ್ಯಾಗಮ uhci_poll_intr(pipe_fl, data);
    ಪಕ್ಷ USB_TYPE_OHCI:
        ಪ್ರತ್ಯಾಗಮ ohci_poll_intr(pipe_fl, data);
    ಪಕ್ಷ USB_TYPE_EHCI:
        ಪ್ರತ್ಯಾಗಮ ehci_poll_intr(pipe_fl, data);
    ಪಕ್ಷ USB_TYPE_XHCI: ;
        ಪ್ರತ್ಯಾಗಮ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

ಪೂರ್ಣಾಙ್ಕ usb_32bit_pipe(ಸಂರಚನಾ usb_pipe *pipe_fl)
{
    ಪ್ರತ್ಯಾಗಮ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
ಸಂರಚನಾ usb_pipe *
usb_alloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev
               , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಪ್ರತ್ಯಾಗಮ usb_realloc_pipe(usbdev, ರಿಕ್ತ, epdesc);
}

// Free an allocated control or bulk pipe.
ಶೂನ್ಯ
usb_free_pipe(ಸಂರಚನಾ usbdevice_s *usbdev, ಸಂರಚನಾ usb_pipe *pipe)
{
    ಯದಿ (!pipe)
        ಪ್ರತ್ಯಾಗಮ;
    usb_realloc_pipe(usbdev, pipe, ರಿಕ್ತ);
}

// Send a message to the default control pipe of a device.
ಪೂರ್ಣಾಙ್ಕ
usb_send_default_control(ಸಂರಚನಾ usb_pipe *pipe, ಧ್ರುವ ಸಂರಚನಾ usb_ctrlrequest *req
                         , ಶೂನ್ಯ *data)
{
    ಪ್ರತ್ಯಾಗಮ usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
ಪೂರ್ಣಾಙ್ಕ
usb_send_bulk(ಸಂರಚನಾ usb_pipe *pipe_fl, ಪೂರ್ಣಾಙ್ಕ dir, ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datasize)
{
    ಪ್ರತ್ಯಾಗಮ usb_send_pipe(pipe_fl, dir, ರಿಕ್ತ, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
ಪೂರ್ಣಾಙ್ಕ
usb_is_freelist(ಸಂರಚನಾ usb_s *cntl, ಸಂರಚನಾ usb_pipe *pipe)
{
    ಪ್ರತ್ಯಾಗಮ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
ಶೂನ್ಯ
usb_add_freelist(ಸಂರಚನಾ usb_pipe *pipe)
{
    ಯದಿ (!pipe)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
ಸಂರಚನಾ usb_pipe *
usb_get_freelist(ಸಂರಚನಾ usb_s *cntl, u8 eptype)
{
    ಸಂರಚನಾ usb_pipe **pfree = &cntl->freelist;
    ಪ್ರತಿ (;;) {
        ಸಂರಚನಾ usb_pipe *pipe = *pfree;
        ಯದಿ (!pipe)
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        ಯದಿ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ಪ್ರತ್ಯಾಗಮ pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
ಶೂನ್ಯ
usb_desc2pipe(ಸಂರಚನಾ usb_pipe *pipe, ಸಂರಚನಾ usbdevice_s *usbdev
              , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_ಸಂಖ್ಯೆ_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
ಪೂರ್ಣಾಙ್ಕ
usb_get_period(ಸಂರಚನಾ usbdevice_s *usbdev
               , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಪೂರ್ಣಾಙ್ಕ period = epdesc->bInterval;
    ಯದಿ (usbdev->speed != USB_HIGHSPEED)
        ಪ್ರತ್ಯಾಗಮ (period <= 0) ? 0 : __fls(period);
    ಪ್ರತ್ಯಾಗಮ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
ಪೂರ್ಣಾಙ್ಕ
usb_xfer_time(ಸಂರಚನಾ usb_pipe *pipe, ಪೂರ್ಣಾಙ್ಕ datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ಯದಿ (!GET_LOWFLAT(pipe->devaddr))
        ಪ್ರತ್ಯಾಗಮ USB_TIME_STATUS + 100;
    ಪ್ರತ್ಯಾಗಮ USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
ಸಂರಚನಾ usb_endpoint_descriptor *
usb_find_desc(ಸಂರಚನಾ usbdevice_s *usbdev, ಪೂರ್ಣಾಙ್ಕ type, ಪೂರ್ಣಾಙ್ಕ dir)
{
    ಸಂರಚನಾ usb_endpoint_descriptor *epdesc = (ಶೂನ್ಯ*)&usbdev->iface[1];
    ಪ್ರತಿ (;;) {
        ಯದಿ ((ಶೂನ್ಯ*)epdesc >= (ಶೂನ್ಯ*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        }
        ಯದಿ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            ಪ್ರತ್ಯಾಗಮ epdesc;
        epdesc = (ಶೂನ್ಯ*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
get_device_info8(ಸಂರಚನಾ usb_pipe *pipe, ಸಂರಚನಾ usb_device_descriptor *dinfo)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ಮೌಲ್ಯ = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    ಪ್ರತ್ಯಾಗಮ usb_send_default_control(pipe, &req, dinfo);
}

ಸ್ಥಿರ ಸಂರಚನಾ usb_config_descriptor *
get_device_config(ಸಂರಚನಾ usb_pipe *pipe)
{
    ಸಂರಚನಾ usb_config_descriptor cfg;

    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ಮೌಲ್ಯ = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = ಪರಿಮಾಣ(cfg);
    ಪೂರ್ಣಾಙ್ಕ ret = usb_send_default_control(pipe, &req, &cfg);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    ಸಂರಚನಾ usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    ಯದಿ (!config) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    ಯದಿ (ret || config->wTotalLength != cfg.wTotalLength) {
        ಮುಕ್ತ(config);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    //hexdump(config, cfg.wTotalLength);
    ಪ್ರತ್ಯಾಗಮ config;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
set_configuration(ಸಂರಚನಾ usb_pipe *pipe, u16 val)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_ಮೌಲ್ಯ = val;
    req.wIndex = 0;
    req.wLength = 0;
    ಪ್ರತ್ಯಾಗಮ usb_send_default_control(pipe, &req, ರಿಕ್ತ);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_set_address(ಸಂರಚನಾ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ಸಂರಚನಾ usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ಯದಿ (cntl->maxaddr >= USB_MAXADDR)
        ಪ್ರತ್ಯಾಗಮ -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    ಸಂರಚನಾ usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ಯದಿ (!usbdev->defpipe)
        ಪ್ರತ್ಯಾಗಮ -1;

    // Send set_address command.
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_ಮೌಲ್ಯ = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    ಪೂರ್ಣಾಙ್ಕ ret = usb_send_default_control(usbdev->defpipe, &req, ರಿಕ್ತ);
    ಯದಿ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ಯದಿ (!usbdev->defpipe)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
configure_usb_device(ಸಂರಚನಾ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    ಸಂರಚನಾ usb_device_descriptor dinfo;
    ಪೂರ್ಣಾಙ್ಕ ret = get_device_info8(usbdev->defpipe, &dinfo);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ಯದಿ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ಯದಿ (maxpacket < 8)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಸಂರಚನಾ usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ಯದಿ (!usbdev->defpipe)
        ಪ್ರತ್ಯಾಗಮ -1;

    // Get configuration
    ಸಂರಚನಾ usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ಯದಿ (!config)
        ಪ್ರತ್ಯಾಗಮ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_iface = config->b_ಅಂಕೆ_Interfaces;
    ಶೂನ್ಯ *config_end = (ಶೂನ್ಯ*)config + config->wTotalLength;
    ಸಂರಚನಾ usb_interface_descriptor *iface = (ಶೂನ್ಯ*)(&config[1]);
    ಪ್ರತಿ (;;) {
        ಯದಿ (!ಅಂಕೆ_iface || (ಶೂನ್ಯ*)iface + iface->bLength > config_end)
            // Not a supported device.
            ಗಚ್ಛ fail;
        ಯದಿ (iface->bDescriptorType == USB_DT_INTERFACE) {
            ಅಂಕೆ_iface--;
            ಯದಿ (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                ಭಙ್ಗ;
        }
        iface = (ಶೂನ್ಯ*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_ಮೌಲ್ಯ);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (ಶೂನ್ಯ*)config + config->wTotalLength - (ಶೂನ್ಯ*)iface;
    ಯದಿ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    ಅನ್ಯಥಾ ಯದಿ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ಯದಿ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        ಯದಿ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } ಅನ್ಯಥಾ
        ret = usb_hid_setup(usbdev);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;

    ಮುಕ್ತ(config);
    ಪ್ರತ್ಯಾಗಮ 1;
fail:
    ಮುಕ್ತ(config);
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
usb_hub_port_setup(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ usbdevice_s *usbdev = data;
    ಸಂರಚನಾ usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    ಪ್ರತಿ (;;) {
        // Detect if device present (and possibly start reset)
        ಪೂರ್ಣಾಙ್ಕ ret = hub->op->detect(hub, port);
        ಯದಿ (ret > 0)
            // Device connected.
            ಭಙ್ಗ;
        ಯದಿ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            ಗಚ್ಛ done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    ಪೂರ್ಣಾಙ್ಕ ret = hub->op->reset(hub, port);
    ಯದಿ (ret < 0)
        // Reset failed
        ಗಚ್ಛ resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    ಯದಿ (ret) {
        hub->op->disconnect(hub, port);
        ಗಚ್ಛ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ಯದಿ (!ಎಣಿಕೆ)
        hub->op->disconnect(hub, port);
    hub->devcount += ಎಣಿಕೆ;
done:
    hub->threads--;
    ಮುಕ್ತ(usbdev);
    ಪ್ರತ್ಯಾಗಮ;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    ಗಚ್ಛ done;
}

ಸ್ಥಿರ u32 usb_time_sigatt;

ಶೂನ್ಯ
usb_enumerate(ಸಂರಚನಾ usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<portcount; i++) {
        ಸಂರಚನಾ usbdevice_s *usbdev = malloc_tmphigh(ಪರಿಮಾಣ(*usbdev));
        ಯದಿ (!usbdev) {
            warn_noalloc();
            ಅನುವರ್ತ;
        }
        memset(usbdev, 0, ಪರಿಮಾಣ(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    ಯಾವತ್ (hub->threads)
        yield();
}

ಶೂನ್ಯ
usb_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_USB)
        ಪ್ರತ್ಯಾಗಮ;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
