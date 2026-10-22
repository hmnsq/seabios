// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-ehci.h" // ehci_setup
#అన్తర్భావ "usb-xhci.h" // xhci_setup
#అన్తర్భావ "usb-hid.h" // usb_keyboard_setup
#అన్తర్భావ "usb-hub.h" // usb_hub_setup
#అన్తర్భావ "usb-msc.h" // usb_msc_setup
#అన్తర్భావ "usb-ohci.h" // ohci_setup
#అన్తర్భావ "usb-uas.h" // usb_uas_setup
#అన్తర్భావ "usb-uhci.h" // uhci_setup
#అన్తర్భావ "util.h" // msleep
#అన్తర్భావ "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
స్థిర నిర్మాణం usb_pipe *
usb_realloc_pipe(నిర్మాణం usbdevice_s *usbdev, నిర్మాణం usb_pipe *pipe
                 , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    ఎంపిక (usbdev->hub->cntl->type) {
    అప్రమేయ:
    స్థితి USB_TYPE_UHCI:
        తిరిగి uhci_realloc_pipe(usbdev, pipe, epdesc);
    స్థితి USB_TYPE_OHCI:
        తిరిగి ohci_realloc_pipe(usbdev, pipe, epdesc);
    స్థితి USB_TYPE_EHCI:
        తిరిగి ehci_realloc_pipe(usbdev, pipe, epdesc);
    స్థితి USB_TYPE_XHCI:
        తిరిగి xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
స్థిర పూర్ణాంకం
usb_send_pipe(నిర్మాణం usb_pipe *pipe_fl, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
              , శూన్య *దత్తాంశం, పూర్ణాంకం datasize)
{
    ఎంపిక (GET_LOWFLAT(pipe_fl->type)) {
    అప్రమేయ:
    స్థితి USB_TYPE_UHCI:
        తిరిగి uhci_send_pipe(pipe_fl, dir, cmd, దత్తాంశం, datasize);
    స్థితి USB_TYPE_OHCI:
        ఒకవేళ (MODESEGMENT)
            తిరిగి -1;
        తిరిగి ohci_send_pipe(pipe_fl, dir, cmd, దత్తాంశం, datasize);
    స్థితి USB_TYPE_EHCI:
        తిరిగి ehci_send_pipe(pipe_fl, dir, cmd, దత్తాంశం, datasize);
    స్థితి USB_TYPE_XHCI:
        ఒకవేళ (MODESEGMENT)
            తిరిగి -1;
        తిరిగి xhci_send_pipe(pipe_fl, dir, cmd, దత్తాంశం, datasize);
    }
}

పూర్ణాంకం
usb_poll_intr(నిర్మాణం usb_pipe *pipe_fl, శూన్య *దత్తాంశం)
{
    ASSERT16();
    ఎంపిక (GET_LOWFLAT(pipe_fl->type)) {
    అప్రమేయ:
    స్థితి USB_TYPE_UHCI:
        తిరిగి uhci_poll_intr(pipe_fl, దత్తాంశం);
    స్థితి USB_TYPE_OHCI:
        తిరిగి ohci_poll_intr(pipe_fl, దత్తాంశం);
    స్థితి USB_TYPE_EHCI:
        తిరిగి ehci_poll_intr(pipe_fl, దత్తాంశం);
    స్థితి USB_TYPE_XHCI: ;
        తిరిగి call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), దత్తాంశం), 0, -1);
    }
}

