// USB functions and data.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USB_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USB_H

#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // struct mutex_s

// Information on a USB end point.
ਬਣਤਰ usb_pipe {
    ਸੰਘ {
        ਬਣਤਰ usb_s *cntl;
        ਬਣਤਰ usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 ਰਫਤਾਰ;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
ਬਣਤਰ usbdevice_s {
    ਬਣਤਰ usbhub_s *hub;
    ਬਣਤਰ usb_pipe *defpipe;
    u32 ਪੋਰਟ;
    ਬਣਤਰ usb_config_descriptor *config;
    ਬਣਤਰ usb_interface_descriptor *iface;
    ਪੂਰਨ imax;
    u8 ਰਫਤਾਰ;
    u8 devaddr;
};

// Common information for usb controllers.
ਬਣਤਰ usb_s {
    ਬਣਤਰ usb_pipe *freelist;
    ਬਣਤਰ mutex_s resetlock;
    ਬਣਤਰ pci_device *pci;
    ਖਾਲੀ *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
ਬਣਤਰ usbhub_s {
    ਬਣਤਰ usbhub_op_s *op;
    ਬਣਤਰ usbdevice_s *usbdev;
    ਬਣਤਰ usb_s *cntl;
    ਬਣਤਰ mutex_s ਤਾਲਾ;
    u32 detectend;
    u32 ਪੋਰਟ;
    u32 ਧਾਗੇ;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
ਬਣਤਰ usbhub_op_s {
    ਪੂਰਨ (*detect)(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ);
    ਪੂਰਨ (*reset)(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ);
    ਪੂਰਨ (*portmap)(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ);
    ਖਾਲੀ (*disconnect)(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ);
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_UHCI  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_OHCI  2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_EHCI  3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_XHCI  4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_FULLSPEED  0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_LOWSPEED   1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_HIGHSPEED  2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_SUPERSPEED 3

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_SIGATT 100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_ATTDB  100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_DRST   10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_DRSTR  50
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_RSTRCY 10

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_ਸਥਿਤੀ  50
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_DATAIN  500
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_COMMAND 5000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ਸਮਾਂ_SETADDR_RECOVERY 2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_PID_ਬਾਹਰਲਾ                     0xe1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_PID_ਵਿੱਚ                      0x69
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_PID_ਤਿਆਰੀ                   0x2d

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DIR_ਬਾਹਰਲਾ                     0               /* to device */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DIR_ਵਿੱਚ                      0x80            /* to host */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_ਨਕਾਬ                   (0x03 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_STANDARD               (0x00 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_CLASS                  (0x01 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_VENDOR                 (0x02 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TYPE_RESERVED               (0x03 << 5)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_RECIP_ਨਕਾਬ                  0x1f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_RECIP_DEVICE                0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_RECIP_INTERFACE             0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_RECIP_ENDPOINT              0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_RECIP_OTHER                 0x03

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_GET_ਸਥਿਤੀ              0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_CLEAR_FEATURE           0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_ਸੈੱਟ_FEATURE             0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_ਸੈੱਟ_ਪਤਾ             0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_GET_DESCRIPTOR          0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_ਸੈੱਟ_DESCRIPTOR          0x07
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_GET_CONFIGURATION       0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_ਸੈੱਟ_CONFIGURATION       0x09
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_GET_INTERFACE           0x0A
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_ਸੈੱਟ_INTERFACE           0x0B
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_REQ_SYNCH_ਚੌਖਟਾ             0x0C

ਬਣਤਰ usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_ਮੁੱਲ;
    u16 w_ਸੂਚਕਾਂਕ;
    u16 w_ਲੰਮਾਈ;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_DEVICE                   0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_CONFIG                   0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_STRING                   0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_INTERFACE                0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_ENDPOINT                 0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_DEVICE_QUALIFIER         0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_OTHER_ਰਫਤਾਰ_CONFIG       0x07
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DT_ENDPOINT_COMPANION       0x30

ਬਣਤਰ usb_device_descriptor {
    u8  b_ਲੰਮਾਈ;
    u8  bDescriptorType;

    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 id_ਗੁਣਨਫਲ;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  i_ਗੁਣਨਫਲ;
    u8  i_Serial_ਸੰਖਿਆ;
    u8  b_ਅੰਕ_Configurations;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_ਪ੍ਰਤੀ_INTERFACE         0       /* for DeviceClass */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_AUDIO                 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_COMM                  2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_HID                   3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_PHYSICAL              5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_STILL_IMAGE           6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_PRINTER               7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_MASS_STORAGE          8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CLASS_HUB                   9

ਬਣਤਰ usb_config_descriptor {
    u8  b_ਲੰਮਾਈ;
    u8  bDescriptorType;

    u16 w_ਕੁੱਲ_ਲੰਮਾਈ;
    u8  b_ਅੰਕ_Interfaces;
    u8  b_Configuration_ਮੁੱਲ;
    u8  iConfiguration;
    u8  bm_ਗੁਣ;
    u8  bMaxPower;
} PACKED;

ਬਣਤਰ usb_interface_descriptor {
    u8  b_ਲੰਮਾਈ;
    u8  bDescriptorType;

    u8  b_Interface_ਸੰਖਿਆ;
    u8  bAlternateSetting;
    u8  b_ਅੰਕ_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

ਬਣਤਰ usb_endpoint_descriptor {
    u8  b_ਲੰਮਾਈ;
    u8  bDescriptorType;

    u8  b_Endpoint_ਪਤਾ;
    u8  bm_ਗੁਣ;
    u16 w_Max_Packet_ਮਾਪ;
    u8  bInterval;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_ਸੰਖਿਆ_ਨਕਾਬ        0x0f    /* in bEndpointAddress */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_DIR_ਨਕਾਬ           0x80

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_XFERTYPE_ਨਕਾਬ      0x03    /* in bmAttributes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_XFER_CONTROL       0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_XFER_ISOC          1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_XFER_BULK          2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_XFER_INT           3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CONTROL_ਤਿਆਰੀ_ਮਾਪ          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ US_SC_ATAPI_8020   0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ US_SC_ATAPI_8070   0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ US_SC_SCSI         0x06

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ US_PR_BULK         0x50  /* bulk-only transport */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
ਪੂਰਨ usb_send_bulk(ਬਣਤਰ usb_pipe *pipe, ਪੂਰਨ dir, ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize);
ਪੂਰਨ usb_poll_intr(ਬਣਤਰ usb_pipe *pipe, ਖਾਲੀ *ਅੰਕੜੇ);
ਪੂਰਨ usb_32bit_pipe(ਬਣਤਰ usb_pipe *pipe_fl);
ਬਣਤਰ usb_pipe *usb_alloc_pipe(ਬਣਤਰ usbdevice_s *usbdev
                                , ਬਣਤਰ usb_endpoint_descriptor *epdesc);
ਖਾਲੀ usb_free_pipe(ਬਣਤਰ usbdevice_s *usbdev, ਬਣਤਰ usb_pipe *pipe);
ਪੂਰਨ usb_send_default_control(ਬਣਤਰ usb_pipe *pipe
                             , ਅਚਲ ਬਣਤਰ usb_ctrlrequest *req, ਖਾਲੀ *ਅੰਕੜੇ);
ਪੂਰਨ usb_is_freelist(ਬਣਤਰ usb_s *cntl, ਬਣਤਰ usb_pipe *pipe);
ਖਾਲੀ usb_ਜੋੜੋ_freelist(ਬਣਤਰ usb_pipe *pipe);
ਬਣਤਰ usb_pipe *usb_get_freelist(ਬਣਤਰ usb_s *cntl, u8 eptype);
ਖਾਲੀ usb_desc2pipe(ਬਣਤਰ usb_pipe *pipe, ਬਣਤਰ usbdevice_s *usbdev
                   , ਬਣਤਰ usb_endpoint_descriptor *epdesc);
ਪੂਰਨ usb_get_period(ਬਣਤਰ usbdevice_s *usbdev
                   , ਬਣਤਰ usb_endpoint_descriptor *epdesc);
ਪੂਰਨ usb_xfer_ਸਮਾਂ(ਬਣਤਰ usb_pipe *pipe, ਪੂਰਨ datalen);
ਬਣਤਰ usb_endpoint_descriptor *usb_ਲੱਭੋ_desc(ਬਣਤਰ usbdevice_s *usbdev
                                              , ਪੂਰਨ type, ਪੂਰਨ dir);
ਖਾਲੀ usb_enumerate(ਬਣਤਰ usbhub_s *hub);
ਖਾਲੀ usb_ਤਿਆਰੀ(ਖਾਲੀ);

#ਜੇ_ਅੰਤ // usb.h
