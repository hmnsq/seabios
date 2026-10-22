// USB functions and data.
#ifndef __USB_H
#define __USB_H

#अन्तर्भाव "stacks.h" // struct mutex_s

// Information on a USB end point.
संरचना usb_pipe {
    संघ {
        संरचना usb_s *cntl;
        संरचना usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 speed;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
संरचना usbdevice_s {
    संरचना usbhub_s *hub;
    संरचना usb_pipe *defpipe;
    u32 port;
    संरचना usb_config_descriptor *config;
    संरचना usb_interface_descriptor *iface;
    पूर्णांक imax;
    u8 speed;
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
    संरचना mutex_s lock;
    u32 detectend;
    u32 port;
    u32 threads;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
संरचना usbhub_op_s {
    पूर्णांक (*detect)(संरचना usbhub_s *hub, u32 port);
    पूर्णांक (*reset)(संरचना usbhub_s *hub, u32 port);
    पूर्णांक (*portmap)(संरचना usbhub_s *hub, u32 port);
    शून्य (*disconnect)(संरचना usbhub_s *hub, u32 port);
};

#define USB_TYPE_UHCI  1
#define USB_TYPE_OHCI  2
#define USB_TYPE_EHCI  3
#define USB_TYPE_XHCI  4

#define USB_FULLSPEED  0
#define USB_LOWSPEED   1
#define USB_HIGHSPEED  2
#define USB_SUPERSPEED 3

#define USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#define USB_TIME_SIGATT 100
#define USB_TIME_ATTDB  100
#define USB_TIME_DRST   10
#define USB_TIME_DRSTR  50
#define USB_TIME_RSTRCY 10

#define USB_TIME_STATUS  50
#define USB_TIME_DATAIN  500
#define USB_TIME_COMMAND 5000

#define USB_TIME_SETADDR_RECOVERY 2

#define USB_PID_OUT                     0xe1
#define USB_PID_IN                      0x69
#define USB_PID_SETUP                   0x2d

#define USB_DIR_OUT                     0               /* to device */
#define USB_DIR_IN                      0x80            /* to host */

#define USB_TYPE_MASK                   (0x03 << 5)
#define USB_TYPE_STANDARD               (0x00 << 5)
#define USB_TYPE_CLASS                  (0x01 << 5)
#define USB_TYPE_VENDOR                 (0x02 << 5)
#define USB_TYPE_RESERVED               (0x03 << 5)

#define USB_RECIP_MASK                  0x1f
#define USB_RECIP_DEVICE                0x00
#define USB_RECIP_INTERFACE             0x01
#define USB_RECIP_ENDPOINT              0x02
#define USB_RECIP_OTHER                 0x03

#define USB_REQ_GET_STATUS              0x00
#define USB_REQ_CLEAR_FEATURE           0x01
#define USB_REQ_SET_FEATURE             0x03
#define USB_REQ_SET_ADDRESS             0x05
#define USB_REQ_GET_DESCRIPTOR          0x06
#define USB_REQ_SET_DESCRIPTOR          0x07
#define USB_REQ_GET_CONFIGURATION       0x08
#define USB_REQ_SET_CONFIGURATION       0x09
#define USB_REQ_GET_INTERFACE           0x0A
#define USB_REQ_SET_INTERFACE           0x0B
#define USB_REQ_SYNCH_FRAME             0x0C

संरचना usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_मूल्य;
    u16 wIndex;
    u16 wLength;
} PACKED;

#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIG                   0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05
#define USB_DT_DEVICE_QUALIFIER         0x06
#define USB_DT_OTHER_SPEED_CONFIG       0x07
#define USB_DT_ENDPOINT_COMPANION       0x30

संरचना usb_device_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 idProduct;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  iProduct;
    u8  i_Serial_संख्या;
    u8  b_अंक_Configurations;
} PACKED;

#define USB_CLASS_PER_INTERFACE         0       /* for DeviceClass */
#define USB_CLASS_AUDIO                 1
#define USB_CLASS_COMM                  2
#define USB_CLASS_HID                   3
#define USB_CLASS_PHYSICAL              5
#define USB_CLASS_STILL_IMAGE           6
#define USB_CLASS_PRINTER               7
#define USB_CLASS_MASS_STORAGE          8
#define USB_CLASS_HUB                   9

संरचना usb_config_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u16 wTotalLength;
    u8  b_अंक_Interfaces;
    u8  b_Configuration_मूल्य;
    u8  iConfiguration;
    u8  bmAttributes;
    u8  bMaxPower;
} PACKED;

संरचना usb_interface_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u8  b_Interface_संख्या;
    u8  bAlternateSetting;
    u8  b_अंक_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

संरचना usb_endpoint_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u8  bEndpointAddress;
    u8  bmAttributes;
    u16 wMaxPacketSize;
    u8  bInterval;
} PACKED;

#define USB_ENDPOINT_संख्या_MASK        0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK           0x80

#define USB_ENDPOINT_XFERTYPE_MASK      0x03    /* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL       0
#define USB_ENDPOINT_XFER_ISOC          1
#define USB_ENDPOINT_XFER_BULK          2
#define USB_ENDPOINT_XFER_INT           3
#define USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#define USB_CONTROL_SETUP_SIZE          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#define US_SC_ATAPI_8020   0x02
#define US_SC_ATAPI_8070   0x05
#define US_SC_SCSI         0x06

#define US_PR_BULK         0x50  /* bulk-only transport */
#define US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
पूर्णांक usb_send_bulk(संरचना usb_pipe *pipe, पूर्णांक dir, शून्य *data, पूर्णांक datasize);
पूर्णांक usb_poll_intr(संरचना usb_pipe *pipe, शून्य *data);
पूर्णांक usb_32bit_pipe(संरचना usb_pipe *pipe_fl);
संरचना usb_pipe *usb_alloc_pipe(संरचना usbdevice_s *usbdev
                                , संरचना usb_endpoint_descriptor *epdesc);
शून्य usb_free_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *pipe);
पूर्णांक usb_send_default_control(संरचना usb_pipe *pipe
                             , स्थिरांक संरचना usb_ctrlrequest *req, शून्य *data);
पूर्णांक usb_is_freelist(संरचना usb_s *cntl, संरचना usb_pipe *pipe);
शून्य usb_add_freelist(संरचना usb_pipe *pipe);
संरचना usb_pipe *usb_get_freelist(संरचना usb_s *cntl, u8 eptype);
शून्य usb_desc2pipe(संरचना usb_pipe *pipe, संरचना usbdevice_s *usbdev
                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णांक usb_get_period(संरचना usbdevice_s *usbdev
                   , संरचना usb_endpoint_descriptor *epdesc);
पूर्णांक usb_xfer_time(संरचना usb_pipe *pipe, पूर्णांक datalen);
संरचना usb_endpoint_descriptor *usb_find_desc(संरचना usbdevice_s *usbdev
                                              , पूर्णांक type, पूर्णांक dir);
शून्य usb_enumerate(संरचना usbhub_s *hub);
शून्य usb_setup(शून्य);

#endif // usb.h
