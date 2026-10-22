#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
struct disk_op_s;
цео usb_process_op(struct disk_op_s *op);
struct usbdevice_s;
цео usb_msc_setup(struct usbdevice_s *usbdev);

#endif // ush-msc.h
