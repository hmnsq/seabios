// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_USB_HUB
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-hub.h" // struct usb_hub_descriptor
#அந்தர்பாவ "util.h" // timer_calc

நிலையான முழு
get_hub_desc(கட்டமைப்பு usb_pipe *pipe, கட்டமைப்பு usb_hub_descriptor *desc)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    எனில் (pipe->speed == USB_SUPERSPEED)
        req.w_மதிப்பு = USB_DT_HUB3<<8;
    இல்லையெனில்
        req.w_மதிப்பு = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = அளவு(*desc);
    திரும்பு usb_send_default_control(pipe, &req, desc);
}

நிலையான முழு
set_hub_depth(கட்டமைப்பு usb_pipe *pipe, u16 depth)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_மதிப்பு = depth;
    req.wIndex = 0;
    req.wLength = 0;
    திரும்பு usb_send_default_control(pipe, &req, வெற்று);
}

நிலையான முழு
set_port_feature(கட்டமைப்பு usbhub_s *hub, முழு port, முழு feature)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_மதிப்பு = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    முழு ret = usb_send_default_control(hub->usbdev->defpipe, &req, வெற்று);
    mutex_unlock(&hub->lock);
    திரும்பு ret;
}

நிலையான முழு
clear_port_feature(கட்டமைப்பு usbhub_s *hub, முழு port, முழு feature)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_மதிப்பு = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    முழு ret = usb_send_default_control(hub->usbdev->defpipe, &req, வெற்று);
    mutex_unlock(&hub->lock);
    திரும்பு ret;
}

நிலையான முழு
get_port_status(கட்டமைப்பு usbhub_s *hub, முழு port, கட்டமைப்பு usb_port_status *sts)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_மதிப்பு = 0;
    req.wIndex = port + 1;
    req.wLength = அளவு(*sts);
    mutex_lock(&hub->lock);
    முழு ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    திரும்பு ret;
}

// Check if device attached to port
நிலையான முழு
usb_hub_detect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_port_status sts;
    முழு ret = get_port_status(hub, port, &sts);
    எனில் (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        திரும்பு -1;
    }
    திரும்பு (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
நிலையான வெறுமை
usb_hub_disconnect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    முழு ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    எனில் (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
நிலையான முழு
usb_hub_reset(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    முழு ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    எனில் (ret)
        செல் fail;

    // Wait for reset to complete.
    கட்டமைப்பு usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    க்கு (;;) {
        ret = get_port_status(hub, port, &sts);
        எனில் (ret)
            செல் fail;
        எனில் (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            செல் fail;
        }
        msleep(5);
    }

    // Reset complete.
    எனில் (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        திரும்பு -1;

    எனில் (hub->usbdev->speed == USB_SUPERSPEED)
        திரும்பு USB_SUPERSPEED;
    திரும்பு ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    திரும்பு -1;
}

நிலையான கட்டமைப்பு usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
முழு
usb_hub_setup(கட்டமைப்பு usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    எனில் (!CONFIG_USB_HUB)
        திரும்பு -1;

    கட்டமைப்பு usb_hub_descriptor desc;
    முழு ret = get_hub_desc(usbdev->defpipe, &desc);
    எனில் (ret)
        திரும்பு ret;

    கட்டமைப்பு usbhub_s hub;
    memset(&hub, 0, அளவு(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    எனில் (usbdev->speed == USB_SUPERSPEED) {
        முழு depth = 0;
        கட்டமைப்பு usbdevice_s *parent = usbdev->hub->usbdev;
        போது (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        எனில் (ret)
            திரும்பு ret;
    }

    // Turn on power to ports.
    முழு port;
    க்கு (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        எனில் (ret)
            திரும்பு ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    எனில் (hub.devcount)
        திரும்பு 0;
    திரும்பு -1;
}
