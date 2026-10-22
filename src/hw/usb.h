// USB functions and data.
#اگر_غیر_تعریف_شدہ __USB_H
#تعریف_کرو __USB_H

#شامل "stacks.h" // struct mutex_s

// Information on a USB end point.
ساخت usb_pipe {
    اتحاد {
        ساخت usb_s *cntl;
        ساخت usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 رفتار;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
ساخت usbdevice_s {
    ساخت usbhub_s *hub;
    ساخت usb_pipe *defpipe;
    u32 پورٹ;
    ساخت usb_config_descriptor *config;
    ساخت usb_interface_descriptor *iface;
    صحیح imax;
    u8 رفتار;
    u8 devaddr;
};

// Common information for usb controllers.
ساخت usb_s {
    ساخت usb_pipe *freelist;
    ساخت mutex_s resetlock;
    ساخت pci_device *pci;
    خالی *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
ساخت usbhub_s {
    ساخت usbhub_op_s *op;
    ساخت usbdevice_s *usbdev;
    ساخت usb_s *cntl;
    ساخت mutex_s تالا_2;
    u32 detectend;
    u32 پورٹ;
    u32 دھاگے;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
ساخت usbhub_op_s {
    صحیح (*detect)(ساخت usbhub_s *hub, u32 پورٹ);
    صحیح (*reset)(ساخت usbhub_s *hub, u32 پورٹ);
    صحیح (*portmap)(ساخت usbhub_s *hub, u32 پورٹ);
    خالی (*disconnect)(ساخت usbhub_s *hub, u32 پورٹ);
};

#تعریف_کرو USB_TYPE_UHCI  1
#تعریف_کرو USB_TYPE_OHCI  2
#تعریف_کرو USB_TYPE_EHCI  3
#تعریف_کرو USB_TYPE_XHCI  4

#تعریف_کرو USB_FULLSPEED  0
#تعریف_کرو USB_LOWSPEED   1
#تعریف_کرو USB_HIGHSPEED  2
#تعریف_کرو USB_SUPERSPEED 3

#تعریف_کرو USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#تعریف_کرو USB_وقت_SIGATT 100
#تعریف_کرو USB_وقت_ATTDB  100
#تعریف_کرو USB_وقت_DRST   10
#تعریف_کرو USB_وقت_DRSTR  50
#تعریف_کرو USB_وقت_RSTRCY 10

#تعریف_کرو USB_وقت_حالت  50
#تعریف_کرو USB_وقت_DATAIN  500
#تعریف_کرو USB_وقت_COMMAND 5000

#تعریف_کرو USB_وقت_SETADDR_RECOVERY 2

#تعریف_کرو USB_PID_برآمد                     0xe1
#تعریف_کرو USB_PID_اندر                      0x69
#تعریف_کرو USB_PID_تیاری                   0x2d

#تعریف_کرو USB_DIR_برآمد                     0               /* to device */
#تعریف_کرو USB_DIR_اندر                      0x80            /* to host */

#تعریف_کرو USB_TYPE_نقاب                   (0x03 << 5)
#تعریف_کرو USB_TYPE_STANDARD               (0x00 << 5)
#تعریف_کرو USB_TYPE_CLASS                  (0x01 << 5)
#تعریف_کرو USB_TYPE_VENDOR                 (0x02 << 5)
#تعریف_کرو USB_TYPE_RESERVED               (0x03 << 5)

#تعریف_کرو USB_RECIP_نقاب                  0x1f
#تعریف_کرو USB_RECIP_DEVICE                0x00
#تعریف_کرو USB_RECIP_INTERFACE             0x01
#تعریف_کرو USB_RECIP_ENDPOINT              0x02
#تعریف_کرو USB_RECIP_OTHER                 0x03

#تعریف_کرو USB_REQ_GET_حالت              0x00
#تعریف_کرو USB_REQ_CLEAR_FEATURE           0x01
#تعریف_کرو USB_REQ_سیٹ_FEATURE             0x03
#تعریف_کرو USB_REQ_سیٹ_پتہ             0x05
#تعریف_کرو USB_REQ_GET_DESCRIPTOR          0x06
#تعریف_کرو USB_REQ_سیٹ_DESCRIPTOR          0x07
#تعریف_کرو USB_REQ_GET_CONFIGURATION       0x08
#تعریف_کرو USB_REQ_سیٹ_CONFIGURATION       0x09
#تعریف_کرو USB_REQ_GET_INTERFACE           0x0A
#تعریف_کرو USB_REQ_سیٹ_INTERFACE           0x0B
#تعریف_کرو USB_REQ_SYNCH_چوکھٹا             0x0C

ساخت usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_قدر;
    u16 w_اشاریہ;
    u16 w_لمبائی;
} PACKED;

#تعریف_کرو USB_DT_DEVICE                   0x01
#تعریف_کرو USB_DT_CONFIG                   0x02
#تعریف_کرو USB_DT_STRING                   0x03
#تعریف_کرو USB_DT_INTERFACE                0x04
#تعریف_کرو USB_DT_ENDPOINT                 0x05
#تعریف_کرو USB_DT_DEVICE_QUALIFIER         0x06
#تعریف_کرو USB_DT_OTHER_رفتار_CONFIG       0x07
#تعریف_کرو USB_DT_ENDPOINT_COMPANION       0x30

ساخت usb_device_descriptor {
    u8  b_لمبائی;
    u8  bDescriptorType;

    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 id_حاصل_ضرب;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  i_حاصل_ضرب;
    u8  i_Serial_نمبر;
    u8  b_عدد_Configurations;
} PACKED;

#تعریف_کرو USB_CLASS_فی_INTERFACE         0       /* for DeviceClass */
#تعریف_کرو USB_CLASS_AUDIO                 1
#تعریف_کرو USB_CLASS_COMM                  2
#تعریف_کرو USB_CLASS_HID                   3
#تعریف_کرو USB_CLASS_PHYSICAL              5
#تعریف_کرو USB_CLASS_STILL_IMAGE           6
#تعریف_کرو USB_CLASS_PRINTER               7
#تعریف_کرو USB_CLASS_MASS_STORAGE          8
#تعریف_کرو USB_CLASS_HUB                   9

ساخت usb_config_descriptor {
    u8  b_لمبائی;
    u8  bDescriptorType;

    u16 w_کل_لمبائی;
    u8  b_عدد_Interfaces;
    u8  b_Configuration_قدر;
    u8  iConfiguration;
    u8  bm_اوصاف;
    u8  bMaxPower;
} PACKED;

ساخت usb_interface_descriptor {
    u8  b_لمبائی;
    u8  bDescriptorType;

    u8  b_Interface_نمبر;
    u8  bAlternateSetting;
    u8  b_عدد_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

ساخت usb_endpoint_descriptor {
    u8  b_لمبائی;
    u8  bDescriptorType;

    u8  b_Endpoint_پتہ;
    u8  bm_اوصاف;
    u16 w_Max_Packet_سائز;
    u8  bInterval;
} PACKED;

#تعریف_کرو USB_ENDPOINT_نمبر_نقاب        0x0f    /* in bEndpointAddress */
#تعریف_کرو USB_ENDPOINT_DIR_نقاب           0x80

#تعریف_کرو USB_ENDPOINT_XFERTYPE_نقاب      0x03    /* in bmAttributes */
#تعریف_کرو USB_ENDPOINT_XFER_CONTROL       0
#تعریف_کرو USB_ENDPOINT_XFER_ISOC          1
#تعریف_کرو USB_ENDPOINT_XFER_BULK          2
#تعریف_کرو USB_ENDPOINT_XFER_INT           3
#تعریف_کرو USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#تعریف_کرو USB_CONTROL_تیاری_سائز          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#تعریف_کرو US_SC_ATAPI_8020   0x02
#تعریف_کرو US_SC_ATAPI_8070   0x05
#تعریف_کرو US_SC_SCSI         0x06

#تعریف_کرو US_PR_BULK         0x50  /* bulk-only transport */
#تعریف_کرو US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
صحیح usb_send_bulk(ساخت usb_pipe *pipe, صحیح dir, خالی *ڈیٹا, صحیح datasize);
صحیح usb_poll_intr(ساخت usb_pipe *pipe, خالی *ڈیٹا);
صحیح usb_32bit_pipe(ساخت usb_pipe *pipe_fl);
ساخت usb_pipe *usb_alloc_pipe(ساخت usbdevice_s *usbdev
                                , ساخت usb_endpoint_descriptor *epdesc);
خالی usb_free_pipe(ساخت usbdevice_s *usbdev, ساخت usb_pipe *pipe);
صحیح usb_send_default_control(ساخت usb_pipe *pipe
                             , ثابت ساخت usb_ctrlrequest *req, خالی *ڈیٹا);
صحیح usb_is_freelist(ساخت usb_s *cntl, ساخت usb_pipe *pipe);
خالی usb_جوڑو_freelist(ساخت usb_pipe *pipe);
ساخت usb_pipe *usb_get_freelist(ساخت usb_s *cntl, u8 eptype);
خالی usb_desc2pipe(ساخت usb_pipe *pipe, ساخت usbdevice_s *usbdev
                   , ساخت usb_endpoint_descriptor *epdesc);
صحیح usb_get_period(ساخت usbdevice_s *usbdev
                   , ساخت usb_endpoint_descriptor *epdesc);
صحیح usb_xfer_وقت(ساخت usb_pipe *pipe, صحیح datalen);
ساخت usb_endpoint_descriptor *usb_ڈھونڈو_desc(ساخت usbdevice_s *usbdev
                                              , صحیح type, صحیح dir);
خالی usb_enumerate(ساخت usbhub_s *hub);
خالی usb_تیاری(خالی);

#اگر_ختم // usb.h
