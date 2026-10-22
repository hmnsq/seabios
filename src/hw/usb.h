// USB functions and data.
#నిర్వచితం_కాకపోతే __USB_H
#నిర్వచించు __USB_H

#అన్తర్భావ "stacks.h" // struct mutex_s

// Information on a USB end point.
నిర్మాణం usb_pipe {
    సంఘం {
        నిర్మాణం usb_s *cntl;
        నిర్మాణం usb_pipe *freenext;
    };
    u8 type;
    u8 ep;
    u8 devaddr;
    u8 వేగం;
    u16 maxpacket;
    u8 eptype;
};

// Common information for usb devices.
నిర్మాణం usbdevice_s {
    నిర్మాణం usbhub_s *hub;
    నిర్మాణం usb_pipe *defpipe;
    u32 పోర్టు;
    నిర్మాణం usb_config_descriptor *config;
    నిర్మాణం usb_interface_descriptor *iface;
    పూర్ణాంకం imax;
    u8 వేగం;
    u8 devaddr;
};

// Common information for usb controllers.
నిర్మాణం usb_s {
    నిర్మాణం usb_pipe *freelist;
    నిర్మాణం mutex_s resetlock;
    నిర్మాణం pci_device *pci;
    శూన్య *mmio;
    u8 type;
    u8 maxaddr;
};

// Information for enumerating USB hubs
నిర్మాణం usbhub_s {
    నిర్మాణం usbhub_op_s *op;
    నిర్మాణం usbdevice_s *usbdev;
    నిర్మాణం usb_s *cntl;
    నిర్మాణం mutex_s తాళం_2;
    u32 detectend;
    u32 పోర్టు;
    u32 దారాలు;
    u32 portcount;
    u32 devcount;
};

// Hub callback (32bit) info
నిర్మాణం usbhub_op_s {
    పూర్ణాంకం (*detect)(నిర్మాణం usbhub_s *hub, u32 పోర్టు);
    పూర్ణాంకం (*reset)(నిర్మాణం usbhub_s *hub, u32 పోర్టు);
    పూర్ణాంకం (*portmap)(నిర్మాణం usbhub_s *hub, u32 పోర్టు);
    శూన్య (*disconnect)(నిర్మాణం usbhub_s *hub, u32 పోర్టు);
};

#నిర్వచించు USB_TYPE_UHCI  1
#నిర్వచించు USB_TYPE_OHCI  2
#నిర్వచించు USB_TYPE_EHCI  3
#నిర్వచించు USB_TYPE_XHCI  4

#నిర్వచించు USB_FULLSPEED  0
#నిర్వచించు USB_LOWSPEED   1
#నిర్వచించు USB_HIGHSPEED  2
#నిర్వచించు USB_SUPERSPEED 3

#నిర్వచించు USB_MAXADDR  127


/****************************************************************
 * usb structs and flags
 ****************************************************************/

// USB mandated timings (in ms)
#నిర్వచించు USB_సమయం_SIGATT 100
#నిర్వచించు USB_సమయం_ATTDB  100
#నిర్వచించు USB_సమయం_DRST   10
#నిర్వచించు USB_సమయం_DRSTR  50
#నిర్వచించు USB_సమయం_RSTRCY 10

#నిర్వచించు USB_సమయం_స్థితి  50
#నిర్వచించు USB_సమయం_DATAIN  500
#నిర్వచించు USB_సమయం_COMMAND 5000

#నిర్వచించు USB_సమయం_SETADDR_RECOVERY 2

#నిర్వచించు USB_PID_బహిర్గతం                     0xe1
#నిర్వచించు USB_PID_లోపల                      0x69
#నిర్వచించు USB_PID_అమరిక                   0x2d

#నిర్వచించు USB_DIR_బహిర్గతం                     0               /* to device */
#నిర్వచించు USB_DIR_లోపల                      0x80            /* to host */

