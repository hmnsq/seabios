// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_USB_HUB
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-hub.h" // struct usb_hub_descriptor
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc

ਸਥਿਰ ਪੂਰਨ
get_hub_desc(ਬਣਤਰ usb_pipe *pipe, ਬਣਤਰ usb_hub_descriptor *desc)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਵਿੱਚ | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    ਜੇ (pipe->ਰਫਤਾਰ == USB_SUPERSPEED)
        req.w_ਮੁੱਲ = USB_DT_HUB3<<8;
    ਨਹੀਂਤਾਂ
        req.w_ਮੁੱਲ = USB_DT_HUB<<8;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = ਆਕਾਰ(*desc);
    ਵਾਪਸ usb_send_default_control(pipe, &req, desc);
}

ਸਥਿਰ ਪੂਰਨ
ਸੈੱਟ_hub_ਡੂੰਘਾਈ(ਬਣਤਰ usb_pipe *pipe, u16 ਡੂੰਘਾਈ)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਬਾਹਰਲਾ | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_ਸੈੱਟ_HUB_ਡੂੰਘਾਈ;
    req.w_ਮੁੱਲ = ਡੂੰਘਾਈ;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = 0;
    ਵਾਪਸ usb_send_default_control(pipe, &req, ਨਿਰਾ);
}

ਸਥਿਰ ਪੂਰਨ
ਸੈੱਟ_ਪੋਰਟ_feature(ਬਣਤਰ usbhub_s *hub, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ feature)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਬਾਹਰਲਾ | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_ਸੈੱਟ_FEATURE;
    req.w_ਮੁੱਲ = feature;
    req.w_ਸੂਚਕਾਂਕ = ਪੋਰਟ + 1;
    req.w_ਲੰਮਾਈ = 0;
    mutex_ਤਾਲਾ(&hub->ਤਾਲਾ);
    ਪੂਰਨ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ਨਿਰਾ);
    mutex_unlock(&hub->ਤਾਲਾ);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
clear_ਪੋਰਟ_feature(ਬਣਤਰ usbhub_s *hub, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ feature)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਬਾਹਰਲਾ | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_ਮੁੱਲ = feature;
    req.w_ਸੂਚਕਾਂਕ = ਪੋਰਟ + 1;
    req.w_ਲੰਮਾਈ = 0;
    mutex_ਤਾਲਾ(&hub->ਤਾਲਾ);
    ਪੂਰਨ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ਨਿਰਾ);
    mutex_unlock(&hub->ਤਾਲਾ);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
get_ਪੋਰਟ_ਸਥਿਤੀ(ਬਣਤਰ usbhub_s *hub, ਪੂਰਨ ਪੋਰਟ, ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ *sts)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਵਿੱਚ | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_ਸਥਿਤੀ;
    req.w_ਮੁੱਲ = 0;
    req.w_ਸੂਚਕਾਂਕ = ਪੋਰਟ + 1;
    req.w_ਲੰਮਾਈ = ਆਕਾਰ(*sts);
    mutex_ਤਾਲਾ(&hub->ਤਾਲਾ);
    ਪੂਰਨ ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->ਤਾਲਾ);
    ਵਾਪਸ ret;
}

