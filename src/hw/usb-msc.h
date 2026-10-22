#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
ساختار disk_op_s;
صحیح usb_process_op(ساختار disk_op_s *op);
ساختار usbdevice_s;
صحیح usb_msc_setup(ساختار usbdevice_s *usbdev);

#endif // ush-msc.h
