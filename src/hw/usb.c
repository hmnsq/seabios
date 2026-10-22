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
ಸ್ಥಿರ ರಚನೆ usb_pipe *
usb_realloc_pipe(ರಚನೆ usbdevice_s *usbdev, ರಚನೆ usb_pipe *pipe
                 , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಆಯ್ಕೆ (usbdev->hub->cntl->type) {
    ಪೂರ್ವನಿಯೋಜಿತ:
    ಸ್ಥಿತಿ USB_TYPE_UHCI:
        ಹಿಂತಿರುಗಿ uhci_realloc_pipe(usbdev, pipe, epdesc);
    ಸ್ಥಿತಿ USB_TYPE_OHCI:
        ಹಿಂತಿರುಗಿ ohci_realloc_pipe(usbdev, pipe, epdesc);
    ಸ್ಥಿತಿ USB_TYPE_EHCI:
        ಹಿಂತಿರುಗಿ ehci_realloc_pipe(usbdev, pipe, epdesc);
    ಸ್ಥಿತಿ USB_TYPE_XHCI:
        ಹಿಂತಿರುಗಿ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_send_pipe(ರಚನೆ usb_pipe *pipe_fl, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
              , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize)
{
    ಆಯ್ಕೆ (GET_LOWFLAT(pipe_fl->type)) {
    ಪೂರ್ವನಿಯೋಜಿತ:
    ಸ್ಥಿತಿ USB_TYPE_UHCI:
        ಹಿಂತಿರುಗಿ uhci_send_pipe(pipe_fl, dir, cmd, ದತ್ತಾಂಶ, datasize);
    ಸ್ಥಿತಿ USB_TYPE_OHCI:
        ಆದರೆ (MODESEGMENT)
            ಹಿಂತಿರುಗಿ -1;
        ಹಿಂತಿರುಗಿ ohci_send_pipe(pipe_fl, dir, cmd, ದತ್ತಾಂಶ, datasize);
    ಸ್ಥಿತಿ USB_TYPE_EHCI:
        ಹಿಂತಿರುಗಿ ehci_send_pipe(pipe_fl, dir, cmd, ದತ್ತಾಂಶ, datasize);
    ಸ್ಥಿತಿ USB_TYPE_XHCI:
        ಆದರೆ (MODESEGMENT)
            ಹಿಂತಿರುಗಿ -1;
        ಹಿಂತಿರುಗಿ xhci_send_pipe(pipe_fl, dir, cmd, ದತ್ತಾಂಶ, datasize);
    }
}

ಪೂರ್ಣಾಂಕ
usb_poll_intr(ರಚನೆ usb_pipe *pipe_fl, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ASSERT16();
    ಆಯ್ಕೆ (GET_LOWFLAT(pipe_fl->type)) {
    ಪೂರ್ವನಿಯೋಜಿತ:
    ಸ್ಥಿತಿ USB_TYPE_UHCI:
        ಹಿಂತಿರುಗಿ uhci_poll_intr(pipe_fl, ದತ್ತಾಂಶ);
    ಸ್ಥಿತಿ USB_TYPE_OHCI:
        ಹಿಂತಿರುಗಿ ohci_poll_intr(pipe_fl, ದತ್ತಾಂಶ);
    ಸ್ಥಿತಿ USB_TYPE_EHCI:
        ಹಿಂತಿರುಗಿ ehci_poll_intr(pipe_fl, ದತ್ತಾಂಶ);
    ಸ್ಥಿತಿ USB_TYPE_XHCI: ;
        ಹಿಂತಿರುಗಿ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), ದತ್ತಾಂಶ), 0, -1);
    }
}

