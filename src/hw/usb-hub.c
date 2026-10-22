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

static ціле
get_hub_desc(struct usb_pipe *pipe, struct usb_hub_descriptor *desc)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    якщо (pipe->speed == USB_SUPERSPEED)
        req.wValue = USB_DT_HUB3<<8;
    інакше
        req.wValue = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = sizeof(*desc);
    повернути usb_send_default_control(pipe, &req, desc);
}

static ціле
set_hub_depth(struct usb_pipe *pipe, u16 depth)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.wValue = depth;
    req.wIndex = 0;
    req.wLength = 0;
    повернути usb_send_default_control(pipe, &req, NULL);
}

static ціле
set_port_feature(struct usbhub_s *hub, ціле port, ціле feature)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ціле ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    повернути ret;
}

static ціле
clear_port_feature(struct usbhub_s *hub, ціле port, ціле feature)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ціле ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    повернути ret;
}

static ціле
get_port_status(struct usbhub_s *hub, ціле port, struct usb_port_status *sts)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.wValue = 0;
    req.wIndex = port + 1;
    req.wLength = sizeof(*sts);
    mutex_lock(&hub->lock);
    ціле ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    повернути ret;
}

// Check if device attached to port
static ціле
usb_hub_detect(struct usbhub_s *hub, u32 port)
{
    struct usb_port_status sts;
    ціле ret = get_port_status(hub, port, &sts);
    якщо (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        повернути -1;
    }
    повернути (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
static void
usb_hub_disconnect(struct usbhub_s *hub, u32 port)
{
    ціле ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    якщо (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
static ціле
usb_hub_reset(struct usbhub_s *hub, u32 port)
{
    ціле ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    якщо (ret)
        goto fail;

    // Wait for reset to complete.
    struct usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    для (;;) {
        ret = get_port_status(hub, port, &sts);
        якщо (ret)
            goto fail;
        якщо (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            break;
        якщо (timer_check(end)) {
            warn_timeout();
            goto fail;
        }
        msleep(5);
    }

    // Reset complete.
    якщо (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        повернути -1;

    якщо (hub->usbdev->speed == USB_SUPERSPEED)
        повернути USB_SUPERSPEED;
    повернути ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    повернути -1;
}

static struct usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
ціле
usb_hub_setup(struct usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    якщо (!CONFIG_USB_HUB)
        повернути -1;

    struct usb_hub_descriptor desc;
    ціле ret = get_hub_desc(usbdev->defpipe, &desc);
    якщо (ret)
        повернути ret;

    struct usbhub_s hub;
    memset(&hub, 0, sizeof(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    якщо (usbdev->speed == USB_SUPERSPEED) {
        ціле depth = 0;
        struct usbdevice_s *parent = usbdev->hub->usbdev;
        поки (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        якщо (ret)
            повернути ret;
    }

    // Turn on power to ports.
    ціле port;
    для (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        якщо (ret)
            повернути ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    якщо (hub.devcount)
        повернути 0;
    повернути -1;
}
