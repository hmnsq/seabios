// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadint
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-ehci.h" // ehci_setup
#شامل "usb-xhci.h" // xhci_setup
#شامل "usb-hid.h" // usb_keyboard_setup
#شامل "usb-hub.h" // usb_hub_setup
#شامل "usb-msc.h" // usb_msc_setup
#شامل "usb-ohci.h" // ohci_setup
#شامل "usb-uas.h" // usb_uas_setup
#شامل "usb-uhci.h" // uhci_setup
#شامل "util.h" // msleep
#شامل "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
ساکن ساخت usb_pipe *
usb_realloc_pipe(ساخت usbdevice_s *usbdev, ساخت usb_pipe *pipe
                 , ساخت usb_endpoint_descriptor *epdesc)
{
    انتخاب (usbdev->hub->cntl->type) {
    طےشدہ:
    حالت USB_TYPE_UHCI:
        واپس uhci_realloc_pipe(usbdev, pipe, epdesc);
    حالت USB_TYPE_OHCI:
        واپس ohci_realloc_pipe(usbdev, pipe, epdesc);
    حالت USB_TYPE_EHCI:
        واپس ehci_realloc_pipe(usbdev, pipe, epdesc);
    حالت USB_TYPE_XHCI:
        واپس xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ساکن صحیح
usb_send_pipe(ساخت usb_pipe *pipe_fl, صحیح dir, ثابت خالی *cmd
              , خالی *ڈیٹا, صحیح datasize)
{
    انتخاب (GET_LOWFLAT(pipe_fl->type)) {
    طےشدہ:
    حالت USB_TYPE_UHCI:
        واپس uhci_send_pipe(pipe_fl, dir, cmd, ڈیٹا, datasize);
    حالت USB_TYPE_OHCI:
        اگر (MODESEGMENT)
            واپس -1;
        واپس ohci_send_pipe(pipe_fl, dir, cmd, ڈیٹا, datasize);
    حالت USB_TYPE_EHCI:
        واپس ehci_send_pipe(pipe_fl, dir, cmd, ڈیٹا, datasize);
    حالت USB_TYPE_XHCI:
        اگر (MODESEGMENT)
            واپس -1;
        واپس xhci_send_pipe(pipe_fl, dir, cmd, ڈیٹا, datasize);
    }
}

صحیح
usb_poll_intr(ساخت usb_pipe *pipe_fl, خالی *ڈیٹا)
{
    ASSERT16();
    انتخاب (GET_LOWFLAT(pipe_fl->type)) {
    طےشدہ:
    حالت USB_TYPE_UHCI:
        واپس uhci_poll_intr(pipe_fl, ڈیٹا);
    حالت USB_TYPE_OHCI:
        واپس ohci_poll_intr(pipe_fl, ڈیٹا);
    حالت USB_TYPE_EHCI:
        واپس ehci_poll_intr(pipe_fl, ڈیٹا);
    حالت USB_TYPE_XHCI: ;
        واپس call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), ڈیٹا), 0, -1);
    }
}

