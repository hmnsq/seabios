// USB functions and data.
#अगर_अघोषित __USB_H
#घोषणा __USB_H

#समावेश "stacks.h" // struct mutex_s

// Information on a USB end point.
काष्ठा usb_pipe {
    जोड़ {
        काष्ठा usb_s *cntl;
        काष्ठा usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 speed;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
काष्ठा usbdevice_s {
    काष्ठा usbhub_s *hub;
    काष्ठा usb_pipe *defpipe;
    u32 port;
    काष्ठा usb_config_descriptor *config;
    काष्ठा usb_interface_descriptor *iface;
    पूर्णांक imax;
    u8 speed;
    u8 devaddr;
};

// Common information for usb controllers.
काष्ठा usb_s {
    काष्ठा usb_pipe *freelist;
    काष्ठा mutex_s resetlock;
    काष्ठा pci_device *pci;
    व्योम *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
काष्ठा usbhub_s {
    काष्ठा usbhub_op_s *op;
    काष्ठा usbdevice_s *usbdev;
    काष्ठा usb_s *cntl;
    काष्ठा mutex_s lock;
    u32 detectend;
    u32 port;
    u32 threads;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
काष्ठा usbhub_op_s {
    पूर्णांक (*detect)(काष्ठा usbhub_s *hub, u32 port);
    पूर्णांक (*reset)(काष्ठा usbhub_s *hub, u32 port);
    पूर्णांक (*portmap)(काष्ठा usbhub_s *hub, u32 port);
    व्योम (*disconnect)(काष्ठा usbhub_s *hub, u32 port);
};

#घोषणा USB_TYPE_UHCI  1
#घोषणा USB_TYPE_OHCI  2
#घोषणा USB_TYPE_EHCI  3
#घोषणा USB_TYPE_XHCI  4

#घोषणा USB_FULLSPEED  0
#घोषणा USB_LOWSPEED   1
#घोषणा USB_HIGHSPEED  2
#घोषणा USB_SUPERSPEED 3

#घोषणा USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#घोषणा USB_TIME_SIGATT 100
#घोषणा USB_TIME_ATTDB  100
#घोषणा USB_TIME_DRST   10
#घोषणा USB_TIME_DRSTR  50
#घोषणा USB_TIME_RSTRCY 10

#घोषणा USB_TIME_STATUS  50
#घोषणा USB_TIME_DATAIN  500
#घोषणा USB_TIME_COMMAND 5000

#घोषणा USB_TIME_SETADDR_RECOVERY 2

#घोषणा USB_PID_OUT                     0xe1
#घोषणा USB_PID_IN                      0x69
#घोषणा USB_PID_SETUP                   0x2d

#घोषणा USB_DIR_OUT                     0               /* to device */
#घोषणा USB_DIR_IN                      0x80            /* to host */

#घोषणा USB_TYPE_MASK                   (0x03 << 5)
#घोषणा USB_TYPE_STANDARD               (0x00 << 5)
#घोषणा USB_TYPE_CLASS                  (0x01 << 5)
#घोषणा USB_TYPE_VENDOR                 (0x02 << 5)
#घोषणा USB_TYPE_RESERVED               (0x03 << 5)

#घोषणा USB_RECIP_MASK                  0x1f
#घोषणा USB_RECIP_DEVICE                0x00
#घोषणा USB_RECIP_INTERFACE             0x01
#घोषणा USB_RECIP_ENDPOINT              0x02
#घोषणा USB_RECIP_OTHER                 0x03

#घोषणा USB_REQ_GET_STATUS              0x00
#घोषणा USB_REQ_CLEAR_FEATURE           0x01
#घोषणा USB_REQ_SET_FEATURE             0x03
#घोषणा USB_REQ_SET_ADDRESS             0x05
#घोषणा USB_REQ_GET_DESCRIPTOR          0x06
#घोषणा USB_REQ_SET_DESCRIPTOR          0x07
#घोषणा USB_REQ_GET_CONFIGURATION       0x08
#घोषणा USB_REQ_SET_CONFIGURATION       0x09
#घोषणा USB_REQ_GET_INTERFACE           0x0A
#घोषणा USB_REQ_SET_INTERFACE           0x0B
#घोषणा USB_REQ_SYNCH_FRAME             0x0C

काष्ठा usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_मूल्य;
    u16 w_सूचकांक;
    u16 w_लंबाई;
} PACKED;

#घोषणा USB_DT_DEVICE                   0x01
#घोषणा USB_DT_CONFIG                   0x02
#घोषणा USB_DT_STRING                   0x03
#घोषणा USB_DT_INTERFACE                0x04
#घोषणा USB_DT_ENDPOINT                 0x05
#घोषणा USB_DT_DEVICE_QUALIFIER         0x06
#घोषणा USB_DT_OTHER_SPEED_CONFIG       0x07
#घोषणा USB_DT_ENDPOINT_COMPANION       0x30

काष्ठा usb_device_descriptor {
    u8  b_लंबाई;
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

#घोषणा USB_CLASS_PER_INTERFACE         0       /* for DeviceClass */
#घोषणा USB_CLASS_AUDIO                 1
#घोषणा USB_CLASS_COMM                  2
#घोषणा USB_CLASS_HID                   3
#घोषणा USB_CLASS_PHYSICAL              5
#घोषणा USB_CLASS_STILL_IMAGE           6
#घोषणा USB_CLASS_PRINTER               7
#घोषणा USB_CLASS_MASS_STORAGE          8
#घोषणा USB_CLASS_HUB                   9

काष्ठा usb_config_descriptor {
    u8  b_लंबाई;
    u8  bDescriptorType;

    u16 w_योग_लंबाई;
    u8  b_अंक_Interfaces;
    u8  b_Configuration_मूल्य;
    u8  iConfiguration;
    u8  bmAttributes;
    u8  bMaxPower;
} PACKED;

काष्ठा usb_interface_descriptor {
    u8  b_लंबाई;
    u8  bDescriptorType;

    u8  b_Interface_संख्या;
    u8  bAlternateSetting;
    u8  b_अंक_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

काष्ठा usb_endpoint_descriptor {
    u8  b_लंबाई;
    u8  bDescriptorType;

    u8  bEndpointAddress;
    u8  bmAttributes;
    u16 w_Max_Packet_आकार;
    u8  bInterval;
} PACKED;

#घोषणा USB_ENDPOINT_संख्या_MASK        0x0f    /* in bEndpointAddress */
#घोषणा USB_ENDPOINT_DIR_MASK           0x80

#घोषणा USB_ENDPOINT_XFERTYPE_MASK      0x03    /* in bmAttributes */
#घोषणा USB_ENDPOINT_XFER_CONTROL       0
#घोषणा USB_ENDPOINT_XFER_ISOC          1
#घोषणा USB_ENDPOINT_XFER_BULK          2
#घोषणा USB_ENDPOINT_XFER_INT           3
#घोषणा USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#घोषणा USB_CONTROL_SETUP_आकार          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#घोषणा US_SC_ATAPI_8020   0x02
#घोषणा US_SC_ATAPI_8070   0x05
#घोषणा US_SC_SCSI         0x06

#घोषणा US_PR_BULK         0x50  /* bulk-only transport */
#घोषणा US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
पूर्णांक usb_send_bulk(काष्ठा usb_pipe *pipe, पूर्णांक dir, व्योम *तथ्य, पूर्णांक datasize);
पूर्णांक usb_poll_intr(काष्ठा usb_pipe *pipe, व्योम *तथ्य);
पूर्णांक usb_32bit_pipe(काष्ठा usb_pipe *pipe_fl);
काष्ठा usb_pipe *usb_alloc_pipe(काष्ठा usbdevice_s *usbdev
                                , काष्ठा usb_endpoint_descriptor *epdesc);
व्योम usb_free_pipe(काष्ठा usbdevice_s *usbdev, काष्ठा usb_pipe *pipe);
पूर्णांक usb_send_default_control(काष्ठा usb_pipe *pipe
                             , स्थिर काष्ठा usb_ctrlrequest *req, व्योम *तथ्य);
पूर्णांक usb_is_freelist(काष्ठा usb_s *cntl, काष्ठा usb_pipe *pipe);
व्योम usb_add_freelist(काष्ठा usb_pipe *pipe);
काष्ठा usb_pipe *usb_get_freelist(काष्ठा usb_s *cntl, u8 eptype);
व्योम usb_desc2pipe(काष्ठा usb_pipe *pipe, काष्ठा usbdevice_s *usbdev
                   , काष्ठा usb_endpoint_descriptor *epdesc);
पूर्णांक usb_get_period(काष्ठा usbdevice_s *usbdev
                   , काष्ठा usb_endpoint_descriptor *epdesc);
पूर्णांक usb_xfer_time(काष्ठा usb_pipe *pipe, पूर्णांक datalen);
काष्ठा usb_endpoint_descriptor *usb_find_desc(काष्ठा usbdevice_s *usbdev
                                              , पूर्णांक type, पूर्णांक dir);
व्योम usb_enumerate(काष्ठा usbhub_s *hub);
व्योम usb_setup(व्योम);

#पूर्ण_अगर // usb.h
