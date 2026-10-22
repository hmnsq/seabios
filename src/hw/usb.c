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
        ສົ່ງຄືນ uhci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_OHCI:
        ສົ່ງຄືນ ohci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_EHCI:
        ສົ່ງຄືນ ehci_realloc_pipe(usbdev, pipe, epdesc);
    pak_sa USB_TYPE_XHCI:
        ສົ່ງຄືນ xhci_realloc_pipe(usbdev, pipe, epdesc);
    }
}

// Send a message on a control pipe using the default control descriptor.
static ຈຳນວນເຕັມ
usb_send_pipe(sa_nracanaa usb_pipe *pipe_fl, ຈຳນວນເຕັມ dir, const void *cmd
              , void *data, ຈຳນວນເຕັມ datasize)
{
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    pak_sa USB_TYPE_UHCI:
        ສົ່ງຄືນ uhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_OHCI:
        ຖ້າ (MODESEGMENT)
            ສົ່ງຄືນ -1;
        ສົ່ງຄືນ ohci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_EHCI:
        ສົ່ງຄືນ ehci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    pak_sa USB_TYPE_XHCI:
        ຖ້າ (MODESEGMENT)
            ສົ່ງຄືນ -1;
        ສົ່ງຄືນ xhci_send_pipe(pipe_fl, dir, cmd, data, datasize);
    }
}

ຈຳນວນເຕັມ
usb_poll_intr(sa_nracanaa usb_pipe *pipe_fl, void *data)
{
    ASSERT16();
    switch (GET_LOWFLAT(pipe_fl->type)) {
    default:
    pak_sa USB_TYPE_UHCI:
        ສົ່ງຄືນ uhci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_OHCI:
        ສົ່ງຄືນ ohci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_EHCI:
        ສົ່ງຄືນ ehci_poll_intr(pipe_fl, data);
    pak_sa USB_TYPE_XHCI: ;
        ສົ່ງຄືນ call32_params(xhci_poll_intr, pipe_fl
                             , MAKE_FLATPTR(GET_SEG(SS), data), 0, -1);
    }
}

ຈຳນວນເຕັມ usb_32bit_pipe(sa_nracanaa usb_pipe *pipe_fl)
{
    ສົ່ງຄືນ (CONFIG_USB_XHCI && GET_LOWFLAT(pipe_fl->type) == USB_TYPE_XHCI)
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
    ສົ່ງຄືນ usb_realloc_pipe(usbdev, NULL, epdesc);
}

// Free an allocated control or bulk pipe.
void
usb_free_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *pipe)
{
    ຖ້າ (!pipe)
        ສົ່ງຄືນ;
    usb_realloc_pipe(usbdev, pipe, NULL);
}

// Send a message to the default control pipe of a device.
ຈຳນວນເຕັມ
usb_send_default_control(sa_nracanaa usb_pipe *pipe, const sa_nracanaa usb_ctrlrequest *req
                         , void *data)
{
    ສົ່ງຄືນ usb_send_pipe(pipe, req->bRequestType & USB_DIR_IN, req
                         , data, req->wLength);
}

// Send a message to a bulk endpoint
ຈຳນວນເຕັມ
usb_send_bulk(sa_nracanaa usb_pipe *pipe_fl, ຈຳນວນເຕັມ dir, void *data, ຈຳນວນເຕັມ datasize)
{
    ສົ່ງຄືນ usb_send_pipe(pipe_fl, dir, NULL, data, datasize);
}

// Check if a pipe for a given controller is on the freelist
ຈຳນວນເຕັມ
usb_is_freelist(sa_nracanaa usb_s *cntl, sa_nracanaa usb_pipe *pipe)
{
    ສົ່ງຄືນ pipe->cntl != cntl;
}

// Add a pipe to the controller's freelist
void
usb_add_freelist(sa_nracanaa usb_pipe *pipe)
{
    ຖ້າ (!pipe)
        ສົ່ງຄືນ;
    sa_nracanaa usb_s *cntl = pipe->cntl;
    pipe->freenext = cntl->freelist;
    cntl->freelist = pipe;
}

