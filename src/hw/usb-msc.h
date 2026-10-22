#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
ಸಂರಚನಾ disk_op_s;
ಪೂರ್ಣಾಙ್ಕ usb_process_op(ಸಂರಚನಾ disk_op_s *op);
ಸಂರಚನಾ usbdevice_s;
ಪೂರ್ಣಾಙ್ಕ usb_msc_setup(ಸಂರಚನಾ usbdevice_s *usbdev);

#endif // ush-msc.h