صحیح usb_32bit_pipe(ساخت usb_pipe *pipe_fl)
{
    واپس (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
ساخت usb_pipe *
usb_alloc_pipe(ساخت usbdevice_s *usbdev
               , ساخت usb_endpoint_descriptor *epdesc)
{
    واپس usb_realloc_pipe(usbdev, تہی, epdesc);
}

// Free an allocated control or bulk pipe.
خالی
usb_free_pipe(ساخت usbdevice_s *usbdev, ساخت usb_pipe *pipe)
{
    اگر (!pipe)
        واپس;
    usb_realloc_pipe(usbdev, pipe, تہی);
}

// Send a message to the default control pipe of a device.
صحیح
usb_send_default_control(ساخت usb_pipe *pipe, ثابت ساخت usb_ctrlrequest *req
                         , خالی *ڈیٹا)
{
    واپس usb_send_pipe(pipe, req->bRequestType & USB_DIR_اندر, req
                         , ڈیٹا, req->w_لمبائی);
}

// Send a message to a bulk endpoint
صحیح
usb_send_bulk(ساخت usb_pipe *pipe_fl, صحیح dir, خالی *ڈیٹا, صحیح datasize)
{
    واپس usb_send_pipe(pipe_fl, dir, تہی, ڈیٹا, datasize);
}

// Check if a pipe for a given controller is on the freelist
صحیح
usb_is_freelist(ساخت usb_s *cntl, ساخت usb_pipe *pipe)
{
    واپس pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
خالی
usb_جوڑو_freelist(ساخت usb_pipe *pipe)
{
    اگر (!pipe)
        واپس;
    ساخت usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
ساخت usb_pipe *
usb_get_freelist(ساخت usb_s *cntl, u8 eptype)
{
    ساخت usb_pipe **pfree = &cntl->freelist;
    برائے (;;) {
        ساخت usb_pipe *pipe = *pfree;
        اگر (!pipe)
            واپس تہی;
        اگر (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            واپس pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
خالی
usb_desc2pipe(ساخت usb_pipe *pipe, ساخت usbdevice_s *usbdev
              , ساخت usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->b_Endpoint_پتہ & USB_ENDPOINT_نمبر_نقاب;
    pipe->devaddr = usbdev->devaddr;
    pipe->رفتار = usbdev->رفتار;
    pipe->maxpacket = epdesc->w_Max_Packet_سائز;
    pipe->eptype = epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب;
}

// Find the exponential period of the requested interrupt end point.
صحیح
usb_get_period(ساخت usbdevice_s *usbdev
               , ساخت usb_endpoint_descriptor *epdesc)
{
    صحیح period = epdesc->bInterval;
    اگر (usbdev->رفتار != USB_HIGHSPEED)
        واپس (period <= 0) ? 0 : __fls(period);
    واپس (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
صحیح
usb_xfer_وقت(ساخت usb_pipe *pipe, صحیح datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    اگر (!GET_LOWFLAT(pipe->devaddr))
        واپس USB_وقت_حالت + 100;
    واپس USB_وقت_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
ساخت usb_endpoint_descriptor *
usb_ڈھونڈو_desc(ساخت usbdevice_s *usbdev, صحیح type, صحیح dir)
{
    ساخت usb_endpoint_descriptor *epdesc = (خالی*)&usbdev->iface[1];
    برائے (;;) {
        اگر ((خالی*)epdesc >= (خالی*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            واپس تہی;
        }
        اگر (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->b_Endpoint_پتہ & USB_ENDPOINT_DIR_نقاب) == dir
            && (epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب) == type)
            واپس epdesc;
        epdesc = (خالی*)epdesc + epdesc->b_لمبائی;
    }
}

// Get the first 8 bytes of the device descriptor.
ساکن صحیح
get_device_info8(ساخت usb_pipe *pipe, ساخت usb_device_descriptor *dinfo)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_اندر | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_قدر = USB_DT_DEVICE<<8;
    req.w_اشاریہ = 0;
    req.w_لمبائی = 8;
    واپس usb_send_default_control(pipe, &req, dinfo);
}

ساکن ساخت usb_config_descriptor *
get_device_config(ساخت usb_pipe *pipe)
{
    ساخت usb_config_descriptor cfg;

    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_اندر | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_قدر = USB_DT_CONFIG<<8;
    req.w_اشاریہ = 0;
    req.w_لمبائی = حجم(cfg);
    صحیح ret = usb_send_default_control(pipe, &req, &cfg);
    اگر (ret)
        واپس تہی;

    ساخت usb_config_descriptor *config = malloc_tmphigh(cfg.w_کل_لمبائی);
    اگر (!config) {
        warn_noalloc();
        واپس تہی;
    }
    req.w_لمبائی = cfg.w_کل_لمبائی;
    ret = usb_send_default_control(pipe, &req, config);
    اگر (ret || config->w_کل_لمبائی != cfg.w_کل_لمبائی) {
        آزاد(config);
        واپس تہی;
    }
    //hexdump(config, cfg.wTotalLength);
    واپس config;
}

ساکن صحیح
سیٹ_configuration(ساخت usb_pipe *pipe, u16 val)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_برآمد | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_سیٹ_CONFIGURATION;
    req.w_قدر = val;
    req.w_اشاریہ = 0;
    req.w_لمبائی = 0;
    واپس usb_send_default_control(pipe, &req, تہی);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ساکن ثابت صحیح رفتار_مقصد_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ساکن صحیح
usb_سیٹ_پتہ(ساخت usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ساخت usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    اگر (cntl->maxaddr >= USB_MAXADDR)
        واپس -1;

    msleep(USB_وقت_RSTRCY);

    // Create a pipe for the default address.
    ساخت usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_سائز = رفتار_مقصد_ctlsize[usbdev->رفتار],
        .bm_اوصاف = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    اگر (!usbdev->defpipe)
        واپس -1;

    // Send set_address command.
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_برآمد | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_سیٹ_پتہ;
    req.w_قدر = cntl->maxaddr + 1;
    req.w_اشاریہ = 0;
    req.w_لمبائی = 0;
    صحیح ret = usb_send_default_control(usbdev->defpipe, &req, تہی);
    اگر (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        واپس -1;
    }

    msleep(USB_وقت_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    اگر (!usbdev->defpipe)
        واپس -1;
    واپس 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ساکن صحیح
configure_usb_device(ساخت usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    ساخت usb_device_descriptor dinfo;
    صحیح ret = get_device_info8(usbdev->defpipe, &dinfo);
    اگر (ret)
        واپس 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    اگر (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    اگر (maxpacket < 8)
        واپس 0;
    ساخت usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_سائز = maxpacket,
        .bm_اوصاف = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    اگر (!usbdev->defpipe)
        واپس -1;

    // Get configuration
    ساخت usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    اگر (!config)
        واپس 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    صحیح عدد_iface = config->b_عدد_Interfaces;
    خالی *config_انتہا = (خالی*)config + config->w_کل_لمبائی;
    ساخت usb_interface_descriptor *iface = (خالی*)(&config[1]);
    برائے (;;) {
        اگر (!عدد_iface || (خالی*)iface + iface->b_لمبائی > config_انتہا)
            // Not a supported device.
            جاؤ fail;
        اگر (iface->bDescriptorType == USB_DT_INTERFACE) {
            عدد_iface--;
            اگر (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                توڑو;
        }
        iface = (خالی*)iface + iface->b_لمبائی;
    }

    // Set the configuration.
    ret = سیٹ_configuration(usbdev->defpipe, config->b_Configuration_قدر);
    اگر (ret)
        جاؤ fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (خالی*)config + config->w_کل_لمبائی - (خالی*)iface;
    اگر (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_تیاری(usbdev);
    ورنہ اگر (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        اگر (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_تیاری(usbdev);
        اگر (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_تیاری(usbdev);
    } ورنہ
        ret = usb_hid_تیاری(usbdev);
    اگر (ret)
        جاؤ fail;

    آزاد(config);
    واپس 1;
fail:
    آزاد(config);
    واپس 0;
}

ساکن خالی
usb_hub_پورٹ_تیاری(خالی *ڈیٹا)
{
    ساخت usbdevice_s *usbdev = ڈیٹا;
    ساخت usbhub_s *hub = usbdev->hub;
    u32 پورٹ = usbdev->پورٹ;

    برائے (;;) {
        // Detect if device present (and possibly start reset)
        صحیح ret = hub->op->detect(hub, پورٹ);
        اگر (ret > 0)
            // Device connected.
            توڑو;
        اگر (ret < 0 || timer_check(hub->detectend))
            // No device found.
            جاؤ مکمل;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_تالا(&hub->cntl->resetlock);
    صحیح ret = hub->op->reset(hub, پورٹ);
    اگر (ret < 0)
        // Reset failed
        جاؤ resetfail;
    usbdev->رفتار = ret;

    // Set address of port
    ret = usb_سیٹ_پتہ(usbdev);
    اگر (ret) {
        hub->op->disconnect(hub, پورٹ);
        جاؤ resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    صحیح گنتی = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    اگر (!گنتی)
        hub->op->disconnect(hub, پورٹ);
    hub->devcount += گنتی;
مکمل:
    hub->دھاگے--;
    آزاد(usbdev);
    واپس;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    جاؤ مکمل;
}

ساکن u32 usb_وقت_sigatt;

خالی
usb_enumerate(ساخت usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->دھاگے = portcount;
    hub->detectend = timer_calc(usb_وقت_sigatt);

    // Launch a thread for every port.
    صحیح i;
    برائے (i=0; i<portcount; i++) {
        ساخت usbdevice_s *usbdev = malloc_tmphigh(حجم(*usbdev));
        اگر (!usbdev) {
            warn_noalloc();
            جاری;
        }
        memset(usbdev, 0, حجم(*usbdev));
        usbdev->hub = hub;
        usbdev->پورٹ = i;
        تسلسل_thread(usb_hub_پورٹ_تیاری, usbdev);
    }

    // Wait for threads to complete.
    جب (hub->دھاگے)
        yield();
}

خالی
usb_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (! CONFIG_USB)
        واپس;
    dprintf(3, "init usb\n");
    usb_وقت_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_وقت_SIGATT);
    xhci_تیاری();
    ehci_تیاری();
    uhci_تیاری();
    ohci_تیاری();
}
