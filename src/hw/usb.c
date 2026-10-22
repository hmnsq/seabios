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
static struct usb_pipe *
usb_realloc_pipe(struct usbdevice_s *usbdev, struct usb_pipe *pipe
                 , struct usb_endpoint_descriptor *epdesc)
{
    switch (usbdev->hub->cntl->type) {
    default:
    case USB_TYPE_UHCI:
        върни uhci_realloc_pipe(usbdev, pipe, epdesc);
    case USB_TYPE_OHCI:
        върни ohci_realloc_pipe(usbdev, pipe, epdesc);
    case USB_TYPE_EHCI:
        върни ehci_realloc_pipe(usbdev, pipe, epdesc);
    case USB_TYPE_XHCI:
        върни xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
static цяло
usb_send_pipe(struct usb_pipe *pipe_fl, цяло dir, const void *cmd
              , void *data, цяло datasize)
{
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    case USB_TYPE_UHCI:
        върни uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    case USB_TYPE_OHCI:
        ако (MODESEGMENT)
            върни -1;
        върни ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    case USB_TYPE_EHCI:
        върни ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    case USB_TYPE_XHCI:
        ако (MODESEGMENT)
            върни -1;
        върни xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

цяло
usb_poll_intr(struct usb_pipe *pipe_fl, void *data)
{
    ASSERT16();
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    case USB_TYPE_UHCI:
        върни uhci_poll_intr(pipe_fl, data);
    case USB_TYPE_OHCI:
        върни ohci_poll_intr(pipe_fl, data);
    case USB_TYPE_EHCI:
        върни ehci_poll_intr(pipe_fl, data);
    case USB_TYPE_XHCI: ;
        върни call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

цяло usb_32bit_pipe(struct usb_pipe *pipe_fl)
{
    върни (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
        || (CONFIG_USB_OHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_OHCI);
}


/****************************************************************
 * Helper functions
 ****************************************************************/

// Allocate a usb pipe.
struct usb_pipe *
usb_alloc_pipe(struct usbdevice_s *usbdev
               , struct usb_endpoint_descriptor *epdesc)
{
    върни usb_realloc_pipe(usbdev, NULL, epdesc);
}

// Free an allocated control or bulk pipe.
void
usb_free_pipe(struct usbdevice_s *usbdev, struct usb_pipe *pipe)
{
    ако (!pipe)
        върни;
    usb_realloc_pipe(usbdev, pipe, NULL);
}

// Send a message to the default control pipe of a device.
цяло
usb_send_default_control(struct usb_pipe *pipe, const struct usb_ctrlrequest *req
                         , void *data)
{
    върни usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
цяло
usb_send_bulk(struct usb_pipe *pipe_fl, цяло dir, void *data, цяло datasize)
{
    върни usb_send_pipe(pipe_fl, dir, NULL, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
цяло
usb_is_freelist(struct usb_s *cntl, struct usb_pipe *pipe)
{
    върни pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
void
usb_add_freelist(struct usb_pipe *pipe)
{
    ако (!pipe)
        върни;
    struct usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
struct usb_pipe *
usb_get_freelist(struct usb_s *cntl, u8 eptype)
{
    struct usb_pipe **pfree = &cntl->freelist;
    за (;;) {
        struct usb_pipe *pipe = *pfree;
        ако (!pipe)
            върни NULL;
        ако (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            върни pipe;
        }
        pfree = &pipe->freenext;
    }
}

// Fill "pipe" endpoint info from an endpoint descriptor.
void
usb_desc2pipe(struct usb_pipe *pipe, struct usbdevice_s *usbdev
              , struct usb_endpoint_descriptor *epdesc)
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
цяло
usb_get_period(struct usbdevice_s *usbdev
               , struct usb_endpoint_descriptor *epdesc)
{
    цяло period = epdesc->bInterval;
    ако (usbdev->speed != USB_HIGHSPEED)
        върни (period <= 0) ? 0 : __fls(period);
    върни (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
цяло
usb_xfer_time(struct usb_pipe *pipe, цяло datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ако (!GET_LOWFLAT(pipe->devaddr))
        върни USB_TIME_STATUS + 100;
    върни USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
struct usb_endpoint_descriptor *
usb_find_desc(struct usbdevice_s *usbdev, цяло type, цяло dir)
{
    struct usb_endpoint_descriptor *epdesc = (void*)&usbdev->iface[1];
    за (;;) {
        ако ((void*)epdesc >= (void*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            върни NULL;
        }
        ако (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            върни epdesc;
        epdesc = (void*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
static цяло
get_device_info8(struct usb_pipe *pipe, struct usb_device_descriptor *dinfo)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    върни usb_send_default_control(pipe, &req, dinfo);
}

static struct usb_config_descriptor *
get_device_config(struct usb_pipe *pipe)
{
    struct usb_config_descriptor cfg;

    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_CONFIG<<8;
    req.wIndex = 0;
    req.wLength = sizeof(cfg);
    цяло ret = usb_send_default_control(pipe, &req, &cfg);
    ако (ret)
        върни NULL;

    struct usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    ако (!config) {
        warn_noalloc();
        върни NULL;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    ако (ret || config->wTotalLength != cfg.wTotalLength) {
        free(config);
        върни NULL;
    }
    //hexdump(config, cfg.wTotalLength);
    върни config;
}

static цяло
set_configuration(struct usb_pipe *pipe, u16 val)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.wValue = val;
    req.wIndex = 0;
    req.wLength = 0;
    върни usb_send_default_control(pipe, &req, NULL);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

static const цяло speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
static цяло
usb_set_address(struct usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    struct usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ако (cntl->maxaddr >= USB_MAXADDR)
        върни -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    struct usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ако (!usbdev->defpipe)
        върни -1;

    // Send set_address command.
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.wValue = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    цяло ret = usb_send_default_control(usbdev->defpipe, &req, NULL);
    ако (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        върни -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ако (!usbdev->defpipe)
        върни -1;
    върни 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
static цяло
configure_usb_device(struct usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    struct usb_device_descriptor dinfo;
    цяло ret = get_device_info8(usbdev->defpipe, &dinfo);
    ако (ret)
        върни 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ако (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ако (maxpacket < 8)
        върни 0;
    struct usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ако (!usbdev->defpipe)
        върни -1;

    // Get configuration
    struct usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ако (!config)
        върни 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    цяло num_iface = config->bNumInterfaces;
    void *config_end = (void*)config + config->wTotalLength;
    struct usb_interface_descriptor *iface = (void*)(&config[1]);
    за (;;) {
        ако (!num_iface || (void*)iface + iface->bLength > config_end)
            // Not a supported device.
            goto fail;
        ако (iface->bDescriptorType == USB_DT_INTERFACE) {
            num_iface--;
            ако (iface->bInterfaceClass == USB_CLASS_HUB
                || (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE
                    && (iface->bInterfaceProtocol == US_PR_BULK
                        || iface->bInterfaceProtocol == US_PR_UAS))
                || (iface->bInterfaceClass == USB_CLASS_HID
                    && iface->bInterfaceSubClass == USB_INTERFACE_SUBCLASS_BOOT))
                break;
        }
        iface = (void*)iface + iface->bLength;
    }

    // Set the configuration.
    ret = set_configuration(usbdev->defpipe, config->bConfigurationValue);
    ако (ret)
        goto fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (void*)config + config->wTotalLength - (void*)iface;
    ако (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    иначе ако (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ако (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        ако (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } иначе
        ret = usb_hid_setup(usbdev);
    ако (ret)
        goto fail;

    free(config);
    върни 1;
fail:
    free(config);
    върни 0;
}

static void
usb_hub_port_setup(void *data)
{
    struct usbdevice_s *usbdev = data;
    struct usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    за (;;) {
        // Detect if device present (and possibly start reset)
        цяло ret = hub->op->detect(hub, port);
        ако (ret > 0)
            // Device connected.
            break;
        ако (ret < 0 || timer_check(hub->detectend))
            // No device found.
            goto done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    цяло ret = hub->op->reset(hub, port);
    ако (ret < 0)
        // Reset failed
        goto resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    ако (ret) {
        hub->op->disconnect(hub, port);
        goto resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    цяло count = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ако (!count)
        hub->op->disconnect(hub, port);
    hub->devcount += count;
done:
    hub->threads--;
    free(usbdev);
    върни;

resetfail:
    mutex_unlock(&hub->cntl->resetlock);
    goto done;
}

static u32 usb_time_sigatt;

void
usb_enumerate(struct usbhub_s *hub)
{
    u32 portcount = hub->portcount;
    hub->threads = portcount;
    hub->detectend = timer_calc(usb_time_sigatt);

    // Launch a thread for every port.
    цяло i;
    за (i=0; i<portcount; i++) {
        struct usbdevice_s *usbdev = malloc_tmphigh(sizeof(*usbdev));
        ако (!usbdev) {
            warn_noalloc();
            continue;
        }
        memset(usbdev, 0, sizeof(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    докато (hub->threads)
        yield();
}

void
usb_setup(void)
{
    ASSERT32FLAT();
    ако (! CONFIG_USB)
        върни;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
