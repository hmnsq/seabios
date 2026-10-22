// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-ehci.h" // ehci_setup
#अन्तर्भाव "usb-xhci.h" // xhci_setup
#अन्तर्भाव "usb-hid.h" // usb_keyboard_setup
#अन्तर्भाव "usb-hub.h" // usb_hub_setup
#अन्तर्भाव "usb-msc.h" // usb_msc_setup
#अन्तर्भाव "usb-ohci.h" // ohci_setup
#अन्तर्भाव "usb-uas.h" // usb_uas_setup
#अन्तर्भाव "usb-uhci.h" // uhci_setup
#अन्तर्भाव "util.h" // msleep
#अन्तर्भाव "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
स्थिर संरचना usb_pipe *
usb_realloc_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *pipe
                 , संरचना usb_endpoint_descriptor *epdesc)
{
    विकल्प (usbdev->hub->cntl->type) {
    स्वभाव:
    पक्ष USB_TYPE_UHCI:
        प्रत्यागम uhci_realloc_pipe(usbdev, pipe, epdesc);
    पक्ष USB_TYPE_OHCI:
        प्रत्यागम ohci_realloc_pipe(usbdev, pipe, epdesc);
    पक्ष USB_TYPE_EHCI:
        प्रत्यागम ehci_realloc_pipe(usbdev, pipe, epdesc);
    पक्ष USB_TYPE_XHCI:
        प्रत्यागम xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
स्थिर पूर्णाङ्क
usb_send_pipe(संरचना usb_pipe *pipe_fl, पूर्णाङ्क dir, ध्रुव शून्य *cmd
              , शून्य *दत्त, पूर्णाङ्क datasize)
{
    विकल्प (GET_LOWFLAT(pipe_fl->type)) {
    स्वभाव:
    पक्ष USB_TYPE_UHCI:
        प्रत्यागम uhci_send_pipe(pipe_fl, dir, cmd, दत्त, datasize);
    पक्ष USB_TYPE_OHCI:
        यदि (MODESEGMENT)
            प्रत्यागम -1;
        प्रत्यागम ohci_send_pipe(pipe_fl, dir, cmd, दत्त, datasize);
    पक्ष USB_TYPE_EHCI:
        प्रत्यागम ehci_send_pipe(pipe_fl, dir, cmd, दत्त, datasize);
    पक्ष USB_TYPE_XHCI:
        यदि (MODESEGMENT)
            प्रत्यागम -1;
        प्रत्यागम xhci_send_pipe(pipe_fl, dir, cmd, दत्त, datasize);
    }
}

पूर्णाङ्क
usb_poll_intr(संरचना usb_pipe *pipe_fl, शून्य *दत्त)
{
    ASSERT16();
    विकल्प (GET_LOWFLAT(pipe_fl->type)) {
    स्वभाव:
    पक्ष USB_TYPE_UHCI:
        प्रत्यागम uhci_poll_intr(pipe_fl, दत्त);
    पक्ष USB_TYPE_OHCI:
        प्रत्यागम ohci_poll_intr(pipe_fl, दत्त);
    पक्ष USB_TYPE_EHCI:
        प्रत्यागम ehci_poll_intr(pipe_fl, दत्त);
    पक्ष USB_TYPE_XHCI: ;
        प्रत्यागम call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), दत्त), 0, -1);
    }
}