// Check for an available pipe on the freelist.
sa_nracanaa usb_pipe *
usb_get_freelist(sa_nracanaa usb_s *cntl, u8 eptype)
{
    sa_nracanaa usb_pipe **pfree = &cntl->freelist;
    ສຳລັບ (;;) {
        sa_nracanaa usb_pipe *pipe = *pfree;
        ຖ້າ (!pipe)
            ສົ່ງຄືນ NULL;
        ຖ້າ (pipe->eptype == eptype) {
            *pfree = pipe->freenext;
            ສົ່ງຄືນ pipe;
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
ຈຳນວນເຕັມ
usb_get_period(sa_nracanaa usbdevice_s *usbdev
               , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ຈຳນວນເຕັມ period = epdesc->bInterval;
    ຖ້າ (usbdev->speed != USB_HIGHSPEED)
        ສົ່ງຄືນ (period <= 0) ? 0 : __fls(period);
    ສົ່ງຄືນ (period <= 4) ? 0 : period - 4;
}

// Maximum time (in ms) a data transfer should take
ຈຳນວນເຕັມ
usb_xfer_time(sa_nracanaa usb_pipe *pipe, ຈຳນວນເຕັມ datalen)
{
    // Use the maximum command time (5 seconds), except for
    // set_address commands where we don't want to stall the boot if
    // the device doesn't actually exist.  Add 100ms to account for
    // any controller delays.
    ຖ້າ (!GET_LOWFLAT(pipe->devaddr))
        ສົ່ງຄືນ USB_TIME_STATUS + 100;
    ສົ່ງຄືນ USB_TIME_COMMAND + 100;
}

// Find the first endpoint of a given type in an interface description.
sa_nracanaa usb_endpoint_descriptor *
usb_find_desc(sa_nracanaa usbdevice_s *usbdev, ຈຳນວນເຕັມ type, ຈຳນວນເຕັມ dir)
{
    sa_nracanaa usb_endpoint_descriptor *epdesc = (void*)&usbdev->iface[1];
    ສຳລັບ (;;) {
        ຖ້າ ((void*)epdesc >= (void*)usbdev->iface + usbdev->imax
            || epdesc->bDescriptorType == USB_DT_INTERFACE) {
            ສົ່ງຄືນ NULL;
        }
        ຖ້າ (epdesc->bDescriptorType == USB_DT_ENDPOINT
            && (epdesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir
            && (epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == type)
            ສົ່ງຄືນ epdesc;
        epdesc = (void*)epdesc + epdesc->bLength;
    }
}

// Get the first 8 bytes of the device descriptor.
static ຈຳນວນເຕັມ
get_device_info8(sa_nracanaa usb_pipe *pipe, sa_nracanaa usb_device_descriptor *dinfo)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    req.wValue = USB_DT_DEVICE<<8;
    req.wIndex = 0;
    req.wLength = 8;
    ສົ່ງຄືນ usb_send_default_control(pipe, &req, dinfo);
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
    ຈຳນວນເຕັມ ret = usb_send_default_control(pipe, &req, &cfg);
    ຖ້າ (ret)
        ສົ່ງຄືນ NULL;

    sa_nracanaa usb_config_descriptor *config = malloc_tmphigh(cfg.wTotalLength);
    ຖ້າ (!config) {
        warn_noalloc();
        ສົ່ງຄືນ NULL;
    }
    req.wLength = cfg.wTotalLength;
    ret = usb_send_default_control(pipe, &req, config);
    ຖ້າ (ret || config->wTotalLength != cfg.wTotalLength) {
        free(config);
        ສົ່ງຄືນ NULL;
    }
    //hexdump(config, cfg.wTotalLength);
    ສົ່ງຄືນ config;
}

static ຈຳນວນເຕັມ
set_configuration(sa_nracanaa usb_pipe *pipe, u16 val)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_CONFIGURATION;
    req.wValue = val;
    req.wIndex = 0;
    req.wLength = 0;
    ສົ່ງຄືນ usb_send_default_control(pipe, &req, NULL);
}


/****************************************************************
 * Initialization and enumeration
 ****************************************************************/

static const ຈຳນວນເຕັມ speed_to_ctlsize[] = {
    [ USB_FULLSPEED  ] = 8,
    [ USB_LOWSPEED   ] = 8,
    [ USB_HIGHSPEED  ] = 64,
    [ USB_SUPERSPEED ] = 512,
};

// Assign an address to a device in the default state on the given
// controller.
static ຈຳນວນເຕັມ
usb_set_address(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    sa_nracanaa usb_s *cntl = usbdev->hub->cntl;
    dprintf(3, "set_address %p\n", cntl);
    ຖ້າ (cntl->maxaddr >= USB_MAXADDR)
        ສົ່ງຄືນ -1;

    msleep(USB_TIME_RSTRCY);

    // Create a pipe for the default address.
    sa_nracanaa usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = speed_to_ctlsize[usbdev->speed],
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_alloc_pipe(usbdev, &epdesc);
    ຖ້າ (!usbdev->defpipe)
        ສົ່ງຄືນ -1;

    // Send set_address command.
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_SET_ADDRESS;
    req.wValue = cntl->maxaddr + 1;
    req.wIndex = 0;
    req.wLength = 0;
    ຈຳນວນເຕັມ ret = usb_send_default_control(usbdev->defpipe, &req, NULL);
    ຖ້າ (ret) {
        usb_free_pipe(usbdev, usbdev->defpipe);
        ສົ່ງຄືນ -1;
    }

    msleep(USB_TIME_SETADDR_RECOVERY);

    cntl->maxaddr++;
    usbdev->devaddr = cntl->maxaddr;
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ຖ້າ (!usbdev->defpipe)
        ສົ່ງຄືນ -1;
    ສົ່ງຄືນ 0;
}

// Called for every found device - see if a driver is available for
// this device and do setup if so.
static ຈຳນວນເຕັມ
configure_usb_device(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    dprintf(3, "config_usb: %p\n", usbdev->defpipe);

    // Set the max packet size for endpoint 0 of this device.
    sa_nracanaa usb_device_descriptor dinfo;
    ຈຳນວນເຕັມ ret = get_device_info8(usbdev->defpipe, &dinfo);
    ຖ້າ (ret)
        ສົ່ງຄືນ 0;
    u16 maxpacket = dinfo.bMaxPacketSize0;
    ຖ້າ (dinfo.bcdUSB >= 0x0300)
        maxpacket = 1 << dinfo.bMaxPacketSize0;
    dprintf(3, "device rev=%04x cls=%02x sub=%02x proto=%02x size=%d\n"
            , dinfo.bcdUSB, dinfo.bDeviceClass, dinfo.bDeviceSubClass
            , dinfo.bDeviceProtocol, maxpacket);
    ຖ້າ (maxpacket < 8)
        ສົ່ງຄືນ 0;
    sa_nracanaa usb_endpoint_descriptor epdesc = {
        .wMaxPacketSize = maxpacket,
        .bmAttributes = USB_ENDPOINT_XFER_CONTROL,
    };
    usbdev->defpipe = usb_realloc_pipe(usbdev, usbdev->defpipe, &epdesc);
    ຖ້າ (!usbdev->defpipe)
        ສົ່ງຄືນ -1;

    // Get configuration
    sa_nracanaa usb_config_descriptor *config = get_device_config(usbdev->defpipe);
    ຖ້າ (!config)
        ສົ່ງຄືນ 0;

    // Determine if a driver exists for this device - only look at the
    // interfaces of the first configuration.
    ຈຳນວນເຕັມ num_iface = config->bNumInterfaces;
    void *config_end = (void*)config + config->wTotalLength;
    sa_nracanaa usb_interface_descriptor *iface = (void*)(&config[1]);
    ສຳລັບ (;;) {
        ຖ້າ (!num_iface || (void*)iface + iface->bLength > config_end)
            // Not a supported device.
            goto fail;
        ຖ້າ (iface->bDescriptorType == USB_DT_INTERFACE) {
            num_iface--;
            ຖ້າ (iface->bInterfaceClass == USB_CLASS_HUB
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
    ຖ້າ (ret)
        goto fail;

    // Configure driver.
    usbdev->config = config;
    usbdev->iface = iface;
    usbdev->imax = (void*)config + config->wTotalLength - (void*)iface;
    ຖ້າ (iface->bInterfaceClass == USB_CLASS_HUB)
        ret = usb_hub_setup(usbdev);
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE) {
        ຖ້າ (iface->bInterfaceProtocol == US_PR_BULK)
            ret = usb_msc_setup(usbdev);
        ຖ້າ (iface->bInterfaceProtocol == US_PR_UAS)
            ret = usb_uas_setup(usbdev);
    } ບໍ່ດັ່ງນັ້ນ
        ret = usb_hid_setup(usbdev);
    ຖ້າ (ret)
        goto fail;

    free(config);
    ສົ່ງຄືນ 1;
fail:
    free(config);
    ສົ່ງຄືນ 0;
}

static void
usb_hub_port_setup(void *data)
{
    sa_nracanaa usbdevice_s *usbdev = data;
    sa_nracanaa usbhub_s *hub = usbdev->hub;
    u32 port = usbdev->port;

    ສຳລັບ (;;) {
        // Detect if device present (and possibly start reset)
        ຈຳນວນເຕັມ ret = hub->op->detect(hub, port);
        ຖ້າ (ret > 0)
            // Device connected.
            bha_nga;
        ຖ້າ (ret < 0 || timer_check(hub->detectend))
            // No device found.
            goto done;
        msleep(5);
    }

    // XXX - wait USB_TIME_ATTDB time?

    // Reset port and determine device speed
    mutex_lock(&hub->cntl->resetlock);
    ຈຳນວນເຕັມ ret = hub->op->reset(hub, port);
    ຖ້າ (ret < 0)
        // Reset failed
        goto resetfail;
    usbdev->speed = ret;

    // Set address of port
    ret = usb_set_address(usbdev);
    ຖ້າ (ret) {
        hub->op->disconnect(hub, port);
        goto resetfail;
    }
    mutex_unlock(&hub->cntl->resetlock);

    // Configure the device
    ຈຳນວນເຕັມ count = configure_usb_device(usbdev);
    usb_free_pipe(usbdev, usbdev->defpipe);
    ຖ້າ (!count)
        hub->op->disconnect(hub, port);
    hub->devcount += count;
done:
    hub->threads--;
    free(usbdev);
    ສົ່ງຄືນ;

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
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<portcount; i++) {
        sa_nracanaa usbdevice_s *usbdev = malloc_tmphigh(parimaa_na(*usbdev));
        ຖ້າ (!usbdev) {
            warn_noalloc();
            continue;
        }
        memset(usbdev, 0, parimaa_na(*usbdev));
        usbdev->hub = hub;
        usbdev->port = i;
        run_thread(usb_hub_port_setup, usbdev);
    }

    // Wait for threads to complete.
    ໃນຂະນະທີ່ (hub->threads)
        yield();
}

void
usb_setup(void)
{
    ASSERT32FLAT();
    ຖ້າ (! CONFIG_USB)
        ສົ່ງຄືນ;
    dprintf(3, "init usb\n");
    usb_time_sigatt = romfile_loadint("etc/usb-time-sigatt", USB_TIME_SIGATT);
    xhci_setup();
    ehci_setup();
    uhci_setup();
    ohci_setup();
}
