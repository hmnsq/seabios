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
    req.bRequestType = USB_DIR_درون | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    إذا (pipe->سرعت == USB_SUPERSPEED)
        req.w_مقدار = USB_DT_HUB3<<8;
    وإلا
        req.w_مقدار = USB_DT_HUB<<8;
    req.w_شاخص = 0;
    req.w_درازی = حجم(*desc);
    أرجع usb_send_default_control(pipe, &req, desc);
}

ساكن صحيح
مجموعه_hub_عمق(بنية usb_pipe *pipe, u16 عمق)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بیرون | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_مجموعه_HUB_عمق;
    req.w_مقدار = عمق;
    req.w_شاخص = 0;
    req.w_درازی = 0;
    أرجع usb_send_default_control(pipe, &req, عدم);
}

ساكن صحيح
مجموعه_پورت_feature(بنية usbhub_s *hub, صحيح پورت, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بیرون | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_مجموعه_FEATURE;
    req.w_مقدار = feature;
    req.w_شاخص = پورت + 1;
    req.w_درازی = 0;
    mutex_قفل(&hub->قفل_2);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->قفل_2);
    أرجع ret;
}

ساكن صحيح
clear_پورت_feature(بنية usbhub_s *hub, صحيح پورت, صحيح feature)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_بیرون | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_مقدار = feature;
    req.w_شاخص = پورت + 1;
    req.w_درازی = 0;
    mutex_قفل(&hub->قفل_2);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, عدم);
    mutex_unlock(&hub->قفل_2);
    أرجع ret;
}

ساكن صحيح
get_پورت_وضعیت(بنية usbhub_s *hub, صحيح پورت, بنية usb_پورت_وضعیت *sts)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_درون | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_وضعیت;
    req.w_مقدار = 0;
    req.w_شاخص = پورت + 1;
    req.w_درازی = حجم(*sts);
    mutex_قفل(&hub->قفل_2);
    صحيح ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->قفل_2);
    أرجع ret;
}

// Check if device attached to port
ساكن صحيح
usb_hub_detect(بنية usbhub_s *hub, u32 پورت)
{
    بنية usb_پورت_وضعیت sts;
    صحيح ret = get_پورت_وضعیت(hub, پورت, &sts);
    إذا (ret) {
        dprintf(1, "Failure on hub port %d detect\n", پورت);
        أرجع -1;
    }
    أرجع (sts.w_پورت_وضعیت & USB_پورت_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ساكن فراغ
usb_hub_disconnect(بنية usbhub_s *hub, u32 پورت)
{
    صحيح ret = clear_پورت_feature(hub, پورت, USB_پورت_FEAT_ENABLE);
    إذا (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", پورت);
}

// Reset device on port
ساكن صحيح
usb_hub_reset(بنية usbhub_s *hub, u32 پورت)
{
    صحيح ret = مجموعه_پورت_feature(hub, پورت, USB_پورت_FEAT_RESET);
    إذا (ret)
        اذهب fail;

    // Wait for reset to complete.
    بنية usb_پورت_وضعیت sts;
    u32 انتها = timer_calc(USB_زمان_DRST * 2);
    لكل (;;) {
        ret = get_پورت_وضعیت(hub, پورت, &sts);
        إذا (ret)
            اذهب fail;
        إذا (!(sts.w_پورت_وضعیت & USB_پورت_STAT_RESET)
            && (hub->usbdev->سرعت != USB_SUPERSPEED
                || !(sts.w_پورت_وضعیت & USB_پورت_STAT_LINK_ماسک)))
            اكسر;
        إذا (timer_check(انتها)) {
            warn_timeout();
            اذهب fail;
        }
        msleep(5);
    }

    // Reset complete.
    إذا (!(sts.w_پورت_وضعیت & USB_پورت_STAT_CONNECTION))
        // Device no longer present
        أرجع -1;

    إذا (hub->usbdev->سرعت == USB_SUPERSPEED)
        أرجع USB_SUPERSPEED;
    أرجع ((sts.w_پورت_وضعیت & USB_پورت_STAT_سرعت_ماسک)
            >> USB_پورت_STAT_سرعت_لغزش);

fail:
    dprintf(1, "Failure on hub port %d reset\n", پورت);
    usb_hub_disconnect(hub, پورت);
    أرجع -1;
}

ساكن بنية usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
صحيح
usb_hub_راهاندازی(بنية usbdevice_s *usbdev)
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

    إذا (usbdev->سرعت == USB_SUPERSPEED) {
        صحيح عمق = 0;
        بنية usbdevice_s *والد = usbdev->hub->usbdev;
        طالما (والد) {
            عمق++;
            والد = والد->hub->usbdev;
        }

        ret = مجموعه_hub_عمق(usbdev->defpipe, عمق);
        إذا (ret)
            أرجع ret;
    }

    // Turn on power to ports.
    صحيح پورت;
    لكل (پورت=0; پورت<desc.bNbrPorts; پورت++) {
        ret = مجموعه_پورت_feature(&hub, پورت, USB_پورت_FEAT_POWER);
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