पूर्णाङ्क usb_32bit_pipe(संरचना usb_pipe *pipe_fl)
{
    प्रत्यागम (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
संरचना usb_pipe *
usb_alloc_pipe(संरचना usbdevice_s *usbdev
               , संरचना usb_endpoint_descriptor *epdesc)
{
    प्रत्यागम usb_realloc_pipe(usbdev, रिक्त, epdesc);
}

// Free an allocated control or bulk pipe.
शून्य
usb_free_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *pipe)
{
    यदि (!pipe)
        प्रत्यागम;
    usb_realloc_pipe(usbdev, pipe, रिक्त);
}

// Send a message to the default control pipe of a device.
पूर्णाङ्क
usb_send_default_control(संरचना usb_pipe *pipe, ध्रुव संरचना usb_ctrlrequest *req
                         , शून्य *दत्त)
{
    प्रत्यागम usb_send_pipe(pipe, req->bRequestType & USB_DIR_अन्तर्, req
                         , दत्त, req->w_दीर्घता);
}

// Send a message to a bulk endpoint
पूर्णाङ्क
usb_send_bulk(संरचना usb_pipe *pipe_fl, पूर्णाङ्क dir, शून्य *दत्त, पूर्णाङ्क datasize)
{
    प्रत्यागम usb_send_pipe(pipe_fl, dir, रिक्त, दत्त, datasize);
}

// Check if a pipe for a given controller is on the freelist
पूर्णाङ्क
usb_is_freelist(संरचना usb_s *cntl, संरचना usb_pipe *pipe)
{
    प्रत्यागम pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
शून्य
usb_सङ्कलय_freelist(संरचना usb_pipe *pipe)
{
    यदि (!pipe)
        प्रत्यागम;
    संरचना usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
संरचना usb_pipe *
usb_get_freelist(संरचना usb_s *cntl, u8 eptype)
{
    संरचना usb_pipe **pfree = &cntl->freelist;
    प्रति (;;) {
        संरचना usb_pipe *pipe = *pfree;
        यदि (!pipe)
            प्रत्यागम रिक्त;
        यदि (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            प्रत्यागम pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
शून्य
usb_desc2pipe(संरचना usb_pipe *pipe, संरचना usbdevice_s *usbdev
              , संरचना usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->b_Endpoint_स्थानाङ्कः & USB_ENDPOINT_सङ्ख्या_आवरणम्;
    pipe->devaddr = usbdev->devaddr;
    pipe->जवः = usbdev->जवः;
    pipe->maxpacket = epdesc->w_Max_Packet_आकार;
    pipe->eptype = epdesc->bm_गुणाः & USB_ENDPOINT_XFERTYPE_आवरणम्;
}

// Find the exponential period of the requested interrupt end point.
पूर्णाङ्क
usb_get_period(संरचना usbdevice_s *usbdev
               , संरचना usb_endpoint_descriptor *epdesc)
{
    पूर्णाङ्क period = epdesc->bInterval;
    यदि (usbdev->जवः != USB_HIGHSPEED)
        प्रत्यागम (period <= 0) ? 0 : __fls(period);
    प्रत्यागम (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
पूर्णाङ्क
usb_xfer_time(संरचना usb_pipe *pipe, पूर्णाङ्क datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    यदि (!GET_LOWFLAT(pipe->devaddr))
        प्रत्यागम USB_TIME_स्थितिः + 100;
    प्रत्यागम USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
संरचना usb_endpoint_descriptor *
usb_find_desc(संरचना usbdevice_s *usbdev, पूर्णाङ्क type, पूर्णाङ्क dir)
{
    संरचना usb_endpoint_descriptor *epdesc = (शून्य*)&usbdev->iface[1];
    प्रति (;;) {
        यदि ((शून्य*)epdesc >= (शून्य*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            प्रत्यागम रिक्त;
        }
        यदि (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->b_Endpoint_स्थानाङ्कः & USB_ENDPOINT_DIR_आवरणम्) == dir
            && (epdesc->bm_गुणाः & USB_ENDPOINT_XFERTYPE_आवरणम्) == type)
            प्रत्यागम epdesc;
        epdesc = (शून्य*)epdesc + epdesc->b_दीर्घता;
    }
}

// Get the first 8 bytes of the device descriptor.
स्थिर पूर्णाङ्क
get_device_info8(संरचना usb_pipe *pipe, संरचना usb_device_descriptor *dinfo)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_अन्तर् | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_मानम् = USB_DT_DEVICE<<8;
    req.w_सूचकांक = 0;
    req.w_दीर्घता = 8;
    प्रत्यागम usb_send_default_control(pipe, &req, dinfo);
}

स्थिर संरचना usb_config_descriptor *
get_device_config(संरचना usb_pipe *pipe)
{
    संरचना usb_config_descriptor cfg;

    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_अन्तर् | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_मानम् = USB_DT_CONFIG<<8;
    req.w_सूचकांक = 0;
    req.w_दीर्घता = परिमाण(cfg);
    पूर्णाङ्क ret = usb_send_default_control(pipe, &req, &cfg);
    यदि (ret)
        प्रत्यागम रिक्त;

    संरचना usb_config_descriptor *config = malloc_tmphigh(cfg.w_योग_दीर्घता);
    यदि (!config) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    req.w_दीर्घता = cfg.w_योग_दीर्घता;
    ret = usb_send_default_control(pipe, &req, config);
    यदि (ret || config->w_योग_दीर्घता != cfg.w_योग_दीर्घता) {
        मुक्त(config);
        प्रत्यागम रिक्त;
    }
    //hexdump(config, cfg.wTotalLength);
    प्रत्यागम config;
}

स्थिर पूर्णाङ्क
समुच्चयः_configuration(संरचना usb_pipe *pipe, u16 val)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_बहिर्गमनम् | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_समुच्चयः_CONFIGURATION;
    req.w_मानम् = val;
    req.w_सूचकांक = 0;
    req.w_दीर्घता = 0;
    प्रत्यागम usb_send_default_control(pipe, &req, रिक्त);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

स्थिर ध्रुव पूर्णाङ्क जवः_गन्तव्यम्_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
स्थिर पूर्णाङ्क
usb_समुच्चयः_स्थानाङ्कः(संरचना usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    संरचना usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    यदि (cntl->maxaddr >= USB_MAXADDR)
        प्रत्यागम -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    संरचना usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_आकार = जवः_गन्तव्यम्_ctlsize[usbdev->जवः],
        .bm_गुणाः = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    यदि (!usbdev->defpipe)
        प्रत्यागम -1;

    // Send set_address command.
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_बहिर्गमनम् | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_समुच्चयः_स्थानाङ्कः;
    req.w_मानम् = cntl->maxaddr + 1;
    req.w_सूचकांक = 0;
    req.w_दीर्घता = 0;
    पूर्णाङ्क ret = usb_send_default_control(usbdev->defpipe, &req, रिक्त);
    यदि (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        प्रत्यागम -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    यदि (!usbdev->defpipe)
        प्रत्यागम -1;
    प्रत्यागम 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
स्थिर पूर्णाङ्क
configure_usb_device(संरचना usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    संरचना usb_device_descriptor dinfo;
    पूर्णाङ्क ret = get_device_info8(usbdev->defpipe, &dinfo);
    यदि (ret)
        प्रत्यागम 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    यदि (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    यदि (maxpacket < 8)
        प्रत्यागम 0;
    संरचना usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_आकार = maxpacket,
        .bm_गुणाः = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    यदि (!usbdev->defpipe)
        प्रत्यागम -1;

    // Get configuration
    संरचना usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    यदि (!config)
        प्रत्यागम 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    पूर्णाङ्क अङ्कः_iface = config->b_अङ्कः_Interfaces;
    शून्य *config_अन्तः = (शून्य*)config + config->w_योग_दीर्घता;
    संरचना usb_interface_descriptor *iface = (शून्य*)(&config[1]);
    प्रति (;;) {
        यदि (!अङ्कः_iface || (शून्य*)iface + iface->b_दीर्घता > config_अन्तः)
            // Not a supported device.
            गच्छ fail;
        यदि (iface->bDescriptorType == USB_DT_INTERFACE) {
            अङ्कः_iface--;
            यदि (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                भङ्ग;
        }
        iface = (शून्य*)iface + iface->b_दीर्घता;
    }

    // Set the configuration.
    ret = समुच्चयः_configuration(usbdev->defpipe, config->b_Configuration_मानम्);
    यदि (ret)
        गच्छ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (शून्य*)config + config->w_योग_दीर्घता - (शून्य*)iface;
    यदि (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_सज्जता(usbdev);
    अन्यथा यदि (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        यदि (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_सज्जता(usbdev);
        यदि (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_सज्जता(usbdev);
    } अन्यथा
        ret = usb_hid_सज्जता(usbdev);
    यदि (ret)
        गच्छ fail;

    मुक्त(config);
    प्रत्यागम 1;
fail:
    मुक्त(config);
    प्रत्यागम 0;
}

स्थिर शून्य
usb_hub_पत्तनम्_सज्जता(शून्य *दत्त)
{
    संरचना usbdevice_s *usbdev = दत्त;
    संरचना usbhub_s *hub = usbdev->hub;
    u32 पत्तनम् = usbdev->पत्तनम्;

    प्रति (;;) {
        // Detect if device present (and possibly start reset)
        पूर्णाङ्क ret = hub->op->detect(hub, पत्तनम्);
        यदि (ret > 0)
            // Device connected.
            भङ्ग;
        यदि (ret < 0 || timer_check(hub->detectend))
            // No device found.
            गच्छ समाप्तम्;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_तालकम्(&hub->cntl->resetlock);
    पूर्णाङ्क ret = hub->op->reset(hub, पत्तनम्);
    यदि (ret < 0)
        // Reset failed
        गच्छ resetfail;
    usbdev->जवः = ret;

    // Set address of port
    ret = usb_समुच्चयः_स्थानाङ्कः(usbdev);
    यदि (ret) {
        hub->op->disconnect(hub, पत्तनम्);
        गच्छ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    पूर्णाङ्क गणनम् = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    यदि (!गणनम्)
        hub->op->disconnect(hub, पत्तनम्);
    hub->devcount += गणनम्;
समाप्तम्:
    hub->तन्तवः--;
    मुक्त(usbdev);
    प्रत्यागम;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    गच्छ समाप्तम्;
}

स्थिर u32 usb_time_sigatt;

शून्य
usb_enumerate(संरचना usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->तन्तवः = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    पूर्णाङ्क i;
    प्रति (i=0; i<portcount; i++) {
        संरचना usbdevice_s *usbdev = malloc_tmphigh(परिमाण(*usbdev));
        यदि (!usbdev) {
            warn_noalloc();
            अनुवर्त;
        }
        स्मृतिं_पूरय(usbdev, 0, परिमाण(*usbdev));
        usbdev->hub = hub;
        usbdev->पत्तनम् = i;
        सततम्_thread(usb_hub_पत्तनम्_सज्जता, usbdev);
    }

    // Wait for threads to complete.
    यावत् (hub->तन्तवः)
        yield();
}

शून्य
usb_सज्जता(शून्य)
{
    ASSERT32FLAT();
    यदि (! CONFIG_USB)
        प्रत्यागम;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_सज्जता();
    ehci_सज्जता();
    uhci_सज्जता();
    ohci_सज्जता();
}
