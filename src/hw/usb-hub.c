// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_USB_HUB
#شامل "output.h" // dprintf
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-hub.h" // struct usb_hub_descriptor
#شامل "util.h" // timer_calc

ایستا صحیح
get_hub_desc(ساختار usb_pipe *pipe, ساختار usb_hub_descriptor *desc)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    اگر (pipe->speed == USB_SUPERSPEED)
        req.w_مقدار = USB_DT_HUB3<<8;
    وگرنه
        req.w_مقدار = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = اندازه(*desc);
    بازگشت usb_send_default_control(pipe, &req, desc);
}

ایستا صحیح
set_hub_depth(ساختار usb_pipe *pipe, u16 depth)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_مقدار = depth;
    req.wIndex = 0;
    req.wLength = 0;
    بازگشت usb_send_default_control(pipe, &req, پوچ);
}

ایستا صحیح
set_port_feature(ساختار usbhub_s *hub, صحیح port, صحیح feature)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_مقدار = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    صحیح ret = usb_send_default_control(hub->usbdev->defpipe, &req, پوچ);
    mutex_unlock(&hub->lock);
    بازگشت ret;
}

ایستا صحیح
clear_port_feature(ساختار usbhub_s *hub, صحیح port, صحیح feature)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_مقدار = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    صحیح ret = usb_send_default_control(hub->usbdev->defpipe, &req, پوچ);
    mutex_unlock(&hub->lock);
    بازگشت ret;
}

ایستا صحیح
get_port_status(ساختار usbhub_s *hub, صحیح port, ساختار usb_port_status *sts)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_مقدار = 0;
    req.wIndex = port + 1;
    req.wLength = اندازه(*sts);
    mutex_lock(&hub->lock);
    صحیح ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    بازگشت ret;
}

// Check if device attached to port
ایستا صحیح
usb_hub_detect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_port_status sts;
    صحیح ret = get_port_status(hub, port, &sts);
    اگر (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        بازگشت -1;
    }
    بازگشت (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ایستا تهی
usb_hub_disconnect(ساختار usbhub_s *hub, u32 port)
{
    صحیح ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    اگر (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
ایستا صحیح
usb_hub_reset(ساختار usbhub_s *hub, u32 port)
{
    صحیح ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    اگر (ret)
        برو fail;

    // Wait for reset to complete.
    ساختار usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    برای (;;) {
        ret = get_port_status(hub, port, &sts);
        اگر (ret)
            برو fail;
        اگر (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            برو fail;
        }
        msleep(5);
    }

    // Reset complete.
    اگر (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        بازگشت -1;

    اگر (hub->usbdev->speed == USB_SUPERSPEED)
        بازگشت USB_SUPERSPEED;
    بازگشت ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    بازگشت -1;
}

ایستا ساختار usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
صحیح
usb_hub_setup(ساختار usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    اگر (!CONFIG_USB_HUB)
        بازگشت -1;

    ساختار usb_hub_descriptor desc;
    صحیح ret = get_hub_desc(usbdev->defpipe, &desc);
    اگر (ret)
        بازگشت ret;

    ساختار usbhub_s hub;
    memset(&hub, 0, اندازه(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    اگر (usbdev->speed == USB_SUPERSPEED) {
        صحیح depth = 0;
        ساختار usbdevice_s *parent = usbdev->hub->usbdev;
        وقتی (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        اگر (ret)
            بازگشت ret;
    }

    // Turn on power to ports.
    صحیح port;
    برای (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        اگر (ret)
            بازگشت ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    اگر (hub.devcount)
        بازگشت 0;
    بازگشت -1;
}
