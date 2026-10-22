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
স্থির কাঠামো usb_pipe *
usb_realloc_pipe(কাঠামো usbdevice_s *usbdev, কাঠামো usb_pipe *pipe
                 , কাঠামো usb_endpoint_descriptor *epdesc)
{
    নির্বাচন (usbdev->hub->cntl->type) {
    পূর্বনির্ধারিত:
    ক্ষেত্র USB_TYPE_UHCI:
        ফেরত uhci_realloc_pipe(usbdev, pipe, epdesc);
    ক্ষেত্র USB_TYPE_OHCI:
        ফেরত ohci_realloc_pipe(usbdev, pipe, epdesc);
    ক্ষেত্র USB_TYPE_EHCI:
        ফেরত ehci_realloc_pipe(usbdev, pipe, epdesc);
    ক্ষেত্র USB_TYPE_XHCI:
        ফেরত xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
স্থির পূর্ণসংখ্যা
usb_send_pipe(কাঠামো usb_pipe *pipe_fl, পূর্ণসংখ্যা dir, ধ্রুবক শূন্য *cmd
              , শূন্য *তথ্য, পূর্ণসংখ্যা datasize)
{
    নির্বাচন (GET_LOWFLAT(pipe_fl->type)) {
    পূর্বনির্ধারিত:
    ক্ষেত্র USB_TYPE_UHCI:
        ফেরত uhci_send_pipe(pipe_fl, dir, cmd, তথ্য, datasize);
    ক্ষেত্র USB_TYPE_OHCI:
        যদি (MODESEGMENT)
            ফেরত -1;
        ফেরত ohci_send_pipe(pipe_fl, dir, cmd, তথ্য, datasize);
    ক্ষেত্র USB_TYPE_EHCI:
        ফেরত ehci_send_pipe(pipe_fl, dir, cmd, তথ্য, datasize);
    ক্ষেত্র USB_TYPE_XHCI:
        যদি (MODESEGMENT)
            ফেরত -1;
        ফেরত xhci_send_pipe(pipe_fl, dir, cmd, তথ্য, datasize);
    }
}

পূর্ণসংখ্যা
usb_poll_intr(কাঠামো usb_pipe *pipe_fl, শূন্য *তথ্য)
{
    ASSERT16();
    নির্বাচন (GET_LOWFLAT(pipe_fl->type)) {
    পূর্বনির্ধারিত:
    ক্ষেত্র USB_TYPE_UHCI:
        ফেরত uhci_poll_intr(pipe_fl, তথ্য);
    ক্ষেত্র USB_TYPE_OHCI:
        ফেরত ohci_poll_intr(pipe_fl, তথ্য);
    ক্ষেত্র USB_TYPE_EHCI:
        ফেরত ehci_poll_intr(pipe_fl, তথ্য);
    ক্ষেত্র USB_TYPE_XHCI: ;
        ফেরত call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), তথ্য), 0, -1);
    }
}