ಪೂರ್ಣಾಂಕ usb_32bit_pipe(ರಚನೆ usb_pipe *pipe_fl)
{
    ಹಿಂತಿರುಗಿ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
ರಚನೆ usb_pipe *
usb_alloc_pipe(ರಚನೆ usbdevice_s *usbdev
               , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಹಿಂತಿರುಗಿ usb_realloc_pipe(usbdev, ಖಾಲಿ, epdesc);
}

// Free an allocated control or bulk pipe.
ಶೂನ್ಯ
usb_free_pipe(ರಚನೆ usbdevice_s *usbdev, ರಚನೆ usb_pipe *pipe)
{
    ಆದರೆ (!pipe)
        ಹಿಂತಿರುಗಿ;
    usb_realloc_pipe(usbdev, pipe, ಖಾಲಿ);
}

// Send a message to the default control pipe of a device.
ಪೂರ್ಣಾಂಕ
usb_send_default_control(ರಚನೆ usb_pipe *pipe, ಸ್ಥಿರಾಂಕ ರಚನೆ usb_ctrlrequest *req
                         , ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ಹಿಂತಿರುಗಿ usb_send_pipe(pipe, req->bRequestType & USB_DIR_ಒಳ, req
                         , ದತ್ತಾಂಶ, req->w_ಉದ್ದಳತೆ);
}

// Send a message to a bulk endpoint
ಪೂರ್ಣಾಂಕ
usb_send_bulk(ರಚನೆ usb_pipe *pipe_fl, ಪೂರ್ಣಾಂಕ dir, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize)
{
    ಹಿಂತಿರುಗಿ usb_send_pipe(pipe_fl, dir, ಖಾಲಿ, ದತ್ತಾಂಶ, datasize);
}

// Check if a pipe for a given controller is on the freelist
ಪೂರ್ಣಾಂಕ
usb_is_freelist(ರಚನೆ usb_s *cntl, ರಚನೆ usb_pipe *pipe)
{
    ಹಿಂತಿರುಗಿ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
ಶೂನ್ಯ
usb_ಕೂಡಿಸು_freelist(ರಚನೆ usb_pipe *pipe)
{
    ಆದರೆ (!pipe)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
ರಚನೆ usb_pipe *
usb_get_freelist(ರಚನೆ usb_s *cntl, u8 eptype)
{
    ರಚನೆ usb_pipe **pfree = &cntl->freelist;
    ಗಾಗಿ (;;) {
        ರಚನೆ usb_pipe *pipe = *pfree;
        ಆದರೆ (!pipe)
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        ಆದರೆ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ಹಿಂತಿರುಗಿ pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
ಶೂನ್ಯ
usb_desc2pipe(ರಚನೆ usb_pipe *pipe, ರಚನೆ usbdevice_s *usbdev
              , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->b_Endpoint_ವಿಳಾಸ & USB_ENDPOINT_ಸಂಖ್ಯೆ_ಮುಖವಾಡ;
    pipe->devaddr = usbdev->devaddr;
    pipe->ವೇಗ = usbdev->ವೇಗ;
    pipe->maxpacket = epdesc->w_Max_Packet_ಗಾತ್ರ;
    pipe->eptype = epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ;
}

// Find the exponential period of the requested interrupt end point.
ಪೂರ್ಣಾಂಕ
usb_get_period(ರಚನೆ usbdevice_s *usbdev
               , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಪೂರ್ಣಾಂಕ period = epdesc->bInterval;
    ಆದರೆ (usbdev->ವೇಗ != USB_HIGHSPEED)
        ಹಿಂತಿರುಗಿ (period <= 0) ? 0 : __fls(period);
    ಹಿಂತಿರುಗಿ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
ಪೂರ್ಣಾಂಕ
usb_xfer_ಸಮಯ(ರಚನೆ usb_pipe *pipe, ಪೂರ್ಣಾಂಕ datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ಆದರೆ (!GET_LOWFLAT(pipe->devaddr))
        ಹಿಂತಿರುಗಿ USB_ಸಮಯ_ಸ್ಥಿತಿ + 100;
    ಹಿಂತಿರುಗಿ USB_ಸಮಯ_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
ರಚನೆ usb_endpoint_descriptor *
usb_ಕಂಡುಹಿಡಿ_desc(ರಚನೆ usbdevice_s *usbdev, ಪೂರ್ಣಾಂಕ type, ಪೂರ್ಣಾಂಕ dir)
{
    ರಚನೆ usb_endpoint_descriptor *epdesc = (ಶೂನ್ಯ*)&usbdev->iface[1];
    ಗಾಗಿ (;;) {
        ಆದರೆ ((ಶೂನ್ಯ*)epdesc >= (ಶೂನ್ಯ*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        }
        ಆದರೆ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->b_Endpoint_ವಿಳಾಸ & USB_ENDPOINT_DIR_ಮುಖವಾಡ) == dir
            && (epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ) == type)
            ಹಿಂತಿರುಗಿ epdesc;
        epdesc = (ಶೂನ್ಯ*)epdesc + epdesc->b_ಉದ್ದಳತೆ;
    }
}

// Get the first 8 bytes of the device descriptor.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_device_info8(ರಚನೆ usb_pipe *pipe, ರಚನೆ usb_device_descriptor *dinfo)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಒಳ | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ಮೌಲ್ಯ = USB_DT_DEVICE<<8;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = 8;
    ಹಿಂತಿರುಗಿ usb_send_default_control(pipe, &req, dinfo);
}

ಸ್ಥಿರ ರಚನೆ usb_config_descriptor *
get_device_config(ರಚನೆ usb_pipe *pipe)
{
    ರಚನೆ usb_config_descriptor cfg;

    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಒಳ | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.w_ಮೌಲ್ಯ = USB_DT_CONFIG<<8;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = ಗಾತ್ರ(cfg);
    ಪೂರ್ಣಾಂಕ ret = usb_send_default_control(pipe, &req, &cfg);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    ರಚನೆ usb_config_descriptor *config = malloc_tmphigh(cfg.w_ಒಟ್ಟು_ಉದ್ದಳತೆ);
    ಆದರೆ (!config) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    req.w_ಉದ್ದಳತೆ = cfg.w_ಒಟ್ಟು_ಉದ್ದಳತೆ;
    ret = usb_send_default_control(pipe, &req, config);
    ಆದರೆ (ret || config->w_ಒಟ್ಟು_ಉದ್ದಳತೆ != cfg.w_ಒಟ್ಟು_ಉದ್ದಳತೆ) {
        ಬಿಡುಗಡೆ(config);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    //hexdump(config, cfg.wTotalLength);
    ಹಿಂತಿರುಗಿ config;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಗಣ_configuration(ರಚನೆ usb_pipe *pipe, u16 val)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಹೊರಹೋಗು | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_ಗಣ_CONFIGURATION;
    req.w_ಮೌಲ್ಯ = val;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = 0;
    ಹಿಂತಿರುಗಿ usb_send_default_control(pipe, &req, ಖಾಲಿ);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ವೇಗ_ಗಮ್ಯ_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_ಗಣ_ವಿಳಾಸ(ರಚನೆ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ರಚನೆ usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ಆದರೆ (cntl->maxaddr >= USB_MAXADDR)
        ಹಿಂತಿರುಗಿ -1;

    msleep(USB_ಸಮಯ_RSTRCY);

    // Create a pipe for the default address.
    ರಚನೆ usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_ಗಾತ್ರ = ವೇಗ_ಗಮ್ಯ_ctlsize[usbdev->ವೇಗ],
        .bm_ಗುಣಲಕ್ಷಣಗಳು = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ಆದರೆ (!usbdev->defpipe)
        ಹಿಂತಿರುಗಿ -1;

    // Send set_address command.
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಹೊರಹೋಗು | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_ಗಣ_ವಿಳಾಸ;
    req.w_ಮೌಲ್ಯ = cntl->maxaddr + 1;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = 0;
    ಪೂರ್ಣಾಂಕ ret = usb_send_default_control(usbdev->defpipe, &req, ಖಾಲಿ);
    ಆದರೆ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ಹಿಂತಿರುಗಿ -1;
    }

    msleep(USB_ಸಮಯ_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ಆದರೆ (!usbdev->defpipe)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
configure_usb_device(ರಚನೆ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    ರಚನೆ usb_device_descriptor dinfo;
    ಪೂರ್ಣಾಂಕ ret = get_device_info8(usbdev->defpipe, &dinfo);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ಆದರೆ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ಆದರೆ (maxpacket < 8)
        ಹಿಂತಿರುಗಿ 0;
    ರಚನೆ usb_endpoint_descriptor epdesc = {
        .w_Max_Packet_ಗಾತ್ರ = maxpacket,
        .bm_ಗುಣಲಕ್ಷಣಗಳು = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ಆದರೆ (!usbdev->defpipe)
        ಹಿಂತಿರುಗಿ -1;

    // Get configuration
    ರಚನೆ usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ಆದರೆ (!config)
        ಹಿಂತಿರುಗಿ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    ಪೂರ್ಣಾಂಕ ಅಂಕೆ_iface = config->b_ಅಂಕೆ_Interfaces;
    ಶೂನ್ಯ *config_ಕೊನೆ = (ಶೂನ್ಯ*)config + config->w_ಒಟ್ಟು_ಉದ್ದಳತೆ;
    ರಚನೆ usb_interface_descriptor *iface = (ಶೂನ್ಯ*)(&config[1]);
    ಗಾಗಿ (;;) {
        ಆದರೆ (!ಅಂಕೆ_iface || (ಶೂನ್ಯ*)iface + iface->b_ಉದ್ದಳತೆ > config_ಕೊನೆ)
            // Not a supported device.
            ಹೋಗು fail;
        ಆದರೆ (iface->bDescriptorType == USB_DT_INTERFACE) {
            ಅಂಕೆ_iface--;
            ಆದರೆ (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                ನಿಲ್ಲಿಸು;
        }
        iface = (ಶೂನ್ಯ*)iface + iface->b_ಉದ್ದಳತೆ;
    }

    // Set the configuration.
    ret = ಗಣ_configuration(usbdev->defpipe, config->b_Configuration_ಮೌಲ್ಯ);
    ಆದರೆ (ret)
        ಹೋಗು fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (ಶೂನ್ಯ*)config + config->w_ಒಟ್ಟು_ಉದ್ದಳತೆ - (ಶೂನ್ಯ*)iface;
    ಆದರೆ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_ಸಿದ್ಧತೆ(usbdev);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ಆದರೆ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_ಸಿದ್ಧತೆ(usbdev);
        ಆದರೆ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_ಸಿದ್ಧತೆ(usbdev);
    } ಇಲ್ಲದಿದ್ದರೆ
        ret = usb_hid_ಸಿದ್ಧತೆ(usbdev);
    ಆದರೆ (ret)
        ಹೋಗು fail;

    ಬಿಡುಗಡೆ(config);
    ಹಿಂತಿರುಗಿ 1;
fail:
    ಬಿಡುಗಡೆ(config);
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
usb_hub_ಪೋರ್ಟ್_ಸಿದ್ಧತೆ(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ usbdevice_s *usbdev = ದತ್ತಾಂಶ;
    ರಚನೆ usbhub_s *hub = usbdev->hub;
    u32 ಪೋರ್ಟ್ = usbdev->ಪೋರ್ಟ್;

    ಗಾಗಿ (;;) {
        // Detect if device present (and possibly start reset)
        ಪೂರ್ಣಾಂಕ ret = hub->op->detect(hub, ಪೋರ್ಟ್);
        ಆದರೆ (ret > 0)
            // Device connected.
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            ಹೋಗು ಮುಗಿದ;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_ಬೀಗ(&hub->cntl->resetlock);
    ಪೂರ್ಣಾಂಕ ret = hub->op->reset(hub, ಪೋರ್ಟ್);
    ಆದರೆ (ret < 0)
        // Reset failed
        ಹೋಗು resetfail;
    usbdev->ವೇಗ = ret;

    // Set address of port
    ret = usb_ಗಣ_ವಿಳಾಸ(usbdev);
    ಆದರೆ (ret) {
        hub->op->disconnect(hub, ಪೋರ್ಟ್);
        ಹೋಗು resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ಆದರೆ (!ಎಣಿಕೆ_2)
        hub->op->disconnect(hub, ಪೋರ್ಟ್);
    hub->devcount += ಎಣಿಕೆ_2;
ಮುಗಿದ:
    hub->ಎಳೆಗಳು--;
    ಬಿಡುಗಡೆ(usbdev);
    ಹಿಂತಿರುಗಿ;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    ಹೋಗು ಮುಗಿದ;
}

ಸ್ಥಿರ u32 usb_ಸಮಯ_sigatt;

ಶೂನ್ಯ
usb_enumerate(ರಚನೆ usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->ಎಳೆಗಳು = portcount;
    hub->detectend = timer_calc(usb_ಸಮಯ_sigatt);

    // Launch a thread for every port.
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<portcount; i++) {
        ರಚನೆ usbdevice_s *usbdev = malloc_tmphigh(ಗಾತ್ರ(*usbdev));
        ಆದರೆ (!usbdev) {
            warn_noalloc();
            ಮುಂದುವರಿ;
        }
        memset(usbdev, 0, ಗಾತ್ರ(*usbdev));
        usbdev->hub = hub;
        usbdev->ಪೋರ್ಟ್ = i;
        ಸತತ_thread(usb_hub_ಪೋರ್ಟ್_ಸಿದ್ಧತೆ, usbdev);
    }

    // Wait for threads to complete.
    ಆಗ (hub->ಎಳೆಗಳು)
        yield();
}

ಶೂನ್ಯ
usb_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_USB)
        ಹಿಂತಿರುಗಿ;
    dprintf(3, "init usb\n");
    usb_ಸಮಯ_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_ಸಮಯ_SIGATT);
    xhci_ಸಿದ್ಧತೆ();
    ehci_ಸಿದ್ಧತೆ();
    uhci_ಸಿದ್ಧತೆ();
    ohci_ಸಿದ್ಧತೆ();
}
