// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_USB_HUB
#ضمن "output.h" // dprintf
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-hub.h" // struct usb_hub_descriptor
#ضمن "util.h" // timer_calc

ساكن صحيح
get_hub_desc(بنية usb_pipe *pipe, بنية usb_hub_descriptor *desc)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    إذا (pipe->speed == USB_SUPERSPEED)
        req.w_درج = USB_DT_HUB3<<8;
    وإلا
        req.w_درج = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = حجم(*desc);
    أرجع usb_send_default_control(pipe, &req, desc);
}

ساكن صحيح
set_hub_depth(بنية usb_pipe *pipe, u16 depth)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_درج = depth;
    req.wIndex = 0;
    req.wLength = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

ساكن صحيح
set_port_feature(بنية usbhub_s *hub, صحيح port, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_درج = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->lock);
    أرجع ret;
}

ساكن صحيح
clear_port_feature(بنية usbhub_s *hub, صحيح port, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_درج = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->lock);
    أرجع ret;
}

ساكن صحيح
get_port_status(بنية usbhub_s *hub, صحيح port, بنية usb_port_status *sts)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_درج = 0;
    req.wIndex = port + 1;
    req.wLength = حجم(*sts);
    mutex_lock(&hub->lock);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    أرجع ret;
}

// Check if device attached to port
ساكن صحيح
usb_hub_detect(بنية usbhub_s *hub, u32 port)
{
    بنية usb_port_status sts;
    صحيح ret = get_port_status(hub, port, &sts);
    إذا (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        أرجع -1;
    }
    أرجع (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ساكن فراغ
usb_hub_disconnect(بنية usbhub_s *hub, u32 port)
{
    صحيح ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    إذا (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
ساكن صحيح
usb_hub_reset(بنية usbhub_s *hub, u32 port)
{
    صحيح ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    إذا (ret)
        اذهب fail;

    // Wait for reset to complete.
    بنية usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    لكل (;;) {
        ret = get_port_status(hub, port, &sts);
        إذا (ret)
            اذهب fail;
        إذا (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            اكسر;
        إذا (timer_check(end)) {
            warn_timeout();
            اذهب fail;
        }
        msleep(5);
    }

    // Reset complete.
    إذا (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        أرجع -1;

    إذا (hub->usbdev->speed == USB_SUPERSPEED)
        أرجع USB_SUPERSPEED;
    أرجع ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    أرجع -1;
}

ساكن بنية usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
صحيح
usb_hub_setup(بنية usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    إذا (!CONFIG_USB_HUB)
        أرجع -1;

    بنية usb_hub_descriptor desc;
    صحيح ret = get_hub_desc(usbdev->defpipe, &desc);
    إذا (ret)
        أرجع ret;

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    إذا (usbdev->speed == USB_SUPERSPEED) {
        صحيح depth = 0;
        بنية usbdevice_s *parent = usbdev->hub->usbdev;
        طالما (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        إذا (ret)
            أرجع ret;
    }

    // Turn on power to ports.
    صحيح port;
    لكل (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        إذا (ret)
            أرجع ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    إذا (hub.devcount)
        أرجع 0;
    أرجع -1;
}
