// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-ehci.h" // ehci_setup
#ଅନ୍ତର୍ଭାଵ "usb-xhci.h" // xhci_setup
#ଅନ୍ତର୍ଭାଵ "usb-hid.h" // usb_keyboard_setup
#ଅନ୍ତର୍ଭାଵ "usb-hub.h" // usb_hub_setup
#ଅନ୍ତର୍ଭାଵ "usb-msc.h" // usb_msc_setup
#ଅନ୍ତର୍ଭାଵ "usb-ohci.h" // ohci_setup
#ଅନ୍ତର୍ଭାଵ "usb-uas.h" // usb_uas_setup
#ଅନ୍ତର୍ଭାଵ "usb-uhci.h" // uhci_setup
#ଅନ୍ତର୍ଭାଵ "util.h" // msleep
#ଅନ୍ତର୍ଭାଵ "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
ସ୍ଥିର ସଂରଚନା usb_pipe *
usb_realloc_pipe(ସଂରଚନା usbdevice_s *usbdev, ସଂରଚନା usb_pipe *pipe
                 , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ବାଛ (usbdev->hub->cntl->type) {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ସ୍ଥିତି USB_TYPE_UHCI:
        ଫେରନ୍ତୁ uhci_realloc_pipe(usbdev, pipe, epdesc);
    ସ୍ଥିତି USB_TYPE_OHCI:
        ଫେରନ୍ତୁ ohci_realloc_pipe(usbdev, pipe, epdesc);
    ସ୍ଥିତି USB_TYPE_EHCI:
        ଫେରନ୍ତୁ ehci_realloc_pipe(usbdev, pipe, epdesc);
    ସ୍ଥିତି USB_TYPE_XHCI:
        ଫେରନ୍ତୁ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_send_pipe(ସଂରଚନା usb_pipe *pipe_fl, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଅଚଳ ଶୂନ୍ୟ *cmd
              , ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datasize)
{
    ବାଛ (GET_LOWFLAT(pipe_fl->type)) {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ସ୍ଥିତି USB_TYPE_UHCI:
        ଫେରନ୍ତୁ uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    ସ୍ଥିତି USB_TYPE_OHCI:
        ଯଦି (MODESEGMENT)
            ଫେରନ୍ତୁ -1;
        ଫେରନ୍ତୁ ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    ସ୍ଥିତି USB_TYPE_EHCI:
        ଫେରନ୍ତୁ ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    ସ୍ଥିତି USB_TYPE_XHCI:
        ଯଦି (MODESEGMENT)
            ଫେରନ୍ତୁ -1;
        ଫେରନ୍ତୁ xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_poll_intr(ସଂରଚନା usb_pipe *pipe_fl, ଶୂନ୍ୟ *data)
{
    ASSERT16();
    ବାଛ (GET_LOWFLAT(pipe_fl->type)) {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ସ୍ଥିତି USB_TYPE_UHCI:
        ଫେରନ୍ତୁ uhci_poll_intr(pipe_fl, data);
    ସ୍ଥିତି USB_TYPE_OHCI:
        ଫେରନ୍ତୁ ohci_poll_intr(pipe_fl, data);
    ସ୍ଥିତି USB_TYPE_EHCI:
        ଫେରନ୍ତୁ ehci_poll_intr(pipe_fl, data);
    ସ୍ଥିତି USB_TYPE_XHCI: ;
        ଫେରନ୍ତୁ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_32bit_pipe(ସଂରଚନା usb_pipe *pipe_fl)
{
    ଫେରନ୍ତୁ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
ସଂରଚନା usb_pipe *
usb_alloc_pipe(ସଂରଚନା usbdevice_s *usbdev
               , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ଫେରନ୍ତୁ usb_realloc_pipe(usbdev, ଖାଲି, epdesc);
}

// Free an allocated control or bulk pipe.
ଶୂନ୍ୟ
usb_free_pipe(ସଂରଚନା usbdevice_s *usbdev, ସଂରଚନା usb_pipe *pipe)
{
    ଯଦି (!pipe)
        ଫେରନ୍ତୁ;
    usb_realloc_pipe(usbdev, pipe, ଖାଲି);
}

// Send a message to the default control pipe of a device.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_send_default_control(ସଂରଚନା usb_pipe *pipe, ଅଚଳ ସଂରଚନା usb_ctrlrequest *req
                         , ଶୂନ୍ୟ *data)
{
    ଫେରନ୍ତୁ usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_send_bulk(ସଂରଚନା usb_pipe *pipe_fl, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datasize)
{
    ଫେରନ୍ତୁ usb_send_pipe(pipe_fl, dir, ଖାଲି, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_is_freelist(ସଂରଚନା usb_s *cntl, ସଂରଚନା usb_pipe *pipe)
{
    ଫେରନ୍ତୁ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
ଶୂନ୍ୟ
usb_add_freelist(ସଂରଚନା usb_pipe *pipe)
{
    ଯଦି (!pipe)
        ଫେରନ୍ତୁ;
    ସଂରଚନା usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
ସଂରଚନା usb_pipe *
usb_get_freelist(ସଂରଚନା usb_s *cntl, u8 eptype)
{
    ସଂରଚନା usb_pipe **pfree = &cntl->freelist;
    ପାଇଁ (;;) {
        ସଂରଚନା usb_pipe *pipe = *pfree;
        ଯଦି (!pipe)
            ଫେରନ୍ତୁ ଖାଲି;
        ଯଦି (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ଫେରନ୍ତୁ pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
ଶୂନ୍ୟ
usb_desc2pipe(ସଂରଚନା usb_pipe *pipe, ସଂରଚନା usbdevice_s *usbdev
              , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_ସଂଖ୍ୟା_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_get_period(ସଂରଚନା usbdevice_s *usbdev
               , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା period = epdesc->bInterval;
    ଯଦି (usbdev->speed != USB_HIGHSPEED)
        ଫେରନ୍ତୁ (period <= 0) ? 0 : __fls(period);
    ଫେରନ୍ତୁ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_xfer_time(ସଂରଚନା usb_pipe *pipe, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ଯଦି (!GET_LOWFLAT(pipe->devaddr))
        ଫେରନ୍ତୁ USB_TIME_STATUS + 100;
    ଫେରନ୍ତୁ USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
ସଂରଚନା usb_endpoint_descriptor *
usb_find_desc(ସଂରଚନା usbdevice_s *usbdev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir)
{
    ସଂରଚନା usb_endpoint_descriptor *epdesc = (ଶୂନ୍ୟ*)&usbdev->iface[1];
    ପାଇଁ (;;) {
        ଯଦି ((ଶୂନ୍ୟ*)epdesc >= (ଶୂନ୍ୟ*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ଫେରନ୍ତୁ ଖାଲି;
        }
        ଯଦି (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            ଫେରନ୍ତୁ epdesc;
        epdesc = (ଶୂନ୍ୟ*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_device_info8(ସଂରଚନା usb_pipe *pipe, ସଂରଚନା usb_device_descriptor *dinfo)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ମୂଲ୍ୟ = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    ଫେରନ୍ତୁ usb_send_default_control(pipe, &req, dinfo);
}

ସ୍ଥିର ସଂରଚନା usb_config_descriptor *
get_device_config(ସଂରଚନା usb_pipe *pipe)
{
    ସଂରଚନା usb_config_descriptor cfg;

    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ମୂଲ୍ୟ = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = ଆକାର(cfg);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_send_default_control(pipe, &req, &cfg);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ଖାଲି;

    ସଂରଚନା usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    ଯଦି (!config) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    ଯଦି (ret || config->wTotalLength != cfg.wTotalLength) {
        ମୁକ୍ତ(config);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    //hexdump(config, cfg.wTotalLength);
    ଫେରନ୍ତୁ config;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
set_configuration(ସଂରଚନା usb_pipe *pipe, u16 val)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_ମୂଲ୍ୟ = val;
    req.wIndex = 0;
    req.wLength = 0;
    ଫେରନ୍ତୁ usb_send_default_control(pipe, &req, ଖାଲି);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_set_address(ସଂରଚନା usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ସଂରଚନା usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ଯଦି (cntl->maxaddr >= USB_MAXADDR)
        ଫେରନ୍ତୁ -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    ସଂରଚନା usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ଯଦି (!usbdev->defpipe)
        ଫେରନ୍ତୁ -1;

    // Send set_address command.
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_ମୂଲ୍ୟ = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_send_default_control(usbdev->defpipe, &req, ଖାଲି);
    ଯଦି (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ଫେରନ୍ତୁ -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ଯଦି (!usbdev->defpipe)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
configure_usb_device(ସଂରଚନା usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    ସଂରଚନା usb_device_descriptor dinfo;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = get_device_info8(usbdev->defpipe, &dinfo);
    ଯଦି (ret)
        ଫେରନ୍ତୁ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ଯଦି (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ଯଦି (maxpacket < 8)
        ଫେରନ୍ତୁ 0;
    ସଂରଚନା usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ଯଦି (!usbdev->defpipe)
        ଫେରନ୍ତୁ -1;

    // Get configuration
    ସଂରଚନା usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ଯଦି (!config)
        ଫେରନ୍ତୁ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_iface = config->b_ଅଙ୍କ_Interfaces;
    ଶୂନ୍ୟ *config_end = (ଶୂନ୍ୟ*)config + config->wTotalLength;
    ସଂରଚନା usb_interface_descriptor *iface = (ଶୂନ୍ୟ*)(&config[1]);
    ପାଇଁ (;;) {
        ଯଦି (!ଅଙ୍କ_iface || (ଶୂନ୍ୟ*)iface + iface->bLength > config_end)
            // Not a supported device.
            ଯାଅ fail;
        ଯଦି (iface->bDescriptorType == USB_DT_INTERFACE) {
            ଅଙ୍କ_iface--;
            ଯଦି (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                ଭାଙ୍ଗ;
        }
        iface = (ଶୂନ୍ୟ*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_ମୂଲ୍ୟ);
    ଯଦି (ret)
        ଯାଅ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (ଶୂନ୍ୟ*)config + config->wTotalLength - (ଶୂନ୍ୟ*)iface;
    ଯଦି (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    ଅନ୍ୟଥା ଯଦି (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ଯଦି (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        ଯଦି (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } ଅନ୍ୟଥା
        ret = usb_hid_setup(usbdev);
    ଯଦି (ret)
        ଯାଅ fail;

    ମୁକ୍ତ(config);
    ଫେରନ୍ତୁ 1;
fail:
    ମୁକ୍ତ(config);
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ
usb_hub_port_setup(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା usbdevice_s *usbdev = data;
    ସଂରଚନା usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    ପାଇଁ (;;) {
        // Detect if device present (and possibly start reset)
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = hub->op->detect(hub, port);
        ଯଦି (ret > 0)
            // Device connected.
            ଭାଙ୍ଗ;
        ଯଦି (ret < 0 || timer_check(hub->detectend))
            // No device found.
            ଯାଅ done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = hub->op->reset(hub, port);
    ଯଦି (ret < 0)
        // Reset failed
        ଯାଅ resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    ଯଦି (ret) {
        hub->op->disconnect(hub, port);
        ଯାଅ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ଯଦି (!ଗଣନା_2)
        hub->op->disconnect(hub, port);
    hub->devcount += ଗଣନା_2;
done:
    hub->threads--;
    ମୁକ୍ତ(usbdev);
    ଫେରନ୍ତୁ;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    ଯାଅ done;
}

ସ୍ଥିର u32 usb_time_sigatt;

ଶୂନ୍ୟ
usb_enumerate(ସଂରଚନା usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<portcount; i++) {
        ସଂରଚନା usbdevice_s *usbdev = malloc_tmphigh(ଆକାର(*usbdev));
        ଯଦି (!usbdev) {
            warn_noalloc();
            ଜାରି;
        }
        memset(usbdev, 0, ଆକାର(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    ଯେତେବେଳେ (hub->threads)
        yield();
}

ଶୂନ୍ୟ
usb_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_USB)
        ଫେରନ୍ତୁ;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
