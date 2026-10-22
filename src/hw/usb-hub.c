// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_USB_HUB
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-hub.h" // struct usb_hub_descriptor
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_hub_desc(ସଂରଚନା usb_pipe *pipe, ସଂରଚନା usb_hub_descriptor *desc)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    ଯଦି (pipe->speed == USB_SUPERSPEED)
        req.w_ମୂଲ୍ୟ = USB_DT_HUB3<<8;
    ଅନ୍ୟଥା
        req.w_ମୂଲ୍ୟ = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = ଆକାର(*desc);
    ଫେରନ୍ତୁ usb_send_default_control(pipe, &req, desc);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
set_hub_depth(ସଂରଚନା usb_pipe *pipe, u16 depth)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_ମୂଲ୍ୟ = depth;
    req.wIndex = 0;
    req.wLength = 0;
    ଫେରନ୍ତୁ usb_send_default_control(pipe, &req, ଖାଲି);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
set_port_feature(ସଂରଚନା usbhub_s *hub, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ପୂର୍ଣ୍ଣସଂଖ୍ୟା feature)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_ମୂଲ୍ୟ = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_send_default_control(hub->usbdev->defpipe, &req, ଖାଲି);
    mutex_unlock(&hub->lock);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
clear_port_feature(ସଂରଚନା usbhub_s *hub, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ପୂର୍ଣ୍ଣସଂଖ୍ୟା feature)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_ମୂଲ୍ୟ = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_send_default_control(hub->usbdev->defpipe, &req, ଖାଲି);
    mutex_unlock(&hub->lock);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_port_status(ସଂରଚନା usbhub_s *hub, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ସଂରଚନା usb_port_status *sts)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_ମୂଲ୍ୟ = 0;
    req.wIndex = port + 1;
    req.wLength = ଆକାର(*sts);
    mutex_lock(&hub->lock);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    ଫେରନ୍ତୁ ret;
}

// Check if device attached to port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_hub_detect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_port_status sts;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = get_port_status(hub, port, &sts);
    ଯଦି (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ସ୍ଥିର ଶୂନ୍ୟ
usb_hub_disconnect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    ଯଦି (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_hub_reset(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    ଯଦି (ret)
        ଯାଅ fail;

    // Wait for reset to complete.
    ସଂରଚନା usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    ପାଇଁ (;;) {
        ret = get_port_status(hub, port, &sts);
        ଯଦି (ret)
            ଯାଅ fail;
        ଯଦି (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଯାଅ fail;
        }
        msleep(5);
    }

    // Reset complete.
    ଯଦି (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        ଫେରନ୍ତୁ -1;

    ଯଦି (hub->usbdev->speed == USB_SUPERSPEED)
        ଫେରନ୍ତୁ USB_SUPERSPEED;
    ଫେରନ୍ତୁ ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ସଂରଚନା usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_hub_setup(ସଂରଚନା usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_USB_HUB)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା usb_hub_descriptor desc;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = get_hub_desc(usbdev->defpipe, &desc);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ସଂରଚନା usbhub_s hub;
    memset(&hub, 0, ଆକାର(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    ଯଦି (usbdev->speed == USB_SUPERSPEED) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth = 0;
        ସଂରଚନା usbdevice_s *parent = usbdev->hub->usbdev;
        ଯେତେବେଳେ (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
    }

    // Turn on power to ports.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା port;
    ପାଇଁ (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    ଯଦି (hub.devcount)
        ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ -1;
}
