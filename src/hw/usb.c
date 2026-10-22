// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_loadint
#समावेश "string.h" // memset
#समावेश "usb.h" // struct usb_s
#समावेश "usb-ehci.h" // ehci_setup
#समावेश "usb-xhci.h" // xhci_setup
#समावेश "usb-hid.h" // usb_keyboard_setup
#समावेश "usb-hub.h" // usb_hub_setup
#समावेश "usb-msc.h" // usb_msc_setup
#समावेश "usb-ohci.h" // ohci_setup
#समावेश "usb-uas.h" // usb_uas_setup
#समावेश "usb-uhci.h" // uhci_setup
#समावेश "util.h" // msleep
#समावेश "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
अटल काष्ठा usb_pipe *
usb_realloc_pipe(काष्ठा usbdevice_s *usbdev, काष्ठा usb_pipe *pipe
                 , काष्ठा usb_endpoint_descriptor *epdesc)
{
    चयन (usbdev->hub->cntl->type) {
    शेष:
    हाल USB_TYPE_UHCI:
        वापस uhci_realloc_pipe(usbdev, pipe, epdesc);
    हाल USB_TYPE_OHCI:
        वापस ohci_realloc_pipe(usbdev, pipe, epdesc);
    हाल USB_TYPE_EHCI:
        वापस ehci_realloc_pipe(usbdev, pipe, epdesc);
    हाल USB_TYPE_XHCI:
        वापस xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
अटल पूर्णांक
usb_send_pipe(काष्ठा usb_pipe *pipe_fl, पूर्णांक dir, स्थिर व्योम *cmd
              , व्योम *तथ्य, पूर्णांक datasize)
{
    चयन (GET_LOWFLAT(pipe_fl->type)) {
    शेष:
    हाल USB_TYPE_UHCI:
        वापस uhci_send_pipe(pipe_fl, dir, cmd, तथ्य, datasize);
    हाल USB_TYPE_OHCI:
        अगर (MODESEGMENT)
            वापस -1;
        वापस ohci_send_pipe(pipe_fl, dir, cmd, तथ्य, datasize);
    हाल USB_TYPE_EHCI:
        वापस ehci_send_pipe(pipe_fl, dir, cmd, तथ्य, datasize);
    हाल USB_TYPE_XHCI:
        अगर (MODESEGMENT)
            वापस -1;
        वापस xhci_send_pipe(pipe_fl, dir, cmd, तथ्य, datasize);
    }
}

पूर्णांक
usb_poll_intr(काष्ठा usb_pipe *pipe_fl, व्योम *तथ्य)
{
    ASSERT16();
    चयन (GET_LOWFLAT(pipe_fl->type)) {
    शेष:
    हाल USB_TYPE_UHCI:
        वापस uhci_poll_intr(pipe_fl, तथ्य);
    हाल USB_TYPE_OHCI:
        वापस ohci_poll_intr(pipe_fl, तथ्य);
    हाल USB_TYPE_EHCI:
        वापस ehci_poll_intr(pipe_fl, तथ्य);
    हाल USB_TYPE_XHCI: ;
        वापस call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), तथ्य), 0, -1);
    }
}

