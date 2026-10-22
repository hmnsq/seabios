// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-ehci.h" // ehci_setup
#અન્તર્ભાવ "usb-xhci.h" // xhci_setup
#અન્તર્ભાવ "usb-hid.h" // usb_keyboard_setup
#અન્તર્ભાવ "usb-hub.h" // usb_hub_setup
#અન્તર્ભાવ "usb-msc.h" // usb_msc_setup
#અન્તર્ભાવ "usb-ohci.h" // ohci_setup
#અન્તર્ભાવ "usb-uas.h" // usb_uas_setup
#અન્તર્ભાવ "usb-uhci.h" // uhci_setup
#અન્તર્ભાવ "util.h" // msleep
#અન્તર્ભાવ "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
સ્થિર સંરચના usb_pipe *
usb_realloc_pipe(સંરચના usbdevice_s *usbdev, સંરચના usb_pipe *pipe
                 , સંરચના usb_endpoint_descriptor *epdesc)
{
    પસંદગી (usbdev->hub->cntl->type) {
    મૂળભૂત:
    સ્થિતિ USB_TYPE_UHCI:
        પાછા uhci_realloc_pipe(usbdev, pipe, epdesc);
    સ્થિતિ USB_TYPE_OHCI:
        પાછા ohci_realloc_pipe(usbdev, pipe, epdesc);
    સ્થિતિ USB_TYPE_EHCI:
        પાછા ehci_realloc_pipe(usbdev, pipe, epdesc);
    સ્થિતિ USB_TYPE_XHCI:
        પાછા xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
સ્થિર પૂર્ણાંક
usb_send_pipe(સંરચના usb_pipe *pipe_fl, પૂર્ણાંક dir, અચલ શૂન્ય *cmd
              , શૂન્ય *data, પૂર્ણાંક datasize)
{
    પસંદગી (GET_LOWFLAT(pipe_fl->type)) {
    મૂળભૂત:
    સ્થિતિ USB_TYPE_UHCI:
        પાછા uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    સ્થિતિ USB_TYPE_OHCI:
        જો (MODESEGMENT)
            પાછા -1;
        પાછા ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    સ્થિતિ USB_TYPE_EHCI:
        પાછા ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    સ્થિતિ USB_TYPE_XHCI:
        જો (MODESEGMENT)
            પાછા -1;
        પાછા xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

પૂર્ણાંક
usb_poll_intr(સંરચના usb_pipe *pipe_fl, શૂન્ય *data)
{
    ASSERT16();
    પસંદગી (GET_LOWFLAT(pipe_fl->type)) {
    મૂળભૂત:
    સ્થિતિ USB_TYPE_UHCI:
        પાછા uhci_poll_intr(pipe_fl, data);
    સ્થિતિ USB_TYPE_OHCI:
        પાછા ohci_poll_intr(pipe_fl, data);
    સ્થિતિ USB_TYPE_EHCI:
        પાછા ehci_poll_intr(pipe_fl, data);
    સ્થિતિ USB_TYPE_XHCI: ;
        પાછા call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

પૂર્ણાંક usb_32bit_pipe(સંરચના usb_pipe *pipe_fl)
{
    પાછા (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
સંરચના usb_pipe *
usb_alloc_pipe(સંરચના usbdevice_s *usbdev
               , સંરચના usb_endpoint_descriptor *epdesc)
{
    પાછા usb_realloc_pipe(usbdev, ખાલી, epdesc);
}

// Free an allocated control or bulk pipe.
શૂન્ય
usb_free_pipe(સંરચના usbdevice_s *usbdev, સંરચના usb_pipe *pipe)
{
    જો (!pipe)
        પાછા;
    usb_realloc_pipe(usbdev, pipe, ખાલી);
}

// Send a message to the default control pipe of a device.
પૂર્ણાંક
usb_send_default_control(સંરચના usb_pipe *pipe, અચલ સંરચના usb_ctrlrequest *req
                         , શૂન્ય *data)
{
    પાછા usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
પૂર્ણાંક
usb_send_bulk(સંરચના usb_pipe *pipe_fl, પૂર્ણાંક dir, શૂન્ય *data, પૂર્ણાંક datasize)
{
    પાછા usb_send_pipe(pipe_fl, dir, ખાલી, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
પૂર્ણાંક
usb_is_freelist(સંરચના usb_s *cntl, સંરચના usb_pipe *pipe)
{
    પાછા pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
શૂન્ય
usb_add_freelist(સંરચના usb_pipe *pipe)
{
    જો (!pipe)
        પાછા;
    સંરચના usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
સંરચના usb_pipe *
usb_get_freelist(સંરચના usb_s *cntl, u8 eptype)
{
    સંરચના usb_pipe **pfree = &cntl->freelist;
    માટે (;;) {
        સંરચના usb_pipe *pipe = *pfree;
        જો (!pipe)
            પાછા ખાલી;
        જો (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            પાછા pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
શૂન્ય
usb_desc2pipe(સંરચના usb_pipe *pipe, સંરચના usbdevice_s *usbdev
              , સંરચના usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_સંખ્યા_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
પૂર્ણાંક
usb_get_period(સંરચના usbdevice_s *usbdev
               , સંરચના usb_endpoint_descriptor *epdesc)
{
    પૂર્ણાંક period = epdesc->bInterval;
    જો (usbdev->speed != USB_HIGHSPEED)
        પાછા (period <= 0) ? 0 : __fls(period);
    પાછા (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
પૂર્ણાંક
usb_xfer_time(સંરચના usb_pipe *pipe, પૂર્ણાંક datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    જો (!GET_LOWFLAT(pipe->devaddr))
        પાછા USB_TIME_STATUS + 100;
    પાછા USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
સંરચના usb_endpoint_descriptor *
usb_find_desc(સંરચના usbdevice_s *usbdev, પૂર્ણાંક type, પૂર્ણાંક dir)
{
    સંરચના usb_endpoint_descriptor *epdesc = (શૂન્ય*)&usbdev->iface[1];
    માટે (;;) {
        જો ((શૂન્ય*)epdesc >= (શૂન્ય*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            પાછા ખાલી;
        }
        જો (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            પાછા epdesc;
        epdesc = (શૂન્ય*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
સ્થિર પૂર્ણાંક
get_device_info8(સંરચના usb_pipe *pipe, સંરચના usb_device_descriptor *dinfo)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_કિંમત = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    પાછા usb_send_default_control(pipe, &req, dinfo);
}

સ્થિર સંરચના usb_config_descriptor *
get_device_config(સંરચના usb_pipe *pipe)
{
    સંરચના usb_config_descriptor cfg;

    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_કિંમત = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = કદ(cfg);
    પૂર્ણાંક ret = usb_send_default_control(pipe, &req, &cfg);
    જો (ret)
        પાછા ખાલી;

    સંરચના usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    જો (!config) {
        warn_noalloc();
        પાછા ખાલી;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    જો (ret || config->wTotalLength != cfg.wTotalLength) {
        મુક્ત(config);
        પાછા ખાલી;
    }
    //hexdump(config, cfg.wTotalLength);
    પાછા config;
}

સ્થિર પૂર્ણાંક
set_configuration(સંરચના usb_pipe *pipe, u16 val)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_કિંમત = val;
    req.wIndex = 0;
    req.wLength = 0;
    પાછા usb_send_default_control(pipe, &req, ખાલી);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

સ્થિર અચલ પૂર્ણાંક speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
સ્થિર પૂર્ણાંક
usb_set_address(સંરચના usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    સંરચના usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    જો (cntl->maxaddr >= USB_MAXADDR)
        પાછા -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    સંરચના usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    જો (!usbdev->defpipe)
        પાછા -1;

    // Send set_address command.
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_કિંમત = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    પૂર્ણાંક ret = usb_send_default_control(usbdev->defpipe, &req, ખાલી);
    જો (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        પાછા -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    જો (!usbdev->defpipe)
        પાછા -1;
    પાછા 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
સ્થિર પૂર્ણાંક
configure_usb_device(સંરચના usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    સંરચના usb_device_descriptor dinfo;
    પૂર્ણાંક ret = get_device_info8(usbdev->defpipe, &dinfo);
    જો (ret)
        પાછા 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    જો (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    જો (maxpacket < 8)
        પાછા 0;
    સંરચના usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    જો (!usbdev->defpipe)
        પાછા -1;

    // Get configuration
    સંરચના usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    જો (!config)
        પાછા 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    પૂર્ણાંક અંક_iface = config->b_અંક_Interfaces;
    શૂન્ય *config_end = (શૂન્ય*)config + config->wTotalLength;
    સંરચના usb_interface_descriptor *iface = (શૂન્ય*)(&config[1]);
    માટે (;;) {
        જો (!અંક_iface || (શૂન્ય*)iface + iface->bLength > config_end)
            // Not a supported device.
            જાઓ fail;
        જો (iface->bDescriptorType == USB_DT_INTERFACE) {
            અંક_iface--;
            જો (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                તોડો;
        }
        iface = (શૂન્ય*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_કિંમત);
    જો (ret)
        જાઓ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (શૂન્ય*)config + config->wTotalLength - (શૂન્ય*)iface;
    જો (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    નહીંતર જો (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        જો (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        જો (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } નહીંતર
        ret = usb_hid_setup(usbdev);
    જો (ret)
        જાઓ fail;

    મુક્ત(config);
    પાછા 1;
fail:
    મુક્ત(config);
    પાછા 0;
}

સ્થિર શૂન્ય
usb_hub_port_setup(શૂન્ય *data)
{
    સંરચના usbdevice_s *usbdev = data;
    સંરચના usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    માટે (;;) {
        // Detect if device present (and possibly start reset)
        પૂર્ણાંક ret = hub->op->detect(hub, port);
        જો (ret > 0)
            // Device connected.
            તોડો;
        જો (ret < 0 || timer_check(hub->detectend))
            // No device found.
            જાઓ done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    પૂર્ણાંક ret = hub->op->reset(hub, port);
    જો (ret < 0)
        // Reset failed
        જાઓ resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    જો (ret) {
        hub->op->disconnect(hub, port);
        જાઓ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    પૂર્ણાંક ગણતરી = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    જો (!ગણતરી)
        hub->op->disconnect(hub, port);
    hub->devcount += ગણતરી;
done:
    hub->threads--;
    મુક્ત(usbdev);
    પાછા;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    જાઓ done;
}

સ્થિર u32 usb_time_sigatt;

શૂન્ય
usb_enumerate(સંરચના usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    પૂર્ણાંક i;
    માટે (i=0; i<portcount; i++) {
        સંરચના usbdevice_s *usbdev = malloc_tmphigh(કદ(*usbdev));
        જો (!usbdev) {
            warn_noalloc();
            ચાલુ;
        }
        memset(usbdev, 0, કદ(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    જ્યારે (hub->threads)
        yield();
}

શૂન્ય
usb_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (! CONFIG_USB)
        પાછા;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
