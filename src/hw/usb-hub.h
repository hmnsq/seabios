#جيڪڏهن_وصف_نه_ٿيل __USB_HUB_H
#وصف_ڪريو __USB_HUB_H

// usb-hub.c
بنية usbdevice_s;
صحيح usb_hub_تياري(بنية usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#وصف_ڪريو USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#وصف_ڪريو USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#وصف_ڪريو HUB_REQ_سيٽ_HUB_اونهائي           0x0C

بنية usb_hub_descriptor {
    u8  b_Desc_ڊيگهائي;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#وصف_ڪريو USB_پورٽ_FEAT_CONNECTION        0
#وصف_ڪريو USB_پورٽ_FEAT_ENABLE            1
#وصف_ڪريو USB_پورٽ_FEAT_SUSPEND           2
#وصف_ڪريو USB_پورٽ_FEAT_OVER_CURRENT      3
#وصف_ڪريو USB_پورٽ_FEAT_RESET             4
#وصف_ڪريو USB_پورٽ_FEAT_POWER             8
#وصف_ڪريو USB_پورٽ_FEAT_LOWSPEED          9
#وصف_ڪريو USB_پورٽ_FEAT_C_CONNECTION      16
#وصف_ڪريو USB_پورٽ_FEAT_C_ENABLE          17
#وصف_ڪريو USB_پورٽ_FEAT_C_SUSPEND         18
#وصف_ڪريو USB_پورٽ_FEAT_C_OVER_CURRENT    19
#وصف_ڪريو USB_پورٽ_FEAT_C_RESET           20
#وصف_ڪريو USB_پورٽ_FEAT_TEST              21
#وصف_ڪريو USB_پورٽ_FEAT_INDICATOR         22
#وصف_ڪريو USB_پورٽ_FEAT_C_پورٽ_L1         23

بنية usb_پورٽ_حالت {
    u16 w_پورٽ_حالت;
    u16 w_پورٽ_Change;
} PACKED;

#وصف_ڪريو USB_پورٽ_STAT_CONNECTION        0x0001
#وصف_ڪريو USB_پورٽ_STAT_ENABLE            0x0002
#وصف_ڪريو USB_پورٽ_STAT_SUSPEND           0x0004
#وصف_ڪريو USB_پورٽ_STAT_OVERCURRENT       0x0008
#وصف_ڪريو USB_پورٽ_STAT_RESET             0x0010
#وصف_ڪريو USB_پورٽ_STAT_LINK_سرڪايو        5
#وصف_ڪريو USB_پورٽ_STAT_LINK_نقاب         (0x7 << USB_پورٽ_STAT_LINK_سرڪايو)
#وصف_ڪريو USB_پورٽ_STAT_POWER             0x0100
#وصف_ڪريو USB_پورٽ_STAT_رفتار_سرڪايو       9
#وصف_ڪريو USB_پورٽ_STAT_رفتار_نقاب        (0x3 << USB_پورٽ_STAT_رفتار_سرڪايو)
#وصف_ڪريو USB_پورٽ_STAT_هيٺيون_رفتار         0x0200
#وصف_ڪريو USB_پورٽ_STAT_مٿاهون_رفتار        0x0400
#وصف_ڪريو USB_پورٽ_STAT_TEST              0x0800
#وصف_ڪريو USB_پورٽ_STAT_INDICATOR         0x1000

#جيڪڏهن_پورو // ush-hid.h
