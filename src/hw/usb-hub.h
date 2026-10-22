#నిర్వచితం_కాకపోతే __USB_HUB_H
#నిర్వచించు __USB_HUB_H

// usb-hub.c
నిర్మాణం usbdevice_s;
పూర్ణాంకం usb_hub_అమరిక(నిర్మాణం usbdevice_s *usbdev);


/****************************************************************
 * hub flags
 ****************************************************************/

#నిర్వచించు USB_DT_HUB                      (USB_TYPE_CLASS | 0x09)
#నిర్వచించు USB_DT_HUB3                     (USB_TYPE_CLASS | 0x0a)

#నిర్వచించు HUB_REQ_సమితి_HUB_లోతు           0x0C

నిర్మాణం usb_hub_descriptor {
    u8  b_Desc_నిడివి;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    // Variable length fields for DeviceRemovable[], PortPwrCtrlMask[] follow.
} PACKED;

#నిర్వచించు USB_పోర్టు_FEAT_CONNECTION        0
#నిర్వచించు USB_పోర్టు_FEAT_ENABLE            1
#నిర్వచించు USB_పోర్టు_FEAT_SUSPEND           2
#నిర్వచించు USB_పోర్టు_FEAT_OVER_CURRENT      3
#నిర్వచించు USB_పోర్టు_FEAT_RESET             4
#నిర్వచించు USB_పోర్టు_FEAT_POWER             8
#నిర్వచించు USB_పోర్టు_FEAT_LOWSPEED          9
#నిర్వచించు USB_పోర్టు_FEAT_C_CONNECTION      16
#నిర్వచించు USB_పోర్టు_FEAT_C_ENABLE          17
#నిర్వచించు USB_పోర్టు_FEAT_C_SUSPEND         18
#నిర్వచించు USB_పోర్టు_FEAT_C_OVER_CURRENT    19
#నిర్వచించు USB_పోర్టు_FEAT_C_RESET           20
#నిర్వచించు USB_పోర్టు_FEAT_TEST              21
#నిర్వచించు USB_పోర్టు_FEAT_INDICATOR         22
#నిర్వచించు USB_పోర్టు_FEAT_C_పోర్టు_L1         23

నిర్మాణం usb_పోర్టు_స్థితి {
    u16 w_పోర్టు_స్థితి;
    u16 w_పోర్టు_Change;
} PACKED;

#నిర్వచించు USB_పోర్టు_STAT_CONNECTION        0x0001
#నిర్వచించు USB_పోర్టు_STAT_ENABLE            0x0002
#నిర్వచించు USB_పోర్టు_STAT_SUSPEND           0x0004
#నిర్వచించు USB_పోర్టు_STAT_OVERCURRENT       0x0008
#నిర్వచించు USB_పోర్టు_STAT_RESET             0x0010
#నిర్వచించు USB_పోర్టు_STAT_LINK_జరుపు        5
#నిర్వచించు USB_పోర్టు_STAT_LINK_ముసుగు         (0x7 << USB_పోర్టు_STAT_LINK_జరుపు)
#నిర్వచించు USB_పోర్టు_STAT_POWER             0x0100
#నిర్వచించు USB_పోర్టు_STAT_వేగం_జరుపు       9
#నిర్వచించు USB_పోర్టు_STAT_వేగం_ముసుగు        (0x3 << USB_పోర్టు_STAT_వేగం_జరుపు)
#నిర్వచించు USB_పోర్టు_STAT_దిగువ_వేగం         0x0200
#నిర్వచించు USB_పోర్టు_STAT_ఎగువ_వేగం        0x0400
#నిర్వచించు USB_పోర్టు_STAT_TEST              0x0800
#నిర్వచించు USB_పోర్టు_STAT_INDICATOR         0x1000

#అయితే_ముగింపు // ush-hid.h
