// Code for handling standard USB hubs.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_USB_HUB
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-hub.h" // struct usb_hub_descriptor
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_hub_desc(ರಚನೆ usb_pipe *pipe, ರಚನೆ usb_hub_descriptor *desc)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಒಳ | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = USB_REQ_GET_DESCRIPTOR;
    ಆದರೆ (pipe->ವೇಗ == USB_SUPERSPEED)
        req.w_ಮೌಲ್ಯ = USB_DT_HUB3<<8;
    ಇಲ್ಲದಿದ್ದರೆ
        req.w_ಮೌಲ್ಯ = USB_DT_HUB<<8;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = ಗಾತ್ರ(*desc);
    ಹಿಂತಿರುಗಿ usb_send_default_control(pipe, &req, desc);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಗಣ_hub_ಆಳ(ರಚನೆ usb_pipe *pipe, u16 ಆಳ)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಹೊರಹೋಗು | USB_TYPE_CLASS | USB_RECIP_DEVICE;
    req.bRequest = HUB_REQ_ಗಣ_HUB_ಆಳ;
    req.w_ಮೌಲ್ಯ = ಆಳ;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = 0;
    ಹಿಂತಿರುಗಿ usb_send_default_control(pipe, &req, ಖಾಲಿ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಗಣ_ಪೋರ್ಟ್_feature(ರಚನೆ usbhub_s *hub, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ feature)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಹೊರಹೋಗು | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_ಗಣ_FEATURE;
    req.w_ಮೌಲ್ಯ = feature;
    req.w_ಸೂಚ್ಯಂಕ = ಪೋರ್ಟ್ + 1;
    req.w_ಉದ್ದಳತೆ = 0;
    mutex_ಬೀಗ(&hub->ಬೀಗ_2);
    ಪೂರ್ಣಾಂಕ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ಖಾಲಿ);
    mutex_unlock(&hub->ಬೀಗ_2);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
clear_ಪೋರ್ಟ್_feature(ರಚನೆ usbhub_s *hub, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ feature)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಹೊರಹೋಗು | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_CLEAR_FEATURE;
    req.w_ಮೌಲ್ಯ = feature;
    req.w_ಸೂಚ್ಯಂಕ = ಪೋರ್ಟ್ + 1;
    req.w_ಉದ್ದಳತೆ = 0;
    mutex_ಬೀಗ(&hub->ಬೀಗ_2);
    ಪೂರ್ಣಾಂಕ ret = usb_send_default_control(hub->usbdev->defpipe, &req, ಖಾಲಿ);
    mutex_unlock(&hub->ಬೀಗ_2);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(ರಚನೆ usbhub_s *hub, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ *sts)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಒಳ | USB_TYPE_CLASS | USB_RECIP_OTHER;
    req.bRequest = USB_REQ_GET_ಸ್ಥಿತಿ;
    req.w_ಮೌಲ್ಯ = 0;
    req.w_ಸೂಚ್ಯಂಕ = ಪೋರ್ಟ್ + 1;
    req.w_ಉದ್ದಳತೆ = ಗಾತ್ರ(*sts);
    mutex_ಬೀಗ(&hub->ಬೀಗ_2);
    ಪೂರ್ಣಾಂಕ ret = usb_send_default_control(hub->usbdev->defpipe, &req, sts);
    mutex_unlock(&hub->ಬೀಗ_2);
    ಹಿಂತಿರುಗಿ ret;
}