#నిర్వచించు USB_TYPE_ముసుగు                   (0x03 << 5)
#నిర్వచించు USB_TYPE_STANDARD               (0x00 << 5)
#నిర్వచించు USB_TYPE_CLASS                  (0x01 << 5)
#నిర్వచించు USB_TYPE_VENDOR                 (0x02 << 5)
#నిర్వచించు USB_TYPE_RESERVED               (0x03 << 5)

#నిర్వచించు USB_RECIP_ముసుగు                  0x1f
#నిర్వచించు USB_RECIP_DEVICE                0x00
#నిర్వచించు USB_RECIP_INTERFACE             0x01
#నిర్వచించు USB_RECIP_ENDPOINT              0x02
#నిర్వచించు USB_RECIP_OTHER                 0x03

#నిర్వచించు USB_REQ_GET_స్థితి              0x00
#నిర్వచించు USB_REQ_CLEAR_FEATURE           0x01
#నిర్వచించు USB_REQ_సమితి_FEATURE             0x03
#నిర్వచించు USB_REQ_సమితి_చిరునామా             0x05
#నిర్వచించు USB_REQ_GET_DESCRIPTOR          0x06
#నిర్వచించు USB_REQ_సమితి_DESCRIPTOR          0x07
#నిర్వచించు USB_REQ_GET_CONFIGURATION       0x08
#నిర్వచించు USB_REQ_సమితి_CONFIGURATION       0x09
#నిర్వచించు USB_REQ_GET_INTERFACE           0x0A
#నిర్వచించు USB_REQ_సమితి_INTERFACE           0x0B
#నిర్వచించు USB_REQ_SYNCH_చట్రం             0x0C

నిర్మాణం usb_ctrlrequest {
    u8 bRequestType;
    u8 bRequest;
    u16 w_విలువ;
    u16 w_సూచీ;
    u16 w_నిడివి;
} PACKED;

#నిర్వచించు USB_DT_DEVICE                   0x01
#నిర్వచించు USB_DT_CONFIG                   0x02
#నిర్వచించు USB_DT_STRING                   0x03
#నిర్వచించు USB_DT_INTERFACE                0x04
#నిర్వచించు USB_DT_ENDPOINT                 0x05
#నిర్వచించు USB_DT_DEVICE_QUALIFIER         0x06
#నిర్వచించు USB_DT_OTHER_వేగం_CONFIG       0x07
#నిర్వచించు USB_DT_ENDPOINT_COMPANION       0x30

నిర్మాణం usb_device_descriptor {
    u8  b_నిడివి;
    u8  bDescriptorType;

    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 id_లబ్ధం;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  i_లబ్ధం;
    u8  i_Serial_సంఖ్య;
    u8  b_అంకె_Configurations;
} PACKED;

#నిర్వచించు USB_CLASS_చొప్పున_INTERFACE         0       /* for DeviceClass */
#నిర్వచించు USB_CLASS_AUDIO                 1
#నిర్వచించు USB_CLASS_COMM                  2
#నిర్వచించు USB_CLASS_HID                   3
#నిర్వచించు USB_CLASS_PHYSICAL              5
#నిర్వచించు USB_CLASS_STILL_IMAGE           6
#నిర్వచించు USB_CLASS_PRINTER               7
#నిర్వచించు USB_CLASS_MASS_STORAGE          8
#నిర్వచించు USB_CLASS_HUB                   9

నిర్మాణం usb_config_descriptor {
    u8  b_నిడివి;
    u8  bDescriptorType;

    u16 w_మొత్తం_నిడివి;
    u8  b_అంకె_Interfaces;
    u8  b_Configuration_విలువ;
    u8  iConfiguration;
    u8  bm_లక్షణాలు;
    u8  bMaxPower;
} PACKED;

నిర్మాణం usb_interface_descriptor {
    u8  b_నిడివి;
    u8  bDescriptorType;

    u8  b_Interface_సంఖ్య;
    u8  bAlternateSetting;
    u8  b_అంకె_Endpoints;
    u8  bInterfaceClass;
    u8  bInterfaceSubClass;
    u8  bInterfaceProtocol;
    u8  iInterface;
} PACKED;

