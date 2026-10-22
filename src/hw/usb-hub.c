// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // CONFIG_USB_HUB
#include "output.h" // dprintf
#include "string.h" // memset
#include "usb.h" // struct usb_s
#include "usb-hub.h" // struct usb_hub_descriptor
#include "util.h" // timer_calc

static số_nguyên
get_hub_desc(struct usb_pipe *pipe, struct usb_hub_descriptor *desc)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    nếu (pipe->speed == USB_SUPERSPEED)
        req.wValue = USB_DT_HUB3<<8;
    không_thì
        req.wValue = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = sizeof(*desc);
    trả_về usb_send_default_control(pipe, &req, desc);
}

static số_nguyên
set_hub_depth(struct usb_pipe *pipe, u16 depth)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.wValue = depth;
    req.wIndex = 0;
    req.wLength = 0;
    trả_về usb_send_default_control(pipe, &req, NULL);
}

static số_nguyên
set_port_feature(struct usbhub_s *hub, số_nguyên port, số_nguyên feature)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    số_nguyên ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    trả_về ret;
}

static số_nguyên
clear_port_feature(struct usbhub_s *hub, số_nguyên port, số_nguyên feature)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    số_nguyên ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    trả_về ret;
}

static số_nguyên
get_port_status(struct usbhub_s *hub, số_nguyên port, struct usb_port_status *sts)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.wValue = 0;
    req.wIndex = port + 1;
    req.wLength = sizeof(*sts);
    mutex_lock(&hub->lock);
    số_nguyên ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    trả_về ret;
}

// Check if device attached to port
static số_nguyên
usb_hub_detect(struct usbhub_s *hub, u32 port)
{
    struct usb_port_status sts;
    số_nguyên ret = get_port_status(hub, port, &sts);
    nếu (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        trả_về -1;
    }
    trả_về (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
static void
usb_hub_disconnect(struct usbhub_s *hub, u32 port)
{
    số_nguyên ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    nếu (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
static số_nguyên
usb_hub_reset(struct usbhub_s *hub, u32 port)
{
    số_nguyên ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    nếu (ret)
        goto fail;

    // Wait for reset to complete.
    struct usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    cho (;;) {
        ret = get_port_status(hub, port, &sts);
        nếu (ret)
            goto fail;
        nếu (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            break;
        nếu (timer_check(end)) {
            warn_timeout();
            goto fail;
        }
        msleep(5);
    }

    // Reset complete.
    nếu (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        trả_về -1;

    nếu (hub->usbdev->speed == USB_SUPERSPEED)
        trả_về USB_SUPERSPEED;
    trả_về ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    trả_về -1;
}

static struct usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
số_nguyên
usb_hub_setup(struct usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    nếu (!CONFIG_USB_HUB)
        trả_về -1;

    struct usb_hub_descriptor desc;
    số_nguyên ret = get_hub_desc(usbdev->defpipe, &desc);
    nếu (ret)
        trả_về ret;

    struct usbhub_s hub;
    memset(&hub, 0, sizeof(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    nếu (usbdev->speed == USB_SUPERSPEED) {
        số_nguyên depth = 0;
        struct usbdevice_s *parent = usbdev->hub->usbdev;
        trong_khi (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        nếu (ret)
            trả_về ret;
    }

    // Turn on power to ports.
    số_nguyên port;
    cho (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        nếu (ret)
            trả_về ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    nếu (hub.devcount)
        trả_về 0;
    trả_về -1;
}
