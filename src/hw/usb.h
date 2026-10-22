// USB functions and data.
#ifndef __USB_H
#define __USB_H

#অন্তর্ভাৱ "stacks.h" // struct mutex_s

// Information on a USB end point.
সংরচনা usb_pipe {
    সঙ্ঘ {
        সংরচনা usb_s *cntl;
        সংরচনা usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 speed;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
সংরচনা usbdevice_s {
    সংরচনা usbhub_s *hub;
    সংরচনা usb_pipe *defpipe;
    u32 port;
    সংরচনা usb_config_descriptor *config;
    সংরচনা usb_interface_descriptor *iface;
    পূর্ণাঙ্ক imax;
    u8 speed;
    u8 devaddr;
};

// Common information for usb controllers.
সংরচনা usb_s {
    সংরচনা usb_pipe *freelist;
    সংরচনা mutex_s resetlock;
    সংরচনা pci_device *pci;
    শূন্য *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
সংরচনা usbhub_s {
    সংরচনা usbhub_op_s *op;
    সংরচনা usbdevice_s *usbdev;
    সংরচনা usb_s *cntl;
    সংরচনা mutex_s lock;
    u32 detectend;
    u32 port;
    u32 threads;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
সংরচনা usbhub_op_s {
    পূর্ণাঙ্ক (*detect)(সংরচনা usbhub_s *hub, u32 port);
    পূর্ণাঙ্ক (*reset)(সংরচনা usbhub_s *hub, u32 port);
    পূর্ণাঙ্ক (*portmap)(সংরচনা usbhub_s *hub, u32 port);
    শূন্য (*disconnect)(সংরচনা usbhub_s *hub, u32 port);
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

সংরচনা usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_মান;
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

সংরচনা usb_device_descriptor {
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
    u8  i_Serial_সংখ্যা;
    u8  b_অঙ্ক_Configurations;
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

সংরচনা usb_config_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u16 wTotalLength;
    u8  b_অঙ্ক_Interfaces;
    u8  b_Configuration_মান;
    u8  iConfiguration;
    u8  bmAttributes;
    u8  bMaxPower;
} PACKED;

সংরচনা usb_interface_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u8  b_Interface_সংখ্যা;
    u8  bAlternateSetting;
    u8  b_অঙ্ক_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

সংরচনা usb_endpoint_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u8  bEndpointAddress;
    u8  bmAttributes;
    u16 wMaxPacketSize;
    u8  bInterval;
} PACKED;

#define USB_ENDPOINT_সংখ্যা_MASK        0x0f    /* in bEndpointAddress */
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
পূর্ণাঙ্ক usb_send_bulk(সংরচনা usb_pipe *pipe, পূর্ণাঙ্ক dir, শূন্য *data, পূর্ণাঙ্ক datasize);
পূর্ণাঙ্ক usb_poll_intr(সংরচনা usb_pipe *pipe, শূন্য *data);
পূর্ণাঙ্ক usb_32bit_pipe(সংরচনা usb_pipe *pipe_fl);
সংরচনা usb_pipe *usb_alloc_pipe(সংরচনা usbdevice_s *usbdev
                                , সংরচনা usb_endpoint_descriptor *epdesc);
শূন্য usb_free_pipe(সংরচনা usbdevice_s *usbdev, সংরচনা usb_pipe *pipe);
পূর্ণাঙ্ক usb_send_default_control(সংরচনা usb_pipe *pipe
                             , ধ্রুৱ সংরচনা usb_ctrlrequest *req, শূন্য *data);
পূর্ণাঙ্ক usb_is_freelist(সংরচনা usb_s *cntl, সংরচনা usb_pipe *pipe);
শূন্য usb_add_freelist(সংরচনা usb_pipe *pipe);
সংরচনা usb_pipe *usb_get_freelist(সংরচনা usb_s *cntl, u8 eptype);
শূন্য usb_desc2pipe(সংরচনা usb_pipe *pipe, সংরচনা usbdevice_s *usbdev
                   , সংরচনা usb_endpoint_descriptor *epdesc);
পূর্ণাঙ্ক usb_get_period(সংরচনা usbdevice_s *usbdev
                   , সংরচনা usb_endpoint_descriptor *epdesc);
পূর্ণাঙ্ক usb_xfer_time(সংরচনা usb_pipe *pipe, পূর্ণাঙ্ক datalen);
সংরচনা usb_endpoint_descriptor *usb_find_desc(সংরচনা usbdevice_s *usbdev
                                              , পূর্ণাঙ্ক type, পূর্ণাঙ্ক dir);
শূন্য usb_enumerate(সংরচনা usbhub_s *hub);
শূন্য usb_setup(শূন্য);

#endif // usb.h
