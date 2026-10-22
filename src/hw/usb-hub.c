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

static 정수
get_hub_desc(struct usb_pipe *pipe, struct usb_hub_descriptor *desc)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    만약 (pipe->speed == USB_SUPERSPEED)
        req.wValue = USB_DT_HUB3<<8;
    아니면
        req.wValue = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = sizeof(*desc);
    반환 usb_send_default_control(pipe, &req, desc);
}

static 정수
set_hub_depth(struct usb_pipe *pipe, u16 depth)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.wValue = depth;
    req.wIndex = 0;
    req.wLength = 0;
    반환 usb_send_default_control(pipe, &req, NULL);
}

static 정수
set_port_feature(struct usbhub_s *hub, 정수 port, 정수 feature)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    정수 ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    반환 ret;
}

static 정수
clear_port_feature(struct usbhub_s *hub, 정수 port, 정수 feature)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    정수 ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    반환 ret;
}

static 정수
get_port_status(struct usbhub_s *hub, 정수 port, struct usb_port_status *sts)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.wValue = 0;
    req.wIndex = port + 1;
    req.wLength = sizeof(*sts);
    mutex_lock(&hub->lock);
    정수 ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    반환 ret;
}

// Check if device attached to port
static 정수
usb_hub_detect(struct usbhub_s *hub, u32 port)
{
    struct usb_port_status sts;
    정수 ret = get_port_status(hub, port, &sts);
    만약 (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        반환 -1;
    }
    반환 (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
static void
usb_hub_disconnect(struct usbhub_s *hub, u32 port)
{
    정수 ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    만약 (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
static 정수
usb_hub_reset(struct usbhub_s *hub, u32 port)
{
    정수 ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    만약 (ret)
        goto fail;

    // Wait for reset to complete.
    struct usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    위해 (;;) {
        ret = get_port_status(hub, port, &sts);
        만약 (ret)
            goto fail;
        만약 (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            break;
        만약 (timer_check(end)) {
            warn_timeout();
            goto fail;
        }
        msleep(5);
    }

    // Reset complete.
    만약 (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        반환 -1;

    만약 (hub->usbdev->speed == USB_SUPERSPEED)
        반환 USB_SUPERSPEED;
    반환 ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    반환 -1;
}

static struct usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
정수
usb_hub_setup(struct usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    만약 (!CONFIG_USB_HUB)
        반환 -1;

    struct usb_hub_descriptor desc;
    정수 ret = get_hub_desc(usbdev->defpipe, &desc);
    만약 (ret)
        반환 ret;

    struct usbhub_s hub;
    memset(&hub, 0, sizeof(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    만약 (usbdev->speed == USB_SUPERSPEED) {
        정수 depth = 0;
        struct usbdevice_s *parent = usbdev->hub->usbdev;
        동안 (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        만약 (ret)
            반환 ret;
    }

    // Turn on power to ports.
    정수 port;
    위해 (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        만약 (ret)
            반환 ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    만약 (hub.devcount)
        반환 0;
    반환 -1;
}
