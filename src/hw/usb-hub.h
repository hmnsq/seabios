#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USB_HUB_H
#ವ್ಯಾಖ್ಯಾನಿಸು __USB_HUB_H

// usb-hub.c
ರಚನೆ usbdevice_s;
ಪೂರ್ಣಾಂಕ usb_hub_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#ವ್ಯಾಖ್ಯಾನಿಸು HUB_REQ_ಗಣ_HUB_ಆಳ           0x0C

ರಚನೆ usb_hub_descriptor {
    u8  b_Desc_ಉದ್ದಳತೆ;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_CONNECTION        0
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_ENABLE            1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_SUSPEND           2
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_OVER_CURRENT      3
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_RESET             4
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_POWER             8
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_LOWSPEED          9
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_C_CONNECTION      16
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_C_ENABLE          17
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_C_SUSPEND         18
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_C_OVER_CURRENT    19
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_C_RESET           20
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_TEST              21
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_INDICATOR         22
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_FEAT_C_ಪೋರ್ಟ್_L1         23

ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ {
    u16 w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ;
    u16 w_ಪೋರ್ಟ್_Change;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_CONNECTION        0x0001
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_ENABLE            0x0002
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_SUSPEND           0x0004
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_OVERCURRENT       0x0008
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_RESET             0x0010
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_LINK_ಸರಿಸು        5
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_LINK_ಮುಖವಾಡ         (0x7 << USB_ಪೋರ್ಟ್_STAT_LINK_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_POWER             0x0100
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_ವೇಗ_ಸರಿಸು       9
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_ವೇಗ_ಮುಖವಾಡ        (0x3 << USB_ಪೋರ್ಟ್_STAT_ವೇಗ_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_ಕೆಳಗಿನ_ವೇಗ         0x0200
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_ಮೇಲಿನ_ವೇಗ        0x0400
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_TEST              0x0800
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಪೋರ್ಟ್_STAT_INDICATOR         0x1000

#ಆದರೆ_ಅಂತ್ಯ // ush-hid.h
