// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadint
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-ehci.h" // ehci_setup
#ضمن "usb-xhci.h" // xhci_setup
#ضمن "usb-hid.h" // usb_keyboard_setup
#ضمن "usb-hub.h" // usb_hub_setup
#ضمن "usb-msc.h" // usb_msc_setup
#ضمن "usb-ohci.h" // ohci_setup
#ضمن "usb-uas.h" // usb_uas_setup
#ضمن "usb-uhci.h" // uhci_setup
#ضمن "util.h" // msleep
#ضمن "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
ساكن بنية usb_pipe *
usb_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *pipe
                 , بنية usb_endpoint_descriptor *epdesc)
{
    بدل (usbdev->hub->cntl->type) {
    افتراضي:
    حالة USB_TYPE_UHCI:
        أرجع uhci_realloc_pipe(usbdev, pipe, epdesc);
    حالة USB_TYPE_OHCI:
        أرجع ohci_realloc_pipe(usbdev, pipe, epdesc);
    حالة USB_TYPE_EHCI:
        أرجع ehci_realloc_pipe(usbdev, pipe, epdesc);
    حالة USB_TYPE_XHCI:
        أرجع xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ساكن صحيح
usb_send_pipe(بنية usb_pipe *pipe_fl, صحيح dir, ثابت فراغ *cmd
              , فراغ *معلومات, صحيح datasize)
{
    بدل (GET_LOWFLAT(pipe_fl->type)) {
    افتراضي:
    حالة USB_TYPE_UHCI:
        أرجع uhci_send_pipe(pipe_fl, dir, cmd, معلومات, datasize);
    حالة USB_TYPE_OHCI:
        إذا (MODESEGMENT)
            أرجع -1;
        أرجع ohci_send_pipe(pipe_fl, dir, cmd, معلومات, datasize);
    حالة USB_TYPE_EHCI:
        أرجع ehci_send_pipe(pipe_fl, dir, cmd, معلومات, datasize);
    حالة USB_TYPE_XHCI:
        إذا (MODESEGMENT)
            أرجع -1;
        أرجع xhci_send_pipe(pipe_fl, dir, cmd, معلومات, datasize);
    }
}

صحيح
usb_poll_intr(بنية usb_pipe *pipe_fl, فراغ *معلومات)
{
    ASSERT16();
    بدل (GET_LOWFLAT(pipe_fl->type)) {
    افتراضي:
    حالة USB_TYPE_UHCI:
        أرجع uhci_poll_intr(pipe_fl, معلومات);
    حالة USB_TYPE_OHCI:
        أرجع ohci_poll_intr(pipe_fl, معلومات);
    حالة USB_TYPE_EHCI:
        أرجع ehci_poll_intr(pipe_fl, معلومات);
    حالة USB_TYPE_XHCI: ;
        أرجع call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), معلومات), 0, -1);
    }
}

