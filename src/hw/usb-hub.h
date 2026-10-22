#اگر_غیر_تعریف_شدہ __USB_HUB_H
#تعریف_کرو __USB_HUB_H

// usb-hub.c
ساخت usbdevice_s;
صحیح usb_hub_تیاری(ساخت usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#تعریف_کرو USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#تعریف_کرو USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#تعریف_کرو HUB_REQ_سیٹ_HUB_گہرائی           0x0C

ساخت usb_hub_descriptor {
    u8  b_Desc_لمبائی;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#تعریف_کرو USB_پورٹ_FEAT_CONNECTION        0
#تعریف_کرو USB_پورٹ_FEAT_ENABLE            1
#تعریف_کرو USB_پورٹ_FEAT_SUSPEND           2
#تعریف_کرو USB_پورٹ_FEAT_OVER_CURRENT      3
#تعریف_کرو USB_پورٹ_FEAT_RESET             4
#تعریف_کرو USB_پورٹ_FEAT_POWER             8
#تعریف_کرو USB_پورٹ_FEAT_LOWSPEED          9
#تعریف_کرو USB_پورٹ_FEAT_C_CONNECTION      16
#تعریف_کرو USB_پورٹ_FEAT_C_ENABLE          17
#تعریف_کرو USB_پورٹ_FEAT_C_SUSPEND         18
#تعریف_کرو USB_پورٹ_FEAT_C_OVER_CURRENT    19
#تعریف_کرو USB_پورٹ_FEAT_C_RESET           20
#تعریف_کرو USB_پورٹ_FEAT_TEST              21
#تعریف_کرو USB_پورٹ_FEAT_INDICATOR         22
#تعریف_کرو USB_پورٹ_FEAT_C_پورٹ_L1         23

ساخت usb_پورٹ_حالت {
    u16 w_پورٹ_حالت;
    u16 w_پورٹ_Change;
} PACKED;

#تعریف_کرو USB_پورٹ_STAT_CONNECTION        0x0001
#تعریف_کرو USB_پورٹ_STAT_ENABLE            0x0002
#تعریف_کرو USB_پورٹ_STAT_SUSPEND           0x0004
#تعریف_کرو USB_پورٹ_STAT_OVERCURRENT       0x0008
#تعریف_کرو USB_پورٹ_STAT_RESET             0x0010
#تعریف_کرو USB_پورٹ_STAT_LINK_سرکاؤ        5
#تعریف_کرو USB_پورٹ_STAT_LINK_نقاب         (0x7 << USB_پورٹ_STAT_LINK_سرکاؤ)
#تعریف_کرو USB_پورٹ_STAT_POWER             0x0100
#تعریف_کرو USB_پورٹ_STAT_رفتار_سرکاؤ       9
#تعریف_کرو USB_پورٹ_STAT_رفتار_نقاب        (0x3 << USB_پورٹ_STAT_رفتار_سرکاؤ)
#تعریف_کرو USB_پورٹ_STAT_کم_رفتار         0x0200
#تعریف_کرو USB_پورٹ_STAT_بلند_رفتار        0x0400
#تعریف_کرو USB_پورٹ_STAT_TEST              0x0800
#تعریف_کرو USB_پورٹ_STAT_INDICATOR         0x1000

#اگر_ختم // ush-hid.h