నిర్మాణం usb_endpoint_descriptor {
    u8  b_నిడివి;
    u8  bDescriptorType;

    u8  b_Endpoint_చిరునామా;
    u8  bm_లక్షణాలు;
    u16 w_Max_Packet_కొలత;
    u8  bInterval;
} PACKED;

#నిర్వచించు USB_ENDPOINT_సంఖ్య_ముసుగు        0x0f    /* in bEndpointAddress */
#నిర్వచించు USB_ENDPOINT_DIR_ముసుగు           0x80

#నిర్వచించు USB_ENDPOINT_XFERTYPE_ముసుగు      0x03    /* in bmAttributes */
#నిర్వచించు USB_ENDPOINT_XFER_CONTROL       0
#నిర్వచించు USB_ENDPOINT_XFER_ISOC          1
#నిర్వచించు USB_ENDPOINT_XFER_BULK          2
#నిర్వచించు USB_ENDPOINT_XFER_INT           3
#నిర్వచించు USB_ENDPOINT_MAX_ADJUSTABLE     0x80

#నిర్వచించు USB_CONTROL_అమరిక_కొలత          8


/****************************************************************
 * usb mass storage flags
 ****************************************************************/

#నిర్వచించు US_SC_ATAPI_8020   0x02
#నిర్వచించు US_SC_ATAPI_8070   0x05
#నిర్వచించు US_SC_SCSI         0x06

#నిర్వచించు US_PR_BULK         0x50  /* bulk-only transport */
#నిర్వచించు US_PR_UAS          0x62  /* usb attached scsi   */

/****************************************************************
 * function defs
 ****************************************************************/

// usb.c
పూర్ణాంకం usb_send_bulk(నిర్మాణం usb_pipe *pipe, పూర్ణాంకం dir, శూన్య *దత్తాంశం, పూర్ణాంకం datasize);
పూర్ణాంకం usb_poll_intr(నిర్మాణం usb_pipe *pipe, శూన్య *దత్తాంశం);
పూర్ణాంకం usb_32bit_pipe(నిర్మాణం usb_pipe *pipe_fl);
నిర్మాణం usb_pipe *usb_alloc_pipe(నిర్మాణం usbdevice_s *usbdev
                                , నిర్మాణం usb_endpoint_descriptor *epdesc);
శూన్య usb_free_pipe(నిర్మాణం usbdevice_s *usbdev, నిర్మాణం usb_pipe *pipe);
పూర్ణాంకం usb_send_default_control(నిర్మాణం usb_pipe *pipe
                             , స్థిరరాశి నిర్మాణం usb_ctrlrequest *req, శూన్య *దత్తాంశం);
పూర్ణాంకం usb_is_freelist(నిర్మాణం usb_s *cntl, నిర్మాణం usb_pipe *pipe);
శూన్య usb_కూడు_freelist(నిర్మాణం usb_pipe *pipe);
నిర్మాణం usb_pipe *usb_get_freelist(నిర్మాణం usb_s *cntl, u8 eptype);
శూన్య usb_desc2pipe(నిర్మాణం usb_pipe *pipe, నిర్మాణం usbdevice_s *usbdev
                   , నిర్మాణం usb_endpoint_descriptor *epdesc);
పూర్ణాంకం usb_get_period(నిర్మాణం usbdevice_s *usbdev
                   , నిర్మాణం usb_endpoint_descriptor *epdesc);
పూర్ణాంకం usb_xfer_సమయం(నిర్మాణం usb_pipe *pipe, పూర్ణాంకం datalen);
నిర్మాణం usb_endpoint_descriptor *usb_కనుగొను_desc(నిర్మాణం usbdevice_s *usbdev
                                              , పూర్ణాంకం type, పూర్ణాంకం dir);
శూన్య usb_enumerate(నిర్మాణం usbhub_s *hub);
శూన్య usb_అమరిక(శూన్య);

#అయితే_ముగింపు // usb.h
