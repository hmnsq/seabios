// USB functions and data.
#ifndef __USB_H
#define __USB_H

#અન્તર્ભાવ "stacks.h" // struct mutex_s

// Information on a USB end point.
સંરચના usb_pipe {
    સંઘ {
        સંરચના usb_s *cntl;
        સંરચના usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 speed;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
સંરચના usbdevice_s {
    સંરચના usbhub_s *hub;
    સંરચના usb_pipe *defpipe;
    u32 port;
    સંરચના usb_config_descriptor *config;
    સંરચના usb_interface_descriptor *iface;
    પૂર્ણાંક imax;
    u8 speed;
    u8 devaddr;
};

// Common information for usb controllers.
સંરચના usb_s {
    સંરચના usb_pipe *freelist;
    સંરચના mutex_s resetlock;
    સંરચના pci_device *pci;
    શૂન્ય *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
સંરચના usbhub_s {
    સંરચના usbhub_op_s *op;
    સંરચના usbdevice_s *usbdev;
    સંરચના usb_s *cntl;
    સંરચના mutex_s lock;
    u32 detectend;
    u32 port;
    u32 threads;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
સંરચના usbhub_op_s {
    પૂર્ણાંક (*detect)(સંરચના usbhub_s *hub, u32 port);
    પૂર્ણાંક (*reset)(સંરચના usbhub_s *hub, u32 port);
    પૂર્ણાંક (*portmap)(સંરચના usbhub_s *hub, u32 port);
    શૂન્ય (*disconnect)(સંરચના usbhub_s *hub, u32 port);
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

સંરચના usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_કિંમત;
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

સંરચના usb_device_descriptor {
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
    u8  i_Serial_સંખ્યા;
    u8  b_અંક_Configurations;
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

સંરચના usb_config_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u16 wTotalLength;
    u8  b_અંક_Interfaces;
    u8  b_Configuration_કિંમત;
    u8  iConfiguration;
    u8  bmAttributes;
    u8  bMaxPower;
} PACKED;

સંરચના usb_interface_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u8  b_Interface_સંખ્યા;
    u8  bAlternateSetting;
    u8  b_અંક_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

સંરચના usb_endpoint_descriptor {
    u8  bLength;
    u8  bDescriptorType;

    u8  bEndpointAddress;
    u8  bmAttributes;
    u16 wMaxPacketSize;
    u8  bInterval;
} PACKED;

#define USB_ENDPOINT_સંખ્યા_MASK        0x0f    /* in bEndpointAddress */
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
પૂર્ણાંક usb_send_bulk(સંરચના usb_pipe *pipe, પૂર્ણાંક dir, શૂન્ય *data, પૂર્ણાંક datasize);
પૂર્ણાંક usb_poll_intr(સંરચના usb_pipe *pipe, શૂન્ય *data);
પૂર્ણાંક usb_32bit_pipe(સંરચના usb_pipe *pipe_fl);
સંરચના usb_pipe *usb_alloc_pipe(સંરચના usbdevice_s *usbdev
                                , સંરચના usb_endpoint_descriptor *epdesc);
શૂન્ય usb_free_pipe(સંરચના usbdevice_s *usbdev, સંરચના usb_pipe *pipe);
પૂર્ણાંક usb_send_default_control(સંરચના usb_pipe *pipe
                             , અચલ સંરચના usb_ctrlrequest *req, શૂન્ય *data);
પૂર્ણાંક usb_is_freelist(સંરચના usb_s *cntl, સંરચના usb_pipe *pipe);
શૂન્ય usb_add_freelist(સંરચના usb_pipe *pipe);
સંરચના usb_pipe *usb_get_freelist(સંરચના usb_s *cntl, u8 eptype);
શૂન્ય usb_desc2pipe(સંરચના usb_pipe *pipe, સંરચના usbdevice_s *usbdev
                   , સંરચના usb_endpoint_descriptor *epdesc);
પૂર્ણાંક usb_get_period(સંરચના usbdevice_s *usbdev
                   , સંરચના usb_endpoint_descriptor *epdesc);
પૂર્ણાંક usb_xfer_time(સંરચના usb_pipe *pipe, પૂર્ણાંક datalen);
સંરચના usb_endpoint_descriptor *usb_find_desc(સંરચના usbdevice_s *usbdev
                                              , પૂર્ણાંક type, પૂર્ણાંક dir);
શૂન્ય usb_enumerate(સંરચના usbhub_s *hub);
શૂન્ય usb_setup(શૂન્ય);

#endif // usb.h