// Check if device attached to port
ਸਥਿਰ ਪੂਰਨ
usb_hub_detect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ sts;
    ਪੂਰਨ ret = get_ਪੋਰਟ_ਸਥਿਤੀ(hub, ਪੋਰਟ, &sts);
    ਜੇ (ret) {
        dprintf(1, "Failure on hub port %d detect\n", ਪੋਰਟ);
        ਵਾਪਸ -1;
    }
    ਵਾਪਸ (sts.w_ਪੋਰਟ_ਸਥਿਤੀ & USB_ਪੋਰਟ_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ਸਥਿਰ ਖਾਲੀ
usb_hub_disconnect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਪੂਰਨ ret = clear_ਪੋਰਟ_feature(hub, ਪੋਰਟ, USB_ਪੋਰਟ_FEAT_ENABLE);
    ਜੇ (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", ਪੋਰਟ);
}

// Reset device on port
ਸਥਿਰ ਪੂਰਨ
usb_hub_reset(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਪੂਰਨ ret = ਸੈੱਟ_ਪੋਰਟ_feature(hub, ਪੋਰਟ, USB_ਪੋਰਟ_FEAT_RESET);
    ਜੇ (ret)
        ਜਾਓ fail;

    // Wait for reset to complete.
    ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ sts;
    u32 ਸਿਰਾ = timer_calc(USB_ਸਮਾਂ_DRST * 2);
    ਲਈ (;;) {
        ret = get_ਪੋਰਟ_ਸਥਿਤੀ(hub, ਪੋਰਟ, &sts);
        ਜੇ (ret)
            ਜਾਓ fail;
        ਜੇ (!(sts.w_ਪੋਰਟ_ਸਥਿਤੀ & USB_ਪੋਰਟ_STAT_RESET)
            && (hub->usbdev->ਰਫਤਾਰ != USB_SUPERSPEED
                || !(sts.w_ਪੋਰਟ_ਸਥਿਤੀ & USB_ਪੋਰਟ_STAT_LINK_ਨਕਾਬ)))
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਜਾਓ fail;
        }
        msleep(5);
    }

    // Reset complete.
    ਜੇ (!(sts.w_ਪੋਰਟ_ਸਥਿਤੀ & USB_ਪੋਰਟ_STAT_CONNECTION))
        // Device no longer present
        ਵਾਪਸ -1;

    ਜੇ (hub->usbdev->ਰਫਤਾਰ == USB_SUPERSPEED)
        ਵਾਪਸ USB_SUPERSPEED;
    ਵਾਪਸ ((sts.w_ਪੋਰਟ_ਸਥਿਤੀ & USB_ਪੋਰਟ_STAT_ਰਫਤਾਰ_ਨਕਾਬ)
            >> USB_ਪੋਰਟ_STAT_ਰਫਤਾਰ_ਖਿਸਕਾਓ);

fail:
    dprintf(1, "Failure on hub port %d reset\n", ਪੋਰਟ);
    usb_hub_disconnect(hub, ਪੋਰਟ);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਬਣਤਰ usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
ਪੂਰਨ
usb_hub_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_USB_HUB)
        ਵਾਪਸ -1;

    ਬਣਤਰ usb_hub_descriptor desc;
    ਪੂਰਨ ret = get_hub_desc(usbdev->defpipe, &desc);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਬਣਤਰ usbhub_s hub;
    memset(&hub, 0, ਆਕਾਰ(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    ਜੇ (usbdev->ਰਫਤਾਰ == USB_SUPERSPEED) {
        ਪੂਰਨ ਡੂੰਘਾਈ = 0;
        ਬਣਤਰ usbdevice_s *ਮਾਪਾ = usbdev->hub->usbdev;
        ਜਦੋਂ (ਮਾਪਾ) {
            ਡੂੰਘਾਈ++;
            ਮਾਪਾ = ਮਾਪਾ->hub->usbdev;
        }

        ret = ਸੈੱਟ_hub_ਡੂੰਘਾਈ(usbdev->defpipe, ਡੂੰਘਾਈ);
        ਜੇ (ret)
            ਵਾਪਸ ret;
    }

    // Turn on power to ports.
    ਪੂਰਨ ਪੋਰਟ;
    ਲਈ (ਪੋਰਟ=0; ਪੋਰਟ<desc.bNbrPorts; ਪੋਰਟ++) {
        ret = ਸੈੱਟ_ਪੋਰਟ_feature(&hub, ਪੋਰਟ, USB_ਪੋਰਟ_FEAT_POWER);
        ਜੇ (ret)
            ਵਾਪਸ ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    ਜੇ (hub.devcount)
        ਵਾਪਸ 0;
    ਵਾਪਸ -1;
}
