// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_USB_HUB
#समावेश "output.h" // dprintf
#समावेश "string.h" // memset
#समावेश "usb.h" // struct usb_s
#समावेश "usb-hub.h" // struct usb_hub_descriptor
#समावेश "util.h" // timer_calc

अटल पूर्णांक
get_hub_desc(काष्ठा usb_pipe *pipe, काष्ठा usb_hub_descriptor *desc)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    अगर (pipe->speed == USB_SUPERSPEED)
        req.w_मूल्य = USB_DT_HUB3<<8;
    अन्यथा
        req.w_मूल्य = USB_DT_HUB<<8;
    req.w_सूचकांक = 0;
    req.w_लंबाई = माप(*desc);
    वापस usb_send_default_control(pipe, &req, desc);
}

अटल पूर्णांक
set_hub_depth(काष्ठा usb_pipe *pipe, u16 depth)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_SET_HUB_DEPTH;
    req.w_मूल्य = depth;
    req.w_सूचकांक = 0;
    req.w_लंबाई = 0;
    वापस usb_send_default_control(pipe, &req, शून्य);
}

अटल पूर्णांक
set_port_feature(काष्ठा usbhub_s *hub, पूर्णांक port, पूर्णांक feature)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_SET_FEATURE;
    req.w_मूल्य = feature;
    req.w_सूचकांक = port + 1;
    req.w_लंबाई = 0;
    mutex_lock(&hub->lock);
    पूर्णांक ret = usb_send_default_control(hub->usbdev->defpipe, &req, शून्य);
    mutex_unlock(&hub->lock);
    वापस ret;
}

अटल पूर्णांक
clear_port_feature(काष्ठा usbhub_s *hub, पूर्णांक port, पूर्णांक feature)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_मूल्य = feature;
    req.w_सूचकांक = port + 1;
    req.w_लंबाई = 0;
    mutex_lock(&hub->lock);
    पूर्णांक ret = usb_send_default_control(hub->usbdev->defpipe, &req, शून्य);
    mutex_unlock(&hub->lock);
    वापस ret;
}

अटल पूर्णांक
get_port_status(काष्ठा usbhub_s *hub, पूर्णांक port, काष्ठा usb_port_status *sts)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_STATUS;
    req.w_मूल्य = 0;
    req.w_सूचकांक = port + 1;
    req.w_लंबाई = माप(*sts);
    mutex_lock(&hub->lock);
    पूर्णांक ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->lock);
    वापस ret;
}

// Check if device attached to port
अटल पूर्णांक
usb_hub_detect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_port_status sts;
    पूर्णांक ret = get_port_status(hub, port, &sts);
    अगर (ret) {
        dprintf(1, "Failure on hub port %d detect\n", port);
        वापस -1;
    }
    वापस (sts.wPortStatus & USB_PORT_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
अटल व्योम
usb_hub_disconnect(काष्ठा usbhub_s *hub, u32 port)
{
    पूर्णांक ret = clear_port_feature(hub, port, USB_PORT_FEAT_ENABLE);
    अगर (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", port);
}

// Reset device on port
अटल पूर्णांक
usb_hub_reset(काष्ठा usbhub_s *hub, u32 port)
{
    पूर्णांक ret = set_port_feature(hub, port, USB_PORT_FEAT_RESET);
    अगर (ret)
        जाओ fail;

    // Wait for reset to complete.
    काष्ठा usb_port_status sts;
    u32 end = timer_calc(USB_TIME_DRST * 2);
    क्रम (;;) {
        ret = get_port_status(hub, port, &sts);
        अगर (ret)
            जाओ fail;
        अगर (!(sts.wPortStatus & USB_PORT_STAT_RESET)
            && (hub->usbdev->speed != USB_SUPERSPEED
                || !(sts.wPortStatus & USB_PORT_STAT_LINK_MASK)))
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            जाओ fail;
        }
        msleep(5);
    }

    // Reset complete.
    अगर (!(sts.wPortStatus & USB_PORT_STAT_CONNECTION))
        // Device no longer present
        वापस -1;

    अगर (hub->usbdev->speed == USB_SUPERSPEED)
        वापस USB_SUPERSPEED;
    वापस ((sts.wPortStatus & USB_PORT_STAT_SPEED_MASK)
            >> USB_PORT_STAT_SPEED_SHIFT);

fail:
    dprintf(1, "Failure on hub port %d reset\n", port);
    usb_hub_disconnect(hub, port);
    वापस -1;
}

अटल काष्ठा usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
पूर्णांक
usb_hub_setup(काष्ठा usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    अगर (!CONFIG_USB_HUB)
        वापस -1;

    काष्ठा usb_hub_descriptor desc;
    पूर्णांक ret = get_hub_desc(usbdev->defpipe, &desc);
    अगर (ret)
        वापस ret;

    काष्ठा usbhub_s hub;
    स_रखो(&hub, 0, माप(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    अगर (usbdev->speed == USB_SUPERSPEED) {
        पूर्णांक depth = 0;
        काष्ठा usbdevice_s *parent = usbdev->hub->usbdev;
        जबतक (parent) {
            depth++;
            parent = parent->hub->usbdev;
        }

        ret = set_hub_depth(usbdev->defpipe, depth);
        अगर (ret)
            वापस ret;
    }

    // Turn on power to ports.
    पूर्णांक port;
    क्रम (port=0; port<desc.bNbrPorts; port++) {
        ret = set_port_feature(&hub, port, USB_PORT_FEAT_POWER);
        अगर (ret)
            वापस ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    अगर (hub.devcount)
        वापस 0;
    वापस -1;
}
