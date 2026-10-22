// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_USB_HUB
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-hub.h" // struct usb_hub_descriptor
#অন্তর্ভাব "util.h" // timer_calc

স্থির পূর্ণসংখ্যা
get_hub_desc(কাঠামো usb_pipe *pipe, কাঠামো usb_hub_descriptor *desc)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    যদি (pipe->speed == USB_SUPERSPEED)
        req.w_মান = USB_DT_HUB3<<8;
    অন্যথা
        req.w_মান = USB_DT_HUB<<8;
    req.w_সূচক = 0;
    req.w_দৈর্ঘ্য = আকার(*desc);
    ফেরত usb_send_default_control(pipe, &req, desc);
}

স্থির পূর্ণসংখ্যা
set_hub_depth(কাঠামো usb_pipe *pipe, u16 depth)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_মান = depth;
    req.w_সূচক = 0;
    req.w_দৈর্ঘ্য = 0;
    ফেরত usb_send_default_control(pipe, &req, ফাঁকা);
}

স্থির পূর্ণসংখ্যা
set_port_feature(কাঠামো usbhub_s *hub, পূর্ণসংখ্যা port, পূর্ণসংখ্যা feature)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_মান = feature;
    req.w_সূচক = port + 1;
    req.w_দৈর্ঘ্য = 0;
    mutex_lock(&hub->lock);
    পূর্ণসংখ্যা ret = usb_send_default_control(hub->usbdev->defpipe, &req, ফাঁকা);
    mutex_unlock(&hub->lock);
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
clear_port_feature(কাঠামো usbhub_s *hub, পূর্ণসংখ্যা port, পূর্ণসংখ্যা feature)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_মান = feature;
    req.w_সূচক = port + 1;
    req.w_দৈর্ঘ্য = 0;
    mutex_lock(&hub->lock);
    পূর্ণসংখ্যা ret = usb_send_default_control(hub->usbdev->defpipe, &req, ফাঁকা);
    mutex_unlock(&hub->lock);
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা
get_port_status(কাঠামো usbhub_s *hub, পূর্ণসংখ্যা port, কাঠামো usb_port_status *sts)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_মান = 0;
    req.w_সূচক = port + 1;
    req.w_দৈর্ঘ্য = আকার(*sts);
    mutex_lock(&hub->lock);
    পূর্ণসংখ্যা ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    ফেরত ret;
}

// Check if device attached to port
স্থির পূর্ণসংখ্যা
usb_hub_detect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_port_status sts;
    পূর্ণসংখ্যা ret = get_port_status(hub, port, &sts);
    যদি (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        ফেরত -1;
    }
    ফেরত (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
স্থির শূন্য
usb_hub_disconnect(কাঠামো usbhub_s *hub, u32 port)
{
    পূর্ণসংখ্যা ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    যদি (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
স্থির পূর্ণসংখ্যা
usb_hub_reset(কাঠামো usbhub_s *hub, u32 port)
{
    পূর্ণসংখ্যা ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    যদি (ret)
        যাও fail;

    // Wait for reset to complete.
    কাঠামো usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    জন্য (;;) {
        ret = get_port_status(hub, port, &sts);
        যদি (ret)
            যাও fail;
        যদি (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            যাও fail;
        }
        msleep(5);
    }

    // Reset complete.
    যদি (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        ফেরত -1;

    যদি (hub->usbdev->speed == USB_SUPERSPEED)
        ফেরত USB_SUPERSPEED;
    ফেরত ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    ফেরত -1;
}

স্থির কাঠামো usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
পূর্ণসংখ্যা
usb_hub_setup(কাঠামো usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    যদি (!CONFIG_USB_HUB)
        ফেরত -1;

    কাঠামো usb_hub_descriptor desc;
    পূর্ণসংখ্যা ret = get_hub_desc(usbdev->defpipe, &desc);
    যদি (ret)
        ফেরত ret;

    কাঠামো usbhub_s hub;
    memset(&hub, 0, আকার(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    যদি (usbdev->speed == USB_SUPERSPEED) {
        পূর্ণসংখ্যা depth = 0;
        কাঠামো usbdevice_s *parent = usbdev->hub->usbdev;
        যখন (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        যদি (ret)
            ফেরত ret;
    }

    // Turn on power to ports.
    পূর্ণসংখ্যা port;
    জন্য (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        যদি (ret)
            ফেরত ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    যদি (hub.devcount)
        ফেরত 0;
    ফেরত -1;
}
