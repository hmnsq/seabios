// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_USB_HUB
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-hub.h" // struct usb_hub_descriptor
#अन्तर्भाव "util.h" // timer_calc

स्थिर पूर्णांक
get_hub_desc(संरचना usb_pipe *pipe, संरचना usb_hub_descriptor *desc)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    जर (pipe->speed == USB_SUPERSPEED)
        req.w_मूल्य = USB_DT_HUB3<<8;
    अन्यथा
        req.w_मूल्य = USB_DT_HUB<<8;
    req.wIndex = 0;
    req.wLength = आकार(*desc);
    परत usb_send_default_control(pipe, &req, desc);
}

स्थिर पूर्णांक
set_hub_depth(संरचना usb_pipe *pipe, u16 depth)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_मूल्य = depth;
    req.wIndex = 0;
    req.wLength = 0;
    परत usb_send_default_control(pipe, &req, रिक्त);
}

स्थिर पूर्णांक
set_port_feature(संरचना usbhub_s *hub, पूर्णांक port, पूर्णांक feature)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_मूल्य = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    पूर्णांक ret = usb_send_default_control(hub->usbdev->defpipe, &req, रिक्त);
    mutex_unlock(&hub->lock);
    परत ret;
}

स्थिर पूर्णांक
clear_port_feature(संरचना usbhub_s *hub, पूर्णांक port, पूर्णांक feature)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_मूल्य = feature;
    req.wIndex = port + 1;
    req.wLength = 0;
    mutex_lock(&hub->lock);
    पूर्णांक ret = usb_send_default_control(hub->usbdev->defpipe, &req, रिक्त);
    mutex_unlock(&hub->lock);
    परत ret;
}

स्थिर पूर्णांक
get_port_status(संरचना usbhub_s *hub, पूर्णांक port, संरचना usb_port_status *sts)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_मूल्य = 0;
    req.wIndex = port + 1;
    req.wLength = आकार(*sts);
    mutex_lock(&hub->lock);
    पूर्णांक ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    परत ret;
}

// Check if device attached to port
स्थिर पूर्णांक
usb_hub_detect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_port_status sts;
    पूर्णांक ret = get_port_status(hub, port, &sts);
    जर (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        परत -1;
    }
    परत (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
स्थिर शून्य
usb_hub_disconnect(संरचना usbhub_s *hub, u32 port)
{
    पूर्णांक ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    जर (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
स्थिर पूर्णांक
usb_hub_reset(संरचना usbhub_s *hub, u32 port)
{
    पूर्णांक ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    जर (ret)
        जा fail;

    // Wait for reset to complete.
    संरचना usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    साठी (;;) {
        ret = get_port_status(hub, port, &sts);
        जर (ret)
            जा fail;
        जर (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            थांबा;
        जर (timer_check(end)) {
            warn_timeout();
            जा fail;
        }
        msleep(5);
    }

    // Reset complete.
    जर (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        परत -1;

    जर (hub->usbdev->speed == USB_SUPERSPEED)
        परत USB_SUPERSPEED;
    परत ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    परत -1;
}

स्थिर संरचना usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
पूर्णांक
usb_hub_setup(संरचना usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    जर (!CONFIG_USB_HUB)
        परत -1;

    संरचना usb_hub_descriptor desc;
    पूर्णांक ret = get_hub_desc(usbdev->defpipe, &desc);
    जर (ret)
        परत ret;

    संरचना usbhub_s hub;
    स_रखो(&hub, 0, आकार(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    जर (usbdev->speed == USB_SUPERSPEED) {
        पूर्णांक depth = 0;
        संरचना usbdevice_s *parent = usbdev->hub->usbdev;
        पर्यंत (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        जर (ret)
            परत ret;
    }

    // Turn on power to ports.
    पूर्णांक port;
    साठी (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        जर (ret)
            परत ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    जर (hub.devcount)
        परत 0;
    परत -1;
}
