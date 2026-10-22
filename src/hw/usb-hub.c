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
    req.bRequestType = USB_DIR_وچ | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    إذا (pipe->رفتار == USB_SUPERSPEED)
        req.w_قدر = USB_DT_HUB3<<8;
    وإلا
        req.w_قدر = USB_DT_HUB<<8;
    req.w_اشاریہ = 0;
    req.w_لمائی = حجم(*desc);
    أرجع usb_send_default_control(pipe, &req, desc);
}

ساكن صحيح
سیٹ_hub_ڈونگھائی(بنية usb_pipe *pipe, u16 ڈونگھائی)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_باہرلا | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_سیٹ_HUB_ڈونگھائی;
    req.w_قدر = ڈونگھائی;
    req.w_اشاریہ = 0;
    req.w_لمائی = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

ساكن صحيح
سیٹ_پورٹ_feature(بنية usbhub_s *hub, صحيح پورٹ, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_باہرلا | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_سیٹ_FEATURE;
    req.w_قدر = feature;
    req.w_اشاریہ = پورٹ + 1;
    req.w_لمائی = 0;
    mutex_جندرا(&hub->جندرا);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->جندرا);
    أرجع ret;
}

ساكن صحيح
clear_پورٹ_feature(بنية usbhub_s *hub, صحيح پورٹ, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_باہرلا | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_قدر = feature;
    req.w_اشاریہ = پورٹ + 1;
    req.w_لمائی = 0;
    mutex_جندرا(&hub->جندرا);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->جندرا);
    أرجع ret;
}

ساكن صحيح
get_پورٹ_حالت(بنية usbhub_s *hub, صحيح پورٹ, بنية usb_پورٹ_حالت *sts)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_وچ | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_حالت;
    req.w_قدر = 0;
    req.w_اشاریہ = پورٹ + 1;
    req.w_لمائی = حجم(*sts);
    mutex_جندرا(&hub->جندرا);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->جندرا);
    أرجع ret;
}

// Check if device attached to port
ساكن صحيح
usb_hub_detect(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_پورٹ_حالت sts;
    صحيح ret = get_پورٹ_حالت(hub, پورٹ, &sts);
    إذا (ret) {
        dprintf(1, "Failure on hub port %d detect\n", پورٹ);
        أرجع -1;
    }
    أرجع (sts.w_پورٹ_حالت & USB_پورٹ_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ساكن فراغ
usb_hub_disconnect(بنية usbhub_s *hub, u32 پورٹ)
{
    صحيح ret = clear_پورٹ_feature(hub, پورٹ, USB_پورٹ_FEAT_ENABLE);
    إذا (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", پورٹ);
}

// Reset device on port
ساكن صحيح
usb_hub_reset(بنية usbhub_s *hub, u32 پورٹ)
{
    صحيح ret = سیٹ_پورٹ_feature(hub, پورٹ, USB_پورٹ_FEAT_RESET);
    إذا (ret)
        اذهب fail;

    // Wait for reset to complete.
    بنية usb_پورٹ_حالت sts;
    u32 سرا = timer_calc(USB_ویلا_DRST * 2);
    لكل (;;) {
        ret = get_پورٹ_حالت(hub, پورٹ, &sts);
        إذا (ret)
            اذهب fail;
        إذا (!(sts.w_پورٹ_حالت & USB_پورٹ_STAT_RESET)
            && (hub->usbdev->رفتار != USB_SUPERSPEED
                || !(sts.w_پورٹ_حالت & USB_پورٹ_STAT_LINK_نقاب)))
            اكسر;
        إذا (timer_check(سرا)) {
            warn_timeout();
            اذهب fail;
        }
        msleep(5);
    }

    // Reset complete.
    إذا (!(sts.w_پورٹ_حالت & USB_پورٹ_STAT_CONNECTION))
        // Device no longer present
        أرجع -1;

    إذا (hub->usbdev->رفتار == USB_SUPERSPEED)
        أرجع USB_SUPERSPEED;
    أرجع ((sts.w_پورٹ_حالت & USB_پورٹ_STAT_رفتار_نقاب)
            >> USB_پورٹ_STAT_رفتار_کھسکاؤ);

fail:
    dprintf(1, "Failure on hub port %d reset\n", پورٹ);
    usb_hub_disconnect(hub, پورٹ);
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

    إذا (usbdev->رفتار == USB_SUPERSPEED) {
        صحيح ڈونگھائی = 0;
        بنية usbdevice_s *ماپا = usbdev->hub->usbdev;
        طالما (ماپا) {
            ڈونگھائی++;
            ماپا = ماپا->hub->usbdev;
        }

        ret = سیٹ_hub_ڈونگھائی(usbdev->defpipe, ڈونگھائی);
        إذا (ret)
            أرجع ret;
    }

    // Turn on power to ports.
    صحيح پورٹ;
    لكل (پورٹ=0; پورٹ<desc.bNbrPorts; پورٹ++) {
        ret = سیٹ_پورٹ_feature(&hub, پورٹ, USB_پورٹ_FEAT_POWER);
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