పూర్ణాంకం usb_32bit_pipe(నిర్మాణం usb_pipe *pipe_fl)
{
    తిరిగి (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
నిర్మాణం usb_pipe *
usb_alloc_pipe(నిర్మాణం usbdevice_s *usbdev
               , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    తిరిగి usb_realloc_pipe(usbdev, ఖాళీ, epdesc);
}

// Free an allocated control or bulk pipe.
శూన్య
usb_free_pipe(నిర్మాణం usbdevice_s *usbdev, నిర్మాణం usb_pipe *pipe)
{
    ఒకవేళ (!pipe)
        తిరిగి;
    usb_realloc_pipe(usbdev, pipe, ఖాళీ);
}

// Send a message to the default control pipe of a device.
పూర్ణాంకం
usb_send_default_control(నిర్మాణం usb_pipe *pipe, స్థిరరాశి నిర్మాణం usb_ctrlrequest *req
                         , శూన్య *దత్తాంశం)
{
    తిరిగి usb_send_pipe(pipe, req->bRequestType & USB_DIR_లోపల, req
                         , దత్తాంశం, req->w_నిడివి);
}

// Send a message to a bulk endpoint
పూర్ణాంకం
usb_send_bulk(నిర్మాణం usb_pipe *pipe_fl, పూర్ణాంకం dir, శూన్య *దత్తాంశం, పూర్ణాంకం datasize)
{
    తిరిగి usb_send_pipe(pipe_fl, dir, ఖాళీ, దత్తాంశం, datasize);
}

// Check if a pipe for a given controller is on the freelist
పూర్ణాంకం
usb_is_freelist(నిర్మాణం usb_s *cntl, నిర్మాణం usb_pipe *pipe)
{
    తిరిగి pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
శూన్య
usb_కూడు_freelist(నిర్మాణం usb_pipe *pipe)
{
    ఒకవేళ (!pipe)
        తిరిగి;
    నిర్మాణం usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
నిర్మాణం usb_pipe *
usb_get_freelist(నిర్మాణం usb_s *cntl, u8 eptype)
{
    నిర్మాణం usb_pipe **pfree = &cntl->freelist;
    కొరకు (;;) {
        నిర్మాణం usb_pipe *pipe = *pfree;
        ఒకవేళ (!pipe)
            తిరిగి ఖాళీ;
        ఒకవేళ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            తిరిగి pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
శూన్య
usb_desc2pipe(నిర్మాణం usb_pipe *pipe, నిర్మాణం usbdevice_s *usbdev
              , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->b_Endpoint_చిరునామా & USB_ENDPOINT_సంఖ్య_ముసుగు;
    pipe->devaddr = usbdev->devaddr;
    pipe->వేగం = usbdev->వేగం;
    pipe->maxpacket = epdesc->w_Max_Packet_కొలత;
    pipe->eptype = epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు;
}

// Find the exponential period of the requested interrupt end point.
పూర్ణాంకం
usb_get_period(నిర్మాణం usbdevice_s *usbdev
               , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    పూర్ణాంకం period = epdesc->bInterval;
    ఒకవేళ (usbdev->వేగం != USB_HIGHSPEED)
        తిరిగి (period <= 0) ? 0 : __fls(period);
    తిరిగి (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
పూర్ణాంకం
usb_xfer_సమయం(నిర్మాణం usb_pipe *pipe, పూర్ణాంకం datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ఒకవేళ (!GET_LOWFLAT(pipe->devaddr))
        తిరిగి USB_సమయం_స్థితి + 100;
    తిరిగి USB_సమయం_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
నిర్మాణం usb_endpoint_descriptor *
usb_కనుగొను_desc(నిర్మాణం usbdevice_s *usbdev, పూర్ణాంకం type, పూర్ణాంకం dir)
{
    నిర్మాణం usb_endpoint_descriptor *epdesc = (శూన్య*)&usbdev->iface[1];
    కొరకు (;;) {
        ఒకవేళ ((శూన్య*)epdesc >= (శూన్య*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            తిరిగి ఖాళీ;
        }
        ఒకవేళ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->b_Endpoint_చిరునామా & USB_ENDPOINT_DIR_ముసుగు) == dir
            && (epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు) == type)
            తిరిగి epdesc;
        epdesc = (శూన్య*)epdesc + epdesc->b_నిడివి;
    }
}

// Get the first 8 bytes of the device descriptor.
స్థిర పూర్ణాంకం
get_device_info8(నిర్మాణం usb_pipe *pipe, నిర్మాణం usb_device_descriptor *dinfo)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_లోపల | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_విలువ = USB_DT_DEVICE<<8;
    req.w_సూచీ = 0;
    req.w_నిడివి = 8;
    తిరిగి usb_send_default_control(pipe, &req, dinfo);
}

స్థిర నిర్మాణం usb_config_descriptor *
get_device_config(నిర్మాణం usb_pipe *pipe)
{
    నిర్మాణం usb_config_descriptor cfg;

    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_లోపల | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_విలువ = USB_DT_CONFIG<<8;
    req.w_సూచీ = 0;
    req.w_నిడివి = పరిమాణం(cfg);
    పూర్ణాంకం ret = usb_send_default_control(pipe, &req, &cfg);
    ఒకవేళ (ret)
        తిరిగి ఖాళీ;

    నిర్మాణం usb_config_descriptor *config = malloc_tmphigh(cfg.w_మొత్తం_నిడివి);
    ఒకవేళ (!config) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    req.w_నిడివి = cfg.w_మొత్తం_నిడివి;
    ret = usb_send_default_control(pipe, &req, config);
    ఒకవేళ (ret || config->w_మొత్తం_నిడివి != cfg.w_మొత్తం_నిడివి) {
        విడుదల(config);
        తిరిగి ఖాళీ;
    }
    //hexdump(config, cfg.wTotalLength);
    తిరిగి config;
}

స్థిర పూర్ణాంకం
సమితి_configuration(నిర్మాణం usb_pipe *pipe, u16 val)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_బహిర్గతం | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_సమితి_CONFIGURATION;
    req.w_విలువ = val;
    req.w_సూచీ = 0;
    req.w_నిడివి = 0;
    తిరిగి usb_send_default_control(pipe, &req, ఖాళీ);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

స్థిర స్థిరరాశి పూర్ణాంకం వేగం_గమ్యం_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
స్థిర పూర్ణాంకం
usb_సమితి_చిరునామా(నిర్మాణం usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    నిర్మాణం usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ఒకవేళ (cntl->maxaddr >= USB_MAXADDR)
        తిరిగి -1;

    msleep(USB_సమయం_RSTRCY);

    // Create a pipe for the default address.
    నిర్మాణం usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_కొలత = వేగం_గమ్యం_ctlsize[usbdev->వేగం],
        .bm_లక్షణాలు = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ఒకవేళ (!usbdev->defpipe)
        తిరిగి -1;

    // Send set_address command.
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_బహిర్గతం | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_సమితి_చిరునామా;
    req.w_విలువ = cntl->maxaddr + 1;
    req.w_సూచీ = 0;
    req.w_నిడివి = 0;
    పూర్ణాంకం ret = usb_send_default_control(usbdev->defpipe, &req, ఖాళీ);
    ఒకవేళ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        తిరిగి -1;
    }

    msleep(USB_సమయం_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ఒకవేళ (!usbdev->defpipe)
        తిరిగి -1;
    తిరిగి 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
స్థిర పూర్ణాంకం
configure_usb_device(నిర్మాణం usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    నిర్మాణం usb_device_descriptor dinfo;
    పూర్ణాంకం ret = get_device_info8(usbdev->defpipe, &dinfo);
    ఒకవేళ (ret)
        తిరిగి 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ఒకవేళ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ఒకవేళ (maxpacket < 8)
        తిరిగి 0;
    నిర్మాణం usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_కొలత = maxpacket,
        .bm_లక్షణాలు = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ఒకవేళ (!usbdev->defpipe)
        తిరిగి -1;

    // Get configuration
    నిర్మాణం usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ఒకవేళ (!config)
        తిరిగి 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    పూర్ణాంకం అంకె_iface = config->b_అంకె_Interfaces;
    శూన్య *config_చివర = (శూన్య*)config + config->w_మొత్తం_నిడివి;
    నిర్మాణం usb_interface_descriptor *iface = (శూన్య*)(&config[1]);
    కొరకు (;;) {
        ఒకవేళ (!అంకె_iface || (శూన్య*)iface + iface->b_నిడివి > config_చివర)
            // Not a supported device.
            వెళ్ళు fail;
        ఒకవేళ (iface->bDescriptorType == USB_DT_INTERFACE) {
            అంకె_iface--;
            ఒకవేళ (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                ఆపు;
        }
        iface = (శూన్య*)iface + iface->b_నిడివి;
    }

    // Set the configuration.
    ret = సమితి_configuration(usbdev->defpipe, config->b_Configuration_విలువ);
    ఒకవేళ (ret)
        వెళ్ళు fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (శూన్య*)config + config->w_మొత్తం_నిడివి - (శూన్య*)iface;
    ఒకవేళ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_అమరిక(usbdev);
    లేకపోతే ఒకవేళ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ఒకవేళ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_అమరిక(usbdev);
        ఒకవేళ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_అమరిక(usbdev);
    } లేకపోతే
        ret = usb_hid_అమరిక(usbdev);
    ఒకవేళ (ret)
        వెళ్ళు fail;

    విడుదల(config);
    తిరిగి 1;
fail:
    విడుదల(config);
    తిరిగి 0;
}

స్థిర శూన్య
usb_hub_పోర్టు_అమరిక(శూన్య *దత్తాంశం)
{
    నిర్మాణం usbdevice_s *usbdev = దత్తాంశం;
    నిర్మాణం usbhub_s *hub = usbdev->hub;
    u32 పోర్టు = usbdev->పోర్టు;

    కొరకు (;;) {
        // Detect if device present (and possibly start reset)
        పూర్ణాంకం ret = hub->op->detect(hub, పోర్టు);
        ఒకవేళ (ret > 0)
            // Device connected.
            ఆపు;
        ఒకవేళ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            వెళ్ళు పూర్తి;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_తాళం(&hub->cntl->resetlock);
    పూర్ణాంకం ret = hub->op->reset(hub, పోర్టు);
    ఒకవేళ (ret < 0)
        // Reset failed
        వెళ్ళు resetfail;
    usbdev->వేగం = ret;

    // Set address of port
    ret = usb_సమితి_చిరునామా(usbdev);
    ఒకవేళ (ret) {
        hub->op->disconnect(hub, పోర్టు);
        వెళ్ళు resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    పూర్ణాంకం లెక్క = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ఒకవేళ (!లెక్క)
        hub->op->disconnect(hub, పోర్టు);
    hub->devcount += లెక్క;
పూర్తి:
    hub->దారాలు--;
    విడుదల(usbdev);
    తిరిగి;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    వెళ్ళు పూర్తి;
}

స్థిర u32 usb_సమయం_sigatt;

శూన్య
usb_enumerate(నిర్మాణం usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->దారాలు = portcount;
    hub->detectend = timer_calc(usb_సమయం_sigatt);

    // Launch a thread for every port.
    పూర్ణాంకం i;
    కొరకు (i=0; i<portcount; i++) {
        నిర్మాణం usbdevice_s *usbdev = malloc_tmphigh(పరిమాణం(*usbdev));
        ఒకవేళ (!usbdev) {
            warn_noalloc();
            కొనసాగు;
        }
        memset(usbdev, 0, పరిమాణం(*usbdev));
        usbdev->hub = hub;
        usbdev->పోర్టు = i;
        నిరంతరం_thread(usb_hub_పోర్టు_అమరిక, usbdev);
    }

    // Wait for threads to complete.
    అయితే (hub->దారాలు)
        yield();
}

శూన్య
usb_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_USB)
        తిరిగి;
    dprintf(3, "init usb\n");
    usb_సమయం_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_సమయం_SIGATT);
    xhci_అమరిక();
    ehci_అమరిక();
    uhci_అమరిక();
    ohci_అమరిక();
}
