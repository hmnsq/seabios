// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_USB_HUB
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-hub.h" // struct usb_hub_descriptor
#અન્તર્ભાવ "util.h" // timer_calc

સ્થિર પૂર્ણાંક
get_hub_desc(સંરચના usb_pipe *pipe, સંરચના usb_hub_descriptor *desc)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    જો (pipe->speed == USB_SUPERSPEED)
        req.w_કિંમત = USB_DT_HUB3<<8;
    નહીંતર
        req.w_કિંમત = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = કદ(*desc);
    પાછા usb_send_default_control(pipe, &req, desc);
}

સ્થિર પૂર્ણાંક
set_hub_depth(સંરચના usb_pipe *pipe, u16 depth)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_કિંમત = depth;
    req.wIndex = 0;
    req.wLength = 0;
    પાછા usb_send_default_control(pipe, &req, ખાલી);
}

સ્થિર પૂર્ણાંક
set_port_feature(સંરચના usbhub_s *hub, પૂર્ણાંક port, પૂર્ણાંક feature)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_કિંમત = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    પૂર્ણાંક ret = usb_send_default_control(hub->usbdev->defpipe, &req, ખાલી);
    mutex_unlock(&hub->lock);
    પાછા ret;
}

સ્થિર પૂર્ણાંક
clear_port_feature(સંરચના usbhub_s *hub, પૂર્ણાંક port, પૂર્ણાંક feature)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_કિંમત = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    પૂર્ણાંક ret = usb_send_default_control(hub->usbdev->defpipe, &req, ખાલી);
    mutex_unlock(&hub->lock);
    પાછા ret;
}

સ્થિર પૂર્ણાંક
get_port_status(સંરચના usbhub_s *hub, પૂર્ણાંક port, સંરચના usb_port_status *sts)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_કિંમત = 0;
    req.wIndex = port + 1;
    req.wLength = કદ(*sts);
    mutex_lock(&hub->lock);
    પૂર્ણાંક ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    પાછા ret;
}

// Check if device attached to port
સ્થિર પૂર્ણાંક
usb_hub_detect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_port_status sts;
    પૂર્ણાંક ret = get_port_status(hub, port, &sts);
    જો (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        પાછા -1;
    }
    પાછા (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
સ્થિર શૂન્ય
usb_hub_disconnect(સંરચના usbhub_s *hub, u32 port)
{
    પૂર્ણાંક ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    જો (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
સ્થિર પૂર્ણાંક
usb_hub_reset(સંરચના usbhub_s *hub, u32 port)
{
    પૂર્ણાંક ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    જો (ret)
        જાઓ fail;

    // Wait for reset to complete.
    સંરચના usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    માટે (;;) {
        ret = get_port_status(hub, port, &sts);
        જો (ret)
            જાઓ fail;
        જો (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            જાઓ fail;
        }
        msleep(5);
    }

    // Reset complete.
    જો (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        પાછા -1;

    જો (hub->usbdev->speed == USB_SUPERSPEED)
        પાછા USB_SUPERSPEED;
    પાછા ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    પાછા -1;
}

સ્થિર સંરચના usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
પૂર્ણાંક
usb_hub_setup(સંરચના usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    જો (!CONFIG_USB_HUB)
        પાછા -1;

    સંરચના usb_hub_descriptor desc;
    પૂર્ણાંક ret = get_hub_desc(usbdev->defpipe, &desc);
    જો (ret)
        પાછા ret;

    સંરચના usbhub_s hub;
    memset(&hub, 0, કદ(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    જો (usbdev->speed == USB_SUPERSPEED) {
        પૂર્ણાંક depth = 0;
        સંરચના usbdevice_s *parent = usbdev->hub->usbdev;
        જ્યારે (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        જો (ret)
            પાછા ret;
    }

    // Turn on power to ports.
    પૂર્ણાંક port;
    માટે (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        જો (ret)
            પાછા ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    જો (hub.devcount)
        પાછા 0;
    પાછા -1;
}
