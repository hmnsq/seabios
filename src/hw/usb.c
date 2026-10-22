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
        반환 uhci_realloc_pipe(usbdev, pipe, epdesc);
    case USB_TYPE_OHCI:
        반환 ohci_realloc_pipe(usbdev, pipe, epdesc);
    case USB_TYPE_EHCI:
        반환 ehci_realloc_pipe(usbdev, pipe, epdesc);
    case USB_TYPE_XHCI:
        반환 xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
static 정수
usb_send_pipe(struct usb_pipe *pipe_fl, 정수 dir, const void *cmd
              , void *data, 정수 datasize)
{
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    case USB_TYPE_UHCI:
        반환 uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    case USB_TYPE_OHCI:
        만약 (MODESEGMENT)
            반환 -1;
        반환 ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    case USB_TYPE_EHCI:
        반환 ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    case USB_TYPE_XHCI:
        만약 (MODESEGMENT)
            반환 -1;
        반환 xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

정수
usb_poll_intr(struct usb_pipe *pipe_fl, void *data)
{
    ASSERT16();
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    case USB_TYPE_UHCI:
        반환 uhci_poll_intr(pipe_fl, data);
    case USB_TYPE_OHCI:
        반환 ohci_poll_intr(pipe_fl, data);
    case USB_TYPE_EHCI:
        반환 ehci_poll_intr(pipe_fl, data);
    case USB_TYPE_XHCI: ;
        반환 call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

정수 usb_32bit_pipe(struct usb_pipe *pipe_fl)
{
    반환 (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
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
    반환 usb_realloc_pipe(usbdev, NULL, epdesc);
}

// Free an allocated control or bulk pipe.
void
usb_free_pipe(struct usbdevice_s *usbdev, struct usb_pipe *pipe)
{
    만약 (!pipe)
        반환;
    usb_realloc_pipe(usbdev, pipe, NULL);
}

// Send a message to the default control pipe of a device.
정수
usb_send_default_control(struct usb_pipe *pipe, const struct usb_ctrlrequest *req
                         , void *data)
{
    반환 usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
정수
usb_send_bulk(struct usb_pipe *pipe_fl, 정수 dir, void *data, 정수 datasize)
{
    반환 usb_send_pipe(pipe_fl, dir, NULL, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
정수
usb_is_freelist(struct usb_s *cntl, struct usb_pipe *pipe)
{
    반환 pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
void
usb_add_freelist(struct usb_pipe *pipe)
{
    만약 (!pipe)
        반환;
    struct usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
struct usb_pipe *
usb_get_freelist(struct usb_s *cntl, u8 eptype)
{
    struct usb_pipe **pfree = &cntl->freelist;
    위해 (;;) {
        struct usb_pipe *pipe = *pfree;
        만약 (!pipe)
            반환 NULL;
        만약 (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            반환 pipe;
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
정수
usb_get_period(struct usbdevice_s *usbdev
               , struct usb_endpoint_descriptor *epdesc)
{
    정수 period = epdesc->bInterval;
    만약 (usbdev->speed != USB_HIGHSPEED)
        반환 (period <= 0) ? 0 : __fls(period);
    반환 (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
정수
usb_xfer_time(struct usb_pipe *pipe, 정수 datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    만약 (!GET_LOWFLAT(pipe->devaddr))
        반환 USB_TIME_STATUS + 100;
    반환 USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
struct usb_endpoint_descriptor *
usb_find_desc(struct usbdevice_s *usbdev, 정수 type, 정수 dir)
{
    struct usb_endpoint_descriptor *epdesc = (void*)&usbdev->iface[1];
    위해 (;;) {
        만약 ((void*)epdesc >= (void*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            반환 NULL;
        }
        만약 (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            반환 epdesc;
        epdesc = (void*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
static 정수
get_device_info8(struct usb_pipe *pipe, struct usb_device_descriptor *dinfo)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    반환 usb_send_default_control(pipe, &req, dinfo);
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
    정수 ret = usb_send_default_control(pipe, &req, &cfg);
    만약 (ret)
        반환 NULL;

    struct usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    만약 (!config) {
        warn_noalloc();
        반환 NULL;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    만약 (ret || config->wTotalLength != cfg.wTotalLength) {
        free(config);
        반환 NULL;
    }
    //hexdump(config, cfg.wTotalLength);
    반환 config;
}

static 정수
set_configuration(struct usb_pipe *pipe, u16 val)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.wValue = val;
    req.wIndex = 0;
    req.wLength = 0;
    반환 usb_send_default_control(pipe, &req, NULL);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

static const 정수 speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
static 정수
usb_set_address(struct usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    struct usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    만약 (cntl->maxaddr >= USB_MAXADDR)
        반환 -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    struct usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    만약 (!usbdev->defpipe)
        반환 -1;

    // Send set_address command.
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.wValue = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    정수 ret = usb_send_default_control(usbdev->defpipe, &req, NULL);
    만약 (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        반환 -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    만약 (!usbdev->defpipe)
        반환 -1;
    반환 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
static 정수
configure_usb_device(struct usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    struct usb_device_descriptor dinfo;
    정수 ret = get_device_info8(usbdev->defpipe, &dinfo);
    만약 (ret)
        반환 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    만약 (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    만약 (maxpacket < 8)
        반환 0;
    struct usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    만약 (!usbdev->defpipe)
        반환 -1;

    // Get configuration
    struct usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    만약 (!config)
        반환 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    정수 num_iface = config->bNumInterfaces;
    void *config_end = (void*)config + config->wTotalLength;
    struct usb_interface_descriptor *iface = (void*)(&config[1]);
    위해 (;;) {
        만약 (!num_iface || (void*)iface + iface->bLength > config_end)
            // Not a supported device.
            goto fail;
        만약 (iface->bDescriptorType == USB_DT_INTERFACE) {
            num_iface--;
            만약 (iface->bInterfaceClass == USB_CLASS_HUB
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
    만약 (ret)
        goto fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (void*)config + config->wTotalLength - (void*)iface;
    만약 (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    아니면 만약 (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        만약 (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        만약 (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } 아니면
        ret = usb_hid_setup(usbdev);
    만약 (ret)
        goto fail;

    free(config);
    반환 1;
fail:
    free(config);
    반환 0;
}

static void
usb_hub_port_setup(void *data)
{
    struct usbdevice_s *usbdev = data;
    struct usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    위해 (;;) {
        // Detect if device present (and possibly start reset)
        정수 ret = hub->op->detect(hub, port);
        만약 (ret > 0)
            // Device connected.
            break;
        만약 (ret < 0 || timer_check(hub->detectend))
            // No device found.
            goto done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    정수 ret = hub->op->reset(hub, port);
    만약 (ret < 0)
        // Reset failed
        goto resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    만약 (ret) {
        hub->op->disconnect(hub, port);
        goto resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    정수 count = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    만약 (!count)
        hub->op->disconnect(hub, port);
    hub->devcount += count;
done:
    hub->threads--;
    free(usbdev);
    반환;

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
    정수 i;
    위해 (i=0; i<portcount; i++) {
        struct usbdevice_s *usbdev = malloc_tmphigh(sizeof(*usbdev));
        만약 (!usbdev) {
            warn_noalloc();
            continue;
        }
        memset(usbdev, 0, sizeof(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    동안 (hub->threads)
        yield();
}

void
usb_setup(void)
{
    ASSERT32FLAT();
    만약 (! CONFIG_USB)
        반환;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
