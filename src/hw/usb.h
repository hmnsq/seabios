// USB functions and data.
#यदि_अपरिभाषितम् __USB_H
#परिभाषय __USB_H

#अन्तर्भाव "stacks.h" // struct mutex_s

// Information on a USB end point.
संरचना usb_pipe {
    सङ्घ {
        संरचना usb_s *cntl;
        संरचना usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 जवः;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
संरचना usbdevice_s {
    संरचना usbhub_s *hub;
    संरचना usb_pipe *defpipe;
    u32 पत्तनम्;
    संरचना usb_config_descriptor *config;
    संरचना usb_interface_descriptor *iface;
    पूर्णाङ्क imax;
    u8 जवः;
    u8 devaddr;
};

// Common information for usb controllers.
संरचना usb_s {
    संरचना usb_pipe *freelist;
    संरचना mutex_s resetlock;
    संरचना pci_device *pci;
    शून्य *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
संरचना usbhub_s {
    संरचना usbhub_op_s *op;
    संरचना usbdevice_s *usbdev;
    संरचना usb_s *cntl;
    संरचना mutex_s तालकम्;
    u32 detectend;
    u32 पत्तनम्;
    u32 तन्तवः;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
संरचना usbhub_op_s {
    पूर्णाङ्क (*detect)(संरचना usbhub_s *hub, u32 पत्तनम्);
    पूर्णाङ्क (*reset)(संरचना usbhub_s *hub, u32 पत्तनम्);
    पूर्णाङ्क (*portmap)(संरचना usbhub_s *hub, u32 पत्तनम्);
    शून्य (*disconnect)(संरचना usbhub_s *hub, u32 पत्तनम्);
};

#परिभाषय USB_TYPE_UHCI  1
#परिभाषय USB_TYPE_OHCI  2
#परिभाषय USB_TYPE_EHCI  3
#परिभाषय USB_TYPE_XHCI  4

#परिभाषय USB_FULLSPEED  0
#परिभाषय USB_LOWSPEED   1
#परिभाषय USB_HIGHSPEED  2
#परिभाषय USB_SUPERSPEED 3

#परिभाषय USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#परिभाषय USB_TIME_SIGATT 100
#परिभाषय USB_TIME_ATTDB  100
#परिभाषय USB_TIME_DRST   10
#परिभाषय USB_TIME_DRSTR  50
#परिभाषय USB_TIME_RSTRCY 10

#परिभाषय USB_TIME_स्थितिः  50
#परिभाषय USB_TIME_DATAIN  500
#परिभाषय USB_TIME_COMMAND 5000

#परिभाषय USB_TIME_SETADDR_RECOVERY 2

#परिभाषय USB_PID_बहिर्गमनम्                     0xe1
#परिभाषय USB_PID_अन्तर्                      0x69
#परिभाषय USB_PID_सज्जता                   0x2d

#परिभाषय USB_DIR_बहिर्गमनम्                     0               /* to device */
#परिभाषय USB_DIR_अन्तर्                      0x80            /* to host */

#परिभाषय USB_TYPE_आवरणम्                   (0x03 << 5)
#परिभाषय USB_TYPE_STANDARD               (0x00 << 5)
#परिभाषय USB_TYPE_CLASS                  (0x01 << 5)
#परिभाषय USB_TYPE_VENDOR                 (0x02 << 5)
#परिभाषय USB_TYPE_RESERVED               (0x03 << 5)

#परिभाषय USB_RECIP_आवरणम्                  0x1f
#परिभाषय USB_RECIP_DEVICE                0x00
#परिभाषय USB_RECIP_INTERFACE             0x01
#परिभाषय USB_RECIP_ENDPOINT              0x02
#परिभाषय USB_RECIP_OTHER                 0x03

#परिभाषय USB_REQ_GET_स्थितिः              0x00
#परिभाषय USB_REQ_CLEAR_FEATURE           0x01
#परिभाषय USB_REQ_समुच्चयः_FEATURE             0x03
#परिभाषय USB_REQ_समुच्चयः_स्थानाङ्कः             0x05
#परिभाषय USB_REQ_GET_DESCRIPTOR          0x06
#परिभाषय USB_REQ_समुच्चयः_DESCRIPTOR          0x07
#परिभाषय USB_REQ_GET_CONFIGURATION       0x08
#परिभाषय USB_REQ_समुच्चयः_CONFIGURATION       0x09
#परिभाषय USB_REQ_GET_INTERFACE           0x0A
#परिभाषय USB_REQ_समुच्चयः_INTERFACE           0x0B
#परिभाषय USB_REQ_SYNCH_FRAME             0x0C

संरचना usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_मानम्;
    u16 w_सूचकांक;
    u16 w_दीर्घता;
} PACKED;

#परिभाषय USB_DT_DEVICE                   0x01
#परिभाषय USB_DT_CONFIG                   0x02
#परिभाषय USB_DT_STRING                   0x03
#परिभाषय USB_DT_INTERFACE                0x04
#परिभाषय USB_DT_ENDPOINT                 0x05
#परिभाषय USB_DT_DEVICE_QUALIFIER         0x06
#परिभाषय USB_DT_OTHER_जवः_CONFIG       0x07
#परिभाषय USB_DT_ENDPOINT_COMPANION       0x30

संरचना usb_device_descriptor {
    u8  b_दीर्घता;
    u8  bDescriptorType;

    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 id_गुणनफलम्;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  i_गुणनफलम्;
    u8  i_Serial_सङ्ख्या;
    u8  b_अङ्कः_Configurations;
} PACKED;

#परिभाषय USB_CLASS_प्रत्येकम्_INTERFACE         0       /* for DeviceClass */
#परिभाषय USB_CLASS_AUDIO                 1
#परिभाषय USB_CLASS_COMM                  2
#परिभाषय USB_CLASS_HID                   3
#परिभाषय USB_CLASS_PHYSICAL              5
#परिभाषय USB_CLASS_STILL_IMAGE           6
#परिभाषय USB_CLASS_PRINTER               7
#परिभाषय USB_CLASS_MASS_STORAGE          8
#परिभाषय USB_CLASS_HUB                   9

संरचना usb_config_descriptor {
    u8  b_दीर्घता;
    u8  bDescriptorType;

    u16 w_योग_दीर्घता;
    u8  b_अङ्कः_Interfaces;
    u8  b_Configuration_मानम्;
    u8  iConfiguration;
    u8  bm_गुणाः;
    u8  bMaxPower;
} PACKED;

संरचना usb_interface_descriptor {
    u8  b_दीर्घता;
    u8  bDescriptorType;

    u8  b_Interface_सङ्ख्या;
    u8  bAlternateSetting;
    u8  b_अङ्कः_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

संरचना usb_endpoint_descriptor {
    u8  b_दीर्घता;
    u8  bDescriptorType;

    u8  b_Endpoint_स्थानाङ्कः;
    u8  bm_गुणाः;
    u16 w_Max_Packet_आकार;
    u8  bInterval;
} PACKED;

#परिभाषय USB_ENDPOINT_सङ्ख्या_आवरणम्        0x0f    /* in bEndpointAddress */
#परिभाषय USB_ENDPOINT_DIR_आवरणम्           0x80

#परिभाषय USB_ENDPOINT_XFERTYPE_आवरणम्      0x03    /* in bmAttributes */
#परिभाषय USB_ENDPOINT_XFER_CONTROL       0
#परिभाषय USB_ENDPOINT_XFER_ISOC          1
#परिभाषय USB_ENDPOINT_XFER_BULK          2
#परिभाषय USB_ENDPOINT_XFER_INT           3
#परिभाषय USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#परिभाषय USB_CONTROL_सज्जता_आकार          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#परिभाषय US_SC_ATAPI_8020   0x02
#परिभाषय US_SC_ATAPI_8070   0x05
#परिभाषय US_SC_SCSI         0x06

#परिभाषय US_PR_BULK         0x50  /* bulk-only transport */
#परिभाषय US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
पूर्णाङ्क usb_send_bulk(संरचना usb_pipe *pipe, पूर्णाङ्क dir, शून्य *दत्त, पूर्णाङ्क datasize);
पूर्णाङ्क usb_poll_intr(संरचना usb_pipe *pipe, शून्य *दत्त);
पूर्णाङ्क usb_32bit_pipe(संरचना usb_pipe *pipe_fl);
संरचना usb_pipe *usb_alloc_pipe(संरचना usbdevice_s *usbdev
                                , संरचना usb_endpoint_descriptor *epdesc);
शून्य usb_free_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *pipe);
पूर्णाङ्क usb_send_default_control(संरचना usb_pipe *pipe
                             , ध्रुव संरचना usb_ctrlrequest *req, शून्य *दत्त);
पूर्णाङ्क usb_is_freelist(संरचना usb_s *cntl, संरचना usb_pipe *pipe);
शून्य usb_सङ्कलय_freelist(संरचना usb_pipe *pipe);
संरचना usb_pipe *usb_get_freelist(संरचना usb_s *cntl, u8 eptype);
शून्य usb_desc2pipe(संरचना usb_pipe *pipe, संरचना usbdevice_s *usbdev
                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णाङ्क usb_get_period(संरचना usbdevice_s *usbdev
                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णाङ्क usb_xfer_time(संरचना usb_pipe *pipe, पूर्णाङ्क datalen);
संरचना usb_endpoint_descriptor *usb_find_desc(संरचना usbdevice_s *usbdev
                                              , पूर्णाङ्क type, पूर्णाङ्क dir);
शून्य usb_enumerate(संरचना usbhub_s *hub);
शून्य usb_सज्जता(शून्य);

#यदि_अन्तः // usb.h
