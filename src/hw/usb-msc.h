#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
ସଂରଚନା disk_op_s;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_process_op(ସଂରଚନା disk_op_s *op);
ସଂରଚନା usbdevice_s;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_msc_setup(ସଂରଚନା usbdevice_s *usbdev);

#endif // ush-msc.h
