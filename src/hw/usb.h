// USB functions and data.
#جے_بنا_تعریف __USB_H
#تعریف_کرو __USB_H

#ضمن "stacks.h" // struct mutex_s

// Information on a USB end point.
بنية usb_pipe {
    اتحاد {
        بنية usb_s *cntl;
        بنية usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 رفتار;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
بنية usbdevice_s {
    بنية usbhub_s *hub;
    بنية usb_pipe *defpipe;
    u32 پورٹ;
    بنية usb_config_descriptor *config;
    بنية usb_interface_descriptor *iface;
    صحيح imax;
    u8 رفتار;
    u8 devaddr;
};

// Common information for usb controllers.
بنية usb_s {
    بنية usb_pipe *freelist;
    بنية mutex_s resetlock;
    بنية pci_device *pci;
    فراغ *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
بنية usbhub_s {
    بنية usbhub_op_s *op;
    بنية usbdevice_s *usbdev;
    بنية usb_s *cntl;
    بنية mutex_s جندرا;
    u32 detectend;
    u32 پورٹ;
    u32 دھاگے;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
بنية usbhub_op_s {
    صحيح (*detect)(بنية usbhub_s *hub, u32 پورٹ);
    صحيح (*reset)(بنية usbhub_s *hub, u32 پورٹ);
    صحيح (*portmap)(بنية usbhub_s *hub, u32 پورٹ);
    فراغ (*disconnect)(بنية usbhub_s *hub, u32 پورٹ);
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
#تعریف_کرو USB_ویلا_SIGATT 100
#تعریف_کرو USB_ویلا_ATTDB  100
#تعریف_کرو USB_ویلا_DRST   10
#تعریف_کرو USB_ویلا_DRSTR  50
#تعریف_کرو USB_ویلا_RSTRCY 10

#تعریف_کرو USB_ویلا_حالت  50
#تعریف_کرو USB_ویلا_DATAIN  500
#تعریف_کرو USB_ویلا_COMMAND 5000

#تعریف_کرو USB_ویلا_SETADDR_RECOVERY 2

#تعریف_کرو USB_PID_باہرلا                     0xe1
#تعریف_کرو USB_PID_وچ                      0x69
#تعریف_کرو USB_PID_تیاری                   0x2d

#تعریف_کرو USB_DIR_باہرلا                     0               /* to device */
#تعریف_کرو USB_DIR_وچ                      0x80            /* to host */

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

بنية usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_قدر;
    u16 w_اشاریہ;
    u16 w_لمائی;
} PACKED;

#تعریف_کرو USB_DT_DEVICE                   0x01
#تعریف_کرو USB_DT_CONFIG                   0x02
#تعریف_کرو USB_DT_STRING                   0x03
#تعریف_کرو USB_DT_INTERFACE                0x04
#تعریف_کرو USB_DT_ENDPOINT                 0x05
#تعریف_کرو USB_DT_DEVICE_QUALIFIER         0x06
#تعریف_کرو USB_DT_OTHER_رفتار_CONFIG       0x07
#تعریف_کرو USB_DT_ENDPOINT_COMPANION       0x30

بنية usb_device_descriptor {
    u8  b_لمائی;
    u8  bDescriptorType;

    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 id_ضرب_حاصل;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  i_ضرب_حاصل;
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

بنية usb_config_descriptor {
    u8  b_لمائی;
    u8  bDescriptorType;

    u16 w_کل_لمائی;
    u8  b_عدد_Interfaces;
    u8  b_Configuration_قدر;
    u8  iConfiguration;
    u8  bm_گن;
    u8  bMaxPower;
} PACKED;

بنية usb_interface_descriptor {
    u8  b_لمائی;
    u8  bDescriptorType;

    u8  b_Interface_نمبر;
    u8  bAlternateSetting;
    u8  b_عدد_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

بنية usb_endpoint_descriptor {
    u8  b_لمائی;
    u8  bDescriptorType;

    u8  b_Endpoint_پتہ;
    u8  bm_گن;
    u16 w_Max_Packet_ناپ;
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

#تعریف_کرو USB_CONTROL_تیاری_ناپ          8


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
صحيح usb_send_bulk(بنية usb_pipe *pipe, صحيح dir, فراغ *انکڑے, صحيح datasize);
صحيح usb_poll_intr(بنية usb_pipe *pipe, فراغ *انکڑے);
صحيح usb_32bit_pipe(بنية usb_pipe *pipe_fl);
بنية usb_pipe *usb_alloc_pipe(بنية usbdevice_s *usbdev
                                , بنية usb_endpoint_descriptor *epdesc);
فراغ usb_free_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *pipe);
صحيح usb_send_default_control(بنية usb_pipe *pipe
                             , ثابت بنية usb_ctrlrequest *req, فراغ *انکڑے);
صحيح usb_is_freelist(بنية usb_s *cntl, بنية usb_pipe *pipe);
فراغ usb_جوڑو_freelist(بنية usb_pipe *pipe);
بنية usb_pipe *usb_get_freelist(بنية usb_s *cntl, u8 eptype);
فراغ usb_desc2pipe(بنية usb_pipe *pipe, بنية usbdevice_s *usbdev
                   , بنية usb_endpoint_descriptor *epdesc);
صحيح usb_get_period(بنية usbdevice_s *usbdev
                   , بنية usb_endpoint_descriptor *epdesc);
صحيح usb_xfer_ویلا(بنية usb_pipe *pipe, صحيح datalen);
بنية usb_endpoint_descriptor *usb_لبھو_desc(بنية usbdevice_s *usbdev
                                              , صحيح type, صحيح dir);
فراغ usb_enumerate(بنية usbhub_s *hub);
فراغ usb_تیاری(فراغ);

#جے_انت // usb.h
