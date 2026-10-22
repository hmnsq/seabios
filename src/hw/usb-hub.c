// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_USB_HUB
#ضمن "output.h" // dprintf
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-hub.h" // struct usb_hub_descriptor
#ضمن "util.h" // timer_calc

ساكن صحيح
get_hub_desc(بنية usb_pipe *pipe, بنية usb_hub_descriptor *desc)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_دننه | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    إذا (pipe->چټکتیا == USB_SUPERSPEED)
        req.w_ارزښت = USB_DT_HUB3<<8;
    وإلا
        req.w_ارزښت = USB_DT_HUB<<8;
    req.w_شاخص = 0;
    req.w_اوږدتیا = حجم(*desc);
    أرجع usb_send_default_control(pipe, &req, desc);
}

ساكن صحيح
سېټ_hub_ژوروالی(بنية usb_pipe *pipe, u16 ژوروالی)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بهر | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_سېټ_HUB_ژوروالی;
    req.w_ارزښت = ژوروالی;
    req.w_شاخص = 0;
    req.w_اوږدتیا = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

ساكن صحيح
سېټ_پورټ_feature(بنية usbhub_s *hub, صحيح پورټ, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بهر | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_سېټ_FEATURE;
    req.w_ارزښت = feature;
    req.w_شاخص = پورټ + 1;
    req.w_اوږدتیا = 0;
    mutex_قلف(&hub->قلف);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->قلف);
    أرجع ret;
}

ساكن صحيح
clear_پورټ_feature(بنية usbhub_s *hub, صحيح پورټ, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بهر | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_ارزښت = feature;
    req.w_شاخص = پورټ + 1;
    req.w_اوږدتیا = 0;
    mutex_قلف(&hub->قلف);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->قلف);
    أرجع ret;
}

ساكن صحيح
get_پورټ_حالت(بنية usbhub_s *hub, صحيح پورټ, بنية usb_پورټ_حالت *sts)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_دننه | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_حالت;
    req.w_ارزښت = 0;
    req.w_شاخص = پورټ + 1;
    req.w_اوږدتیا = حجم(*sts);
    mutex_قلف(&hub->قلف);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->قلف);
    أرجع ret;
}

// Check if device attached to port
ساكن صحيح
usb_hub_detect(بنية usbhub_s *hub, u32 پورټ)
{
    بنية usb_پورټ_حالت sts;
    صحيح ret = get_پورټ_حالت(hub, پورټ, &sts);
    إذا (ret) {
        dprintf(1, "Failure on hub port %d detect\n", پورټ);
        أرجع -1;
    }
    أرجع (sts.w_پورټ_حالت & USB_پورټ_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ساكن فراغ
usb_hub_disconnect(بنية usbhub_s *hub, u32 پورټ)
{
    صحيح ret = clear_پورټ_feature(hub, پورټ, USB_پورټ_FEAT_ENABLE);
    إذا (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", پورټ);
}

// Reset device on port
ساكن صحيح
usb_hub_reset(بنية usbhub_s *hub, u32 پورټ)
{
    صحيح ret = سېټ_پورټ_feature(hub, پورټ, USB_پورټ_FEAT_RESET);
    إذا (ret)
        اذهب fail;

    // Wait for reset to complete.
    بنية usb_پورټ_حالت sts;
    u32 پای_ټکی = timer_calc(USB_وخت_DRST * 2);
    لكل (;;) {
        ret = get_پورټ_حالت(hub, پورټ, &sts);
        إذا (ret)
            اذهب fail;
        إذا (!(sts.w_پورټ_حالت & USB_پورټ_STAT_RESET)
            && (hub->usbdev->چټکتیا != USB_SUPERSPEED
                || !(sts.w_پورټ_حالت & USB_پورټ_STAT_LINK_ماسک)))
            اكسر;
        إذا (timer_check(پای_ټکی)) {
            warn_timeout();
            اذهب fail;
        }
        msleep(5);
    }

    // Reset complete.
    إذا (!(sts.w_پورټ_حالت & USB_پورټ_STAT_CONNECTION))
        // Device no longer present
        أرجع -1;

    إذا (hub->usbdev->چټکتیا == USB_SUPERSPEED)
        أرجع USB_SUPERSPEED;
    أرجع ((sts.w_پورټ_حالت & USB_پورټ_STAT_چټکتیا_ماسک)
            >> USB_پورټ_STAT_چټکتیا_ښويدنه);

fail:
    dprintf(1, "Failure on hub port %d reset\n", پورټ);
    usb_hub_disconnect(hub, پورټ);
    أرجع -1;
}

ساكن بنية usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
صحيح
usb_hub_تیاری(بنية usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    إذا (!CONFIG_USB_HUB)
        أرجع -1;

    بنية usb_hub_descriptor desc;
    صحيح ret = get_hub_desc(usbdev->defpipe, &desc);
    إذا (ret)
        أرجع ret;

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    إذا (usbdev->چټکتیا == USB_SUPERSPEED) {
        صحيح ژوروالی = 0;
        بنية usbdevice_s *پلار = usbdev->hub->usbdev;
        طالما (پلار) {
            ژوروالی++;
            پلار = پلار->hub->usbdev;
        }

        ret = سېټ_hub_ژوروالی(usbdev->defpipe, ژوروالی);
        إذا (ret)
            أرجع ret;
    }

    // Turn on power to ports.
    صحيح پورټ;
    لكل (پورټ=0; پورټ<desc.bNbrPorts; پورټ++) {
        ret = سېټ_پورټ_feature(&hub, پورټ, USB_پورټ_FEAT_POWER);
        إذا (ret)
            أرجع ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    إذا (hub.devcount)
        أرجع 0;
    أرجع -1;
}
