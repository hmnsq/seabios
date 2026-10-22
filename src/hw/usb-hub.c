// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_USB_HUB
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-hub.h" // struct usb_hub_descriptor
#അന്തര്ഭാവ "util.h" // timer_calc

സ്ഥിതം പൂർണ്ണസംഖ്യ
get_hub_desc(ഘടന usb_pipe *pipe, ഘടന usb_hub_descriptor *desc)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    എങ്കിൽ (pipe->speed == USB_SUPERSPEED)
        req.w_മൂല്യം = USB_DT_HUB3<<8;
    അല്ലെങ്കിൽ
        req.w_മൂല്യം = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = വലുപ്പം(*desc);
    തിരികെ usb_send_default_control(pipe, &req, desc);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
set_hub_depth(ഘടന usb_pipe *pipe, u16 depth)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_മൂല്യം = depth;
    req.wIndex = 0;
    req.wLength = 0;
    തിരികെ usb_send_default_control(pipe, &req, ശൂന്യ);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
set_port_feature(ഘടന usbhub_s *hub, പൂർണ്ണസംഖ്യ port, പൂർണ്ണസംഖ്യ feature)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_മൂല്യം = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    പൂർണ്ണസംഖ്യ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ശൂന്യ);
    mutex_unlock(&hub->lock);
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
clear_port_feature(ഘടന usbhub_s *hub, പൂർണ്ണസംഖ്യ port, പൂർണ്ണസംഖ്യ feature)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_മൂല്യം = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    പൂർണ്ണസംഖ്യ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ശൂന്യ);
    mutex_unlock(&hub->lock);
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
get_port_status(ഘടന usbhub_s *hub, പൂർണ്ണസംഖ്യ port, ഘടന usb_port_status *sts)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_മൂല്യം = 0;
    req.wIndex = port + 1;
    req.wLength = വലുപ്പം(*sts);
    mutex_lock(&hub->lock);
    പൂർണ്ണസംഖ്യ ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    തിരികെ ret;
}

// Check if device attached to port
സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_hub_detect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_port_status sts;
    പൂർണ്ണസംഖ്യ ret = get_port_status(hub, port, &sts);
    എങ്കിൽ (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        തിരികെ -1;
    }
    തിരികെ (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
സ്ഥിതം ശൂന്യം
usb_hub_disconnect(ഘടന usbhub_s *hub, u32 port)
{
    പൂർണ്ണസംഖ്യ ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    എങ്കിൽ (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_hub_reset(ഘടന usbhub_s *hub, u32 port)
{
    പൂർണ്ണസംഖ്യ ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    എങ്കിൽ (ret)
        പോകുക fail;

    // Wait for reset to complete.
    ഘടന usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    വേണ്ടി (;;) {
        ret = get_port_status(hub, port, &sts);
        എങ്കിൽ (ret)
            പോകുക fail;
        എങ്കിൽ (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            പോകുക fail;
        }
        msleep(5);
    }

    // Reset complete.
    എങ്കിൽ (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        തിരികെ -1;

    എങ്കിൽ (hub->usbdev->speed == USB_SUPERSPEED)
        തിരികെ USB_SUPERSPEED;
    തിരികെ ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    തിരികെ -1;
}

സ്ഥിതം ഘടന usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
പൂർണ്ണസംഖ്യ
usb_hub_setup(ഘടന usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    എങ്കിൽ (!CONFIG_USB_HUB)
        തിരികെ -1;

    ഘടന usb_hub_descriptor desc;
    പൂർണ്ണസംഖ്യ ret = get_hub_desc(usbdev->defpipe, &desc);
    എങ്കിൽ (ret)
        തിരികെ ret;

    ഘടന usbhub_s hub;
    memset(&hub, 0, വലുപ്പം(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    എങ്കിൽ (usbdev->speed == USB_SUPERSPEED) {
        പൂർണ്ണസംഖ്യ depth = 0;
        ഘടന usbdevice_s *parent = usbdev->hub->usbdev;
        ആകുമ്പോൾ (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        എങ്കിൽ (ret)
            തിരികെ ret;
    }

    // Turn on power to ports.
    പൂർണ്ണസംഖ്യ port;
    വേണ്ടി (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        എങ്കിൽ (ret)
            തിരികെ ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    എങ്കിൽ (hub.devcount)
        തിരികെ 0;
    തിരികെ -1;
}
