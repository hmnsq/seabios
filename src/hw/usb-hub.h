#اگر_تعریف_نشده __USB_HUB_H
#تعریف_کن __USB_HUB_H

// usb-hub.c
بنية usbdevice_s;
صحيح usb_hub_راهاندازی(بنية usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#تعریف_کن USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#تعریف_کن USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#تعریف_کن HUB_REQ_مجموعه_HUB_عمق           0x0C

بنية usb_hub_descriptor {
    u8  b_Desc_درازی;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#تعریف_کن USB_پورت_FEAT_CONNECTION        0
#تعریف_کن USB_پورت_FEAT_ENABLE            1
#تعریف_کن USB_پورت_FEAT_SUSPEND           2
#تعریف_کن USB_پورت_FEAT_OVER_CURRENT      3
#تعریف_کن USB_پورت_FEAT_RESET             4
#تعریف_کن USB_پورت_FEAT_POWER             8
#تعریف_کن USB_پورت_FEAT_LOWSPEED          9
#تعریف_کن USB_پورت_FEAT_C_CONNECTION      16
#تعریف_کن USB_پورت_FEAT_C_ENABLE          17
#تعریف_کن USB_پورت_FEAT_C_SUSPEND         18
#تعریف_کن USB_پورت_FEAT_C_OVER_CURRENT    19
#تعریف_کن USB_پورت_FEAT_C_RESET           20
#تعریف_کن USB_پورت_FEAT_TEST              21
#تعریف_کن USB_پورت_FEAT_INDICATOR         22
#تعریف_کن USB_پورت_FEAT_C_پورت_L1         23

بنية usb_پورت_وضعیت {
    u16 w_پورت_وضعیت;
    u16 w_پورت_Change;
} PACKED;

#تعریف_کن USB_پورت_STAT_CONNECTION        0x0001
#تعریف_کن USB_پورت_STAT_ENABLE            0x0002
#تعریف_کن USB_پورت_STAT_SUSPEND           0x0004
#تعریف_کن USB_پورت_STAT_OVERCURRENT       0x0008
#تعریف_کن USB_پورت_STAT_RESET             0x0010
#تعریف_کن USB_پورت_STAT_LINK_لغزش        5
#تعریف_کن USB_پورت_STAT_LINK_ماسک         (0x7 << USB_پورت_STAT_LINK_لغزش)
#تعریف_کن USB_پورت_STAT_POWER             0x0100
#تعریف_کن USB_پورت_STAT_سرعت_لغزش       9
#تعریف_کن USB_پورت_STAT_سرعت_ماسک        (0x3 << USB_پورت_STAT_سرعت_لغزش)
#تعریف_کن USB_پورت_STAT_کم_سرعت         0x0200
#تعریف_کن USB_پورت_STAT_بالا_سرعت        0x0400
#تعریف_کن USB_پورت_STAT_TEST              0x0800
#تعریف_کن USB_پورت_STAT_INDICATOR         0x1000

#پایان_اگر // ush-hid.h
