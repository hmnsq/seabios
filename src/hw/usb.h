// USB functions and data.
#جيڪڏهن_وصف_نه_ٿيل __USB_H
#وصف_ڪريو __USB_H

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
    u32 پورٽ;
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
    بنية mutex_s تالو;
    u32 detectend;
    u32 پورٽ;
    u32 ڌاڳا;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
بنية usbhub_op_s {
    صحيح (*detect)(بنية usbhub_s *hub, u32 پورٽ);
    صحيح (*reset)(بنية usbhub_s *hub, u32 پورٽ);
    صحيح (*portmap)(بنية usbhub_s *hub, u32 پورٽ);
    فراغ (*disconnect)(بنية usbhub_s *hub, u32 پورٽ);
};

#وصف_ڪريو USB_TYPE_UHCI  1
#وصف_ڪريو USB_TYPE_OHCI  2
#وصف_ڪريو USB_TYPE_EHCI  3
#وصف_ڪريو USB_TYPE_XHCI  4

#وصف_ڪريو USB_FULLSPEED  0
#وصف_ڪريو USB_LOWSPEED   1
#وصف_ڪريو USB_HIGHSPEED  2
#وصف_ڪريو USB_SUPERSPEED 3

#وصف_ڪريو USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#وصف_ڪريو USB_وقت_SIGATT 100
#وصف_ڪريو USB_وقت_ATTDB  100
#وصف_ڪريو USB_وقت_DRST   10
#وصف_ڪريو USB_وقت_DRSTR  50
#وصف_ڪريو USB_وقت_RSTRCY 10

#وصف_ڪريو USB_وقت_حالت  50
#وصف_ڪريو USB_وقت_DATAIN  500
#وصف_ڪريو USB_وقت_COMMAND 5000

#وصف_ڪريو USB_وقت_SETADDR_RECOVERY 2

#وصف_ڪريو USB_PID_ٻاهريون                     0xe1
#وصف_ڪريو USB_PID_اندريون                      0x69
#وصف_ڪريو USB_PID_تياري                   0x2d

#وصف_ڪريو USB_DIR_ٻاهريون                     0               /* to device */
#وصف_ڪريو USB_DIR_اندريون                      0x80            /* to host */

#وصف_ڪريو USB_TYPE_نقاب                   (0x03 << 5)
#وصف_ڪريو USB_TYPE_STANDARD               (0x00 << 5)
#وصف_ڪريو USB_TYPE_CLASS                  (0x01 << 5)
#وصف_ڪريو USB_TYPE_VENDOR                 (0x02 << 5)
#وصف_ڪريو USB_TYPE_RESERVED               (0x03 << 5)

#وصف_ڪريو USB_RECIP_نقاب                  0x1f
#وصف_ڪريو USB_RECIP_DEVICE                0x00
#وصف_ڪريو USB_RECIP_INTERFACE             0x01
#وصف_ڪريو USB_RECIP_ENDPOINT              0x02
#وصف_ڪريو USB_RECIP_OTHER                 0x03

#وصف_ڪريو USB_REQ_GET_حالت              0x00
#وصف_ڪريو USB_REQ_CLEAR_FEATURE           0x01
#وصف_ڪريو USB_REQ_سيٽ_FEATURE             0x03
#وصف_ڪريو USB_REQ_سيٽ_پتو             0x05
#وصف_ڪريو USB_REQ_GET_DESCRIPTOR          0x06
#وصف_ڪريو USB_REQ_سيٽ_DESCRIPTOR          0x07
#وصف_ڪريو USB_REQ_GET_CONFIGURATION       0x08
#وصف_ڪريو USB_REQ_سيٽ_CONFIGURATION       0x09
#وصف_ڪريو USB_REQ_GET_INTERFACE           0x0A
#وصف_ڪريو USB_REQ_سيٽ_INTERFACE           0x0B
#وصف_ڪريو USB_REQ_SYNCH_چوکٺو             0x0C

بنية usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_قدر;
    u16 w_اشاريو;
    u16 w_ڊيگهائي;
} PACKED;

#وصف_ڪريو USB_DT_DEVICE                   0x01
#وصف_ڪريو USB_DT_CONFIG                   0x02
#وصف_ڪريو USB_DT_STRING                   0x03
#وصف_ڪريو USB_DT_INTERFACE                0x04
#وصف_ڪريو USB_DT_ENDPOINT                 0x05
#وصف_ڪريو USB_DT_DEVICE_QUALIFIER         0x06
#وصف_ڪريو USB_DT_OTHER_رفتار_CONFIG       0x07
#وصف_ڪريو USB_DT_ENDPOINT_COMPANION       0x30

