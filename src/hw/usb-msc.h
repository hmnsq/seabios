#ifndef __USB_MSC_H
#define __USB_MSC_H

// usb-msc.c
संरचना disk_op_s;
पूर्णांक usb_process_op(संरचना disk_op_s *op);
संरचना usbdevice_s;
पूर्णांक usb_msc_setup(संरचना usbdevice_s *usbdev);

#endif // ush-msc.h