// Check if device attached to port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_hub_detect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ sts;
    ಪೂರ್ಣಾಂಕ ret = get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(hub, ಪೋರ್ಟ್, &sts);
    ಆದರೆ (ret) {
        dprintf(1, "Failure on hub port %d detect\n", ಪೋರ್ಟ್);
        ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ (sts.w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ & USB_ಪೋರ್ಟ್_STAT_CONNECTION) ? 1 : 0;
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
usb_hub_disconnect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ಪೂರ್ಣಾಂಕ ret = clear_ಪೋರ್ಟ್_feature(hub, ಪೋರ್ಟ್, USB_ಪೋರ್ಟ್_FEAT_ENABLE);
    ಆದರೆ (ret)
        dprintf(1, "Failure on hub port %d disconnect\n", ಪೋರ್ಟ್);
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_hub_reset(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ಪೂರ್ಣಾಂಕ ret = ಗಣ_ಪೋರ್ಟ್_feature(hub, ಪೋರ್ಟ್, USB_ಪೋರ್ಟ್_FEAT_RESET);
    ಆದರೆ (ret)
        ಹೋಗು fail;

    // Wait for reset to complete.
    ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ sts;
    u32 ಕೊನೆ = timer_calc(USB_ಸಮಯ_DRST * 2);
    ಗಾಗಿ (;;) {
        ret = get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(hub, ಪೋರ್ಟ್, &sts);
        ಆದರೆ (ret)
            ಹೋಗು fail;
        ಆದರೆ (!(sts.w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ & USB_ಪೋರ್ಟ್_STAT_RESET)
            && (hub->usbdev->ವೇಗ != USB_SUPERSPEED
                || !(sts.w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ & USB_ಪೋರ್ಟ್_STAT_LINK_ಮುಖವಾಡ)))
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹೋಗು fail;
        }
        msleep(5);
    }

    // Reset complete.
    ಆದರೆ (!(sts.w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ & USB_ಪೋರ್ಟ್_STAT_CONNECTION))
        // Device no longer present
        ಹಿಂತಿರುಗಿ -1;

    ಆದರೆ (hub->usbdev->ವೇಗ == USB_SUPERSPEED)
        ಹಿಂತಿರುಗಿ USB_SUPERSPEED;
    ಹಿಂತಿರುಗಿ ((sts.w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ & USB_ಪೋರ್ಟ್_STAT_ವೇಗ_ಮುಖವಾಡ)
            >> USB_ಪೋರ್ಟ್_STAT_ವೇಗ_ಸರಿಸು);

fail:
    dprintf(1, "Failure on hub port %d reset\n", ಪೋರ್ಟ್);
    usb_hub_disconnect(hub, ಪೋರ್ಟ್);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ರಚನೆ usbhub_op_s HubOp = {
    .detect = usb_hub_detect,
    .reset = usb_hub_reset,
    .disconnect = usb_hub_disconnect,
};

// Configure a usb hub and then find devices connected to it.
ಪೂರ್ಣಾಂಕ
usb_hub_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_USB_HUB)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ usb_hub_descriptor desc;
    ಪೂರ್ಣಾಂಕ ret = get_hub_desc(usbdev->defpipe, &desc);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ರಚನೆ usbhub_s hub;
    memset(&hub, 0, ಗಾತ್ರ(hub));
    hub.usbdev = usbdev;
    hub.cntl = usbdev->defpipe->cntl;
    hub.portcount = desc.bNbrPorts;
    hub.op = &HubOp;

    ಆದರೆ (usbdev->ವೇಗ == USB_SUPERSPEED) {
        ಪೂರ್ಣಾಂಕ ಆಳ = 0;
        ರಚನೆ usbdevice_s *ಪೋಷಕ = usbdev->hub->usbdev;
        ಆಗ (ಪೋಷಕ) {
            ಆಳ++;
            ಪೋಷಕ = ಪೋಷಕ->hub->usbdev;
        }

        ret = ಗಣ_hub_ಆಳ(usbdev->defpipe, ಆಳ);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
    }

    // Turn on power to ports.
    ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್;
    ಗಾಗಿ (ಪೋರ್ಟ್=0; ಪೋರ್ಟ್<desc.bNbrPorts; ಪೋರ್ಟ್++) {
        ret = ಗಣ_ಪೋರ್ಟ್_feature(&hub, ಪೋರ್ಟ್, USB_ಪೋರ್ಟ್_FEAT_POWER);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
    }
    // Wait for port power to stabilize.
    msleep(desc.bPwrOn2PwrGood * 2);

    usb_enumerate(&hub);

    dprintf(1, "Initialized USB HUB (%d ports used)\n", hub.devcount);
    ಆದರೆ (hub.devcount)
        ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ -1;
}