بنية usb_device_descriptor {
    u8  b_ڊيگهائي;
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
    u8  i_Serial_عدد;
    u8  b_انگ_Configurations;
} PACKED;

#وصف_ڪريو USB_CLASS_في_INTERFACE         0       /* for DeviceClass */
#وصف_ڪريو USB_CLASS_AUDIO                 1
#وصف_ڪريو USB_CLASS_COMM                  2
#وصف_ڪريو USB_CLASS_HID                   3
#وصف_ڪريو USB_CLASS_PHYSICAL              5
#وصف_ڪريو USB_CLASS_STILL_IMAGE           6
#وصف_ڪريو USB_CLASS_PRINTER               7
#وصف_ڪريو USB_CLASS_MASS_STORAGE          8
#وصف_ڪريو USB_CLASS_HUB                   9

بنية usb_config_descriptor {
    u8  b_ڊيگهائي;
    u8  bDescriptorType;

    u16 w_ڪل_ڊيگهائي;
    u8  b_انگ_Interfaces;
    u8  b_Configuration_قدر;
    u8  iConfiguration;
    u8  bm_خاصيتون;
    u8  bMaxPower;
} PACKED;

بنية usb_interface_descriptor {
    u8  b_ڊيگهائي;
    u8  bDescriptorType;

    u8  b_Interface_عدد;
    u8  bAlternateSetting;
    u8  b_انگ_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

بنية usb_endpoint_descriptor {
    u8  b_ڊيگهائي;
    u8  bDescriptorType;

    u8  b_Endpoint_پتو;
    u8  bm_خاصيتون;
    u16 w_Max_Packet_سائيز;
    u8  bInterval;
} PACKED;

#وصف_ڪريو USB_ENDPOINT_عدد_نقاب        0x0f    /* in bEndpointAddress */
#وصف_ڪريو USB_ENDPOINT_DIR_نقاب           0x80

#وصف_ڪريو USB_ENDPOINT_XFERTYPE_نقاب      0x03    /* in bmAttributes */
#وصف_ڪريو USB_ENDPOINT_XFER_CONTROL       0
#وصف_ڪريو USB_ENDPOINT_XFER_ISOC          1
#وصف_ڪريو USB_ENDPOINT_XFER_BULK          2
#وصف_ڪريو USB_ENDPOINT_XFER_INT           3
#وصف_ڪريو USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#وصف_ڪريو USB_CONTROL_تياري_سائيز          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#وصف_ڪريو US_SC_ATAPI_8020   0x02
#وصف_ڪريو US_SC_ATAPI_8070   0x05
#وصف_ڪريو US_SC_SCSI         0x06

#وصف_ڪريو US_PR_BULK         0x50  /* bulk-only transport */
#وصف_ڪريو US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
صحيح usb_send_bulk(بنية usb_pipe *pipe, صحيح dir, فراغ *معلومات, صحيح datasize);
صحيح usb_poll_intr(بنية usb_pipe *pipe, فراغ *معلومات);
صحيح usb_32bit_pipe(بنية usb_pipe *pipe_fl);
بنية usb_pipe *usb_alloc_pipe(بنية usbdevice_s *usbdev
                                , بنية usb_endpoint_descriptor *epdesc);
فراغ usb_free_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *pipe);
صحيح usb_send_default_control(بنية usb_pipe *pipe
                             , ثابت بنية usb_ctrlrequest *req, فراغ *معلومات);
صحيح usb_is_freelist(بنية usb_s *cntl, بنية usb_pipe *pipe);
فراغ usb_جوڙيو_freelist(بنية usb_pipe *pipe);
بنية usb_pipe *usb_get_freelist(بنية usb_s *cntl, u8 eptype);
فراغ usb_desc2pipe(بنية usb_pipe *pipe, بنية usbdevice_s *usbdev
                   , بنية usb_endpoint_descriptor *epdesc);
صحيح usb_get_period(بنية usbdevice_s *usbdev
                   , بنية usb_endpoint_descriptor *epdesc);
صحيح usb_xfer_وقت(بنية usb_pipe *pipe, صحيح datalen);
بنية usb_endpoint_descriptor *usb_ڳوليو_desc(بنية usbdevice_s *usbdev
                                              , صحيح type, صحيح dir);
فراغ usb_enumerate(بنية usbhub_s *hub);
فراغ usb_تياري(فراغ);

#جيڪڏهن_پورو // usb.h
