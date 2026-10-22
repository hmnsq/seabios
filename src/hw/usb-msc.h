#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
sa_nracanaa disk_op_s;
පූර්ණ_සංඛ්‍යා usb_process_op(sa_nracanaa disk_op_s *op);
sa_nracanaa usbdevice_s;
පූර්ණ_සංඛ්‍යා usb_msc_setup(sa_nracanaa usbdevice_s *usbdev);

#endif // ush-msc.h