পূর্ণসংখ্যা usb_32bit_pipe(কাঠামো usb_pipe *pipe_fl)
{
    ফেরত (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
কাঠামো usb_pipe *
usb_alloc_pipe(কাঠামো usbdevice_s *usbdev
               , কাঠামো usb_endpoint_descriptor *epdesc)
{
    ফেরত usb_realloc_pipe(usbdev, ফাঁকা, epdesc);
}

// Free an allocated control or bulk pipe.
শূন্য
usb_free_pipe(কাঠামো usbdevice_s *usbdev, কাঠামো usb_pipe *pipe)
{
    যদি (!pipe)
        ফেরত;
    usb_realloc_pipe(usbdev, pipe, ফাঁকা);
}

// Send a message to the default control pipe of a device.
পূর্ণসংখ্যা
usb_send_default_control(কাঠামো usb_pipe *pipe, ধ্রুবক কাঠামো usb_ctrlrequest *req
                         , শূন্য *তথ্য)
{
    ফেরত usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , তথ্য, req->w_দৈর্ঘ্য);
}

// Send a message to a bulk endpoint
পূর্ণসংখ্যা
usb_send_bulk(কাঠামো usb_pipe *pipe_fl, পূর্ণসংখ্যা dir, শূন্য *তথ্য, পূর্ণসংখ্যা datasize)
{
    ফেরত usb_send_pipe(pipe_fl, dir, ফাঁকা, তথ্য, datasize);
}

// Check if a pipe for a given controller is on the freelist
পূর্ণসংখ্যা
usb_is_freelist(কাঠামো usb_s *cntl, কাঠামো usb_pipe *pipe)
{
    ফেরত pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
শূন্য
usb_add_freelist(কাঠামো usb_pipe *pipe)
{
    যদি (!pipe)
        ফেরত;
    কাঠামো usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
কাঠামো usb_pipe *
usb_get_freelist(কাঠামো usb_s *cntl, u8 eptype)
{
    কাঠামো usb_pipe **pfree = &cntl->freelist;
    জন্য (;;) {
        কাঠামো usb_pipe *pipe = *pfree;
        যদি (!pipe)
            ফেরত ফাঁকা;
        যদি (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ফেরত pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
শূন্য
usb_desc2pipe(কাঠামো usb_pipe *pipe, কাঠামো usbdevice_s *usbdev
              , কাঠামো usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_সংখ্যা_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->w_Max_Packet_আকার;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
পূর্ণসংখ্যা
usb_get_period(কাঠামো usbdevice_s *usbdev
               , কাঠামো usb_endpoint_descriptor *epdesc)
{
    পূর্ণসংখ্যা period = epdesc->bInterval;
    যদি (usbdev->speed != USB_HIGHSPEED)
        ফেরত (period <= 0) ? 0 : __fls(period);
    ফেরত (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
পূর্ণসংখ্যা
usb_xfer_time(কাঠামো usb_pipe *pipe, পূর্ণসংখ্যা datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    যদি (!GET_LOWFLAT(pipe->devaddr))
        ফেরত USB_TIME_STATUS + 100;
    ফেরত USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
কাঠামো usb_endpoint_descriptor *
usb_find_desc(কাঠামো usbdevice_s *usbdev, পূর্ণসংখ্যা type, পূর্ণসংখ্যা dir)
{
    কাঠামো usb_endpoint_descriptor *epdesc = (শূন্য*)&usbdev->iface[1];
    জন্য (;;) {
        যদি ((শূন্য*)epdesc >= (শূন্য*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ফেরত ফাঁকা;
        }
        যদি (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            ফেরত epdesc;
        epdesc = (শূন্য*)epdesc + epdesc->b_দৈর্ঘ্য;
    }
}

// Get the first 8 bytes of the device descriptor.
স্থির পূর্ণসংখ্যা
get_device_info8(কাঠামো usb_pipe *pipe, কাঠামো usb_device_descriptor *dinfo)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_মান = USB_DT_DEVICE<<8;
    req.w_সূচক = 0;
    req.w_দৈর্ঘ্য = 8;
    ফেরত usb_send_default_control(pipe, &req, dinfo);
}

স্থির কাঠামো usb_config_descriptor *
get_device_config(কাঠামো usb_pipe *pipe)
{
    কাঠামো usb_config_descriptor cfg;

    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_মান = USB_DT_CONFIG<<8;
    req.w_সূচক = 0;
    req.w_দৈর্ঘ্য = আকার(cfg);
    পূর্ণসংখ্যা ret = usb_send_default_control(pipe, &req, &cfg);
    যদি (ret)
        ফেরত ফাঁকা;

    কাঠামো usb_config_descriptor *config = malloc_tmphigh(cfg.w_যোগ_দৈর্ঘ্য);
    যদি (!config) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    req.w_দৈর্ঘ্য = cfg.w_যোগ_দৈর্ঘ্য;
    ret = usb_send_default_control(pipe, &req, config);
    যদি (ret || config->w_যোগ_দৈর্ঘ্য != cfg.w_যোগ_দৈর্ঘ্য) {
        মুক্ত(config);
        ফেরত ফাঁকা;
    }
    //hexdump(config, cfg.wTotalLength);
    ফেরত config;
}

স্থির পূর্ণসংখ্যা
set_configuration(কাঠামো usb_pipe *pipe, u16 val)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.w_মান = val;
    req.w_সূচক = 0;
    req.w_দৈর্ঘ্য = 0;
    ফেরত usb_send_default_control(pipe, &req, ফাঁকা);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

স্থির ধ্রুবক পূর্ণসংখ্যা speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
স্থির পূর্ণসংখ্যা
usb_set_address(কাঠামো usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    কাঠামো usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    যদি (cntl->maxaddr >= USB_MAXADDR)
        ফেরত -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    কাঠামো usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_আকার = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    যদি (!usbdev->defpipe)
        ফেরত -1;

    // Send set_address command.
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.w_মান = cntl->maxaddr + 1;
    req.w_সূচক = 0;
    req.w_দৈর্ঘ্য = 0;
    পূর্ণসংখ্যা ret = usb_send_default_control(usbdev->defpipe, &req, ফাঁকা);
    যদি (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ফেরত -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    যদি (!usbdev->defpipe)
        ফেরত -1;
    ফেরত 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
স্থির পূর্ণসংখ্যা
configure_usb_device(কাঠামো usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    কাঠামো usb_device_descriptor dinfo;
    পূর্ণসংখ্যা ret = get_device_info8(usbdev->defpipe, &dinfo);
    যদি (ret)
        ফেরত 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    যদি (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    যদি (maxpacket < 8)
        ফেরত 0;
    কাঠামো usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_আকার = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    যদি (!usbdev->defpipe)
        ফেরত -1;

    // Get configuration
    কাঠামো usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    যদি (!config)
        ফেরত 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    পূর্ণসংখ্যা অঙ্ক_iface = config->b_অঙ্ক_Interfaces;
    শূন্য *config_end = (শূন্য*)config + config->w_যোগ_দৈর্ঘ্য;
    কাঠামো usb_interface_descriptor *iface = (শূন্য*)(&config[1]);
    জন্য (;;) {
        যদি (!অঙ্ক_iface || (শূন্য*)iface + iface->b_দৈর্ঘ্য > config_end)
            // Not a supported device.
            যাও fail;
        যদি (iface->bDescriptorType == USB_DT_INTERFACE) {
            অঙ্ক_iface--;
            যদি (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                বিরতি;
        }
        iface = (শূন্য*)iface + iface->b_দৈর্ঘ্য;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->b_Configuration_মান);
    যদি (ret)
        যাও fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (শূন্য*)config + config->w_যোগ_দৈর্ঘ্য - (শূন্য*)iface;
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
        যাও fail;

    মুক্ত(config);
    ফেরত 1;
fail:
    মুক্ত(config);
    ফেরত 0;
}

স্থির শূন্য
usb_hub_port_setup(শূন্য *তথ্য)
{
    কাঠামো usbdevice_s *usbdev = তথ্য;
    কাঠামো usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    জন্য (;;) {
        // Detect if device present (and possibly start reset)
        পূর্ণসংখ্যা ret = hub->op->detect(hub, port);
        যদি (ret > 0)
            // Device connected.
            বিরতি;
        যদি (ret < 0 || timer_check(hub->detectend))
            // No device found.
            যাও done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    পূর্ণসংখ্যা ret = hub->op->reset(hub, port);
    যদি (ret < 0)
        // Reset failed
        যাও resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    যদি (ret) {
        hub->op->disconnect(hub, port);
        যাও resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    পূর্ণসংখ্যা গুনতি = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    যদি (!গুনতি)
        hub->op->disconnect(hub, port);
    hub->devcount += গুনতি;
done:
    hub->threads--;
    মুক্ত(usbdev);
    ফেরত;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    যাও done;
}

স্থির u32 usb_time_sigatt;

শূন্য
usb_enumerate(কাঠামো usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<portcount; i++) {
        কাঠামো usbdevice_s *usbdev = malloc_tmphigh(আকার(*usbdev));
        যদি (!usbdev) {
            warn_noalloc();
            চালু;
        }
        memset(usbdev, 0, আকার(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    যখন (hub->threads)
        yield();
}

শূন্য
usb_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (! CONFIG_USB)
        ফেরত;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
