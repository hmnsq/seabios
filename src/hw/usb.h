// USB functions and data.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USB_H
#ವ್ಯಾಖ್ಯಾನಿಸು __USB_H

#ಅನ್ತರ್ಭಾವ "stacks.h" // struct mutex_s

// Information on a USB end point.
ರಚನೆ usb_pipe {
    ಒಕ್ಕೂಟ {
        ರಚನೆ usb_s *cntl;
        ರಚನೆ usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 ವೇಗ;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
ರಚನೆ usbdevice_s {
    ರಚನೆ usbhub_s *hub;
    ರಚನೆ usb_pipe *defpipe;
    u32 ಪೋರ್ಟ್;
    ರಚನೆ usb_config_descriptor *config;
    ರಚನೆ usb_interface_descriptor *iface;
    ಪೂರ್ಣಾಂಕ imax;
    u8 ವೇಗ;
    u8 devaddr;
};

// Common information for usb controllers.
ರಚನೆ usb_s {
    ರಚನೆ usb_pipe *freelist;
    ರಚನೆ mutex_s resetlock;
    ರಚನೆ pci_device *pci;
    ಶೂನ್ಯ *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
ರಚನೆ usbhub_s {
    ರಚನೆ usbhub_op_s *op;
    ರಚನೆ usbdevice_s *usbdev;
    ರಚನೆ usb_s *cntl;
    ರಚನೆ mutex_s ಬೀಗ_2;
    u32 detectend;
    u32 ಪೋರ್ಟ್;
    u32 ಎಳೆಗಳು;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
ರಚನೆ usbhub_op_s {
    ಪೂರ್ಣಾಂಕ (*detect)(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್);
    ಪೂರ್ಣಾಂಕ (*reset)(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್);
    ಪೂರ್ಣಾಂಕ (*portmap)(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್);
    ಶೂನ್ಯ (*disconnect)(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್);
};

#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_UHCI  1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_OHCI  2
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_EHCI  3
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_XHCI  4

#ವ್ಯಾಖ್ಯಾನಿಸು USB_FULLSPEED  0
#ವ್ಯಾಖ್ಯಾನಿಸು USB_LOWSPEED   1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_HIGHSPEED  2
#ವ್ಯಾಖ್ಯಾನಿಸು USB_SUPERSPEED 3

#ವ್ಯಾಖ್ಯಾನಿಸು USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_SIGATT 100
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_ATTDB  100
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_DRST   10
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_DRSTR  50
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_RSTRCY 10

#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_ಸ್ಥಿತಿ  50
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_DATAIN  500
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_COMMAND 5000

#ವ್ಯಾಖ್ಯಾನಿಸು USB_ಸಮಯ_SETADDR_RECOVERY 2

#ವ್ಯಾಖ್ಯಾನಿಸು USB_PID_ಹೊರಹೋಗು                     0xe1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_PID_ಒಳ                      0x69
#ವ್ಯಾಖ್ಯಾನಿಸು USB_PID_ಸಿದ್ಧತೆ                   0x2d

#ವ್ಯಾಖ್ಯಾನಿಸು USB_DIR_ಹೊರಹೋಗು                     0               /* to device */
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DIR_ಒಳ                      0x80            /* to host */

#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_ಮುಖವಾಡ                   (0x03 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_STANDARD               (0x00 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_CLASS                  (0x01 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_VENDOR                 (0x02 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TYPE_RESERVED               (0x03 << 5)

#ವ್ಯಾಖ್ಯಾನಿಸು USB_RECIP_ಮುಖವಾಡ                  0x1f
#ವ್ಯಾಖ್ಯಾನಿಸು USB_RECIP_DEVICE                0x00
#ವ್ಯಾಖ್ಯಾನಿಸು USB_RECIP_INTERFACE             0x01
#ವ್ಯಾಖ್ಯಾನಿಸು USB_RECIP_ENDPOINT              0x02
#ವ್ಯಾಖ್ಯಾನಿಸು USB_RECIP_OTHER                 0x03

#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_GET_ಸ್ಥಿತಿ              0x00
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_CLEAR_FEATURE           0x01
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_ಗಣ_FEATURE             0x03
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_ಗಣ_ವಿಳಾಸ             0x05
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_GET_DESCRIPTOR          0x06
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_ಗಣ_DESCRIPTOR          0x07
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_GET_CONFIGURATION       0x08
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_ಗಣ_CONFIGURATION       0x09
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_GET_INTERFACE           0x0A
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_ಗಣ_INTERFACE           0x0B
#ವ್ಯಾಖ್ಯಾನಿಸು USB_REQ_SYNCH_ಚೌಕಟ್ಟು             0x0C

ರಚನೆ usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_ಮೌಲ್ಯ;
    u16 w_ಸೂಚ್ಯಂಕ;
    u16 w_ಉದ್ದಳತೆ;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_DEVICE                   0x01
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_CONFIG                   0x02
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_STRING                   0x03
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_INTERFACE                0x04
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_ENDPOINT                 0x05
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_DEVICE_QUALIFIER         0x06
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_OTHER_ವೇಗ_CONFIG       0x07
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DT_ENDPOINT_COMPANION       0x30

ರಚನೆ usb_device_descriptor {
    u8  b_ಉದ್ದಳತೆ;
    u8  bDescriptorType;

    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 id_ಗುಣಲಬ್ಧ;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  i_ಗುಣಲಬ್ಧ;
    u8  i_Serial_ಸಂಖ್ಯೆ;
    u8  b_ಅಂಕೆ_Configurations;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_ಪ್ರತಿ_INTERFACE         0       /* for DeviceClass */
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_AUDIO                 1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_COMM                  2
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_HID                   3
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_PHYSICAL              5
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_STILL_IMAGE           6
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_PRINTER               7
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_MASS_STORAGE          8
#ವ್ಯಾಖ್ಯಾನಿಸು USB_CLASS_HUB                   9

ರಚನೆ usb_config_descriptor {
    u8  b_ಉದ್ದಳತೆ;
    u8  bDescriptorType;

    u16 w_ಒಟ್ಟು_ಉದ್ದಳತೆ;
    u8  b_ಅಂಕೆ_Interfaces;
    u8  b_Configuration_ಮೌಲ್ಯ;
    u8  iConfiguration;
    u8  bm_ಗುಣಲಕ್ಷಣಗಳು;
    u8  bMaxPower;
} PACKED;

ರಚನೆ usb_interface_descriptor {
    u8  b_ಉದ್ದಳತೆ;
    u8  bDescriptorType;

    u8  b_Interface_ಸಂಖ್ಯೆ;
    u8  bAlternateSetting;
    u8  b_ಅಂಕೆ_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

ರಚನೆ usb_endpoint_descriptor {
    u8  b_ಉದ್ದಳತೆ;
    u8  bDescriptorType;

    u8  b_Endpoint_ವಿಳಾಸ;
    u8  bm_ಗುಣಲಕ್ಷಣಗಳು;
    u16 w_Max_Packet_ಗಾತ್ರ;
    u8  bInterval;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_ಸಂಖ್ಯೆ_ಮುಖವಾಡ        0x0f    /* in bEndpointAddress */
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_DIR_ಮುಖವಾಡ           0x80

#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ      0x03    /* in bmAttributes */
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_XFER_CONTROL       0
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_XFER_ISOC          1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_XFER_BULK          2
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_XFER_INT           3
#ವ್ಯಾಖ್ಯಾನಿಸು USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#ವ್ಯಾಖ್ಯಾನಿಸು USB_CONTROL_ಸಿದ್ಧತೆ_ಗಾತ್ರ          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು US_SC_ATAPI_8020   0x02
#ವ್ಯಾಖ್ಯಾನಿಸು US_SC_ATAPI_8070   0x05
#ವ್ಯಾಖ್ಯಾನಿಸು US_SC_SCSI         0x06

#ವ್ಯಾಖ್ಯಾನಿಸು US_PR_BULK         0x50  /* bulk-only transport */
#ವ್ಯಾಖ್ಯಾನಿಸು US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
ಪೂರ್ಣಾಂಕ usb_send_bulk(ರಚನೆ usb_pipe *pipe, ಪೂರ್ಣಾಂಕ dir, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize);
ಪೂರ್ಣಾಂಕ usb_poll_intr(ರಚನೆ usb_pipe *pipe, ಶೂನ್ಯ *ದತ್ತಾಂಶ);
ಪೂರ್ಣಾಂಕ usb_32bit_pipe(ರಚನೆ usb_pipe *pipe_fl);
ರಚನೆ usb_pipe *usb_alloc_pipe(ರಚನೆ usbdevice_s *usbdev
                                , ರಚನೆ usb_endpoint_descriptor *epdesc);
ಶೂನ್ಯ usb_free_pipe(ರಚನೆ usbdevice_s *usbdev, ರಚನೆ usb_pipe *pipe);
ಪೂರ್ಣಾಂಕ usb_send_default_control(ರಚನೆ usb_pipe *pipe
                             , ಸ್ಥಿರಾಂಕ ರಚನೆ usb_ctrlrequest *req, ಶೂನ್ಯ *ದತ್ತಾಂಶ);
ಪೂರ್ಣಾಂಕ usb_is_freelist(ರಚನೆ usb_s *cntl, ರಚನೆ usb_pipe *pipe);
ಶೂನ್ಯ usb_ಕೂಡಿಸು_freelist(ರಚನೆ usb_pipe *pipe);
ರಚನೆ usb_pipe *usb_get_freelist(ರಚನೆ usb_s *cntl, u8 eptype);
ಶೂನ್ಯ usb_desc2pipe(ರಚನೆ usb_pipe *pipe, ರಚನೆ usbdevice_s *usbdev
                   , ರಚನೆ usb_endpoint_descriptor *epdesc);
ಪೂರ್ಣಾಂಕ usb_get_period(ರಚನೆ usbdevice_s *usbdev
                   , ರಚನೆ usb_endpoint_descriptor *epdesc);
ಪೂರ್ಣಾಂಕ usb_xfer_ಸಮಯ(ರಚನೆ usb_pipe *pipe, ಪೂರ್ಣಾಂಕ datalen);
ರಚನೆ usb_endpoint_descriptor *usb_ಕಂಡುಹಿಡಿ_desc(ರಚನೆ usbdevice_s *usbdev
                                              , ಪೂರ್ಣಾಂಕ type, ಪೂರ್ಣಾಂಕ dir);
ಶೂನ್ಯ usb_enumerate(ರಚನೆ usbhub_s *hub);
ಶೂನ್ಯ usb_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ // usb.h
