// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_USB_HUB
#شامل "output.h" // dprintf
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-hub.h" // struct usb_hub_descriptor
#شامل "util.h" // timer_calc

ساکن صحیح
get_hub_desc(ساخت usb_pipe *pipe, ساخت usb_hub_descriptor *desc)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_اندر | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    اگر (pipe->رفتار == USB_SUPERSPEED)
        req.w_قدر = USB_DT_HUB3<<8;
    ورنہ
        req.w_قدر = USB_DT_HUB<<8;
    req.w_اشاریہ = 0;
    req.w_لمبائی = حجم(*desc);
    واپس usb_send_default_control(pipe, &req, desc);
}

ساکن صحیح
سیٹ_hub_گہرائی(ساخت usb_pipe *pipe, u16 گہرائی)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_برآمد | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_سیٹ_HUB_گہرائی;
    req.w_قدر = گہرائی;
    req.w_اشاریہ = 0;
    req.w_لمبائی = 0;
    واپس usb_send_default_control(pipe, &req, تہی);
}

ساکن صحیح
سیٹ_پورٹ_feature(ساخت usbhub_s *hub, صحیح پورٹ, صحیح feature)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_برآمد | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_سیٹ_FEATURE;
    req.w_قدر = feature;
    req.w_اشاریہ = پورٹ + 1;
    req.w_لمبائی = 0;
    mutex_تالا(&hub->تالا_2);
    صحیح ret = usb_send_default_control(hub->usbdev->defpipe, &req, تہی);
    mutex_unlock(&hub->تالا_2);
    واپس ret;
}

ساکن صحیح
clear_پورٹ_feature(ساخت usbhub_s *hub, صحیح پورٹ, صحیح feature)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_برآمد | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_قدر = feature;
    req.w_اشاریہ = پورٹ + 1;
    req.w_لمبائی = 0;
    mutex_تالا(&hub->تالا_2);
    صحیح ret = usb_send_default_control(hub->usbdev->defpipe, &req, تہی);
    mutex_unlock(&hub->تالا_2);
    واپس ret;
}

ساکن صحیح
get_پورٹ_حالت(ساخت usbhub_s *hub, صحیح پورٹ, ساخت usb_پورٹ_حالت *sts)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_اندر | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_حالت;
    req.w_قدر = 0;
    req.w_اشاریہ = پورٹ + 1;
    req.w_لمبائی = حجم(*sts);
    mutex_تالا(&hub->تالا_2);
    صحیح ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->تالا_2);
    واپس ret;
}

// Check if device attached to port
ساکن صحیح
usb_hub_detect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_پورٹ_حالت sts;
    صحیح ret = get_پورٹ_حالت(hub, پورٹ, &sts);
    اگر (ret) {
        dprintf(1, "Failure on hub port %d detect\n", پورٹ);
        واپس -1;
    }
    واپس (sts.w_پورٹ_حالت & USB_پورٹ_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ساکن خالی
usb_hub_disconnect(ساخت usbhub_s *hub, u32 پورٹ)
{
    صحیح ret = clear_پورٹ_feature(hub, پورٹ, USB_پورٹ_FEAT_ENABLE);
    اگر (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", پورٹ);
}

// Reset device on port
ساکن صحیح
usb_hub_reset(ساخت usbhub_s *hub, u32 پورٹ)
{
    صحیح ret = سیٹ_پورٹ_feature(hub, پورٹ, USB_پورٹ_FEAT_RESET);
    اگر (ret)
        جاؤ fail;

    // Wait for reset to complete.
    ساخت usb_پورٹ_حالت sts;
    u32 انتہا = timer_calc(USB_وقت_DRST * 2);
    برائے (;;) {
        ret = get_پورٹ_حالت(hub, پورٹ, &sts);
        اگر (ret)
            جاؤ fail;
        اگر (!(sts.w_پورٹ_حالت & USB_پورٹ_STAT_RESET)
            && (hub->usbdev->رفتار != USB_SUPERSPEED
                || !(sts.w_پورٹ_حالت & USB_پورٹ_STAT_LINK_نقاب)))
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            جاؤ fail;
        }
        msleep(5);
    }

    // Reset complete.
    اگر (!(sts.w_پورٹ_حالت & USB_پورٹ_STAT_CONNECTION))
        // Device no longer present
        واپس -1;

    اگر (hub->usbdev->رفتار == USB_SUPERSPEED)
        واپس USB_SUPERSPEED;
    واپس ((sts.w_پورٹ_حالت & USB_پورٹ_STAT_رفتار_نقاب)
            >> USB_پورٹ_STAT_رفتار_سرکاؤ);

fail:
    dprintf(1, "Failure on hub port %d reset\n", پورٹ);
    usb_hub_disconnect(hub, پورٹ);
    واپس -1;
}

ساکن ساخت usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
صحیح
usb_hub_تیاری(ساخت usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    اگر (!CONFIG_USB_HUB)
        واپس -1;

    ساخت usb_hub_descriptor desc;
    صحیح ret = get_hub_desc(usbdev->defpipe, &desc);
    اگر (ret)
        واپس ret;

    ساخت usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    اگر (usbdev->رفتار == USB_SUPERSPEED) {
        صحیح گہرائی = 0;
        ساخت usbdevice_s *والد = usbdev->hub->usbdev;
        جب (والد) {
            گہرائی++;
            والد = والد->hub->usbdev;
        }

        ret = سیٹ_hub_گہرائی(usbdev->defpipe, گہرائی);
        اگر (ret)
            واپس ret;
    }

    // Turn on power to ports.
    صحیح پورٹ;
    برائے (پورٹ=0; پورٹ<desc.bNbrPorts; پورٹ++) {
        ret = سیٹ_پورٹ_feature(&hub, پورٹ, USB_پورٹ_FEAT_POWER);
        اگر (ret)
            واپس ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    اگر (hub.devcount)
        واپس 0;
    واپس -1;
}
