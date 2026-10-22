// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_USB_HUB
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-hub.h" // struct usb_hub_descriptor
#అన్తర్భావ "util.h" // timer_calc

స్థిర పూర్ణాంకం
get_hub_desc(నిర్మాణం usb_pipe *pipe, నిర్మాణం usb_hub_descriptor *desc)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_లోపల | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    ఒకవేళ (pipe->వేగం == USB_SUPERSPEED)
        req.w_విలువ = USB_DT_HUB3<<8;
    లేకపోతే
        req.w_విలువ = USB_DT_HUB<<8;
    req.w_సూచీ = 0;
    req.w_నిడివి = పరిమాణం(*desc);
    తిరిగి usb_send_default_control(pipe, &req, desc);
}

స్థిర పూర్ణాంకం
సమితి_hub_లోతు(నిర్మాణం usb_pipe *pipe, u16 లోతు)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_బహిర్గతం | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_సమితి_HUB_లోతు;
    req.w_విలువ = లోతు;
    req.w_సూచీ = 0;
    req.w_నిడివి = 0;
    తిరిగి usb_send_default_control(pipe, &req, ఖాళీ);
}

స్థిర పూర్ణాంకం
సమితి_పోర్టు_feature(నిర్మాణం usbhub_s *hub, పూర్ణాంకం పోర్టు, పూర్ణాంకం feature)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_బహిర్గతం | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_సమితి_FEATURE;
    req.w_విలువ = feature;
    req.w_సూచీ = పోర్టు + 1;
    req.w_నిడివి = 0;
    mutex_తాళం(&hub->తాళం_2);
    పూర్ణాంకం ret = usb_send_default_control(hub->usbdev->defpipe, &req, ఖాళీ);
    mutex_unlock(&hub->తాళం_2);
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
clear_పోర్టు_feature(నిర్మాణం usbhub_s *hub, పూర్ణాంకం పోర్టు, పూర్ణాంకం feature)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_బహిర్గతం | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_విలువ = feature;
    req.w_సూచీ = పోర్టు + 1;
    req.w_నిడివి = 0;
    mutex_తాళం(&hub->తాళం_2);
    పూర్ణాంకం ret = usb_send_default_control(hub->usbdev->defpipe, &req, ఖాళీ);
    mutex_unlock(&hub->తాళం_2);
    తిరిగి ret;
}

స్థిర పూర్ణాంకం
get_పోర్టు_స్థితి(నిర్మాణం usbhub_s *hub, పూర్ణాంకం పోర్టు, నిర్మాణం usb_పోర్టు_స్థితి *sts)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_లోపల | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_స్థితి;
    req.w_విలువ = 0;
    req.w_సూచీ = పోర్టు + 1;
    req.w_నిడివి = పరిమాణం(*sts);
    mutex_తాళం(&hub->తాళం_2);
    పూర్ణాంకం ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->తాళం_2);
    తిరిగి ret;
}

// Check if device attached to port
స్థిర పూర్ణాంకం
usb_hub_detect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_పోర్టు_స్థితి sts;
    పూర్ణాంకం ret = get_పోర్టు_స్థితి(hub, పోర్టు, &sts);
    ఒకవేళ (ret) {
        dprintf(1, "Failure on hub port %d detect\n", పోర్టు);
        తిరిగి -1;
    }
    తిరిగి (sts.w_పోర్టు_స్థితి & USB_పోర్టు_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
స్థిర శూన్య
usb_hub_disconnect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    పూర్ణాంకం ret = clear_పోర్టు_feature(hub, పోర్టు, USB_పోర్టు_FEAT_ENABLE);
    ఒకవేళ (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", పోర్టు);
}

// Reset device on port
స్థిర పూర్ణాంకం
usb_hub_reset(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    పూర్ణాంకం ret = సమితి_పోర్టు_feature(hub, పోర్టు, USB_పోర్టు_FEAT_RESET);
    ఒకవేళ (ret)
        వెళ్ళు fail;

    // Wait for reset to complete.
    నిర్మాణం usb_పోర్టు_స్థితి sts;
    u32 చివర = timer_calc(USB_సమయం_DRST * 2);
    కొరకు (;;) {
        ret = get_పోర్టు_స్థితి(hub, పోర్టు, &sts);
        ఒకవేళ (ret)
            వెళ్ళు fail;
        ఒకవేళ (!(sts.w_పోర్టు_స్థితి & USB_పోర్టు_STAT_RESET)
            && (hub->usbdev->వేగం != USB_SUPERSPEED
                || !(sts.w_పోర్టు_స్థితి & USB_పోర్టు_STAT_LINK_ముసుగు)))
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            వెళ్ళు fail;
        }
        msleep(5);
    }

    // Reset complete.
    ఒకవేళ (!(sts.w_పోర్టు_స్థితి & USB_పోర్టు_STAT_CONNECTION))
        // Device no longer present
        తిరిగి -1;

    ఒకవేళ (hub->usbdev->వేగం == USB_SUPERSPEED)
        తిరిగి USB_SUPERSPEED;
    తిరిగి ((sts.w_పోర్టు_స్థితి & USB_పోర్టు_STAT_వేగం_ముసుగు)
            >> USB_పోర్టు_STAT_వేగం_జరుపు);

fail:
    dprintf(1, "Failure on hub port %d reset\n", పోర్టు);
    usb_hub_disconnect(hub, పోర్టు);
    తిరిగి -1;
}

స్థిర నిర్మాణం usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
పూర్ణాంకం
usb_hub_అమరిక(నిర్మాణం usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_USB_HUB)
        తిరిగి -1;

    నిర్మాణం usb_hub_descriptor desc;
    పూర్ణాంకం ret = get_hub_desc(usbdev->defpipe, &desc);
    ఒకవేళ (ret)
        తిరిగి ret;

    నిర్మాణం usbhub_s hub;
    memset(&hub, 0, పరిమాణం(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    ఒకవేళ (usbdev->వేగం == USB_SUPERSPEED) {
        పూర్ణాంకం లోతు = 0;
        నిర్మాణం usbdevice_s *మాతృ = usbdev->hub->usbdev;
        అయితే (మాతృ) {
            లోతు++;
            మాతృ = మాతృ->hub->usbdev;
        }

        ret = సమితి_hub_లోతు(usbdev->defpipe, లోతు);
        ఒకవేళ (ret)
            తిరిగి ret;
    }

    // Turn on power to ports.
    పూర్ణాంకం పోర్టు;
    కొరకు (పోర్టు=0; పోర్టు<desc.bNbrPorts; పోర్టు++) {
        ret = సమితి_పోర్టు_feature(&hub, పోర్టు, USB_పోర్టు_FEAT_POWER);
        ఒకవేళ (ret)
            తిరిగి ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    ఒకవేళ (hub.devcount)
        తిరిగి 0;
    తిరిగి -1;
}
