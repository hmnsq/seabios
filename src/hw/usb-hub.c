// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // CONFIG_USB_HUB
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "usb.h" // struct usb_s
#অন্তর্ভাৱ "usb-hub.h" // struct usb_hub_descriptor
#অন্তর্ভাৱ "util.h" // timer_calc

স্থির পূর্ণাঙ্ক
get_hub_desc(সংরচনা usb_pipe *pipe, সংরচনা usb_hub_descriptor *desc)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    যদি (pipe->speed == USB_SUPERSPEED)
        req.w_মান = USB_DT_HUB3<<8;
    অন্যথা
        req.w_মান = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = পরিমাণ(*desc);
    ঘূৰাই usb_send_default_control(pipe, &req, desc);
}

স্থির পূর্ণাঙ্ক
set_hub_depth(সংরচনা usb_pipe *pipe, u16 depth)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_মান = depth;
    req.wIndex = 0;
    req.wLength = 0;
    ঘূৰাই usb_send_default_control(pipe, &req, রিক্ত);
}

স্থির পূর্ণাঙ্ক
set_port_feature(সংরচনা usbhub_s *hub, পূর্ণাঙ্ক port, পূর্ণাঙ্ক feature)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_মান = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    পূর্ণাঙ্ক ret = usb_send_default_control(hub->usbdev->defpipe, &req, রিক্ত);
    mutex_unlock(&hub->lock);
    ঘূৰাই ret;
}

স্থির পূর্ণাঙ্ক
clear_port_feature(সংরচনা usbhub_s *hub, পূর্ণাঙ্ক port, পূর্ণাঙ্ক feature)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_মান = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    পূর্ণাঙ্ক ret = usb_send_default_control(hub->usbdev->defpipe, &req, রিক্ত);
    mutex_unlock(&hub->lock);
    ঘূৰাই ret;
}

স্থির পূর্ণাঙ্ক
get_port_status(সংরচনা usbhub_s *hub, পূর্ণাঙ্ক port, সংরচনা usb_port_status *sts)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_মান = 0;
    req.wIndex = port + 1;
    req.wLength = পরিমাণ(*sts);
    mutex_lock(&hub->lock);
    পূর্ণাঙ্ক ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    ঘূৰাই ret;
}

// Check if device attached to port
স্থির পূর্ণাঙ্ক
usb_hub_detect(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_port_status sts;
    পূর্ণাঙ্ক ret = get_port_status(hub, port, &sts);
    যদি (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        ঘূৰাই -1;
    }
    ঘূৰাই (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
স্থির শূন্য
usb_hub_disconnect(সংরচনা usbhub_s *hub, u32 port)
{
    পূর্ণাঙ্ক ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    যদি (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
স্থির পূর্ণাঙ্ক
usb_hub_reset(সংরচনা usbhub_s *hub, u32 port)
{
    পূর্ণাঙ্ক ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    যদি (ret)
        গচ্ছ fail;

    // Wait for reset to complete.
    সংরচনা usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    বাবে (;;) {
        ret = get_port_status(hub, port, &sts);
        যদি (ret)
            গচ্ছ fail;
        যদি (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            ভঙ্গ;
        যদি (timer_check(end)) {
            warn_timeout();
            গচ্ছ fail;
        }
        msleep(5);
    }

    // Reset complete.
    যদি (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        ঘূৰাই -1;

    যদি (hub->usbdev->speed == USB_SUPERSPEED)
        ঘূৰাই USB_SUPERSPEED;
    ঘূৰাই ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    ঘূৰাই -1;
}

স্থির সংরচনা usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
পূর্ণাঙ্ক
usb_hub_setup(সংরচনা usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    যদি (!CONFIG_USB_HUB)
        ঘূৰাই -1;

    সংরচনা usb_hub_descriptor desc;
    পূর্ণাঙ্ক ret = get_hub_desc(usbdev->defpipe, &desc);
    যদি (ret)
        ঘূৰাই ret;

    সংরচনা usbhub_s hub;
    memset(&hub, 0, পরিমাণ(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    যদি (usbdev->speed == USB_SUPERSPEED) {
        পূর্ণাঙ্ক depth = 0;
        সংরচনা usbdevice_s *parent = usbdev->hub->usbdev;
        যেতিয়ালৈ (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        যদি (ret)
            ঘূৰাই ret;
    }

    // Turn on power to ports.
    পূর্ণাঙ্ক port;
    বাবে (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        যদি (ret)
            ঘূৰাই ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    যদি (hub.devcount)
        ঘূৰাই 0;
    ঘূৰাই -1;
}
