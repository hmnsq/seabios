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
        ပွၵ်ႈ uhci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_OHCI:
        ပွၵ်ႈ ohci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_EHCI:
        ပွၵ်ႈ ehci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_XHCI:
        ပွၵ်ႈ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
static puur_naa_nka
usb_send_pipe(sa_nracanaa usb_pipe *pipe_fl, puur_naa_nka dir, const void *cmd
              , void *data, puur_naa_nka datasize)
{
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    pak_sa USB_TYPE_UHCI:
        ပွၵ်ႈ uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_OHCI:
        ပေႃး (MODESEGMENT)
            ပွၵ်ႈ -1;
        ပွၵ်ႈ ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_EHCI:
        ပွၵ်ႈ ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_XHCI:
        ပေႃး (MODESEGMENT)
            ပွၵ်ႈ -1;
        ပွၵ်ႈ xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

puur_naa_nka
usb_poll_intr(sa_nracanaa usb_pipe *pipe_fl, void *data)
{
    ASSERT16();
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    pak_sa USB_TYPE_UHCI:
        ပွၵ်ႈ uhci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_OHCI:
        ပွၵ်ႈ ohci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_EHCI:
        ပွၵ်ႈ ehci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_XHCI: ;
        ပွၵ်ႈ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

puur_naa_nka usb_32bit_pipe(sa_nracanaa usb_pipe *pipe_fl)
{
    ပွၵ်ႈ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
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
    ပွၵ်ႈ usb_realloc_pipe(usbdev, NULL, epdesc);
}

// Free an allocated control or bulk pipe.
void
usb_free_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *pipe)
{
    ပေႃး (!pipe)
        ပွၵ်ႈ;
    usb_realloc_pipe(usbdev, pipe, NULL);
}

// Send a message to the default control pipe of a device.
puur_naa_nka
usb_send_default_control(sa_nracanaa usb_pipe *pipe, const sa_nracanaa usb_ctrlrequest *req
                         , void *data)
{
    ပွၵ်ႈ usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
puur_naa_nka
usb_send_bulk(sa_nracanaa usb_pipe *pipe_fl, puur_naa_nka dir, void *data, puur_naa_nka datasize)
{
    ပွၵ်ႈ usb_send_pipe(pipe_fl, dir, NULL, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
puur_naa_nka
usb_is_freelist(sa_nracanaa usb_s *cntl, sa_nracanaa usb_pipe *pipe)
{
    ပွၵ်ႈ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
void
usb_add_freelist(sa_nracanaa usb_pipe *pipe)
{
    ပေႃး (!pipe)
        ပွၵ်ႈ;
    sa_nracanaa usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
sa_nracanaa usb_pipe *
usb_get_freelist(sa_nracanaa usb_s *cntl, u8 eptype)
{
    sa_nracanaa usb_pipe **pfree = &cntl->freelist;
    for (;;) {
        sa_nracanaa usb_pipe *pipe = *pfree;
        ပေႃး (!pipe)
            ပွၵ်ႈ NULL;
        ပေႃး (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ပွၵ်ႈ pipe;
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
puur_naa_nka
usb_get_period(sa_nracanaa usbdevice_s *usbdev
               , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    puur_naa_nka period = epdesc->bInterval;
    ပေႃး (usbdev->speed != USB_HIGHSPEED)
        ပွၵ်ႈ (period <= 0) ? 0 : __fls(period);
    ပွၵ်ႈ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
puur_naa_nka
usb_xfer_time(sa_nracanaa usb_pipe *pipe, puur_naa_nka datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ပေႃး (!GET_LOWFLAT(pipe->devaddr))
        ပွၵ်ႈ USB_TIME_STATUS + 100;
    ပွၵ်ႈ USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
sa_nracanaa usb_endpoint_descriptor *
usb_find_desc(sa_nracanaa usbdevice_s *usbdev, puur_naa_nka type, puur_naa_nka dir)
{
    sa_nracanaa usb_endpoint_descriptor *epdesc = (void*)&usbdev->iface[1];
    for (;;) {
        ပေႃး ((void*)epdesc >= (void*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ပွၵ်ႈ NULL;
        }
        ပေႃး (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            ပွၵ်ႈ epdesc;
        epdesc = (void*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
static puur_naa_nka
get_device_info8(sa_nracanaa usb_pipe *pipe, sa_nracanaa usb_device_descriptor *dinfo)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    ပွၵ်ႈ usb_send_default_control(pipe, &req, dinfo);
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
    puur_naa_nka ret = usb_send_default_control(pipe, &req, &cfg);
    ပေႃး (ret)
        ပွၵ်ႈ NULL;

    sa_nracanaa usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    ပေႃး (!config) {
        warn_noalloc();
        ပွၵ်ႈ NULL;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    ပေႃး (ret || config->wTotalLength != cfg.wTotalLength) {
        free(config);
        ပွၵ်ႈ NULL;
    }
    //hexdump(config, cfg.wTotalLength);
    ပွၵ်ႈ config;
}

static puur_naa_nka
set_configuration(sa_nracanaa usb_pipe *pipe, u16 val)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.wValue = val;
    req.wIndex = 0;
    req.wLength = 0;
    ပွၵ်ႈ usb_send_default_control(pipe, &req, NULL);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

static const puur_naa_nka speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
static puur_naa_nka
usb_set_address(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    sa_nracanaa usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ပေႃး (cntl->maxaddr >= USB_MAXADDR)
        ပွၵ်ႈ -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    sa_nracanaa usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ပေႃး (!usbdev->defpipe)
        ပွၵ်ႈ -1;

    // Send set_address command.
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.wValue = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    puur_naa_nka ret = usb_send_default_control(usbdev->defpipe, &req, NULL);
    ပေႃး (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ပွၵ်ႈ -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ပေႃး (!usbdev->defpipe)
        ပွၵ်ႈ -1;
    ပွၵ်ႈ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
static puur_naa_nka
configure_usb_device(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    sa_nracanaa usb_device_descriptor dinfo;
    puur_naa_nka ret = get_device_info8(usbdev->defpipe, &dinfo);
    ပေႃး (ret)
        ပွၵ်ႈ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ပေႃး (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ပေႃး (maxpacket < 8)
        ပွၵ်ႈ 0;
    sa_nracanaa usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ပေႃး (!usbdev->defpipe)
        ပွၵ်ႈ -1;

    // Get configuration
    sa_nracanaa usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ပေႃး (!config)
        ပွၵ်ႈ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    puur_naa_nka num_iface = config->bNumInterfaces;
    void *config_end = (void*)config + config->wTotalLength;
    sa_nracanaa usb_interface_descriptor *iface = (void*)(&config[1]);
    for (;;) {
        ပေႃး (!num_iface || (void*)iface + iface->bLength > config_end)
            // Not a supported device.
            goto fail;
        ပေႃး (iface->bDescriptorType == USB_DT_INTERFACE) {
            num_iface--;
            ပေႃး (iface->bInterfaceClass == USB_CLASS_HUB
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
    ပေႃး (ret)
        goto fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (void*)config + config->wTotalLength - (void*)iface;
    ပေႃး (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    ပေႃးဢမ်ႇ ပေႃး (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ပေႃး (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        ပေႃး (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } ပေႃးဢမ်ႇ
        ret = usb_hid_setup(usbdev);
    ပေႃး (ret)
        goto fail;

    free(config);
    ပွၵ်ႈ 1;
fail:
    free(config);
    ပွၵ်ႈ 0;
}

static void
usb_hub_port_setup(void *data)
{
    sa_nracanaa usbdevice_s *usbdev = data;
    sa_nracanaa usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    for (;;) {
        // Detect if device present (and possibly start reset)
        puur_naa_nka ret = hub->op->detect(hub, port);
        ပေႃး (ret > 0)
            // Device connected.
            bha_nga;
        ပေႃး (ret < 0 || timer_check(hub->detectend))
            // No device found.
            goto done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    puur_naa_nka ret = hub->op->reset(hub, port);
    ပေႃး (ret < 0)
        // Reset failed
        goto resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    ပေႃး (ret) {
        hub->op->disconnect(hub, port);
        goto resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    puur_naa_nka count = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ပေႃး (!count)
        hub->op->disconnect(hub, port);
    hub->devcount += count;
done:
    hub->threads--;
    free(usbdev);
    ပွၵ်ႈ;

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
    puur_naa_nka i;
    for (i=0; i<portcount; i++) {
        sa_nracanaa usbdevice_s *usbdev = malloc_tmphigh(parimaa_na(*usbdev));
        ပေႃး (!usbdev) {
            warn_noalloc();
            continue;
        }
        memset(usbdev, 0, parimaa_na(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    ၶၢဝ်း (hub->threads)
        yield();
}

void
usb_setup(void)
{
    ASSERT32FLAT();
    ပေႃး (! CONFIG_USB)
        ပွၵ်ႈ;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
