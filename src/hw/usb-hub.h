#यदि_अपरिभाषितम् __USB_HUB_H
#परिभाषय __USB_HUB_H

// usb-hub.c
संरचना usbdevice_s;
पूर्णाङ्क usb_hub_सज्जता(संरचना usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#परिभाषय USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#परिभाषय USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#परिभाषय HUB_REQ_समुच्चयः_HUB_गभीरता           0x0C

संरचना usb_hub_descriptor {
    u8  b_Desc_दीर्घता;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#परिभाषय USB_पत्तनम्_FEAT_CONNECTION        0
#परिभाषय USB_पत्तनम्_FEAT_ENABLE            1
#परिभाषय USB_पत्तनम्_FEAT_SUSPEND           2
#परिभाषय USB_पत्तनम्_FEAT_OVER_CURRENT      3
#परिभाषय USB_पत्तनम्_FEAT_RESET             4
#परिभाषय USB_पत्तनम्_FEAT_POWER             8
#परिभाषय USB_पत्तनम्_FEAT_LOWSPEED          9
#परिभाषय USB_पत्तनम्_FEAT_C_CONNECTION      16
#परिभाषय USB_पत्तनम्_FEAT_C_ENABLE          17
#परिभाषय USB_पत्तनम्_FEAT_C_SUSPEND         18
#परिभाषय USB_पत्तनम्_FEAT_C_OVER_CURRENT    19
#परिभाषय USB_पत्तनम्_FEAT_C_RESET           20
#परिभाषय USB_पत्तनम्_FEAT_TEST              21
#परिभाषय USB_पत्तनम्_FEAT_INDICATOR         22
#परिभाषय USB_पत्तनम्_FEAT_C_पत्तनम्_L1         23

संरचना usb_पत्तनम्_स्थितिः {
    u16 w_पत्तनम्_स्थितिः;
    u16 w_पत्तनम्_Change;
} PACKED;

#परिभाषय USB_पत्तनम्_STAT_CONNECTION        0x0001
#परिभाषय USB_पत्तनम्_STAT_ENABLE            0x0002
#परिभाषय USB_पत्तनम्_STAT_SUSPEND           0x0004
#परिभाषय USB_पत्तनम्_STAT_OVERCURRENT       0x0008
#परिभाषय USB_पत्तनम्_STAT_RESET             0x0010
#परिभाषय USB_पत्तनम्_STAT_LINK_सरणम्        5
#परिभाषय USB_पत्तनम्_STAT_LINK_आवरणम्         (0x7 << USB_पत्तनम्_STAT_LINK_सरणम्)
#परिभाषय USB_पत्तनम्_STAT_POWER             0x0100
#परिभाषय USB_पत्तनम्_STAT_जवः_सरणम्       9
#परिभाषय USB_पत्तनम्_STAT_जवः_आवरणम्        (0x3 << USB_पत्तनम्_STAT_जवः_सरणम्)
#परिभाषय USB_पत्तनम्_STAT_नीचः_जवः         0x0200
#परिभाषय USB_पत्तनम्_STAT_उच्चः_जवः        0x0400
#परिभाषय USB_पत्तनम्_STAT_TEST              0x0800
#परिभाषय USB_पत्तनम्_STAT_INDICATOR         0x1000

#यदि_अन्तः // ush-hid.h
