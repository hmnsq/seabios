// Main code for handling USB controllers and devices.
//
// Copyright (C) 2009-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBAL
#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "output.h" // dprintf
#include "romfile.h" // romfile_loadint
#include "string.h" // memset
#include "usb.h" // struct usb_s
#include "usb-ehci.h" // ehci_setup
#include "usb-xhci.h" // xhci_setup
#include "usb-hid.h" // usb_keyboard_setup
#include "usb-hub.h" // usb_hub_setup
#include "usb-msc.h" // usb_msc_setup
#include "usb-ohci.h" // ohci_setup
#include "usb-uas.h" // usb_uas_setup
#include "usb-uhci.h" // uhci_setup
#include "util.h" // msleep
#include "x86.h" // __fls


/****************************************************************
 * Controller function wrappers
 ****************************************************************/

// Allocate, update, or free a usb pipe.
static sa_nracanaa usb_pipe *
usb_realloc_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *pipe
                 , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    switch (usbdev->hub->cntl->type) {
    default:
    pak_sa USB_TYPE_UHCI:
        ត្រឡប់ uhci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_OHCI:
        ត្រឡប់ ohci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_EHCI:
        ត្រឡប់ ehci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_XHCI:
        ត្រឡប់ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
static ចំនួនគត់
usb_send_pipe(sa_nracanaa usb_pipe *pipe_fl, ចំនួនគត់ dir, const void *cmd
              , void *data, ចំនួនគត់ datasize)
{
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    pak_sa USB_TYPE_UHCI:
        ត្រឡប់ uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_OHCI:
        ប្រសិនបើ (MODESEGMENT)
            ត្រឡប់ -1;
        ត្រឡប់ ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_EHCI:
        ត្រឡប់ ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_XHCI:
        ប្រសិនបើ (MODESEGMENT)
            ត្រឡប់ -1;
        ត្រឡប់ xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

ចំនួនគត់
usb_poll_intr(sa_nracanaa usb_pipe *pipe_fl, void *data)
{
    ASSERT16();
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    pak_sa USB_TYPE_UHCI:
        ត្រឡប់ uhci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_OHCI:
        ត្រឡប់ ohci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_EHCI:
        ត្រឡប់ ehci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_XHCI: ;
        ត្រឡប់ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

ចំនួនគត់ usb_32bit_pipe(sa_nracanaa usb_pipe *pipe_fl)
{
    ត្រឡប់ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
sa_nracanaa usb_pipe *
usb_alloc_pipe(sa_nracanaa usbdevice_s *usbdev
               , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ត្រឡប់ usb_realloc_pipe(usbdev, NULL, epdesc);
}

// Free an allocated control or bulk pipe.
void
usb_free_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *pipe)
{
    ប្រសិនបើ (!pipe)
        ត្រឡប់;
    usb_realloc_pipe(usbdev, pipe, NULL);
}

// Send a message to the default control pipe of a device.
ចំនួនគត់
usb_send_default_control(sa_nracanaa usb_pipe *pipe, const sa_nracanaa usb_ctrlrequest *req
                         , void *data)
{
    ត្រឡប់ usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
ចំនួនគត់
usb_send_bulk(sa_nracanaa usb_pipe *pipe_fl, ចំនួនគត់ dir, void *data, ចំនួនគត់ datasize)
{
    ត្រឡប់ usb_send_pipe(pipe_fl, dir, NULL, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
ចំនួនគត់
usb_is_freelist(sa_nracanaa usb_s *cntl, sa_nracanaa usb_pipe *pipe)
{
    ត្រឡប់ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
void
usb_add_freelist(sa_nracanaa usb_pipe *pipe)
{
    ប្រសិនបើ (!pipe)
        ត្រឡប់;
    sa_nracanaa usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
sa_nracanaa usb_pipe *
usb_get_freelist(sa_nracanaa usb_s *cntl, u8 eptype)
{
    sa_nracanaa usb_pipe **pfree = &cntl->freelist;
    សម្រាប់ (;;) {
        sa_nracanaa usb_pipe *pipe = *pfree;
        ប្រសិនបើ (!pipe)
            ត្រឡប់ NULL;
        ប្រសិនបើ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ត្រឡប់ pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
void
usb_desc2pipe(sa_nracanaa usb_pipe *pipe, sa_nracanaa usbdevice_s *usbdev
              , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    pipe->cntl = usbdev->hub->cntl;
    pipe->type = usbdev->hub->cntl->type;
    pipe->ep = epdesc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
    pipe->devaddr = usbdev->devaddr;
    pipe->speed = usbdev->speed;
    pipe->maxpacket = epdesc->wMaxPacketSize;
    pipe->eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

// Find the exponential period of the requested interrupt end point.
ចំនួនគត់
usb_get_period(sa_nracanaa usbdevice_s *usbdev
               , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ចំនួនគត់ period = epdesc->bInterval;
    ប្រសិនបើ (usbdev->speed != USB_HIGHSPEED)
        ត្រឡប់ (period <= 0) ? 0 : __fls(period);
    ត្រឡប់ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
ចំនួនគត់
usb_xfer_time(sa_nracanaa usb_pipe *pipe, ចំនួនគត់ datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ប្រសិនបើ (!GET_LOWFLAT(pipe->devaddr))
        ត្រឡប់ USB_TIME_STATUS + 100;
    ត្រឡប់ USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
sa_nracanaa usb_endpoint_descriptor *
usb_find_desc(sa_nracanaa usbdevice_s *usbdev, ចំនួនគត់ type, ចំនួនគត់ dir)
{
    sa_nracanaa usb_endpoint_descriptor *epdesc = (void*)&usbdev->iface[1];
    សម្រាប់ (;;) {
        ប្រសិនបើ ((void*)epdesc >= (void*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ត្រឡប់ NULL;
        }
        ប្រសិនបើ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            ត្រឡប់ epdesc;
        epdesc = (void*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
static ចំនួនគត់
get_device_info8(sa_nracanaa usb_pipe *pipe, sa_nracanaa usb_device_descriptor *dinfo)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    ត្រឡប់ usb_send_default_control(pipe, &req, dinfo);
}

static sa_nracanaa usb_config_descriptor *
get_device_config(sa_nracanaa usb_pipe *pipe)
{
    sa_nracanaa usb_config_descriptor cfg;

    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = parimaa_na(cfg);
    ចំនួនគត់ ret = usb_send_default_control(pipe, &req, &cfg);
    ប្រសិនបើ (ret)
        ត្រឡប់ NULL;

    sa_nracanaa usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    ប្រសិនបើ (!config) {
        warn_noalloc();
        ត្រឡប់ NULL;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    ប្រសិនបើ (ret || config->wTotalLength != cfg.wTotalLength) {
        free(config);
        ត្រឡប់ NULL;
    }
    //hexdump(config, cfg.wTotalLength);
    ត្រឡប់ config;
}

static ចំនួនគត់
set_configuration(sa_nracanaa usb_pipe *pipe, u16 val)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.wValue = val;
    req.wIndex = 0;
    req.wLength = 0;
    ត្រឡប់ usb_send_default_control(pipe, &req, NULL);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

static const ចំនួនគត់ speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
static ចំនួនគត់
usb_set_address(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    sa_nracanaa usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ប្រសិនបើ (cntl->maxaddr >= USB_MAXADDR)
        ត្រឡប់ -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    sa_nracanaa usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ប្រសិនបើ (!usbdev->defpipe)
        ត្រឡប់ -1;

    // Send set_address command.
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.wValue = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    ចំនួនគត់ ret = usb_send_default_control(usbdev->defpipe, &req, NULL);
    ប្រសិនបើ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ត្រឡប់ -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ប្រសិនបើ (!usbdev->defpipe)
        ត្រឡប់ -1;
    ត្រឡប់ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
static ចំនួនគត់
configure_usb_device(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    sa_nracanaa usb_device_descriptor dinfo;
    ចំនួនគត់ ret = get_device_info8(usbdev->defpipe, &dinfo);
    ប្រសិនបើ (ret)
        ត្រឡប់ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ប្រសិនបើ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ប្រសិនបើ (maxpacket < 8)
        ត្រឡប់ 0;
    sa_nracanaa usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ប្រសិនបើ (!usbdev->defpipe)
        ត្រឡប់ -1;

    // Get configuration
    sa_nracanaa usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ប្រសិនបើ (!config)
        ត្រឡប់ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    ចំនួនគត់ num_iface = config->bNumInterfaces;
    void *config_end = (void*)config + config->wTotalLength;
    sa_nracanaa usb_interface_descriptor *iface = (void*)(&config[1]);
    សម្រាប់ (;;) {
        ប្រសិនបើ (!num_iface || (void*)iface + iface->bLength > config_end)
            // Not a supported device.
            goto fail;
        ប្រសិនបើ (iface->bDescriptorType == USB_DT_INTERFACE) {
            num_iface--;
            ប្រសិនបើ (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                bha_nga;
        }
        iface = (void*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->bConfigurationValue);
    ប្រសិនបើ (ret)
        goto fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (void*)config + config->wTotalLength - (void*)iface;
    ប្រសិនបើ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    បើមិនដូច្នេះ ប្រសិនបើ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ប្រសិនបើ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        ប្រសិនបើ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } បើមិនដូច្នេះ
        ret = usb_hid_setup(usbdev);
    ប្រសិនបើ (ret)
        goto fail;

    free(config);
    ត្រឡប់ 1;
fail:
    free(config);
    ត្រឡប់ 0;
}

static void
usb_hub_port_setup(void *data)
{
    sa_nracanaa usbdevice_s *usbdev = data;
    sa_nracanaa usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    សម្រាប់ (;;) {
        // Detect if device present (and possibly start reset)
        ចំនួនគត់ ret = hub->op->detect(hub, port);
        ប្រសិនបើ (ret > 0)
            // Device connected.
            bha_nga;
        ប្រសិនបើ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            goto done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    ចំនួនគត់ ret = hub->op->reset(hub, port);
    ប្រសិនបើ (ret < 0)
        // Reset failed
        goto resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    ប្រសិនបើ (ret) {
        hub->op->disconnect(hub, port);
        goto resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    ចំនួនគត់ count = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ប្រសិនបើ (!count)
        hub->op->disconnect(hub, port);
    hub->devcount += count;
done:
    hub->threads--;
    free(usbdev);
    ត្រឡប់;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    goto done;
}

static u32 usb_time_sigatt;

void
usb_enumerate(sa_nracanaa usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    ចំនួនគត់ i;
    សម្រាប់ (i=0; i<portcount; i++) {
        sa_nracanaa usbdevice_s *usbdev = malloc_tmphigh(parimaa_na(*usbdev));
        ប្រសិនបើ (!usbdev) {
            warn_noalloc();
            continue;
        }
        memset(usbdev, 0, parimaa_na(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    ខណៈពេល (hub->threads)
        yield();
}

void
usb_setup(void)
{
    ASSERT32FLAT();
    ប្រសិនបើ (! CONFIG_USB)
        ត្រឡប់;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
