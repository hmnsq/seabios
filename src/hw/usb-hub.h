#که_نه_تعریف_شوی __USB_HUB_H
#تعریف_کړه __USB_HUB_H

// usb-hub.c
بنية usbdevice_s;
صحيح usb_hub_تیاری(بنية usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#تعریف_کړه USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#تعریف_کړه USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#تعریف_کړه HUB_REQ_سېټ_HUB_ژوروالی           0x0C

بنية usb_hub_descriptor {
    u8  b_Desc_اوږدتیا;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#تعریف_کړه USB_پورټ_FEAT_CONNECTION        0
#تعریف_کړه USB_پورټ_FEAT_ENABLE            1
#تعریف_کړه USB_پورټ_FEAT_SUSPEND           2
#تعریف_کړه USB_پورټ_FEAT_OVER_CURRENT      3
#تعریف_کړه USB_پورټ_FEAT_RESET             4
#تعریف_کړه USB_پورټ_FEAT_POWER             8
#تعریف_کړه USB_پورټ_FEAT_LOWSPEED          9
#تعریف_کړه USB_پورټ_FEAT_C_CONNECTION      16
#تعریف_کړه USB_پورټ_FEAT_C_ENABLE          17
#تعریف_کړه USB_پورټ_FEAT_C_SUSPEND         18
#تعریف_کړه USB_پورټ_FEAT_C_OVER_CURRENT    19
#تعریف_کړه USB_پورټ_FEAT_C_RESET           20
#تعریف_کړه USB_پورټ_FEAT_TEST              21
#تعریف_کړه USB_پورټ_FEAT_INDICATOR         22
#تعریف_کړه USB_پورټ_FEAT_C_پورټ_L1         23

بنية usb_پورټ_حالت {
    u16 w_پورټ_حالت;
    u16 w_پورټ_Change;
} PACKED;

#تعریف_کړه USB_پورټ_STAT_CONNECTION        0x0001
#تعریف_کړه USB_پورټ_STAT_ENABLE            0x0002
#تعریف_کړه USB_پورټ_STAT_SUSPEND           0x0004
#تعریف_کړه USB_پورټ_STAT_OVERCURRENT       0x0008
#تعریف_کړه USB_پورټ_STAT_RESET             0x0010
#تعریف_کړه USB_پورټ_STAT_LINK_ښويدنه        5
#تعریف_کړه USB_پورټ_STAT_LINK_ماسک         (0x7 << USB_پورټ_STAT_LINK_ښويدنه)
#تعریف_کړه USB_پورټ_STAT_POWER             0x0100
#تعریف_کړه USB_پورټ_STAT_چټکتیا_ښويدنه       9
#تعریف_کړه USB_پورټ_STAT_چټکتیا_ماسک        (0x3 << USB_پورټ_STAT_چټکتیا_ښويدنه)
#تعریف_کړه USB_پورټ_STAT_ټیټ_چټکتیا         0x0200
#تعریف_کړه USB_پورټ_STAT_لوړ_چټکتیا        0x0400
#تعریف_کړه USB_پورټ_STAT_TEST              0x0800
#تعریف_کړه USB_پورټ_STAT_INDICATOR         0x1000

#د_که_پای // ush-hid.h