पूर्णांक usb_32bit_pipe(काष्ठा usb_pipe *pipe_fl)
{
    वापस (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
काष्ठा usb_pipe *
usb_alloc_pipe(काष्ठा usbdevice_s *usbdev
               , काष्ठा usb_endpoint_descriptor *epdesc)
{
    वापस usb_realloc_pipe(usbdev, शून्य, epdesc);
}

// Free an allocated control or bulk pipe.
व्योम
usb_free_pipe(काष्ठा usbdevice_s *usbdev, काष्ठा usb_pipe *pipe)
{
    अगर (!pipe)
        वापस;
    usb_realloc_pipe(usbdev, pipe, शून्य);
}

// Send a message to the default control pipe of a device.
पूर्णांक
usb_send_default_control(काष्ठा usb_pipe *pipe, स्थिर काष्ठा usb_ctrlrequest *req
                         , व्योम *तथ्य)
{
    वापस usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , तथ्य, req->w_लंबाई);
}

// Send a message to a bulk endpoint
पूर्णांक
usb_send_bulk(काष्ठा usb_pipe *pipe_fl, पूर्णांक dir, व्योम *तथ्य, पूर्णांक datasize)
{
    वापस usb_send_pipe(pipe_fl, dir, शून्य, तथ्य, datasize);
}

// Check if a pipe for a given controller is on the freelist
पूर्णांक
usb_is_freelist(काष्ठा usb_s *cntl, काष्ठा usb_pipe *pipe)
{
    वापस pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
व्योम
usb_add_freelist(काष्ठा usb_pipe *pipe)
{
    अगर (!pipe)
        वापस;
    काष्ठा usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
काष्ठा usb_pipe *
usb_get_freelist(काष्ठा usb_s *cntl, u8 eptype)
{
    काष्ठा usb_pipe **pfree = &cntl->freelist;
    क्रम (;;) {
        काष्ठा usb_pipe *pipe = *pfree;
        अगर (!pipe)
            वापस शून्य;
        अगर (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            वापस pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
व्योम
usb_desc2pipe(काष्ठा usb_pipe *pipe, काष्ठा usbdevice_s *usbdev
              , काष्ठा usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_संख्या_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->w_Max_Packet_आकार;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
पूर्णांक
usb_get_period(काष्ठा usbdevice_s *usbdev
               , काष्ठा usb_endpoint_descriptor *epdesc)
{
    पूर्णांक period = epdesc->bInterval;
    अगर (usbdev->speed != USB_HIGHSPEED)
        वापस (period <= 0) ? 0 : __fls(period);
    वापस (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
पूर्णांक
usb_xfer_time(काष्ठा usb_pipe *pipe, पूर्णांक datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    अगर (!GET_LOWFLAT(pipe->devaddr))
        वापस USB_TIME_STATUS + 100;
    वापस USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
काष्ठा usb_endpoint_descriptor *
usb_find_desc(काष्ठा usbdevice_s *usbdev, पूर्णांक type, पूर्णांक dir)
{
    काष्ठा usb_endpoint_descriptor *epdesc = (व्योम*)&usbdev->iface[1];
    क्रम (;;) {
        अगर ((व्योम*)epdesc >= (व्योम*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            वापस शून्य;
        }
        अगर (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            वापस epdesc;
        epdesc = (व्योम*)epdesc + epdesc->b_लंबाई;
    }
}

// Get the first 8 bytes of the device descriptor.
अटल पूर्णांक
get_device_info8(काष्ठा usb_pipe *pipe, काष्ठा usb_device_descriptor *dinfo)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_मूल्य = USB_DT_DEVICE<<8;
    req.w_सूचकांक = 0;
    req.w_लंबाई = 8;
    वापस usb_send_default_control(pipe, &req, dinfo);
}

अटल काष्ठा usb_config_descriptor *
get_device_config(काष्ठा usb_pipe *pipe)
{
    काष्ठा usb_config_descriptor cfg;

    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_मूल्य = USB_DT_CONFIG<<8;
    req.w_सूचकांक = 0;
    req.w_लंबाई = माप(cfg);
    पूर्णांक ret = usb_send_default_control(pipe, &req, &cfg);
    अगर (ret)
        वापस शून्य;

    काष्ठा usb_config_descriptor *config = malloc_tmphigh(cfg.w_योग_लंबाई);
    अगर (!config) {
        warn_noalloc();
        वापस शून्य;
    }
    req.w_लंबाई = cfg.w_योग_लंबाई;
    ret = usb_send_default_control(pipe, &req, config);
    अगर (ret || config->w_योग_लंबाई != cfg.w_योग_लंबाई) {
        मुक्त(config);
        वापस शून्य;
    }
    //hexdump(config, cfg.wTotalLength);
    वापस config;
}

अटल पूर्णांक
set_configuration(काष्ठा usb_pipe *pipe, u16 val)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_मूल्य = val;
    req.w_सूचकांक = 0;
    req.w_लंबाई = 0;
    वापस usb_send_default_control(pipe, &req, शून्य);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

अटल स्थिर पूर्णांक speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
अटल पूर्णांक
usb_set_address(काष्ठा usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    काष्ठा usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    अगर (cntl->maxaddr >= USB_MAXADDR)
        वापस -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    काष्ठा usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_आकार = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    अगर (!usbdev->defpipe)
        वापस -1;

    // Send set_address command.
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_मूल्य = cntl->maxaddr + 1;
    req.w_सूचकांक = 0;
    req.w_लंबाई = 0;
    पूर्णांक ret = usb_send_default_control(usbdev->defpipe, &req, शून्य);
    अगर (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        वापस -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    अगर (!usbdev->defpipe)
        वापस -1;
    वापस 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
अटल पूर्णांक
configure_usb_device(काष्ठा usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    काष्ठा usb_device_descriptor dinfo;
    पूर्णांक ret = get_device_info8(usbdev->defpipe, &dinfo);
    अगर (ret)
        वापस 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    अगर (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    अगर (maxpacket < 8)
        वापस 0;
    काष्ठा usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_आकार = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    अगर (!usbdev->defpipe)
        वापस -1;

    // Get configuration
    काष्ठा usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    अगर (!config)
        वापस 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    पूर्णांक अंक_iface = config->b_अंक_Interfaces;
    व्योम *config_end = (व्योम*)config + config->w_योग_लंबाई;
    काष्ठा usb_interface_descriptor *iface = (व्योम*)(&config[1]);
    क्रम (;;) {
        अगर (!अंक_iface || (व्योम*)iface + iface->b_लंबाई > config_end)
            // Not a supported device.
            जाओ fail;
        अगर (iface->bDescriptorType == USB_DT_INTERFACE) {
            अंक_iface--;
            अगर (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                अवरोध;
        }
        iface = (व्योम*)iface + iface->b_लंबाई;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_मूल्य);
    अगर (ret)
        जाओ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (व्योम*)config + config->w_योग_लंबाई - (व्योम*)iface;
    अगर (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    अन्यथा अगर (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        अगर (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        अगर (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } अन्यथा
        ret = usb_hid_setup(usbdev);
    अगर (ret)
        जाओ fail;

    मुक्त(config);
    वापस 1;
fail:
    मुक्त(config);
    वापस 0;
}

अटल व्योम
usb_hub_port_setup(व्योम *तथ्य)
{
    काष्ठा usbdevice_s *usbdev = तथ्य;
    काष्ठा usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    क्रम (;;) {
        // Detect if device present (and possibly start reset)
        पूर्णांक ret = hub->op->detect(hub, port);
        अगर (ret > 0)
            // Device connected.
            अवरोध;
        अगर (ret < 0 || timer_check(hub->detectend))
            // No device found.
            जाओ done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    पूर्णांक ret = hub->op->reset(hub, port);
    अगर (ret < 0)
        // Reset failed
        जाओ resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    अगर (ret) {
        hub->op->disconnect(hub, port);
        जाओ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    पूर्णांक गिनती = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    अगर (!गिनती)
        hub->op->disconnect(hub, port);
    hub->devcount += गिनती;
done:
    hub->threads--;
    मुक्त(usbdev);
    वापस;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    जाओ done;
}

अटल u32 usb_time_sigatt;

व्योम
usb_enumerate(काष्ठा usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    पूर्णांक i;
    क्रम (i=0; i<portcount; i++) {
        काष्ठा usbdevice_s *usbdev = malloc_tmphigh(माप(*usbdev));
        अगर (!usbdev) {
            warn_noalloc();
            जारी;
        }
        स_रखो(usbdev, 0, माप(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    जबतक (hub->threads)
        yield();
}

व्योम
usb_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (! CONFIG_USB)
        वापस;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
