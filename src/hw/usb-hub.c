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

स्थिर पूर्णाङ्क
get_hub_desc(संरचना usb_pipe *pipe, संरचना usb_hub_descriptor *desc)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_अन्तर् | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    यदि (pipe->जवः == USB_SUPERSPEED)
        req.w_मानम् = USB_DT_HUB3<<8;
    अन्यथा
        req.w_मानम् = USB_DT_HUB<<8;
    req.w_सूचकांक = 0;
    req.w_दीर्घता = परिमाण(*desc);
    प्रत्यागम usb_send_default_control(pipe, &req, desc);
}

स्थिर पूर्णाङ्क
समुच्चयः_hub_गभीरता(संरचना usb_pipe *pipe, u16 गभीरता)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_बहिर्गमनम् | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_समुच्चयः_HUB_गभीरता;
    req.w_मानम् = गभीरता;
    req.w_सूचकांक = 0;
    req.w_दीर्घता = 0;
    प्रत्यागम usb_send_default_control(pipe, &req, रिक्त);
}

स्थिर पूर्णाङ्क
समुच्चयः_पत्तनम्_feature(संरचना usbhub_s *hub, पूर्णाङ्क पत्तनम्, पूर्णाङ्क feature)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_बहिर्गमनम् | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_समुच्चयः_FEATURE;
    req.w_मानम् = feature;
    req.w_सूचकांक = पत्तनम् + 1;
    req.w_दीर्घता = 0;
    mutex_तालकम्(&hub->तालकम्);
    पूर्णाङ्क ret = usb_send_default_control(hub->usbdev->defpipe, &req, रिक्त);
    mutex_unlock(&hub->तालकम्);
    प्रत्यागम ret;
}

स्थिर पूर्णाङ्क
clear_पत्तनम्_feature(संरचना usbhub_s *hub, पूर्णाङ्क पत्तनम्, पूर्णाङ्क feature)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_बहिर्गमनम् | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_मानम् = feature;
    req.w_सूचकांक = पत्तनम् + 1;
    req.w_दीर्घता = 0;
    mutex_तालकम्(&hub->तालकम्);
    पूर्णाङ्क ret = usb_send_default_control(hub->usbdev->defpipe, &req, रिक्त);
    mutex_unlock(&hub->तालकम्);
    प्रत्यागम ret;
}

स्थिर पूर्णाङ्क
get_पत्तनम्_स्थितिः(संरचना usbhub_s *hub, पूर्णाङ्क पत्तनम्, संरचना usb_पत्तनम्_स्थितिः *sts)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_अन्तर् | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_स्थितिः;
    req.w_मानम् = 0;
    req.w_सूचकांक = पत्तनम् + 1;
    req.w_दीर्घता = परिमाण(*sts);
    mutex_तालकम्(&hub->तालकम्);
    पूर्णाङ्क ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->तालकम्);
    प्रत्यागम ret;
}

// Check if device attached to port
स्थिर पूर्णाङ्क
usb_hub_detect(संरचना usbhub_s *hub, u32 पत्तनम्)
{
    संरचना usb_पत्तनम्_स्थितिः sts;
    पूर्णाङ्क ret = get_पत्तनम्_स्थितिः(hub, पत्तनम्, &sts);
    यदि (ret) {
        dprintf(1, "Failure on hub port %d detect\n", पत्तनम्);
        प्रत्यागम -1;
    }
    प्रत्यागम (sts.w_पत्तनम्_स्थितिः & USB_पत्तनम्_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
स्थिर शून्य
usb_hub_disconnect(संरचना usbhub_s *hub, u32 पत्तनम्)
{
    पूर्णाङ्क ret = clear_पत्तनम्_feature(hub, पत्तनम्, USB_पत्तनम्_FEAT_ENABLE);
    यदि (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", पत्तनम्);
}

// Reset device on port
स्थिर पूर्णाङ्क
usb_hub_reset(संरचना usbhub_s *hub, u32 पत्तनम्)
{
    पूर्णाङ्क ret = समुच्चयः_पत्तनम्_feature(hub, पत्तनम्, USB_पत्तनम्_FEAT_RESET);
    यदि (ret)
        गच्छ fail;

    // Wait for reset to complete.
    संरचना usb_पत्तनम्_स्थितिः sts;
    u32 अन्तः = timer_calc(USB_TIME_DRST * 2);
    प्रति (;;) {
        ret = get_पत्तनम्_स्थितिः(hub, पत्तनम्, &sts);
        यदि (ret)
            गच्छ fail;
        यदि (!(sts.w_पत्तनम्_स्थितिः & USB_पत्तनम्_STAT_RESET)
            && (hub->usbdev->जवः != USB_SUPERSPEED
                || !(sts.w_पत्तनम्_स्थितिः & USB_पत्तनम्_STAT_LINK_आवरणम्)))
            भङ्ग;
        यदि (timer_check(अन्तः)) {
            warn_timeout();
            गच्छ fail;
        }
        msleep(5);
    }

    // Reset complete.
    यदि (!(sts.w_पत्तनम्_स्थितिः & USB_पत्तनम्_STAT_CONNECTION))
        // Device no longer present
        प्रत्यागम -1;

    यदि (hub->usbdev->जवः == USB_SUPERSPEED)
        प्रत्यागम USB_SUPERSPEED;
    प्रत्यागम ((sts.w_पत्तनम्_स्थितिः & USB_पत्तनम्_STAT_जवः_आवरणम्)
            >> USB_पत्तनम्_STAT_जवः_सरणम्);

fail:
    dprintf(1, "Failure on hub port %d reset\n", पत्तनम्);
    usb_hub_disconnect(hub, पत्तनम्);
    प्रत्यागम -1;
}

स्थिर संरचना usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
पूर्णाङ्क
usb_hub_सज्जता(संरचना usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    यदि (!CONFIG_USB_HUB)
        प्रत्यागम -1;

    संरचना usb_hub_descriptor desc;
    पूर्णाङ्क ret = get_hub_desc(usbdev->defpipe, &desc);
    यदि (ret)
        प्रत्यागम ret;

    संरचना usbhub_s hub;
    स्मृतिं_पूरय(&hub, 0, परिमाण(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    यदि (usbdev->जवः == USB_SUPERSPEED) {
        पूर्णाङ्क गभीरता = 0;
        संरचना usbdevice_s *parent = usbdev->hub->usbdev;
        यावत् (parent) {
            गभीरता++;
            parent = parent->hub->usbdev;
        }

        ret = समुच्चयः_hub_गभीरता(usbdev->defpipe, गभीरता);
        यदि (ret)
            प्रत्यागम ret;
    }

    // Turn on power to ports.
    पूर्णाङ्क पत्तनम्;
    प्रति (पत्तनम्=0; पत्तनम्<desc.bNbrPorts; पत्तनम्++) {
        ret = समुच्चयः_पत्तनम्_feature(&hub, पत्तनम्, USB_पत्तनम्_FEAT_POWER);
        यदि (ret)
            प्रत्यागम ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    यदि (hub.devcount)
        प्रत्यागम 0;
    प्रत्यागम -1;
}
