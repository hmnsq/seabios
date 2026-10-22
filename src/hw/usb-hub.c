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

static පූර්ණ_සංඛ්‍යා
get_hub_desc(sa_nracanaa usb_pipe *pipe, sa_nracanaa usb_hub_descriptor *desc)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    නම් (pipe->speed == USB_SUPERSPEED)
        req.wValue = USB_DT_HUB3<<8;
    නැතහොත්
        req.wValue = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = parimaa_na(*desc);
    ආපසු usb_send_default_control(pipe, &req, desc);
}

static පූර්ණ_සංඛ්‍යා
set_hub_depth(sa_nracanaa usb_pipe *pipe, u16 depth)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.wValue = depth;
    req.wIndex = 0;
    req.wLength = 0;
    ආපසු usb_send_default_control(pipe, &req, NULL);
}

static පූර්ණ_සංඛ්‍යා
set_port_feature(sa_nracanaa usbhub_s *hub, පූර්ණ_සංඛ්‍යා port, පූර්ණ_සංඛ්‍යා feature)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    පූර්ණ_සංඛ්‍යා ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා
clear_port_feature(sa_nracanaa usbhub_s *hub, පූර්ණ_සංඛ්‍යා port, පූර්ණ_සංඛ්‍යා feature)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    පූර්ණ_සංඛ්‍යා ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා
get_port_status(sa_nracanaa usbhub_s *hub, පූර්ණ_සංඛ්‍යා port, sa_nracanaa usb_port_status *sts)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.wValue = 0;
    req.wIndex = port + 1;
    req.wLength = parimaa_na(*sts);
    mutex_lock(&hub->lock);
    පූර්ණ_සංඛ්‍යා ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    ආපසු ret;
}

// Check if device attached to port
static පූර්ණ_සංඛ්‍යා
usb_hub_detect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_port_status sts;
    පූර්ණ_සංඛ්‍යා ret = get_port_status(hub, port, &sts);
    නම් (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        ආපසු -1;
    }
    ආපසු (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
static void
usb_hub_disconnect(sa_nracanaa usbhub_s *hub, u32 port)
{
    පූර්ණ_සංඛ්‍යා ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    නම් (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
static පූර්ණ_සංඛ්‍යා
usb_hub_reset(sa_nracanaa usbhub_s *hub, u32 port)
{
    පූර්ණ_සංඛ්‍යා ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    නම් (ret)
        goto fail;

    // Wait for reset to complete.
    sa_nracanaa usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    සඳහා (;;) {
        ret = get_port_status(hub, port, &sts);
        නම් (ret)
            goto fail;
        නම් (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            bha_nga;
        නම් (timer_check(end)) {
            warn_timeout();
            goto fail;
        }
        msleep(5);
    }

    // Reset complete.
    නම් (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        ආපසු -1;

    නම් (hub->usbdev->speed == USB_SUPERSPEED)
        ආපසු USB_SUPERSPEED;
    ආපසු ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    ආපසු -1;
}

static sa_nracanaa usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
පූර්ණ_සංඛ්‍යා
usb_hub_setup(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    නම් (!CONFIG_USB_HUB)
        ආපසු -1;

    sa_nracanaa usb_hub_descriptor desc;
    පූර්ණ_සංඛ්‍යා ret = get_hub_desc(usbdev->defpipe, &desc);
    නම් (ret)
        ආපසු ret;

    sa_nracanaa usbhub_s hub;
    memset(&hub, 0, parimaa_na(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    නම් (usbdev->speed == USB_SUPERSPEED) {
        පූර්ණ_සංඛ්‍යා depth = 0;
        sa_nracanaa usbdevice_s *parent = usbdev->hub->usbdev;
        අතරතුර (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        නම් (ret)
            ආපසු ret;
    }

    // Turn on power to ports.
    පූර්ණ_සංඛ්‍යා port;
    සඳහා (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        නම් (ret)
            ආපසු ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    නම් (hub.devcount)
        ආපසු 0;
    ආපසු -1;
}
