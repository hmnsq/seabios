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

static ကိန်းပြည့်
get_hub_desc(sa_nracanaa usb_pipe *pipe, sa_nracanaa usb_hub_descriptor *desc)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    အကယ်၍ (pipe->speed == USB_SUPERSPEED)
        req.wValue = USB_DT_HUB3<<8;
    မဟုတ်ပါက
        req.wValue = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = parimaa_na(*desc);
    ပြန်ပေး usb_send_default_control(pipe, &req, desc);
}

static ကိန်းပြည့်
set_hub_depth(sa_nracanaa usb_pipe *pipe, u16 depth)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.wValue = depth;
    req.wIndex = 0;
    req.wLength = 0;
    ပြန်ပေး usb_send_default_control(pipe, &req, NULL);
}

static ကိန်းပြည့်
set_port_feature(sa_nracanaa usbhub_s *hub, ကိန်းပြည့် port, ကိန်းပြည့် feature)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ကိန်းပြည့် ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    ပြန်ပေး ret;
}

static ကိန်းပြည့်
clear_port_feature(sa_nracanaa usbhub_s *hub, ကိန်းပြည့် port, ကိန်းပြည့် feature)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.wValue = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ကိန်းပြည့် ret = usb_send_default_control(hub->usbdev->defpipe, &req, NULL);
    mutex_unlock(&hub->lock);
    ပြန်ပေး ret;
}

static ကိန်းပြည့်
get_port_status(sa_nracanaa usbhub_s *hub, ကိန်းပြည့် port, sa_nracanaa usb_port_status *sts)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.wValue = 0;
    req.wIndex = port + 1;
    req.wLength = parimaa_na(*sts);
    mutex_lock(&hub->lock);
    ကိန်းပြည့် ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    ပြန်ပေး ret;
}

// Check if device attached to port
static ကိန်းပြည့်
usb_hub_detect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_port_status sts;
    ကိန်းပြည့် ret = get_port_status(hub, port, &sts);
    အကယ်၍ (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        ပြန်ပေး -1;
    }
    ပြန်ပေး (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
static void
usb_hub_disconnect(sa_nracanaa usbhub_s *hub, u32 port)
{
    ကိန်းပြည့် ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    အကယ်၍ (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
static ကိန်းပြည့်
usb_hub_reset(sa_nracanaa usbhub_s *hub, u32 port)
{
    ကိန်းပြည့် ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    အကယ်၍ (ret)
        goto fail;

    // Wait for reset to complete.
    sa_nracanaa usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    အတွက် (;;) {
        ret = get_port_status(hub, port, &sts);
        အကယ်၍ (ret)
            goto fail;
        အကယ်၍ (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            bha_nga;
        အကယ်၍ (timer_check(end)) {
            warn_timeout();
            goto fail;
        }
        msleep(5);
    }

    // Reset complete.
    အကယ်၍ (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        ပြန်ပေး -1;

    အကယ်၍ (hub->usbdev->speed == USB_SUPERSPEED)
        ပြန်ပေး USB_SUPERSPEED;
    ပြန်ပေး ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    ပြန်ပေး -1;
}

static sa_nracanaa usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
ကိန်းပြည့်
usb_hub_setup(sa_nracanaa usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    အကယ်၍ (!CONFIG_USB_HUB)
        ပြန်ပေး -1;

    sa_nracanaa usb_hub_descriptor desc;
    ကိန်းပြည့် ret = get_hub_desc(usbdev->defpipe, &desc);
    အကယ်၍ (ret)
        ပြန်ပေး ret;

    sa_nracanaa usbhub_s hub;
    memset(&hub, 0, parimaa_na(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    အကယ်၍ (usbdev->speed == USB_SUPERSPEED) {
        ကိန်းပြည့် depth = 0;
        sa_nracanaa usbdevice_s *parent = usbdev->hub->usbdev;
        နေစဉ် (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        အကယ်၍ (ret)
            ပြန်ပေး ret;
    }

    // Turn on power to ports.
    ကိန်းပြည့် port;
    အတွက် (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        အကယ်၍ (ret)
            ပြန်ပေး ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    အကယ်၍ (hub.devcount)
        ပြန်ပေး 0;
    ပြန်ပေး -1;
}
