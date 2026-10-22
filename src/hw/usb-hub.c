// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_USB_HUB
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-hub.h" // struct usb_hub_descriptor
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
get_hub_desc(ಸಂರಚನಾ usb_pipe *pipe, ಸಂರಚನಾ usb_hub_descriptor *desc)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    ಯದಿ (pipe->speed == USB_SUPERSPEED)
        req.w_ಮೌಲ್ಯ = USB_DT_HUB3<<8;
    ಅನ್ಯಥಾ
        req.w_ಮೌಲ್ಯ = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = ಪರಿಮಾಣ(*desc);
    ಪ್ರತ್ಯಾಗಮ usb_send_default_control(pipe, &req, desc);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
set_hub_depth(ಸಂರಚನಾ usb_pipe *pipe, u16 depth)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_ಮೌಲ್ಯ = depth;
    req.wIndex = 0;
    req.wLength = 0;
    ಪ್ರತ್ಯಾಗಮ usb_send_default_control(pipe, &req, ರಿಕ್ತ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
set_port_feature(ಸಂರಚನಾ usbhub_s *hub, ಪೂರ್ಣಾಙ್ಕ port, ಪೂರ್ಣಾಙ್ಕ feature)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_ಮೌಲ್ಯ = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ಪೂರ್ಣಾಙ್ಕ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ರಿಕ್ತ);
    mutex_unlock(&hub->lock);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
clear_port_feature(ಸಂರಚನಾ usbhub_s *hub, ಪೂರ್ಣಾಙ್ಕ port, ಪೂರ್ಣಾಙ್ಕ feature)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_ಮೌಲ್ಯ = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    ಪೂರ್ಣಾಙ್ಕ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ರಿಕ್ತ);
    mutex_unlock(&hub->lock);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
get_port_status(ಸಂರಚನಾ usbhub_s *hub, ಪೂರ್ಣಾಙ್ಕ port, ಸಂರಚನಾ usb_port_status *sts)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_ಮೌಲ್ಯ = 0;
    req.wIndex = port + 1;
    req.wLength = ಪರಿಮಾಣ(*sts);
    mutex_lock(&hub->lock);
    ಪೂರ್ಣಾಙ್ಕ ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    ಪ್ರತ್ಯಾಗಮ ret;
}

// Check if device attached to port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_hub_detect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_port_status sts;
    ಪೂರ್ಣಾಙ್ಕ ret = get_port_status(hub, port, &sts);
    ಯದಿ (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
usb_hub_disconnect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಪೂರ್ಣಾಙ್ಕ ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    ಯದಿ (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_hub_reset(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಪೂರ್ಣಾಙ್ಕ ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;

    // Wait for reset to complete.
    ಸಂರಚನಾ usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    ಪ್ರತಿ (;;) {
        ret = get_port_status(hub, port, &sts);
        ಯದಿ (ret)
            ಗಚ್ಛ fail;
        ಯದಿ (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಗಚ್ಛ fail;
        }
        msleep(5);
    }

    // Reset complete.
    ಯದಿ (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        ಪ್ರತ್ಯಾಗಮ -1;

    ಯದಿ (hub->usbdev->speed == USB_SUPERSPEED)
        ಪ್ರತ್ಯಾಗಮ USB_SUPERSPEED;
    ಪ್ರತ್ಯಾಗಮ ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಸಂರಚನಾ usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
ಪೂರ್ಣಾಙ್ಕ
usb_hub_setup(ಸಂರಚನಾ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ಯದಿ (!CONFIG_USB_HUB)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ usb_hub_descriptor desc;
    ಪೂರ್ಣಾಙ್ಕ ret = get_hub_desc(usbdev->defpipe, &desc);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಸಂರಚನಾ usbhub_s hub;
    memset(&hub, 0, ಪರಿಮಾಣ(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    ಯದಿ (usbdev->speed == USB_SUPERSPEED) {
        ಪೂರ್ಣಾಙ್ಕ depth = 0;
        ಸಂರಚನಾ usbdevice_s *parent = usbdev->hub->usbdev;
        ಯಾವತ್ (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ ret;
    }

    // Turn on power to ports.
    ಪೂರ್ಣಾಙ್ಕ port;
    ಪ್ರತಿ (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    ಯದಿ (hub.devcount)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ -1;
}