صحيح usb_32bit_pipe(بنية usb_pipe *pipe_fl)
{
    أرجع (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
بنية usb_pipe *
usb_alloc_pipe(بنية usbdevice_s *usbdev
               , بنية usb_endpoint_descriptor *epdesc)
{
    أرجع usb_realloc_pipe(usbdev, عدم, epdesc);
}

// Free an allocated control or bulk pipe.
فراغ
usb_free_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *pipe)
{
    إذا (!pipe)
        أرجع;
    usb_realloc_pipe(usbdev, pipe, عدم);
}

// Send a message to the default control pipe of a device.
صحيح
usb_send_default_control(بنية usb_pipe *pipe, ثابت بنية usb_ctrlrequest *req
                         , فراغ *معلومات)
{
    أرجع usb_send_pipe(pipe, req->bRequestType & USB_DIR_دننه, req
                         , معلومات, req->w_اوږدتیا);
}

// Send a message to a bulk endpoint
صحيح
usb_send_bulk(بنية usb_pipe *pipe_fl, صحيح dir, فراغ *معلومات, صحيح datasize)
{
    أرجع usb_send_pipe(pipe_fl, dir, عدم, معلومات, datasize);
}

// Check if a pipe for a given controller is on the freelist
صحيح
usb_is_freelist(بنية usb_s *cntl, بنية usb_pipe *pipe)
{
    أرجع pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
فراغ
usb_جمع_کړه_freelist(بنية usb_pipe *pipe)
{
    إذا (!pipe)
        أرجع;
    بنية usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
بنية usb_pipe *
usb_get_freelist(بنية usb_s *cntl, u8 eptype)
{
    بنية usb_pipe **pfree = &cntl->freelist;
    لكل (;;) {
        بنية usb_pipe *pipe = *pfree;
        إذا (!pipe)
            أرجع عدم;
        إذا (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            أرجع pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
فراغ
usb_desc2pipe(بنية usb_pipe *pipe, بنية usbdevice_s *usbdev
              , بنية usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->b_Endpoint_پته & USB_ENDPOINT_شمېره_ماسک;
    pipe->devaddr = usbdev->devaddr;
    pipe->چټکتیا = usbdev->چټکتیا;
    pipe->maxpacket = epdesc->w_Max_Packet_اندازه;
    pipe->eptype = epdesc->bm_ځانګړتیاوې & USB_ENDPOINT_XFERTYPE_ماسک;
}

// Find the exponential period of the requested interrupt end point.
صحيح
usb_get_period(بنية usbdevice_s *usbdev
               , بنية usb_endpoint_descriptor *epdesc)
{
    صحيح period = epdesc->bInterval;
    إذا (usbdev->چټکتیا != USB_HIGHSPEED)
        أرجع (period <= 0) ? 0 : __fls(period);
    أرجع (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
صحيح
usb_xfer_وخت(بنية usb_pipe *pipe, صحيح datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    إذا (!GET_LOWFLAT(pipe->devaddr))
        أرجع USB_وخت_حالت + 100;
    أرجع USB_وخت_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
بنية usb_endpoint_descriptor *
usb_پیدا_کړه_desc(بنية usbdevice_s *usbdev, صحيح type, صحيح dir)
{
    بنية usb_endpoint_descriptor *epdesc = (فراغ*)&usbdev->iface[1];
    لكل (;;) {
        إذا ((فراغ*)epdesc >= (فراغ*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            أرجع عدم;
        }
        إذا (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->b_Endpoint_پته & USB_ENDPOINT_DIR_ماسک) == dir
            && (epdesc->bm_ځانګړتیاوې & USB_ENDPOINT_XFERTYPE_ماسک) == type)
            أرجع epdesc;
        epdesc = (فراغ*)epdesc + epdesc->b_اوږدتیا;
    }
}

// Get the first 8 bytes of the device descriptor.
ساكن صحيح
get_device_info8(بنية usb_pipe *pipe, بنية usb_device_descriptor *dinfo)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_دننه | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ارزښت = USB_DT_DEVICE<<8;
    req.w_شاخص = 0;
    req.w_اوږدتیا = 8;
    أرجع usb_send_default_control(pipe, &req, dinfo);
}

ساكن بنية usb_config_descriptor *
get_device_config(بنية usb_pipe *pipe)
{
    بنية usb_config_descriptor cfg;

    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_دننه | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ارزښت = USB_DT_CONFIG<<8;
    req.w_شاخص = 0;
    req.w_اوږدتیا = حجم(cfg);
    صحيح ret = usb_send_default_control(pipe, &req, &cfg);
    إذا (ret)
        أرجع عدم;

    بنية usb_config_descriptor *config = malloc_tmphigh(cfg.w_ټول_اوږدتیا);
    إذا (!config) {
        warn_noalloc();
        أرجع عدم;
    }
    req.w_اوږدتیا = cfg.w_ټول_اوږدتیا;
    ret = usb_send_default_control(pipe, &req, config);
    إذا (ret || config->w_ټول_اوږدتیا != cfg.w_ټول_اوږدتیا) {
        حرر(config);
        أرجع عدم;
    }
    //hexdump(config, cfg.wTotalLength);
    أرجع config;
}

ساكن صحيح
سېټ_configuration(بنية usb_pipe *pipe, u16 val)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بهر | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_سېټ_CONFIGURATION;
    req.w_ارزښت = val;
    req.w_شاخص = 0;
    req.w_اوږدتیا = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ساكن ثابت صحيح چټکتیا_منزل_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ساكن صحيح
usb_سېټ_پته(بنية usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    بنية usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    إذا (cntl->maxaddr >= USB_MAXADDR)
        أرجع -1;

    msleep(USB_وخت_RSTRCY);

    // Create a pipe for the default address.
    بنية usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_اندازه = چټکتیا_منزل_ctlsize[usbdev->چټکتیا],
        .bm_ځانګړتیاوې = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    إذا (!usbdev->defpipe)
        أرجع -1;

    // Send set_address command.
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بهر | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_سېټ_پته;
    req.w_ارزښت = cntl->maxaddr + 1;
    req.w_شاخص = 0;
    req.w_اوږدتیا = 0;
    صحيح ret = usb_send_default_control(usbdev->defpipe, &req, عدم);
    إذا (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        أرجع -1;
    }

    msleep(USB_وخت_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    إذا (!usbdev->defpipe)
        أرجع -1;
    أرجع 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ساكن صحيح
configure_usb_device(بنية usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    بنية usb_device_descriptor dinfo;
    صحيح ret = get_device_info8(usbdev->defpipe, &dinfo);
    إذا (ret)
        أرجع 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    إذا (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    إذا (maxpacket < 8)
        أرجع 0;
    بنية usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_اندازه = maxpacket,
        .bm_ځانګړتیاوې = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    إذا (!usbdev->defpipe)
        أرجع -1;

    // Get configuration
    بنية usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    إذا (!config)
        أرجع 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    صحيح عدد_iface = config->b_عدد_Interfaces;
    فراغ *config_پای_ټکی = (فراغ*)config + config->w_ټول_اوږدتیا;
    بنية usb_interface_descriptor *iface = (فراغ*)(&config[1]);
    لكل (;;) {
        إذا (!عدد_iface || (فراغ*)iface + iface->b_اوږدتیا > config_پای_ټکی)
            // Not a supported device.
            اذهب fail;
        إذا (iface->bDescriptorType == USB_DT_INTERFACE) {
            عدد_iface--;
            إذا (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                اكسر;
        }
        iface = (فراغ*)iface + iface->b_اوږدتیا;
    }

    // Set the configuration.
    ret = سېټ_configuration(usbdev->defpipe, config->b_Configuration_ارزښت);
    إذا (ret)
        اذهب fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (فراغ*)config + config->w_ټول_اوږدتیا - (فراغ*)iface;
    إذا (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_تیاری(usbdev);
    وإلا إذا (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        إذا (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_تیاری(usbdev);
        إذا (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_تیاری(usbdev);
    } وإلا
        ret = usb_hid_تیاری(usbdev);
    إذا (ret)
        اذهب fail;

    حرر(config);
    أرجع 1;
fail:
    حرر(config);
    أرجع 0;
}

ساكن فراغ
usb_hub_پورټ_تیاری(فراغ *معلومات)
{
    بنية usbdevice_s *usbdev = معلومات;
    بنية usbhub_s *hub = usbdev->hub;
    u32 پورټ = usbdev->پورټ;

    لكل (;;) {
        // Detect if device present (and possibly start reset)
        صحيح ret = hub->op->detect(hub, پورټ);
        إذا (ret > 0)
            // Device connected.
            اكسر;
        إذا (ret < 0 || timer_check(hub->detectend))
            // No device found.
            اذهب بشپړ;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_قلف(&hub->cntl->resetlock);
    صحيح ret = hub->op->reset(hub, پورټ);
    إذا (ret < 0)
        // Reset failed
        اذهب resetfail;
    usbdev->چټکتیا = ret;

    // Set address of port
    ret = usb_سېټ_پته(usbdev);
    إذا (ret) {
        hub->op->disconnect(hub, پورټ);
        اذهب resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    صحيح شمېر = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    إذا (!شمېر)
        hub->op->disconnect(hub, پورټ);
    hub->devcount += شمېر;
بشپړ:
    hub->تارونه--;
    حرر(usbdev);
    أرجع;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    اذهب بشپړ;
}

ساكن u32 usb_وخت_sigatt;

فراغ
usb_enumerate(بنية usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->تارونه = portcount;
    hub->detectend = timer_calc(usb_وخت_sigatt);

    // Launch a thread for every port.
    صحيح i;
    لكل (i=0; i<portcount; i++) {
        بنية usbdevice_s *usbdev = malloc_tmphigh(حجم(*usbdev));
        إذا (!usbdev) {
            warn_noalloc();
            استمر;
        }
        memset(usbdev, 0, حجم(*usbdev));
        usbdev->hub = hub;
        usbdev->پورټ = i;
        پرلپسې_thread(usb_hub_پورټ_تیاری, usbdev);
    }

    // Wait for threads to complete.
    طالما (hub->تارونه)
        yield();
}

فراغ
usb_تیاری(فراغ)
{
    ASSERT32FLAT();
    إذا (! CONFIG_USB)
        أرجع;
    dprintf(3, "init usb\n");
    usb_وخت_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_وخت_SIGATT);
    xhci_تیاری();
    ehci_تیاری();
    uhci_تیاری();
    ohci_تیاری();
}
