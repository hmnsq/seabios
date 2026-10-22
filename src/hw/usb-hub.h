#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USB_HUB_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USB_HUB_H

// usb-hub.c
ਬਣਤਰ usbdevice_s;
ਪੂਰਨ usb_hub_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HUB_REQ_ਸੈੱਟ_HUB_ਡੂੰਘਾਈ           0x0C

ਬਣਤਰ usb_hub_descriptor {
    u8  b_Desc_ਲੰਮਾਈ;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_CONNECTION        0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_ENABLE            1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_SUSPEND           2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_OVER_CURRENT      3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_RESET             4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_POWER             8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_LOWSPEED          9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_C_CONNECTION      16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_C_ENABLE          17
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_C_SUSPEND         18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_C_OVER_CURRENT    19
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_C_RESET           20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_TEST              21
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_INDICATOR         22
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_FEAT_C_ਪੋਰਟ_L1         23

ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ {
    u16 w_ਪੋਰਟ_ਸਥਿਤੀ;
    u16 w_ਪੋਰਟ_Change;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_CONNECTION        0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_ENABLE            0x0002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_SUSPEND           0x0004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_OVERCURRENT       0x0008
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_RESET             0x0010
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_LINK_ਖਿਸਕਾਓ        5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_LINK_ਨਕਾਬ         (0x7 << USB_ਪੋਰਟ_STAT_LINK_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_POWER             0x0100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_ਰਫਤਾਰ_ਖਿਸਕਾਓ       9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_ਰਫਤਾਰ_ਨਕਾਬ        (0x3 << USB_ਪੋਰਟ_STAT_ਰਫਤਾਰ_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_ਨੀਵਾਂ_ਰਫਤਾਰ         0x0200
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_ਉੱਚਾ_ਰਫਤਾਰ        0x0400
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_TEST              0x0800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਪੋਰਟ_STAT_INDICATOR         0x1000

#ਜੇ_ਅੰਤ // ush-hid.h
